// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 Samsung Electronics Co., Ltd.
 */

#include <linux/device-mapper.h>

#define DM_MSG_PREFIX "zoned-npo2"

struct dmz_npo2_target {
	struct dm_dev *dev;
	sector_t zsze;
	sector_t zsze_po2;
	sector_t zsze_diff;
	u32 nr_zones;
};

enum dmz_npo2_io_cond {
	DMZ_NPO2_IO_INSIDE_ZONE,
	DMZ_NPO2_IO_ACROSS_ZONE,
	DMZ_NPO2_IO_OUTSIDE_ZONE,
};

static inline u32 npo2_zone_no(struct dmz_npo2_target *dmh, sector_t sect)
{
	return div64_u64(sect, dmh->zsze);
}

static inline u32 po2_zone_no(struct dmz_npo2_target *dmh, sector_t sect)
{
	return sect >> ilog2(dmh->zsze_po2);
}

static inline sector_t target_to_device_sect(struct dmz_npo2_target *dmh,
					     sector_t sect)
{
	u32 zone_idx = po2_zone_no(dmh, sect);

	sect -= (zone_idx * dmh->zsze_diff);

	return sect;
}

static inline sector_t device_to_target_sect(struct dmz_npo2_target *dmh,
					     sector_t sect)
{
	u32 zone_idx = npo2_zone_no(dmh, sect);

	sect += (zone_idx * dmh->zsze_diff);

	return sect;
}

/*
 * <dev-path>
 * This target works on the complete zoned device. Partial mapping is not
 * supported
 */
static int dmz_npo2_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	struct dmz_npo2_target *dmh = NULL;
	int ret = 0;
	sector_t zsze;
	sector_t disk_size;

	if (argc < 1)
		return -EINVAL;

	dmh = kmalloc(sizeof(*dmh), GFP_KERNEL);
	if (!dmh)
		return -ENOMEM;

	ret = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table),
			    &dmh->dev);

	zsze = bdev_zone_sectors(dmh->dev->bdev);

	disk_size = get_capacity(dmh->dev->bdev->bd_disk);

	if (ti->len != disk_size || ti->begin) {
		DMERR("%pg Partial mapping of the target not supported",
		      dmh->dev->bdev);
		return -EINVAL;
	}

	if (is_power_of_2(zsze)) {
		DMERR("%pg zone size is power of 2", dmh->dev->bdev);
		return -EINVAL;
	}

	dmh->zsze = zsze;
	dmh->zsze_po2 = 1 << get_count_order_long(zsze);
	dmh->zsze_diff = dmh->zsze_po2 - dmh->zsze;

	ti->private = dmh;
	ti->num_flush_bios = 1;
	ti->num_discard_bios = 1;
	ti->num_secure_erase_bios = 1;
	ti->num_write_zeroes_bios = 1;

	dmh->nr_zones = npo2_zone_no(dmh, ti->len);
	ti->len = dmh->zsze_po2 * dmh->nr_zones;

	return 0;
}

static int dmz_npo2_report_zones_cb(struct blk_zone *zone, unsigned int idx,
				    void *data)
{
	struct dm_report_zones_args *args = data;
	struct dmz_npo2_target *dmh = args->tgt->private;

	zone->start = device_to_target_sect(dmh, zone->start);
	zone->wp = device_to_target_sect(dmh, zone->wp);
	zone->len = dmh->zsze_po2;
	args->next_sector = zone->start + zone->len;

	return args->orig_cb(zone, args->zone_idx++, args->orig_data);
}

static int dmz_npo2_report_zones(struct dm_target *ti,
				 struct dm_report_zones_args *args,
				 unsigned int nr_zones)
{
	struct dmz_npo2_target *dmh = ti->private;
	int ret = 0;
	sector_t sect = po2_zone_no(dmh, args->next_sector) * dmh->zsze;

	ret = blkdev_report_zones(dmh->dev->bdev, sect, nr_zones,
				  dmz_npo2_report_zones_cb, args);
	if (ret < 0)
		DMERR("report zones error");

	return ret;
}

static int check_zone_boundary_violation(struct dmz_npo2_target *dmh,
					 sector_t sect, sector_t size)
{
	u32 zone_idx = po2_zone_no(dmh, sect);
	sector_t relative_sect = 0;

	sect = target_to_device_sect(dmh, sect);
	relative_sect = sect - (zone_idx * dmh->zsze);

	if ((relative_sect + size) <= dmh->zsze)
		return DMZ_NPO2_IO_INSIDE_ZONE;
	else if (relative_sect >= dmh->zsze)
		return DMZ_NPO2_IO_OUTSIDE_ZONE;

	return DMZ_NPO2_IO_ACROSS_ZONE;
}

static void split_io_across_zone_boundary(struct dmz_npo2_target *dmh,
					  struct bio *bio)
{
	sector_t sect = bio->bi_iter.bi_sector;
	sector_t sects_from_zone_start;

	sect = target_to_device_sect(dmh, sect);
	div64_u64_rem(sect, dmh->zsze, &sects_from_zone_start);
	dm_accept_partial_bio(bio, dmh->zsze - sects_from_zone_start);
	bio->bi_iter.bi_sector = sect;
}

static int handle_zone_boundary_violation(struct dmz_npo2_target *dmh,
					  struct bio *bio,
					  enum dmz_npo2_io_cond cond)
{
	/* Read should return zeroed page */
	if (bio_op(bio) == REQ_OP_READ) {
		if (cond == DMZ_NPO2_IO_ACROSS_ZONE) {
			split_io_across_zone_boundary(dmh, bio);
			return DM_MAPIO_REMAPPED;
		}
		zero_fill_bio(bio);
		bio_endio(bio);
		return DM_MAPIO_SUBMITTED;
	}
	return DM_MAPIO_KILL;
}

static int dmz_npo2_end_io(struct dm_target *ti, struct bio *bio,
			   blk_status_t *error)
{
	struct dmz_npo2_target *dmh = ti->private;

	if (bio->bi_status == BLK_STS_OK && bio_op(bio) == REQ_OP_ZONE_APPEND)
		bio->bi_iter.bi_sector =
			device_to_target_sect(dmh, bio->bi_iter.bi_sector);

	return DM_ENDIO_DONE;
}

static int dmz_npo2_map(struct dm_target *ti, struct bio *bio)
{
	struct dmz_npo2_target *dmh = ti->private;
	enum dmz_npo2_io_cond cond;

	bio_set_dev(bio, dmh->dev->bdev);
	if (bio_sectors(bio) || op_is_zone_mgmt(bio_op(bio))) {
		cond = check_zone_boundary_violation(dmh, bio->bi_iter.bi_sector,
						     bio->bi_iter.bi_size >> SECTOR_SHIFT);

		/*
		 * If the starting sector is in the emulated area then fill
		 * all the bio with zeros. If bio is across boundaries,
		 * split the bio across boundaries and fill zeros only for the
		 * bio that is outside the zone capacity
		 */
		switch (cond) {
		case DMZ_NPO2_IO_INSIDE_ZONE:
			bio->bi_iter.bi_sector = target_to_device_sect(dmh,
								       bio->bi_iter.bi_sector);
			break;
		case DMZ_NPO2_IO_ACROSS_ZONE:
		case DMZ_NPO2_IO_OUTSIDE_ZONE:
			return handle_zone_boundary_violation(dmh, bio, cond);
		}
	}
	return DM_MAPIO_REMAPPED;
}

static int dmz_npo2_iterate_devices(struct dm_target *ti,
				    iterate_devices_callout_fn fn, void *data)
{
	struct dmz_npo2_target *dmh = ti->private;
	sector_t len = 0;

	len = dmh->nr_zones * dmh->zsze;
	return fn(ti, dmh->dev, 0, len, data);
}

static struct target_type dmz_npo2_target = {
	.name = "zoned-npo2",
	.version = { 1, 0, 0 },
	.features = DM_TARGET_ZONED_HM,
	.map = dmz_npo2_map,
	.end_io = dmz_npo2_end_io,
	.report_zones = dmz_npo2_report_zones,
	.iterate_devices = dmz_npo2_iterate_devices,
	.module = THIS_MODULE,
	.ctr = dmz_npo2_ctr,
};

static int __init dmz_npo2_init(void)
{
	int r = dm_register_target(&dmz_npo2_target);

	if (r < 0)
		DMERR("register failed %d", r);

	return r;
}

static void __exit dmz_npo2_exit(void)
{
	dm_unregister_target(&dmz_npo2_target);
}

/* Module hooks */
module_init(dmz_npo2_init);
module_exit(dmz_npo2_exit);

MODULE_DESCRIPTION(DM_NAME " non power 2 zoned target");
MODULE_AUTHOR("Pankaj Raghav <p.raghav@samsung.com>");
MODULE_LICENSE("GPL");


#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define iov_iter_len 2
#define PAGE_SIZE 16384

/* Run:
 *
 * $ mkfs.xfs -f -b size=16k, -s size=4k /dev/nvme1n1
 * $ mount -t xfs /dev/nvme1n1 /mnt
 * $ gcc splice-xfs.c -o splice-xfs
 * $ ./splice-xfs /mnt/test /mnt/test1
 *
 */
int verify_buf(int fd, loff_t offset, loff_t len, char *ref) {
	char *readbuf = malloc(PAGE_SIZE);
	int ret = 0;

	memset(readbuf, 0, PAGE_SIZE);

	ret = pread(fd, readbuf, len, offset);

	if (memcmp(ref, readbuf, len)) {
		return -1;
	}

	return 0;
}

int main(int argc, char **argv) {
	int ret = 0;
	struct iovec iov[iov_iter_len];
	char *write_buf[iov_iter_len];

	if (argc != 3) {
		printf("./pwrite-xfs file1 file2\n");
		exit(1);
	}

	int fd = open(argv[1], O_RDWR | O_CREAT | O_DIRECT);

	if (fd < 0) {
		printf("Error opening the file \n");
		return 1;
	}

	for (int i = 0; i < iov_iter_len; ++i) {
		write_buf[i] = malloc(PAGE_SIZE);
		memset(write_buf[i], 'A' + i, PAGE_SIZE);
		iov[i].iov_base = write_buf[i];
		iov[i].iov_len = PAGE_SIZE;
	}

	ret = pwritev2(fd, iov, iov_iter_len, 0, 0);

	if (ret < 0) {
		printf("pwritev failed \n");
		return -1;
	}

	if (ret != (2 * PAGE_SIZE)) {
		printf("pwritev partial write \n");
		return -1;
	}

	printf("Successfully wrote 2 pages \n");

	ret = pwrite(fd, write_buf[1], PAGE_SIZE, PAGE_SIZE / 2);

	if (ret < 0) {
		printf("pwrite failed \n");
		return -1;
	}

	if (ret != (PAGE_SIZE)) {
		printf("pwrite partial write \n");
		return -1;
	}

	printf("Verify section!! \n");

	ret = verify_buf(fd, 0, PAGE_SIZE / 2, write_buf[0]);

	if (ret < 0) return -1;

	ret = verify_buf(fd, (PAGE_SIZE / 2), PAGE_SIZE, write_buf[1]);

	if (ret < 0) return -1;

	ret = verify_buf(fd, PAGE_SIZE + (PAGE_SIZE / 2), PAGE_SIZE / 2,
			 write_buf[1]);

	if (ret < 0) return -1;

	printf("Copy file range \n");
	close(fd);

	// Open without O_DIRECT to pull it into the page cache
	fd = open(argv[1], O_RDWR | O_CREAT);

	if (fd < 0) {
		printf("Error opening the file \n");
		return 1;
	}
	int fd1 = open(argv[2], O_RDWR | O_CREAT);
	if (fd1 < 0) {
		printf("Error opening the second file \n");
		return 1;
	}

	loff_t in, out;

	in = PAGE_SIZE / 2;
	out = 0;

	// This should trigger an OOPs in the kernel
	ret = copy_file_range(fd, &in, fd1, &out, PAGE_SIZE, 0);

	if (ret < 0) {
		printf("copy file range failed \n");
		return -1;
	}

	return 0;
}

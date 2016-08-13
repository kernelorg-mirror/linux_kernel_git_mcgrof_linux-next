#define pr_fmt(fmt) "ACME: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>

void *acme_todo(void *arg);
static DECLARE_WORK(acme_work, acme_todo);

void *acme_todo(void *arg)
{
	pr_info("Running scheduled work\n");
	pthread_exit(NULL);
}

static int acme_init_driver(void) {
	pr_info("Initializing ...\n");
	sleep(2);
	pr_info("Finished init ... !\n");

	init_work(&acme_work);
	schedule_work(&acme_work);

	return 0;
}
module_init(acme_init_driver);

static void acme_exit(void)
{
	cancel_work_sync(&acme_work);
};
module_exit(acme_exit);

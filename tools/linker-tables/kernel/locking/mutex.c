#include <linux/kernel.h>
#include <linux/mutex.h>

DEFINE_SECTION_RANGE(sched_text, SECTION_TEXT);

void __sched mutex_init(struct mutex *lock)
{
	int r;

	r = pthread_mutex_init(&lock->lock, NULL);
	if (r)
		BUG_ON(r);
}

void __sched mutex_destroy(struct mutex *lock)
{
	pthread_mutex_destroy(&lock->lock);
}

void __sched mutex_lock(struct mutex *lock)
{
	pthread_mutex_lock(&lock->lock);
}

void __sched mutex_unlock(struct mutex *lock)
{
	pthread_mutex_unlock(&lock->lock);
}

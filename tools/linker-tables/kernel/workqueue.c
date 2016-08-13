#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <linux/workqueue.h>
#include <linux/kernel.h>

void schedule_work(struct work *w)
{
	mutex_lock(&w->mutex);
	pthread_cond_signal(&w->cond);
	mutex_unlock(&w->mutex);
}

void cancel_work_sync(struct work *w)
{
	pthread_exit(NULL);
}

void *run_work(void *arg)
{
	struct work *w;
	int r;

	w = (struct work *) arg;

	mutex_lock(&w->mutex);

	while (true) {
		if (!w->ready)
			w->ready = true;
		r = pthread_cond_wait(&w->cond, &w->mutex.lock);
		if (r != 0) {
			printf("(%s)\n", strerror(r));
			BUG_ON(r);
		}
		w->work_cb(w->arg);
	}

	mutex_unlock(&w->mutex);

	pthread_exit(NULL);
}

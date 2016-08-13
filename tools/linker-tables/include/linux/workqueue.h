#ifndef _LINUX_WORKQUEUE_H
#define _LINUX_WORKQUEUE_H

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <pthread.h>

struct work {
	bool ready;

	pthread_t thread;
	struct mutex mutex;
	pthread_cond_t cond;

	void *arg;
	void *(*work_cb)(void *arg);
};

#define DECLARE_WORK(_w, _w_cb) \
struct work _w = { \
	.work_cb = _w_cb, \
	.arg = NULL, \
};

extern void *run_work(void *arg);

static inline void init_work(struct work *w)
{
	w->ready = false;

	mutex_init(&w->mutex);
	pthread_cond_init(&w->cond, NULL);

	pthread_create(&w->thread, NULL, run_work, (void *) w);

	while (1) {
		mutex_lock(&w->mutex);
		if (w->ready) {
			pthread_mutex_unlock(&w->mutex.lock);
			break;
		}
		mutex_unlock(&w->mutex);
	}
}

void schedule_work(struct work *w);
void cancel_work_sync(struct work *w);
void init_work(struct work *w);

#endif /* _LINUX_WORKQUEUE_H */

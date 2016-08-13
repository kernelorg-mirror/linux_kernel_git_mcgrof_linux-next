#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include <sys/types.h>
#include <linux/sched.h>
#include <pthread.h>

struct mutex {
	pthread_mutex_t lock;
};

void mutex_init(struct mutex *lock);
void mutex_destroy(struct mutex *lock);
void mutex_lock(struct mutex *lock);
void mutex_unlock(struct mutex *lock);

#endif /* __LINUX_MUTEX_H */

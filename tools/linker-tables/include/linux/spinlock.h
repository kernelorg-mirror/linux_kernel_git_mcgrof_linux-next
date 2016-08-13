#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

#include <pthread.h>

#define spinlock_t pthread_spinlock_t

void spin_lock_init(spinlock_t *lock);
void spin_lock_destroy(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif /* __LINUX_SPINLOCK_H */

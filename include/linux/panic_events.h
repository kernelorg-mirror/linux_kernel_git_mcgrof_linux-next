// SPDX-License-Identifier: GPL-2.0+
#ifndef _LINUX_PANIC_EVENTS_H

#include <uapi/linux/panic_events.h>

#ifdef CONFIG_PANIC_EVENTS

void panic_events_init(void);
void panic_uevent(enum panic_uevent event);
void panic_uevent_taint(unsigned flag, struct module *mod);

#else
static inline void panic_events_init(void)
{
}

static inline panic_uevent(enum panic_uevent event)
{
}

static inline void panic_uevent_taint(unsigned flag, struct module *mod)
{
}
#endif

#endif /* _LINUX_PANIC_EVENTS_H */

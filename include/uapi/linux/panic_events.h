/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef _UAPI_PANIC_EVENTS_H
#define _UAPI_PANIC_EVENTS_H

/**
 * enum panic_uevent - panic uevents
 *
 * @PANIC_LOCKDEP_DISABLED: lockdep has been disabled
 * @PANIC_TAINT: lockdep has been disabled
 */
enum panic_uevent {
	PANIC_LOCKDEP_DISABLED,
	PANIC_TAINT,
	__PANIC_MAX, /* non-ABI */
};

#endif /* _UAPI_PANIC_EVENTS_H */

#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#include <linux/ranges.h>

DECLARE_SECTION_RANGE(sched_text);
#define __sched		__LINUX_RANGE(SECTION_TEXT, sched_text)

#endif /* _LINUX_SCHED_H */

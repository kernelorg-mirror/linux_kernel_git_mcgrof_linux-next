// SPDX-License-Identifier: GPL-2.0+

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/panic_events.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>

/**
 *  DOC: Panic events
 *
 *  Panic events are sent to userspace to inform userspace that something
 *  critical has happened to the kernel. These events can happen in any
 *  context, and so to send these events to userspace we preallocate memory
 *  needed during initialization as needed for operation. The events are
 *  queued and later dispatched. The uevents sent are best effort, if we are
 *  short of memory kobject_uevent_env() can fail.
 */

/* The max amount of lines on a uevent we support */
#define PANIC_UEVENT_MAX_LINES	8

/* We assume each possible CPU can trigger these events */
#define PANIC_MAX_EVENTS_PER_CPU 4

/* The max number of concurrent uvents we support, otherwise we drop events */
#define PANIC_NUM_CACHE_EVENTS num_possible_cpus() * PANIC_MAX_EVENTS_PER_CPU

static LIST_HEAD(panic_free_list);
static spinlock_t free_lock;

static LIST_HEAD(panic_pend_list);
static spinlock_t pend_lock;

struct panic_event {
	enum panic_uevent uevent;
	char module_name[MODULE_NAME_LEN];
	enum system_states sys_state;
	unsigned flag;
	struct list_head list;
};

static struct panic_event *panic_events;

static struct kset *panic_kset;
static struct kobj_type empty_ktype;
static struct kobject *panic_events_kobj;
static struct kobject _panic_events_kobj;

static void panic_process_events(struct work_struct *work);
static DECLARE_WORK(panic_events_work, panic_process_events);

static char (*panic_envp)[PATH_MAX];
/* Protects panic_envp */
static DEFINE_MUTEX(panic_mutex);

struct panic_event *get_free_panic_event(void)
{
	struct panic_event *event = NULL;

	spin_lock(&free_lock);
	if (list_empty(&panic_free_list)) {
		pr_warn_once("Not enough free panic pool events, we need to bump PANIC_NUM_CACHE_EVENTS, please report this\n");
		goto out;
	}
	event = list_first_entry(&panic_free_list, struct panic_event, list);
	list_del_init(&event->list);

out:
	spin_unlock(&free_lock);

	return event;
}

static void queue_panic_event(struct panic_event *event)
{
	spin_lock(&pend_lock);
	list_add_tail(&event->list, &panic_pend_list);
	spin_unlock(&pend_lock);
}

struct panic_event *get_pend_panic_event(void)
{
	struct panic_event *event = NULL;

	spin_lock(&pend_lock);
	if (list_empty(&panic_pend_list))
		goto out;

	event = list_first_entry(&panic_pend_list, struct panic_event, list);
	list_del_init(&event->list);

out:
	spin_unlock(&pend_lock);

	return event;
}

static void panic_send_event(struct panic_event *event)
{
	unsigned int idx = 0, i;
	bool pending = false;
	char *envp[PANIC_UEVENT_MAX_LINES];
	int r;

	mutex_lock(&panic_mutex);
	memset(panic_envp, 0, PATH_MAX * PANIC_UEVENT_MAX_LINES);
	snprintf(panic_envp[idx++], PATH_MAX, "SYSTEM_STATE=%d",
		 event->sys_state);
	snprintf(panic_envp[idx++], PATH_MAX, "EVENT=%d", event->uevent);

	if (event->uevent == PANIC_LOCKDEP_DISABLED)
		goto out_send;

	/*
	 * add_taint_module() will trigger two uevents, one for the kernel,
	 * and another for the module, if the module was not built-in.
	 */
	if (event->uevent == PANIC_TAINT) {
		snprintf(panic_envp[idx++], PATH_MAX, "TAINT=%d", event->flag);
		if (strcmp(event->module_name, "") != 0)
			snprintf(panic_envp[idx++], PATH_MAX, "MODULE_NAME=%s",
				 event->module_name);
	}

out_send:
	for (i=0; i < idx; i++)
		envp[i] = panic_envp[i];
	envp[idx] = NULL;

	r = kobject_uevent_env(panic_events_kobj, KOBJ_CHANGE, envp);
	if (!r)
		pr_debug("failed to sent uevent: %d\n", event->uevent);
	mutex_unlock(&panic_mutex);

	memset(event, 0, sizeof(struct panic_event));

	spin_lock(&free_lock);
	list_add_tail(&event->list, &panic_free_list);
	spin_unlock(&free_lock);

	spin_lock(&pend_lock);
	if (!list_empty(&panic_pend_list))
		pending = true;
	spin_unlock(&pend_lock);

	if (pending)
		schedule_work(&panic_events_work);
}

static void panic_process_events(struct work_struct *work)
{
	struct panic_event *event;
	bool pending = false;

	event = get_pend_panic_event();
	if (!event)
		goto out;

	panic_send_event(event);

out:
	spin_lock(&pend_lock);
	if (!list_empty(&panic_pend_list))
		pending = true;
	spin_unlock(&pend_lock);

	if (pending)
		schedule_work(&panic_events_work);
}

/* For simple panic uvents which only need an event type specified */
void panic_uevent(enum panic_uevent uevent)
{
	struct panic_event *event;

	if (!panic_events_kobj)
		return;

	event = get_free_panic_event();
	if (!event)
		return;

	event->uevent = uevent;
	event->sys_state = system_state;

	queue_panic_event(event);
	schedule_work(&panic_events_work);
}

void panic_uevent_taint(unsigned flag, struct module *mod)
{
	struct panic_event *event;

	if (!panic_events_kobj)
		return;

	event = get_free_panic_event();
	if (!event)
		return;

	event->uevent = PANIC_TAINT;
	event->sys_state = system_state;
	event->flag = flag;

	if (mod)
		strncpy(event->module_name, module_name(mod), MODULE_NAME_LEN);

	queue_panic_event(event);
	schedule_work(&panic_events_work);
}

static __init void panic_events_init(void)
{
	struct panic_event *event;
	char *envp[] =  { NULL, NULL };
	unsigned int i;
	size_t used;
	int r;

	spin_lock_init(&free_lock);
	spin_lock_init(&pend_lock);

	mutex_lock(&panic_mutex);

	panic_envp = kzalloc(PATH_MAX * PANIC_UEVENT_MAX_LINES, GFP_KERNEL);
	if (!panic_envp) {
		pr_warn("Could not preallocate environment for panic events\n");
		goto out_unlock;
	}

	panic_events = kzalloc(sizeof(struct panic_event) *
			       PANIC_NUM_CACHE_EVENTS, GFP_KERNEL);
	if (!panic_events) {
		pr_warn("Could not preallocate panic events\n");
		goto out_env;
	}

	for (i=0; i < PANIC_NUM_CACHE_EVENTS; i++) {
		event = &panic_events[i];
		list_add_tail(&event->list, &panic_free_list);
	}

	snprintf(panic_envp[0], PATH_MAX, "MAX_EVENT_SUPPORTED=%d",
		 __PANIC_MAX-1);

	envp[0] = panic_envp[0];

	panic_kset = kset_create_and_add("panic", NULL, kernel_kobj);
	if (!panic_kset)
		goto out_events;

	_panic_events_kobj.kset = panic_kset;

	r = kobject_init_and_add(&_panic_events_kobj, &empty_ktype,
				 NULL, "events");
	if (r) {
		pr_warn("Could not add panic events kobject\n");
		goto out_kset;
	}

	/* Without this set this infrastructure is ignored */
	panic_events_kobj = &_panic_events_kobj;

	/* Inform userspace which events we'll send uevents for */
	r = kobject_uevent_env(panic_events_kobj, KOBJ_ADD, envp);
	if (r != 0)
		pr_debug("failed to send first event\n");

	mutex_unlock(&panic_mutex);

	used = (PATH_MAX * PANIC_UEVENT_MAX_LINES) +
	       (sizeof(struct panic_event) * PANIC_NUM_CACHE_EVENTS);

	pr_info("panic events initialized using %zu preallocated bytes\n", used);

	return;

out_kset:
	kset_unregister(panic_kset);
out_events:
	kfree(panic_events);
out_env:
	kfree(panic_envp);
out_unlock:
	mutex_unlock(&panic_mutex);
}

core_initcall(panic_events_init);

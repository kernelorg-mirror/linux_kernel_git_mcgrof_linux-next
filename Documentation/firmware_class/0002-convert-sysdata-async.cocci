// You can use this to help convert a device driver from the old firmware
// request_firmware_nowait() API the new flexible sysdata API for async
// requests.
//
// Confidence: Medium
//
// Reason for low confidence:
//
// Copyright: (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org> GPLv2.
// Copyright: (C) 2016 Julia Lawall, Inria/LIP6.  GPLv2
//
// Options: --include-headers --in-place --no-show-diff
// Requires: 1.0.5

virtual patch

@ async_get_t @
expression name, dev;
bool true;
identifier drv_callback;
type T;
T *drv;
@@

(
request_firmware_nowait(THIS_MODULE, true, name, dev, GFP_KERNEL, drv, drv_callback)
|
request_firmware_nowait(THIS_MODULE, 1, name, dev, GFP_KERNEL, drv, drv_callback)
|
request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG, name, dev, GFP_KERNEL, drv, drv_callback)
)

@ find_request_async depends on async_get_t @
expression name, dev, uevent;
identifier drv_callback, drv, f;
@@

f (...)
{
<+...
-request_firmware_nowait(THIS_MODULE, uevent, name, dev, GFP_KERNEL, drv, drv_callback)
+sysdata_file_request_async(name, &sysdata_desc, dev, &drv->sysdata_async_cookie)
...+>
}

@ add_desc_async @
type T;
identifier find_request_async.f;
identifier find_request_async.drv;
identifier find_request_async.drv_callback;
@@

f (...)
{
...
T *drv;
+const struct sysdata_file_desc sysdata_desc = {
+	SYSDATA_DEFAULT_ASYNC(drv_callback, drv),
+};
...
}

@ add_desc_direct_async @
type T;
identifier find_request_async.f;
identifier find_request_async.drv;
identifier find_request_async.drv_callback;
@@

f (...,
   T *drv
   ,...)
{
+const struct sysdata_file_desc sysdata_desc = {
+	SYSDATA_DEFAULT_ASYNC(drv_callback, drv),
+};
...
}

@ found_callback depends on async_get_t @
identifier find_request_async.drv_callback;
identifier find_request_async.drv;
identifier data, arg;
type T1;
@@

 drv_callback(
-const struct firmware *data,
+const struct sysdata_file *data,
 void *arg)
 {
	...
	T1 *drv = arg;
	...
 }

@ found_completion depends on found_callback @
identifier find_request_async.drv_callback;
type found_callback.T1;
T1 *drv;
identifier cmpl;
@@

 drv_callback(...)
 {
	<+...
(
-	complete(&drv->cmpl);
|
-	complete_all(&drv->cmpl);
)
	...+>
 }

@ drop_init_completion @
identifier find_request_async.drv;
identifier found_completion.cmpl;
@@

-init_completion(&drv->cmpl);

@ replace_completion_wait @
identifier find_request_async.drv;
identifier found_completion.cmpl;
@@

-wait_for_completion(&drv->cmpl);
+sysdata_synchronize_request(drv->sysdata_async_cookie);

@ async_cookie exists @
typedef async_cookie_t;
type T;
@@

T {
	...
	async_cookie_t sysdata_async_cookie;
	...
};

@ modify_drv depends on !async_cookie @
type async_get_t.T;
identifier found_completion.cmpl;
@@

T {
	...
-	struct completion cmpl;
+	async_cookie_t sysdata_async_cookie;
	...
};

@ modify_drv2 depends on !modify_drv @
type found_callback.T1;
identifier found_completion.cmpl;
@@

T1 {
	...
-	struct completion cmpl;
+	async_cookie_t sysdata_async_cookie;
	...
};

@ modify_drv3 depends on !(modify_drv || modify_drv2)@
type add_desc_async.T;
@@

T {
	...
+	async_cookie_t sysdata_async_cookie;
};

@ modify_drv_direct depends on !(modify_drv || modify_drv2 || modify_drv3) @
type add_desc_direct_async.T;
@@

T {
	...
+	async_cookie_t sysdata_async_cookie;
};

// common cleanup: we can't include files at the end of a cocci file yet
// so we make this highly dependent on these set of rules.
// Despite the care some of these changes may still be too aggressive
// given we do not ensure the const struct firmware *data was the one
// used on the start rule

@ use_new_struct depends on add_desc_async || add_desc_direct_async @
identifier consumer, data;
@@

consumer(...,
-	const struct firmware *data
+	const struct sysdata_file *data
	,...)
{
...
}

@ modify_decl depends on add_desc_async || add_desc_direct_async @
type T;
identifier consumer, data;
@@

T consumer(...,
-	const struct firmware *data
+	const struct sysdata_file *data
	,...);

@ replace_struct_on_types depends on add_desc_async || add_desc_direct_async @
type T;
identifier data;
@@

T {
	...
-	const struct firmware *data;
+	const struct sysdata_file *data;
	...
};

@ drop_fw_release_goto depends on add_desc_async || add_desc_direct_async @
identifier out, some_fn;
@@

void some_fn (...) {
<+...
- goto out;
+ return;
...+>
-out:
-release_firmware(...);
}

@ drop_fw_release_fn depends on add_desc_async || add_desc_direct_async @
identifier fn;
@@

-fn (...) {
-release_firmware(...);
-}

@ drop_fw_release_fn_uses depends on add_desc_async || add_desc_direct_async @
identifier drop_fw_release_fn.fn;
@@

-fn(...);

@ drop_fw_release_branch depends on add_desc_async || add_desc_direct_async @
@@

-if (...)
-release_firmware(...);

@ drop_fw_release depends on add_desc_async || add_desc_direct_async @
@@

-release_firmware(...);

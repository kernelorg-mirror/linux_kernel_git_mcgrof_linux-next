// You can use this to help convert a device driver from the old firmware
// request_firmware*() API the new flexible sysdata API, this covers the
// generic conversions for both sync and async cases.
//
// Confidence: High
//
// Copyright: (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org> GPLv2.
// Copyright: (C) 2016 Julia Lawall, INRIA/LIP6.  GPLv2
//
// Options: --include-headers --in-place --no-show-diff
// Requires: 1.0.5

virtual patch

@ uses_fw_api @
@@

(
request_firmware(...)
|
request_firmware_nowait(...)
)

@ uses_sysdata @
@@

(
sysdata_file_request(...)
|
sysdata_file_request_async(...)
)

@ replace_header depends on uses_sysdata && !uses_fw_api @
@@

-#include <linux/firmware.h>
+#include <linux/sysdata.h>

@ add_header depends on uses_sysdata && uses_fw_api @
@@

#include <linux/firmware.h>
+#include <linux/sysdata.h>

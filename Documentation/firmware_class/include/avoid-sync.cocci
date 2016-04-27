@ fw_sync_returns_int @
identifier f;
position p;
@@

int f@p (...) {
<+...
request_firmware(...)
...+>
}

@ fw_sync_returns_non_int depends on !fw_sync_returns_int @
identifier f;
position p;
@@

f@p (...) {
<+...
request_firmware(...)
...+>
}

@ fw_sync_more_calls depends on fw_sync_returns_int @
identifier fw_sync_returns_int.f;
position p;
@@

f@p (...) {
...
request_firmware(...)
...
request_firmware(...)
...
}

@ sync_request_direct exists @
identifier fw;
local idexpression local_fw;
position p1;
@@

(
request_firmware@p1(&local_fw, ...)
|
request_firmware@p1((const struct firmware **) &fw, ...)
)

@ sync_request_non_direct exists @
expression E;
position p2 != sync_request_direct.p1;
@@

request_firmware@p2(E, ...)


@script:python depends on fw_sync_returns_non_int @
@@

cocci.exit()

@script:python depends on fw_sync_more_calls @
@@

cocci.exit()

@script:python depends on sync_request_non_direct @
@@

cocci.exit()

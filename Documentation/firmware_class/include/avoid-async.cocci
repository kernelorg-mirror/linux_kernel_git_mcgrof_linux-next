@ fw_async_returns_int @
identifier f;
position p;
@@

int f@p (...) {
<+...
request_firmware_nowait(...)
...+>
}

@ fw_async_returns_non_int depends on !fw_async_returns_int @
identifier f;
position p;
@@

f@p (...) {
<+...
request_firmware_nowait(...)
...+>
}

@ fw_async_more_calls depends on fw_async_returns_int @
identifier fw_async_returns_int.f;
position p;
@@

f@p (...) {
...
request_firmware_nowait(...)
...
request_firmware_nowait(...)
...
}

@script:python depends on fw_async_returns_non_int @
@@

cocci.exit()

@script:python depends on fw_async_more_calls @
@@

cocci.exit()

// Ingores affecting header declaration and implementation.
//
// Confidence: High
//
// Copyright: (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org> GPLv2.

@ decl_found_head @
position p0;
@@

int request_firmware@p0(...);

@ decl_found_code @
position p1;
@@

int request_firmware@p1(...) { ... }

@ ihex_code_found @
position p2;
@@
int request_ihex_firmware@p2(...) { ... }

@ uses_fw_on_struct @
type T;
identifier data;
@@

T {
	...
	const struct firmware *data;
	...
};

@script:python depends on decl_found_head @
@@
cocci.exit()

@script:python depends on decl_found_code @
@@
cocci.exit()

@script:python depends on ihex_code_found @
@@
cocci.exit()

@script:python depends on uses_fw_on_struct @
@@

cocci.exit()

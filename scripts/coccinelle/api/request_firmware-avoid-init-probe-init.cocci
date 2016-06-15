///
/// Thou shalt not request firmware on init or probe
///
// Confidence: High
// Copyright: (C) 2016 Luis R. Rodriguez <mcgrof@kernel.org>
// Copyright: (C) 2015 Julia Lawall, Inria/LIP6.
//
// GPLv2.
// URL: http://coccinelle.lip6.fr/
// Options: --no-includes --include-headers --no-show-diff
// Requires: 1.0.5
//
// Coccinelle 1.0.5 is required given that this uses the shiny new
// python iteration feature.

virtual report
virtual after_start

// -------------------------------------------------------------------------

@initialize:python@
@@

seen = set()

def add_if_not_present(f, file, starter_var):
    if (f, file, starter_var) not in seen:
        seen.add((f, file, starter_var))
        it = Iteration()
        if file != None:
            it.set_files([file])
    it.add_virtual_rule(after_start)
    it.add_virtual_rule(report)
    it.add_virtual_identifier(fn, f)
    it.add_virtual_identifier(starter, starter_var)
    it.register()

reported = set()

def print_err(str, file, line):
    if (file, line) not in reported:
        reported.add((file, line))
	print "%s: ERROR: driver call request firmware call on its %s routine on line %d." % (file, str, line)

@ defines_module_init@
declarer name module_init;
identifier init;
@@

module_init(init);

@ has_probe depends on defines_module_init@
identifier drv_calls, drv_probe;
type bus_driver;
identifier probe_op =~ "(probe)";
@@

bus_driver drv_calls = {
	.probe_op = drv_probe,
};

@hascall depends on !after_start && defines_module_init@
position p;
@@

(
request_firmware@p(...)
|
request_firmware_nowait@p(...)
|
request_firmware_direct@p(...)
)

@script:python@
init << defines_module_init.init;
p << hascall.p;
@@

if p[0].current_element == init:
    print_err("init", p[0].file, int(p[0].line))
    cocci.include_match(False)

@script:python@
drv_probe << has_probe.drv_probe;
p << hascall.p;
@@

if p[0].current_element == drv_probe:
    print_err("probe", p[0].file, int(p[0].line))
    cocci.include_match(False)

@script:python@
p << hascall.p;
@@

add_if_not_present(p[0].current_element, p[0].file, p[0].line)

@hasrecall depends on after_start@
position p;
identifier virtual.fn;
@@

fn@p(...)

@script:python@
init << defines_module_init.init;
p << hasrecall.p;
starter << virtual.starter;
@@

if p[0].current_element == init:
    print_err("init", p[0].file, int(starter))
    cocci.include_match(False)

@script:python@
drv_probe << has_probe.drv_probe;
p << hasrecall.p;
starter << virtual.starter;
@@

if p[0].current_element == drv_probe:
    print_err("probe", p[0].file, int(starter))
    cocci.include_match(False)

@script:python@
p << hasrecall.p;
starter << virtual.starter;
@@

add_if_not_present(p[0].current_element, p[0].file, starter)

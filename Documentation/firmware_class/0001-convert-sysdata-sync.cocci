// Copyright: (C) 2016 Julia Lawall, Inria/LIP6.  GPLv2
//
// Options: --allow-inconsistent-paths --in-place  --force-diff -D index="" -D sysdata=sysdata
// Requires: 1.0.5
//
// Identify the functions that we can treat.
// Request_firmware must be called in the main execution path, not under an if
// Unless all if branches call request_firmware
// This is enforced by the ...s and their lack of when any annotation
// This requires the first argument of request_firmware to have the form &e
// There are a couple of calls in the kernel where this property does not hold

virtual patch

// identifies as function as a combination of its name and parameter list,
// in hopes that this is unique

@initialize:ocaml@
@@

let redo index =
  let it = new iteration() in
  let file = List.hd (Coccilib.files()) in
  it#set_files [file];
  let index = if index = "" then 1 else (1+(int_of_string index)) in
  it#add_virtual_identifier Index (string_of_int index);
  it#add_virtual_identifier Sysdata (Printf.sprintf "sysdata%d" index);
  it#register()

@exists@
expression x;
@@

request_firmware(...)
<...
- return
+ SRETURN_(
x
+)
 ;
...>

@inverted@
position p;
@@

request_firmware@p(...) == 0

@start@
identifier f, ret, ret1;
local idexpression fw;
expression name, dev;
fresh identifier sync_found_cb = f ## "_found_cb";
fresh identifier reqtype = f ## "_req";
fresh identifier sreq = "sreq" ## virtual.index;
fresh identifier context = "context" ## virtual.index;
statement S;
position p1 != inverted.p;
parameter list ps;
@@

int f(ps) {
+struct reqtype { void *nothing; };
+struct reqtype sreq;
+const struct sysdata_file_desc sysdata_desc = {
+	SYSDATA_DEFAULT_SYNC(sync_found_cb, &sreq),
+};
... when != request_firmware(...)
(
- ret = request_firmware@p1(&fw, name, dev);
+ ret = sysdata_file_request(name, &sysdata_desc, dev);
if (<+...ret...+>) S
+ if(__true__) {
+ struct reqtype *sreq = context;
  ... when any
+ return ret1;
+}
SRETURN_(
- ret1
+ ret
  );
|
if (<+...
-        request_firmware@p1(&fw, name, dev)
+        sysdata_file_request(name, &sysdata_desc, dev)
    ...+>) S
+ if(__true__) {
+ struct reqtype *sreq = context;
  ... when any
      when != request_firmware(...)
+ return ret1;
+}
SRETURN_(
- ret1
+ 0
  );
)
}

@redoable depends on start@
@@
request_firmware(...)

@script:ocaml depends on redoable@
index << virtual.index;
@@
redo index

// Rename firmware structure and type
@exists@
identifier start.f,virtual.sysdata;
local idexpression start.fw;
symbol __true__;
parameter list start.ps;
@@

f (ps) {
<...
 if(__true__) {
<...
- fw
+ sysdata
...>
}
...>
}

@@
identifier start.f, i;
parameter list start.ps;
@@

f (ps) {
<...
  struct
- firmware
+ sysdata_file
  i;
...>
}

@depends on start@
identifier l,l1;
@@

if (...) {
   ...
   goto l;
}
... when != if (...) { ... goto l1; }
if(__true__) {
  ...
+ if (__false__) {
l:
...
+}
}

// Propagate renames in hopes of reducing the number of variables that
// have to be stored in the context structure
// Needs to come first because the propagated thing is not a decl, so decls
// have to be added before it.
@depends on start@
identifier b, start.reqtype, start.sreq, context;
expression x,e1,e2,a;
@@

x = a->b;
...  when != x = e1
     when != a = e2
if(__true__) {
    struct reqtype *sreq = context;
++  x = a->b;
    ... when exists
    x
    ... when any
}

// Push downwards unused initializers as well as calls on parameter (risky)
// requires if on ehc

@@
identifier start.f, x, start.reqtype, start.sreq, context;
constant C;
type T;
symbol __false__;
parameter list start.ps;
@@

f (ps) {
 ... when any
     when strict
-T x = C;
 ... when != x
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  T x = C;
    <+...x...+>
    if (__false__) { ... when != x
    }
 }
... when != x
    when any
}

@@ // structs are like constants
identifier start.f, x, i, start.reqtype, start.sreq, context;
symbol __false__;
parameter list start.ps;
@@

f (ps) {
 ... when any
     when strict
-struct i x;
 ... when != x
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  struct i x;
    <+...x...+>
    if (__false__) { ... when != x
    }
 }
... when != x
    when any
}

@@
identifier start.f, h, x, i, j, start.reqtype, start.sreq, context;
expression e,e1;
type T,T1;
@@

f (...,T1 i,...) {
 ... when any
     when strict
-T x = \(i->@e j\|h(i)@e\);
 ... when != x
     when != i = e1
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  T x = e;
    <+...x...+>
    if (__false__) { ... when != x
    }
 }
... when != x
    when any
}

@nofalse@
identifier start.f;
parameter list start.ps;
statement S;
@@

f(ps) {
  ... when != if (__false__) S
}

@depends on nofalse@
identifier start.f, x, start.reqtype, start.sreq, context;
constant C;
type T;
parameter list start.ps;
@@

f (ps) {
 ... when any
     when strict
-T x = C;
 ... when != x
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  T x = C;
    ... when exists
    x
    ... when any
 }
... when != x
    when any
}

@depends on nofalse@ // structs are like constants
identifier start.f, x, i != start.reqtype, start.reqtype, start.sreq, context;
parameter list start.ps;
@@

f (ps) {
 ... when any
     when strict
-struct i x;
 ... when != x
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  struct i x;
    ... when exists
    x
    ... when any
 }
... when != x
    when any
}

@depends on nofalse@
identifier start.f, h, x, i, j, start.reqtype, start.sreq, context;
expression e,e1;
type T,T1;
@@

f (...,T1 i,...) {
 ... when any
     when strict
-T x = \(i->@e j\|h(i)@e\);
 ... when != x
     when != i = e1
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  T x = e;
    ... when exists
    x
    ... when any
 }
... when != x
    when any
}

// Initialize return vaue if needed

@updret depends on start exists@
identifier ret,r;
type T;
expression e1,e2;
statement S;
position p;
@@

T ret;
...
ret = sysdata_file_request(...);
if (...) S
if@p(__true__) {
  ... when != ret = e1
      when != T ret;
(
  ret = e2
|
  ret@r
)
  ... when any
}

@depends on start exists@
identifier updret.r, start.reqtype, start.sreq, context;
type updret.T;
expression e1;
position updret.p;
@@

if@p(__true__) {
  struct reqtype *sreq = context;
++ T r = 0;
  ... when != r = e1
      when != T r;
  r
  ... when any
}


// Duplicates decl
@@
identifier start.f, x, i, start.reqtype, start.sreq, context;
constant C;
type T,T1;
expression e;
@@

f (...,T1 i,...) {
 ... when any
T x = C;
 ... when != x = e
     when any
 if(__true__) {
    struct reqtype *sreq = context;
++  T x = C;
    ... when exists
    x
    ... when any
 }
... when any
}

// Remove if on ehc

@ehc depends on start exists@
statement list sl;
@@

if(__true__) {
  ... when any
- if (__false__) {
 sl
-}
 ... when any
}

@@
expression x;
@@

- SRETURN_(
+ return
  x
- )
  ;

// Try to construct the context: working on the written variables

@write1 depends on start exists@
position pa1,px1;
type T;
identifier i;
expression e;
@@

if (__true__) { ... when any
  T i@pa1@px1 = e;
  ... when any
}

@write2 depends on start exists@
type T;
T v;
identifier i;
expression e;
position pa2,px2;
@@

if (__true__) { ... when any
  v@i@pa2@px2 = e
  ... when any
}

@write3 depends on start exists@
type T;
T v;
identifier i,g;
position pa3,px3;
@@

if (__true__) { ... when any
  g(...,&v@i@pa3@px3,...)
  ... when any
}

@writen@
identifier i;
position any write1.pa1;
position any write2.pa2;
position any write3.pa3;
@@

(
i@pa1
|
i@pa2
|
i@pa3
)

// var is read on some path before it was written
@read depends on start exists@
local idexpression v;
identifier virtual.sysdata;
identifier writen.i;
expression e,e1;
position p != {write1.px1,write2.px2,write3.px3};
position any write1.pa1;
position any write2.pa2;
position any write3.pa3;
statement S;
type T;
@@

if (__true__) {
  ... when any
      when != T i@pa1 = e1;
      when != i@pa2
      when != i@pa3
(
  for (<+...i = e...+>; ...; ...) S
|
  sizeof(<+...i...+>) // not a reference
|
  sysdata
|
  v@i@p
)
  ... when any
}

// other occurrences of vars that were somewhere first read
@firstread depends on start exists@
local idexpression read.v;
identifier writen.i;
position p;
@@

if (__true__) {
  <...
  v@i@p
  ...>
}

@r depends on start exists@
type T;
local idexpression T v;
identifier writen.i,reqtype,start.f,start.sreq, context;
position p != firstread.p;
parameter list start.ps;
@@

f(ps) {
...
if (__true__) {
   struct reqtype *sreq = context;
  ... when != T i;
  v@i@p
  ... when any
}
... when any
}

@depends on start@
type r.T;
identifier writen.i,start.f,start.sreq, context;
identifier reqtype;
parameter list start.ps;
@@

f(ps) {
... when any
if (__true__) {
   struct reqtype *sreq = context;
++ T i;
  ...
}
... when any
}

// Try to construct the context: working on the read variables

@assignfor depends on start exists@
identifier i;
expression e;
position p,p1,p2;
statement S;
@@

if (__true__) { ... when any
  for(<+...i@p = e...+>; <+...i@p1...+>; <+...i@p2...+>) S
  ... when any
}

@assignaddr depends on start exists@
identifier i,g;
position p;
@@

if (__true__) { ... when any
  g(...,&i@p,...)
  ... when any
}

// The following overlaps with assignaddr, so has to be a separate rule
@assign depends on start exists@
identifier i;
expression e;
position p;
type T;
@@

if (__true__) { ... when any
(
  T i@p = e;
|
  T i@p;
|
  i@p = e
)
  ... when any
}

@doesread depends on start exists@
position p != {assignfor.p,assignfor.p1,assignfor.p2,assignaddr.p,assign.p};
identifier i,reqtype,start.f,start.sreq,context;
type T,T1;
local idexpression T v;
parameter list start.ps;
@@

f(ps) {
...
if (__true__) {
  struct reqtype *sreq = context;
  ... when any
      when != i // matches exp
      when != T1 i;
(
  sizeof(<+...v...+>)
|
  v@i@p
)
  ... when any
}
... when any
}

@cc depends on start exists@
type doesread.T,T1;
identifier doesread.i,start.f,start.sreq,context;
identifier reqtype, ret;
statement S;
parameter list start.ps;
@@

f(ps) {
struct reqtype {
- void *nothing;
  ...
++ T i;
};
... when any
(
++sreq.i = i;
ret = sysdata_file_request(...);
if (<+...ret...+>) S
|
++sreq.i = i;
if (<+...sysdata_file_request(...)...+>) S
)
if (__true__) {
  struct reqtype *sreq = context;
++ T i = sreq->i;
  ... when any
      when != i // matches exp
      when != T1 i;
  i
  ... when any
}
  ...  when any
}

// Convert arrays to pointers
@@
identifier cc.reqtype;
type T;
identifier i;
@@

struct reqtype {
  ...
  T
+ *
  i
- [...]
  ;
  ...
};

// Hack to deal with array types and with a weakness in the pretty printer,
// part 2.
@depends on start exists@
type T;
identifier i,start.sreq;
@@

if (__true__) { ... when any
  T
+ *
  i
- [...]
  = sreq->i;
  ... when any
}

@sz depends on start exists@ // vars only used in sizeof
type T1,T;
identifier reqtype, i, start.sreq, context;
local idexpression T v;
@@

if (__true__) {
  struct reqtype *sreq = context;
++ T i;
  ... when != T1 i;
  sizeof(v@i)
  ... when any
}

// Drop the context structure if it contains only one element

@dropstructparam exists@
identifier start.f,start.reqtype,x,start.sreq,context;
type T;
expression e;
symbol sysdata_desc;
@@

f(...,T x,...) {
-struct reqtype { T x; };
-struct reqtype sreq;
const struct sysdata_file_desc sysdata_desc = {
	SYSDATA_DEFAULT_SYNC(e,
-       &sreq
+       (void *)x
        ),
};
...
-sreq.x = x;
...
if (__true__) {
-  struct reqtype *sreq = context;
-  T x = sreq->x;
+  T x = context;
   ...
}
... when any
}

@dropstructlocal exists@
identifier start.f,start.reqtype,x,start.sreq,context;
statement S;
expression e;
type T;
parameter list start.ps;
@@

f(ps) {
-struct reqtype { T x; };
-struct reqtype sreq;
-const struct sysdata_file_desc sysdata_desc = {
-	SYSDATA_DEFAULT_SYNC(e, &sreq),
-};
... when != S
T x = ...;
+const struct sysdata_file_desc sysdata_desc = {
+	SYSDATA_DEFAULT_SYNC(e, (void *)x),
+};
...
-sreq.x = x;
...
if (__true__) {
-  struct reqtype *sreq = context;
-  T x = sreq->x;
+  T x = context;
   ...
}
... when any
}

@dropstruct exists@
identifier start.f,start.reqtype,start.sreq,context;
expression e;
parameter list start.ps;
@@

f(ps) {
-struct reqtype { void *nothing; };
-struct reqtype sreq;
const struct sysdata_file_desc sysdata_desc = {
	SYSDATA_DEFAULT_SYNC(e,
- &sreq
+ NULL
  ),
};
...
if (__true__) {
-  struct reqtype *sreq = context;
   ...
}
... when any
}

// Move the context structure declaration out to top level

@depends on !dropstructparam && !dropstructlocal && !dropstruct@
identifier start.f;
type T;
parameter list start.ps;
@@

+ T;

f(ps) {
- T;
  ...
}

// Construct the callback function

// The conjunction ( & ) is for efficiency: the first case is easy to
// remove, while the second allows transporting the code without the braces.
@ add_cb_sync1 depends on ehc@
identifier start.f,virtual.sysdata;
fresh identifier sync_found_cb = f ## "_found_cb";
fresh identifier context = "context" ## virtual.index;
statement S;
statement list S1, ehc.sl;
expression ret;
parameter list start.ps;
@@

+static int sync_found_cb(void *context, const struct sysdata_file *sysdata)
+{
+ S1
+}

f (ps) {
... when any
(
- if (__true__) S
&
if (__true__) { S1 }
)
-return ret;
+sl
}

@@
expression x;
@@

- SRETURN_(x);
+ return x;

@ add_cb_sync2 depends on !ehc @
identifier start.f,virtual.sysdata;
fresh identifier sync_found_cb = f ## "_found_cb";
fresh identifier context = "context" ## virtual.index;
statement S;
statement list S1;
expression ret;
parameter list start.ps;
@@

+static int sync_found_cb(void *context, const struct sysdata_file *sysdata)
+{
+ S1
+}

f (ps) {
... when any
(
- if (__true__) S
&
if (__true__) { S1 }
)
return ret;
}

// Check for a single retry
// Not safe.  We just hope that the fw that is consistent with f, name, and dev
// is the right one...

@@
identifier start.f,l,ret;
expression name, dev, ret1, name1, dev1;
local idexpression start.fw;
statement S;
parameter list start.ps;
@@

f(ps) { <...
(
ret = sysdata_file_request(name, &sysdata_desc, dev);
if (...) {
  ... when != goto l;
(
- ret1 = request_firmware(&fw, name1, dev1);
+ ret1 = sysdata_file_request(name1, &sysdata_desc, dev);
if (<+...ret1...+>) S
|
if (<+...
-        request_firmware(&fw, name1, dev1)
+        sysdata_file_request(name1, &sysdata_desc, dev)
    ...+>) S
)
}
|
if (<+...sysdata_file_request(name, &sysdata_desc, dev)...+>) {
  ... when != goto l;
(
- ret1 = request_firmware(&fw, name1, dev1);
+ ret1 = sysdata_file_request(name1, &sysdata_desc, dev);
if (<+...ret1...+>) S
|
if (<+...
-        request_firmware(&fw, name1, dev1)
+        sysdata_file_request(name1, &sysdata_desc, dev)
    ...+>) S
)
}
)
...> }

// drop trivial labels

@@
identifier start.f,out;
local idexpression x;
parameter list start.ps;
@@

f(ps) {
... when != goto out;
- x =
+ return
   sysdata_file_request(...);
-if (<+...x...+>) goto out;
-out:
-return x;
}

@@
identifier start.f,out;
local idexpression x;
parameter list start.ps;
statement S, S1;
@@

f(ps) {
...
 x =
   sysdata_file_request(...);
(
 if (<+...x...+>)
 {
  if (...) S else S1
- goto out;
 }
|
 if (<+...x...+>)
- {
  S
- goto out;
- }
)
out:
...
}

// drop unused labels

@l1 exists@
identifier f = {start.sync_found_cb,start.f};
identifier l;
position p,p1;
@@

f@p1(...) {
  <... when any
  l@p:
  ...>
}

@l2@
identifier l1.f, l1.l;
position l1.p1;
@@

f@p1(...) {
  ... when != goto l;
      when strict
}

@l3 depends on l2@
identifier l1.f, l1.l;
position l1.p,l1.p1;
@@

f@p1(...) {
  <...
- l@p:
  ...>
}

@@
identifier start.f;
local idexpression x;
parameter list start.ps;
@@

f(ps) {
...
- x =
+ return
   sysdata_file_request(...);
-if (<+...x...+>) return x;
-return x;
...
}

@@
identifier start.f;
local idexpression x;
statement S,S1;
parameter list start.ps;
@@

f(ps) {
...
 x =
   sysdata_file_request(...);
(
if (...) { if (...) S else S1
- return x;
 }
|
if (<+...x...+>)
- {
  S
- return x;}
)
return x;
}

// drop any now (or previously...) unused variables
@decld exists@
identifier start.f;
type T;
identifier x;
constant C;
parameter list start.ps;
position p;
@@

f(ps) {
  ... when any
      when strict
(
 T x@p = C;
|
 T x@p;
)
  ... when any
}

@unused@
identifier start.f;
identifier decld.x;
parameter list start.ps;
@@

f(ps) {
  ... when != x
}

@depends on unused@
position decld.p;
type T;
identifier x;
constant C;
@@

(
- T x@p = C;
|
- T x@p;
)

// common cleanup: we can't include files at the end of a cocci file yet
// so we make this highly dependent on these set of rules.
// Despite the care some of these changes may still be too aggressive
// given we do not ensure the const struct firmware *data was the one
// used on the start rule

@ use_new_struct depends on start @
identifier consumer, data;
@@

consumer(...,
-	const struct firmware *data
+	const struct sysdata_file *data
	,...)
{
...
}

@ modify_decl depends on start @
type T;
identifier consumer, data;
@@

T consumer(...,
-	const struct firmware *data
+	const struct sysdata_file *data
	,...);

@ replace_struct_on_types depends on start @
type T;
identifier data;
@@

T {
	...
-	const struct firmware *data;
+	const struct sysdata_file *data;
	...
};

@ drop_fw_release_goto depends on start @
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

@ drop_fw_release_fn depends on start @
identifier fn;
@@

-fn (...) {
-release_firmware(...);
-}

@ drop_fw_release_fn_uses depends on start @
identifier drop_fw_release_fn.fn;
@@

-fn(...);

@ drop_fw_release_branch depends on start @
@@

-if (...)
-release_firmware(...);

@ drop_fw_release depends on start @
@@

-release_firmware(...);

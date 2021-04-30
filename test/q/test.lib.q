/# vim: set et noai ts=2 sw=2 syntax=q:
/////////////////////////////////////////////////////////////////////////////
\d .util

lib:{ `$$[.z.o like"w*";"";"lib"],$[-11h=type x;string x;x] }

/////////////////////////////////////////////////////////////////////////////
\d .

DLL:.util.lib`q_ffi@dll_suffix@
VERSION:(`version;1)
LOADER:(`load_fun;5)
GETTER:(`get_var;3)
SETTER:(`set_var;3)
SIZE_T:(`size_type;1)
PTR_T:(`ptr_type;1)
ADDR_OF:(`addr_of;1)
ADDR_GETTER:(`get_from_addr;2)
ADDR_SETTER:(`set_to_addr;2)
FREE_ADDR:(`free_addr;1)

TEST_DLL:.util.lib`$"@test_dll@"

/////////////////////////////////////////////////////////////////////////////
\d .test

.test.log:{ -2"# ",$[10h=type x;x;.Q.s1 x]; }
indent:{ "\n"sv"\t",/:"\n"vs x }

assert:{[cond;msg] if[not cond;'msg]; }
assert_eq:{[x;y] assert[    x~y;"(",.Q.s1[x],") != (",.Q.s1[y],")"] }
assert_ne:{[x;y] assert[not x~y;"(",.Q.s1[x],") == (",.Q.s1[y],")"] }

/////////////////////////////////////////////////////////////////////////////
\d .mem

sample:.Q.w

delta:{[m0;m1] where[0<>d]#d:m1-m0 }

verify:{[m0;m1]
  d:delta[m0;m1];
  if[.z.o like"w64";  /FIXME: w64 "used" memory always increases after 2:
    :.test.log"\n",.Q.s enlist d ];
  .test.assert[not count d;
    "unexpected memory overhead:\n",.test.indent .Q.s enlist d ]
 }

/////////////////////////////////////////////////////////////////////////////
\d .

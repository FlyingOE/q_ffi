if[not system"s";
  '"Start ",string[.z.f]," with -s!" ];

/////////////////////////////////////////////////////////////////////////////
\d .util

lib:{ `$$[.z.o like"w*";"";"lib"],$[-11h=type x;string x;x] }

/////////////////////////////////////////////////////////////////////////////
\d .

DLL:.util.lib`q_ffi
LOADER:`load

TEST_DLL:.util.lib`test_q_ffi_dll
if[4>count .z.x; show .z.x;
  '"q test_ffcall.q -q -s 4 <func> <abi> <retType> <argTypes>" ]
TEST_FUNC:`$.z.x 0
ABI:`$.z.x 1
RET_TYPE:first .z.x 2
ARG_TYPES:.z.x 3

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
  .test.assert[not count d;
    "unexpected memory overhead:\n",.test.indent .Q.s d:delta[m0;m1] ]
 }

/////////////////////////////////////////////////////////////////////////////
\d .

mem:{.mem.sample[]}each til 10
mid:-1
mem_check:{[check] check . mem mid+-1 0 }
hr:40

.test.log hr#"-";
loader:0N!DLL 2:(LOADER;5)
.test.log"After loading q_ffi.dll";

.test.log hr#"-";
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;RET_TYPE;ARG_TYPES]
mem[mid+:1]:.mem.sample[]
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;RET_TYPE;ARG_TYPES]
mem[mid+:1]:.mem.sample[]
.test.log"After loading 3rd-party DLL (*2)":
mem_check .mem.verify;

.test.log hr#"-";
TESTS:{x$y%10}[RET_TYPE;](0N;count ARG_TYPES)#(100*count ARG_TYPES)?RET_TYPE$1e300
mem[mid :0]:.mem.sample[]
results:tester ./:TESTS
mem[mid+:1]:.mem.sample[]
.test.log"\n",.Q.s mem_check .mem.delta;

mem[mid+:1]:.mem.sample[]
results:tester ./:TESTS
mem[mid+:1]:.mem.sample[]
.test.log"After running tests (*2)"
mem_check .mem.verify;

.test.log hr#"=";
results:.[tester;]peach TESTS
mem[mid+:1]:.mem.sample[]
.test.log"\n",.Q.s mem_check .mem.delta;
if[RET_TYPE in"ijef";
  expected:RET_TYPE$(+). flip TESTS;
  if[any not ok:results=expected;
    errs:{ "\t",.Q.s1(TESTS;results;expected)@\:x }each where not ok;
    '"incorrect result(s):\n","\n"sv errs ]
 ]

.test.log"All tests completed"
\\
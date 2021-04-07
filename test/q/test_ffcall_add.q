/# vim: set et noai ts=2 sw=2 syntax=q:
/////////////////////////////////////////////////////////////////////////////
system"l ",string .Q.dd[first` vs hsym .z.f;`test.lib.q]

if[not system"s"; '"Start ",string[.z.f]," with -s!" ];

if[4>count .z.x; show .z.x;
  '"q ",string[.z.f]," -q -s 4 <func> <abi> <retType> <argTypes>" ]
TEST_FUNC:`$.z.x 0
ABI:`$.z.x 1
RET_TYPE:first .z.x 2
ARG_TYPES:.z.x 3

/////////////////////////////////////////////////////////////////////////////
mem:{.mem.sample[]}each til 10
mid:-1
mem_check:{[check] check . mem mid+-1 0 }
hr:40

/////////////////////////////////////////////////////////////////////////////
.test.log hr#"-";
loader:0N!DLL 2:(LOADER;5)
.test.log"After loading DLL ",.Q.s1[DLL];

.test.log hr#"-";
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;RET_TYPE;ARG_TYPES]
mem[mid+:1]:.mem.sample[]
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;RET_TYPE;ARG_TYPES]
mem[mid+:1]:.mem.sample[]
.test.log"After loading DLL ",.Q.s1[TEST_DLL]," (*2)":
mem_check .mem.verify;

.test.log hr#"-";
TESTS:{x$y%10}[RET_TYPE;](0N;count ARG_TYPES)#(100*count ARG_TYPES)?RET_TYPE$1e30
mem[mid :0]:.mem.sample[]
results:tester ./:TESTS
mem[mid+:1]:.mem.sample[]
.test.log"\n",.Q.s enlist mem_check .mem.delta;

mem[mid+:1]:.mem.sample[]
results:tester ./:TESTS
mem[mid+:1]:.mem.sample[]
.test.log"After running tests (*2)"
mem_check .mem.verify;

.test.log hr#"=";
results:.[tester;]peach TESTS
mem[mid+:1]:.mem.sample[]
.test.log"\n",.Q.s enlist mem_check .mem.delta;

simulate:$[ // Truncate values to reverse q's auto type promotion
  "x"~RET_TYPE;(last 0x0 vs(+).)each;
  "h"~RET_TYPE;(0x0 sv -2#0x0 vs(+).)each;
  "i"~RET_TYPE;(0x0 sv -4#0x0 vs(+).)each;
  "j"~RET_TYPE;(+)./:;
  "e"~RET_TYPE;`real$(+)./:;
  "f"~RET_TYPE;(+)./:;
    '"nyi: add support for type \"",RET_TYPE,"\"" ]
expected:simulate TESTS;
if[any not ok:results=expected;
  errs:{ "\t",.Q.s1(TESTS;results;expected)@\:x }each where not ok;
  '"incorrect result(s):\n","\n"sv errs ]

.test.log"All tests completed";
\\
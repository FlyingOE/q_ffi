/# vim: set et noai ts=2 sw=2 syntax=q:
/////////////////////////////////////////////////////////////////////////////
system"l ",string .Q.dd[first` vs hsym .z.f;`test.lib.q]

if[not system"s"; '"Start ",string[.z.f]," with -s!" ];

if[3>count .z.x; show .z.x;
  '"q ",string[.z.f]," -q -s 4 <func> <abi> <kType>" ]
TEST_FUNC:`$.z.x 0
if[not TEST_FUNC like"*add_*";
  '"invalid test function: ",.Q.s1 TEST_FUNC ]
ABI:`$.z.x 1
TYPE:first .z.x 2

/////////////////////////////////////////////////////////////////////////////
mem:{.mem.sample[]}each til 10
mid:-1
mem_check:{[check] check . mem mid+-1 0 }
hr:40

/////////////////////////////////////////////////////////////////////////////
.test.log hr#"-";
loader:0N!DLL 2:LOADER
.test.log"After loading DLL ",.Q.s1[DLL];

.test.log hr#"-";
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;TYPE;2#TYPE]
mem[mid+:1]:.mem.sample[]
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;TYPE;2#TYPE]
mem[mid+:1]:.mem.sample[]
.test.log"After loading DLL ",.Q.s1[TEST_DLL]," (*2)":
mem_check .mem.verify;

.test.log hr#"-";
N:100
TESTS:{x$y%10}[TYPE;] 0N 2#(N*2)?TYPE$1e30
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
  "x"~TYPE;(last 0x0 vs(+).)each;
  "h"~TYPE;(0x0 sv -2#0x0 vs(+).)each;
  "i"~TYPE;(0x0 sv -4#0x0 vs(+).)each;
  "j"~TYPE;(+)./:;
  "e"~TYPE;`real$(+)./:;
  "f"~TYPE;(+)./:;
    '"nyi: add test support for type \"",TYPE,"\"" ]
expected:simulate TESTS;
if[any not ok:results=expected;
  errs:{ "\t",.Q.s1(TESTS;results;expected)@\:x }each where not ok;
  '"incorrect result(s):\n","\n"sv errs ]

.test.log"All tests completed";
\\
if[not system"s";
  '"Start ",string[.z.f]," with -s!" ];

/////////////////////////////////////////////////////////////////////////////
delta_memory:{[mem0;mem1]
  where[0<>delta]#delta:mem1-mem0
 };

verify_memory:{[mem0;mem1]
  prob:delta_memory[mem0;mem1];
  if[count prob;
    '"unexpected memory overhead:\n",.Q.s prob ]
 };

/////////////////////////////////////////////////////////////////////////////
DLL:`q_ffi
LOADER:`load

TEST_DLL:`test_q_ffi_dll
TEST_FUNC:`add_int32_t_cdecl
ABI:`

mem:{.Q.w[]}each til 10

-1 enlist 40#"-";
loader:0N!DLL 2:(LOADER;5)
-1"# After loading q_ffi.dll";

-1 enlist 40#"-";
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;"i";"ii"]
mem[0]:.Q.w[]
tester:0N!loader[TEST_DLL;TEST_FUNC;ABI;"i";"ii"]
mem[1]:.Q.w[]
-1"# After loading 3rd-party dll (*2)";
verify_memory . mem 0 1

-1 enlist 40#"-";
TESTS:0N 2#1000?0Wi
mem[0]:.Q.w[]
results:tester ./:TESTS
mem[1]:.Q.w[]
show delta_memory . mem 0 1
results:tester ./:TESTS
mem[2]:.Q.w[]
-1"# After running tests (*2)";
verify_memory . mem 1 2

-1 enlist 40#"=";
results:{tester . x}peach TESTS
outcome:results=(+)./:TESTS
mem[3]:.Q.w[]
show delta_memory . mem 2 3
if[any not outcome;
  '"incorrect result(s) @ ",.Q.s1 where not outcome ]

\\
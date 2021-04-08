/# vim: set et noai ts=2 sw=2 syntax=q:
/////////////////////////////////////////////////////////////////////////////
system"l ",string .Q.dd[first` vs hsym .z.f;`test.lib.q]

if[not system"s"; '"Start ",string[.z.f]," with -s!" ];

/////////////////////////////////////////////////////////////////////////////
mem:{.mem.sample[]}each til 10
mid:-1
mem_check:{[check] check . mem mid+-1 0 }

mangle:{[pref;func;post]
  $[.z.o in`w32`l32; pref,func,post;
    .z.o in`w64`l64; func;
      '"nyi: Implement name mangling rule for ",.Q.s1 .z.o ]
 }

/////////////////////////////////////////////////////////////////////////////
loader:0N!DLL 2:(LOADER;5)
.test.log"After loading DLL ",.Q.s1[DLL];

MAX_ARGC:8;
FUN:til[MAX_ARGC]!MAX_ARGC#enlist()!()

FUN[0;`]:0N!loader[TEST_DLL;`f0 ;` ;"i";""]
FUN[1;`]:0N!loader[TEST_DLL;"f1";::;"x";"x"]    / any string-like type can work
FUN[2;`]:0N!loader[TEST_DLL;"f2";` ;"x";"xj"]
FUN[3;`]:0N!loader[TEST_DLL;`f3 ;` ;"h";"xhj"]
FUN[4;`]:0N!loader[TEST_DLL;`f4 ;"";"h";"xihj"]
FUN[5;`]:0N!loader[TEST_DLL;`f5 ;` ;"h";`xfihj ]
FUN[6;`]:0N!loader[TEST_DLL;`f6 ;` ;"h";"exihjf"]
FUN[7;`]:0N!loader[TEST_DLL;`f7 ;` ;"i";"exihjfc"]
.test.log"After loading \"normal\" functions from DLL ",.Q.s1[TEST_DLL];

FUN[0;`std]:0N!loader[TEST_DLL;mangle["_";"f0_stdcall";"@0"] ;`stdcall;"i";""];
FUN[1;`std]:0N!loader[TEST_DLL;mangle["_";"f1_stdcall";"@4"] ;`stdcall;"x";"x"];
FUN[2;`std]:0N!loader[TEST_DLL;mangle["_";"f2_stdcall";"@12"];`stdcall;"x";"xj"];
FUN[3;`std]:0N!loader[TEST_DLL;mangle["_";"f3_stdcall";"@16"];`stdcall;"h";"xhj"];
FUN[4;`std]:0N!loader[TEST_DLL;mangle["_";"f4_stdcall";"@20"];`stdcall;"h";"xihj"];
FUN[5;`std]:0N!loader[TEST_DLL;mangle["_";"f5_stdcall";"@28"];`stdcall;"h";`xfihj ];
FUN[6;`std]:0N!loader[TEST_DLL;mangle["_";"f6_stdcall";"@32"];`stdcall;"h";"exihjf"];
FUN[7;`std]:0N!loader[TEST_DLL;mangle["_";"f7_stdcall";"@36"];`stdcall;"i";"exihjfc"];
.test.log"After loading \"stdcall\" functions from DLL ",.Q.s1[TEST_DLL];

FUN[0;`fast]:0N!loader[TEST_DLL;mangle["@";"f0_fastcall";"@0"] ;`fastcall;"i";""];
FUN[1;`fast]:0N!loader[TEST_DLL;mangle["@";"f1_fastcall";"@4"] ;`fastcall;"x";"x"];
FUN[2;`fast]:0N!loader[TEST_DLL;mangle["@";"f2_fastcall";"@12"];`fastcall;"x";"xj"];
FUN[3;`fast]:0N!loader[TEST_DLL;mangle["@";"f3_fastcall";"@16"];`fastcall;"h";"xhj"];
FUN[4;`fast]:0N!loader[TEST_DLL;mangle["@";"f4_fastcall";"@20"];`fastcall;"h";"xihj"];
FUN[5;`fast]:0N!loader[TEST_DLL;mangle["@";"f5_fastcall";"@28"];`fastcall;"h";`xfihj ];
FUN[6;`fast]:0N!loader[TEST_DLL;mangle["@";"f6_fastcall";"@32"];`fastcall;"h";"exihjf"];
FUN[7;`fast]:0N!loader[TEST_DLL;mangle["@";"f7_fastcall";"@36"];`fastcall;"i";"exihjfc"];
.test.log"After loading \"fastcall\" functions from DLL ",.Q.s1[TEST_DLL];

// For logics of f*[...], refer to ./dll/test_dll.cpp
test_log:{
  .test.log"Testing calling convention ",.Q.s1[y]," with ",string[x]," argument(s)"
 };
ABIs:``std`fast

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][  ];-1i];    / max<uint32_t> = -1i
  .test.assert_eq[FUN[x;y][`a];-1i];    / niladic functions can take (and ignore) 1 parameter
  .test.assert[.[FUN[x;y];(`a;"A");::]like"rank*";"'rank expected"];
 }/:[0;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y]["x"$"a"];"x"$"W"];
  .test.assert_eq[FUN[x;y][   0x61];"x"$"W"];   / anything integral is casted to target type
  .test.assert[.[FUN[x;y];(`a;"A");::]like"rank*";"'rank expected"];
 }/:[1;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][0x61;  0];"x"$"a"];
  .test.assert_eq[FUN[x;y][0x61; -1];0x61];
  .test.assert_eq[FUN[x;y][0x61;255];0x9E];
  .test.assert[.[FUN[x;y];(`a;"A";0);::]like"rank*";"'rank expected"];
  .test.assert_eq[type FUN[x;y][`a];104h];      / projection cannot detect parameter error
 }/:[2;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][0x61;  0h; 0];  "h"$"a"    ];
  .test.assert_eq[FUN[x;y][0x61; -1h;-1];0x0 sv 0xFF9E];
  .test.assert_eq[FUN[x;y][0x61;255h;-1];0x0 sv 0x009E];
  .test.assert_eq[FUN[x;y][0x61;255h;-2];0x0 sv 0x009F];
  .test.assert[.[FUN[x;y];(`a;"A";0;0);::]like"rank*";"'rank expected"];
  .test.assert_eq[type FUN[x;y][`a;"A"];104h];  / projection cannot detect parameter error
 }/:[3;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][0x61;  0i; 0h; 0]; "h"$"a"     ];
  .test.assert_eq[FUN[x;y][0x61; -1i;-1h;-1];0x0 sv 0xFF9E];
  .test.assert_eq[FUN[x;y][0x61;255i;-1h;-1];0x0 sv 0xFF61];
  .test.assert_eq[FUN[x;y][0x61;255i;-2h;-2];0x0 sv 0xFF61];
  .test.assert[.[FUN[x;y];(`a;"A";0;0;0);::]like"rank*";"'rank expected"];
  .test.assert_eq[type FUN[x;y][`a;"A";0];104h];
 }/:[4;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][0x61; 1. ;  0i; 0h; 0]; "h"$"a"     ];
  .test.assert_eq[FUN[x;y][0x61;  .5; -1i;-1h;-1];0x0 sv 0xFFCF];
  .test.assert_eq[FUN[x;y][0x61;-1. ;255i;-1h;-1];0x0 sv 0x009F];
  .test.assert_eq[FUN[x;y][0x61; 2.5;255i;-2h;-2];0x0 sv 0xFE73];
  .test.assert[.[FUN[x;y];(`a;"A";0;0;0;"A");::]like"rank*";"'rank expected"];
  .test.assert_eq[type FUN[x;y][`a;"A";0;0];104h];
 }/:[5;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][ 1e    ;0x61;  0i; 0h; 0; 1. ]; "h"$"a"     ];
  .test.assert_eq[FUN[x;y][  .5e  ;0x61; -1i;-1h;-1;  .5];0x0 sv 0xFF9E];
  .test.assert_eq[FUN[x;y][-1e    ;0x61;255i;-1h;-1; 1. ];0x0 sv 0x009F];
  .test.assert_eq[FUN[x;y]["e"$1%3;0x61;255i;-2h;-2;-2. ];0x0 sv 0x001A];
  .test.assert[.[FUN[x;y];(`a;"A";0;0;0;"A";());::]like"rank*";"'rank expected"];
  .test.assert_eq[type FUN[x;y][`a;"A";0;0;0];104h];
 }/:[6;ABIs];

{ test_log[x;y];
  .test.assert_eq[FUN[x;y][ 1e    ;0x61;  0i; 0h; 0; 1. ;"\000"]; "i"$"a"         ];
  .test.assert_eq[FUN[x;y][  .5e  ;0x61; -1i;-1h;-1;  .5;"\377"];0x0 sv 0xFFFFFF61];
  .test.assert_eq[FUN[x;y][-1e    ;0x61;255i;-1h;-1; 1. ;"\001"];0x0 sv 0xFFFFFF61];
  .test.assert_eq[FUN[x;y]["e"$1%3;0x61;255i;-2h;-2;-2. ;"\000"];0x0 sv 0xFFFFFFE6];
  .test.assert[.[FUN[x;y];(`a;"A";0;0;0;"A";();::);::]like"rank*";"'rank expected"];
  .test.assert_eq[type FUN[x;y][`a;"A";0;0;0;"A"];104h];
 }/:[7;ABIs];

// Test erroneous cases
.test.assert[
  10h=type .[loader;(TEST_DLL;`f8;`;" ";"        ");::];
  "foreign-function not found" ];
.test.assert[
  10h=type .[loader;(TEST_DLL;`f1;`;"#";"c");::];
  "invalid return type" ];
.test.assert[
  10h=type .[loader;(TEST_DLL;`f1;`;"c";"#");::];
  "invalid argument type" ];

.test.log"All tests completed";
\\

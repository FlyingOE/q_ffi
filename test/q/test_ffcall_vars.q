/# vim: set et noai ts=2 sw=2 syntax=q:
/////////////////////////////////////////////////////////////////////////////
system"l ",string .Q.dd[first` vs hsym .z.f;`test.lib.q]

if[not system"s"; '"Start ",string[.z.f]," with -s!" ];
\P 17

/////////////////////////////////////////////////////////////////////////////
mem:{.mem.sample[]}each til 10
mid:-1
mem_check:{[check] check . mem mid+-1 0 }

/////////////////////////////////////////////////////////////////////////////
getter:0N!DLL 2:(GETTER;3)
setter:0N!DLL 2:(SETTER;4)
.test.log"After loading DLL ",.Q.s1[DLL];

VAR:()!()
VAR[`]:()

// For values of v_*, refer to ./dll/test_dll.cpp
VAR[`c]:0N!getter[TEST_DLL;`v_char ;"c"]
.test.assert_eq[VAR.c;"C"];

VAR[`x]:0N!getter[TEST_DLL;"v_byte";"x"]
.test.assert_eq[VAR.x;0xCC];

VAR[`h]:0N!getter[TEST_DLL;`v_short;"h"]
.test.assert_eq[VAR.h;0x0 sv 0xDEAD];

VAR[`i]:0N!getter[TEST_DLL;`v_int  ;"i"]
.test.assert_eq[VAR.i;0x0 sv 0xDEADBEEF];

VAR[`j]:0N!getter[TEST_DLL;`v_long ;"j"]
.test.assert_eq[VAR.j;0x0 sv 0xDEADBEEF8BADF00D];

VAR[`e]:0N!getter[TEST_DLL;`v_real ;"e"]
.test.assert_eq[VAR.e;3.14159265e];

VAR[`f]:0N!getter[TEST_DLL;`v_float;"f"]
.test.assert_eq[VAR.f;3.141592653589793];

.test.log"After loading variables from DLL ",.Q.s1[TEST_DLL];

//===
v:0N!setter[TEST_DLL;`v_char ;"c"; "a"]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_char ;"c"]; "a"];

v:0N!setter[TEST_DLL;`v_byte ;"x"; 0b sv@[;0;not]0b vs VAR.x]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_byte ;"x"]; 0x4C];

v:0N!setter[TEST_DLL;`v_short;"h"; 0b sv@[;0;not]0b vs VAR.h]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_short;"h"]; 0x0 sv 0x5EAD];

v:0N!setter[TEST_DLL;`v_int  ;"i"; 0b sv@[;0; not]0b vs VAR.i]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_int  ;"i"]; 0x0 sv 0x5EADBEEF];

v:0N!setter[TEST_DLL;`v_long ;"j"; 0b sv@[;0;not]0b vs VAR.j]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_long ;"j"]; 0x0 sv 0x5EADBEEF8BADF00D];

v:0N!setter[TEST_DLL;`v_real ;"e"; neg VAR.e]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_real ;"e"]; -3.14159265e];

v:0N!setter[TEST_DLL;`v_float;"f"; neg VAR.f]
.test.assert_eq[v;::]
.test.assert_eq[getter[TEST_DLL;`v_float;"f"]; -3.141592653589793];

// Test erroneous cases
.test.assert[
  10h=type .[getter;(TEST_DLL;`v_none;"c");::];
  "foreign variable not found (R)" ];
.test.assert[
  10h=type .[getter;(TEST_DLL;`v_none;"#");::];
  "invalid variable type (R)" ];

.test.assert[
  10h=type .[setter;(TEST_DLL;`v_none;"c";"\000");::];
  "foreign variable not found (W)" ];
.test.assert[
  10h=type .[setter;(TEST_DLL;`v_none;"#";"\000");::];
  "invalid variable type (W)" ];
.test.assert[
  10h=type .[setter;(TEST_DLL;`v_char;"c";3.14);::];
  "invalid value type (W)" ];

.test.log"All tests completed";
\\
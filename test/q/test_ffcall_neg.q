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
loader:0N!DLL 2:(LOADER;5)
to_addr:0N!DLL 2:(ADDR_GETTER;1)
size_t:0N!(DLL 2:(SIZE_T;1))[]
ptr_t :0N!(DLL 2:(PTR_T ;1))[]
.test.log"After loading DLL ",.Q.s1[DLL];

/////////////////////////////////////////////////////////////////////////////
FUN:()!()
{ FUN[x]:0N!loader[TEST_DLL;`$"negate_",string x;`;"&";"&",size_t]
 }each
  `bool`char`int8_t`int16_t`int32_t`int64_t`float`double;

neg_test:{[id;gen;op]
  n:rand size_t$1e6;
  .test.log"Testing ",.Q.s1[id]," list w/ ",string[n]," elements...";
  par0:gen n;
  par1:op par0;
  0N!0x0 vs addr:to_addr par0;
  0N!0x0 vs res:FUN[id][addr;n];
  .test.assert_eq[addr;res];
  .test.assert_eq[par0;par1];
 }

neg_test[`bool   ;{         x?0b   };not     ];
neg_test[`char   ;{         x?.Q.b6};upper   ];
neg_test[`int8_t ;{         x?0x00 };"x"$neg@];
neg_test[`int16_t;{         x?0h   };neg     ];
neg_test[`int32_t;{         x?0i   };neg     ];
neg_test[`int64_t;{         x?0    };neg     ];
neg_test[`float  ;{"e"$-1e6+x?2e6  };neg     ];
neg_test[`double ;{    -1e6+x?2e6  };neg     ];

n:size_t$8
.test.log"Testing `symbol characters as simple type...";
/ symbol types can be used as both "&" and "s"
FUN[`symbol]:0N!loader[TEST_DLL;`trans_symbol;`;"&";"&s",size_t]
par0:first 1?`$string n;
out0:n#"\000";
par1:upper string par0;
0N!0x0 vs addr:to_addr out0;
0N!0x0 vs res:FUN.symbol[addr;par0;n];
.test.assert_eq[addr;res];
.test.assert_eq[out0;par1];

.test.log"Testing `symbol characters as pointer type...";
/ symbol types can be used as both "&" and "s"
FUN[`symbol]:0N!loader[TEST_DLL;`trans_symbol;`;"&";"&&",size_t]
par0:first 1?`$string n;
out0:n#"\000";
par1:upper string par0;
0N!0x0 vs addr:to_addr out0;
0N!0x0 vs addp:to_addr par0;
0N!0x0 vs res:FUN.symbol[addr;addp;n];
.test.assert_eq[addr;res];
.test.assert_eq[out0;par1];

.test.log"All tests completed";
\\

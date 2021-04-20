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
loader:0N!DLL 2:LOADER
addr_of:0N!DLL 2:ADDR_OF
size_t:0N!(DLL 2:SIZE_T)[]
ptr_t :0N!(DLL 2:PTR_T)[]
.test.log"After loading DLL ",.Q.s1[DLL];

/////////////////////////////////////////////////////////////////////////////
neg_test:{[id;ptr;gen;op]
  n:rand size_t$1e6;
  .test.log"Testing ",.Q.s1[id]," ",
    $[ptr;"pointer";"list"]," w/ ",string[n]," elements...";
  par0:gen n;
  par1:op par0;
  0N!0x0 vs addr:addr_of par0;
  res:size_t$0;
  mem[0]:.mem.sample[];
  $[ptr;
    0N!0x0 vs res:FUN[id][addr;n] ;
              res:FUN[id][par0;n] ];
  mem[1]:.mem.sample[];
  .mem.verify . mem 0 1;
  .test.assert_eq[addr;res];
  .test.assert_eq[par0;par1];
 }

FUN:()!()
{ FUN[x]:0N!loader[TEST_DLL;`$"negate_",string x;`;"&";"&",size_t]
 }each
  `bool`char`int8_t`int16_t`int32_t`int64_t`float`double;
neg_test[`bool   ;1b;{         x?0b   };not     ];
neg_test[`char   ;1b;{         x?.Q.b6};upper   ];
neg_test[`int8_t ;1b;{         x?0x00 };"x"$neg@];
neg_test[`int16_t;1b;{         x?0h   };neg     ];
neg_test[`int32_t;1b;{         x?0i   };neg     ];
neg_test[`int64_t;1b;{         x?0    };neg     ];
neg_test[`float  ;1b;{"e"$-1e6+x?2e6  };neg     ];
neg_test[`double ;1b;{    -1e6+x?2e6  };neg     ];

FUN:()!()
{ FUN[x]:0N!loader[TEST_DLL;`$"negate_",string x;`;"&";y,size_t]
 }./:flip(
  `bool`char`int8_t`int16_t`int32_t`int64_t`float`double;"BCXHIJEF");
neg_test[`bool   ;0b;{         x?0b   };not     ];
neg_test[`char   ;0b;{         x?.Q.b6};upper   ];
neg_test[`int8_t ;0b;{         x?0x00 };"x"$neg@];
neg_test[`int16_t;0b;{         x?0h   };neg     ];
neg_test[`int32_t;0b;{         x?0i   };neg     ];
neg_test[`int64_t;0b;{         x?0    };neg     ];
neg_test[`float  ;0b;{"e"$-1e6+x?2e6  };neg     ];
neg_test[`double ;0b;{    -1e6+x?2e6  };neg     ];

/////////////////////////////////////////////////////////////////////////////
n:size_t$8
.test.log"Testing `symbol characters as simple type...";
/ symbol types can be used as both "&" and "s"
FUN[`symbol]:0N!loader[TEST_DLL;`trans_symbol;`;"&";"&s",size_t]
par0:first 1?`$string n;
out0:n#"\000";
par1:upper string par0;
0N!0x0 vs addr:addr_of out0;
0N!0x0 vs res:FUN.symbol[addr;par0;n];
.test.assert_eq[addr;res];
.test.assert_eq[out0;par1];

.test.log"Testing `symbol characters as pointer type...";
/ symbol types can be used as both "&" and "s"
FUN[`symbol]:0N!loader[TEST_DLL;`trans_symbol;`;"&";"&&",size_t]
par0:first 1?`$string n;
out0:n#"\000";
par1:upper string par0;
0N!0x0 vs addr:addr_of out0;
0N!0x0 vs addp:addr_of par0;
0N!0x0 vs res:FUN.symbol[addr;addp;n];
.test.assert_eq[addr;res];
.test.assert_eq[out0;par1];

.test.log"All tests completed";
\\

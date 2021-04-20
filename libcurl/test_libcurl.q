BUILD_MODE:"Release"
BUILD_MODE:"Debug"

basedir:first` vs hsym .z.f;
system"cd ",1_string .Q.dd[basedir;]`$
  "../out/install/",$[.z.o like"*64";"x64";"x86"],"-",BUILD_MODE,"/bin";
\l q_ffi.q
\cd ../../../../libcurl

\l libcurl.q
-1 .curl.version[];

0N!0x0 vs
  curl:.curl.easy_init[];

.curl.easy_setopt["C"][curl;.curl.OPT_URL;"http://example.com"]

0N!.curl.easy_perform curl;

.curl.easy_cleanup curl;

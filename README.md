# q_ffi

## K9-like Foreign Function Interface for k4

This project provides a k9-like <a href="https://en.wikipedia.org/wiki/Foreign_function_interface"><abbr title="foreign function interface">FFI</abbr></a> for k4 inspired by k9.

K9 ([Shakti](https://shakti.com/)) introduced a [new FFI](https://groups.google.com/forum/#!msg/shaktidb/Kk-OM3yXew0/dFjXTu_7BAAJ) in 2020. This new FFI improve over the FFI (`2:`) in k4 ([kdb+/q](https://kx.com/)) and allows direct loading of foreign functions that are not written specifically for k9.

In the original FFI of k4, foreign functions must be compiled with [`k.h`](https://github.com/KxSystems/kdb/blob/master/c/c/k.h) and all arguments of such functions must be of type [`K`](https://github.com/KxSystems/kdb/blob/master/c/c/k.h#L11). This restriction results in a lot of boilerplate code if there are a significant number of foreign functions. And this also makes it more tedious to reuse existing third-party libraries in q scripts.

The new FFI in k9 avoids the above problems by allowing direct specification of function signature during library loading, thus eliminating the need for boilerplate code to convert `K` into data types usable in a foreign language.

### License

[Apache License v2.0](./LICENSE)

### Build

#### Building on Linux-like environments

FIXME

`q_ffi` is built with [CMake](https://cmake.org/), following the classic CMake Build Procedure&trade;:

```sh
mkdir build    # build directory
cd build
cmake ..
make
make install
```

#### Building with Visual Studio

Microsoft Visual Studio has provided [native support for CMake][1] since Visual Studio 2017.

To build with the IDE, simply use the "Open Folder..." feature in Visual Studio and the project should automatically imported. Please refer to [Microsoft's document][2] on how to work with CMake projects in Visual Studio.

To build from the command line, follow [this document][3] to launch the build environment, then adapt the build procedure [above](#Building on Linux-like environments) accordingly:

```batch
:: Run in "Developer Command Prompt for VS 2017/2019"
mkdir build                                   :: build directory
cd build
cmake .. -G"Visual Studio 2017 15"            :: VS 2017, default (x64) build
cmake .. -G"Visual Studio 2017 15 Win32"      :: VS 2017, Win32 (x86) build
cmake .. -G"Visual Studio 2019 16"            :: VS 2019, default (x64) build
cmake .. -G"Visual Studio 2019 16" -A Win32   :: VS 2019, Win32 (x86) build
cmake --build .
cmake --build . --target install
```

[1]: https://devblogs.microsoft.com/cppblog/cmake-support-in-visual-studio/
[2]: https://docs.microsoft.com/cpp/build/cmake-projects-in-visual-studio
[3]: https://docs.microsoft.com/cpp/build/building-on-the-command-line
# q_ffi

## K9-like Foreign Function Interface for k4

This project provides a k9-like <a href="https://en.wikipedia.org/wiki/Foreign_function_interface"><abbr title="foreign function interface">FFI</abbr></a> for k4.

K9 ([Shakti](https://shakti.com/)) introduced a [new FFI](https://groups.google.com/forum/#!msg/shaktidb/Kk-OM3yXew0/dFjXTu_7BAAJ) in 2020. This new FFI improve over the old FFI in k4 ([kdb+/q](https://kx.com/)) and allows direct loading of foreign functions that are not written specifically for k9.

In the original FFI of k4, foreign functions must be compiled with [`k.h`](https://github.com/KxSystems/kdb/blob/master/c/c/k.h) and all arguments of such functions must be of type [`K`](https://github.com/KxSystems/kdb/blob/master/c/c/k.h#L11). This restriction results in a lot of boilerplate code if there are a significant number of foreign functions. And this also makes it more tedious to reuse existing third-party libraries in q scripts.

The new FFI in k9 avoids the above problems by allowing direct specification of function signature during library loading, thus eliminating the need for boilerplate code to convert `K` into data types usable in a foreign language.

### License

[Apache License v2.0](./LICENSE)

### Build
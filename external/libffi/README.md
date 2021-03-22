# How to build `libffi`

## Rationale

The oiginal libffi project supports Windows x86 and x64 builds only via MinGW or Cygwin environment. Furthermore, the original project utilizes Automake, which makes it hard to integrate into `q_ffi`'s CMake biuld system. Although there have been efforts to provide [CMake support](#References) for libffi, none of the current projects can work properly.

The following instructions targets libffi v3.3. Furture versions might require minor tweaks if applicable.

## How to build `libffi` for GCC on Linux

### Build Environment

- GCC
- autoconf & automake & libtool

### Build Instructions

The following steps assume a x86_64 system.

#### 64-bit (x86_64) Build

0. Download (and unpack) or checkout (Git tag=`v3.3`) libffi sources.
   - If checked out from GitHub, run the following to generate `configure` script:
     ```bash
     ./autogen.sh
     ```
1. Run configure, no need to build documentations:
   ```bash
   ./configure --disable-docs
   ```
2. Build it:
   ```bash
   make
   ```
3. The binaries will be built into the subdirectory `./x86_64-pc-linux-gnu/.libs/`.
   - `libffi.a`/`libffi.so.8.1.0`: libraries for dynamically linking with libffi.
   - `libffi_convenience.a`: library for statically linking with libffi (must define `FFI_BUILDING` before including `ffi.h`).
4. Finally, copy the output files into q_ffi project's source directory:
   ```bash
   cp ./x86_64-pc-linux-gnu/include/ffi.h {q_ffi_dir}/external/libffi/linux/x86_64/include/
   cp ./x86_64-pc-linux-gnu/include/ffitarget.h {q_ffi_dir}/external/libffi/linux/x86_64/include/
   cp ./x86_64-pc-linux-gnu/.libs/libffi_convenience.{a,la} {q_ffi_dir}/external/libffi/linux/x86_64/lib/
   cp ./x86_64-pc-linux-gnu/.libs/libffi.{a,la} {q_ffi_dir}/external/libffi/linux/x86_64/lib/
   cp ./x86_64-pc-linux-gnu/.libs/libffi.so.8.1.0 {q_ffi_dir}/external/libffi/linux/x86_64/lib/
   ln -s libffi.so.8.1.0 {q_ffi_dir}/external/libffi/linux/x86_64/lib/libffi.so.8
   ln -s libffi.so.8.1.0 {q_ffi_dir}/external/libffi/linux/x86_64/lib/libffi.so
   ```

#### 32-bit (x86) Build

0. Download (and unpack) or checkout (Git tag=`v3.3`) libffi sources.
   - If checked out from GitHub, run the following to generate `configure` script:
     ```bash
     ./autogen.sh
     ```
1. Run configure, no need to build documentations:
   ```bash
   ./configure --disable-docs --build=i686-pc-linux-gnu
   ```
2. Build it:
   ```bash
   make
   ```
3. The binaries will be built into the subdirectory `./i686-pc-linux-gnu/.libs/`.
   - `libffi.a`/`libffi.so.8.1.0`: libraries for dynamically linking with libffi.
   - `libffi_convenience.a`: library for statically linking with libffi (must define `FFI_BUILDING` before including `ffi.h`).
4. Finally, copy the output files into q_ffi project's source directory:
   ```bash
   cp ./i686-pc-linux-gnu/include/ffi.h {q_ffi_dir}/external/libffi/linux/i686/include/
   cp ./i686-pc-linux-gnu/include/ffitarget.h {q_ffi_dir}/external/libffi/linux/i686/include/
   cp ./i686-pc-linux-gnu/.libs/libffi_convenience.{a,la} {q_ffi_dir}/external/libffi/linux/i686/lib/
   cp ./i686-pc-linux-gnu/.libs/libffi.{a,la} {q_ffi_dir}/external/libffi/linux/i686/lib/
   cp ./i686-pc-linux-gnu/.libs/libffi.so.8.1.0 {q_ffi_dir}/external/libffi/linux/i686/lib/
   ln -s libffi.so.8.1.0 {q_ffi_dir}/external/libffi/linux/i686/lib/libffi.so.8
   ln -s libffi.so.8.1.0 {q_ffi_dir}/external/libffi/linux/i686/lib/libffi.so
   ```

## How to build `libffi` for MSVC on Windows

### Build Environment

- Microsoft Visual Studio Community 2019
  - C++ development environment
- Cygwin
  - GNU Make

### Build Instructions

#### 64-bit (x64) Build

0. Download (and unpack) or checkout (Git tag=`v3.3`) libffi sources.
1. From Start menu, run "**x64** Native Tools Command Prompt for VS 2019".
   This opens the MSVC development command prompt for x64.
2. Start Cygwin in the development command prompt, for example:  
   ```batch
   D:\cygwin\Cygwin.bat
   ```
3. Go into the directory with libffi sources, for example:  
   ```bash
   cd E:/DEV/libffi-3.3
   ```
4. Make a new directory for the building process and go into it, for example:  
  ```bash
  mkdir build-x64
  cd ./build-x64
  ```
5. Depending on which VC runtime you want to use, run one of the following commands:
   - Dynamic runtime (`/MD`)  
     ```bash
     ../configure CC="../msvcc.sh -m64" CXX="../msvcc.sh -m64" LD=link CPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL" --build=x86_64-unknown-cygwin
     ```
   - Static runtime (`/MT`)  
     ```bash
     ../configure CC="../msvcc.sh -DUSE_STATIC_RTL -DUSE_DEBUG_RTL -m64" CXX="../msvcc.sh -DUSE_STATIC_RTL -DUSE_DEBUG_RTL -m64" LD=link CPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL" --build=x86_64-unknown-cygwin
     ```
6. Verify that `./include/ffitarget.h` is not a symlink.  
   If it is, as Visual Studio does not understand symlinks, replace it with a real file:  
   ```bash
   rm ./include/ffitarget.h
   cp ../src/x86/ffitarget.h ./include/
   ```
7. If necessary, comment out the following line in `./include/ffitarget.h`:  
   ```cpp
   #define FFI_TARGET_HAS_COMPLEX_TYPE
   ```
8. Build it:  
   ```bash
   make
   ```
9. The binaries will be built into the subdirectory `./.libs/`.
   - `libffi-7.lib`/`libffi-7.dll`: libraries for dynamically linking with libffi.
   - `libffi_convenience.lib`: library for statically linking with libffi (must define `FFI_BUILDING` before including `ffi.h`).
10. If met problems during the build process, please refer to the blog post [below](#References) for details.
11. Finally, copy the output files into q_ffi project's source directory:  
    ```bash
    cp ./include/ffi.h {q_ffi_dir}/external/libffi/msvc/x64/include/
    cp ./include/ffitarget.h {q_ffi_dir}/external/libffi/msvc/x64/include/
    cp ./.libs/libffi_convenience.lib {q_ffi_dir}/external/libffi/msvc/x64/lib/
    cp ./.libs/libffi-7.{dll,lib,exp} {q_ffi_dir}/external/libffi/msvc/x64/lib/
    ```

#### 32-bit (x86) Build

0. Download (and unpack) or checkout (Git tag=`v3.3`) libffi sources.
1. From Start menu, run "**x86** Native Tools Command Prompt for VS 2019".
   This opens the MSVC development command prompt for x86.
2. Start Cygwin in the development command prompt, for example:  
   ```batch
   D:\cygwin\Cygwin.bat
   ```
3. Go into the directory with libffi sources, for example:  
   ```bash
   cd E:/DEV/libffi-3.3
   ```
4. Make a new directory for the building process and go into it, for example:  
  ```bash
  mkdir build-x86
  cd ./build-x86
  ```
5. Depending on which VC runtime you want to use, run one of the following commands:
   - Dynamic runtime (`/MD`)  
     ```bash
     ../configure CC="../msvcc.sh" CXX="../msvcc.sh" LD=link CPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL" --build=i686-unknown-cygwin
     ```
   - Static runtime (`/MT`)  
     ```bash
     ../configure CC="../msvcc.sh -DUSE_STATIC_RTL" CXX="../msvcc.sh -DUSE_STATIC_RTL" LD=link CPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL" --build=i686-unknown-cygwin
     ```
6. Verify that `./include/ffitarget.h` is not a symlink.  
   If it is, as Visual Studio does not understand symlinks, replace it with a real file:  
   ```bash
   rm ./include/ffitarget.h
   cp ../src/x86/ffitarget.h ./include/
   ```
7. If necessary, comment out the following line in `./include/ffitarget.h`:  
   ```cpp
   #define FFI_TARGET_HAS_COMPLEX_TYPE
   ```
8. Build it:  
   ```bash
   make
   ```
9. The binaries will be built into the subdirectory `./.libs/`.
   - `libffi-7.lib`/`libffi-7.dll`: libraries for dynamically linking with libffi.
   - `libffi_convenience.lib`: library for statically linking with libffi (must define `FFI_BUILDING` before including `ffi.h`).
10. If met problems during the build process, please refer to the blog post [below](#References) for details.
11. Finally, copy the output files into q_ffi project's source directory:  
    ```bash
    cp ./include/ffi.h {q_ffi_dir}/external/libffi/msvc/x86/include/
    cp ./include/ffitarget.h {q_ffi_dir}/external/libffi/msvc/x86/include/
    cp ./.libs/libffi_convenience.lib {q_ffi_dir}/external/libffi/msvc/x86/lib/
    cp ./.libs/libffi-7.{dll,lib,exp} {q_ffi_dir}/external/libffi/msvc/x86/lib/
    ```

## References

- [`LICENSE`](./LICENSE)
  - https://github.com/libffi/libffi (original project, using Automake)
  - https://github.com/am11/libffi (CMake build -- can build lib, but crashes upon FFI invocation) **FIXME** pending investigation!
  -  https://github.com/newlawrence/Libffi (CMake build -- not based upon the original project structure, hard to keep version updated)
- [Building libffi on Windows by Visual Studio](https://hostagebrain.blogspot.com/2015/06/building-libffi-on-windows-by-visual.html) ([cached HTML](./Building libffi on Windows by Visual Studio.html))

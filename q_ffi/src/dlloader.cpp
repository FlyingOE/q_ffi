#include "dlloader.hpp"
#include <dlfcn.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#ifndef NDEBUG
#   include <iostream>
#   include <iomanip>
#endif

using namespace std;

q_ffi::DLLoader::DLLoader(char const* filename)
    : dll_{ nullptr }
{
#ifndef NDEBUG
    cout << "Loading DLL `" << filename << "'..." << endl;
#endif
    assert(nullptr != filename);
    dll_ = dlopen(filename, RTLD_LAZY);

    if (nullptr == dll_) {
        ostringstream buffer;
        buffer << "Failed to load DLL `" << filename << "' (error = " << dlerror() << ')';
        throw runtime_error(buffer.str());
    }
    else {
#   ifndef NDEBUG
        cout << "Loaded DLL `" << filename << "' into 0x" << hex << dll_ << endl;
#   endif
    }
}

q_ffi::DLLoader::~DLLoader()
{
    if (nullptr != dll_) {
#   ifndef NDEBUG
        cout << "Unloading DLL from 0x" << hex << dll_ << endl;
#   endif
        dlclose(dll_);
        dll_ = nullptr;
    }
}

void
q_ffi::DLLoader::verifyState() const
{
    if (nullptr == dll_)
        throw runtime_error("DLL not loaded yet");
}

void*
q_ffi::DLLoader::locateProc(char const* funcName) const
{
    assert(nullptr != funcName);
    verifyState();
    dlerror();  // clear error condition
    auto const fp = dlsym(dll_, funcName);
    string err{ dlerror() };
    if (err.empty())
        throw runtime_error(err);
    else
        return fp;
}

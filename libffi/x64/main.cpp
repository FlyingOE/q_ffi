#include <iostream>
#include <dlfcn.h>
#include <alloca.h>
#include <ffi.h>


//typedef void* (*arbitrary)();

typedef int(* FUNC_ADD)(int, int);


int main(){
    int m=3,n=2;
    //printf("m=%d, n=%d\n",m,n);

    //arbitrary func;
    void *handler=dlopen("lib_x64_shared.so",RTLD_LAZY);
    if (dlerror()!=NULL){
        std::cout<<dlerror()<<std::endl;
    }

    void* functionPtr1=(void*)dlsym(handler,"add");
    void* functionPtr2=(void*)dlsym(handler,"sub");
    int argCount = 2;

    //argtypes
    ffi_type **ffiArgTypes =(ffi_type**) alloca(sizeof(ffi_type *) *argCount);
    ffiArgTypes[0] = &ffi_type_sint;
    ffiArgTypes[1] = &ffi_type_sint;

    //args
    void **ffiArgs = (void**)alloca(sizeof(void *) *argCount);
    void *ffiArgPtr = alloca(ffiArgTypes[0]->size);
    int *argPtr = (int*)ffiArgPtr;
    *argPtr = 5;
    ffiArgs[0] = ffiArgPtr;

    void *ffiArgPtr2 = alloca(ffiArgTypes[1]->size);
    int *argPtr2 =(int*) ffiArgPtr2;
    *argPtr2 = 3;
    ffiArgs[1] = ffiArgPtr2;

    //ffi_cfi
    ffi_cif cif;
    ffi_type *returnFfiType = &ffi_type_sint;
    ffi_status ffiPrepStatus = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)argCount, returnFfiType, ffiArgTypes);

    if (ffiPrepStatus == FFI_OK) {
        //生成用于保存返回值的内存
        void *returnPtr = NULL;
        if (returnFfiType->size) {
            returnPtr = alloca(returnFfiType->size);
        }
        //根据cif函数原型，函数指针，返回值内存指针，函数参数数据调用这个函数
        ffi_call(&cif, (void(*)()) functionPtr1, returnPtr, ffiArgs);
        //拿到返回值
        int returnValue = *(int *)returnPtr;
        std::printf("ret_add: %d \n", returnValue);

        ffi_call(&cif, (void(*)()) functionPtr2, returnPtr, ffiArgs);
        returnValue = *(int *)returnPtr;
        std::printf("ret_sub: %d \n", returnValue);

    }


    dlclose(handler);

    return 0;
}
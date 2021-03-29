#include <stdio.h>
#include <ffi.h>

int testFunc(int m, int n) {
    printf("params: %d %d \n", m, n);
    return m+n;
}

int main(){
    testFunc(1, 2);

    //拿函数指针
    void* functionPtr = &testFunc;
    int argCount = 2;

    //参数类型数组
    ffi_type **ffiArgTypes = alloca(sizeof(ffi_type *) *argCount);
    ffiArgTypes[0] = &ffi_type_sint64;
    ffiArgTypes[1] = &ffi_type_sint64;

    //参数数据数组
    void **ffiArgs = alloca(sizeof(void *) *argCount);
    void *ffiArgPtr = alloca(ffiArgTypes[0]->size);
    int *argPtr = ffiArgPtr;
    *argPtr = 5;
    ffiArgs[0] = ffiArgPtr;

    void *ffiArgPtr2 = alloca(ffiArgTypes[1]->size);
    int *argPtr2 = ffiArgPtr2;
    *argPtr2 = 3;
    ffiArgs[1] = ffiArgPtr2;

    //生成函数原型 ffi_cfi 对象
    ffi_cif cif;
    ffi_type *returnFfiType = &ffi_type_sint64;
    ffi_status ffiPrepStatus = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)argCount, returnFfiType, ffiArgTypes);

    if (ffiPrepStatus == FFI_OK) {
        //生成用于保存返回值的内存
        void *returnPtr = NULL;
        if (returnFfiType->size) {
            returnPtr = alloca(returnFfiType->size);
        }
        //根据cif函数原型，函数指针，返回值内存指针，函数参数数据调用这个函数
        ffi_call(&cif, functionPtr, returnPtr, ffiArgs);

        //拿到返回值
        int returnValue = *(int *)returnPtr;
        printf("ret: %d \n", returnValue);
    }

return  0;
}

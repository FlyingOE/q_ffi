#include <stdio.h>
#include <ffi.h>
// 函数实体
void calCircleArea(ffi_cif *cif,
                  float *ret,
                  void *args[],
                  FILE *stream) {
    float pi = 3.14;
    float r = **(float **)args[0];
    float area = pi * r * r;
    *ret = area;
    printf("我是那个要被动态调用的函数\n area:%.2f\n *ret = %.2f",area,*ret);
}


int main(int argc, const char * argv[]) {

    ///函数原型
    ffi_cif cif;
    ///参数
    ffi_type *args[1];
    ///回调闭包
    ffi_closure *closure;
    ///声明一个函数指针,通过此指针动态调用已准备好的函数
    float (*bound_calCircleArea)(float *);
    float rc = 0;
    
    /* Allocate closure and bound_calCircleArea */  //创建closure
    closure = ffi_closure_alloc(sizeof(ffi_closure), &bound_calCircleArea);
    
    if (closure) {
        /* Initialize the argument info vectors */
        args[0] = &ffi_type_pointer;
        /* Initialize the cif */  //生成函数原型 &ffi_type_float：返回值类型
        if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_float, args) == FFI_OK) {
            /* Initialize the closure, setting stream to stdout */
                // 通过 ffi_closure 把 函数原型_cifPtr / 函数实体JPBlockInterpreter / 上下文对象self / 函数指针blockImp 关联起来
            if (ffi_prep_closure_loc(closure, &cif, calCircleArea,stdout, bound_calCircleArea) == FFI_OK) {
                    float r = 10.0;
                    //当执行了bound_calCircleArea函数时，获得所有输入参数, 后续将执行calCircleArea。
                    //动态调用calCircleArea
                    rc = bound_calCircleArea(&r);
                    printf("rc = %.2f\n",rc);
                }
            }
        }
    /* Deallocate both closure, and bound_calCircleArea */
    ffi_closure_free(closure);   //释放闭包
    return 0;
}
#include <stdio.h>
#include <ffi.h>

/* Acts like puts with the file given at time of enclosure */
void puts_binding(ffi_cif* cif, void* ret, void* args[], void* stream) {
  *(ffi_arg*) ret = fputs(*(char**) args[0], (FILE*) stream);
}

typedef int (*puts_t)(char*);

int main() {
  ffi_cif cif; /* The call interface */
  ffi_type* args[1]; /* The array of pointers to function argument types */
  ffi_closure* closure; /* The allocated closure writable address */
  void* bound_puts; /* The allocated closure executable address */
  int rc; /* The function invocation return code */

  /* Allocate closure (writable address) and bound_puts (executable address) */
  closure = ffi_closure_alloc(sizeof(ffi_closure), &bound_puts);

  if (closure) {
    /* Initialize the array of pointers to function argument types */
    args[0] = &ffi_type_pointer;

    /* Initialize the call interface describing the function prototype */
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_sint, args) == FFI_OK) {
      /* Initialize the closure, setting stream to stdout */
      if (ffi_prep_closure_loc(closure, &cif, puts_binding, stdout, bound_puts) == FFI_OK) {
        rc = ((puts_t) bound_puts)("Hello World!");
        /* rc now holds the result of the call to fputs */
      }
    }
  }

  /* Deallocate both closure, and bound_puts */
  ffi_closure_free(closure);

  return 0;
}
#ifndef OLISP_TINY_GC
#define OLISP_TINY_GC

typedef struct _olisp_tiny_gc_env{
    EBM_ALLOCATOR allocator;
    struct _olisp_tiny_gc_env *parent_env;
    EBM_FREE free;
    EBM_WRITE_BARRIER write_barrier;
    uintptr_t env;
}olisp_tiny_gc_env;

#endif

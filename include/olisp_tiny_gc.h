#ifndef OLISP_TINY_GC
#define OLISP_TINY_GC

typedef struct _olisp_tiny_gc_env{
    EBM_ALLOCATOR allocator;
    uintptr_t allocator_env;
    struct _olisp_tiny_gc_env *parent_env;
    EBM_FREE free;
    EBM_GC_INTERFACE *gc_interface;
    uintptr_t arena;

    uintptr_t state;
    uintptr_t root;
    uintptr_t scan_stack;
    uintptr_t step_counter;

}olisp_tiny_gc_env;

#define OLISP_TINY_GC_STEP_NUMBER 50

uintptr_t EBM_allocate_olisp_tiny_gc_env(EBM_ALLOCATOR parent_allocator,uintptr_t parent_allocator_env);

uintptr_t EBM_olisp_tiny_allocate(size_t size,uintptr_t env);


uintptr_t EBM_olisp_tiny_gc_write_barrier(uintptr_t object,uintptr_t setted_object,uintptr_t env_ptr);

#endif

#ifndef EBM_H
#define EBM_H

#include<stdint.h>
#include<stddef.h>
#include<stdlib.h>

//
//CONFIGS
//
#define EBM_CONFIG_SET_POINTER_INFORMATION 1
#define EBM_CONFIG_POINTER_INFORMATION_SIZE 2

//
//BITS OPERATORS
//
#define EBM_POINTER_INFORMATION_BITS ((1<<EBM_CONFIG_POINTER_INFORMATION_SIZE)-1)
#define EBM_ADD_TYPE(p,type) (((uintptr_t)p)+type)
#define EBM_GET_TYPE(p) (((uintptr_t)p)&EBM_POINTER_INFORMATION_BITS)
#define EBM_REMOVE_TYPE(p) ((((uintptr_t)p) >>EBM_CONFIG_POINTER_INFORMATION_SIZE ) << EBM_CONFIG_POINTER_INFORMATION_SIZE )

//
//INFO IN POINTER
//
#define EBM_STATICS 0
#define EBM_TYPE_PAIR 1
#define EBM_TYPE_RECORD 2

//
//STATIC OBJECTS
//
#define EBM_FALSE 0
#define EBM_UNDEF (0+(1<<EBM_CONFIG_POINTER_INFORMATION_SIZE))

//
//OBJECT MANAGER
//
typedef uintptr_t (*EBM_ALLOCATOR)(size_t,uintptr_t);
uintptr_t EBM_malloc_wrapper(size_t size,uintptr_t env);
void EBM_free_wrapper(uintptr_t obj,uintptr_t env);

//
//RECORD TYPES
//
#define EBM_BUILT_IN_RECORD_TYPE_PORT (5<<EBM_CONFIG_POINTER_INFORMATION_SIZE)

//RECORD OPERATIONS
//
uintptr_t EBM_allocate_record(uint32_t size,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_record_primitive_set(uintptr_t record,size_t index,uintptr_t p);

//
//OPTIMIZATION MACROS FOR C　
//
#ifndef __GNUC__
#define EBM_COPT_LIKELY(x) __builtin_expect(!!(x),1)
#define EBM_COPT_UNLIKELY(x) __builtin_expect(!!(x),0)
#else
#define EBM_COPT_LIKELY(x) (x)
#define EBM_COPT_UNLIKELY(x) (x)
#endif

#endif

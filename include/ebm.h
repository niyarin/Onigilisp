#ifndef EBM_H
#define EBM_H

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

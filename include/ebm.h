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
#define EBM_NULL (1<<(EBM_CONFIG_POINTER_INFORMATION_SIZE+1))
#define EBM_UNDEF (2<<(EBM_CONFIG_POINTER_INFORMATION_SIZE+1))

//
//STATIC OBJECTS OPERATIONS
//
#define EBM_IS_NULL_CR(object) (object == EBM_NULL)

//
//NAIVE INTEGER
//
#define EBM_allocate_NAIVE_INTEGER_CA(cint) ((0b01<<EBM_CONFIG_POINTER_INFORMATION_SIZE) + (cint << (EBM_CONFIG_POINTER_INFORMATION_SIZE + 2)))
#define EBM_allocate_character_CA(cchar) ((0b11<<EBM_CONFIG_POINTER_INFORMATION_SIZE) + (cchar << (EBM_CONFIG_POINTER_INFORMATION_SIZE + 2)))
#define EBM_NAIVE_INTEGER_TO_C_INTEGER_CR(integer) (integer>>(EBM_CONFIG_POINTER_INFORMATION_SIZE + 2))
#define EBM_NAIVE_ADD_INTEGER(n1,n2) (EBM_NAIVE_INTEGER((EBM_NAIVE_INTEGER_TO_C_INTEGER_CR(n1)) + EBM_NAIVE_INTEGER_TO_C_INTEGER_CR(n2)))

//
//PAIR OPERATIONS
//
#define EBM_CAR(p) (((uintptr_t*)(EBM_REMOVE_TYPE(p)))[0])
#define EBM_CDR(p) (((uintptr_t*)(EBM_REMOVE_TYPE(p)))[1])
#define EBM_SET_CAR(p,o) ((((uintptr_t*)(EBM_REMOVE_TYPE(p)))[0]) = o)
#define EBM_SET_CDR(p,o) ((((uintptr_t*)(EBM_REMOVE_TYPE(p)))[1]) = o)

#define EBM_CAAR(p) EBM_CAR(EBM_CAR(p))
#define EBM_CADR(p) EBM_CAR(EBM_CDR(p))
#define EBM_CDAR(p) EBM_CDR(EBM_CAR(p))
#define EBM_CDDR(p) EBM_CDR(EBM_CDR(p))
#define EBM_CADDR(p) EBM_CAR(EBM_CDDR(p))


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
#define EBM_BUILT_IN_RECORD_TYPE_VECTOR (2<<EBM_CONFIG_POINTER_INFORMATION_SIZE )

//
//RECORD OPERATIONS
//
uintptr_t EBM_allocate_record_CA(uint32_t size,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_record_primitive_set_CA(uintptr_t record,size_t index,uintptr_t p);
uintptr_t EBM_record_ref_CA(uintptr_t record,size_t index);
#define EBM_IS_RECORD_CR(object) (EBM_GET_TYPE(object) == EBM_TYPE_RECORD)
#define EBM_record_length_CR(object) (((uintptr_t*)object)[0]) 
#define EBM_record_first(object) (((uintptr_t*)object)[1])
#define EBM_record_second(object) (((uintptr_t*)object)[2])
#define EBM_record_third(object) (((uintptr_t*)object)[3])


//
//vector operation
//
uintptr_t EBM_allocate_vector_CA(size_t size,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_vector_re_allocate_CA(uintptr_t vector,uintptr_t new_size,uintptr_t else_fill,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_vector_primitive_set_CA(uintptr_t vector,size_t index,uintptr_t object);

#define EBM_vector_length_CR(vector) EBM_record_ref_CA(vector,1)
#define EBM_vector_ref_CA(vector,index) EBM_record_ref_CA(vector,index + 2)

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

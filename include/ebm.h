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

#define EBM_SMALL_OBJECT_LENGTH 5

//
//BITS OPERATORS
//
#define EBM_POINTER_INFORMATION_BITS ((1<<EBM_CONFIG_POINTER_INFORMATION_SIZE)-1)
#define EBM_ADD_TYPE(p,type) (((uintptr_t)p)+type)
#define EBM_GET_TYPE(p) (((uintptr_t)p)&EBM_POINTER_INFORMATION_BITS)
#define EBM_REMOVE_TYPE(p) ((((uintptr_t)(p)) >>EBM_CONFIG_POINTER_INFORMATION_SIZE ) << EBM_CONFIG_POINTER_INFORMATION_SIZE )

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
#define EBM_DUMMY_OBJECT (3<<(EBM_CONFIG_POINTER_INFORMATION_SIZE+1))
#define EBM_TRUE (4<<(EBM_CONFIG_POINTER_INFORMATION_SIZE+1))
#define EBM_EOF (5<<(EBM_CONFIG_POINTER_INFORMATION_SIZE+1))
//
//STATIC OBJECTS OPERATIONS
//
#define EBM_IS_NULL_CR(object) (object == EBM_NULL)

//
//NAIVE INTEGER
//
#define EBM_allocate_character_CA(cchar) ((0b11<<EBM_CONFIG_POINTER_INFORMATION_SIZE) + (cchar << (EBM_CONFIG_POINTER_INFORMATION_SIZE + 3)))
#define EBM_char2unicode_CR(echar) (echar>>(EBM_CONFIG_POINTER_INFORMATION_SIZE+3))

//SRFI 143 SUBSET
#define EBM_allocate_FX_NUMBER_CA(cint) ((((intptr_t)0b01)<<EBM_CONFIG_POINTER_INFORMATION_SIZE) + (((intptr_t)cint) << (EBM_CONFIG_POINTER_INFORMATION_SIZE + 2)))
#define EBM_IS_FX_NUMBER_CR(object) ((((object)&(1<<(EBM_CONFIG_POINTER_INFORMATION_SIZE)))&((1<<(EBM_CONFIG_POINTER_INFORMATION_SIZE+2))-1)) == (0b01<<EBM_CONFIG_POINTER_INFORMATION_SIZE))
#define EBM_FX_NUMBER_TO_C_INTEGER_CR(integer) (integer>>(EBM_CONFIG_POINTER_INFORMATION_SIZE + 2))
#define EBM_FX_ADD(n1,n2) (EBM_allocate_FX_NUMBER_CA((EBM_FX_NUMBER_TO_C_INTEGER_CR(n1)) + EBM_FX_NUMBER_TO_C_INTEGER_CR(n2)))
#define EBM_FX_GREATEST_CR (INTPTR_MAX>>(EBM_CONFIG_POINTER_INFORMATION_SIZE+2))

//
//OBJECT MANAGER
//
typedef uintptr_t (*EBM_ALLOCATOR)(size_t,uintptr_t);
typedef uintptr_t (*EBM_GC)(uintptr_t);
typedef uintptr_t (*EBM_FREE)(uintptr_t ,uintptr_t);
typedef uintptr_t (*EBM_WRITE_BARRIER)(uintptr_t,uintptr_t,uintptr_t);
typedef uintptr_t (*EBM_REALLOC)(size_t,uintptr_t,uintptr_t);
typedef uintptr_t (*EBM_GC_STEP)(uintptr_t);
typedef uintptr_t (*EBM_GC_ADD_ROOT)(uintptr_t,uintptr_t);


uintptr_t EBM_malloc_wrapper(size_t size,uintptr_t env);
void EBM_free_wrapper(uintptr_t obj,uintptr_t env);

typedef struct {
    EBM_ALLOCATOR allocator;
    EBM_FREE free;
    EBM_WRITE_BARRIER write_barrier;
    uintptr_t env;
}EBM_GC_INTERFACE;

//
//PAIR OPERATIONS
//
uintptr_t EBM_allocate_pair(uintptr_t car,uintptr_t cdr,EBM_ALLOCATOR allocator,uintptr_t allocator_env);
#define EBM_IS_PAIR_CR(object) (EBM_GET_TYPE(object)==EBM_TYPE_PAIR)
#define EBM_CAR(p) (((uintptr_t*)(EBM_REMOVE_TYPE(p)))[0])
#define EBM_CDR(p) (((uintptr_t*)(EBM_REMOVE_TYPE(p)))[1])
#define EBM_PRIMITIVE_SET_CAR(p,o) ((((uintptr_t*)(EBM_REMOVE_TYPE(p)))[0]) = o)
#define EBM_PRIMITIVE_SET_CDR(p,o) ((((uintptr_t*)(EBM_REMOVE_TYPE(p)))[1]) = o)
uintptr_t EBM_set_car(uintptr_t pair,uintptr_t object,EBM_GC_INTERFACE *gc_interface);
uintptr_t EBM_set_cdr(uintptr_t pair,uintptr_t object,EBM_GC_INTERFACE *gc_interface);
uintptr_t EBM_allocate_rev_list(int size,EBM_ALLOCATOR allocator,uintptr_t allocator_env, ...);

#define EBM_CAAR(p) EBM_CAR(EBM_CAR(p))
#define EBM_CADR(p) EBM_CAR(EBM_CDR(p))
#define EBM_CDAR(p) EBM_CDR(EBM_CAR(p))
#define EBM_CDDR(p) EBM_CDR(EBM_CDR(p))
#define EBM_CDDDR(p) EBM_CDR(EBM_CDDR(p))
#define EBM_CADDR(p) EBM_CAR(EBM_CDDR(p))


//
//CHARACTER TABLE OPERATIONS
//
uintptr_t EBM_char_table_create_CA(size_t size,size_t start_index,EBM_ALLOCATOR allocator,uintptr_t allocator_env);
uintptr_t EBM_char_table_ref_CA(uintptr_t table,size_t cc);
uintptr_t EBM_char_table_primitive_insert_CA(uintptr_t table,uint32_t cc,uintptr_t object,EBM_ALLOCATOR allocator,uintptr_t allocator_env);
uintptr_t EBM_char_table_set_CA(uintptr_t table,uint32_t cc,uintptr_t object,EBM_GC_INTERFACE *gc_interface);

//
//RECORD TYPES
//
#define EBM_BUILT_IN_RECORD_TYPE_SYMBOL EBM_allocate_FX_NUMBER_CA(1)
#define EBM_BUILT_IN_RECORD_TYPE_PORT EBM_allocate_FX_NUMBER_CA(5)
#define EBM_BUILT_IN_RECORD_TYPE_VECTOR EBM_allocate_FX_NUMBER_CA(2)
#define EBM_BUILT_IN_RECORD_TYPE_SYMBOL_TRIE EBM_allocate_FX_NUMBER_CA(3)
#define EBM_BUILT_IN_RECORD_TYPE_RECORD_POINTER EBM_allocate_FX_NUMBER_CA(29)
#define EBM_BUILT_IN_RECORD_TYPE_POINTER_BOX EBM_allocate_FX_NUMBER_CA(30)
#define EBM_BUILT_IN_RECORD_TYPE_OLISP_FUNCTION EBM_allocate_FX_NUMBER_CA(52)

//
//RECORD OPERATIONS
//
uintptr_t EBM_allocate_record_CA(uint32_t size,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_record_primitive_set_CA(uintptr_t record,size_t index,uintptr_t p);
uintptr_t EBM_record_set_CA(uintptr_t record,size_t index,uintptr_t target_object,EBM_WRITE_BARRIER write_barrier,uintptr_t allocator_env);
uintptr_t EBM_record_ref_CA(uintptr_t record,size_t index);
#define EBM_IS_RECORD_CR(object) (EBM_GET_TYPE(object) == EBM_TYPE_RECORD)
#define EBM_record_length_CR(object) (((uintptr_t*)EBM_REMOVE_TYPE(object))[0]) 
#define EBM_record_first(object) (((uintptr_t*)EBM_REMOVE_TYPE(object))[1])
#define EBM_record_second(object) (((uintptr_t*)EBM_REMOVE_TYPE(object))[2])
#define EBM_record_third(object) (((uintptr_t*)EBM_REMOVE_TYPE(object))[3])

//
//VECTOR OPERATIONS
//
uintptr_t EBM_allocate_vector_CA(size_t size,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_vector_re_allocate_CA(uintptr_t vector,uintptr_t new_size,uintptr_t else_fill,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_vector_primitive_set_CA(uintptr_t vector,size_t index,uintptr_t object);
#define EBM_vector_set_CA(vector,index,object,write_barrier,allocator_env) EBM_record_set_CA(vector,index+2,object,write_barrier,allocator_env);
#define EBM_vector_length_CR(vector) EBM_record_ref_CA(vector,1)
#define EBM_vector_ref_CA(vector,index) EBM_record_ref_CA(vector,index + 2)

//
//SYMBOL OPERATIONS
//

uintptr_t EBM_allocate_symbol_CA(uint32_t *symbol,EBM_ALLOCATOR allocator,uintptr_t allocator_env);
uintptr_t EBM_allocate_symbol_from_cstring_CA(char *symbol,EBM_ALLOCATOR allocator,uintptr_t allocator_env);
#define EBM_IS_SYMBOL_CR(object) (EBM_IS_RECORD_CR(object)&&(EBM_record_first(object)==EBM_BUILT_IN_RECORD_TYPE_SYMBOL))


//
//POINTER BOX OPERATIONS
//
uintptr_t EBM_allocate_pointer_box_CA(uintptr_t val,EBM_ALLOCATOR allocator,uintptr_t env);
#define EBM_pointer_box_ref_CR(pointer_box) EBM_record_ref_CA(pointer_box,1)
uintptr_t EBM_pointer_box_set(uintptr_t pointer_box,uintptr_t ptr);

//
//RECORD POINTER OPERATIONS
//
uintptr_t EBM_allocate_record_range_CA(uintptr_t record,uintptr_t left,uintptr_t right,uintptr_t gc_flag,EBM_ALLOCATOR allocator,uintptr_t allocate_env);
#define EBM_record_pointer_range_ref_record(record_pointer)  (EBM_record_ref_CA(record_pointer,2))
#define EBM_record_pointer_range_ref_left_CR(record_pointer)  (EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_ref_CA(record_pointer,4)))
#define EBM_record_pointer_range_set_left(record_pointer,fxnum)  (EBM_record_primitive_set_CA(record_pointer,4,fxnum))
#define EBM_record_pointer_range_ref_right_CR(record_pointer)  (EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_ref_CA(record_pointer,5)))
#define EBM_record_pointer_range_set_right(record_pointer,fxnum)  (EBM_record_primitive_set_CA(record_pointer,5,fxnum))

//
//SYMBOL TRIE OPERATIONS
//
uintptr_t EBM_symbol_trie_ref(uintptr_t trie,uintptr_t symbol);
uintptr_t EBM_allocate_symbol_trie(EBM_ALLOCATOR allocator,uintptr_t allocator_env);
uintptr_t EBM_symbol_trie_set(uintptr_t trie,uintptr_t symbol,uintptr_t object,EBM_GC_INTERFACE *gc_interface);

//
//OLISP FUNCTION OPERATIONS
//
#define EBM_iS_OLISP_FUNCTION_CR(object) (EBM_IS_RECORD_CR(object)&&(EBM_record_first(object)==EBM_BUILT_IN_RECORD_TYPE_OLISP_FUNCTION ))

//
// OTHERS
//
uintptr_t EBM_object_heap_size_CR(uintptr_t object);


//
//OPTIMIZATION MACROS FOR C　
//
#ifndef __GNUC__
#define EBM_COPT_LIKELY(x) __builtin_expect(!!(x),1)
#define EBM_COPT_UNLIKELY(x) __builtin_expect(!!(x),0)
# if !defined(EBM_HAVE_ALLOCA)
#    define EBM_HAVE_ALLOCA
# endif
#else
#define EBM_COPT_LIKELY(x) (x)
#define EBM_COPT_UNLIKELY(x) (x)
#endif

#endif

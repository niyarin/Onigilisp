#include "ebm.h"


uintptr_t EBM_char_table_create_CA(size_t size,size_t start_index,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_vector_CA(size,allocator,allocator_env);
    int i;
    for ( i=1;i<size;i++){
        EBM_vector_primitive_set_CA(res,i,EBM_NULL);
    }
    EBM_vector_primitive_set_CA(res,0,EBM_allocate_FX_NUMBER_CA(start_index));
    return res;
}


//return apair/false
//nullにしても良いかも..
uintptr_t EBM_char_table_ref_CA(uintptr_t table,size_t cc){
    uintptr_t index = (cc%(EBM_vector_length_CR(table)-1))+1;
    uintptr_t cell = EBM_vector_ref_CA(table,index);
    uintptr_t ebm_c = EBM_allocate_character_CA(cc);
    while (!EBM_IS_NULL_CR(cell)){
        if (EBM_CAAR(cell) == ebm_c){
            return EBM_CAR(cell);
        }
        cell = EBM_CDR(cell);
    }
    return EBM_FALSE;
}

uintptr_t EBM_char_table_primitive_insert_CA(uintptr_t table,uint32_t cc,uintptr_t object,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t index = (cc%(EBM_vector_length_CR(table)-1))+1;
    EBM_vector_primitive_set_CA(table,index,EBM_allocate_pair(EBM_allocate_pair(EBM_allocate_character_CA(cc),object,allocator,allocator_env),EBM_vector_ref_CA(table,index),allocator,allocator_env));
    return EBM_UNDEF;
}

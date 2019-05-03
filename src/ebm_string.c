#include "ebm.h"

uintptr_t EBM_allocate_triple_tuple(uintptr_t first,uintptr_t second,uintptr_t third,uintptr_t native_bit,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(5,allocator,env);
    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TRIPLE_TUPLE);
    EBM_record_primitive_set_CA(res,1,first);
    EBM_record_primitive_set_CA(res,2,second);
    EBM_record_primitive_set_CA(res,3,third);
    EBM_record_primitive_set_CA(res,4,native_bit);
    return res;
}

static uintptr_t EBM_allocate_rope_aux_CA(uint32_t *utf32_string,uintptr_t index,uintptr_t size,EBM_ALLOCATOR allocator,uintptr_t env){
    
    if (size > 4){
        uint32_t left_size = size/2;
        uintptr_t left = EBM_allocate_rope_aux_CA(utf32_string,index,left_size,allocator,env);
        uintptr_t right = EBM_allocate_rope_aux_CA(utf32_string,index + left_size,size - left_size,allocator,env);
        
        return EBM_allocate_triple_tuple(left_size,left,right,0b110,allocator,env);
    }else{
        uint32_t *string_chank = allocator(size * sizeof(uint32_t),env);
        int i=0;
        for (i=0;i<size;i++){
            string_chank[i] = utf32_string[index + i];
        }
        return (uintptr_t)string_chank;
    }
}

uintptr_t EBM_allocate_rope_CA(uint32_t *utf32_string,EBM_GC_INTERFACE *gc_interface){
    uintptr_t length = 0;
    while (utf32_string[length]){length++;}
    uintptr_t btree = EBM_allocate_rope_aux_CA(utf32_string,0,length,gc_interface->allocator,gc_interface->env);
    uintptr_t res = EBM_allocate_record_CA(3,gc_interface->allocator,gc_interface->env);
    EBM_record_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_STRING,gc_interface->write_barrier,gc_interface->env);
    EBM_record_set_CA(res,1,length,gc_interface->write_barrier,gc_interface->env);
    EBM_record_set_CA(res,2,btree,gc_interface->write_barrier,gc_interface->env);
    return res;
}

#include "ebm.h"

uintptr_t EBM_allocate_byte_vector_CA(uintptr_t size,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_record_CA(3,allocator,allocator_env);
    EBM_record_primitive_set_CA(res,
                  0,
                  EBM_BUILT_IN_RECORD_TYPE_BYTE_VECTOR);
    unsigned char* bytes = (unsigned char*)allocator(size,allocator_env);

    EBM_record_primitive_set_CA(res,1,bytes);
    EBM_record_primitive_set_CA(res,2,size);
    return res;
}

uintptr_t EBM_byte_vector_ref_CACR(uintptr_t byte_vector,uintptr_t index){
    unsigned char* bytes = EBM_record_ref_CA(byte_vector,1);
    return bytes[index];
}

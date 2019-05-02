#include "ebm.h"
#include "ebm_aux.h"

//hash table comparator= eq?
uintptr_t EBM_allocate_simple_hash_table(
        uintptr_t table_size,
        EBM_ALLOCATOR allocator,
        uintptr_t allocator_env){
    
    uintptr_t res = EBM_allocate_record_CA(3,allocator,allocator_env);
    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_HASH_TABLE);
    EBM_record_primitive_set_CA(res,1,EBM_NULL);//comparator
    uintptr_t vector = EBM_allocate_vector_CA(table_size,allocator,allocator_env);
    int i;
    for (i=0;i<table_size;i++){
        EBM_vector_primitive_set_CA(vector,i,EBM_NULL);
    }
    EBM_record_primitive_set_CA(res,2,vector);

    return res;
}

uintptr_t EBM_simple_hash_table_set(
        uintptr_t hash_table,
        uintptr_t key,
        uintptr_t value,
        EBM_GC_INTERFACE *gc_interface){
    
    uintptr_t table_vector = EBM_record_ref_CA(hash_table,2);
    uintptr_t hash_value = key % EBM_vector_length_CR(table_vector);
    uintptr_t alist = EBM_vector_ref_CA(table_vector,hash_value);
    uintptr_t apair = EBM_assq(key,alist);
    if (apair == EBM_FALSE){
        EBM_vector_set_CA(
                table_vector,
                hash_value,
                EBM_allocate_pair(
                    EBM_allocate_pair(
                        key,
                        value,
                        gc_interface->allocator,
                        gc_interface->env),
                    alist,
                    gc_interface->allocator,
                    gc_interface->env),
                gc_interface->write_barrier,
                gc_interface->env);
        return EBM_UNDEF;
    }

    EBM_set_cdr(apair,value,gc_interface);
    return EBM_UNDEF;
}

uintptr_t EBM_simple_hash_table_ref(uintptr_t hash_table,uintptr_t key){
    uintptr_t table_vector = EBM_record_ref_CA(hash_table,2);
    uintptr_t hash_value = key % EBM_vector_length_CR(table_vector);

    uintptr_t alist = EBM_vector_ref_CA(table_vector,hash_value);
    uintptr_t apair = EBM_assq(key,alist);
    if (apair == EBM_FALSE){
        return EBM_UNDEF;//?? 
    }
    return EBM_CDR(apair);
}

uintptr_t EBM_simple_hash_table_delete(uintptr_t hash_table,uintptr_t key,EBM_GC_INTERFACE *gc_interface){
    uintptr_t table_vector = EBM_record_ref_CA(hash_table,2);
    uintptr_t hash_value = key % EBM_vector_length_CR(table_vector);

    uintptr_t alist = EBM_vector_ref_CA(table_vector,hash_value);
    uintptr_t prev_cell = EBM_NULL;
    while (alist != EBM_NULL){
        if (EBM_CAAR(alist) == key){
            break;
        }
        
        prev_cell = alist;
        alist = EBM_CDR(alist);
    }

    if (alist == EBM_NULL){
        return EBM_UNDEF;
    }

    if (prev_cell == EBM_NULL){
        EBM_vector_set_CA(
                table_vector,
                hash_value,
                EBM_CDR(alist),
                gc_interface->write_barrier,
                gc_interface->env);
        return EBM_UNDEF;
    }

    EBM_set_cdr(
            prev_cell,
            EBM_CDR(alist),
            gc_interface);
    return EBM_UNDEF;
}

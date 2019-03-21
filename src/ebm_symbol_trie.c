#include "ebm.h"

#define EBM_TRIE_INITIAL_DOUBLE_ARRAY_SIZE 256

#define EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE 7
#define EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE_SIZE 8


uintptr_t EBM_allocate_symbol_trie(EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_record_CA(9,allocator,allocator_env);

    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_SYMBOL_TRIE);
    uintptr_t mem_size = EBM_TRIE_INITIAL_DOUBLE_ARRAY_SIZE;

    EBM_record_primitive_set_CA(res,1,mem_size);
    EBM_record_primitive_set_CA(res,2,sizeof(uint16_t));
    int i,j;
    for (i=3;i<5;i++){
        uint16_t *mem = (uint16_t*)allocator(sizeof(uint16_t) * mem_size,allocator_env);
        EBM_record_primitive_set_CA(res,i,(uintptr_t)mem);
        for (j=0;j<mem_size;j++){
            mem[j] = 0;
        }

        if (i==3){
            mem[0] = 1;
        }else if (i==4){
            mem[1] = 0;
        }
    }

    uintptr_t *mem = (uintptr_t*)allocator(sizeof(uintptr_t) * mem_size,allocator_env);
    EBM_record_primitive_set_CA(res,5,(uintptr_t)mem);
    for (i=0;i<mem_size;i++){
        mem[i] = EBM_UNDEF;
    }

    EBM_record_primitive_set_CA(res,6,EBM_NULL);
    
    EBM_record_primitive_set_CA(res,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE,EBM_char_table_create_CA(128,0,allocator,allocator_env));
    EBM_record_primitive_set_CA(res,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE_SIZE,EBM_allocate_FX_NUMBER_CA(0));
    return res;
}

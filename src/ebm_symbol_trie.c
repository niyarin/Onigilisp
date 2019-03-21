#include "ebm.h"

#define EBM_TRIE_INITIAL_DOUBLE_ARRAY_SIZE 256


#define EBM_SYMBOL_TRIE_INDEX_POSITION_TABLE 3
#define EBM_SYMBOL_TRIE_INDEX_CHECK_TABLE 4
#define EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE 7
#define EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE_SIZE 8


uintptr_t EBM_allocate_symbol_trie(EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_record_CA(9,allocator,allocator_env);

    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_SYMBOL_TRIE);
    uintptr_t mem_size = EBM_TRIE_INITIAL_DOUBLE_ARRAY_SIZE;

    EBM_record_primitive_set_CA(res,1,mem_size);
    EBM_record_primitive_set_CA(res,2,sizeof(uint16_t));
    int i,j,_i;
    int[2] position_and_check_table = {EBM_SYMBOL_TRIE_INDEX_POSITION_TABLE,EBM_SYMBOL_TRIE_INDEX_CHECK_TABLE};
    for (_i=0;_i<2;_i++){
        int i=position_and_check_table[_i];
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


uintptr_t EBM_symbol_trie_ref(uintptr_t trie,uintptr_t symbol){
    uintptr_t mem_size = EBM_record_ref(trie,1);
    uint32_t *symbol_data = (uint32_t*)EBM_record_ref(symbol,2);
    uint16_t *base = (uint16_t*)EBM_record_ref(trie,EBM_SYMBOL_TRIE_INDEX_POSITION_TABLE);
    uint16_t *check = (uint16_t*)EBM_record_ref(trie,EBM_SYMBOL_TRIE_INDEX_CHECK_TABLE);
    uintptr_t *items = (uintptr_t*)EBM_record_ref(trie,5);
    uintptr_t index = 0;
    uintptr_t pos = 1;
    do {
        uintptr_t new_pos = base[pos] + EBM_char_index(symbol_data[index],0);
        if (check[new_pos] != pos){
            return EBM_UNDEF;
        }
        pos = new_pos;
        index++;
    }while (symbol_data[index]);
    
    if (items[pos] == EBM_UNDEF){
        return EBM_UNDEF;
    }else{
        return EBM_CAR(items[pos]);
    }
}

#include "ebm.h"

#define EBM_TRIE_INITIAL_DOUBLE_ARRAY_SIZE 256


#define EBM_SYMBOL_TRIE_INDEX_POSITION_TABLE 3
#define EBM_SYMBOL_TRIE_INDEX_CHECK_TABLE 4
#define EBM_SYMBOL_TRIE_INDEX_ITEMS 5
#define EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE 7
#define EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE_SIZE 8


uintptr_t EBM_allocate_symbol_trie(EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_record_CA(9,allocator,allocator_env);

    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_SYMBOL_TRIE);
    uintptr_t mem_size = EBM_TRIE_INITIAL_DOUBLE_ARRAY_SIZE;

    EBM_record_primitive_set_CA(res,1,mem_size);
    EBM_record_primitive_set_CA(res,2,sizeof(uint16_t));
    int i,j,_i;
    int position_and_check_table[2] = {EBM_SYMBOL_TRIE_INDEX_POSITION_TABLE,EBM_SYMBOL_TRIE_INDEX_CHECK_TABLE};
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
    EBM_record_primitive_set_CA(res,EBM_SYMBOL_TRIE_INDEX_ITEMS,(uintptr_t)mem);
    for (i=0;i<mem_size;i++){
        mem[i] = EBM_UNDEF;
    }

    EBM_record_primitive_set_CA(res,6,EBM_NULL);
    
    EBM_record_primitive_set_CA(res,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE,EBM_char_table_create_CA(128,0,allocator,allocator_env));
    EBM_record_primitive_set_CA(res,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE_SIZE,EBM_allocate_FX_NUMBER_CA(129));
    return res;
}

uintptr_t EBM_trie_char_index(uintptr_t index,uintptr_t character_table){
    if (index < 128){
        return index+1;
    }
    
    uintptr_t fx_index_apair = EBM_char_table_ref_CA(character_table,index);
    if (fx_index_apair){
       return EBM_CDR(fx_index_apair);
    }
    return 0;
}

static uintptr_t EBM_trie_char_index_and_insert(uintptr_t index,uintptr_t character_table,uintptr_t symbol_trie,EBM_GC_INTERFACE *gc_interface){
    if (index < 128){
        return index+1;
    }
    
    uintptr_t fx_index_apair = EBM_char_table_ref_CA(character_table,index);
    if (fx_index_apair){
       return EBM_CDR(fx_index_apair);
    }

    uintptr_t large_character_table_FX_size = EBM_record_ref_CA(symbol_trie,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE_SIZE);

    EBM_char_table_set_CA( character_table, EBM_FX_NUMBER_TO_C_INTEGER_CR(large_character_table_FX_size), EBM_allocate_FX_NUMBER_CA(large_character_table_FX_size), gc_interface);

    EBM_char_table_set_CA( character_table,index, EBM_FX_ADD(large_character_table_FX_size,EBM_allocate_FX_NUMBER_CA(1)),gc_interface);
    return EBM_FX_NUMBER_TO_C_INTEGER_CR(large_character_table_FX_size);
}

uintptr_t EBM_symbol_trie_ref(uintptr_t trie,uintptr_t symbol){
    uintptr_t mem_size = EBM_record_ref_CA(trie,1);
    uint32_t *symbol_data = (uint32_t*)EBM_record_ref_CA(symbol,2);
    uint16_t *base = (uint16_t*)EBM_record_ref_CA(trie,EBM_SYMBOL_TRIE_INDEX_POSITION_TABLE);
    uint16_t *check = (uint16_t*)EBM_record_ref_CA(trie,EBM_SYMBOL_TRIE_INDEX_CHECK_TABLE);
    uintptr_t *items = (uintptr_t*)EBM_record_ref_CA(trie,EBM_SYMBOL_TRIE_INDEX_ITEMS);
    uintptr_t index = 0;
    uintptr_t pos = 1;

    uintptr_t char_table = EBM_record_ref_CA(trie,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE);

    do {
        uintptr_t symbol_index = EBM_trie_char_index(symbol_data[index],char_table);
        if (symbol_index == 0){
            return EBM_UNDEF;
        }
        
        uintptr_t new_pos = base[pos] + symbol_index;
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

uintptr_t EBM_symbol_trie_set(uintptr_t trie,uintptr_t symbol,uintptr_t object,EBM_GC_INTERFACE *gc_interface){
    uintptr_t mem_size = EBM_record_ref_CA(trie,1);
    uint16_t *base = (uint16_t*)EBM_record_ref_CA(trie,3);
    uint16_t *check = (uint16_t*)EBM_record_ref_CA(trie,4);
    uintptr_t *items = (uintptr_t*)EBM_record_ref_CA(trie,5);
    uintptr_t char_table = EBM_record_ref_CA(trie,EBM_SYMBOL_TRIE_INDEX_LARGE_INDEX_TABLE);

    uint32_t *symbol_data = (uint32_t*)EBM_record_ref_CA(symbol,2);

    uintptr_t index = 0;
    uintptr_t pos = 1;
    uintptr_t already_inserted = 1;

    do{
        uintptr_t new_pos = base[pos] + EBM_trie_char_index_and_insert(symbol_data[index],char_table,trie,gc_interface);

        if (new_pos >= mem_size){
            //OVER FLOW
            exit(1);
        }

        if (check[new_pos] == pos){
            //すでに登録済み
            pos = new_pos;
        }else{
            already_inserted = 0;
            if (check[new_pos] == 0){
                //new_posの先が未使用
                check[new_pos] = (uint16_t)pos;
                
                int base_number = 1;
                uintptr_t next_char_index = EBM_trie_char_index_and_insert(symbol_data[index+1],char_table,trie,gc_interface);//MEMO:次が末尾だと?

                //search free index..
                while (check[base_number + next_char_index] ){
                    base_number++;
                    if (base_number ==  mem_size){
                        //TODO:
                        exit(1);
                    }
                }
                
                base[new_pos] = base_number;
                pos = new_pos;
            }else{//conflict
                int i;
                uintptr_t next_char_index = EBM_trie_char_index_and_insert(symbol_data[index+1],char_table,trie,gc_interface);

                uintptr_t current_char_index = EBM_trie_char_index_and_insert(symbol_data[index],char_table,trie,gc_interface);

                //posから移動できる位置の集合を得る
                uintptr_t same_bases = EBM_NULL;
                for (i=mem_size-1;i>-1;i--){
                    if (check[i] == pos){
                        same_bases = EBM_allocate_pair(i,same_bases,gc_interface->allocator,gc_interface->env);//TODO:pair in c number
                    }
                }

                //新しいbase[pos]を探す
                char flag1 = 0;
                uintptr_t new_base = 0;
                for (new_base=1;new_base<mem_size;new_base++){
                    uintptr_t cell = same_bases;
                    char flag2 = 1;
                    while (cell != EBM_NULL){
                        if (check[EBM_CAR(cell) - base[pos] + new_base]){
                            flag2 = 0;
                            break;
                        }
                        cell = EBM_CDR(cell);
                    }

                    if (flag2){
                        if (check[new_base + current_char_index]){
                            flag2 = 0;
                        } 
                    }


                    if (flag2){//base[pos] = new_baseにできる
                        uintptr_t original_base_pos = base[pos];
                        base[pos] = new_base;
                        cell = same_bases;
                        while (cell != EBM_NULL){
                            uintptr_t org_index = EBM_CAR(cell);
                            uintptr_t new_index = org_index - original_base_pos + new_base; 
                            check[new_index] = pos;
                            base[new_index] = base[org_index];
                            items[new_index] = items[org_index];
                            items[org_index] = EBM_NULL;//?
                            //整合性
                            int j;
                            for (j=0;j<mem_size;j++){
                                if (check[j] == org_index){
                                    check[j] = new_index;
                                }
                            }
                            
                            {//free old index
                                check[org_index] = 0;
                                base[org_index] = 0;
                            }

                            cell = EBM_CDR(cell);
                        }
                        {
                            new_pos = new_base + current_char_index;
                            check[new_pos] = pos;
                        }

                        
                        {
                            int base_number = 1;
                            //search free index..
                            while (check[base_number + next_char_index]){
                                base_number++;
                                if (base_number ==  mem_size){
                                    //TODO:
                                    exit(1);
                                }
                            }
                            base[new_pos] = base_number;
                        }
                        pos = new_pos;
                        flag1 = 1;
                        break;
                    }
                }
                if (!flag1){//入れられる場所がなかった
                    exit(1);
                    //TODO:実装しよう
                }

            }
        }

        index++;
    }while (symbol_data[index]);

    if (!already_inserted || items[pos] == EBM_UNDEF){
        uintptr_t item_pair = EBM_allocate_pair(symbol,EBM_record_ref_CA(trie,6),gc_interface->allocator,gc_interface->env);
        EBM_record_set_CA(trie,6,item_pair,gc_interface->write_barrier,gc_interface->env);
        //TODO:item[pos]にひとつしかいれられないのはおかしいので直す?
        items[pos] = EBM_allocate_pair(object,
                                       EBM_allocate_pair(
                                           item_pair,
                                           pos<<3,
                                           gc_interface->allocator,
                                           gc_interface->env),
                                       gc_interface->allocator,
                                       gc_interface->env);
    }else{
        EBM_set_car(items[pos],
                    object,
                    gc_interface);
    }
}

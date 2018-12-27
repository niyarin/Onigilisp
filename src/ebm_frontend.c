#include "ebm.h"
#include "ebm_frontend.h"
#include "olisp_cinterface.h"

typedef struct {
   uintptr_t line_manager;
   uintptr_t read_function_table;
   uintptr_t dispatch_table;
}OLISP_reader_config;



uintptr_t EBM_frontend_allocate_input_file_port(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(5,allocator,env);
    EBM_record_primitive_set_CA(res, 0,
                   EBM_BUILT_IN_RECORD_TYPE_PORT );
    
    EBM_record_primitive_set_CA(res,
                   1,
                   EBM_FRONTEND_INPUT_PORT);

    EBM_record_primitive_set_CA(res,
                   2,
                   EBM_FRONTEND_FILE_PORT);
    EBM_record_primitive_set_CA(res,
                   3,
                  (uintptr_t)fp);
    return res;
}

uintptr_t EBM_frontend_allocate_input_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(5,allocator,env);
    EBM_record_primitive_set_CA(res,
                   0,
                   EBM_BUILT_IN_RECORD_TYPE_PORT );
    
    EBM_record_primitive_set_CA(res,
                   EBM_FRONTEND_INPUT_PORT,
                   0);


    EBM_record_primitive_set_CA(res,
                   2,
                   EBM_FRONTEND_STRING_PORT);
    EBM_record_primitive_set_CA(res,
                   3,
                  ebm_string);

    EBM_record_primitive_set_CA(res,
                   4,
                   0);
    return res;
}

uintptr_t EBM_frontend_allocate_output_file_port_CA(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(5,allocator,env);
    EBM_record_primitive_set_CA(res, 0,
                   EBM_BUILT_IN_RECORD_TYPE_PORT );
    
    EBM_record_primitive_set_CA(res,
                   1,
                   EBM_FRONTEND_OUTPUT_PORT);

    EBM_record_primitive_set_CA(res,
                   2,
                   EBM_FRONTEND_FILE_PORT);
    EBM_record_primitive_set_CA(res,
                   3,
                  (uintptr_t)fp);
    return res;
}


uintptr_t EBM_frontend_allocate_output_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(5,allocator,env);
    EBM_record_primitive_set_CA(res,
                   0,
                   EBM_BUILT_IN_RECORD_TYPE_PORT );
    
    EBM_record_primitive_set_CA(res,
                   EBM_FRONTEND_OUTPUT_PORT,
                   0);


    EBM_record_primitive_set_CA(res,
                   2,
                   EBM_FRONTEND_STRING_PORT);
    EBM_record_primitive_set_CA(res,
                   3,
                  ebm_string);

    EBM_record_primitive_set_CA(res,
                   4,
                   0);
    return res;
}


//
//      READ FUNCTIONS
//

void EBM_u32_to_u8(uint32_t c,unsigned char *res){
    if (c < 128){
        res[0] = c;
        res[1] = 0;
    }else if (c <= 2047){//11bit
        res[0] = (c >>6) + 192;//192=11000000
        res[1] = (c & 63) + 128;//128 = 10000000
        res[2] = 0;
    }else if (c <= 65535){//16bit
        res[0] = (c >>12) + 224;//224=11100000
        res[1] = ((c & 4032)>>6) + 128;
        res[2] = (c & 63) + 128;
        res[3] = 0;
    }else{
        res[0] = (c >>18) + 240;//240=11110000
        res[1] = ((c & 258048)>>12) + 128;
        res[2] = ((c & 4032)>>6) + 128;
        res[3] = (c & 63) + 128;
        res[4] = 0;
    }
    //ERROR
}

//Schemeでは使わないからstaticで良い。(あと、2文字以上のungetcは本当はダメ)
void EBM_ungetc(uint32_t c,uintptr_t port){
    if (EBM_IS_INPUT_PORT_CR(port)){
        if (EBM_IS_FILE_PORT_CR(port)){

            char tmpc[5];//unsignedでなくて良い?
            EBM_u32_to_u8(c,tmpc);
            int i=0;
            while (tmpc[i]){
                i++;
            }
            FILE *fp = (FILE*)EBM_record_ref_CA(port,3);
            while (i>0){
                i--;
                ungetc(tmpc[i],fp); 
            }
        }
    }else{
    
        
    }
}



static uint32_t fread_char_utf8(FILE* fp){
    //冗長なutf8コードを潰す必要性はなさそう。
    unsigned char t1,t2,t3,t4;
    if (!fp){
        //ERROR
        return EOF;
    }
    
    int num_of_read;
    num_of_read = fread(&t1,1,1,fp);
    if (num_of_read < 1){
        //ERROR
        return 0;
    }

    if (t1 < 128){
        //ascii
        return (uint32_t)t1;
    }

    num_of_read = fread(&t2,1,1,fp);
    if (num_of_read < 1 ||
        (t2 & 0b11000000) != 0b10000000){
        //ERROR
        return 0;
    }

    if ((t1 & 0b11100000) == 0b11000000){
        return ((t1&0b111111)<<6) + (t2&0b111111);//1bit多い?(検証後直す)
    }
    

    num_of_read = fread(&t3,1,1,fp);
    if (num_of_read < 1 ||
        (t3 & 0b11000000) != 0b10000000){
        //ERROR
        return 0;
    }
    
    if ((t1 & 0b11110000) == 0b11100000){
        return ((t1&0b1111)<<12) +
               ((t2&0b111111)<<6) + 
               (t3&0b111111);
    }


   num_of_read = fread(&t4,1,1,fp);
   if (num_of_read < 1 ||
        (t4 & 0b11000000) != 0b10000000){
        //ERROR
        return 0;
    }

   if ((t1 & 0b11111000) == 0b11110000){
         return ((t1&0b111)<<18) +
               ((t2&0b111111)<<12) + 
               ((t3&0b111111)<<6) +
               (t4&0b111111);
   }
   //ERROR
   return 0;
}

uint32_t EBM_read_char_CR(uintptr_t port){
    if (EBM_IS_INPUT_PORT_CR(port)){
        if (EBM_IS_FILE_PORT_CR(port)){
            FILE *fp = (FILE*)EBM_record_ref_CA(port,3); 
            uint32_t c =  fread_char_utf8(fp);
            return c;
        }else if (EBM_IS_STRING_PORT_CR(port)){
            /*
            uintptr_t index = EBM_record_ref(port,4);
            uintptr_t ebm_string = EBM_record_ref(port,3);
            if (index < EBM_string_length_CR(ebm_string)){
                uint32_t c = EBM_C_string_ref(ebm_string,index);
                EBM_record_set(port,4,index + 1);
                return c;
            }else{
                return EOF;
            }
            */
        }
    }
}


uint32_t EBM_peek_char(uintptr_t port){
    uint32_t res = EBM_read_char_CR(port);
    if (res != EOF){
        EBM_ungetc(res,port);
    }
    return res;
}

uintptr_t OLISP_read1_with_character_table(OLISP_state *state);

uintptr_t OLISP_read_function_read_list(OLISP_state *state){
    printf("::::::::::::::::::::[read list]\n");
    if (state->arg_size == 3){
        uintptr_t port = state->args1[0];
        uintptr_t target_ebm_c = state->args1[1];
        uintptr_t reader_config_ptr = state->args1[2];
        OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));
        uintptr_t res_top_cell = EBM_allocate_pair(EBM_NULL,EBM_NULL,state->allocator,state->allocator_env);
        uintptr_t cell = res_top_cell;

        while (1){   
            uintptr_t lobject = OLISP_cfun_call(state,OLISP_read1_with_character_table,4,port,reader_config->read_function_table,reader_config_ptr,EBM_allocate_character_CA(')'));
            if (lobject == EBM_DUMMY_OBJECT){
                break;
            }
            EBM_PRIMITIVE_SET_CDR(cell,EBM_allocate_pair(lobject,EBM_NULL,state->allocator,state->allocator_env));
            cell = EBM_CDR(cell);
        }
        return EBM_CDR(res_top_cell);
    }
    exit(1);
}


uintptr_t OLISP_read_function_read_rparen(OLISP_state *state){
    printf("::::::::::::::::::::::::::::::::::::[RPAREN]\n");
    exit(1);
    return EBM_NULL;
}


uintptr_t OLISP_read_function_read_space(OLISP_state *state){
    if (state->arg_size == 3){
        printf("::::::::::::::::::::::::::::::::::::[SP]\n");
        uintptr_t port = state->args1[0];
        uintptr_t target_ebm_c = state->args1[1];
        uintptr_t reader_config_ptr = state->args1[2];
        OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));
        return OLISP_cfun_call(state,OLISP_read1_with_character_table,3,port,reader_config->read_function_table,reader_config_ptr);
    }
}

uintptr_t OLISP_read_function_read_newline(OLISP_state *state){//TODO:あとで行番号の追加
    if (state->arg_size == 3){
        printf("::::::::::::::::::::::::::::::::::::[NEWLINE]\n");
        uintptr_t port = state->args1[0];
        uintptr_t target_ebm_c = state->args1[1];
        uintptr_t reader_config_ptr = state->args1[2];
        OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));
        return OLISP_cfun_call(state,OLISP_read1_with_character_table,3,port,reader_config->read_function_table,reader_config_ptr);
    }
}


uintptr_t OLISP_read_function_read_quote(OLISP_state *state){
    printf("::::::::::::::::::::::::::::::::::::[QUOTE]\n");
    uintptr_t port = state->args1[0];
    uintptr_t target_ebm_c = state->args1[1];
    uintptr_t reader_config_ptr = state->args1[2];
    OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));

    uintptr_t object = OLISP_cfun_call(state,OLISP_read1_with_character_table,3,port,reader_config->read_function_table,reader_config_ptr);
    

    uintptr_t res = EBM_allocate_pair(EBM_allocate_symbol_from_cstring_CA("quote",state->allocator,state->allocator_env),
                                      EBM_allocate_pair(object,EBM_NULL,state->allocator,state->allocator_env),
                                      state->allocator,state->allocator_env);
    return res;
}

uintptr_t OLISP_read_function_read_dispatch_pattern(OLISP_state *state){
    printf("DISPATCH\n");
    uintptr_t port = state->args1[0];
    uintptr_t cc = EBM_char2unicode_CR(state->args1[1]);
    uintptr_t reader_config_ptr = state->args1[2];
    OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));
    
    uintptr_t dispatch_table_apair = EBM_char_table_ref_CA(reader_config->dispatch_table,cc);

    if (dispatch_table_apair != EBM_FALSE){
        uintptr_t secound_dispatch_table = EBM_CDR(dispatch_table_apair);

        uint32_t cc2 = EBM_peek_char(port);
        uintptr_t read_function_apair = EBM_char_table_ref_CA(secound_dispatch_table,cc2);
        if (read_function_apair != EBM_FALSE){
             uintptr_t read_function = EBM_CDR(read_function_apair);
            return OLISP_cfun_call(state,OLISP_fun_call,5,read_function,port,EBM_allocate_character_CA(cc),EBM_allocate_character_CA(cc2),reader_config_ptr);
        }
    }
    exit(1);
}


uintptr_t OLISP_read_dispatch_function_read_boolean(OLISP_state *state){
    uintptr_t port = state->args1[0];
    uintptr_t cc1 =  EBM_char2unicode_CR(state->args1[1]);
    uintptr_t cc2 =  EBM_char2unicode_CR(state->args1[2]);
    
    uintptr_t reader_config_ptr = state->args1[3];
    if (cc2 == 't'){
 
        printf("::::::::::::::::::::[read true]\n");
        OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));
        uintptr_t maybe_true = OLISP_cfun_call(state,OLISP_read1_with_character_table,3,port,reader_config->read_function_table,reader_config_ptr);
        //maybe_true must be t or true symbol.

        //TODO:check

        
        return EBM_TRUE;
    }else if (cc2 == 'f'){
        OLISP_reader_config *reader_config = (OLISP_reader_config*)(EBM_pointer_box_ref_CR(reader_config_ptr));
        uintptr_t maybe_false = OLISP_cfun_call(state,OLISP_read1_with_character_table,3,port,reader_config->read_function_table,reader_config_ptr);  
        printf("::::::::::::::::::::[read false]\n");
        return EBM_FALSE;
    }
    exit(1);
}


uintptr_t EBM_frontend_create_default_reader_table(EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    //TODO:あとでprimitiveを取る=>大きさ128だし問題なさそう。
    uintptr_t res = EBM_char_table_create_CA(128,0,allocator,allocator_env);
    EBM_char_table_primitive_insert_CA(res,'(',OLISP_create_function_for_ebm(OLISP_read_function_read_list,allocator,allocator_env),allocator,allocator_env);

    EBM_char_table_primitive_insert_CA(res,')',OLISP_create_function_for_ebm(OLISP_read_function_read_rparen,allocator,allocator_env),allocator,allocator_env);

    EBM_char_table_primitive_insert_CA(res,' ',OLISP_create_function_for_ebm(OLISP_read_function_read_space,allocator,allocator_env),allocator,allocator_env);

    EBM_char_table_primitive_insert_CA(res,'\n',OLISP_create_function_for_ebm(OLISP_read_function_read_newline,allocator,allocator_env),allocator,allocator_env);

    EBM_char_table_primitive_insert_CA(res,'#',OLISP_create_function_for_ebm(OLISP_read_function_read_dispatch_pattern,allocator,allocator_env),allocator,allocator_env);


    EBM_char_table_primitive_insert_CA(res,'\'',OLISP_create_function_for_ebm(OLISP_read_function_read_quote,allocator,allocator_env),allocator,allocator_env);
    return res;
}

uintptr_t EBM_frontend_create_default_dispatch_table(EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    //TODO:あとでprimitiveを取る
    uintptr_t res = EBM_char_table_create_CA(8,0,allocator,allocator_env);
    
    uintptr_t sharp_dispatcher = EBM_char_table_create_CA(16,0,allocator,allocator_env);
    EBM_char_table_primitive_insert_CA(res,'#',sharp_dispatcher,allocator,allocator_env);
    {
        EBM_char_table_primitive_insert_CA(sharp_dispatcher,'t',OLISP_create_function_for_ebm( OLISP_read_dispatch_function_read_boolean,allocator,allocator_env),allocator,allocator_env);
        EBM_char_table_primitive_insert_CA(sharp_dispatcher,'f',OLISP_create_function_for_ebm( OLISP_read_dispatch_function_read_boolean,allocator,allocator_env),allocator,allocator_env);
    }
    return res;
}



static uintptr_t _EBM_analyze_token(uint32_t *token_string,size_t length,size_t max_length,OLISP_state *state){
    if (length == max_length){
        token_string = (uint32_t*)realloc(token_string,sizeof(uint32_t) * (max_length + 1));
    }
    token_string[length]  = 0;
    
    
    EBM_ALLOCATOR allocator = state->allocator;
    uintptr_t allocator_env = state->allocator_env;
    return EBM_allocate_symbol_CA(token_string,allocator,allocator_env);
}


uintptr_t OLISP_read1_with_character_table(OLISP_state *state){
    if (state->arg_size >= 3){
        uintptr_t port = state->args1[0];
        uintptr_t character_table = state->args1[1];

        uint32_t delim_cc= 0;
        if (state->arg_size >= 4){
            delim_cc = EBM_char2unicode_CR(state->args1[3]);
        }
    
        
        size_t token_max_length = 8;
        uint32_t *token_string = (uint32_t*)malloc(sizeof(uint32_t)*token_max_length);

        size_t token_length = 0;
        while (1){
            uint32_t cc = EBM_read_char_CR(port);
            printf("%c\n",cc);
            uintptr_t read_function_apair = EBM_char_table_ref_CA(character_table,cc);

            if (read_function_apair != EBM_FALSE){
                uintptr_t config_ptr = state->args1[2];
                if (token_length != 0){
                    EBM_ungetc(cc,port);
                    return _EBM_analyze_token(token_string,token_length,token_max_length,state);
                }

                if (delim_cc){
                    if (delim_cc == cc){
                        printf(":::::::::::::::::::::::::[CLOSE PAREN]\n");
                        return EBM_DUMMY_OBJECT;
                    }
                }
                uintptr_t read_function = EBM_CDR(read_function_apair);
                return OLISP_cfun_call(state,OLISP_fun_call,4,read_function,port, EBM_allocate_character_CA(cc),config_ptr);
            }
            break;
        }
    }
}

uintptr_t OLISP_read(OLISP_state *state){
    uintptr_t port;
    uintptr_t config_ptr = EBM_NULL;
    uintptr_t read_table;
    uintptr_t dispatch_table;
    int pass_config_flag = 0;

    if (state->arg_size >= 1){
        port = state->args1[0];
        if (state->arg_size >= 2){
            pass_config_flag = 1;
            config_ptr = state->args1[1];
            read_table = ((OLISP_reader_config*)(EBM_pointer_box_ref_CR(config_ptr)))->read_function_table;
        }
    }else if (state->arg_size == 0){
        port = EBM_frontend_allocate_input_file_port(stdout,state->allocator,state->allocator_env);
    }


    if (!pass_config_flag){
        OLISP_reader_config reader_config;
        read_table = EBM_frontend_create_default_reader_table(state->allocator,state->allocator_env);
        dispatch_table =  EBM_frontend_create_default_dispatch_table(state->allocator,state->allocator_env);

        reader_config.read_function_table = read_table;
        reader_config.dispatch_table = dispatch_table;
        config_ptr = EBM_allocate_pointer_box_CA((uintptr_t)&reader_config,state->allocator,state->allocator_env);
    }
    uintptr_t res = OLISP_cfun_call(state,OLISP_read1_with_character_table,3,port,read_table,config_ptr);
    return res;
}


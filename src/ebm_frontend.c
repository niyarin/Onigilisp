#include "ebm.h"
#include "ebm_frontend.h"
#include "olisp_cinterface.h"

typedef struct {
   EBM_ALLOCATOR allocator;
   uintptr_t allocator_env;
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


//
//      READ FUNCTIONS
//

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

uintptr_t EBM_frontend_create_default_reader_table(EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    /*
    uintptr_t res = EBM_char_table_create(128,0,allocator,allocator_env);
    return res;
    */
}


uintptr_t OLISP_read1_with_character_table(OLISP_state *state){
    
}


uintptr_t EBM_read(uintptr_t port){
}






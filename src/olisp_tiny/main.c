#include<stdio.h>
#include<stdlib.h>

#include "ebm.h"
#include "ebm_frontend.h"
#include "olisp_cinterface.h"

void main_load_1script(char *filename){
    FILE *fp;
    if ((fp = fopen(filename,"r")) == NULL){
        uintptr_t error_port =  EBM_frontend_allocate_output_file_port_CA(stderr,EBM_malloc_wrapper,EBM_NULL);
        
        EBM_write_cstring_CA("\033[31mError\033[39m",error_port);
        EBM_write_cstring_CA(": No such file or directory. -- ",error_port);
        EBM_write_cstring_CA(filename,error_port);
        EBM_newline(error_port);

        exit(1);
    }

    EBM_ALLOCATOR allocator = EBM_malloc_wrapper;
    uintptr_t allocator_env = 0;
    uintptr_t port = EBM_frontend_allocate_input_file_port(fp,allocator,allocator_env);

    OLISP_state olisp_state;
    olisp_state.allocator = allocator;olisp_state.allocator_env = allocator_env;

    uintptr_t res = OLISP_cfun_call(&olisp_state,OLISP_read,1,port);
}

int main(int argc,char **argv){
    if (argc == 1){
    
    }else if (argc == 2){
        char *filename = argv[1];
        main_load_1script(filename);
    }
    return 0;
}

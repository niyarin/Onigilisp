#include<stdio.h>
#include<stdlib.h>

#include "ebm.h"
#include "ebm_frontend.h"
#include "olisp_cinterface.h"

void main_load_1script(char *filename){
    FILE *fp;
    if ((fp = fopen(filename,"r")) == NULL){
        //TODO:OUTPUT ERROR MESSAGE
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

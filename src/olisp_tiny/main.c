#include<stdio.h>
#include<stdlib.h>

#include "ebm.h"
#include "ebm_frontend.h"

void main_load_1script(char *filename){
    FILE *fp;
    if ((fp = fopen(filename,"r")) == NULL){
        //TODO:OUTPUT ERROR MESSAGE
        exit(1);
    }
    EBM_ALLOCATOR allocator = EBM_malloc_wrapper;
    uintptr_t allocator_env = 0;
    uintptr_t port = EBM_frontend_allocate_input_file_port(fp,allocator,allocator_env);
}

int main(int argc,char **argv){
    if (argc == 1){
    
    }else if (argc == 2){
        char *filename = argv[1];
        main_load_1script(filename);
    }
    return 0;
}

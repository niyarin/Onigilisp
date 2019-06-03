#include "ebm.h"
#include "olisp_cinterface.h"

//とりあえずx86 linux 対応

//stateの第一引数にpointerbox(code)
//2引数にjit info


#include<sys/mman.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>//あとで消したい

uintptr_t OLISP_jit_from_byte_vector(OLISP_state *state){

    uintptr_t byte_vector = 
        state->args1[0];

    unsigned char *original_code = EBM_record_ref_CA(byte_vector,1);
    uintptr_t original_code_length = EBM_record_ref_CA(byte_vector,2);
    unsigned char *code;

    //32bitでも64bit アライメントする(手抜き)

    int advance_flag = 0;
    if (original_code_length & 63){
        advance_flag = 1;
    }

    uintptr_t code_length = (original_code_length>>EBM_CONFIG_POINTER_INFORMATION_SIZE  + advance_flag)*64;

    if (posix_memalign((void**)&code,sysconf(_SC_PAGESIZE),sizeof(char) * code_length)){
        fprintf(stderr,"ERROR MEMALIGN\n");
        exit(1);
    }

    if (mprotect((void*)code,sizeof(char) * code_length,PROT_READ | PROT_WRITE | PROT_EXEC)){
       fprintf(stderr,"ERROR MEMPROTECT \n");
        exit(1);
    }

    int i;
    for (i=0;i<code_length;i++){
        code[i] = 0;
    }

    for (i=0;i<EBM_record_ref_CA(byte_vector,2);i++){
        code[i] = original_code[i];
    }

    return 
        EBM_allocate_pointer_box_CA(
                code,
                state->allocator,
                state->allocator_env);
}

static uintptr_t create_jit_info(uintptr_t *mem){
    mem[0] = malloc(128*sizeof(uintptr_t));//locals
    mem[1] = malloc(128*sizeof(uintptr_t));//heap chank
    {
        ((uintptr_t**)mem[1])[0] = 2;
        ((uintptr_t**)mem[1])[1] = 128/sizeof(uintptr_t);
    }

    mem[2] = mem[0] + 128*sizeof(uintptr_t);//heap e
    mem[3] = 0;//write info
    return mem;
}

static uintptr_t delete_jit_info(uintptr_t *mem){
    return 0;
}

typedef uintptr_t (*simple_fun)(uintptr_t,uintptr_t);

uintptr_t OLISP_jit_run(OLISP_state *state){
    //TODO:1引数にしか対応してない
    simple_fun sfun = EBM_pointer_box_ref_CR(state->args1[0]);
    //return sfun(state->args1[1]);

    uintptr_t jit_info[8];
    create_jit_info(jit_info);

    uintptr_t res = sfun(jit_info,state->args1[1]);
    delete_jit_info(jit_info);
    return res;
}

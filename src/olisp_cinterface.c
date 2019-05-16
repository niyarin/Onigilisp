#include "olisp_cinterface.h"
#include "ebm.h"
#include<stdio.h>
#include<stdlib.h>

void OLISP_set_arg(OLISP_state *state,uintptr_t pos,uintptr_t arg){
    if (pos < OLISP_ARG1_SIZE){
        state->args1[pos] = arg;
    }else{
        if (EBM_vector_length_CR(state->args2) > pos-OLISP_ARG1_SIZE){
            state->args2 = EBM_vector_re_allocate_CA(state->args2,pos,EBM_NULL,state->allocator,state->allocator_env);
        }
        EBM_vector_primitive_set_CA(state->args2,pos-OLISP_ARG1_SIZE,arg);
    }
}

void OLISP_set_arg_size(OLISP_state *state,uintptr_t size){
    state->arg_size = size;
}

uintptr_t OLISP_get_arg(OLISP_state *state,uintptr_t pos){
    if (pos < OLISP_ARG1_SIZE){
        return state->args1[pos];
    }else{
        return EBM_vector_ref_CA(state->args2,pos-OLISP_ARG1_SIZE);
    }   
}

uintptr_t OLISP_cfun_call(OLISP_state *state,OLISP_cfun cfun,uintptr_t arg_size,...){
    va_list argp;
    va_start(argp,arg_size);

    OLISP_set_arg_size(state,arg_size);
    int i;
    for (i=0;i<arg_size;i++){
        OLISP_set_arg(state,i,va_arg(argp,uintptr_t));
    }
    
    return cfun(state);
}

uintptr_t OLISP_create_function_for_ebm(OLISP_cfun cfun,EBM_ALLOCATOR  allocator,uintptr_t allocator_env){
    uintptr_t res = OLISP_create_function_for_ebm_with_name(
                        cfun,
                        EBM_NULL,
                        allocator,
                        allocator_env);
    return res;
}

uintptr_t OLISP_create_function_for_ebm_with_name(OLISP_cfun cfun,uintptr_t name_symbol,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_record_CA(4,allocator,allocator_env);
    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_OLISP_FUNCTION);
    EBM_record_primitive_set_CA(res,1,0);//OLISP fun は常に0
    EBM_record_primitive_set_CA(res,2,(uintptr_t)cfun);
    EBM_record_primitive_set_CA(res,3,name_symbol);
    return res;
}

uintptr_t OLISP_fun_call(OLISP_state *state){
    if (state->arg_size>=1){
        uintptr_t fun_object = state->args1[0];
        if (EBM_record_ref_CA(fun_object,1) == 0){
            int i;
            for (i=1;i<state->arg_size;i++){
                state->args1[i-1] = state->args1[i];
            }
            state->arg_size-=1;
            OLISP_cfun cfun = (OLISP_cfun)EBM_record_ref_CA(fun_object,2);
            return cfun(state);
        }
    }
    //TODO:未実装
}

uintptr_t OLISP_error(OLISP_state *state){
    if (state->error_stack == EBM_NULL){
        exit(1);
    }
    uintptr_t jbuf_ptr_box = EBM_CAR(state->error_stack);
    jmp_buf *jbuf = (jmp_buf*)EBM_pointer_box_ref_CR(jbuf_ptr_box);
    longjmp(*jbuf,1);   
    state->error_stack = EBM_CDR(state->error_stack);
    return EBM_UNDEF;
}

uintptr_t OLISP_simple_error(char *message,OLISP_state *state){
    uint32_t *u32_string = state->allocator((strlen(message)+1) * sizeof(uint32_t),state->allocator_env);
    int i=0;
    while (message[i]){
        u32_string[i] = (uint32_t)message[i];
        i++;
    }

    uintptr_t error_message_object = EBM_allocate_string_CA(u32_string, state->gc_interface);
    
    //TODO:free u32_string

    OLISP_cfun_call(state, OLISP_error,1,error_message_object);
}

uintptr_t OLISP_symbol_intern(uintptr_t symbol,OLISP_state *state){
    uintptr_t trie = state->symbol_intern;
    uintptr_t res = EBM_symbol_trie_ref(trie,symbol);
    if (res == EBM_UNDEF){
        EBM_symbol_trie_set(trie,symbol,symbol,state->gc_interface);   
        return symbol;
    }
    return res;
}

uintptr_t OLISP_AUX_set_exception_handler_CA(jmp_buf *jbuf,OLISP_state *state){
    uintptr_t jbuf_ptr_box = 
        EBM_allocate_pointer_box_CA(
                jbuf,
                state->allocator,
                state->allocator_env);

    state->error_stack = 
        EBM_allocate_pair(
                jbuf_ptr_box,
                state->error_stack,
                state->allocator,
                state->allocator_env);

    return EBM_UNDEF;
}

uintptr_t OLISP_AUX_pop_exception_handler(OLISP_state *state){
    state->error_stack= EBM_CDR(state->error_stack);
    return EBM_UNDEF;
}

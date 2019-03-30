#include "olisp_cinterface.h"
#include "ebm.h"

uintptr_t OLISP_read_function_car(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 car)\n");
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required car)\n");
        }
    }
    uintptr_t pair = state->args1[0];
    return EBM_CAR(pair);
}

uintptr_t OLISP_read_function_cdr(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 cdr)\n");
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required cdr)\n");
        }
    }
    uintptr_t pair = state->args1[0];
    return EBM_CDR(pair);
}

uintptr_t OLISP_read_function_cons(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 cons)\n");
        }
    }

    uintptr_t pair = EBM_allocate_pair(state->args1[0],state->args1[1],state->allocator,state->allocator_env);
    return pair;
}

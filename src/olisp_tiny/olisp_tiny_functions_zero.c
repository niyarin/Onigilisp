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



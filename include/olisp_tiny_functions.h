#ifndef OLISP_TINY_FUNCTIONS_H
#define OLISP_TINY_FUNCTIONS_H
#include "olisp_cinterface.h"

uintptr_t OLISP_read_function_car(OLISP_state *state);
uintptr_t OLISP_read_function_cdr(OLISP_state *state);
uintptr_t OLISP_read_function_cons(OLISP_state *state);
#endif

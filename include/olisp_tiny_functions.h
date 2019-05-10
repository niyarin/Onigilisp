#ifndef OLISP_TINY_FUNCTIONS_H
#define OLISP_TINY_FUNCTIONS_H
#include "olisp_cinterface.h"

uintptr_t OLISP_car(OLISP_state *state);
uintptr_t OLISP_cdr(OLISP_state *state);
uintptr_t OLISP_cons(OLISP_state *state);
uintptr_t OLISP_eq(OLISP_state *state);
uintptr_t OLISP_write_simple(OLISP_state *state);
uintptr_t OLISP_vector(OLISP_state *state);
uintptr_t OLISP_pair_p(OLISP_state *state);
uintptr_t OLISP_symbol_p(OLISP_state *state);

//
uintptr_t OLISP_fx_add(OLISP_state *state);


//
uintptr_t OLISP_record_ref(OLISP_state *state);
uintptr_t OLISP_record_p(OLISP_state *state);
uintptr_t OLISP_make_record(OLISP_state *state);

#endif

#ifndef OLISP_TINY_EVAL_SIMPLE_H
#define OLISP_TINY_EVAL_SIMPLE_H
#include "ebm.h"
#include "olisp_cinterface.h"

uintptr_t EBM_allocate_olisp_tiny_eval_simple_environment(EBM_GC_INTERFACE *gc_interface,uintptr_t offset_box,uintptr_t global_box,OLISP_state *state);

uintptr_t EBM_olisp_tiny_set_library0(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state);

uintptr_t EBM_olisp_tiny_expand_simple(uintptr_t expression,uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_interface);
uintptr_t EBM_olisp_eval_simple(uintptr_t expanded_expression,uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_interface);
#endif

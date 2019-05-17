#ifndef OLISP_JIT_H
#define OLISP_JIT_H


uintptr_t OLISP_jit_from_byte_vector(OLISP_state *state);
uintptr_t OLISP_jit_run(OLISP_state *state);

#endif

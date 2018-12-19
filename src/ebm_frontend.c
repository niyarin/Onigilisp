#include "ebm.h"
#include "ebm_frontend.h"


#define EBM_FRONTEND_INPUT_PORT 0

#define EBM_FRONTEND_FILE_PORT 0
#define EBM_FRONTEND_STRING_PORT (1<<3)



uintptr_t EBM_frontend_allocate_input_file_port(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record(5,allocator,env);
    EBM_record_primitive_set(res, 0,
                   EBM_BUILT_IN_RECORD_TYPE_PORT );
    
    EBM_record_primitive_set(res,
                   1,
                   EBM_FRONTEND_INPUT_PORT);

    EBM_record_primitive_set(res,
                   2,
                   EBM_FRONTEND_FILE_PORT);
    EBM_record_primitive_set(res,
                   3,
                  (uintptr_t)fp);

    return res;
}

uintptr_t EBM_frontend_allocate_input_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record(5,allocator,env);
    EBM_record_primitive_set(res,
                   0,
                   EBM_BUILT_IN_RECORD_TYPE_PORT );
    
    EBM_record_primitive_set(res,
                   EBM_FRONTEND_INPUT_PORT,
                   0);


    EBM_record_primitive_set(res,
                   2,
                   EBM_FRONTEND_STRING_PORT);
    EBM_record_primitive_set(res,
                   3,
                  ebm_string);

    EBM_record_primitive_set(res,
                   4,
                   0);
    return res;
}


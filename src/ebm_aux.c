#include "ebm_aux.h"
#include "ebm.h"

uintptr_t EBM_assq(uintptr_t object,uintptr_t alist){
    while (alist != EBM_NULL){
        if (EBM_eq(EBM_CAAR(alist),object)){
            return EBM_CAR(alist);
        }
        alist = EBM_CDR(alist);
    }
    return EBM_FALSE;
}

uintptr_t EBM_rassq(uintptr_t object,uintptr_t alist){
    while (alist != EBM_NULL){
        if (EBM_eq(EBM_CDAR(alist),object)){
            return EBM_CAR(alist);
        }
        alist = EBM_CDR(alist);
    }
    return EBM_FALSE;
}

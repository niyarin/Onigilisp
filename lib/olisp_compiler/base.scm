(import (olisp-tiny base-library0))

(define-library (olisp-compiler base)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library3)
     )
   (export gen-my-sym my-sym?)
   (begin
     (define-record-type 
       <my-symbol>
       (internal-gen-my-sym id)
       my-sym?
       (id my-sym-ref))


     ))

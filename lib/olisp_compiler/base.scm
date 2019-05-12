(import (olisp-tiny base-library0))

(define-library (olisp-compiler base)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library2)
     (olisp-tiny base-library3)
     )
   (export gen-my-sym my-sym?)
   (begin
     (define-record-type 
       <my-symbol>
       (internal-gen-my-sym id)
       my-sym?
       (id my-sym-ref))

     (define gen-my-sym
       (let ((id-number 0))
         (lambda ()
               (set! id-number (fx+ id-number 1))
               (internal-gen-my-sym id-number))))
     ))

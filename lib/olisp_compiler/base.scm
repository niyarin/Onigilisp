(import (scheme base))

(define-library (olisp-compiler base)
   (import (scheme base))
   (export gen-my-sym my-sym? kind-of-symbol? OLISP-COMPILER-RETURN-TO-GLOBAL )
   (begin

     (define-record-type 
       <my-symbol>
       (internal-gen-my-sym id)
       my-sym?
       (id my-sym-ref))

     (define gen-my-sym
       (let ((id-number 0))
         (lambda ()
               (set! id-number (+ id-number 1))
               (internal-gen-my-sym id-number))))

     (define (kind-of-symbol? obj)
       (or
         (my-sym? obj)
         (symbol? obj)))

     (define OLISP-COMPILER-RETURN-TO-GLOBAL (gen-my-sym))

     ))

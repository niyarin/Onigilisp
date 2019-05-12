(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library4)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library1)
     (olisp-tiny base-library2))

   (export list?)

   (begin
     (define (list? obj);simple
       (let loop ((ls obj) (cnt 0))
         (cond
           ((null? ls) #t)
           ((pair? ls) (loop (cdr ls) (fx+ cnt 1)))
           (else #f))))
     ))


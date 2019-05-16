(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library4)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library1)
     (olisp-tiny base-library2))

   (export list? length)

   (begin
     (define (list? obj);simple
       (let loop ((ls obj) (cnt 0))
         (cond
           ((null? ls) #t)
           ((pair? ls) (loop (cdr ls) (fx+ cnt 1)))
           (else #f))))

     (define (length ls)
       (let loop ((ls ls)(cnt 0))
         (cond
           ((null? ls) cnt)
           ((pair? ls) (loop (cdr ls) (fx+ cnt 1)))
           (else #f)
           )))
     ))


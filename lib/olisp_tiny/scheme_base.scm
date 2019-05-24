;(import (olisp-tiny base-library0))
(define-library (scheme base)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library1)
     (olisp-tiny base-library2)
     (olisp-tiny base-library3)
     (olisp-tiny base-library4))
   (export
     import
     define-library
     export
     define-record-type
     cond 
     else 
     => 
     and 
     or
     when 
     unless
     define
     lambda
     let
     let*
     list? length
     if set! quote begin define-library define-syntax
     apply
     cons car set-car! cdr set-cdr! eq? write-simple vector pair? symbol?
     caar cadr cdar cddr not null? list
     assq
              ))

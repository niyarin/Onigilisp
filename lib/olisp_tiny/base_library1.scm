(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library1)
   (import (olisp-tiny base-library0))
   (export caar cadr cdar cddr not null? list)
   (begin
      (define caar
        (lambda (x) (car (car x))))

      (define cadr 
        (lambda (x) (car (cdr x))))

      (define cdar 
        (lambda (x) (cdr (car x))))

      (define cddr
        (lambda (x) (cdr (cdr x))))

      (define not
        (lambda (x)
          (if (eq? x #f)
            #t
            #f)))

      (define null?
        (lambda (x)
          (eq? x '())))

      (define list
        (lambda x x))

      ))

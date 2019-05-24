(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library1)
   (import (olisp-tiny base-library0))
   (export caar cadr cdar cddr not null? list apply)
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

      (define apply-aux ;NOT TAIL CALL
        (lambda (ls)
          (if (null? (cdr ls))
            (car ls)
            (cons (car ls) (apply-aux (cdr ls))))))


      (define apply 
        (lambda args
          (apply-syntax-v
            (apply-aux args))))
      ))

(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library2)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library1))
   (export else => and)
   (begin
      (define-syntax let1 ;(let1 (bind object) body)
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (list 
              (list 
                'lambda 
                (list (car (cadr expression)))
                (car (cddr expression)))
              (cadr (cadr expression))))))

      (define-syntax and
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (if (null? (cdr expression))
              #t
              (if (null? (cddr expression))
                (cadr expression)
                (list 
                  'if
                  (list 'not (cadr expression))
                  #f
                  (cons 'and (cddr expression))))))))

   
      (define-syntax 
        else
        (ir-macro-transformer
          (lambda (expression inject compare?)
           #t)))

      (define-syntax
        =>
        (if-macro-transformer
          (lambda (expression inject compare?)
            #t)))
      ))

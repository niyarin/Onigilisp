;(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library3)
   (import 
     (olisp-tiny base-library0) 
     (olisp-tiny base-library1) 
     (olisp-tiny base-library2))
   (export define-record-type)
   (begin
      (define-syntax define-record-type
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (list 
              'begin
              (list ;constructor
                'define
                (car (cddr expression))
                (cons
                  'make-record
                  (cons
                    (list 'quote (cadr expression))
                    (cdar (cddr expression)))))
              (list ;predicate
                'define
                (cadr (cddr expression))
                (list 
                  'lambda
                  '(object)
                   (list 
                     'and
                     '(record?  object)
                     (list 
                       'eq? 
                       '(record-ref object 0)
                       (list 
                         'quote 
                         (cadr expression)))
                     #t)))
            ))))
      ))

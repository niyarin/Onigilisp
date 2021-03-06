;(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library2)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library1))
   (export 
     cond 
     else 
     => 
     and 
     or
     when 
     unless
     (rename define2 define)
     (rename lambda2 lambda)
     let
     let*
     )

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

      (define-syntax or
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (if (null? (cdr expression))
              #f
              (list
                'if
                (cadr expression)
                #t
                (cons 'or (cddr expression)))))))
      
   
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

      (define-syntax cond
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (if (compare? (car (cadr expression)) 'else)
              (cons 'begin (cdr (cadr expression)))
              (if (and 
                    (not (null? (cdr (cadr expression))))
                    (compare? (cadr (cadr expression)) '=>))
                (list
                  'let1
                  (list 'tmp (car (cadr expression)))
                  (list 
                    'if
                    'tmp
                    (list (cadr (cdr (cadr expression))) 'tmp)
                    (if (not (null? (cddr expression)))
                       (cons 'cond (cddr expression))
                       '(if #f #f))))
                (list 
                  'if 
                  (car (cadr expression))
                  (cons 'begin (cdr (cadr expression)))
                  (if 
                    (not (null? (cddr expression)))
                    (cons 'cond (cddr expression))
                    '(if #f #f)))
              )))))

      (define-syntax when
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (list 
              'if
              (cadr expression)
              (cons
                'begin
                (cddr expression))))))

      (define-syntax unless
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (list 
              'if
              (list 'not (cadr expression))
              (cons
                'begin
                (cddr expression))))))

      (define-syntax define2;Add support define syntax sugar.
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (if (pair? (cadr expression))
              (list 
                'define 
                (car (cadr expression))
                (cons 
                  'lambda
                  (cons 
                     (cdr (cadr expression))
                     (cddr expression))))
              (cons
                'define
                (cdr expression))))))

      (define-syntax lambda2;Add support multi expression in body.
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (list 
              'lambda
              (cadr expression)
              (cons 
                'begin
                (cddr expression))))))

      (define2 (map1 proc args)
         (if (null? args)
           '()
           (cons
             (proc (car args))
             (map1 proc (cdr args)))))

      (define-syntax let
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (if (symbol? (cadr expression))
              (list ;named let
                'let
                (list (list (cadr expression) #f))
                (list 
                  'begin
                   (list 
                     'set! 
                     (cadr expression)
                     (cons 
                       'lambda2
                       (cons 
                          (map1 car (car (cddr expression)))
                          (cddr (cdr expression)))))
                   (cons (cadr expression)
                         (map1 cadr (car (cddr expression))))))
              (cons;normal let
                (cons
                  'lambda2
                  (cons
                     (map1 car (cadr expression))
                     (cddr expression)))
                (map1 cadr (cadr expression)))))))

      (define-syntax let*
        (ir-macro-transformer
          (lambda (expression inject compare?)
            (if (null? (cadr expression))
              (cons 'begin (cddr expression))
              (list
                'let 
                (list (car (cadr expression)))
                (cons 'let* 
                  (cons (cdr (cadr expression))
                        (cddr expression))))))))

      ))

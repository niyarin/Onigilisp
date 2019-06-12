;(include "./simple-operator-dict.scm")

(define-library (olisp-compiler flat)
      (import (scheme base)
              (scheme fixnum)
              (olisp-compiler simple-operator-dict)
              (olisp-compiler base))

      (export olisp-compiler-flat olisp-compiler-flat-make-local-stack )
      (begin
        (define (%get-operation-information
                  operation-symbol
                  local-stack)
          (cond 
            ((assq operation-symbol olisp-compiler-simple-operator-dict) => cdr)
            (else  #f)))

        (define (%can-flat? operation-information)
          #t);TODO:

        (define (%olisp-compiler-const val local-stack index)
            (cond
              ((and (pair? val) (eq? (car val) 'quote))
                  (list 
                    'OLISP-COMPILER-CONST 
                    (cadr val) 
                    (list 'REGISTER index)
                    ))
              ((and (kind-of-symbol? val) 
                    (assq val local-stack))
               =>
               (lambda (apair)
                 (cond 
                   ((assq 'position (cdr apair))
                    => 
                    (lambda (apair)
                      (list 
                        'OLISP-COMPILER-MV 
                        (cadr apair)
                        (list 'REGISTER index)
                    ))))))
              (else 
                (list
                  'OLISP-COMPILER-CONST
                  val
                  (list 'REGISTER index)))))

        (define (olisp-compiler-flat-make-local-stack args)
          (let loop ((args args)(res '()))
            (if (null? args)
              res
              (loop 
                (cdr args)
                 (cons 
                   (list
                     (car args);TODO:間違ってそう
                     (list 'position (list 'ARG (length (cdr args)))))
                   res))
            )))

        (define (%get-continuation-body cont)
          (cond
            ((eq? cont OLISP-COMPILER-RETURN-TO-GLOBAL)
             #f)
            ((and (pair? cont) (eq? (car cont) 'lambda))
             (cadr (cdr cont)))
            (else 
              #f)))

        (define (%get-continuation-symbol cont)
          (cond
            ((kind-of-symbol? cont) cont)
            ((pair? cont)(car (cadr cont)));TODO:<====== support values
            (else #f)))

        (define (olisp-compiler-flat cps-code . opt)
          (let ((res-top (list '())))
             (let loop ((input-code cps-code)
                        (local-stack 
                          (cond 
                            ((assq 'local-stack opt)
                             => cadr)
                            (else '())))
                        (local-index 
                          (cond
                            ((assq 'local-index opt)
                                 => cadr)
                             (else 0)))
                        (res res-top))

               (let ((operation-information 
                       (%get-operation-information
                         (car input-code)
                         local-stack)))

                 (cond
                   ((not operation-information)
                    #f)
                   ((not (%can-flat? operation-information))
                    #f)
                   (else
                     (begin
                        (let loop ((args  (cddr input-code))
                                   (index 0))
                          (unless (null? args)
                            (set-cdr!
                              res
                              (list 
                                (%olisp-compiler-const (car args) local-stack index)))
                            (set! res (cdr res))
                            (loop (cdr args) (fx+ index 1))))
                        (begin
                          
                           (for-each
                             (lambda (x)
                               (begin

                                  (set-cdr!
                                    res
                                    (cons x '()))
                                  (set! res (cdr res))))
                             (cdr (assq 'byte-code operation-information)))
                           (set-cdr!
                             res
                             (list
                                (list
                                  'OLISP-COMPILER-MV
                                  '(REGISTER 0)
                                  (list
                                    'LOCAL 
                                    local-index))))
                           (set!
                             res
                             (cdr res)))

                        (let* ((cont
                                (cadr input-code))
                               (cont-symbol (%get-continuation-symbol cont))
                               (cont-body (%get-continuation-body cont)))
                          (if cont-body
                            (loop 
                              cont-body 
                              (cons 
                                (list cont-symbol 
                                      (list 'position (list 'LOCAL local-index)))
                                local-stack)
                                (fx+ local-index 1)
                              res)
                            (begin
                               (set-cdr! 
                                   res
                                   (list (list 'RET)))
                               (cdr res-top)
                               )
                            )
                          )))
                 )
                 ))))

        )
      )

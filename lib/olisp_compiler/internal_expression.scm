;(include "./base.scm")

(define-library (olisp-compiler internal-expression)
   (import (scheme base) (olisp-compiler base))
   (export olisp-compiler-cps-conv)
   (begin

      (define (not-have-continuation? expression)
         (cond 
           ((not (list? expression))
               (list expression))
           ((null? expression)
               (list '()))
           ((eq? (car expression) 'quote)
            (list expression))
           ((eq? (car expression) 'lambda)
            (let ((new-sym (gen-my-sym)))
              (list
                 (list
                   'lambda
                   (cons new-sym (cadr expression))
                  (olisp-compiler-cps-conv 
                    (car (cddr expression))
                    (list 'cont-symbol new-sym)
                    )))))
           (else 
             #f)))

      (define (olisp-compiler-cps-conv scm-code . opt)
         (cond 
           ((not-have-continuation? scm-code) =>  car)
           (else
              (let conv-loop ((code scm-code)
                              (continuation-symbol  
                                (cond
                                  ((assq 'cont-symbol opt) => cadr)
                                  (else OLISP-COMPILER-RETURN-TO-GLOBAL)))
                              (stack '()))
               (cond
                 ((and (eq? (car code) 'if)
                       (not-have-continuation? (cadr code))) 
                  => (lambda (test-expression)
                       (set-car! (cdr code) (car test-expression))
                       ))
                     
                 (else 
                   (let loop ((ls code))
                     (cond 
                       ((and (null? ls)  (null? stack));外側
                        (set-cdr! 
                          code
                          (cons continuation-symbol (cdr code)))
                        code)
                       ((null? ls)
                        (set-cdr!
                          code
                          ;(cons `(lambda (,(caar stack))  ,(conv-loop (cdar stack) continuation-symbol (cdr stack))) (cdr code))
                          (cons (list 'lambda (list (caar stack)) (conv-loop (cdar stack) continuation-symbol (cdr stack))) (cdr code))
                          )
                        code)
                        
                       ((not-have-continuation? (car ls))
                        (loop (cdr ls)))
                       (else;継続有り
                         (let ((new-my-sym (gen-my-sym))
                               (new-target-code (car ls)))

                           (set-car! ls new-my-sym)
                           (conv-loop new-target-code
                                      continuation-symbol
                                      (cons (cons new-my-sym code) stack ))))
                       )
                   )
                    ))))))
      ))


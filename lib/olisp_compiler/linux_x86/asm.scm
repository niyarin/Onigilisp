(define-library (olisp-compiler linux-86 asm)
   (import (scheme base)
           (srfi 143))
   (export olisp-compiler-linux-x86-asm)

   (begin
     (define ptr-size 4)

     (define (%encode-operand o1 o2)
       (cond
         ((and (eq? (car o1) 'REGISTER)
               (eq? (car o2) 'REGISTER))
            (fx+
              192;0b11000000
              (fx+
                 (fxarithmetic-shift-left (cadr o2) 3)
                 (cadr o1))))

         ((and (eq? (car o1) 'STACK)
               (eq? (car o2) 'REGISTER))
            (cons
               (fx+
                 128;0b10000000
                 (fx+
                    (fxarithmetic-shift-left (cadr o2) 3)
                    5
                    ))
                (fx* (fx+ (cadr o1) 1) 4))
               )
         (else (error "TBA"))
         ))

     (define (%encode-opecode code)
       (cond
          ((or 
             (and (eq? (car (cadr code)) 'REGISTER)
                  (eq? (car (cadr (cdr code))) 'REGISTER))
             (and (eq? (car (cadr code)) 'STACK)
                  (eq? (car (cadr (cdr code))) 'REGISTER)))
           89)
          ))

     (define (olisp-compiler-linux-x86-asm code)

       (let ((res (make-bytevector 128))
             (index 0))

          (begin ;PUSH EBP;MOV ESP EBP
            (bytevector-u8-set! res 0 55)
            (bytevector-u8-set! res 1 137)
            (bytevector-u8-set! res 2 229)
            (set! index 3))

          (let loop ((code code))
            (begin
              (cond
                 ((eq? (caar code) 'OLISP-COMPILER-MV)
                  (begin
                      (bytevector-u8-set!
                        res
                        index
                        (%encode-opecode (car code)))
                      (set! index (fx+ index 1))

                      (let ((encoded-operand 
                              (%encode-operand
                                (cadr (car code))
                                (cadr (cdr (car code))))))
                        (if (pair? encoded-operand)
                          (begin
                            (bytevector-u8-set!
                              res
                              index
                              (car encoded-operand))
                            (set! index (fx+ index 1))
                            (bytevector-u8-set!
                              res
                              index
                              (cdr encoded-operand))
                            (set! index (fx+ index 1))
                            )
                          (begin
                            (bytevector-u8-set!
                              res
                              index
                              encoded-operand
                              )
                            (set! index (fx+ index 1)))
                      ))
                   ))
               );end cond
               (unless (null? (cdr code))
                  (loop 
                    (cdr code)))
       ))
          (begin;POP EBP;RET;
            (bytevector-u8-set! res index 93)
            (bytevector-u8-set! res (fx+ index 1) 195)
            )
            res
     ))))

'(import 
  (scheme base)
  (olisp-compiler linux-86 asm)
  (scheme write))
'(display 
   (olisp-compiler-linux-x86-asm
  '((OLISP-COMPILER-MV (REGISTER 0) (REGISTER 0))
    (OLISP-COMPILER-MV (STACK 0) (REGISTER 0))
    )))

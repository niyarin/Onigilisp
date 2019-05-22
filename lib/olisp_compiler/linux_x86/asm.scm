(define-library (olisp-compiler linux-86 asm)
   (import (scheme base)
           (scheme cxr)
           (scheme fixnum))
   (export olisp-compiler-linux-x86-asm)

   (begin
     (define *PTR-SIZE 4)

     (define (%encode-operand o1 o2)
       (cond
         ((and (eq? (car o1) 'REGISTER)
               (eq? (car o2) 'REGISTER))
            (list
               (fx+
                 192;0b11000000
                 (fx+
                    (fxarithmetic-shift-left (cadr o2) 3)
                    (cadr o1)))))
         ((and (eq? (car o1) 'ARG)
               (eq? (car o2) 'REGISTER))
            (list
               (fx+
                 64;0b01000000
                 (fx+
                    (fxarithmetic-shift-left (cadr o2) 3)
                    5
                    ))
                (fx* (fx+ (cadr o1) 2) *PTR-SIZE)))

         ((and 
            (eq? (car o1) 'REGISTER-REF)
            (eq? (car o2) 'REGISTER)
            (fx<? (cadr (cdr o1)) 128))
            (list
               (fx+
                 64;0b01000000
                 (fx+
                    (fxarithmetic-shift-left (cadr o2) 3)
                    (cadr o1)
                    ))
                (cadr (cdr o1))))
         (else (error "TBA"))
         ))

     (define (%calc-offset-size reg-ref)
       (caddr reg-ref))

     (define (%encode-opecode-mov code)
       (cond
          ((or 
             (and (eq? (car (cadr code)) 'REGISTER)
                  (eq? (car (caddr code)) 'REGISTER))
             (and (eq? (car (cadr code)) 'ARG)
                  (eq? (car (caddr code)) 'REGISTER))
             (and (eq? (car (cadr code)) 'REGISTER-REF)
                  (eq? (car (caddr code)) 'REGISTER)))

           139)
          (else (error "TBA"))

          ))

     (define (olisp-compiler-linux-x86-asm code)

       (let* ((res (make-bytevector 128))
              (index 0)
              (%mv-encode
                (lambda (mv-code);TODO:組み合わせによっては2回にわける
                      (bytevector-u8-set!
                        res
                        index
                        (%encode-opecode-mov mv-code))
                      (set! index (fx+ index 1))
                      (let ((encoded-operands
                              (%encode-operand
                                (cadr mv-code)
                                (cadr (cdr mv-code)))))

                        (let internal-loop ((encoded-operands encoded-operands))
                          (unless (null? encoded-operands)
                            (bytevector-u8-set!
                              res
                              index
                              (car encoded-operands))
                            (set! index (fx+ index 1))
                            (internal-loop (cdr encoded-operands))
                      )))))
              (%shr-encode
                (lambda (register)
                     (begin ;SHR 2 REG;
                       (bytevector-u8-set! 
                         res
                         index
                         193)
                       (bytevector-u8-set! 
                         res
                         (fx+ index 1)
                         (fx+
                           register
                           224
                           )
                         )
                       (bytevector-u8-set! 
                         res
                         (fx+ index 2)
                         2)
                       (set!
                         index
                         (+ index 3)))))
              )


          (begin ;PUSH EBP;MOV ESP EBP
            (bytevector-u8-set! res 0 85)
            (bytevector-u8-set! res 1 137)
            (bytevector-u8-set! res 2 229)
            (set! index 3))

          (let loop ((code code))
            (display (car code))(newline)
            (begin
              (cond
                 ((eq? (caar code) 'OLISP-COMPILER-MV)
                     (%mv-encode (car code)))
                 ((and (eq? (caar code) 'OLISP-COMPILER-REF)
                       (eq? (car (cadddr (car code))) 'REGISTER)
                       (not (pair? (caddar code))))
;a
                     (let ((ref-code (car code)))
                        (cond 
                          ((eq? (caadr ref-code) 'REGISTER)
                              (error "TBA")
                              (%mv-encode
                                (list
                                  'OLISP-COMPILER-MV
                                  (list 
                                    'REGISTER-REF
                                    (cadadr ref-code)
                                    (fx*
                                       (caddr ref-code)
                                       *PTR-SIZE))
                                  (cadr (cddr ref-code)))))
                          ((eq? (caadr ref-code) 'ARG)
                              (%mv-encode
                                (list
                                  'OLISP-COMPILER-MV
                                  (cadr ref-code)
                                  (cadddr ref-code)))
                              (%shr-encode
                                (fx+
                                 (cadr (cadddr ref-code))
                                 8)
                                )
                              (%shr-encode
                                 (cadr (cadddr ref-code))
                                )
                              (%mv-encode
                                (list
                                  'OLISP-COMPILER-MV
                                  (list 
                                    'REGISTER-REF
                                    (cadadr ref-code)
                                    (fx*
                                       (caddr ref-code)
                                       *PTR-SIZE))
                                  (cadddr ref-code)))
                           )
                          (else (error "TBA"))))
                  )
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

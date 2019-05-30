(define-library (olisp-compiler linux-86 asm)
   (import (scheme base)
           (scheme cxr)
           (scheme write)
           (srfi 143))
   (export olisp-compiler-linux-x86-asm)

   (begin
     (define *PTR-SIZE 4)
     (define *STACK-HEAD 3)
     (define *LOCAL-STACK-POSITION 2)
     (define *TMP-REGISTER 3)

     (define (%encode-operand o1 o2)
       (cond
         ((and (not (pair? o1))
               (eq? (car o2) 'REGISTER))
          (list
             (fx+
                 192;0b11000000
                 (cadr o2))
             o1
             ))

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
                (fx* (fx+ (cadr o1) *STACK-HEAD) *PTR-SIZE)))

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
         (else (error "TBA?"))
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
          ((or 
             (and (eq? (car (cadr code)) 'REGISTER)
                  (eq? (car (caddr code)) 'REGISTER-REF)))
           137)

           ((eq? (car (cadr code)) 'LOCAL-REF)
               ;8b 45 08                mov    0x8(%ebp),%eax
               (list
                 (list
                   'OLISP-COMPILER-MV
                   (list 'ARG -1)
                   (caddr code))

                 (list
                   'OLISP-COMPILER-MV
                   (list 'REGISTER-REF (cadr (caddr code)) 0)
                   (caddr code))

                (list
                  'OLISP-COMPILER-ADD
                  (fx* 
                    (cadr (cadr code))
                    *PTR-SIZE)
                  (caddr code))
                 ))
           ((eq? (car (caddr code)) 'LOCAL)
            (list
              (list
                'OLISP-COMPILER-MV
                (list 'LOCAL-REF (cadr (caddr code)))
                (list 'REGISTER *TMP-REGISTER))

              (list 'OLISP-COMPILER-MV (cadr code) (list 'REGISTER-REF *TMP-REGISTER 0))
            ;stack 1 をedxにおく
            ;MV (edx) edx
            ;lea ずらすedx edx
            ;MOV FROM,(edx)
            ))
          (else (error "TBA" code))
          ))

     (define (olisp-compiler-linux-x86-asm code)

       (letrec ((res (make-bytevector 128))
              (index 0)
              (%mv-encode
                (lambda (mv-code);TODO:組み合わせによっては2回にわける
                  ;TODO:EBX (REGISTER 3) への書き込みはSTACKつかう
                  (let ((mov-code 
                        (%encode-opecode-mov mv-code)))
                    (cond
                      ((pair? mov-code) 
                       (begin
                       (for-each
                         %encode
                         mov-code
                       )))
                      (else
                         (bytevector-u8-set!
                           res
                           index
                           mov-code
                           )
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
                               (internal-loop (cdr encoded-operands))))))
                      ))))

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
              (%add-encode
                (lambda (asm-code)
                  (begin
                     (cond 
                       ((not (pair? (cadr asm-code)))
                           (begin
                              (bytevector-u8-set!      
                                res
                                index
                                131)
                              (set! index (fx+ index 1))))
                       (else
                         (error "ERROR")))
                     
                     (let ((operands
                             (%encode-operand (cadr asm-code) (caddr asm-code))))
                       (if (pair? operands)
                        (for-each
                          (lambda (operand)
                            (bytevector-u8-set!
                              res
                              index
                              operand)
                            (set! index (fx+ index 1)))
                          operands)
                        (begin
                          (bytevector-u8-set!
                              res
                              index
                              operands)
                          (set! index (fx+ index 1)))
                        ))))) 
              (%encode 
                (lambda (asm-code)
                  (cond 
                    ((eq? (car asm-code) 'OLISP-COMPILER-MV)
                        (%mv-encode asm-code))
                    ((eq? (car asm-code) 'OLISP-COMPILER-ADD)
                        (%add-encode asm-code))
                    ((and (eq? (car asm-code) 'OLISP-COMPILER-PTR-REF)
                          (eq? (car (cadddr asm-code)) 'REGISTER)
                          (not (pair? (caddr asm-code))))
                        (cond 
                          ((eq? (caadr asm-code) 'REGISTER)
                              (%shr-encode
                                (fx+
                                 (cadr (cadr asm-code))
                                 8))
                              (%shr-encode
                                 (cadr (cadr asm-code)))
                              (%mv-encode
                                (list
                                  'OLISP-COMPILER-MV
                                  (list 
                                    'REGISTER-REF
                                    (cadadr asm-code)
                                    (fx*
                                       (caddr asm-code)
                                       *PTR-SIZE))
                                  (cadr (cddr asm-code)))))
                          ((eq? (caadr asm-code) 'ARG)
                              (%mv-encode
                                (list
                                  'OLISP-COMPILER-MV
                                  (cadr asm-code)
                                  (cadddr asm-code)))
                              (%shr-encode
                                (fx+
                                 (cadr (cadddr asm-code))
                                 8))
                              (%shr-encode
                                 (cadr (cadddr asm-code)))
                              (%mv-encode
                                (list
                                  'OLISP-COMPILER-MV
                                  (list 
                                    'REGISTER-REF
                                    (cadadr asm-code)
                                    (fx*
                                       (caddr asm-code)
                                       *PTR-SIZE))
                                  (cadddr asm-code))))
                          (else (error "TBA"))))
                 (else 
                   (write asm-code)
                   (write "<===\n")
                   (error "TBA!!!!?"))
               );end cond
                  )
                );%end encode
              )


          (begin ;PUSH EBP;MOV ESP EBP
            (bytevector-u8-set! res 0 85)
            (bytevector-u8-set! res 1 137)
            (bytevector-u8-set! res 2 229)
            (set! index 3))

          (let loop ((code code))
            (unless (null? code)
               (%encode (car code))
               (loop 
                 (cdr code))))
          (begin;POP EBP;RET;
            (bytevector-u8-set! res index 93)
            (bytevector-u8-set! res (fx+ index 1) 195)
            )
            res
     ))))

(import 
  (scheme base)
  (olisp-compiler linux-86 asm)
  (scheme write))
'(display 
   (olisp-compiler-linux-x86-asm
  '(
    (OLISP-COMPILER-REF (ARG 0) 1 (REGISTER 0))

    )))

(display 
  (olisp-compiler-linux-x86-asm
      '(
        ;(OLISP-COMPILER-MV (ARG 0) (REGISTER 0)) 
        ;(OLISP-COMPILER-PTR-REF (REGISTER 0) 0 (REGISTER 0))
        (OLISP-COMPILER-MV (REGISTER 0) (LOCAL 1)) 
        ;(OLISP-COMPILER-MV (LOCAL 0) (REGISTER 0)) 
        ;((OLISP-COMPILER-PTR-REF (REGISTER 0) 1 (REGISTER 0))) 
        ;(OLISP-COMPILER-MV (REGISTER 0) (LOCAL 1)) 
        ;(RET)
        )))
          

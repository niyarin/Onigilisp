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

     (define (convert-ref position)
       (cond 
         ((eq? (car position) 'REGISTER)
          (list 'REGISTER-REF (cadr position) 0))
         ((eq? (car position) 'LOCAL)
          (list 'LOCAL-REF (cadr position)))
         (else (error "ERROR"))
         ))
   
     (define (%encode-jmp-opecode asm-code)
       (cond
         ((eq? (caddr asm-code) '<=)
          119)
         (else (display asm-code)(newline)(error "TBA"))))

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
         ((and
            (eq? (car o1) 'REGISTER)
            (eq? (car o2) 'REGISTER-REF)
            (fx=? (caddr o2) 0))
               (list
                 (fx+
                    (fxarithmetic-shift-left (cadr o1) 3)
                    (cadr o2)
                    )))
         (else (error "TBA?" (list o1 o2)))
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

           ((eq? (caadr code) 'LOCAL)
               (list
                  (list 
                    'OLISP-COMPILER-MV
                    (list 'LOCAL-REF (cadr (cadr code)))
                    (caddr code))
                  (list
                    'OLISP-COMPILER-MV
                    (convert-ref (caddr code))
                    (caddr code))))
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
           ((eq? (car (cadr code)) 'MEMCHANK-REF)
            (list;↑　あとでまとめる
              
                 (list
                   'OLISP-COMPILER-MV
                   (list 'ARG -1)
                   (caddr code))

                 (list
                   'OLISP-COMPILER-MV
                   (list 
                     'REGISTER-REF 
                     (cadr (caddr code)) 
                     *PTR-SIZE)
                   (caddr code));PTR[1] => eax

                (list
                  'OLISP-COMPILER-ADD
                  (fx* 
                    (cadr (cadr code))
                    *PTR-SIZE)
                  (caddr code))

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
              (%cmpl-encode
                (lambda (asm-code)
                  (begin
                    (cond 
                      ((and;TODO:このcond節　未検証
                         (not (pair? (caddr asm-code)))
                         (fx<? (caddr asm-code) 256)
                         (pair? (cadr asm-code))
                         (eq? (car (cadr asm-code)) 'REGISTER))
                       (begin
                         (bytevector-u8-set!
                          res
                          index
                          131)
                         (bytevector-u8-set!
                           res
                           (fx+ index 1)
                           (fx+
                             248
                             (cadr (cadr asm-code))))
                         (bytevector-u8-set!
                           res
                           (fx+ index 2)
                           (caddr asm-code))
                         (set! index (fx+ index 2))))
                      ((and
                        (pair? (caddr asm-code))
                        (pair? (cadr asm-code))
                        (eq? (car (caddr asm-code)) 'REGISTER)
                        (eq? (car (cadr asm-code)) 'REGISTER))

                       (bytevector-u8-set!
                         res
                         index
                         57)

                       (set! index (fx+ index 1))
                      
                       (for-each
                         (lambda (a)
                           (begin
                             (bytevector-u8-set! 
                               res
                               index
                               a)
                             (set! index (fx+ index 1))))
                       (%encode-operand (cadr asm-code) (caddr asm-code))))
                      (else (error "ERROR"))))))

              (%encode 
                (lambda (asm-code)
                  (cond 
                    ((eq? (car asm-code) 'OLISP-COMPILER-MV)
                        (%mv-encode asm-code))
                    ((eq? (car asm-code) 'OLISP-COMPILER-ADD)
                        (%add-encode asm-code))
                    ((eq? (car asm-code) 'OLISP-COMPILER-CMPL)
                        (%cmpl-encode asm-code)
                     )

                    ((eq? (car asm-code) 'OLISP-COMPILER-ALLOCATE)
                     ;cmpl   $0x2,%ebx

                     (%mv-encode
                       (list
                         'OLISP-COMPILER-MV
                         (list
                           'MEMCHANK-REF
                           0)
                         (list
                           'REGISTER
                           *TMP-REGISTER)))
                     (begin;
      

                       )



                     )
                    ((eq? (car asm-code) 'OLISP-COMPILER-RET)
                        (bytevector-u8-set! res index 93)
                        (bytevector-u8-set! res (fx+ index 1) 195)
                        (set! index (fx+ index 2)))
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
                   (write asm-code)(newline)
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
               (cond
                 ((eqv? (caar code) 'OLISP-COMPILER-JA)
                  (begin
                     (bytevector-u8-set!
                       res
                       index
                       (%encode-jmp-opecode (car code)))
                     (bytevector-u8-set!
                       res
                       (fx+ index 1)
                       0)

                     (let ((jmp-address-index (fx+ index 1))
                           (loop-size (cadr (car code))));re18
                       (set! index (fx+ index 2))
                       (let loop-internal  ((code (cdr code))
                                            (cnt 0))
                         (if (fx=? cnt loop-size)
                           (begin
                              (bytevector-u8-set!
                                res
                                jmp-address-index
                                (fx- (fx- index 1) jmp-address-index))
                              (loop code))
                           (begin
                             (%encode (car code))
                             (loop-internal (cdr code) (fx+ cnt 1))))))))
                 (else
                    (%encode (car code))
                  (loop 
                    (cdr code))))))
          (begin;POP EBP;RET;
            (bytevector-u8-set! res index 93)
            (bytevector-u8-set! res (fx+ index 1) 195)
            )
            res
     ))))


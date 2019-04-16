(define-library (olisp-tiny base-library1)
   (export caar cadr cdar cddr not)
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
            #f)))))

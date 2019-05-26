(import (olisp-tiny base-library0))

(define-library (olisp-tiny base-library4)
   (import 
     (olisp-tiny base-library0)
     (olisp-tiny base-library1)
     (olisp-tiny base-library2))

   (export list? length assq for-each)

   (begin
     (define (list? obj);simple
       (let loop ((ls obj) (cnt 0))
         (cond
           ((null? ls) #t)
           ((pair? ls) (loop (cdr ls) (fx+ cnt 1)))
           (else #f))))

     (define (length ls)
       (let loop ((ls ls)(cnt 0))
         (cond
           ((null? ls) cnt)
           ((pair? ls) (loop (cdr ls) (fx+ cnt 1)))
           (else #f)
           )))

     (define (assq obj alist)
       (cond
         ((null? alist) #f)
         ((eq? obj (caar alist))
          (car alist))
         (else
           (assq obj (cdr alist)))))
      
      (define (map1-with-check proc args)
        (let ((listop (list '() '())))
           (let loop ((args args)(res-cell listop))
             (cond 
               ((null? args) (cdr listop))
               ((null? (car args)) #f)
               (else
                 (loop 
                   (cdr args)
                   (begin
                      (set-cdr!
                        res-cell
                        (list (proc (car args))))
                      (cdr res-cell))))))))

      (define (for-each proc . args) 
        (unless (or (null? args) (null? (car args)))
           (let ((result-of-map-car-check (map1-with-check car args)))
              (when result-of-map1-check
                 (apply proc result-of-map-car-check)
                 (apply for-each (cons proc (map1-with-check cdr args)))))))

     ))

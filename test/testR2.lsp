(define scope
    ((lambda (outer)
       (lambda (inner)
         outer))
     1
     ))

(scope 3)

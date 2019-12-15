;(define b ((lambda (v) (setq v 8) v)))
;b
;(b)
(define a ((lambda (v) (lambda () (setq v 8) v)) 10))
;a
(a)

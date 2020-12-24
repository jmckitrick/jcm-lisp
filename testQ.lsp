(define b (lambda (v) v))
(b 9)

(define c ((lambda (v) (setq v 8) v) 99))
c

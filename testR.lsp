(define count
    ((lambda (total)
       (lambda (increment)
         (setq total (+ total increment))
         total))
     1))
(count 2)
(count 3)

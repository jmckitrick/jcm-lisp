(define count
  ((lambda (total)
     (lambda (increment)
       (setq total (+ total increment))
       total))
   0))

(count 5)

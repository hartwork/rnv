; $Id$
; sample scheme datatype library for RVP

(define (dsl-equal? typ val s)
  (string=? val s))

(define (dsl-allows? typ ps s)
  #t)

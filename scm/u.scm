; $Id$
; unicode

(define (string->u-list s)
  (letrec (
      (ux
        (lambda (c ul sl n) 
	  (if (= n 0) (left (cons c ul) sl)
	    (ux 
	      (+ (* c 64) (remainder (char->integer (car sl)) 64)) 
	      ul (cdr sl) (- n 1)))))
      (left
        (lambda (ul sl)
          (if (null? sl) (reverse ul)
            (let ((c (char->integer (car sl))) (sl (cdr sl)))
              (cond 
                  ((< c #x80) (ux c ul sl 0))
                  ((< c #xC0) #f)
                  ((< c #xE0) (ux (remainder c #x20) ul sl 1))
                  ((< c #xF0) (ux (remainder c #x10) ul sl 2))
                  ((< c #xF8) (ux (remainder c #x08) ul sl 3))
                  ((< c #xFC) (ux (remainder c #x04) ul sl 4))
                  ((< c #xFE) (ux (remainder c #x02) ul sl 5)))
                )))))
    (left '() (string->list s))))      

(define (u-in-ranges u ranges)
  (let between ((n 0) (m (- (vector-length ranges) 1)))
    (if (> n m) #f
      (let* ((i (quotient (+ n m) 2)) (r (vector-ref ranges i)))
        (cond
	  ((< u (car r)) (between n (- i 1)))
	  ((> u (cdr r)) (between (+ i 1) m))
	  (else r))))))

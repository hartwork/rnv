; $Id$
; XML Scheme Datatypes Regular Expressions

(load "u.scm")
(load "rx-ranges.scm")

; patterns: none empty any choice group except range class char

(define (rx-compile regex)
  (letrec (
      (s (utf8->lazy-list regex))
      (newpat
        (let ((cache '()))
          (lambda (pattern)
            (car
              (or (member pattern cache)
                (begin (set! cache (cons pattern cache)) cache))))))
      (errors #f)
      (error!
        (lambda () 
	  (or errors 
	    (begin 
	      (display (string-append "bad regex '" regex "'\n"))
	      (set! errors #t)))))
      (chclass
        #f)
      (esc
        (lambda ()
	  (if (null? s) (begin (error!) '(end))
	    (let ((c (car s)))
	      (set! s (force (cdr s))
	      (if (>= c 128) (begin (error!) (cons 'esc c))
	        (case (integer->char c)
		  ((#\p) (cons 'cls (chclass)))
		  ((#\P) (cons 'ncl (chclass)))
		  ((#\s) '(cls . S))
		  ((#\S) '(ncl . S))
		  ((#\i) '(cls . I))
		  ((#\I) '(ncl . I))
		  ((#\c) '(cls . C))
		  ((#\C) '(ncl . C))
		  ((#\d) '(cls . UNd))
		  ((#\D) '(ncl . UNd))
		  ((#\w) '(cls . W))
		  ((#\W) '(ncl . W))
		  ((#\n) '(esc . 10))
		  ((#\r) '(esc . 13))
		  ((#\t) '(esc . 9))
                  ((#\\\ #\| #\. #\- #\^ #\? #\* #\+ 
		      #\{ #\} #\[ #\] #\( #\))
		    (cons 'esc c))
		  (else (error!) (cons 'esc c)))))))))
      (sym
        (lambda ()
          (if (null? s) '(end)
            (let ((c (car s)))
              (set! s (force (cdr s)))
                (or (and (= c 92) (esc)) (and (= c 46) '(ncl . NL)) 
		  (cons 'chr c))))))
      )
    #f))
      
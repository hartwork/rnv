; $Id$
; XML Schema Datatypes Regular Expressions

(load  (in-vicinity (program-vicinity) "u.scm"))
(load  (in-vicinity (program-vicinity) "rx-ranges.scm"))

; patterns: none empty any choice group more except range class char

(define (rx-null p)
  (case (car p)
    ((empty) #t)
    ((choice) (or (rx-null (cadr p)) (rx-null (cddr p))))
    ((group) (and (rx-null (cadr p)) (rx-null (cddr p))))
    ((more) (rx-null (cdr p)))
    (else #f)))

(define rx-newpat ; #f flushes
  (let (
      (cache '())
      (pat=?
	(lambda (p1 p2)
	  (and
	    (eqv? (car p1) (car p2))
	    (case (car p1)
	      ((none empty any) #t)
	      ((choice group except range except range)
		(and (eqv? (cadr p1) (cadr p2)) (eqv? (cddr p1) (cddr p2))))
	      ((more class char range)
		(eqv? (cdr p1) (cdr p2)))))))	
      (old
	(lambda (p cache)
	  (and (pair? cache)
	    (or (and (pat=? p (car cache)) p)
	      (old p (cdr cache)))))))
    (lambda (p)
      (or p
	(or (old p cache)
	  (begin (set! cache (list p))
	    p)
	(begin
	  (set! cache '())
	  #f))))))
	
(define (rx-more p)
  (case (car p)
    ((none empty more) p)
    (else (rx-newpat `(more . ,p)))))

(define rx-choice
  (letrec (
      (same
	(lambda (p1 p2)
	  (or
	    (and (eqv? 'choice (car p1))
	      (or (eqv? (cddr p1) p2) (same (cadr p1) p2)))
	    (eqv? p1 p2)))))
    (lambda (p1 p2)
      (or
	(and (eqv? 'none (car p1)) p2)
	(and (eqv? 'none (car p2)) p1)
	(and (eqv? 'choice (car p2))
	  (rx-choice (rx-choice p1 (cadr p2)) (cddr p2)))
	(and (same p1 p2) p1)
	(and (rx-null p1) (eqv? 'empty (car p2)) p1)
	(and (rx-null p2) (eqv? 'empty (car p1)) p2)
	(rx-newpat `(choice ,p1 . ,p2))))))

(define (rx-group p1 p2)
  (cond
    ((or (eqv? 'none (car p1)) (eqv? 'none (car p2))) (rx-newpat '(none)))
    ((eqv? 'empty (car p1)) p2)
    ((eqv? 'empty (car p2)) p1)
    (else (rx-newpat `(group ,p1 . ,p2)))))

; parser symbols: chr esc cls ncl end

(define (rx-compile regex)
  (letrec (
      (nextc
	(let ((s (utf8->lazy-list regex)))
	  (lambda ()
	    (and (pair? s) (let ((c (car s))) (set! s (force (cdr s))) c)))))
      (newpat
	(let ((cache '()))
	  (lambda (pattern)
	    (car
	      (or (member pattern cache)
		(begin (set! cache (cons pattern cache)) cache))))))
      (errors #f)
      (error!
	(lambda msg
	  (or errors
	    (begin
	      (display (string-append "bad regex '" regex "'"))
	      (if (pair? msg)
		(begin (display ":")
		  (map (lambda (x) (display " ") (display x)) msg)))
	      (display "\n")
	      (set! errors #t)))))
      (chclass
	(lambda ()
	  (if (not (= (nextc) 123)) (begin (error! 1) #f)
	    (let i ((c (nextc)) (l '()))
	      (cond
		((not c) (error! 2) #f)
		((>= c 128) (error! 3) #f)
		((= c 125)
		  (let (
		     (clsym
		       (string->symbol
			 (list->string
			  `(#\u #\-
			 ,@(map (lambda (i)
			     (char-downcase (integer->char i)))
			     (reverse l)))))))
		   (or (assv clsym rx-ranges) (error! "unknown class" clsym))
		   clsym))
	       (else (i (nextc) (cons c l))))))))
      (esc
	(lambda ()
	  (let ((c (nextc)))
	    (if (not c) (begin (error! 4) '(end))
	      (if (>= c 128) (begin (error! 5) `(esc ,c))
		(case (integer->char c)
		  ((#\p) `(cls . ,(chclass)))
		   ((#\P) `(ncl . ,(chclass)))
		  ((#\s) '(cls . S))
		   ((#\S) '(ncl . S))
		  ((#\i) '(cls . I))
		   ((#\I) '(ncl . I))
		  ((#\c) '(cls . C))
		   ((#\C) '(ncl . C))
		  ((#\d) '(cls . U-Nd))
		   ((#\D) '(ncl . U-Nd))
		  ((#\w) '(cls . W))
		   ((#\W) '(ncl . W))
		  ((#\n) '(esc . 10))
		  ((#\r) '(esc . 13))
		  ((#\t) '(esc . 9))
		  ((#\\ #\| #\. #\- #\^ #\? #\* #\+
		      #\{ #\} #\[ #\] #\( #\))
		   `(esc . ,c))
		  (else (error! 6) `(esc . ,c))))))))
      (sym #f)
      (getsym
	(lambda ()
	  (set! sym
	    (let ((c (nextc)))
	      (or (and (not c) '(end))
		(or (and (= c 92) (esc))
		  (and (= c 46) '(ncl . NL)))
	       `(chr . ,c))))))
      (chexpr
	(lambda()
	  #f))
      (atom
	(lambda ()
	  (case (car sym)
	    ((chr)
	      (case (cdr sym)
		((91) ; [
		  (getsym)
		  (let ((p (chexpr)))
		    (or (equal? sym '(chr . 93)) (error! "missing ]"))
		    p))
		((40) ; (
		  (getsym)
		  (let ((p (expr)))
		    (or (equal? sym '(chr . 41)) (error! "missing )"))
		    (getsym)
		    p))
		((41 42 43 63 93 123 124 125) (error! 6) (getsym) p)
		(else
		  (let ((p (rx-newpat `(char . ,(cdr sym))))) (getsym) p))))
	    ((esc)
	      (let ((p (rx-newpat `(char . ,(cdr sym))))) (getsym) p))
	    ((cls)
	      (let ((p (rx-newpat `(class . ,(cdr sym))))) (getsym) p))
	    ((ncl)
	      (let ((p (rx-newpat `(except any .
		      ,(rx-newpat `(class . ,(cdr sym))))))) (getsym) p))
	    (else (error! 7) (getsym) '(none)))))
      (number
	(lambda ()
	  (let digit ((n 0))
	    (or 
	      (and (eqv? (car sym) 'chr)
	        (let ((d (cdr sym)))
		  (and (<= d 57) (>= d 48)
		    (begin (getsym) (digit (+ (* n 10) (- d 48)))))))
	       n))))
      (quantifier
	(lambda (p0)
	  (let ((n0 (number)))
	    (let from ((p (rx-newpat '(empty))) (n n0))
	      (or (and (> n 0) (from (rx-group p p0) (- n 1)))
		(and (eqv? (car sym) 'chr)
		  (or (and (eqv? (cdr sym) 125) p)
		    (and (eqv? (cdr sym) 44) 
		      (begin
			(getsym)
			(if (and (eqv? (car sym) 'chr) (eqv? (cdr sym) 125))
			  (rx-group p
			    (rx-choice (rx-newpat '(empty)) (rx-more p0)))
			  (let till ((p p) (n (- (number) n0)))
			    (if (> n 0)
			      (till
				(rx-group p 
				  (rx-choice (rx-newpat '(empty)) p0))
				(- n 1))
				p)))))))
		 (begin (error! 8) p))))))
      (piece
	(lambda ()
	  (let ((p (atom)))
	    (if (eqv? (car sym) 'chr)
	      (case (cdr sym)
	        ((63) ; ?
	  	  (getsym) (rx-choice p (rx-newpat '(empty))))
	        ((42) ; *
		  (getsym) (rx-choice (rx-more p) (rx-newpat '(empty))))
	        ((43) ; +
		  (getsym) (rx-more p))
	        ((123) ; {
		  (getsym)
		    (let ((p (quantifier p)))
		      (or (equal? sym '(chr . 125)) (error! "missing }"))
		      (getsym)
		      p))
	        (else p))
	      p))))
      (branch
	(lambda ()
	  (let loop ((p (rx-newpat '(empty))))
            (if(or (eqv? (car sym) 'end)
              (and (eqv? (car sym) 'chr) 
                (or (eqv? (cdr sym) 124) (eqv? (cdr sym) 41))))
              p
              (loop (rx-group p (piece)))))))
      (expr
	(lambda ()
	  (let loop ((p (branch)))
	    (if (equal? sym '(chr . 124))
	      (begin
		(getsym)
		(loop (rx-choice p (branch))))
	      p)))))
	
    (rx-newpat #f) (getsym) 	
    (let ((p (expr)))
      (or (equal? sym '(end)) (error! 9))
      p)))

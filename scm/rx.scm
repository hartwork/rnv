; $Id$
; XML Schema Datatypes Regular Expressions

(load  (in-vicinity (program-vicinity) "u.scm"))
(load  (in-vicinity (program-vicinity) "rx-ranges.scm"))

; patterns: none empty any choice group except range class char

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
	  (if (not (= (nextc) 123)) (begin (error!) #f)
	    (let i ((c (nextc)) (l '()))
	      (cond
	       ((not c) (error!) #f)
	       ((>= c 128) (error!) #f)
	       ((= c 125) 
		 (let (
		     (clsym
		       (string->symbol
			 (list->string 
			   `(#\u #\- 
			   ,@(map (lambda (i) (char-downcase (integer->char i))) 
			     (reverse l)))))))
		   (if (not (assv clsym rx-ranges)) (error! "unknown class" clsym))
		   clsym))
	       (else (i (nextc) (cons c l))))))))
      (esc
	(lambda ()
	  (let ((c (nextc)))
	    (if (not c) (begin (error!) '(end))
	      (if (>= c 128) (begin (error!) `(esc ,c))
		(case (integer->char c)
		  ((#\p) `(cls . ,(chclass))) 
		   ((#\P) `(except any . (cls . ,(chclass))))
		  ((#\s) '(cls . S)) 
		   ((#\S) '(except any . (cls . S)))
		  ((#\i) '(cls . I)) 
		   ((#\I) '(except any . (cls . I)))
		  ((#\c) '(cls . C)) 
		   ((#\C) '(except any . (cls . C)))
		  ((#\d) '(cls . U-Nd)) 
		   ((#\D) '(except any . (cls . U-Nd)))
		  ((#\w) '(cls . W)) 
		   ((#\W) '(except any . (cls . W)))
		  ((#\n) '(esc . 10))
		  ((#\r) '(esc . 13))
		  ((#\t) '(esc . 9))
		  ((#\\ #\| #\. #\- #\^ #\? #\* #\+ 
		      #\{ #\} #\[ #\] #\( #\))
		    `(esc . ,c))
		  (else (error!) `(esc . ,c))))))))
      (sym
	(lambda ()
	  (let ((c (nextc))) 
	    (if (not c) '(end)
	     (or (and (= c 92) (esc)) 
		 (and (= c 46) '(except any . (cls . NL)))
		`(chr . ,c))))))
      )
    (let loop ((s (sym)))
      (display s)
      (display #\newline)
      (or (eqv? (car s) 'end) (loop (sym))))))

; $Id$
; unicode

#define ux(u,c) (((u)<<6)|(c&0x3F))
#define u1(t) t[0]
#define u2(t) ux(t[0]&0x1F,t[1])
#define u3(t) ux(ux(t[0]&0xF,t[1]),t[2])
#define u4(t) ux(ux(ux(t[0]&0x7,t[1]),t[2]),t[3])
#define u5(t) ux(ux(ux(ux(t[0]&0x3,t[1]),t[2]),t[3]),t[4])
#define u6(t) ux(ux(ux(ux(ux(t[0]&0x1,t[1]),t[2]),t[3]),t[4]),t[5])

int u_get(int *up,char *s) {
  unsigned char *t=(unsigned char*)s;
  if(*t<0x80) {*up=u1(t); return 1;}
  if(*t<0xC0) return 0;
  if(*t<0xE0) {*up=u2(t); return (*up&B1)?2:0;}
  if(*t<0xF0) {*up=u3(t); return (*up&B2)?3:0;}
  if(*t<0xF8) {*up=u4(t); return (*up&B3)?4:0;}
  if(*t<0xFC) {*up=u5(t); return (*up&B4)?5:0;}
  if(*t<0xFE) {*up=u6(t); return (*up&B5)?6:0;}
  return 0;
}

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

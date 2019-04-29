 
;
;
; kserrequ.l  --  KServer routines which read a request from the port,
;                 and do something for it. (Probably return it to
;                 calling function....)
;

(defun penultimate (lista)
    (cond ((atom (cddr lista)) lista)
          (t (penultimate (cdr lista)))
    )
)

;
; Macros for handling the request:
;

(defmacro REQ_protocol_used (request)   `(car ,request))
(defmacro REQ_method (request)          `(cadr ,request))
(defmacro REQ_keywords (request)        `(cdr (memq '? ,request)))

;
; READ_request  --  read a request from the port, and check the method used.
;
; This reads a request line as the list formatted like:
; (items inside brackets [] are optiobal.)
;
; (method URL [? possible keywords] [possible_protocol])
;
; Only the method GET is implemented, and only the protocol
; HTTP/1.0 is recognized.
;
; If the request contains a valid method (that is, it begins with GET)
; then it's returned modified so that if there's a protocol specifier
; HTTP/1.0 in the end, it's transferred to the beginning, and if it's
; missing then nil is attached to the beginning of the request list.
;
; Currently the rest of the request sent by client is silently ignored.
; Of course we should check the accept-lines, and if there are no
; (Accept: image/gif) line we should send kanjis coded JIS or Shift-JIS or
; EUC or something instead of gifs (maybe those oriental Mosaics tell what
; they can handle in their request?)
;
;
(defun READ_request (portti &aux request protocol)
    (setq request (readline portti))
; If there is a protocol specified in the end of request line:
    (cond ((eq (car (setq protocol (last request))) 'HTTP/1.0)
; then delete it from the end of list:
              (rplacd (penultimate request) ())
              (rplacd protocol request) ; and move it to the beginning
              (setq request protocol) ; instead (without wasting cons-cells)
          )
          ((listp request) (attach () request)) ; Otherwise attach nil to the
; beginning of request to tell that there is no protocol HTTP/1.0 in use.
    )
)


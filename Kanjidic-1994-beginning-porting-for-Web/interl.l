
(cfasl 'interc.o '_init_table 'init_table "c-function")
(getaddress '_read_a_line 'read_a_line "c-function")

(cfasl 'sockfuns.o '_set_select 'set_select "c-function")
(getaddress '_clr_select 'clr_select "c-function")
(getaddress '_set_trace  'set_trace  "c-function")
(getaddress '_clr_trace  'clr_trace  "c-function")
(getaddress '_do_bind    'do_bind    "c-function")
(getaddress '_do_accept  'do_accept  "c-function")

 
(defun reverse_iota (x)
     (cond ((zerop x) (list x))
           (t (cons x (reverse_iota (1- x))))))

;
; (iota 256) produces just what we need.
;

(defun iota (x) (nreverse (reverse_iota x)))

(setq READLIST (iota 256))

(set_trace)
(set_select)


 
;
; Definition of while construct for the ordinary do of Common Lisp:
; (as well as for Franz Lisp)
;
(defmacro while (cond &rest body)
     `(do ((looPcounT 0 (1+ looPcounT)))
          ((not ,cond) looPcounT)
          ,@body
      )
)

 
(defun output_table (outport &aux i sym)
      (setq i 1)
      (while (lessp i 128)
         (cprintf "%3d." i outport)
         (drain outport)
         (cprintf "      %03o   " i outport)
         (cprintf "  0x%02x  " i outport)
         (setq sym (ascii i))
         (cond ((lessp i 32) (cprintf "^%s" (ascii (+ 64 i)) outport))
               ((eq i 127) (print 'DEL outport))
               (t (print sym outport))
         )
         (tab 40 outport)
         (print (getsyntax sym) outport)
         (terpri outport)
         (setq i (add1 i))
      )
)


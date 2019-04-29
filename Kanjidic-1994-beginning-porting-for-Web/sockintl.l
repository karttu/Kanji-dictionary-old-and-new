
;
; sockintl.l  --  an interface between Franz Lisp and UNIX socket-functions.
;
; Lisp module which loads needed C-functions from sockfuns.o and sockintc.o
;
;
; Copyright (C) 1994 by Antti Karttunen
;


(defun reverse_iota (x)
     (cond ((zerop x) (list x))
           (t (cons x (reverse_iota (1- x))))))

;
; (iota 256) produces just what we need.
;

; Name stolen from APL...
(defun iota (x) (nreverse (reverse_iota x)))

(setq READTABLELIST (cons 40 (cons 41 (iota 256))))

(set_trace)
(set_select)

(defun init_c_read_table (&aux i table)
     (setq table (cddr READTABLELIST)) ; Skip the parentheses in beginning.
; Set control-characters from ^@ to ^H and ^N to ^_ to nil so that they
; are simply skipped in input. Leave ^I (TAB) - ^M (CR) because Franz
; handles them as separators (in the same way as space).
     (setq i 0)
     (while (lessp i 9)
            (rplaca (nthcdr i table) nil)
            (setq i (1+ i))
     )
     (setq i 14)
     (while (lessp i 32)
            (rplaca (nthcdr i table) nil)
            (setq i (1+ i))
     )
; Replace left and right parentheses with spaces:
     (rplaca (nthcdr 40 table) (nth 32 table))
     (rplaca (nthcdr 41 table) (nth 32 table))
; For a moment skip also DEL and all 8-bit characters:
     (setq i 127)
     (while (lessp i 256)
            (rplaca (nthcdr i table) nil)
            (setq i (1+ i))
     )

     (init_read_table READTABLELIST 0)
; Substitute spaces for plusses when reading HTTP-lines:
     (rplaca (nthcdr #/+ table) (nth 32 table))
     (init_read_table READTABLELIST 1)
)

(init_c_read_table)

;
; The following characters have special interpretation in lisp reader
; which we want to disable when we read stuff from socket port.
;
;

(setq SPECIAL_CHARACTERS
         '(34  ;   "
           35  ;   #  (We don't want our client's to do #.(*process ...))
           39  ;   '
           44  ;   ,
           46  ;   .
           59  ;   ;
           91  ;   [
           92  ;   \
           93  ;   ]
           96  ;   `
           124 ;   |
          )
)

;
; Change syntaxes of characters in character list to those specified in
; syntax_list. If the latter argument is a circular list of one element,
; then of course the syntaxes of all the characters in character_list are
; changed to be that one and same syntax.
;
(defun change_syntax (character_list syntax_list &aux tmp)
     (mapcar #'(lambda (c syn)
                 (setq tmp (getsyntax (ascii c)))
                 (setsyntax (ascii c) syn)
                 tmp
               )
             character_list
             syntax_list
     )
)


(setq vcharacter_syntax_list (list 'vcharacter))
(rplacd vcharacter_syntax_list vcharacter_syntax_list) ; Make a circular list.

(defun set_server_io (&aux tmp)
; "Castrate" the following characters, so that
;  deviant clients can't choke our reading system with malformed input.
     (setq tmp (change_syntax SPECIAL_CHARACTERS vcharacter_syntax_list))
; Handle question-mark as single-character-symbol, so it's easier to
; parse HTTP-requests:
     (setsyntax '? 'vsingle-character-symbol)
     tmp
)


(defun set_lisp_io (restore_list &aux tmp)
; Restore the previous situation.
     (change_syntax SPECIAL_CHARACTERS restore_list)
)

;
; We reuse READTABLELIST after read table initialization by using it
; as a result parameter for read_a_line_to_list. The line read in is
; rplaca'ed to the elements of READTABLELIST, which is much faster
; than consing it every time again.
;
(defun readline (port &aux result saved_syntaxes)
       (read_HTTP_line_to_list port READTABLELIST)
       (setq saved_syntaxes (set_server_io)) ; Shouldn't do consing every time.
       (setq result (readlist READTABLELIST)) ; Read the chars from list.
; This is here so that we can enter some input to interpreter after this,
; when we still test this function in the interpreter.
       (set_lisp_io saved_syntaxes)
       result
)

;
; Strictly for testing:
;
(defun readlines (port)
       (while (patom (readline port)) (terpri))
)



;
; Kanji Server - A LISP based server software which converts incoming
; references to HTML-documents, which are then sent on-the-fly back to
; Mosaic, or some other client.
; Copyright (C) 1994  Antti Karttunen
;
; Most of this stuff is converted almost directly from KANJIDIC, which
; is an electronic Kanji Dictionary for PC-clones, which is also coded
; (with Lisp-like C-routines) by Antti Karttunen.
;

(include "kanjimac.l")

(defun list_insert (list_item lista n &aux tmp)
    (declare (fixnum n))
    (cond ((zerop n) ; To be inserted as first item?
             (list_insert (cdr list_item)
                          (attach (car list_item) lista)
                          1
             )
          )
          (t (setq tmp (nthcdr (1- n) lista))
             (rplacd tmp (nconc list_item (cdr tmp)))
             lista
          )
    )
)


; If sym is a primitive then return "alias-root" of it:
(defun solve_alias (sym &aux tmp)
    (cond ((and (nonnilsymbolp sym)
                (nonnilsymbolp (setq tmp (word_eval sym))))
           tmp)
          (t sym)
    )
)

;
; This gets entry corresponding to refer, which can be:
;    keyword
;    NI-index (positive number)
;    JI-index (negative number)
;    (word . one-of-the-above)
;

(defun get_entry (refer)
    (cond ((consp refer) (setq refer (cdr refer))))
    (cond ((nonnilsymbolp refer) ; It's a HK-keyword (or a primitive-name)
             (setq refer (word_eval (solve_alias refer)))
             (cond ((single_entryp refer) refer)
                   (t (car refer)) ; If mult. entries with this name take first
             )
          )
          ((fixp refer)
             (cond ((lessp refer 1) (nth_entry (- refer) ':JI)) ; JIS-INDEX?
                   (t (nth_entry refer ':NI)) ; No, it's NI-INDEX
             )
          )
          (t ())
    )
)

;
; Returns entry as result, rplaca's prsymb to the cons cell given as 1st arg:
;
(defun getentry_et_prsymb (ptrtosym refer &aux entry word)
    (setq word ()) ; To be sure it's nil by default?
    (cond ((consp refer) (setq word (car refer)) (setq refer (cdr refer))))
    (setq entry (get_entry refer))
    (cond ((null word) ; If there was no explicit word given in reference
             (cond ((nonnilsymbolp refer) (setq word refer)) ; Ref. itself?
                   (t (setq word (getkeyword entry))) ; No, we have to dig it.
             )
          )
    )
    (rplaca ptrtosym word)
    entry
)


(defun getfield (key entry &aux loc)
   (cond ((fixp (setq loc (getfieldloc key)))
             (setq entry (getpermpart entry))
             (cond ((eq (car entry) '*) ())
                   (t (nth loc entry))
             )
          )
         (t ; Else it's a normal key which is actually there in the entry.
             (cond ((flagp key) (car (memq key entry)))
                   (t           (cadr (memq key entry)))
             )
         )
   )
)

(defun getlfield (key entry &aux result)
    (setq result (getfield key entry))
    (cond ((singlereferp result) (list result))
          (t result)
    )
)

(defun get_first_word (arg)
    (cond ((consp arg) (get_first_word (car arg)))
           (t arg)
    )
)


;
; Oh yeah, some maxiperverted code again. KEYWORD_PRIORITIES is given
; as argument to mapcan, which loops over it. However, when the first
; key having contents is found from entry we immediately break from
; the loop with throw, which returns the field contents corresponding
; to that key. However, if none of the key's in KEYWORD_PRIORITIES
; list produces anything when given to getfield, then mapcan nconcs
; those nil's together resulting nil, which in turn is given as
; argument to get_first_word which also returns nil from that.
;
(defun getkeyword (entry)
    (cond ((null entry) ())
          (t
             (get_first_word
                (catch
                  (mapcan #'(lambda (key)
                              (cond ((setq key (getfield key entry))
; If found key whose field is non-nil, then escape from mapcan loop with that
; field as a value:
                                        (throw key GETKEYWORD_TAG))
                              )
; If the above clause in cond didn't match then this lambda-form returns
; nil, and when they are nconced together by mapcan it's still nil.
                            )
                          KEYWORD_PRIORITIES
                  )
                  GETKEYWORD_TAG
                ) ; catch
             ) ; get_first_word
          ) ; t
    ) ; cond
)

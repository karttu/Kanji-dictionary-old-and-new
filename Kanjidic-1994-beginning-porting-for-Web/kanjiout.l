 
;
; Output functions, which convert data structures implemented with lisp lists
; to HTML-documents.
;
; Copyright (C) Antti Karttunen, 1994.
;
;

(include "kanjimac.l")

(defun output_the_database
  (mode outputti &optional PRINT_TEMP &aux entry index uplim)
    (declare (fixnum index uplim))
    (setq outputti (outfile outputti))
    (setq index 0)
    (setq uplim (1+ (getuplim mode)))
    (while (lessp index uplim)
      (setq entry (nth_entry index mode))
      (print (cond (PRINT_TEMP (gettemppart entry)) (t (getpermpart entry)))
             outputti)
      (terpri outputti)
      (setq index (1+ index))
    ) ; while
    (close outputti)
)


(defun getjis_index (entry)
    (cond ((fixp (setq entry (get_JI_field entry))) entry)
          (t INVALID_KANJI_INDEX)
    )
)

(defun putexpr (expression output &aux nakki)
    (declare (fixnum nakki))
    (setq expression
           (cond ((and (consp expression) (not (listp (cdr expression))))
                     (car expression)
                 )
                 (t expression)
           )
    )
    (cond ((or (symbolp expression) (stringp expression))
             (cond ((primitivep expression)
                       (cprintf "<I>%s</I>" (substring expression 2) output)
                   )
                   ((kanap expression)
                       (cprintf "<TT>%s" (substring expression 2) output)
                       (cond ((plusp (setq nakki (- 4 (pntlen expression))))
                                (while (not (zerop nakki))
; Print non-breakable space:
                                    (patom "&#160;" output)
                                    (setq nakki (1- nakki))
                                )
                             )
                       )
                       (patom "</TT>" output)
                   )
                   (t (patom expression output))
             )
          )
          (t (patom expression output))
    )
)


(defun putexpr_m (expression output)
    (print_reference expression 
                     (cond ((consp expression) (car expression))
                           (t expression)
                     )
                     output
    )
)

(defun print_reference (reference text output)
    (patom "<A HREF=\"kanjiser.cgi?" output)
    (cond ((consp reference)
; Print the dotted-pair references in the format ((word)real_ref)
; instead of the natural format (word . real_ref)
; (Because the latter format gets screwed up when transferred from mosaic
;  to kanjiserver).
             (patom "((" output)
             (patom (car reference) output)
             (patom ")" output)
             (patom (cdr reference) output)
             (patom ")" output)
          )
          (t (patom reference output))
    )
    (patom "\">" output)
    (putexpr text output)
    (patom "</A>" output)
    (terpri output)
)

(defun wrap_index (index mode)
    (cond
          ((eq mode ':JI)
             (cond ((minusp index)            JI_UPLIM)
                   ((greaterp index JI_UPLIM)        0)
                   (t index)
             )
          )
          ((lessp index 1)
               (setq index (cond ((eq mode ':NI) HIGHEST_NI)
                                 ((eq mode ':HI) HI_UPLIM)
                                 (t 1)
                           )
               )
          )
          ((greaterp index (cond ((eq mode ':NI) HIGHEST_NI)
                                 ((eq mode ':HI) HI_UPLIM)
                                 (t 1)
                           )
           ) 1)
          (t index)
    ) ; cond
)

(defun getrefbynum (index mode)
    (cond ((eq mode ':NI) index)
          ((eq mode ':JI) (- index))
          ((eq mode ':HI) (getfield ':HK (nth_entry index mode)))
          (t (list 'No 'reference 'for 'mode mode 'with 'index index))
    )
)

(defun print_index_line (entry mode output)
; If entry is already fixnum (i.e. JIS-index, should be positive!)
; then keep it intact:
    (cond ((or (fixp entry) (setq entry (getfield mode entry)))
              (cprintf "<DT>%s " (getplaintext mode) output)
              (print_reference (getrefbynum (wrap_index (1- entry) mode) mode)
                               "&lt;"
                               output
              )
              (patom entry output)
              (print_reference (getrefbynum (wrap_index (1+ entry) mode) mode)
                               "&gt;"
                               output
              )
              (patom "<DD>" output)
              t
          )
          (t nil)
    )
)


(defun print_JI_field (entry output)
    (cond ((print_index_line entry ':JI output)
;            (putexpr "[JIS]" output) ; Do nothing now.
          )
    )
)

(defun print_HK_field (entry output)
    (cond ((print_index_line entry ':HI output)
             (printlistofkeywords (getfield ':HK entry) output)
          )
    )
)

(defun print_NK_field (entry output &aux keywords readings)
    (cond ((print_index_line entry ':NI output)
             (setq keywords (getfield ':NK entry))
             (setq readings (getfield ':R  entry))
             (cond ((atom keywords)
                       (print_keywords_et_readings keywords readings output))
                   ((null (cdr keywords)) ; length is one
                       (print_keywords_et_readings
                                             (car keywords) readings output))
                   (t (mapc #'(lambda (x)
                                (print_keywords_et_readings x (car readings)
                                                                       output)
                                (patom "<BR>" output) (terpri output)
                                (setq readings (cdr readings))
                              )
                            keywords
                      )
                   ) ; t
             ) ; cond
          ) ; setq
    ) ; cond
)
 
(defun printlistofkeywords (lista output)
    (cond ((null lista) ())
          ((or (atom lista) (not (listp (cdr lista))))
                     (putexpr lista output) (terpri output))
          (t (putexpr (car lista) output)
             (mapc #'(lambda (x)
                        (patom COMMA output)
                        (patom SPACE output)
                        (putexpr x output)
                     )
                   (cdr lista)
             )
             (terpri output)
          )
    )
)

 
(defun printlistofkeywords_m (lista output)
    (cond ((null lista) ())
          ((or (atom lista) (not (listp (cdr lista))))
                     (putexpr_m lista output) (terpri output))
          (t (putexpr_m (car lista) output)
             (mapc #'(lambda (x)
                        (patom COMMA output)
                        (patom SPACE output)
                        (putexpr_m x output)
                     )
                   (cdr lista)
             )
             (terpri output)
          )
    )
)


(defun printlistofkeywords2m (lista output)
    (cond ((null lista) ())
          ((or (atom lista) (not (listp (cdr lista))))
                     (putexpr_m lista output) (terpri output))
          (t (putexpr_m (car lista) output)
             (mapc #'(lambda (x)
                        (patom SPACE output)
                        (patom "+"   output)
                        (patom SPACE output)
                        (putexpr_m x output)
                     )
                   (cdr lista)
             )
             (terpri output)
          )
    )
)

(defun print_keywords_et_readings (keywords readings output)
    (printlistofkeywords keywords output)
    (printlistofkeywords readings output)
)


(defun printfield (entry key output)
    (cond ((setq entry (getfield key entry))
             (cprintf "<DT>%s:<DD>" (getplaintext key) output)
             (printlistofkeywords entry output)
          )
    )
)

;
; Like previous, but marks the keyword(s) contained in the field, i.e.
; puts there link(s) to similarly named references:
;

(defun printfield_m (entry key output)
    (cond ((setq entry (getfield key entry))
             (cprintf "<DT>%s:<DD>" (getplaintext key) output)
             (printlistofkeywords_m entry output)
          )
    )
)

(defun printreferfield (entry key output)
    (cond ((setq entry (getlfield key entry))
             (cprintf "<DT>%s:<DD>" (getplaintext key) output)
             (printkanjis_with_keywords entry output)
          )
    )
)

(setq RESPAIR1 (cons () ()))

(defun printkanjis_with_keywords (referlist output &aux entry)
    (mapc
       #'(lambda (x)
            (cond ((null x) ; Show empty font (JIS-index zero) for NIL
                            (patom "<IMG SRC=\"jg.cgi?0\">" output)
; And print three non-breakable spaces (for $HIRAGANA & $KATAKANA): (kludge!)
                            (patom "<TT>&#160;&#160;&#160;</TT>" output)
                  )
                  ((eq x SLASH) (patom "<BR>" output) (terpri output))
                  (t
                     (setq entry (getentry_et_prsymb RESPAIR1 x))
                     (cprintf "<IMG SRC=\"jg.cgi?%d\">"
                              (getjis_index entry)
                              output
                     )
                     (print_reference x (car RESPAIR1) output)
                  )
            ) ; cond
         ) ; lambda
       referlist
    ) ; mapc
)

(defun printkanjis_without_keywords (referlist output &aux entry)
    (mapc
       #'(lambda (x)
            (cond ((null x) (patom "<IMG SRC=\"jg.cgi?0\">" output))
                  ((eq x SLASH) (patom "<BR>" output) (terpri output))
                  (t
                     (setq entry (get_entry x))
                     (cprintf "<IMG SRC=\"jg.cgi?%d\">"
                              (getjis_index entry)
                              output
                     )
                  )
            ) ; cond
         ) ; lambda
       referlist
    ) ; mapc
)

(defun print_compound (compound output)
    (printkanjis_without_keywords (getcompound_refers compound) output)
    (printlistofkeywords2m (getcompound_refers compound) output)
    (patom " = " output)
    (printlistofkeywords (getcompound_meanings compound) output)
    (printlistofkeywords (getcompound_readings compound) output)
    (patom "<BR>" output)
    (terpri output)
)

(defun print_compounds (compound_list output)
    (mapc #'(lambda (x) (print_compound x output)) compound_list)
)

(defun print_COMPOUNDS_field (entry output)
    (cond ((setq entry (getfield ':COMPOUNDS entry))
             (cprintf "<DT>%s:<DD>" (getplaintext ':COMPOUNDS) output)
             (print_compounds entry output)
          )
    )
)

 
(defun print_group (groupname output &aux group comment)
    (setq group (word_eval groupname))
    (cprintf "<TITLE>%s</TITLE>" groupname output)
    (patom "</HEAD><BODY>" output) (terpri output)
    (cprintf "<H2>%s</H2>" groupname output) (terpri output)
    (cond ((setq comment (getgroup_comment group))
              (patom comment output)
              (patom "<P>" output)
              (terpri output)
          )
    )
    (cond ((compoundgroupnamep groupname)
              (print_compounds (getgroup_reflist group) output)
          )
          (t  (printkanjis_with_keywords (getgroup_reflist group) output))
    )
)

 
(defun print_out_of_limits_msg (numrefer output)
    (cprintf "<TITLE>%d</TITLE>" numrefer output)
    (patom "</HEAD><BODY>" output) (terpri output)
    (cprintf "<H2>Index %d is out of limits!</H2>" numrefer output)
    (terpri output)
)


(defvar JIS_KEYWORD (list 'JIS 'NOT_SET_YET))

(defun print_entry (refer output &aux entry title tmprefer)
; If reference is of the format ((word)real_ref) then change it to format
; (word . real_ref)
    (princ "Content-type: text/html" output) (terpri output) (terpri output)
;   (princ `(print_entry: refer is now ,refer)) (terpri)
    (cond ((and (consp refer) (consp (car refer)))
                  (rplaca refer (caar refer))
                  (rplacd refer (cadr refer))
          )
    )
;   (princ `(print_entry: refer is now ,refer)) (terpri)
    (patom "<HEAD>" output)
    (cond
      ((and (or (fixp (setq tmprefer refer))
                (and (consp refer) (fixp (setq tmprefer (cdr refer))))
            )
            (or (greaterp tmprefer HIGHEST_NI)
                (lessp tmprefer (- JI_UPLIM))
            )
       )
         (print_out_of_limits_msg tmprefer output)
      )
      ((and (or (symbolp refer) (stringp refer)) (groupnamep refer))
          (print_group refer output)
      )
      (t ; It's a normal entry
         (setq entry (get_entry refer))
         (patom "<TITLE>" output)
         (cond ((setq title (getkeyword entry)) title)
               (t (rplaca (cdr JIS_KEYWORD) refer)
                  (setq title JIS_KEYWORD)
               )
         )
         (putexpr title output)
         (patom "</TITLE></HEAD>" output)
         (terpri output)
         (cprintf "<IMG SRC=\"jg.cgi?%d\">"
                  (cond ((and (fixp refer) (lessp refer 1)) (- refer))
                        (t (getjis_index entry))
                  )
                  output
         )
         (terpri output)
         (patom "<BODY>" output)
         (patom "<DL>" output) (terpri output)
         (print_JI_field
                          (cond ((and (fixp refer) (lessp refer 1)) (- refer))
                                (t entry)
                          )
                         output
         )
         (print_HK_field entry output)
         (print_NK_field entry output)
         (printfield entry ':HIRAGANA output)
         (printfield entry ':KATAKANA output)
         (printfield entry ':OTHER    output)
         (printfield entry ':P        output)
         (printfield entry ':COMMENT  output)
         (printreferfield entry ':E     output)
         (printreferfield entry ':S     output)
         (printreferfield entry ':SEE   output)
         (printfield_m entry ':G output)
         (print_COMPOUNDS_field entry output)
         (patom "</DL>" output) (terpri output)
      ) ; t
    ) ; cond
    (patom "</BODY>" output) (terpri output) (drain output)
)

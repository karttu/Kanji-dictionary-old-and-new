 
;
; Output functions, which convert data structures implemented with lisp lists
; to HTML-documents.
;
; Copyright (C) Antti Karttunen, 1994.
;
;

(include "kserdefs.l")

(defun SEND_newline (portti)
    (princ (ascii 13) portti) ; HTTP protocol requires the CR +
    (terpri portti)           ;                                 LF
; (Franz Lisp's terpri does also the flush.)
)

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
      (SEND_newline outputti)
      (setq index (1+ index))
    ) ; while
    (close outputti)
)

(defmacro SEND_stuff (X output) `(patom ,X ,output))
 
(defun SEND_jis (index output)
    (cprintf "<IMG SRC=\"%s/nph-jg.cgi?" ANCHOR_ROOT output)
    (cprintf "%d\">" index output)
    (SEND_newline output)
)


(defun SEND_back2to_refer (portti)
    (cprintf "<A HREF=\"%s/ksertop.html\">"
             ANCHOR_ROOT
             portti
    )
    (cprintf "<IMG SRC=\"%s/back2top.gif\"></A>"
             ANCHOR_ROOT
             portti
    )
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
                                    (SEND_stuff "&#160;" output)
                                    (setq nakki (1- nakki))
                                )
                             )
                       )
                       (SEND_stuff "</TT>" output)
                   )
                   (t (SEND_stuff expression output))
             )
          )
          (t (SEND_stuff expression output))
    )
)


(defun putexpr_m (expression output)
    (SEND_reference expression 
                     (cond ((consp expression) (car expression))
                           (t expression)
                     )
                     output
    )
)

(defun SEND_reference (reference text output)
    (SEND_stuff "<A HREF=\"/KSERVER?" output)
    (cond ((consp reference)
; Print the dotted-pair references in the format ((word)real_ref)
; instead of the natural format (word . real_ref)
; (Because the latter format gets screwed up when transferred from mosaic
;  to kanjiserver).
;  Some lines commented out 19-DEC-94, so that now this prints out only
;  the cdr-part (i.e. the reference itself), because our new 'better'
;  reader doesn't recognize the parentheses at all (because we want to
;  avoid mismatched parentheses in input, either accidentally or
;  incidentally sent by misbehaving client.)
;            (SEND_stuff "((" output)
;            (SEND_stuff (car reference) output)
;            (SEND_stuff ")" output)
             (SEND_stuff (cdr reference) output)
;            (SEND_stuff ")" output)
          )
          (t (SEND_stuff reference output))
    )
    (SEND_stuff "\">" output)
    (putexpr text output)
    (SEND_stuff "</A>" output)
    (SEND_newline output)
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

(defun SEND_index_line (entry mode output)
; If entry is already fixnum (i.e. JIS-index, should be positive!)
; then keep it intact:
    (cond ((or (fixp entry) (setq entry (getfield mode entry)))
              (cprintf "<DT>%s " (getplaintext mode) output)
              (SEND_reference (getrefbynum (wrap_index (1- entry) mode) mode)
                               "&lt;"
                               output
              )
              (SEND_stuff entry output)
              (SEND_reference (getrefbynum (wrap_index (1+ entry) mode) mode)
                               "&gt;"
                               output
              )
              (SEND_stuff "<DD>" output)
              t
          )
          (t nil)
    )
)


(defun SEND_JI_field (entry output)
    (cond ((SEND_index_line entry ':JI output)
;            (putexpr "[JIS]" output) ; Do nothing now.
          )
    )
)

(defun SEND_HK_field (entry output)
    (cond ((SEND_index_line entry ':HI output)
             (printlistofkeywords (getfield ':HK entry) output)
          )
    )
)

(defun SEND_NK_field (entry output &aux keywords readings)
    (cond ((SEND_index_line entry ':NI output)
             (setq keywords (getfield ':NK entry))
             (setq readings (getfield ':R  entry))
             (cond ((atom keywords)
                       (SEND_keywords_et_readings keywords readings output))
                   ((null (cdr keywords)) ; length is one
                       (SEND_keywords_et_readings
                                             (car keywords) readings output))
                   (t (mapc #'(lambda (x)
                                (SEND_keywords_et_readings x (car readings)
                                                                       output)
                                (SEND_stuff "<BR>" output) (SEND_newline output)
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
                     (putexpr lista output) (SEND_newline output))
          (t (putexpr (car lista) output)
             (mapc #'(lambda (x)
                        (SEND_stuff COMMA output)
                        (SEND_stuff SPACE output)
                        (putexpr x output)
                     )
                   (cdr lista)
             )
             (SEND_newline output)
          )
    )
)

 
(defun printlistofkeywords_m (lista output)
    (cond ((null lista) ())
          ((or (atom lista) (not (listp (cdr lista))))
                     (putexpr_m lista output) (SEND_newline output))
          (t (putexpr_m (car lista) output)
             (mapc #'(lambda (x)
                        (SEND_stuff COMMA output)
                        (SEND_stuff SPACE output)
                        (putexpr_m x output)
                     )
                   (cdr lista)
             )
             (SEND_newline output)
          )
    )
)


(defun printlistofkeywords2m (lista output)
    (cond ((null lista) ())
          ((or (atom lista) (not (listp (cdr lista))))
                     (putexpr_m lista output) (SEND_newline output))
          (t (putexpr_m (car lista) output)
             (mapc #'(lambda (x)
                        (SEND_stuff SPACE output)
                        (SEND_stuff "+"   output)
                        (SEND_stuff SPACE output)
                        (putexpr_m x output)
                     )
                   (cdr lista)
             )
             (SEND_newline output)
          )
    )
)

(defun SEND_keywords_et_readings (keywords readings output)
    (printlistofkeywords keywords output)
    (printlistofkeywords readings output)
)


(defun SEND_field (entry key output)
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

(defun SEND_field_m (entry key output)
    (cond ((setq entry (getfield key entry))
             (cprintf "<DT>%s:<DD>" (getplaintext key) output)
             (printlistofkeywords_m entry output)
          )
    )
)

(defun SEND_referfield (entry key output)
    (cond ((setq entry (getlfield key entry))
             (cprintf "<DT>%s:<DD>" (getplaintext key) output)
             (SEND_kanjis_with_keywords entry output)
          )
    )
)

(setq RESPAIR1 (cons () ()))

(defun SEND_kanjis_with_keywords (referlist output &aux entry)
    (mapc
       #'(lambda (x)
            (cond ((null x) ; Show empty font (JIS-index zero) for NIL
                            (SEND_jis 0 output)
; And print three non-breakable spaces (for $HIRAGANA & $KATAKANA): (kludge!)
                            (SEND_stuff "<TT>&#160;&#160;&#160;</TT>" output)
                  )
                  ((eq x SLASH) (SEND_stuff "<BR>" output) (SEND_newline output))
                  (t
                     (setq entry (getentry_et_prsymb RESPAIR1 x))
                     (SEND_jis
                              (getjis_index entry)
                              output
                     )
                     (SEND_reference x (car RESPAIR1) output)
                  )
            ) ; cond
         ) ; lambda
       referlist
    ) ; mapc
)

(defun SEND_kanjis_without_keywords (referlist output &aux entry)
    (mapc
       #'(lambda (x)
            (cond ((null x) (SEND_jis 0 output))
                  ((eq x SLASH) (SEND_stuff "<BR>" output) (SEND_newline output))
                  (t
                     (setq entry (get_entry x))
                     (SEND_jis (getjis_index entry) output)
                  )
            ) ; cond
         ) ; lambda
       referlist
    ) ; mapc
)

(defun SEND_compound (compound output)
    (SEND_kanjis_without_keywords (getcompound_refers compound) output)
    (printlistofkeywords2m (getcompound_refers compound) output)
    (SEND_stuff " = " output)
    (printlistofkeywords (getcompound_meanings compound) output)
    (printlistofkeywords (getcompound_readings compound) output)
    (SEND_stuff "<BR>" output)
    (SEND_newline output)
)

(defun SEND_compounds (compound_list output)
    (mapc #'(lambda (x) (SEND_compound x output)) compound_list)
)

(defun SEND_COMPOUNDS_field (entry output)
    (cond ((setq entry (getfield ':COMPOUNDS entry))
             (cprintf "<DT>%s:<DD>" (getplaintext ':COMPOUNDS) output)
             (SEND_compounds entry output)
          )
    )
)

 
(defun SEND_group (groupname request output &aux group comment)
    (setq group (word_eval groupname))
    (cprintf "<HEAD><TITLE>%s</TITLE>" groupname output)
    (SEND_stuff "</HEAD><BODY>" output) (SEND_newline output)
    (cprintf "<H2>%s</H2>" groupname output) (SEND_newline output)
    (cond ((setq comment (getgroup_comment group))
              (SEND_stuff comment output)
              (SEND_stuff "<P>" output)
              (SEND_newline output)
          )
    )
    (cond ((compoundgroupnamep groupname)
              (SEND_compounds (getgroup_reflist group) output)
          )
          (t  (SEND_kanjis_with_keywords (getgroup_reflist group) output))
    )
    (SEND_stuff "</BODY>" output) (SEND_newline output)
    (sclose output)
)

 
(defvar JIS_KEYWORD (list 'JIS 'NOT_SET_YET))

(defun SEND_header (header_name header_contents portti)
    (princ header_name      portti)
    (princ ": "             portti)
    (princ header_contents  portti)
    (SEND_newline           portti)
)


(defun SEND_response_headers (status msg request portti)
; Send status-message and response-headers only if the client uses
; protocol. (Otherwise we skip directly to sending the body.)
    (cond ((REQ_protocol_used request)
             (cprintf "HTTP/1.0 %d " status portti)     ; The status
             (princ msg portti) (SEND_newline portti)   ;             line.
             (SEND_header "Date"           (get_HTTP_date)     portti)
             (SEND_header "Last-Modified"  STARTING_TIME       portti)
             (SEND_header "Server"         "KServer/1.0"       portti)
             (SEND_header "MIME-version"   "1.0"               portti)
             (SEND_header "Content-Type"   "text/html"         portti)
             (SEND_newline portti) ; One empty line completes the header.
          )
    )
)



(defun SEND_method_not_implemented (request portti &aux status)
; Trace to logfile:
    (princ "KServer: discarding invalid request: ") (princ request)
    (terpri)
    (setq status 501)
    (SEND_response_headers status "Invalid or unimplemented method"
          request portti)
    (cprintf "<HEAD><TITLE>%d Invalid or unimplemented method.</TITLE></HEAD>"
             status
             portti
    )
    (SEND_newline portti)
    (cprintf "<BODY><H1>%d Invalid or unimplemented method: "
             status
             portti
    )
    (print (REQ_method request) portti)
    (SEND_newline portti)
    (princ "</H1>This server supports only GET method.</BODY>" portti)
    (SEND_newline portti)
    (sclose portti)
    ()
)


(defun SEND_invalid_reference (refer status msg request portti)
; Trace to logfile:
    (princ "KServer: received invalid reference: ") (princ request)
    (terpri)
    (SEND_response_headers status msg request portti)
    (cprintf "<HEAD><TITLE>%d "   status portti)
    (cprintf "%s</TITLE></HEAD>"  msg    portti)
    (SEND_newline portti)
    (cprintf "<BODY><H1>%d "      status portti)
    (cprintf "%s: "               msg    portti)
    (print refer portti)
    (SEND_stuff "</H1></BODY>"           portti)
    (SEND_newline portti)
    (sclose portti)
    ()
)

(defun SEND_isindex_page (request portti)
; Trace to logfile:
    (princ "KServer: send isindex page for request: ") (princ request)
    (terpri)
    (SEND_response_headers 200 "OK" request portti)
    (SEND_stuff "<HEAD><TITLE>KServer</TITLE></HEAD><BODY><H1>KServer</H1>"
                portti)
    (SEND_newline portti)
    (SEND_stuff "<ISINDEX></BODY>" portti)
    (SEND_newline portti)
    (sclose portti)
    ()
)

(defun check_reference (refer request portti)
    (cond
          ((or (null refer) (and (not (fixp refer)) (not (symbolp refer))))
             (SEND_invalid_reference refer
                                     400
                                     "Invalid reference"
                                     request
                                     portti
             )
          )
          ((and (fixp refer)
                (or (greaterp refer HIGHEST_NI)
                    (lessp refer (- JI_UPLIM))
                )
           )
             (SEND_invalid_reference refer
                                     400
                                     "Index is out of limits"
                                     request
                                     portti
             )
          )
          ((and (symbolp refer) (null (word_eval refer)))
             (SEND_invalid_reference refer
                                     404
                                     "Not found"
                                     request
                                     portti
             )
          )
          (t refer)
    )
)

(defun handle_element_search (keywords request portti)
     (SEND_invalid_reference keywords
                                   501
                                   "Element searches not implemented yet"
                                   request
                                   portti
     )
)


(defun handle_request (request output &aux keywords)
    (cond
       ((neq (REQ_method request) 'GET) ; Check that there is a valid method.
           (SEND_method_not_implemented request output)
       )
       ((not (setq keywords (REQ_keywords request)))
           (SEND_isindex_page request output))
       ((consp (cdr keywords)) (handle_element_search keywords request output))
       (t (handle_simple_request (car keywords) request output))
    )
)

(defun handle_simple_request (refer request output)
    (cond ((check_reference refer request output)
              (SEND_response_headers 200 "OK" request output)
              (cond ((and (symbolp refer) (groupnamep refer))
                        (SEND_group refer request output))
                    (t (handle_normal_reference refer request output))
              )
          )
    )
)

(defun handle_normal_reference (refer request output &aux entry title tmprefer)
    (setq entry (get_entry refer))
    (cond ((setq title (getkeyword entry)) title)
          (t (rplaca (cdr JIS_KEYWORD) refer)
             (setq title JIS_KEYWORD)
          )
    )
    (SEND_stuff "<HEAD><TITLE>" output)
    (putexpr title output)
    (SEND_stuff "</TITLE></HEAD><BODY>" output)
    (SEND_newline output)
    (SEND_jis
                (cond ((and (fixp refer) (lessp refer 1)) (- refer))
                      (t (getjis_index entry))
                )
                output
    )
    (SEND_stuff "<DL>" output)
    (SEND_JI_field
                     (cond ((and (fixp refer) (lessp refer 1)) (- refer))
                           (t entry)
                     )
                     output
    )
    (SEND_HK_field entry output)
    (SEND_NK_field entry output)
    (SEND_field entry ':HIRAGANA output)
    (SEND_field entry ':KATAKANA output)
    (SEND_field entry ':OTHER    output)
    (SEND_field entry ':P        output)
    (SEND_field entry ':COMMENT  output)
    (SEND_referfield entry ':E     output)
    (SEND_referfield entry ':S     output)
    (SEND_referfield entry ':SEE   output)
    (SEND_field_m entry ':G output)
    (SEND_COMPOUNDS_field entry output)
    (SEND_stuff "</DL>" output)
    (SEND_back2to_refer output)
    (SEND_stuff "</BODY>" output) (SEND_newline output) (sclose output)
)
 

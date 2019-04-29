
;
; Module for Kanji Server, Copyright (C) 1994 by Antti Karttunen
;

;
; Stuff for reading in the database and postformatting it:
; (Note: after you have done this, it's good to take a dump of
;  lisp system, because this is really time-consuming.)
;

;
; This module is just a kludge for converting stuff to SQL INSERT
; statements that are to be entered into msql.
; (pass4, etc.)
;

;
; Note the author's (that's me!) disturbing tendency to use catch - throw
; constructs with mapcan and mapcon to build the loops which can be
; escaped prematurely.
;

(include "ksergen.l")

(defun read_database (file &aux entry i true_ni)
    (declare (fixnum i))
    (setq file (infile file))
    (setq i 0)
    (while (setq entry (read file))
        (cond ((greaterp i NI_UPLIM)
                 (princ "read_database: database contains more than ")
                 (princ NI_UPLIM)
                 (princ " entries, truncated before:") (terpri)
                 (print entry)
                 (terpri)
                 (return ())
               )
              ((zerop (\\ i 100))
                  (print i) (princ (ascii 13)) (drain)
              )
         )
         (setq entry (cons entry entry)) ; Make a permanent - temporary pair
         (cond ((fixp (setq true_ni (getfield ':NI entry)))
                    (setq HIGHEST_NI true_ni)
               )
         )
         (set_NI_entry i entry)
         (setq i (1+ i))
    )
    (close file)
    (terpri)
    (princ "read_database: entries read: ")
    (princ i)
    (terpri)
    (setq NI_UPLIM (sub1 i))
)

(defun get_JI_refer (entry)
    (cond ((fixp (setq entry (get_JI_field entry))) (- entry))
          (t nil)
    )
)

;
; Construct reference which references to this entry: (given as argument)
;

(defun getrefer (entry &aux jisrefer x)
    (setq jisrefer (get_JI_refer entry))
; If there is Heisig-keyword defined for this entry, then use it:
    (cond ((nonnilsymbolp (setq x (getfield ':HK entry))) x)
          ((fixp (setq x (getfield ':NI entry))) x) ; If there is NI-index...
          ((or (nonnilsymbolp (setq x (getfield ':HIRAGANA entry)))
               (nonnilsymbolp (setq x (getfield ':KATAKANA entry)))
           ) (cond ((single_entryp (word_eval x)) x) ; If unique kana-name
                   (t jisrefer) ; otherwise we have to use jis-reference
             )
          )
; If there is one or more primitive names defined for this entry, then use
; the first one of them:
          ((setq x (getfield ':P entry)) (get_first_word x))
; If there is :OTHER field in the entry, and its keyword refers unambiguosly
; to this entry, then use that as reference:
          ((and (setq x (get_first_word (getfield ':OTHER entry)))
                (single_entryp (word_eval x))) x)
          (t jisrefer) ; Use JIS-index as the last change.
    )
)


;
; If in the field (given by key given as the first argument to function)
; of the other_entry is reference to this_entry, then return non-nil,
; otherwise nil.
;

(defun refbyfieldp (field other_entry this_entry)
     (catch
           (mapcan #'(lambda (refer)
                        (cond ((and (neq refer '!SOMETHING)
                                    (eq (get_entry refer) this_entry))
; If found a reference which points back to this_entry,
; then escape from mapcan loop with that reference as a value:
                                        (throw refer REFBYFIELDP_TAG)
                              )
; If the above clause in cond didn't match then this lambda-form returns
; nil, and when they are nconced together by mapcan it's still nil.
                        )
                     ) ; lambda
                   (getlfield field other_entry)
           ) ; mapcan
           REFBYFIELDP_TAG
    ) ; catch
)


(defun get_ptr_to_tmpfield (key entry &aux stopper)
    (setq stopper (getpermpart entry)) ; Stop searching when found this
    (catch
           (mapcon #'(lambda (subtmp)
                        (cond ((eq subtmp stopper) (throw () GET_PTR_TO_TAG))
                              ((eq (car subtmp) key)
; If found a key from temporary part of entry
; then escape from mapcon loop with contents of that key as a value:
                                        (throw (cdr subtmp) GET_PTR_TO_TAG)
                              )
; If the above clause in cond didn't match then this lambda-form returns
; nil, and when they are nconced together by mapcon it's still nil.
                        )
                     ) ; lambda
                   (gettemppart entry)
           ) ; mapcon
           GET_PTR_TO_TAG
    ) ; catch
)

(defun addtotempfield (key entry item &aux old_field new_field)
    (setq old_field (getlfield key entry))
; If there was no key-field already, then new field is kept just as a single
; reference, not as list. Otherwise, cons new reference to the front of
; old references, so that the key-field of the permanent part doesn't get
; changed:
    (setq new_field (cond (old_field (cons item old_field)) (t item)))
; If there is key-field in temp part already, then just replace the old
; contents with new one:
    (cond ((setq old_field (get_ptr_to_tmpfield key entry))
             (rplaca old_field new_field)
          )
; Otherwise we have to create a small list of two elements, that is a key and
; the new_field, and that list is then spliced to the beginning of the
; temporary part of entry:
          (t (addtotemppart entry (list key new_field)))
    )
)

;
; If original element list contains elements which are kanjis which
; themselves are constructed from smaller elements, then expand_elements
; returns a longer element list composed of basic elements only.
; (compound elements are decomposed recursively.)
;

(defun expand_elements (sym &aux els tmp)
    (cond
       ((null sym) ())
       ((symbolp sym)
          (setq tmp (word_eval (setq sym (solve_alias sym))))
; Choose HK-entry (first one) if there are more than one entry associated
; with this word:
          (cond ((not (single_entryp tmp)) (setq tmp (car tmp))))
; If there is !DONT_EXPAND flag in the entry, or if both the :E and :EXE
; fields are missing, then just return a one-element list constructed
; from alias-solved sym:
          (cond ((or (getfield '!DONT_EXPAND tmp)
                     (and (null (setq els (getfield ':EXE tmp)))
                          (null (setq els (getfield ':E   tmp)))
                     )
                 ) (list sym)
                )
; Otherwise there is either an :EXE or :E field in the entry. Use its contents
; to continue the expansion recursively:
; (Note that the :EXE field has a higher priority than :E field)
                (t (nconc (expand_elements (car els))
                          (expand_elements (cdr els))
                   )
                )
          ) ; cond
       ) ; symbolp
; It should be a list of elements (otherwise we have a nasty bug here!)
; Expand it recursively:
       (t (nconc (expand_elements (car sym)) (expand_elements (cdr sym))))
    ) ; cond
)

(defun handle_G (groupname &aux entry)
    (cond
      ((compoundgroupnamep groupname) ()) ; Do nothing for these now.
      (t (mapc #'(lambda (refer)
                    (cond ((specialreferp refer) ()) ; Skip these
; If refer points to something existing, then add there :G field to point to
; this groupname:
                          ((setq entry (get_entry refer))
                                (addtotempfield ':G entry groupname)
                          )
                    )
                 ) ; lambda
               (getgroup_reflist (word_eval groupname))
         ) ; mapc
      ) ; t
    ) ; cond
)


(defun handle_S_fields (entry S_field &aux other_entry refertothis)
    (setq refertothis (getrefer entry))
    (mapc #'(lambda (S_refer)
                (setq other_entry (get_entry S_refer))
; Don't do connecting if this entry is already referenced by other entry's
; :E or :SEE fields:
                (cond ((or (refbyfieldp ':E other_entry entry)
                           (refbyfieldp ':SEE other_entry entry))
                         (setq REJECT_COUNT (1+ REJECT_COUNT))
                      )
                      (t (addtotempfield ':S other_entry refertothis)
                         (setq MADE_CONNECTIONS (1+ MADE_CONNECTIONS))
                      )
                )
            )
           (cond ((singlereferp S_field) (list S_field)) (t S_field))
    )
)

(defun make_passes (mode fun &aux entry index uplim entrylist next)
    (declare (fixnum index uplim))
    (setq index 0)
    (setq uplim (1+ (getuplim mode)))
    (while (lessp index uplim)
      (cond ((zerop (\\ index 100))
               (print index) (princ (ascii 13)) (drain)
            )
      )
      (setq entry (nth_entry index mode))
      (catch
       (cond
; If entry is a special entry (beginning with *) then skip it, unless
; it is equ-entry (i.e. like (* = $groupname (reference list) "comment"))
; and is pass3 going on, then make :G-links to the temporary part of those
; kanjis which are listed in that reference list:
          ((specentryp entry)
             (cond ((and (eq fun #'pass3) (equentryp entry))
; Use next here as a temporary variable to hold the groupname:
                        (set_word (setq next (getgroup_name entry)) entry)
                        (handle_G next)
                   )
             )
          )
; Else it's a normal entry:
          (t ; First handle the fixed fields:
             (mapc #'(lambda (key field)
                        (if field (funcall fun entry key field)))
                   FIXED_FIELDS
                   (getpermpart entry)
             )
; Handle the optional fields (the ones with actual keys, like :S :P etc.):
             (setq entrylist (nthcdr NUM_OF_FIXED_FIELDS (getpermpart entry)))
             (while entrylist
                 (setq next
                       (cond ((keyp (car entrylist)) (cdr entrylist))
                             (t entrylist)
                       )
                 )
                 (funcall fun entry (car entrylist) (car next))
                 (setq entrylist (cdr next))
             ) ; while
          ) ; t
        ) ; cond
        PASSTAG
       ) ; catch
       (setq index (1+ index))
    ) ; while
    (terpri)
)


(defun pass1 (entry key field)
    (cond
      ((eq key ':HK) (set_word field entry))
      ((or (eq key ':HI) (eq key ':JI))
; Index & other error checking unimplemented for a while, just put the
; entry to the corresponding vector:
         (set_entry field key entry)
      )
; Shortcut after the three first fields :JI :HI :HK
; (just to make things faster)
      ((eq key ':NI) (throw () PASSTAG))
    )
)

;
; Pass2
;
 
(defun pass2 (entry key field &aux aliasroot)
    (cond
      ((memq key '(:NK :R :OTHER :HIRAGANA :KATAKANA))
          (assign_values field entry)
      )
      ((eq key ':COMPOUNDS)
          (mapc #'(lambda (compound)
                     (assign_values (getcompound_meanings compound) entry)
                     (assign_values (getcompound_readings compound) entry)
                  )
                field
          )
      )
      ((eq key ':P)
; Now use field as a list of primitive names which is went through:
          (if (not (consp field)) (setq field (list field)))
          (cond ((null (setq aliasroot (getfield ':HK entry)))
; If there is no HK-keyword then use the first primitive-name as an
;  "alias-root":
                    (setq aliasroot (car field))
                    (set_word aliasroot entry)
                    (setq field (cdr field))
                )
          )
; Set (all/rest of) primitive-names to point either to
;  HK-keyword or to the first primitive-name in the field:
          (mapc #'(lambda (primname) (set_word primname aliasroot)) field)
      )
    )
)


;
; Pass3
;
 
(defun pass3 (entry key field)
    (cond
      ((and (eq key ':E) (not (getfield '!DONT_EXPAND entry)))
         (setq key (expand_elements field)) ; Use key as temporary variable
; Create a new :EX field to the temporary part of entry if the expanded
; list is longer than the original elements list given in the :E field
; of the permanent part:
         (cond ((greaterp (length key) (length field))
                   (addtotemppart entry (list ':EX key))
               )
         ) ; cond
      ) ; and
      ((eq key ':S) (handle_S_fields entry field))
      ((or (eq key ':HIRAGANA) (eq key ':KATAKANA))
; Do something (not impelemented now)
      )
    ) ; cond
)


(defun latista (lista &optional tulos)
    (cond ((null lista) tulos)
          ((atom lista) (cons lista tulos))
          (t (latista (car lista)
                      (latista (cdr lista) tulos)
             )
          )
    )
)


;
; Pass4 converts :HK and :HI fields to SQL INSERT clauses.
;

(defvar *MAX_HK_LEN* 0)
(defvar *MAX_HK* ())
(defvar *MAX_NK_LEN* 0)
(defvar *MAX_NK* ())
(defvar *MAX_NK_LIST* ())
(defvar *MAX_NK_KEYWORDS* 0)
(defvar *OUTPUT* poport)

(defvar *MAX_E_LEN* 0)
(defvar *MAX_E_LIST* ())
(defvar *MAX_EX_LEN* 0)
(defvar *MAX_EX_LIST* ())
(defvar *MAX_S_LEN* 0)
(defvar *MAX_S_LIST* ())

(defun pass4 (entry key field)
    (cond
      ((eq key ':HI)
         (cprintf "INSERT INTO Heisig (JIS, Index, Keyword) VALUES (%d,"
                  (get_JI_field entry)
                  *OUTPUT*
         )
         (princ field *OUTPUT*)
      )
      ((eq key ':HK)
         (cprintf ",'%s')\\g" field *OUTPUT*)
         (terpri *OUTPUT*)
         (cond ((greaterp (pntlen field) *MAX_HK_LEN*)
                     (setq *MAX_HK_LEN* (pntlen field))
                     (setq *MAX_HK* field)
               )
         )
      )
; Shortcut after the three first fields :JI :HI :HK
; (just to make things faster)
      ((eq key ':NI) (throw () PASSTAG))
    )
)

;
; Commented out old version, which splits every Gakken-keyword to its
; own INSERT-statement. (That doesn't work with mSQL as primary keys
; must be unique.)
;
;(defun pass5 (entry key field &aux keyword_list JIS)
;    (cond
;      ((eq key ':NI)
;         (setq keyword_list (latista (getfield ':NK entry)))
;         (cond ((greaterp (length keyword_list) *MAX_NK_KEYWORDS*)
;                    (setq *MAX_NK_KEYWORDS* (length keyword_list))
;                    (setq *MAX_NK_LIST* keyword_list)
;               )
;         )
;         (mapc #'(lambda (keyword)
;                   (cprintf
;"INSERT INTO Gakken (JIS, Index, Keyword) VALUES (%d,"
;                      (cond ((null (setq JIS (get_JI_field entry))) 0)
;                            (t JIS)
;                      )
;                      *OUTPUT*
;                   )
;                   (princ field *OUTPUT*)
;                   (cprintf ",'%s')\\g" keyword *OUTPUT*)
;                   (terpri *OUTPUT*)
;                   (cond ((greaterp (pntlen keyword) *MAX_NK_LEN*)
;                             (setq *MAX_NK_LEN* (pntlen keyword))
;                             (setq *MAX_NK* keyword)
;                         )
;                   )
;                 )
;               keyword_list
;         )
;      )
;; Shortcut after the four first fields :JI :HI :HK :NI
;; (just to make things faster)
;      ((eq key ':NK) (throw () PASSTAG))
;    )
;)
;

(defun print_column_names (colname lista count &optional output)
    (cond ((null lista) ())
          (t
             (princ ","     output)
             (princ colname output)
             (princ count   output)
             (print_column_names colname (cdr lista) (1+ count)
                            output)
          )
    )
)


(defun print_keywords (keyword_list separator &optional output)
    (cond ((null keyword_list) 0)
          ((atom keyword_list)
             (princ keyword_list output)
             (pntlen keyword_list)
          )
          (t (+ (print_keywords (car keyword_list) "," output)
                (cond ((cdr keyword_list)
                         (+ (pntlen (patom separator output))
                            (print_keywords (cdr keyword_list) separator
                                                               output)
                         )
                      )
                      (t 0)
                )
             )
          )
    )
)

(defun print*keywords (keyword_list separator &optional output)
    (cond ((null keyword_list) 0)
          ((atom keyword_list)
             (princ separator output)
             (princ keyword_list output)
          )
          (t (print*keywords (car keyword_list) separator output)
             (print*keywords (cdr keyword_list) separator output)
          )
    )
)


(defun pass5 (entry key field &aux keyword_list JIS)
    (cond
      ((eq key ':NI)
         (cprintf
"INSERT INTO Gakken (JIS, Index, Keywords) VALUES (%d,"
                   (cond ((null (setq JIS (get_JI_field entry))) 0)
                         (t JIS)
                   )
                   *OUTPUT*
         )
         (princ field *OUTPUT*)
         (princ ",'" *OUTPUT*)
         (setq *NK_LEN* (print_keywords
                           (setq keyword_list (getfield ':NK entry))
                           ";" *OUTPUT*
                        )
         )
         (princ "')\\g" *OUTPUT*)
         (terpri *OUTPUT*)
         (cond ((greaterp *NK_LEN* *MAX_NK_LEN*)
                     (setq *MAX_NK_LEN* *NK_LEN*)
                     (setq *MAX_NK* keyword_list)
               )
         )
      )
; Shortcut after the four first fields :JI :HI :HK :NI
; (just to make things faster)
      ((eq key ':NK) (throw () PASSTAG))
    )
)

(defun get_*JI_field (entry &aux JIS)
    (cond ((setq JIS (get_JI_field entry)) JIS)
          ((eq (getfield ':P entry) '!SOMETHING) -1)
          (t (- (getfield ':PG entry)))
    )
)


(defun pass6 (entry key field &aux E_list EX_list S_list JIS HK NI)
    (cond
      ((eq key ':JI)
         (princ "INSERT INTO JIS (JIS" *OUTPUT*)
         (cond ((setq NI (getfield ':NI entry))
             (princ ",GakkenIndex" *OUTPUT*)))
         (cond ((setq HK (getfield ':HK entry))
             (princ ",HeisigKeyword" *OUTPUT*)))
         (print_column_names "E"  (setq E_list (getlfield ':E entry))
                              0 *OUTPUT*)
         (print_column_names "EX" (setq EX_list (getlfield ':EX entry))
                              0 *OUTPUT*)
         (print_column_names "S"  (setq S_list (getlfield ':S entry))
                              0 *OUTPUT*)
         (cond ((greaterp (length E_list) *MAX_E_LEN*)
                     (setq *MAX_E_LEN* (length E_list))
                     (setq *MAX_E_LIST* E_list)
               )
         )
         (cond ((greaterp (length EX_list) *MAX_EX_LEN*)
                     (setq *MAX_EX_LEN* (length EX_list))
                     (setq *MAX_EX_LIST* EX_list)
               )
         )
         (cond ((greaterp (length S_list) *MAX_S_LEN*)
                     (setq *MAX_S_LEN* (length S_list))
                     (setq *MAX_S_LIST* S_list)
               )
         )
         (princ ") VALUES (" *OUTPUT*)
         (princ field *OUTPUT*)
         (cond (NI (cprintf ",%d" NI *OUTPUT*)))
         (cond (HK (cprintf ",'%s'" HK *OUTPUT*)))
         (print*keywords (mapcar #'(lambda (x) (get_*JI_field (get_entry x)))
                                 E_list
                         )
                         "," *OUTPUT*
         )
         (print*keywords (sort ; JIS-indexes to ascending order.
                           (mapcar #'(lambda (x) (get_*JI_field (get_entry x)))
                                   EX_list
                           )
                           #'lessp
                         )
                         "," *OUTPUT*
         )
         (print*keywords (mapcar #'(lambda (x) (get_JI_field (get_entry x)))
                                 S_list
                         )
                         "," *OUTPUT*
         )
         (princ ")\\g" *OUTPUT*)
         (terpri *OUTPUT*)
      )
    )
)


(defun assign_values (field entry)
    (cond ((null field) ())
          ((atom field) (addtovalues field entry))
          (t
             (assign_values (car field) entry)
             (assign_values (cdr field) entry)
          )
    )
)


(defun addtovalues (symbol entry &aux previous_values)
    (cond ; If this symbol has no previous values?
       ((null (setq previous_values (word_eval symbol)))
          (set_word symbol entry)
       )
       ((single_entryp previous_values)
          (cond ((neq entry previous_values) ; Don't put same stuff twice
                   (set_word symbol (list previous_values entry))
                )
          )
       )
       (t ; There exists already at least two values for this symbol:
          (cond ((not (memq entry previous_values)) ; Don't put if it's there
                   (nconc previous_values (list entry)) ; Add to end
                )
          )
       )
    )
)


(defun start ()

    (read_database 'database.lst)

    (rplaca (cdr :NI) NI_UPLIM)

    (make_passes ':NI #'pass1)
    (princ "Pass1 passed succesfully.") (terpri)
    (make_passes ':NI #'pass2)
    (princ "Pass2 passed succesfully.") (terpri)
    (make_passes ':NI #'pass3)
    (princ "Pass3 passed succesfully.")
    (cprintf " %d :S-link connections were made." MADE_CONNECTIONS)
    (cprintf " %d :S-link connections were rejected." REJECT_COUNT)
    (terpri)
)

(defun createHeisig (&optional filename)
    (cond (filename (setq *OUTPUT* (outfile filename))))
    (make_passes ':HI #'pass4)
    (cond (filename (close *OUTPUT*)))
    (cprintf "MAX_HK_LEN=%d " *MAX_HK_LEN*)
    (cprintf "MAX_HK=%s" *MAX_HK*)
    (terpri)
)

(defun createGakken (&optional filename)
    (cond (filename (setq *OUTPUT* (outfile filename))))
    (make_passes ':NI #'pass5)
    (cond (filename (close *OUTPUT*)))
    (cprintf "MAX_NK_LEN=%d "      *MAX_NK_LEN*)
    (princ   "MAX_NK=")
    (princ   *MAX_NK*)
    (cprintf " MAX_NK_KEYWORDS=%d " *MAX_NK_KEYWORDS*)
    (princ   "MAX_NK_LIST=")
    (princ   *MAX_NK_LIST*)
    (terpri)
)

(defun createJIS (&optional filename)
    (cond (filename (setq *OUTPUT* (outfile filename))))
    (make_passes ':NI #'pass6)
    (cond (filename (close *OUTPUT*)))
    (cprintf "MAX_E_LEN=%d "      *MAX_E_LEN*)
    (princ   "MAX_E_LIST=")
    (princ   *MAX_E_LIST*)
    (terpri)
    (cprintf "MAX_EX_LEN=%d "     *MAX_EX_LEN*)
    (princ   "MAX_EX_LIST=")
    (princ   *MAX_EX_LIST*)
    (terpri)
    (cprintf "MAX_S_LEN=%d "      *MAX_S_LEN*)
    (princ   "MAX_S_LIST=")
    (princ   *MAX_S_LIST*)
    (terpri)
)


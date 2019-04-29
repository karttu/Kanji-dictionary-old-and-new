;
;
; This module contains the declare and defvar definitions
; as well as general macros for constructing and and accessing
; the database:
;

(declare (macros t))
 
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

;
; Literals are not put into pure space: (See page 283 of LISPcraft)
;
(setq $ourcopylits ())

(declare
  (fixnum INVALID_KANJI_INDEX HIGHEST_NI NI_UPLIM HI_UPLIM JI_UPLIM
          JI_LOC HI_LOC HK_LOC NI_LOC NK_LOC E_LOC
          NUM_OF_FIXED_FIELDS
          MADE_CONNECTIONS
          REJECT_COUNT
  )
)

(defvar COMMA '\,)
(defvar SLASH '/)
(defvar SPACE '| |)

(defvar INVALID_KANJI_INDEX 71)

; These two are later set exactly in read_database:
(defvar HIGHEST_NI 0)
(defvar NI_UPLIM 2470)
(defvar HI_UPLIM 2042)
(defvar JI_UPLIM 7801)

(defvar JI_LOC 0)
(defvar HI_LOC 1)
(defvar HK_LOC 2)
(defvar NI_LOC 3)
(defvar NK_LOC 4)
(defvar  E_LOC 5)

(defvar NIVEC (new-vector (1+ NI_UPLIM)))
(defvar HIVEC (new-vector (1+ HI_UPLIM)))
(defvar JIVEC (new-vector (1+ JI_UPLIM)))

(defvar :JI (list JI_LOC JI_UPLIM "JIS"       JIVEC))
(defvar :HI (list HI_LOC HI_UPLIM "Heisig"    HIVEC))
(defvar :NI (list NI_LOC NI_UPLIM "Gakken"    NIVEC))
(defvar :HK (list HK_LOC))
(defvar :NK (list NK_LOC))
(defvar :E         (list E_LOC  ()       "Elements"))
(defvar :EX        (list ()     ()       "Elements (decomposed)"))
(defvar :EXE       (list ()     ()       "Elements (used for expanding)"))
(defvar :S         (list ()     ()       "Similar ones"))
(defvar :R         (list ()     ()       "Readings"))
(defvar :SEE       (list ()     ()       "See also"))
(defvar :OTHER     (list ()     ()       "Other"))
(defvar :COMMENT   (list ()     ()       "Comment"))
(defvar :COMPOUNDS (list ()     ()       "Compounds"))
(defvar :P         (list ()     ()       "Primitive names"))
(defvar :G         (list ()     ()       "Groups"))
(defvar :HIRAGANA  (list ()     ()       "Hiragana"))
(defvar :KATAKANA  (list ()     ()       "Katakana"))

(defvar !DONT_EXPAND (list ()   ()))
(defvar !SOMETHING   (list ()   ()))

(defvar KEYWORD_PRIORITIES '(:HK :NK :HIRAGANA :KATAKANA :P :OTHER))
(defvar FIXED_FIELDS '(:JI :HI :HK :NI :NK :E))
(defvar NUM_OF_FIXED_FIELDS 6) ; Should be (length FIXED_FIELDS)

(defvar MADE_CONNECTIONS 0)
(defvar REJECT_COUNT     0)

;
; Now defined in kanjiout.l as function:
;
;(defmacro putexpr (expr output) `(patom ,expr ,output))

; Is X a true cons cell? (I.e. a list node which is not nil)
(defmacro consp (X) `(dtpr ,X))

; Is X a true symbol? (I.e. not nil)
(defmacro nonnilsymbolp (X) `(and (symbolp ,X) ,X))

(defmacro word_eval (W)   `(get ,W ':KANJIDIC))
(defmacro set_word  (W X) `(putprop ,W ,X ':KANJIDIC))


(defmacro equentryp  (x) `(eq (cadr (getpermpart ,x)) '=))
(defmacro specentryp (x) `(eq (car (getpermpart ,x)) '*))
(defmacro specialreferp (x) `(or (null ,x)  (eq ,x '/)))
(defmacro isoutoflimits(index mode) `(greaterp ,index (getuplim ,mode)))

(defmacro getfieldloc  (key) `(car    (symeval ,key)))
(defmacro getuplim     (key) `(cadr   (symeval ,key)))
(defmacro getplaintext (key) `(caddr  (symeval ,key)))
(defmacro getvector    (key) `(cadddr (symeval ,key)))

; (* = $NAME (list of references) "Comment")
(defmacro getgroup_name (g)    `(nth 2 (getpermpart ,g)))
(defmacro getgroup_reflist (g) `(nth 3 (getpermpart ,g)))
(defmacro getgroup_comment (g) `(nth 4 (getpermpart ,g)))

(defmacro getcompound_refers (X)    `(car ,X))
(defmacro getcompound_meanings (X)  `(cadr ,X))
(defmacro getcompound_readings (X)  `(nth 2 ,X))
(defmacro getcompound_flags (X)     `(nth 3 ,X))

(defmacro fixlockeyp (key)  `(numberp (getfieldloc ,key)))
(defmacro nth_entry (index mode) `(vref (getvector ,mode) ,index))
(defmacro set_entry (index mode exp) `(vset (getvector ,mode) ,index ,exp))
(defmacro set_NI_entry (index exp) `(vset NIVEC ,index ,exp))

(defmacro getnthpermpart (index mode) `(car (nth_entry ,index ,mode)))
(defmacro getnthtemppart (index mode) `(cdr (nth_entry ,index ,mode)))

(defmacro getpermpart (entry) `(car ,entry))
(defmacro gettemppart (entry) `(cdr ,entry))

;(defmacro get_JI_field (entry) `(car (getpermpart ,entry)))
; This is a little bit faster, I think:
(defmacro get_JI_field (entry) `(caar ,entry))

(defmacro addtotemppart (worklista list_item)
  `(list_insert ,list_item ,worklista 1))

;
; (singlereferp ()) should return nil
;

(defmacro singlereferp (X) `(or (not (listp ,X)) (not (listp (cdr ,X)))))

(defmacro single_entryp (X) `(not (consp (caar ,X))))

(defmacro flagp (X)              `(eq (getchar ,X 1) '!))
(defmacro keyp (X)               `(eq (getchar ,X 1) ':))
(defmacro primitivep (X)         `(eq (getchar ,X 1) '^))
(defmacro groupnamep (X)         `(eq (getchar ,X 1) '$))
; If the second character is '$' too, then group is a compound group.
; (It is assumed that X is first checked with groupnamep)
(defmacro compoundgroupnamep (X) `(eq (getchar ,X 2) '$))
(defmacro kanap (X)              `(eq (getchar ,X 1) '~))


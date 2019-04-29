
;
; Our 'oh so sofisticated' little 'server' for handling incoming
; kanji-reference requests in the uglyan end:
; 

(include "kanjiout.l")

;
; Part of the Kanji Server, Copyright (C) 1994 by Antti Karttunen
;

(defvar PAUSE 1) ; We sleep one second between each test
(defvar INREADY_FILE   'tmp/IN.READY)
(defvar OUTTMP_FILE    'tmp/OUT.TMP)
(defvar OUTREADY_FILE  'tmp/OUT.READY)

(defun server (&aux INFP INPUT_DATA OUTFP)
    (while t
       (princ "Kanji Server: In the beginning of the loop ")
       (princ (time-string)) (terpri)
; Wait in this little loop until the INREADY_FILE arrives to disk:
       (while (not (probef INREADY_FILE)) (sleep PAUSE))
       (cprintf "Kanji Server: Opening %s for input." INREADY_FILE) (terpri)
       (drain)
       (setq INFP (infile INREADY_FILE))
       (setq INPUT_DATA (read INFP))
       (close INFP)
       (sys:unlink INREADY_FILE)
       (princ "Kanji Server: reference read was: ") (princ INPUT_DATA) (terpri)
       (drain)
       (cprintf "Kanji Server: Opening %s for output." OUTTMP_FILE) (terpri)
       (drain)
       (setq OUTFP (outfile OUTTMP_FILE))
       (print_entry INPUT_DATA OUTFP)
       (close OUTFP)
       (cprintf "Kanji Server: Renaming it to %s" OUTREADY_FILE) (terpri)
       (drain)
       (sys:chmod OUTTMP_FILE 511) ; Give everybody full access to that file.
       (sys:link OUTTMP_FILE OUTREADY_FILE)
       (sys:unlink OUTTMP_FILE)
    )
)


(server)


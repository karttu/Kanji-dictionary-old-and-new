
;
; Server Loop for KServer - Coded by Antti Karttunen 18-DEC-1994.
;
; This time this reads requests directly from TCP/IP socket.
; 

(include "ksersend.l")
(include "kserrequ.l")
(include "sockintl.l")

;
; Part of the Kanji Server, Copyright (C) 1994 by Antti Karttunen
;


(defvar DEFAULT_PORT        8008)
(defvar DEFAULT_TIMEOUT     300) ; seconds, i.e. 5 minutes.
(defvar DEFAULT_ANCHOR_ROOT "http://www.cute.fi/~karttu/KServer")
(defvar ANCHOR_ROOT DEFAULT_ANCHOR_ROOT)
(defvar STARTING_TIME (get_HTTP_date))

(defun do_gc ()
    (cprintf "KServer: timeout occurred at %s" (time-string))
    (cprintf "KServer: %d garbage collections have occurred so far. "
       $gccount$)
    (print (ptime)) (terpri)
    (gc)
)


(defun server_loop (&aux port_to_listen master_socket connection timeout)
    (set_select)
    (set_trace)
    (setq port_to_listen DEFAULT_PORT)
    (setq timeout DEFAULT_TIMEOUT)
    (cprintf "KServer: Started: %s" (time-string)) (terpri)
    (cond ((command-line-args) ; There are arguments (at least one)
              (setq port_to_listen (readlist (exploden (argv 1)))))
    )
    (cond ((consp (cdr (command-line-args))); There are at least two args.
              (setq timeout (readlist (exploden (argv 2))))
          )
    )
    (cond ((consp (cddr (command-line-args))); There are at least three args.
              (setq ANCHOR_ROOT (argv 3))
          )
    )
    (setq master_socket (do_bind port_to_listen))
    (while t
       (cprintf "KServer: In the beginning of the server loop at %s"
                (time-string)) (terpri)
       (cprintf "Kserver: %d garbage collections have occurred so far. "
             $gccount$)
       (print (ptime)) (terpri)
       (cond
          ((null (setq connection (sopen master_socket timeout)))
               (do_gc) ; If timeout occurred then just do the garbage collect.
          )
; If read_request returns () then there was something fishy with the stuff
; read in from the connection, and it handles the error messages by itself...
          ((setq request (READ_request connection))
             (princ "KServer: reference read was: ") (princ request) (terpri)
             (handle_request request connection)
             (sclose connection)
          )
       )
    )
)


(server_loop)


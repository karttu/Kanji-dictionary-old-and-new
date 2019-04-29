
;
; This module just loads couple of foreign function (written in C) in.
; After this is loaded in, it's good to take dump of the listp system.
;

;
; sockfuns.c should contain general functions for facilitating the
; socket usage: (but no code related to Franz Lisp in anyway!)
;
(cfasl 'sockfuns.o '_set_select 'set_select "c-function")
(getaddress '_clr_select 'clr_select "c-function")
(getaddress '_set_trace  'set_trace  "c-function")
(getaddress '_clr_trace  'clr_trace  "c-function")
(getaddress '_do_bind    'do_bind    "c-function")
(getaddress '_do_accept  'do_accept  "c-function")

;
; sockintc.c should contain socket-related C-code which is related
; to Franz Lisp, e.g. the code which uses macros and definitions
; defined in franzint.h, and functions which return lispvalues directly.
;
(cfasl 'sockintc.o '_init_read_table 'init_read_table "c-function")
(getaddress '_read_a_line_to_list 'read_a_line_to_list "c-function")
(getaddress '_read_HTTP_line_to_list 'read_HTTP_line_to_list "c-function")
(getaddress '_Lsopen 'sopen "function")
(getaddress '_Lsclose 'sclose "subroutine") ; Returns always t
(getaddress '_Lfileno 'fileno "c-function")

;
; This module contains C-code adopted from CERN daemon, and a lisp interface
; functions to it:
;
(cfasl 'httpdate.o '_get_HTTP_date 'get_HTTP_date "function")


/*
;
; sockintc.c  --  an interface between Franz Lisp and UNIX socket-functions.
;
; C module which implements a couple of functions needed by sockintl.l
;
; Copyright (C) 1994 by Antti Karttunen
;
 */

#include <stdio.h>
#include <ctype.h>

#include "franzint.h"

/* Duplicated from sockfuns.c: */
#define INDEF_TIMEOUT -1

#define TABSIZE 256

lispval read0table[TABSIZE]; /* For each byte. */
lispval read1table[TABSIZE]; /* For each byte. */

lispval LPAR,RPAR;

/* This should be called with a list of integers in the format:
    (40 41 0 1 2 ... to ... 254 255)
    (or something else, if we want to do some read-conversion...)
   If there are nil's in the list, it means that the reader skips
   all occurrences of the corresponding character in input.

   The first two elements should be ascii-values of the left and right
   parentheses, so that we can store them to LPAR and RPAR variables
   respectively. That allows storing something else to their proper
   locations in that list, for example nil's if we just want to skip
   parentheses in the input. (So that deviant clients can't mess our
   reader with mismatched parentheses...)
 */

/* 
(getaddress '_init_read_table 'init_read_table "c-function")
 */

int init_read_table(lista,table)
lispval lista;
int table;
{
    int i;

    LPAR = (MYCAR(lista)); lista = MYCDR(lista);
    RPAR = (MYCAR(lista)); lista = MYCDR(lista);
    for(i=0; (NOTNIL(lista) && (i < TABSIZE)); lista = MYCDR(lista))
     {
       if(!table) { read0table[i] = (MYCAR(lista)); }
       else       { read1table[i] = (MYCAR(lista)); }
       i++;
     }

    return(i);
}

/* This reads a line from socket port to list given as second argument.
   Returns the count of accepted characters as result.
   How to handle EOF?
 */
/*
(getaddress '_read_a_line_to_list 'read_a_line_to_list "c-function")
 */
int read_a_line_to_list(socport,lista)
lispval socport;
register lispval lista;
{
    register lispval next;
    int read_bytes;
    int len=0;
    unsigned char bufferi[1];

    if(ISNIL(lista) || ISNIL(MYCDR(lista))) { return(0); }
    MYRPLACA(lista,LPAR);
    lista = MYCDR(lista);

    while(NOTNIL(next = MYCDR(lista)) &&
          ((read_bytes = read(fileno(GET_PORT(socport)),bufferi,sizeof(char)))
               > 0) &&
                (*bufferi != '\n'))
     {
       if(ISNIL(read0table[*bufferi])) /* Skip this character. */
	{ continue; }
       MYRPLACA(lista,read0table[*bufferi]);
       lista = next;
       len++;
     }

    MYRPLACA(lista,RPAR);

    if(read_bytes < 0) { perror("read_a_line_to_list"); return(read_bytes); }
    if(!read_bytes) { return(-1); } /* Temporary kludge??? */
    return(len);
}

/*
(getaddress '_read_HTTP_line_to_list 'read_HTTP_line_to_list "c-function")
 */
/*

    This is a real mess.

 */
int read_HTTP_line_to_list(socport,lista)
lispval socport;
register lispval lista;
{
    register lispval next;
    int read_bytes;
    int len=0;
    int newchar;
    unsigned char bufferi[3];

    bufferi[2] = '\0';

    if(ISNIL(lista) || ISNIL(MYCDR(lista))) { return(0); }
    MYRPLACA(lista,LPAR);
    lista = MYCDR(lista);

    while(NOTNIL(next = MYCDR(lista)) &&
          ((read_bytes = read(fileno(GET_PORT(socport)),bufferi,sizeof(char)))
               > 0) &&
                (*bufferi != '\n'))
     {
/* I wouldn't want to do this at this level! */
       if(*bufferi == '%')
	{ /* Try to read next two characters: */
          if((read_bytes =
                read(fileno(GET_PORT(socport)),bufferi,(2*sizeof(char))))
               < 2)
	   {
             break;
           }
          else
           {
             if(isxdigit(*bufferi) && isxdigit(bufferi[1]))
              {
                sscanf(bufferi,"%x",&newchar);
		if(ISNIL(read0table[newchar]))
                 { continue; }
                else
                 { MYRPLACA(lista,read0table[newchar]); }
              }
             else { continue; }
           }
        }
       else if(ISNIL(read1table[*bufferi])) /* Skip this character. */
	{ continue; }
       else { MYRPLACA(lista,read1table[*bufferi]);}

       lista = next;
       len++;
     }

    MYRPLACA(lista,RPAR);

    if(read_bytes < 0)
     { perror("read_HTTP_line_to_list"); return(read_bytes); }
    if(!read_bytes) { return(-1); } /* Temporary kludge??? */
    return(len);
}



/* Get a file pointer (called port in Franz Lisp) for a descriptor which
   is given as argument. Mode is always read+write "r+"
   (Should add a mode argument when I have time.)
 */

/*
(getaddress '_Lour_fdopen 'fdopen "function")
 */

lispval Lour_fdopen(fd)
int *fd; /* Passed as a reference with "function" discipline. */
{
    FILE *resfp;
    char name[21]; /* Should be enough. */

    resfp = fdopen(*fd,"r+");
    if(!resfp) { perror("fdopen"); return(nil); }
    else
     { /* Construct a name for this file descriptor and remember it: */
        sprintf(name,"$fd:%d",*fd);
	ioname[PN(resfp)] = (lispval) inewstr((char *)name);
	return(P(resfp));
     }
}

 
/* Returns a file pointer (i.e. a port in Franz Lisp) for the
   socket-connection which is accepted from master_soc (which the
   caller should have created previously with do_bind).
   timeout is a fixnum which specifies how many seconds we wait in
   select (if select is turned on with set_select() function), until
   timeout occurs. In that case (i.e. if timeout occurs and no connection
   has been accepted before that) nil is returned to calling lisp function.
   If the calling lisp function specifies timeout as nil, then we wait
   indefinitely long in the select.
 */

/*
(getaddress '_Lsopen 'sopen "function")
 */
lispval Lsopen(master_soc, timeout)
int *master_soc, *timeout; /* Passed as references with "function" disc. */
{
    int newsoc;
 
    if(!(newsoc =
       do_accept(*master_soc,(ISNIL(timeout) ? INDEF_TIMEOUT : *timeout))))
     { return(nil); }
    else { return(Lour_fdopen(&newsoc)); }
}

/*
(getaddress '_Lsclose 'sclose "subroutine") ; Returns always t
 */
lispval Lsclose(socfp)
lispval socfp;
{
/* Of course we should check that it is a valid port... */
    fflush(GET_PORT(socfp)); /* Must flush it before shutdowning the conn. */
    shutdown(fileno(GET_PORT(socfp)),2);
    fclose(GET_PORT(socfp));
    ioname[PN(GET_PORT(socfp))] = nil; /* Clear the name from table. */
}


/* This might be useful... */
/*
(getaddress '_Lfileno 'fileno "c-function")
 */
int Lfileno(port)
lispval port;
{
    return(fileno(GET_PORT(port)));
}
 

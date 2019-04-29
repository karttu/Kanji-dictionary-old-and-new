
/* Window version of putc by Antti Karttunen at 20. FEB. 1990
    This checks whether file pointer ptr is assigned to any window.
   If not, then original putc (renamed orgputc) is called, and that's all.
    Otherwise character c is printed to its own window.
   From every window is following information kept:
    file pointer(s) assigned to,
    extents, i.e. upperedge, heigth, leftedge and width
    and current cursor position, i.e. currentline and currentcolumn.
   Opened windows are kept in list which is value of symbol *WINDOWS*

   Should later code to handle the most common Ansi ESC-codes (home, clear,
    set cursor, inverse, etc.)
 */


#include "stdio.h"
#include "ctype.h"
#include "mydefs.h"
#include "lists.h"
#include "fundefs.h"
#include "grafuns.h"
#include "wputc.h"


TOB getwindow(),get_windows();
FILE *initwindow();
FILE *initwindow_to_fp();


TOB _WINDOWS_=NIL; /* Symbol whose value is list of opened windows */

/* If this is zero then current fp is normal stream: (to file for example) */
BYTE window_fp_flag=0;

/* This contains file pointer used at previous call to putc: */
FILE *previous_fp=NULL;

/* These are pointers to the integers keeping track of
    cursor position of current window: */
int *ptr_w_column=NULL;
int *ptr_w_line=NULL;

/* Extents of current window: */
int w_upperedge=0;
int w_heigth=0;
int w_leftedge=0;
int w_width=0;
int w_lowestline=0;
int w_rightestcolumn=0;


/* Opens window to screen: (Returns file pointer associated to it) */
FILE *initwindow(upperedge,heigth,leftedge,width)
int upperedge,heigth,leftedge,width;
{
        FILE *myfopen();
        FILE *window_fp;

        window_fp = myfopen("CON","w");
        initwindow_to_fp(window_fp,upperedge,heigth,leftedge,width);
        return(window_fp);
}

/* This assigns existing file pointer (e.g. stdout or stderr) to window: */
FILE *initwindow_to_fp(window_fp,upperedge,heigth,leftedge,width)
FILE *window_fp;
int upperedge,heigth,leftedge,width;
{
        TOB l;

        /* Make window entry: (compact list of length seven) */
        l = clistn(ZERO,ZERO,
                    int_tob(upperedge),int_tob(heigth),
                     int_tob(leftedge),int_tob(width),
                      fp_tob(window_fp),ENDMARK);
        addtowindows(l);
        w_clear(window_fp);
        return(window_fp);
}


/* Assigns new_fp to the same window where old_fp is already associated to. */
assign_to_window(new_fp,old_fp)
FILE *new_fp,*old_fp;
{
        nconc(getwindow(old_fp),list1(fp_tob(new_fp)));
}


/* This changes extents of w_fp to extents in new_extents_list,
    and returns the list of old extents.
 */
TOB change_extents(w_fp,new_extents_list)
FILE *w_fp;
TOB new_extents_list;
{
        TOB old_extents,l;

        l = getwindow(w_fp);

        old_extents = list4(int_tob(getw_upperedge(l)),
                            int_tob(getw_heigth(l)),
                            int_tob(getw_leftedge(l)),
                            int_tob(getw_width(l)));
        setw_upperedge(l,tob_int(nth(0,new_extents_list)));
        setw_heigth(l,tob_int(nth(1,new_extents_list)));
        setw_leftedge(l,tob_int(nth(2,new_extents_list)));
        setw_width(l,tob_int(nth(3,new_extents_list)));
        return(old_extents);
}



/* If fp argument has changed for putc, w_home, w_clear or w_newline,
    then this procedure sets values of current window variables:
 */
change_current_window(fp)
FILE *fp;
{
        register TOB w;

        previous_fp = fp;

        /* If not found from _windows list then this is not window fp: */
        if(nilp(w = getwindow(fp)))
         {
           window_fp_flag = 0;
         }
        else /* Otherwise set current window variables: */
         {
           window_fp_flag  = 1;
           ptr_w_column    = (getptr_w_column(w));
           ptr_w_line      = (getptr_w_line(w));

           w_upperedge     = getw_upperedge(w);
           w_heigth        = getw_heigth(w);
           w_leftedge      = getw_leftedge(w);
           w_width         = getw_width(w);

           w_lowestline     = w_heigth-1;
           w_rightestcolumn = w_width-1;
         }
}


/* This gets window entry from _windows list which is assigned to window_fp */
TOB getwindow(window_fp)
FILE *window_fp;
{
        TOB stream,lista,namu; /* Sun ei tarvi peiton alla imee namua. */

        stream = fp_tob(window_fp);
        lista = get_windows();

        while(!nilp(lista))
         {
           namu = getw_fps(car(lista));
           if(!nilp(memq(stream,namu)))
            { return(car(lista)); }
           lista = cdr(lista);
         }

        return(NIL);
}


/* Returns the list _windows: */
TOB get_windows()
{
        return(nilp(_WINDOWS_) ? NIL : value(_WINDOWS_));
}


addtowindows(window)
TOB window;
{
        if(nilp(_WINDOWS_)) /* Not yet initialized */
         {
           _WINDOWS_ = intern("*WINDOWS*");
         }

        setvalue(_WINDOWS_,cons(window,value(_WINDOWS_)));
}
         


/* This clears character area in hires screen: */
clear_area(upperedge,heigth,leftedge,width)
int upperedge,heigth,leftedge,width;
{
        clear_pixel_area(charlinetopixline(upperedge),
                          (heigth * YPIXELS_IN_CHAR),
                           leftedge,
                            width);
}


/* This inverses character area in hires screen: */
inverse_area(upperedge,heigth,leftedge,width)
int upperedge,heigth,leftedge,width;
{
        inverse_pixel_area(charlinetopixline(upperedge),
                          (heigth * YPIXELS_IN_CHAR),
                           leftedge,
                            width);
}

/*
This scrolls area in hires screen, n characters up (or down if n is negative)
 */
scroll_area(n,upperedge,heigth,leftedge,width)
int n,upperedge,heigth,leftedge,width;
{
        hires_scroll((n * YPIXELS_IN_CHAR),
         charlinetopixline(upperedge),(heigth * YPIXELS_IN_CHAR),
          leftedge,width);
}


/* This sets cursor position of window assigned to fp to upper lefthand
    corner of window.
 */
w_home(fp)
FILE *fp;
{
        if(fp != previous_fp)
         { change_current_window(fp); }

        /* If normal stream, then home cursor of whole screen: */
        if(!window_fp_flag) { scr_curs(0,0); }
        else
         {
           *ptr_w_column = 0;
           *ptr_w_line   = 0;
         }
}


/* This clears window assigned to window_fp (and homes it too) */
w_clear(window_fp)
FILE *window_fp;
{
        if(window_fp != previous_fp)
         { change_current_window(window_fp); }

        /* If normal stream, then clear the whole screen: */
        if(!window_fp_flag)
         { clear_area(0,LINES_IN_SCREEN,0,CHARS_IN_LINE); }
        else
         {
           clear_area(w_upperedge,w_heigth,w_leftedge,w_width);
         }

        w_home(window_fp);
}

/* This puts n newlines to fp: */
w_newlines(fp,n)
FILE *fp;
register int n;
{
        while(n--) { w_newline(fp); }
}

/* This puts one newline to window associated with fp, and scrolls it too
    if necessary.
 */
w_newline(fp)
FILE *fp;
{
        if(fp != previous_fp)
         { change_current_window(fp); }

        /* If normal stream, then just put newline with aputc: */
        if(!window_fp_flag) { return(aputc('\n',fp)); }
        else
         {
           *ptr_w_column = 0;
           ++*ptr_w_line;

           /* Scroll until ptr_w_line is pointing to the lowest_line: */
           for(;(*ptr_w_line > w_lowestline); (--*ptr_w_line))
            {
              scroll_area(1,w_upperedge,w_heigth,w_leftedge,w_width);
            }
         }
}




/* Note that this differs from original putc how newlines ('\n') are handled.
   Usually when cursorpointer is at lowest line, when '\n' is printed
   screen is scrolled immediately. However at this system window is
   not scrolled until something text is printed after newlines.
   This is because windows can be quite small and unnecessary scrolling may
   remove valuable text from window before viewer has seen it.
   Use functions newline and newlines to get that effect what is usually
   desired by putting more than one newlines to screen.
 */
putc(c,ptr)
int c;
FILE *ptr;
{
        int saveline,savecolumn,result;

        /* If fp changed where we are printing to, then check if it is
            assigned to any window: */
        if(ptr != previous_fp)
         { change_current_window(ptr); }

        /* If normal stream, then just use original putc: */
        if(!window_fp_flag) { return(orgputc(c,ptr)); }
        else
         { /* Save current position of cursor: */
           scr_loc(&saveline,&savecolumn);

           /* If printed newline or column has gove over rightedge: */
           if((c == '\n') || (*ptr_w_column > w_rightestcolumn))
            {
              *ptr_w_column = 0;
              ++*ptr_w_line;
            }

/* Don't print LF's and CR's at all because they can scroll inadvertently
    the whole screen */
           if((c == '\n') || (c == '\r')) { return(c); }

           /* Scroll until ptr_w_line is pointing to the lowest_line: */
           for(;(*ptr_w_line > w_lowestline); (--*ptr_w_line))
            {
              scroll_area(1,w_upperedge,w_heigth,w_leftedge,w_width);
            }

           /* If cursor is at lower righthand corner of the whole screen,
               then scroll the window, because otherwise stupid operating
                system scrolls the whole screen if we put character to this
                 location:
            */
           if(((w_upperedge + *ptr_w_line) == (LINES_IN_SCREEN-1)) &&
              ((w_leftedge  + *ptr_w_column) == (CHARS_IN_LINE-1)))
            {
              scroll_area(1,w_upperedge,w_heigth,w_leftedge,w_width);
              --*ptr_w_line;
            }

           /* Set global cursor position to cursor position of this window: */
           scr_curs((w_upperedge + *ptr_w_line),
                     (w_leftedge + *ptr_w_column));

           /* If bell emitted then don't advance column */
           if(c != '\007') { ++*ptr_w_column; } /* Add column */

           result = orgputc(c,ptr); /* Put the character */

           /* Restore the original cursor position: */
           scr_curs(saveline,savecolumn);
         }
        return(result);
}




/* ======================== ORIGINAL AZTEC put.c: ======================= */

/* Copyright (C) 1981,1982,1983,1984 by Manx Software Systems */
/* #include "stdio.h" */

orgputc(c,ptr) /* Name of putc changed to orgputc by A.K. */
int c; register FILE *ptr;
{
	if (ptr->_bp >= ptr->_bend)
		return flsh_(ptr,c&0xff);
	return (*ptr->_bp++ = c) & 0xff;
}

static closall()		/* called by exit to close any open files */
{
	register FILE *fp;

	for ( fp = Cbuffs ; fp < Cbuffs+MAXSTREAM ; )
		fclose(fp++);
}

fclose(ptr)
register FILE *ptr;
{
	register int err;

	err = 0;
	if (!ptr)
		return -1;
	if ( ptr->_flags ) {
		if (ptr->_flags&_DIRTY)	/* if modifed flush buffer */
			err = flsh_(ptr,-1);
		err |= close(ptr->_unit);
		if (ptr->_flags&_ALLBUF)
			free(ptr->_buff);
		if (ptr->_flags&_TEMP) {	/* temp file, delete it */
			unlink(ptr->_tmpname);
			free(ptr->_tmpname);
		}
	}

	ptr->_buff = 
	ptr->_bend =  /* nothing in buffer */
	ptr->_bp = 0;
	ptr->_flags = 0;
	return err;
}

flsh_(ptr,data)
register FILE *ptr;
{
	register int size;
	extern int (*cls_)();

	cls_ = closall;
	if (ptr->_flags & _IOERR)
		return EOF;
	if (ptr->_flags & _DIRTY) {
		size = ptr->_bp - ptr->_buff;
		if (write(ptr->_unit, ptr->_buff, size) != size) {
ioerr:
			ptr->_flags |= _IOERR;
			ptr->_bend = ptr->_bp = NULL;
			return EOF;
		}
	}
	if (data == -1) {
		ptr->_flags &= ~_DIRTY;
		ptr->_bend = ptr->_bp = NULL;
		return 0;
	}
	if (ptr->_buff == NULL)
		getbuff(ptr);
	if (ptr->_buflen == 1) {	/* unbuffered I/O */
		if (write(ptr->_unit, &data, 1) != 1)
			goto ioerr;
		return data;
	}
	ptr->_bp = ptr->_buff;
	ptr->_bend = ptr->_buff + ptr->_buflen;
	ptr->_flags |= _DIRTY;
	return (*ptr->_bp++ = data) & 0xff;
}

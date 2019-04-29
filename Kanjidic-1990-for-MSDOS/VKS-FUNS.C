
/* Here are functions for implementing Virtual Kanji Screen.
   I.e.: for putting kanji and/or expression to VKS,
         for scrolling, for clearing, for printing various things
         in screen mode, etc.
 */


#include "stdio.h"
#include "ctype.h"
#include "signal.h"
#include "setjmp.h"
#include "mydefs.h"
#include "lists.h"
#include "fundefs.h"
#include "globals.h"
#include "grafuns.h"
#include "ansiesc.h"
#include "kanjidic.h"
#include "kdfundef.h"
#include "kdglobs.h"

int qsprintnexpr();

#define MAX_EXPR_LEN 512

#define readnbytes(BUF,N,FP) fread((BUF),sizeof(BYTE),(N),(FP))

#define coerceuli(X) ((ULI) (X))

int fontsize=0;
int truekanjiwidth=0;
int kanjiheigth=0;
int defaultspace=0;
int ver_space=0;
int bytes_in_bitmap=0;
/* What is approximately line in the middle of kanji (vertically) */
/* 1 when font is 16, 2 when font is 24 pixels: */
int middle_level=0;

/* If this is on, then single texts are printed to level approximately at
middle of kanjis, otherwise they are printed to same level as top edge of
kanjis.
*/
BYTE middle_flag = 0;

BYTE double_flag=0;  /* If 1 then bits of bitmap of KANJI are duplicated,
                          so it's widened in X-direction. */
BYTE swap_flag=0;    /* If 1 then adjacent bytes are swapped. */
BYTE reverse_flag=0; /* If 1 then bytes are reversed */
BYTE adjust_flag=0;  /* If 1 then indexes are adjusted to fit font file
                         K16JIS1.FNT */
UINT skip_bytes=0;/* How many bytes are skipped from beginning of font file */


char *current_font1file = "Not set yet.";
char *current_font2file = "";


BYTE _kanji_bytebuf[((MAXFONTSIZE * MAXFONTSIZE) / 8)];
BYTE curs_savebuf[(MAXFONTSIZE*2)/8];


FILE *k_fp=NULL;
FILE *k_own_fp=NULL;
int k_upperedge=0;
int k_heigth=25;
int k_leftedge=0;
int k_width=80;

int k_lowestline=0;
int k_rightestcolumn=0;
int k_lowestline_for_kanji=0;
int k_rightestcolumn_for_kanji=0;

int *ptr_to_k_line=NULL;
int *ptr_to_k_column=NULL;

#define k_line   (*ptr_to_k_line)
#define k_column (*ptr_to_k_column)


UINT cursorpause=0;

/*
Hei mies, varmaankin sin{ tulet Venuksesta, pilvien alaisesta rakennuksesta.
 */

FILE *font1_fp=NULL;
FILE *font2_fp=NULL;

UINT previous_jis_index = different_jis_index;


UINT fieldstartcolumn=11;
UINT meanings_column=26;
UINT readings_column=52;

TOB vks_vector=NIL; /* Virtual Kanji Screen compact list */
/* Pointers to vks_vector: */
TOB last_ptr=NIL; /* To last entry put to vks. */
TOB first_in_realscreen=NIL; /* To first one which is shown in real screen */
TOB last_in_realscreen=NIL;  /* To last  one which is shown in real screen */
TOB cursorpointer=NIL;       /* To that vks entry where cursor is */

TOB saved_extents=NIL;
TOB other_extents=NIL;

int vks_entries=1000;

#define VKS_ENTRIES vks_entries

UINT jis1level_cnt=0;
UINT jis2level_cnt=0;


int getk_rightestcolumn()
{
        return(k_rightestcolumn);
}

int getk_line()
{
        return(k_line);
}

int getk_column()
{
        return(k_column);
}


/* Returns 1 if entry indicates that vks_item is kanji
    (or other JIS-character), or it is marked expression (usually groupname).
 */
int kanji_or_markexp(entry)
TOB entry;
{
        return(consp(entry) || intp(entry) || eq(entry,MARKED_EXPR));
}

/* p_* versions of these testing functions can be used applied to pointers
    to vks_vector:
 */
int p_kanji_or_markexp(ptr)
TOB ptr;
{
        return(kanji_or_markexp(getvks_entry(car(ptr))));
}


int kanjip(entry)
TOB entry;
{
        return(!(nilp(entry) || eq(entry,MARKED_EXPR) || filemarkerp(entry)));
}

int p_kanjip(ptr)
TOB ptr;
{
        return(kanjip(getvks_entry(car(ptr))));
}


int filemarkerp(entry)
TOB entry;
{
        return(eq(entry,FILELINE_MARKER) || eq(entry,FILEOFFSET_MARKER));
}

int p_filemarkerp(ptr)
TOB ptr;
{
        return(filemarkerp(getvks_entry(car(ptr))));
}

forward_if_filemarker(ptr_to_ptr)
TOB *ptr_to_ptr;
{
        if(p_filemarkerp(*ptr_to_ptr))
         { *ptr_to_ptr = vks_next2(*ptr_to_ptr); }
}


FILE *initkanjiwindow(upperedge,heigth,leftedge,width)
int upperedge,heigth,leftedge,width;
{
        FILE *myfopen(),*initwindow();

        extern int w_upperedge,w_heigth,w_leftedge,w_width;
        extern int w_lowestline,w_rightestcolumn;
        extern int *ptr_w_line,*ptr_w_column;

        k_fp = initwindow(upperedge,heigth,leftedge,width);
        k_own_fp = myfopen("CON","w");

        set_k_variables();
        vks_vector = new_clist(VKS_ENTRIES);
        last_ptr = NIL;
        kcls();
        return(k_fp);
}


set_k_variables()
{
        change_current_window(k_fp);

        k_upperedge      =  w_upperedge;
        k_heigth         =  w_heigth;
        k_leftedge       =  w_leftedge;
        k_width          =  w_width;
        k_lowestline     =  w_lowestline;
        k_rightestcolumn =  w_rightestcolumn;

        k_rightestcolumn_for_kanji = (k_rightestcolumn + 1) - KANJIWIDTH;
        k_lowestline_for_kanji = (k_lowestline + 1) - HEIGTH_OF_KANJI_ET_TEXT;

        ptr_to_k_line    =  ptr_w_line;
        ptr_to_k_column  =  ptr_w_column;
}


extend_kanjiwindow()
{
        TOB change_extents();

        if(!nilp(saved_extents))
         {
         erprintf((ER,"**extend_kanjiwindow: saved_extents is not NIL !\n"));
           return(-1);
         }

        saved_extents = change_extents(k_fp,
          (!nilp(other_extents) ? other_extents
             : list4(int_tob(0),int_tob(25),int_tob(2),int_tob(78))));
        set_k_variables();
/*
        kcls();
 */
}


restore_kanjiwindow()
{
        TOB change_extents();

        if(nilp(saved_extents))
         {
           erprintf((ER,"**restore_kanjiwindow: saved_extents is NIL !\n"));
           return(-1);
         }
        else
         {
           kcls();
           other_extents = change_extents(k_fp,saved_extents);
           set_k_variables();
           saved_extents = NIL;
         }
}


/* font_info can be a list of values to be set, or it can be a name of
    environment variable where list of values is read from.
 */
setfontfile(font_info)
TOB font_info;
{
        TOB info;
        char *s,buf[83];

        if(!listp(font_info))
         {
           if(!(s = getenv(pname(font_info))))
            {
              fprintf(NOTICE_FP,"**setfontfile: environment variable ");
              printexpr(font_info,NOTICE_FP);
              fprintf(NOTICE_FP," is undefined !\n");
              return(0);
            }

           strncpy(buf,s,80);
           info = sreadexpr(buf);
           if(!consp(info))
            {
              fprintf(NOTICE_FP,
               "**setfontfile: contents of env. var. %s is invalid: %s",
                  pname(font_info),s);
               return(0);
            }
           font_info = info;
         }

        return(_setfontfile(nth(0,font_info),
                            nth(1,font_info),
                            nth(2,font_info),
                            nth(3,font_info),
                            nth(4,font_info),
                            nth(5,font_info),
                            nth(6,font_info),
                            nth(7,font_info)));
}


/* SET FONT16 = (C:\K16JIS1.FNT C:\K16JIS2.FNT 16 1 1 1 1)
   SET FONT24 = (C:\KANJI24.JIS "" 24 1)
 */

/* Sets double_flag to doubleflag: (and adjust other variables to that) */
set_double_flag(doubleflag)
int doubleflag;
{
        _setfontfile(NIL,NIL,NIL,int_tob(doubleflag),NIL,NIL,NIL,NIL);
}

UINT get_double_flag()
{
        return(double_flag);
}

                   

/* Returns 1 if succeeds, 0 if fails, when trying to set/change font */
_setfontfile(font1filename,font2filename,font__size,double__flag,swap__flag,
reverse__flag,adjust__flag,skip__bytes)
TOB font1filename,font2filename;
TOB font__size;
TOB double__flag,swap__flag,reverse__flag,adjust__flag;
TOB skip__bytes;
{
        FILE *open_new_fontfile();
        FILE *new_fp;

        if(intp(font__size)) 
         {
           int new_fontsize;

           new_fontsize       = tob_int(font__size);

           if((new_fontsize <= 0) || (new_fontsize % 8) ||
                 (new_fontsize > MAXFONTSIZE))
            {
              fprintf(NOTICE_FP,
               "**setfontfile: Invalid fontsize: %d.\n",
                 new_fontsize);
              return(0);
            }
           else { fontsize = new_fontsize; }
         }

        if(!nilp(font1filename) &&
              (new_fp = open_new_fontfile(font1filename,font1_fp)))
         {
           font1_fp = new_fp;
           current_font1file = pname(font1filename);
         }

        if(!nilp(font2filename))
         {
           new_fp = open_new_fontfile(font2filename,font2_fp);
           if(!nilp(new_fp) || !strlen(pname(font2filename)))
            {
              font2_fp = new_fp;
              current_font2file = pname(font2filename);
            }
         }

        if(intp(double__flag))  { double_flag    = tob_int(double__flag); }
        if(intp(swap__flag))    { swap_flag      = tob_int(swap__flag); }
        if(intp(reverse__flag)) { reverse_flag   = tob_int(reverse__flag); }
        if(intp(adjust__flag))  { adjust_flag    = tob_int(adjust__flag); }
        if(intp(skip__bytes))   { skip_bytes     = tob_int(skip__bytes); }


        truekanjiwidth = (fontsize / 8);
        kanjiheigth    = (fontsize / 8);
        ver_space      = HEIGTH_OF_KANJI; /* = kanjiheigth + 1 */
        defaultspace   = ((fontsize > 16) ? 2 : 1);
        middle_level   = ((fontsize > 16) ? 2 : 1);
        /* How many bytes one bitmap takes: */
        bytes_in_bitmap = ((fontsize * fontsize) / 8);

        /* Forces drawkanji to load kanji in, because we don't want
           that old bitmap in readbuf to be shown */
        previous_jis_index = different_jis_index;

/* Can't do this first time, when initializing, because initkanjiwindow
    is called only after this setfontfile (in module INIT1) and 
    set_k_variables needs information about kanji_window.
 */
        if(k_fp != NULL) { set_k_variables(); }

        return(1);
}

FILE *open_new_fontfile(fontfilename,font_fp)
TOB fontfilename;
FILE *font_fp;
{
        FILE *new_fp;

        /* If fontfilename is "" then let file pointer be NULL: */
        if(!strlen(pname(fontfilename))) { new_fp = NULL; }
/*      if(nilp(fontfilename)) { new_fp = NULL; } */
        /* Else try to open new one: */
        else if(!(new_fp = fopen(pname(fontfilename),"r")))
         {
           fprintf(NOTICE_FP,
            "**Cannot open fontfile: %s !\n",pname(fontfilename));
           return(NULL);
         }

/* Close previous fontfile if this is not first time in this procedure: */
        if(font_fp != NULL) { fclose(font_fp); }

        return(new_fp);
}


/* This is a relic function what is used in rawprint mode: */
kanjiput(kanji)
int kanji;
{
        if(k_column > k_rightestcolumn_for_kanji)
         {
           kanjinewlines(1);
         }

        /* Scroll until ptr_w_line is pointing to the lowest_line: */
        for(;(k_line > k_lowestline_for_kanji); (--k_line))
         {
           _k_scroll(1);
         }

        kanjitoloc(kanji,k_column,k_line);
        k_column += DEFAULT_ADD;
}


/* Reads bitmap of kanjind:th kanji into buf from stream:
   (which should be assigned to fontfile).
 */
readkanji(buf,kanjind)
BYTE *buf;
int kanjind;
{
        FILE *stream;

        if(kanjind >= JIS2LEVEL_LIM)
         { /* If font2file is not opened, then
               show level 2 kanjis as invalids: */
           if(!font2_fp) { kanjind = get_invalid_kanji(); stream = font1_fp; }
           else { kanjind -= JIS2LEVEL_LIM; stream = font2_fp; }
         }
        else /* JIS 1 level character */
         {
           stream = font1_fp;

/* Make adjustments for K16JIS1.FNT.
   Kanjis which start from 499 at KANJI24.JIS start from 524 in it,
   and digits, roman alphabet, hiragana & katakana which start from 108
   start from 147 in it.
 */
/* Obsolete stuff, we use now font files JIS24LE1.FNT & JIS24LE2.FNT
    (two pieces of JIS24) were kanjis are just in same order as in
    JIS code instead of KANJI24.JIS were they were in still different
    order.
           if(adjust_flag)
            {
              if(kanjind >= 499) { kanjind += 25; }
              else if(kanjind >= 108) { kanjind += 39; }
            }

   But for 16*16 font we still use K16JIS1.FNT, so correct indexes from
    ordinary JIS code to indexes to that file:
 */
           if(adjust_flag) { kanjind = index_to_k16index(kanjind); }
         }

        myfseek(stream,
         (((ULI) skip_bytes) + ((ULI) (bytes_in_bitmap * ((ULI) kanjind))))
              ,0);
        readnbytes(buf,bytes_in_bitmap,stream);
}


/* Draws jis_index:th kanji from fontfile to screen, so that its upper left
    corner is at location <col,y> where col is measured as "character columns"
    i.e. 0-79 (* 8 = pixel columns) and y is pixel line from 0-199
 */
drawkanji(jis_index,col,y)
int jis_index,col,y;
{
        BYTE inverse_flag;

        /* If jis_index is negative, then show kanji inversed: */
        if(inverse_flag = (jis_index < 0)) { jis_index = -jis_index; }

        /* Don't read kanji in if it is already in bytebuf: */
        /* Kind of "one kanji cache..." Maybe I'll borrow later
           cache code from KD kanjidriver by Izumi Ohzawa.
           (for level2 kanjis, so that level2 font files wouldn't
            need to be at RAM disk...)
         */
        if(jis_index != previous_jis_index)
         {
           readkanji(_kanji_bytebuf,jis_index);
         }

        _drawkanji(col,y,_kanji_bytebuf,inverse_flag);
        previous_jis_index = jis_index;
}


_drawkanji(col,y,bytbuf,inverse_flag)
int col,y;
BYTE *bytbuf;
BYTE inverse_flag;
{
        UINT reverse_byte(),double_byte();

/* Maybe some problems in machines like PDP-16 with this kind of expressions:
        BYTE twobytes[2];
        int *iptr=((int *) twobytes);
 */
/* This should cause no problems for processors like 68k which require
    integers to be at even-byte addresses:
   (Probably they have some more sensible hires system where this bit doubling
     is unnecessary).
 */
        UINT doubled_byte;
        BYTE *byteptr=((BYTE *) &doubled_byte);
        BYTE byytti;

        int i,j,count=0;

        j = KANJIHEIGTH_IN_PIXELS;
        while(j--)
         {
           for(i=0; i < KANJIWIDTH; /* ++?++ */)
            {

              if(swap_flag)
               { /* Take first "later" byte in buffer,
                      and after that, "earlier" byte: */
                 if(!(count++ & 1)) { byytti = *++bytbuf; } /* count even */
                 else { byytti = *(bytbuf++ - 1); } /* count odd */
               }
              else { byytti = *bytbuf++; }

              if(inverse_flag) { byytti = inverse_byte(byytti); }
              if(reverse_flag) { byytti = reverse_byte(byytti); }

              if(double_flag)
               {
                 doubled_byte = double_byte(byytti);
                 plot8bits((col+(i++)),y,*(byteptr+1));
                 plot8bits((col+(i++)),y,*(byteptr));
               }
              else { plot8bits((col+(i++)),y,byytti); }
            }
           towardlower(y,1);
         }
}

file_status()
{
        fprintf(NOTICE_FP,
"%u level 1 characters, %u level 2 characters, %u codes out of range\n",
         jis1level_cnt,jis2level_cnt,get_jis_errcnt());
}


ptr_status(own_ptr)
TOB own_ptr;
{
              fprintf(NOTICE_FP,
"own_ptr: %6ld  last_ptr: %6ld  curs: %6ld  first: %6ld  last: %6ld\n",
               clist_diff(own_ptr,vks_vector),
               clist_diff(last_ptr,vks_vector),
               clist_diff(cursorpointer,vks_vector),
               clist_diff(first_in_realscreen,vks_vector),
               clist_diff(last_in_realscreen,vks_vector));
}


/* ===================================================================== */



/* This writes normal ascii text to location column,line but
    doesn't affect where other ascii output is printed.
    (That's it: the cursor location is saved.)
 */
texttoloc(text,column,line)
char *text;
UINT column,line;
{
        UINT savecolumn,saveline;

        scr_loc(&saveline,&savecolumn);
        scr_curs((k_upperedge + line),(k_leftedge + column));
        fputs(text,k_own_fp);
        scr_curs(saveline,savecolumn);
}

/* Put KANJIWIDTH stuffs to cursor location, but first save original contents
    of that line to curs_savebuf */
plotcursor(column,line,stuff)
UINT column,line,stuff;
{
        UINT i;

        line = getcursorline(line);
        for(i=0; (i < KANJIWIDTH); i++)
         {
           curs_savebuf[i] =
            read8bits((k_leftedge + column),(k_upperedge + line));
           plot8bits((k_leftedge + column),(k_upperedge + line),stuff);
           column++;
         }
}

/* Restore original pixels from curs_savebuf into cursor location: */
wipe_cursor(column,line)
UINT column,line;
{
        UINT i;

        line = getcursorline(line);
        for(i=0; (i < KANJIWIDTH); i++)
         {
           plot8bits((k_leftedge + column),(k_upperedge + line),
                       curs_savebuf[i]);
           column++;
         }
}



setcursor()
{       /* Don't try to blink cursor if it is out of real screen: */
        if(isoutofrealscreen(car(cursorpointer))) { return; }
/*      if(eq(MARKED_EXPR,getvks_entry(car(cursorpointer)))) */
        if(!p_kanjip(cursorpointer))
         { inversecursor(); }
        else
         {
           plotcursor(getvks_column(car(cursorpointer)),
                       getvks_line(car(cursorpointer)),0xFF);
         }
}

wipecursor()
{
        if(isoutofrealscreen(car(cursorpointer))) { return; }
        if(!p_kanjip(cursorpointer))
         { inversecursor(); }
        else
         {
           wipe_cursor(getvks_column(car(cursorpointer)),
                        getvks_line(car(cursorpointer)));
         }
}


inversecursor()
{
        register TOB  emmental;

        emmental = car(cursorpointer);

        inverse_area((k_upperedge + getvks_line(emmental)),1,
                      (k_leftedge + getvks_column(emmental)),
                       qsprintnexpr(getvks_expr(emmental),NULL,MAX_EXPR_LEN));
}

inversekanjiarea(column1,column2,line)
int column1,column2,line;
{
        inverse_pixel_area(getkanjiline(line),KANJIHEIGTH_IN_PIXELS,
         (k_leftedge + column1),((column2 - column1) + KANJIWIDTH));
}



_k_scroll(n)
int n;
{
        if(abs(n) >= k_heigth) { clear_realscreen(); }
        else
         {
           scroll_area((n),k_upperedge,k_heigth,k_leftedge,k_width);
         }
}



knewlines(n)
int n;
{
        k_line += n;
        k_column = 0;
}


kanjinewlines(n)
int n;
{
        knewlines((n) * HEIGTH_OF_KANJI);
}


kanji_et_text_newlines(n)
int n;
{
        knewlines((n) * HEIGTH_OF_KANJI_ET_TEXT);
}



kcharspaces(n)
int n;
{
        k_column += n;
}

kanjispaces(n)
int n;
{
        k_column += (n * DEFAULT_ADD);
}


int getjis_index(entry)
TOB entry;
{
        if(!kanjip(entry)) { return(0); }

        if(intp(entry)) { return(tob_int(entry)); }

        entry = getfield(_JI_,entry);
        if(!intp(entry)) { return(get_invalid_kanji()); }
        else { return(tob_int(entry)); }
}


/* Quoteless sprintexpr: */
int qsprintnexpr(expr,buf,n)
TOB expr;
char *buf;
int n;
{
        BYTE saveflag;
        int zoo;

        saveflag = printquotesflag;
        printquotesflag = 0;
        zoo = sprintnexpr(expr,buf,n);
        printquotesflag = saveflag;
        return(zoo);
}


int putkanji(entry)
TOB entry;
{
        return(putkanji_et_expr(entry,NIL));
}

int putexpr(expr)
TOB expr;
{
        return(putkanji_et_expr(NIL,expr));
}

int putexpr_m(expr)
TOB expr;
{
        return(putkanji_et_expr(MARKED_EXPR,expr));
}


/* Puts kanji and/or expr, to Virtual Kanji Screen,
    and to real screen too if it fits.
   1 is returned if it didn't fit to real screen, 0 otherwise.
 */
int putkanji_et_expr(entry,expr)
TOB entry;
TOB expr;
{
        int len,result,line;
        int pieni_pehmoinen_pallo;
        UINT jis_index = 0;
        char buf[MAX_EXPR_LEN+2];

        if(nilp(expr)) { len = 0; }
        else
         {
           len = qsprintnexpr(expr,buf,MAX_EXPR_LEN);
         }

        pieni_pehmoinen_pallo = (!kanjip(entry) ? 0 : KANJIWIDTH);
        if(len <= k_width) /* length of expr is less than line width */
         { /* If expr (or kanji) doesn't fit to this line any more: */
           if(k_column >
              ((k_rightestcolumn+1) - max(len,pieni_pehmoinen_pallo)))
            { knewlines(!kanjip(entry) ? 1 : (ver_space + !nilp(expr))); }
/* expression !nilp(expr) means: only kanji -> 0, kanji + text -> 1 */
/*          { knewlines(!kanjip(entry) ? 1 : HEIGTH_OF_KANJI_ET_TEXT); } */
         }

        line = k_line;

        if(!kanjip(entry) && middle_flag) { line += middle_level; }

        if(put_to_vks(entry,expr,k_column,line)) { return(-1); }

        if(kanjip(entry))
         {
           jis_index = getjis_index(entry);
           if(jis_index >= JIS2LEVEL_LIM) { jis2level_cnt++; }
           else { jis1level_cnt++; }
         }

        if(line > getlowestline(entry))
         { result = 1; }
        else
         {
           if(nilp(first_in_realscreen)) { first_in_realscreen = last_ptr; }
           last_in_realscreen = last_ptr;
           result = 0;
           kanji_and_or_expr_to_loc(jis_index,entry,expr,k_column,line,buf);
         }

        if(len > k_width) /* If printed string longer than 80 chars. */
         {
           k_line   += (len / k_width);
           k_column += (len % k_width);
         }
        else
         {
           if(!kanjip(entry)) { k_column += (len+1); }
           else { k_column += max((len+1),DEFAULT_ADD); }
         }

        return(result);
}

kanji_and_or_expr_to_loc(jis_index,entry,expr,column,line,buf)
UINT jis_index;
TOB entry,expr;
int column,line;
char *buf;
{

        if(kanjip(entry))
         {
           kanjitoloc(jis_index,column,line);
         }
        if(!nilp(expr))
         {
           texttoloc(buf,column,
            (line + (kanjip(entry) ? (HEIGTH_OF_KANJI_ET_TEXT - 1) : 0)));
         }
}



redraw_vks_item(vks_item)
TOB vks_item;
{
        TOB  entry,expr;
        UINT column,line;
        char buf[MAX_EXPR_LEN+2];

        expr = getvks_expr(vks_item);
        if(!nilp(expr))
         {
           qsprintnexpr(expr,buf,MAX_EXPR_LEN);
         }
        entry  = getvks_entry(vks_item);
        column = getvks_column(vks_item);
        line   = getvks_line(vks_item);
        kanji_and_or_expr_to_loc(getjis_index(entry),entry,expr,column,line,
         buf);
}


int put_to_vks(entry,expr,column,line)
TOB entry,expr;
int column,line;
{
        add_to_vks(makevks_item(entry,expr,column,line));
}


int add_to_vks(vks_item)
TOB vks_item;
{
        if(nilp(last_ptr)) { last_ptr = vks_vector; }
        else { last_ptr = cdr(last_ptr); }

        if(nilp(last_ptr))
         {
           erprintf((ER,
"\007**add_to_vks: vks_vector full ! vks_entries: %d  length(vks_vector): %d\n",
             vks_entries,length(vks_vector)));
           return(-1);
         }

        rplaca(last_ptr,vks_item);
        return(0);
}


int del_last_from_vks()
{
        if(nilp(last_ptr)) { return(0); }
        if(last_ptr < vks_vector)
         {
           fprintf(stderr,"**del_last_from_vks: last_ptr < vks_vector !\n");
           inv2arg("del_last_from_vks",2,vks_vector,last_ptr);
         }
        if(eq(last_ptr,vks_vector)) { last_ptr = NIL; }
        else
         {
           last_ptr = prevcdr(last_ptr);
           return(1);
         }
}


/* Frees vks_item lists and temporary strings in expr fields from
    vks_vector from start to end inclusive.
 */
clear_piece_of_vks(start,end)
TOB start,end;
{
        register TOB ptr;
        TOB expr;

        ptr = start;

        do
         {
           expr = getvks_expr(car(ptr));
           if(stringp(expr)) { free_string(expr); }
           free_list(car(ptr));
           if(eq(ptr,end)) { break; }
           ptr = cdr(ptr);
         } while(1);
}


clear_vks()
{
        if(!nilp(last_ptr)) { clear_piece_of_vks(vks_vector,last_ptr); }

        cursorpointer = last_in_realscreen =
         first_in_realscreen = last_ptr = NIL;
        k_column = k_line = 0;
}


kcls() /* Clear Kanji Screen */
{
        clear_realscreen(); /* Clear real screen (physically) */
        clear_vks();        /* Clear virtual kanji screen (logically) */
}



TOB save_vks(screen_name)
TOB screen_name;
{
        register TOB ptr;
        TOB result;

        ptr = vks_vector;
        result = NIL;

        do
           {
/* Idiotic slow shit:
             result = cons(topcopy(car(ptr)),result);
 */
             result = cons(car(ptr),result);
/*           rplaca(ptr,NIL); */  /* Is this necessary ? Not really. */
             if(eq(ptr,last_ptr)) { break; }
             ptr = cdr(ptr);
           } while(1);


        result = nconc(listn(_QM_,screen_name,cursorpointer,
                               first_in_realscreen,
                                last_in_realscreen,last_ptr,
                                 int_tob(double_flag),ENDMARK),
                         nreverse(result));

/* Just reset pointers to vks_vector in clear_vks, but don't try to
    free anything: */
        last_ptr = NIL;
        clear_vks();
/*
        kcls();
 */
        return(result);
}



TOB restore_vks(saved_vks)
TOB saved_vks;
{
        TOB screen_name;

        /* 0th item in saved_vks is _QM_ (?) */
        screen_name         = nth(1,saved_vks);

/*
 Meditoipas t{t{ viel{ tarkemmin...
  Eli siis mit{ tehd{ kun softa muuten on DOUBLE ON modessa, mutta faili on
  luettu kapeilla charactereilla, ja sitten on INS:sill{ menty johonkin
  characteriin joka on taas n{ytetty leveell{, ja sitten DEL:ill{ palataan
  failiin, niin jos t{m{ koodi on mukana niin n{kyy kyll{ taas kapeana,
  mutta sitten kun menn{{n uudestaan INS:ill{ niin NEKIN n{kyy kapeana.
  Jos taas t{t{ koodinp{tk{{ ei ole niin kun palataan takas failiscreeniin,
  niin n{kyy leve{n{...

        if(filenamep(screen_name))
         {
           int dubbel_flag;
           dubbel_flag = tob_int(getfile_double_flag(value(screen_name)));
           if(dubbel_flag != double_flag)
            { set_double_flag(dubbel_flag); }
         }
 */

        if((fontsize > 16) &&
             (eq(screen_name,_G_HIRAGANA_) || eq(screen_name,_G_KATAKANA_)))
         {
           char *s;

           extend_kanjiwindow();
           w_clear(sidewindow);
           /* Print the group name vertically to sidewindow: */
           s = pname(screen_name);
           while(*s) { putc(*s++,sidewindow); w_newline(sidewindow); }
         }


        kcls(); /* Clear the old vks & screen first */


        cursorpointer       = nth(2,saved_vks);
        first_in_realscreen = nth(3,saved_vks);
        last_in_realscreen  = nth(4,saved_vks);
        last_ptr            = nth(5,saved_vks);


        /* Move items from saved_vks to vks_vector: */
        movelist(vks_vector,nthcdr(7,saved_vks));

        free_list(saved_vks);


        redraw_realscreen();
        return(screen_name);
}


redraw_realscreen()
{
        TOB ptr;

        if(nilp(first_in_realscreen) || nilp(last_in_realscreen))
         {
           erprintf(
       (ER,"**redraw_realscreen: first- or last_in_realscreen is NIL !\n"));
           return(-1);
         }

        ptr = first_in_realscreen;
        forward_if_filemarker(&ptr);

        while(!nilp(ptr))
         {
           if(!isoutofrealscreen(car(ptr)))
            { redraw_vks_item(car(ptr)); }
           if(eq(ptr,last_in_realscreen)) { break; }
           ptr = vks_next2(ptr);
         }
}


/* Subtracts count from line field of every item in vks vector,
    and determines first_in_realscreen and last_in_realscreen
     pointers. And redraw items which move back to real screen.
 */
adjust_vks(count)
int count;
{
        /* register */ TOB ptr;
        register int new_line;

        ptr = vks_vector;
        first_in_realscreen = NIL;

        forward_if_filemarker(&ptr);

        while(!nilp(ptr))
           {
             new_line = subvks_line_and_redraw(car(ptr),count);
             if(nilp(first_in_realscreen)
                 /* && !nilp(getvks_entry(car(ptr))) */
                     && (new_line >= k_upperedge))
              { first_in_realscreen = ptr; }
             if((new_line <= k_lowestline_for_kanji))
                  /* && !nilp(getvks_entry(car(ptr)))) */
              { last_in_realscreen  = ptr; }
             ptr = vks_next2(ptr);
           }

}



/* Subtracts count from line field of vks_item and redraws item if it
    is necessary */
/* New faster version at 25. 3. 1990: */
int subvks_line_and_redraw(vks_item,count)
TOB vks_item;
int count;
{
        BYTE previously_out;
        register int *ptr_to_line;
        TOB entry;

        entry = getvks_entry(vks_item);
        ptr_to_line = getvks_ptrtoline(vks_item);

        previously_out = is_out_of_realscreen(*ptr_to_line,entry);
        *ptr_to_line -= count;

        /* If vks_item was out before 'scrolling', but not anymore,
            then it should be redrawn to real screen: */
        if(previously_out && !is_out_of_realscreen(*ptr_to_line,entry))
         {
           redraw_vks_item(vks_item);
         }
        return(*ptr_to_line);
}


int isoutofrealscreen(vks_item)
TOB vks_item;
{
 if(nilp(vks_item)) { return(1); }
 return(is_out_of_realscreen(getvks_line(vks_item),getvks_entry(vks_item)));
}


int is_out_of_realscreen(line,entry)
int line;
TOB entry;
{
 return((line < k_upperedge) || (line > getlowestline(entry)));
}




setcursorpause(pause)
int pause;
{
        cursorpause = pause;
}


/* Blinks cursor until any key is pressed which is returned:
    (but not yet removed from the input buffer) */
UINT blinkcursor_et_pollchar()
{
    int c;
    int i;

    /* If cursorpointer not set yet, then set it to first kanji in vks: */
    if(goto_exprs_too)
     { /* If goto_exprs_too flag is on, then any other than filemarker entry
           can be used: */
       forward_if_filemarker(&cursorpointer);
     }
    else
     {
/*     if(!kanjip(getvks_entry(car(cursorpointer)))) */
       if(!p_kanji_or_markexp(cursorpointer))
        { cursorpointer = getptr_to_first(first_in_realscreen); }
     }

/*
    if(nilp(cursorpointer))
     { erprintf((ER,"**blinkcursor_et_pollchar: cursorpointer is NIL !\n")); }
 */

    do {
         setcursor();
         for(i=cursorpause; i ; i--)
          {
            if((c = scr_poll()) != -1)
             { wipecursor(); goto pois; }
          }
         wipecursor();
         for(i=cursorpause; i ; i--)
          {
            if((c = scr_poll()) != -1)
             { goto pois; }
          }
        } while(1);
pois:

 /* Make sure that c is not sign extended even if it is over 127: */
    return((UINT) (((UINT) c) & 0x00FF)); /* This is it, the mystical shit. */
}



/* Gets pointer to first kanji vks_item from ptr onward:
   (which should point to somewhere in vks_vector)
 */
TOB getptr_to_first(ptr)
register TOB ptr;
{

/*      Not needed because of kanji_or_markexp test: */
/*      forward_if_filemarker(&ptr); */

        while(!nilp(ptr))
         {
           if(p_kanji_or_markexp(ptr)) { return(ptr); }
           ptr = vks_next2(ptr);
         }

        return(NIL);
}


/* Gets pointer to last kanji vks_item in vks_vector: */
/* Currently nobody is using this, so commented out:
TOB getptr_to_last()
{
        TOB ptr;

        ptr = last_ptr;

        while(!nilp(ptr))
         {
           if(p_kanji_or_markexp(ptr)) { return(ptr); }
           ptr = vks_prev2(ptr);
         }

        return(NIL);
}
*/


/* Gets pointer to lowest kanji or groupname vks_item in vks_vector: */
TOB getptr_to_lowestkanji()
{
     TOB ptr;
     int line,lowestline,rightestcolumn;
     TOB lowestptr,entry;

     ptr = vks_vector;
/*
     if(filemarkerp(getvks_entry(car(ptr)))) { ptr = vks_next(ptr); }
 */

     lowestptr = NIL;
     lowestline     = -32767;
     rightestcolumn = 0;

     while(!nilp(ptr))
      {
        entry = getvks_entry(car(ptr));
        if(kanji_or_markexp(entry))
         {
           line = getvks_line(car(ptr));
           if(kanjip(entry)) { line += (HEIGTH_OF_KANJI_ET_TEXT - 1); }
           if((line > lowestline) ||
        ((line == lowestline) && (getvks_column(car(ptr)) > rightestcolumn)))
            { lowestline = line; lowestptr = ptr; }
         }
        ptr = vks_next(ptr);
     }

     return(lowestptr);
}

/* Gets pointer to lowest vks_item in vks_vector: */
TOB getptr_to_lowest()
{
        TOB ptr;
        int line,lowestline,rightestcolumn;
        TOB lowestptr;

        ptr = vks_vector;
/*
        if(filemarkerp(getvks_entry(car(ptr)))) { ptr = vks_next(ptr); }
 */
        if(p_filemarkerp(ptr)) { ptr = vks_next(ptr); }

        lowestptr = NIL;
        lowestline     = -32767;
        rightestcolumn = 0;

        while(!nilp(ptr))
         {
           line = getvks_line(car(ptr));
           if(p_kanjip(ptr))
            { line += (HEIGTH_OF_KANJI_ET_TEXT - 1); }
           if((line > lowestline) ||
        ((line == lowestline) && (getvks_column(car(ptr)) > rightestcolumn)))
            { lowestline = line; lowestptr = ptr; }
           ptr = vks_next(ptr);
         }

        return(lowestptr);
}



/* Gets pointer to first filemarker vks_item from ptr onward:
   (which should point to somewhere in vks_vector)
 */
TOB getptr_to_next_filemarker(ptr)
register TOB ptr;
{
        while(!nilp(ptr))
         {
           if(p_filemarkerp(ptr)) { return(ptr); }
           if(eq(ptr,last_ptr)) { break; }
           ptr = cdr(ptr);
         }

        return(NIL);
}


/* Gets pointer to first filemarker before the ptr: */
TOB getptr_to_prev_filemarker(ptr)
register TOB ptr;
{
        while(!nilp(ptr))
         {
           if(p_filemarkerp(ptr)) { return(ptr); }
           if(eq(ptr,vks_vector)) { break; }
           ptr = prevcdr(ptr);
         }

        return(NIL);
}



static char *del_f_name = "**del_first_fileline_from_vks:";


del_first_fileline_from_vks(ptr_to_vksptr)
TOB *ptr_to_vksptr;
{
        int isfl_marker();
        TOB x;
        int difference;

/* If last_ptr is not compact cell, then it probably points to the last
    possible vks_item in vks_vector, and if tried to use memcpy for
    that case, it would mess up vks_vector. (= make it shorter by moving
    first non-compact cons cell much lower).
 */
        if((last_ptr < vks_vector) || !compactp(last_ptr))
         {
           fprintf(NOTICE_FP,
"%s last_ptr < vks_vector or not compact cell ! last_ptr: ",del_f_name);
           printhex(last_ptr,NOTICE_FP);
           terpri(NOTICE_FP);
           return;
         }

        x = mempred(isfl_marker,vks_vector);
        if(nilp(x))
         {
           fprintf(ERROR_FP,
"%s Not filescreen !\n",del_f_name);
           return;
         }

        x = mempred(isfl_marker,cdr(x));
        if(nilp(x)) /* There is only one line of the file in vks */
         {
           /* jotakin */
         }
        else /* Delete first file line */
         {
/* Check that these are not pointing to first file line: */
           if((*ptr_to_vksptr) < x) { return; }
           if(last_ptr < x) { return; }

/*
           if(last_in_realscreen < x) { return; }

           if(first_in_realscreen < x)
            { first_in_realscreen = getptr_to_first(x); }

           if(cursorpointer < x)
            { cursorpointer = getptr_to_first(x); }
 */
           if(first_in_realscreen < x) { first_in_realscreen = NIL; }
           if(last_in_realscreen  < x) { last_in_realscreen  = NIL; }
           if(cursorpointer < x)       { cursorpointer       = NIL; }


           clear_piece_of_vks(vks_vector,prevcdr(x));


/*         movelist(vks_vector,x); */
/* This is much faster than that movelist: */           
           memcpy(tob_ptr(vks_vector),tob_ptr(x),
                    getlow((last_ptr - x) + SIZEOF_INTERNAL));

           /* Adjust pointers to vks_vector: */
           difference = (x - vks_vector);
           (*ptr_to_vksptr)    -= difference;
           last_ptr            -= difference;

           if(!nilp(first_in_realscreen))
            { first_in_realscreen -= difference; }
           if(!nilp(last_in_realscreen))
            { last_in_realscreen  -= difference; }
           if(!nilp(cursorpointer))
            { cursorpointer       -= difference; }
         }
}


int isfl_marker(vks_item)
TOB vks_item;
{
        return(eq(getvks_entry(vks_item),FILELINE_MARKER));
}
        



TOB vks_next(vks_ptr)
TOB vks_ptr;
{
        TOB entry;

        if(nilp(vks_ptr) || eq(vks_ptr,last_ptr)) { return(NIL); }

        vks_ptr = cdr(vks_ptr);
        entry = getvks_entry(car(vks_ptr));

        if(eq(entry,FILEOFFSET_MARKER))
         {
#ifdef DEBUG
           if(debug_flag)
            {
              terpri(NOTICE_FP);
              file_status();
              ptr_status(vks_ptr);
              printexpr(car(vks_ptr),NOTICE_FP); /* Fileoffset marker */
              putc(' ',NOTICE_FP);
              printexpr(car(vks_vector),NOTICE_FP);/* 1st one in vks_vector */
            }
#endif
           if(getoffmark_fileline(car(vks_ptr)) == EOF) { return(NIL); }
           if(nilp(read_and_show_one_line(car(vks_ptr)))) { return(NIL); }
           del_first_fileline_from_vks(&vks_ptr);
#ifdef DEBUG
           if(debug_flag)
            {
              putc(' ',NOTICE_FP);
              printexpr(car(vks_ptr),NOTICE_FP); /* First new one */
            }
#endif
           return(vks_next(vks_ptr));
         }

        if(eq(entry,FILELINE_MARKER)) /* Skip file line markers */
         {
           return(vks_next(vks_ptr));
         }

        return(vks_ptr);
}


TOB vks_prev(vks_ptr)
TOB vks_ptr;
{
        TOB entry;

        if(nilp(vks_ptr) || eq(vks_ptr,vks_vector)) { return(NIL); }

        vks_ptr = prevcdr(vks_ptr);
        entry = getvks_entry(car(vks_ptr));

        if(eq(entry,FILEOFFSET_MARKER))
         {
           /* jotakin */
         }
        if(eq(entry,FILELINE_MARKER)) /* Skip file line markers */
         {
           return(vks_prev(vks_ptr));
         }

        return(vks_ptr);
}

/* 
   Functions vks_next2 & vks_prev2:
    These are like previous, but they doesn't follow file offset pointers:
 */
TOB vks_next2(vks_ptr)
TOB vks_ptr;
{
        TOB entry;

        if(nilp(vks_ptr) || eq(vks_ptr,last_ptr)) { return(NIL); }

        vks_ptr = cdr(vks_ptr);
        entry = getvks_entry(car(vks_ptr));

        if(eq(entry,FILEOFFSET_MARKER))
         {
           return(NIL);
         }
        if(eq(entry,FILELINE_MARKER)) /* Skip file line markers */
         {
           return(vks_next2(vks_ptr));
         }

        return(vks_ptr);
}


TOB vks_prev2(vks_ptr)
TOB vks_ptr;
{
        TOB entry;

        if(nilp(vks_ptr) || eq(vks_ptr,vks_vector)) { return(NIL); }

        vks_ptr = prevcdr(vks_ptr);
        entry = getvks_entry(car(vks_ptr));

        if(eq(entry,FILEOFFSET_MARKER))
         {
           return(NIL);
         }
        if(eq(entry,FILELINE_MARKER)) /* Skip file line markers */
         {
           return(vks_prev2(vks_ptr));
         }

        return(vks_ptr);
}





/* Moves cursor to first kanji or marked expression (e.g. groupname) at right:
 */
move_right()
{
        TOB ptr;
        int curslin;

        /* Save this because del_first_fileline may change cursorpointer,
            when following fileoffset marker: */
        curslin = getvks_line(car(cursorpointer));

        ptr = vks_next(cursorpointer);

        while(!nilp(ptr))
         {
           if(goto_exprs_too || p_kanji_or_markexp(ptr))
            { cursorpointer = ptr; break; }
           ptr = vks_next(ptr);
         }

        scroll_if_necessary(curslin);
}


move_left()
{
        TOB ptr;
        int curslin;

        /* Save this because del_first_fileline may change cursorpointer,
            when following fileoffset marker: */
        curslin = getvks_line(car(cursorpointer));

        ptr = vks_prev(cursorpointer);

        while(!nilp(ptr))
         {
           if(goto_exprs_too || p_kanji_or_markexp(ptr))
            { cursorpointer = ptr; break; }
           ptr = vks_prev(ptr);
         }

        scroll_if_necessary(curslin);
}


/* Set cursorpointer to point first kanji or marked expression, which is:
    a) at first line below the current line containing kanjis or markexprs.
    b) whose column is nearest to current column.
   However, if cursorpointer is already pointing to last kanji,
    then do nothing.
 */
move_down()
{
        register TOB ptr,car_of_ptr;
        TOB feasible1,feasible2;
        int column,column2,line,line2,line3;
        int col_feasible1,col_feasible2;

        column = getvks_column(car(cursorpointer));
        line   = getvks_line(car(cursorpointer));

        line3 = 32767; /* Biggest positive integer */
        feasible1 = feasible2 = NIL;

        ptr = vks_next(cursorpointer);

        while(!nilp(ptr))
         {
           car_of_ptr = car(ptr);
           line2 = getvks_line(car_of_ptr);
           /* If first line which is below: */
           if((line2 > line) &&
              (goto_exprs_too || kanji_or_markexp(getvks_entry(car_of_ptr))))
            { /* If below the first line below, then break: */
              if(line2 > line3) { break; }
              line3 = line2;
              column2 = getvks_column(car_of_ptr);
              if(column2 >= column)
               { /* First kanji which is at same column or right of it */
                 feasible2 = ptr;
                 col_feasible2 = column2;
                 break;
               }
              feasible1 = ptr; /* Set nearest kanji to left */
              col_feasible1 = column2;
            }
           ptr = vks_next(ptr);
         }

        /* Choose new cursorpointer: */
        if(!nilp(feasible2))
         {
           if(nilp(feasible1) ||
              ((col_feasible2 - column) < (column - col_feasible1)))
            { cursorpointer = feasible2; }
           else { cursorpointer = feasible1; }
         }
        /* If there was no kanji to right, then choose nearest kanji at
            left, if there is any: */
        else if(!nilp(feasible1))
         { cursorpointer = feasible1; }

        scroll_if_necessary(line);
}



/* Set cursorpointer to point first kanji or marked expression, which is:
    a) at first line above the current line containing kanjis or markexprs.
    b) whose column is nearest to current column.
   However, if cursorpointer is already pointing to first kanji,
    then do nothing.
 */
move_up()
{
        register TOB ptr,car_of_ptr;
        TOB feasible1,feasible2;
        int column,column2,line,line2,line3;
        int col_feasible1,col_feasible2;

Pist{ t{h{n v{liin ja muihin vastaaviin forward_if_filelinemarker(&cursorpointer);
Korjaa my|s scrollaus raw moodissa print_kanjis_with_keywordsissa !

        column = getvks_column(car(cursorpointer));
        line   = getvks_line(car(cursorpointer));

        ptr = vks_prev(cursorpointer);

        line3 = -32768; /* Biggest negative integer */
        feasible1 = feasible2 = NIL;

        while(!nilp(ptr))
         {
           car_of_ptr = car(ptr);
           line2 = getvks_line(car_of_ptr);

           /* If first line which is above: */
           if((line2 < line) &&
             (goto_exprs_too || kanji_or_markexp(getvks_entry(car_of_ptr))))
            { /* If above the first line above, then break: */
              if(line2 < line3) { break; }
              line3 = line2;
              column2 = getvks_column(car_of_ptr);
              if(column2 <= column)
               { /* First kanji which is at same column or left of it */
                 feasible2 = ptr;
                 col_feasible2 = column2;
                 break;
               }
              feasible1 = ptr; /* Set nearest kanji to right */
              col_feasible1 = column2;
            }
           ptr = vks_prev(ptr);
         }

        /* Choose new cursorpointer: */
        if(!nilp(feasible2))
         {
           if(nilp(feasible1) ||
              ((column - col_feasible2) < (col_feasible1 - column)))
            { cursorpointer = feasible2; }
           else { cursorpointer = feasible1; }
         }
        /* If there was no kanji to left, then choose nearest kanji at
            right, if there is any: */
        else if(!nilp(feasible1))
         { cursorpointer = feasible1; }

        scroll_if_necessary(line);
}


move_home()
{
        int curslin;

        cursorpointer = vks_vector;
        forward_if_filemarker(&cursorpointer);
        curslin = getvks_line(car(cursorpointer));
        scroll_if_necessary(curslin);
	if(!goto_exprs_too)
         {
           cursorpointer = getptr_to_first(vks_vector);
         }
}


move_end()
{
        int curslin;

/*      cursorpointer = last_ptr; */
        cursorpointer = getptr_to_lowest();
        curslin = getvks_line(car(cursorpointer));
        scroll_if_necessary(curslin);
/*	cursorpointer = getptr_to_last(); */
        if(!goto_exprs_too)
         {
           cursorpointer = getptr_to_lowestkanji();
         }
}


move_pgdn()
{
}

move_pgup()
{
}




scroll_if_necessary(line)
int line;
{
        int difference,acute_line;
        TOB entry;

        entry = getvks_entry(car(cursorpointer));
/*
        if(cursorpointer > last_in_realscreen)
 */
        /* If cursorpointer is "below" the lower edge of kanjiwindow: */
        if(line > getlowestline(entry))
         {
           acute_line = getlowestline(entry);
         }
/*      else if(cursorpointer < first_in_realscreen) */
        else if(line < k_upperedge)/* If cursorpointer above the upper edge */
         {
           acute_line = k_upperedge;
         }
        else { return; } /* No need for scroll */

        difference = line - acute_line;

        _k_scroll(difference);
        adjust_vks(difference);
}


/* ======================================================================== */

/* ...tastes a little strange ? */


print_HK_field(entry)
TOB entry;
{
        TOB x;

        if(!nilp(x = getfield(_HI_,entry)))
         {
           putexpr(getplaintext(_HI_));
           ksetcolumn(fieldstartcolumn);
           putexpr(x);
           x = getfield(_HK_,entry);
           ksetcolumn(meanings_column);
           printlistofkeywords(x);
           knewline();
         }
}


print_NK_field(entry)
TOB entry;
{
        TOB keywords,readings;

        if(nilp(keywords = getfield(_NK_,entry))) { return; }
        readings = getfield(_R_,entry);

        putexpr(getplaintext(_NI_));
        ksetcolumn(fieldstartcolumn);
        putexpr(getfield(_NI_,entry));

        if(!consp(keywords))
         { print_keywords_et_readings(keywords,readings); }
        else if(length1p(keywords))
         { print_keywords_et_readings(car(keywords),readings); }
        else
         {
           while(!nilp(keywords))
            {
              print_keywords_et_readings(car(keywords),car(readings));
              keywords = cdr(keywords);
              readings = cdr(readings);
            }
         }
}





print_keywords_et_readings(keywords,readings)
TOB keywords,readings;
{
        ksetcolumn(meanings_column);
        printlistofkeywords(keywords);
        ksetcolumn(readings_column);
        printlistofkeywords(readings);
        knewlines(1);
}


printfield(entry,key)
TOB entry,key;
{
        TOB x;

        if(!nilp(x = getfield(key,entry)))
         {
           putexpr(getplaintext(key));
           ksetcolumn(fieldstartcolumn);
           printlistofkeywords(x);
           knewline();
         }
}


/* Like previous, but marks the field to allow cursor to move onto it: */
printfield_m(entry,key)
TOB entry,key;
{
        TOB x;

        if(!nilp(x = getfield(key,entry)))
         {
           putexpr(getplaintext(key));
           ksetcolumn(fieldstartcolumn);
           printlistofkeywords_m(x);
           knewline();
         }
}



printlistofkeywords(lista)
TOB lista;
{
        char savesubstwithblankochar;

        if(nilp(lista)) { return; }

        savesubstwithblankochar = substwithblankochar;
        substwithblankochar = '_';
        if(!consp(lista)) { putexpr(lista); goto pois; }

        putexpr(car(lista));
	lista = cdr(lista);

        while(!nilp(lista))
         {
           k_column--;
           putexpr(_COMMA_);
           putexpr(car(lista));
           lista = cdr(lista);
         }
pois: ;
        substwithblankochar = savesubstwithblankochar;
}


printlistofkeywords_m(lista)
TOB lista;
{
        char savesubstwithblankochar;

        if(nilp(lista)) { return; }

        savesubstwithblankochar = substwithblankochar;
        substwithblankochar = '_';
        if(!consp(lista)) { putexpr_m(lista); goto pois; }

        putexpr_m(car(lista));
	lista = cdr(lista);

        while(!nilp(lista))
         {
           k_column--;
           putexpr(_COMMA_);
           putexpr_m(car(lista));
           lista = cdr(lista);
         }
pois: ;
        substwithblankochar = savesubstwithblankochar;
}


        




printreferfield(entry,key)
TOB entry,key;
{
        TOB keyword,x;

        if(nilp(x = getlfield(key,entry))) { return; }

        putexpr(getplaintext(key));
        ksetcolumn(fieldstartcolumn);
        printkanjis_with_keywords(x);

        if(length1p(x)) { free_list(x); }
        knewlines(HEIGTH_OF_KANJI_ET_TEXT /* +1 */ );
}


printcompounds(entry)
TOB entry;
{
        TOB compounds;

        if(nilp(compounds = getfield(_COMPOUNDS_,entry))) { return; }

        putexpr(getplaintext(_COMPOUNDS_));
        columns_info();

        print_compounds(compounds);
}

columns_info()
{
        ksetcolumn(meanings_column); putexpr(temp_stringsave("Meaning(s):"));
        ksetcolumn(readings_column); putexpr(temp_stringsave("Reading(s):"));
        knewline();
}


print_compounds(compound_list)
register TOB compound_list;
{
        knewline();
        while(!nilp(compound_list))
         {
           print_compound(car(compound_list));
           compound_list = cdr(compound_list);
         }
}


print_compound(compound)
TOB compound;
{
        BYTE save_middle_flag;


        printkanjis_with_keywords(getcompound_refers(compound));
/*
        k_line += middle_level;
 */
        save_middle_flag = middle_flag;
        middle_flag = 1;

        ksetcolumn(meanings_column);
        printlistofkeywords(getcompound_meanings(compound));
        ksetcolumn(readings_column);
        printlistofkeywords(getcompound_readings(compound));
/*      knewlines(KANJIHEIGTH); */
        kanji_et_text_newlines(1);

        middle_flag = save_middle_flag;
}


/* Prints the kanjis and associated keywords referenced by the references
    in referlist. (one after another from left to right).
   However if there is NIL (i.e. () ) in references that is understood
    as a request to print blanko (in terms of kanjies). Likewise the slash
    (/) will cause the newline.
 */
_printkanjis(referlist,keywords_too)
register TOB referlist;
BYTE keywords_too;
{
        TOB entry,keyword;
        int jack;

        while(!nilp(referlist))
         {
           if(nilp(car(referlist))) { kanjispaces(1); } /* Blanko */
           else if(eq(car(referlist),_SLASH_)) /* Newline */
            {
              if(keywords_too) { kanji_et_text_newlines(1); }
              else { kanjinewlines(1); }
            }
           else /* It is normal reference */
            {
              if(keywords_too)
               {
                 entry = getentry_et_prsymb(&keyword,car(referlist));
                 jack = putkanji_et_expr(entry,keyword);
               }
              else
               {
                 entry = get_entry(car(referlist));
                 jack = putkanji(entry);
               }
              /* If no more items fit into vks_vector, then break. */
              if(jack == -1) { break; }

              if(rawprintflag)
               { /* Forces screen to scroll, so that last printed kanji is
                     always in screen. */
                 cursorpointer = last_ptr;
                 scroll_if_necessary();
                 cursorpointer = NIL;
               }
            }
           referlist = cdr(referlist);
         }
}


/* Print in first column JIS index (decimal), in second column its JIS code
    format, and in third column its Shift-JIS code in hexadecimal. */
print_JI_field(entry)
TOB entry;
{
        char *itohex();
        UINT i,lo,hi;
        char buf[81];

        if(nilp(entry)) { return; }

        putexpr(getplaintext(_JI_));

        ksetcolumn(fieldstartcolumn);
        putexpr(entry = getfield(_JI_,entry));

        i  = tob_int(entry);
        lo = index_to_jis(i);
        hi = gethighbyte(lo);
        lo = getlowbyte(lo);
        sprintf(buf,"[%c%c]",hi,lo);
        ksetcolumn(meanings_column);
        putexpr(temp_stringsave(buf));
        
        ksetcolumn(readings_column);
        putexpr(temp_stringsave(itohex(buf,index_to_sj(i))));

        knewline();
}


print_entry(entry)
TOB entry;
{
        kcls();
        if(equentryp(entry)) { print_group(getgroup_name(entry)); return; }
        putkanji(entry);
/*      cursorpointer = last_ptr; */
        kanji_et_text_newlines(1);
        print_HK_field(entry);
        print_NK_field(entry);
        printfield(entry,_HIRAGANA_);
        printfield(entry,_KATAKANA_);
        printfield(entry,_OTHER_);
        printfield(entry,_P_);
        print_JI_field(entry);
        printfield(entry,_COMMENT_);
        printreferfield(entry,_E_);
        printreferfield(entry,_S_);
        printreferfield(entry,_SEE_);
        knewline();
        printfield_m(entry,_G_);
        printcompounds(entry);
}


print_group(group)
TOB group;
{
        TOB comment;

        if((fontsize > 16) &&
              (eq(group,_G_HIRAGANA_) || eq(group,_G_KATAKANA_)))
         {
           char *s;

           extend_kanjiwindow();
           kcls();
           w_clear(sidewindow);
           /* Print the group name vertically to sidewindow: */
           s = pname(group);
           while(*s) { putc(*s++,sidewindow); w_newline(sidewindow); }

           printkanjis_with_keywords(getgroup_reflist(value(group)));
/*         cursorpointer = getptr_to_first(); */
           return;
         }


        /* Fetch value if supplied with group name (i.e. like $NUMBERS) */
        if(groupnamep(group))
         {
           if(compoundgroupnamep(group))
            {
              putexpr(group);
              if(!nilp(comment = getgroup_comment(value(group))))
               { knewlines(1); putexpr(comment); }
              knewlines(1);
              columns_info();
              print_compounds(getgroup_reflist(value(group)));
              knewlines(2);
              return;
             }
            else { group = value(group); }
         }

        putexpr(getgroup_name(group));
        if(!nilp(comment = getgroup_comment(group)))
         { knewlines(1); putexpr(comment); }
        knewlines(1);
        printkanjis_with_keywords(getgroup_reflist(group));
        kanji_et_text_newlines(1);
        knewlines(1);
/*      cursorpointer = getptr_to_first(); */
}


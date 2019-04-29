

#include "kdinclu.h"

int is_sj_firstchar();

char *index(),*rindex(),*sindex();
char *first_in_seq(),*is_out_seq();

static jmp_buf to_show_one_line;

static BYTE sj_flag;

TOB read_and_show_one_line(fileoffset_marker)
TOB fileoffset_marker;
{
        char *myfgets();
        long ftell();
        TOB result,file_item,filename;
        FILE *input;
        int offset,fileline;

#define MAXIM_BUFO 512
        char buf[MAXIM_BUFO+3];

        file_item = value(filename = getoffmark_filename(fileoffset_marker));
        input = getfile_fp(file_item);
/*      offset = getoffmark_offset(fileoffset_marker); */
        fileline = getoffmark_fileline(fileoffset_marker);

        /* If EOF on file: */
        if(!myfgets(buf,MAXIM_BUFO,input))
         { /* EOF in fileline field of offset_marker
               tells that file ends here: */
           setoffmark_fileline(fileoffset_marker,EOF);
           return(NIL);
         }

        offset = getlow(ftell(input));
        setoffmark_offset(fileoffset_marker,offset);
        setoffmark_fileline(fileoffset_marker,(fileline+1));
 /* Overwrite the previous fileoffset marker with a line just read in: */
        del_last_from_vks();

        if(show_one_line(buf,fileline,
             getfile_hor_space(file_item),
             getfile_ver_space(file_item),
             getfile_double_flag(file_item),
             getfile_sj_flag(file_item))
           < 0)
         { /* If vks_vector is full, then take the last item away, and
               put fileoffset marker indicating EOF condition: */
           del_last_from_vks();
           setoffmark_fileline(fileoffset_marker,EOF);
           result = NIL;
         }
        else { result = _T_; }

        if(add_to_vks(fileoffset_marker) < 0)
         {
           del_last_from_vks();
           setoffmark_fileline(fileoffset_marker,EOF);
           add_to_vks(fileoffset_marker);
           return(NIL);
         }
        return(result);
}


show_one_line(line,lineno,hor__space,ver__space,double__flag,sj__flag)
char *line;
UINT lineno;
TOB hor__space;
TOB ver__space;
TOB double__flag;
TOB sj__flag;
{
        extern BYTE middle_flag;
        extern int ver_space,defaultspace,kanjiheigth;
        int save_defaultspace,save_ver_space;
        int r;
        BYTE save_middle_flag;
        BYTE save_double_flag;

        save_middle_flag = middle_flag;
        middle_flag = 1;

        if(intp(hor__space))
         {
           save_defaultspace = defaultspace;
           defaultspace = tob_int(hor__space);
         }

        if(intp(ver__space))
         {
           save_ver_space = ver_space;
           ver_space = (KANJIHEIGTH + tob_int(ver__space));
         }

        if(intp(double__flag))
         {
           save_double_flag = get_double_flag();
           set_double_flag(tob_int(double__flag));
         }

        sj_flag = ((nilp(sj__flag) || eq(sj__flag,ZERO)) ? 0 : 1);

        if(!setjmp(to_show_one_line))
         {
           if(add_to_vks(makefileline_marker(lineno)) < 0) { r = -1; }
           else { _show_one_line(line); r = 0; }
         }
        else { r = -1; } /* Error in some lower level function */

        if(intp(hor__space))   { defaultspace = save_defaultspace; }

        if(intp(ver__space))   { ver_space = save_ver_space; }

        if(intp(double__flag)) { set_double_flag(save_double_flag); }

        middle_flag = save_middle_flag;

        return(r);
}


_show_one_line(line)
char *line;
{
        char *showjiscodes(),*show_sj_codes();
        char *n; /* Pointer to next (Shift- or ESC-) JIS codes */
        char savechar;

        while(*line)
         {
           n = /* Where start next JIS codes in this line ? */
            (sj_flag ? fun_index(line,is_sj_firstchar) : first_in_seq(line));

           if(!n)/* Last ascii piece of line, no more JIS codes encountered */
            {
              put_string(line);
              break;
            }
           else
            {
              savechar = *n;
/* Put temporary end mark to line, so that it can be printed: 
   (necessary only with sj_codes, because first_in_seq has anyway overwritten
    ESC with '\0')
 */
              *n = '\0';
              /* Don't print null strings: */
              if(*line) { put_string(line); }
              *n = savechar;
              line =
               (sj_flag ? show_sj_codes(n) : showjiscodes(n));
            }
         }

        do_newline();
}


/* This shows all kanjis corresponding to jiscodes from line onward.
   Stops when OUTSEQ is encountered (or string ends, although that is
   quite anomalous), and returns pointer to just after OUTSEQ
   (ordinary ascii again).
 */
char *showjiscodes(line)
/* unsigned */ char *line;
{
        UINT jis_index;
        TOB entry;
        char *n;

        while(*line && *(line+1))
         {
           if((n = is_out_seq(line)) || (n = is_out_seq(line+1)))
            { line = n; break; }
           jis_index = jis_to_index(((*line << 8) + *(line+1)));
           entry = nth_entry(jis_index,_JI_);
           if(nilp(entry)) { entry = int_tob(jis_index); }
           if(putkanji(entry) < 0) { longjmp(to_show_one_line,-1); }
           line += 2;
         }

        return(line);
}



/* This is like previous, but shows Shift-JIS codes: */
char *show_sj_codes(line)
unsigned char *line;
{
        UINT jis_index;
        TOB entry;

        while(is_sj_firstchar(*line) && *(line+1))
         {
           jis_index =
            sj_to_index(((*line) << 8) + *(line+1));
           entry = nth_entry(jis_index,_JI_);
           if(nilp(entry)) { entry = int_tob(jis_index); }
           if(putkanji(entry) < 0) { longjmp(to_show_one_line,-1); }
           line += 2;
         }

/* Coerce line because otherwise compiler emits stupid warning messages: */
        return((char *) line);
}



int is_sj_firstchar(c) /* Is c first character of Shift-JIS code ? */
UINT c;
{
        c &= 0xFF; /* If some stupid function has sign-expanded c */
        return((c >= 0x81) && (c <= 0xFC));
}


put_string(s)
char *s;
{
        char savechar;
        char *n;
        UINT krc,len;
        int r; /* Status code from putexpr */

        krc = getk_rightestcolumn()+1;

        while(*s)
         {
           len = strlen(s);
           /* If string doesn't fit to this line, then cut it: */
           if((len + getk_column()) > krc)
            {
              n = s + (krc - getk_column()); /* Let n point to next piece */
              savechar = *n;
/* Put temporary end mark to starting point of n, so that s can be printed: */
              *n = '\0';
              r = putexpr(temp_stringsave(s));
              *n = savechar;
              s = n;
              do_newline();
              if(r < 0) { longjmp(to_show_one_line,-1); }
            }
           else /* This is the last piece (fits to one line) */
            {
              if(putexpr(temp_stringsave(s)) < 0)
               { longjmp(to_show_one_line,-1); }
              break;
            }
          }
}


do_newline()
{
        knewlines(ver_space);
}



char *first_in_seq(line)
char *line;
{
     char *n;

     while(*line)
      {
        if(!(n = index(line,'\033'))) /* Search first ESC */
         {
           return(NULL); /* Didn't find any */
         }

        /* Old or new JIS: */
        if(streq("$B",(n+1)) || streq("$@",(n+1)))
         {
           *n = '\0'; /* Overwrite ESC */
           return(n+3);
         }
        else if(*(n+1) == 'K') /* ESC K   --   NEC Code */
         {
           *n = '\0';
           return(n+2);
         }

        /* Else it's some other ESC code, continue to search next ESC,
            if that would be Kanji-In sequence...
         */

        line = n+1;
      }

     return(NULL);
}


char *is_out_seq(line)
char *line;
{
        /* New or Old JIS code: */
        if(streq("\033(B",line) || streq("\033(J",line))
         { return(line + 3); }
        else if(streq("\033H",line)) { return(line+2); } /* NEC Code */
        else { return(NULL); }
}


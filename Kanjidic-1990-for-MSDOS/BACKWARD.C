

#include "stdio.h"
#include "ctype.h"
#include "mydefs.h"


char *sreverse();

ULI ftell();

int back_getc(fp)
FILE *fp;
{
        /* If at beginning already: */
        if(!ftell(fp)) { return(EOF); }
        myfseek(fp,-1L,1)
        return(getc(fp));
}


ULI back_line(buf,fp,location)
char *buf;
FILE *fp;
ULI location;
{
        ULI save_loc,result;
        int c;

        save_loc = ftell(fp);

        myfseek(fp,location,0);

        do {
             c = back_getc(fp);
             if((c == EOF) || (c == '\n')) { break; }
             *buf++ = c;
           } while(1);

        *buf = '\0';

        sreverse(buf);

        result = ftell(fp);

        myfseek(fp,save_loc,fp);

        return(result);
}



/* Move this later to strfuns2.c: */
/* Reverses string s in place. From K/R (first edition ?) page 59: */
char *sreverse(s)
char *s;
{
        int c,i,j;

        for((i = 0, j = (strlen(s)-1)); (i < j); (i++, j--))
         {
           c      = *(s+i);
           *(s+i) = *(s+j);
           *(s+j) = c;
         }

        return(s);
}


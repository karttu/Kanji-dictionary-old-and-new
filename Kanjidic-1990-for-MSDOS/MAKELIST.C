

#include "stdio.h"
#include "ctype.h"
#include "mydefs.h"


main(argc,argv)
int argc;
char **argv;
{
        char *itohex();
        UINT index_to_jis(),index_to_sj();
        UINT i,start,end;
        UINT hi,lo;
        char buf[81];

        if(argc < 3) { fprintf(stderr,"\nHnefatafl !\n"); exit(1); }

        start = atoi(*(argv+1));
        end   = atoi(*(argv+2));

        for(i=start; i <= end; i++)
         {
           lo = index_to_jis(i);
           hi = lo;
           lo &= 0xFF;
           hi >>= 8;
           printf("     \033$B%c%c\033(B  %6u  %c%c  %s\n",
             hi,lo,i,hi,lo,itohex(buf,index_to_sj(i)));
         }
}



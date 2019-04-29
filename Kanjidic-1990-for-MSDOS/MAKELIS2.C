

#include "stdio.h"
#include "ctype.h"
#include "mydefs.h"


main(argc,argv)
int argc;
char **argv;
{
        char *itohex();
        UINT index_to_jis(),index_to_sj();
        UINT i,i2,start,end,add;
        UINT hi,lo;
        char buf[81];

        if(argc < 4) { fprintf(stderr,"\nHnefatafl !\n"); exit(1); }

        start = atoi(*(argv+1));
        end   = atoi(*(argv+2));
        add   = atoi(*(argv+3));

        for(i=start; i <= end;)
         {
/* If i is divisible with add, and quotient is odd, then skip add steps,
    because they are already printed at right side: */
           if(!(i % add))
            {
              if((i / add) & 1) /* Quotient odd */
               {
                 i += add;
                 continue;
               }
              else 
               {
                 UINT laku;
                 laku = (i / (2 * add));
                 printf("\fPage %u %c\n\n",
                            laku,(laku + '!'));
               }
            }

           lo = index_to_jis(i);
           hi = lo;
           lo &= 0xFF;
           hi >>= 8;
           printf("     \033$B%c%c\033(B %6u  %c%c  %s    |",
             hi,lo,i,hi,lo,itohex(buf,index_to_sj(i)));

           i2 = (i+add);
           lo = index_to_jis(i2);
           hi = lo;
           lo &= 0xFF;
           hi >>= 8;
           printf("    \033$B%c%c\033(B %6u  %c%c  %s\n",
             hi,lo,i2,hi,lo,itohex(buf,index_to_sj(i2)));

           i++;
         }
}



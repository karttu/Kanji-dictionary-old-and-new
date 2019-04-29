

#include "stdio.h"
#include "ctype.h"
#include <sys/types.h>  /* netinet/in.h wants u_short and u_long from here */
#include <netinet/in.h>

#define progname (*argv)

#define MAXBUF 1024
char mybuf[MAXBUF+3];

main(argc,argv)
int argc;
char **argv;
{
    long int atol();
    unsigned long int count,index,length,linecount=0,indbuf[1];
    size_t items_written,items_to_be_written=1;

    if(argc < 2)
     {
       fprintf(stderr,
"usage: %s total_count_of_indexes\n",progname);
       exit(1);
     }

    count = atol(*(argv+1));
    index = count<<2; /* We multiply count by 4 to get the initial index. */

    while(fgets(mybuf,MAXBUF,stdin))
     {
       linecount++;
       sscanf(mybuf,"%lu",&length);
       *indbuf = htonl(index); /* Convert from Host to Network byte order. */
       if((items_written =
              fwrite(((char *)indbuf),sizeof(unsigned long int),
                      items_to_be_written,stdout)) != items_to_be_written)
	{
          fprintf(stderr,
"%s: wrote only %lu items (output file full?)\n",
               progname,((unsigned long int)items_written));
          break;
	}
       index += length;
     }

    if(linecount != count)
     {
       fprintf(stderr,
"%s: linecount (%lu) != count (%lu) Please rerun with a proper argument!\n",
          progname,linecount,count);
       exit(1);
     }
}

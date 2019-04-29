

#include "ctype.h"
#include "stdio.h"
#include <sys/types.h>  /* netinet/in.h wants u_short and u_long from here */
#include <netinet/in.h>

#define progname  "nph-jg"
#define inputfile "jis24x24.mug"

long int atol(),ftell();

char bufferi[BUFSIZ+2];
char errbuf[1024];
char output_written=0;

FILE *inFP;


fatal_error(s)
char *s;
{
    printf("HTTP/1.0 500 %s: %s\r\n\r\n",progname,s);
    exit(1);
}

unsigned long int read_index(inFP)
FILE *inFP;
{
    long int ftell();
    unsigned long int loc,result,resbuf[1];
    size_t items_read,items_to_be_read=1;

    loc = ftell(inFP);
 
    if((items_read = fread(((char *)resbuf),sizeof(unsigned long int),
                              items_to_be_read,inFP)) != items_to_be_read)
     {
       sprintf(errbuf,
"cannot read %u bytes at location %lu. of input-file %s!",
                 sizeof(unsigned long int),loc,inputfile);
       fatal_error(errbuf);
     }

    return(ntohl(*resbuf)); /* Convert from Network to Host byte order. */
}

int no_headers=0;

main(argc,argv)
int argc;
char **argv;
{
    unsigned long int argument,count,index,index2,length;
    size_t bytes_read,bytes_written,chunk;

    if(argc < 2)
     {
       fatal_error(
         "Not enough arguments (usage: nph-jg jis_index [no_headers])");
     }
    else
     {
       argument = atol(*(argv+1));
     }

    if(argc > 2) { no_headers = 1; }

    if(!(inFP = fopen(inputfile,"r")))
     {
       sprintf(errbuf,"Cannot open input-file %s",inputfile);
       fatal_error(errbuf);
     }

    /* Read the zeroth index, which we can use to calculate the number
        of GIF's in this file: */

    index = read_index(inFP);
    if((index & 3))
     {
       sprintf(errbuf,"Input-file %s: The header is not divisible by four",
                  inputfile);
       fatal_error(errbuf);
     }
    else { count = (index >> 2); }

    if(argument >= count)
     {
       sprintf(errbuf,
"The argument %lu is greater than %lu (= max. for the input-file %s)",
               argument,(count-1),inputfile);
       fatal_error(errbuf);
     }

    if(fseek(inFP,(argument<<2),SEEK_SET))
     {
       sprintf(errbuf,
"Cannot seek to location %lu of the input-file %s",(argument<<2),inputfile);
       fatal_error(errbuf);
     }

    index = read_index(inFP);
    if((argument+1) == count) /* If this is an index to the last GIF */
     { /* in the input file then we have to do some extra arithmetics: */
       /* Position the file pointer to the end of input file: */
       if(fseek(inFP,((unsigned long int)0),SEEK_END))
	{
          sprintf(errbuf,
"Cannot seek to the end of the input-file %s",inputfile);
          fatal_error(errbuf);
        }
       /* And read the index to that, i.e. the size of the input file: */
       index2 = ftell(inFP);
     }
    else /* It's not the last one, so read the index to the next one: */
     {
       index2 = read_index(inFP);
       if(index2 <= index)
        {
          sprintf(errbuf,
"Pointer for index %lu should be greater than the pointer for index %lu!",
                   (argument+1),argument);
          fatal_error(errbuf);
        }
     }

    /* Compute the length of GIF: */
    length = (index2 - index);

    if(fseek(inFP,index,SEEK_SET))
     {
       sprintf(errbuf,
"Cannot seek to location %lu of the input-file %s",index,inputfile);
       fatal_error(errbuf);
     }

/* We go through this loop (length/BUFSIZ)+1 times.
   (Only once if length <= BUFSIZ).
 */
    while(length)
     {
       if(length > BUFSIZ) { chunk = BUFSIZ; }
       else                { chunk = length; }

       if((bytes_read = fread(bufferi,1,chunk,inFP)) != chunk)
	{
          sprintf(errbuf,
                  "read only %lu bytes (%lu bytes missing from the end?)",
                         ((unsigned long int)bytes_read),
                         ((unsigned long int)(length-bytes_read)));
          fatal_error(errbuf);
	}

/* Write the HTTP-header before any binary output */
       if(!output_written && !no_headers)
	{
/* Do we need to print anything else? */
          printf("HTTP/1.0 200 OK\r\n");
          printf("Content-Length: %lu\r\n",length);
          printf("Content-Type: image/gif\r\n\r\n");
        }

       if((bytes_written = fwrite(bufferi,1,chunk,stdout)) != chunk)
	{
          sprintf(errbuf,
                  "wrote only %lu bytes (output file full?)",
                          ((unsigned long int)bytes_written));
          fatal_error(errbuf);
	}

       output_written = 1;
       length -= chunk;
     }
}


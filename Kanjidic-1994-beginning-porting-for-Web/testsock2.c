
#include <ctype.h>
#include <stdio.h>

#include "tcp.h"

#define BUFF_SIZE 200
unsigned char bufferi[BUFF_SIZE+5];
#define UPPERLIMIT (bufferi+BUFF_SIZE)

FILE *sockfdopen();
 
main(argc,argv)
int argc;
char **argv;
{
    FILE *sock_fp;
    int port;
    int timeout;
    int master_soc;
    int new_soc;
    int read_bytes,written_bytes;
    unsigned char *bufptr;

    if(argc < 3)
     {
       fprintf(stderr,"Usage: %s port timeout [do_select]\n",*argv);
       exit(1);
     }

    port = atoi(*(argv+1));
    timeout = atoi(*(argv+2));
    if(argc > 3) { set_select(); } else { clr_select(); }
    set_trace();

    fprintf(stderr,
      "%s: port=(%d.,%u.)   timeout=(%d.,%u.)\n",
      *argv,port,port,timeout,timeout);

    master_soc = do_bind(port);

    while(1)
     {
       sock_fp = sockfdopen(master_soc,timeout);

       bufptr = bufferi;
       while((read_bytes = getc(sock_fp)) != EOF)
	{
          if(bufptr >= UPPERLIMIT)
	   {
             break;
	   }
          else if(*(bufptr-1) == '!') { break; }
        }
       *bufptr = '@';

       fprintf(stderr,
        "%s: read_bytes=%d., (bufptr-bufferi)=%d.\n",*argv,
              read_bytes,(bufptr-bufferi));

/* Do the great PsykoEkstaze conversion for the stuff read in: */
       for(bufptr=bufferi; (*bufptr != '@'); bufptr++)
	{
          if(isascii(*bufptr) && isalpha(*bufptr))
	   {
             if(islower(*bufptr))      { *bufptr = toupper(*bufptr); }
             else if(isupper(*bufptr)) { *bufptr = tolower(*bufptr); }
	   }
        }

       written_bytes = fwrite(bufferi,sizeof(char),(bufptr-bufferi),sock_fp);

       fprintf(stderr,
        "%s: written_bytes=%d., (bufptr-bufferi)=%d.\n",*argv,
              written_bytes,(bufptr-bufferi));

/*
       fprintf(stderr,"%s: The result of shutdown=%d.\n",
                 *argv,shutdown(new_soc,2));
 */
       fprintf(stderr,"%s: The result of fclose=%d.\n",
                 *argv,fclose(sock_fp));
     }
}


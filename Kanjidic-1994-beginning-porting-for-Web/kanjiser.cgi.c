

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>

#define INTMP_FILE      "tmp/IN.TMP"
#define INREADY_FILE    "tmp/IN.READY"
#define OUTREADY_FILE   "tmp/OUT.READY"
 
main(argc,argv)
int argc;
char **argv;
{
    int c;
    char *s;
    FILE *inFP=NULL,*outFP=NULL;

    if(argc < 2)
     {
       fprintf(stderr,"%s: argument missing!\n",*argv);
       exit(1);
     }

    if(!(outFP = fopen(INTMP_FILE,"w")))
     {
       fprintf(stderr,"%s: Cannot open file %s for writing!\n",
           *argv,INTMP_FILE);
       exit(1);
     }

/*
    fprintf(outFP,"%s\n",*(argv+1));
 */
    for(s = *(argv+1); *s; s++)
     { /* Don't give escape characters to lisp-process! */
       if(*s == '\\') { continue; }
       putc(*s,outFP);
     }
    putc('\n',outFP);
    fclose(outFP);

/* Change the file mode to be writable for everybody, so that lisp-process
   can delete it even if it is run by different pid:
 */
    if(chmod(INTMP_FILE,0777))
     {
       fprintf(stderr,"%s: Cannot chmod %s to 0777!\n",
          *argv,INTMP_FILE);
       exit(1);
     }

    if(rename(INTMP_FILE,INREADY_FILE))
     {
       fprintf(stderr,"%s: Cannot rename %s to %s\n",
          *argv,INTMP_FILE,INREADY_FILE);
       exit(1);
     }

/* Now the lisp process in ugly handles the reference in INREADY_FILE,
   and when it's ready the result should be in OUTREADY_FILE:
 */

    while(!(inFP = fopen(OUTREADY_FILE,"r"))) {} /* Wait until it opens. */

/*  printf("Content-type: text/html\n\n"); */ /* Now done by lisp-program */
    while((c = fgetc(inFP)) != EOF) { putchar(c); }
    fclose(inFP);
    unlink(OUTREADY_FILE);

/* And that's all folks! */
}

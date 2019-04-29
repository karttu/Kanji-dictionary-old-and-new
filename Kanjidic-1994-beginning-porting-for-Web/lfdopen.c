 

#include <stdio.h>

#include "franzint.h"

#define progname "KServer"

/* This calls do_accept with its arguments, and then gives the result
   of that to fdopen to create a buffered file pointer.
 */
/* Real file descriptor is fetched from pointer, because as this is
    declared as "function" the Franz gives argument as reference (in
    Fortran style).
 */
lispval lfdopen(fd)
int *fd;
{
    FILE *resfp;
    char name[21]; /* Should be enough. */

    resfp = fdopen(*fd,"r+");
    if(!resfp) { perror(progname); return(nil); }
    else
     { /* Construct a name for this file descriptor and remember it: */
        sprintf(name,"$fd:%d",*fd);
	ioname[PN(resfp)] = (lispval) inewstr((char *)name);
	return(P(resfp));
     }
}


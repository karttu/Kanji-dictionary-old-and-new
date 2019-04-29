
#include "tcp.h"                /* The whole mess of include files */
#include "declarat.h" /* Declarations for PUBLIC, PRIVATE and ARGS1, etc. */

#include <stdio.h>

#define progname "KServer"

/* Use this value for waiting indefinitely in select: */
#define INDEF_TIMEOUT -1
 

#ifdef __svr4__
#define LISTEN_BACKLOG 32	/* Number of pending connect requests (TCP) */
#else
#define LISTEN_BACKLOG 5	/* Number of pending connect requests (TCP) */
#endif /* __svr4__ */


PRIVATE fd_set  open_sockets;   /* Mask of channels which are active */
PRIVATE int     num_sockets;    /* Number of sockets to scan */

PRIVATE int Select_Flag=0;
PRIVATE int Trace_Flag=0;


PUBLIC int set_select()
{
    int old_select;

    old_select = Select_Flag;
    Select_Flag = 1;
    return(old_select);
}


PUBLIC int clr_select()
{
    int old_select;

    old_select = Select_Flag;
    Select_Flag = 0;
    return(old_select);
}



PUBLIC int set_trace()
{
    int old_trace;

    old_trace = Trace_Flag;
    Trace_Flag = 1;
    return(old_trace);
}

PUBLIC int clr_trace()
{
    int old_trace;

    old_trace = Trace_Flag;
    Trace_Flag = 0;
    return(old_trace);
}

/*____________________________________________________________________
**
**                      Networking code
*/

/*              Bind to a TCP port
 *              ------------------
 *              Taken from CERN httpd server (HTDaemon.c) and greatly
 *              simplified by AK.
 *
 * On entry,
 *      port    is the port number to listen to;
 *              80      means "listen to anyone on port 80"
 *
 * On exit,
 *      returns         master_socket bound to port given.
 *      exits           if error encountered.
 */
PUBLIC int do_bind ARGS1(int, port)
{
    PRIVATE SockA   server_soc_addr;
    register SockA * sin = &server_soc_addr;
    int master_soc;
    int one=1;
    time_t cur_time;
/*
    struct linger Linger;
    Linger.l_onoff = 0;
    Linger.l_linger = 0;
*/


    if(Trace_Flag)
     {
       time(&cur_time);
       fprintf(stderr,
            "%s: Entering do_bind(%d.) at %s",
            progname,port,ctime(&cur_time));
     }

    if(Select_Flag)
     {
       FD_ZERO(&open_sockets);     /* Clear our record of open sockets */
       num_sockets = 0;
     }

    /*
     *      Set up defaults:
     */
    sin->sin_family = AF_INET;      /* Family = internet, host order  */
    sin->sin_port = htons(port);
    sin->sin_addr.s_addr = INADDR_ANY; /* Default: any address */

    /*  Create internet socket */
    master_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(master_soc<0) { perror(progname); exit(1); }

    if(Trace_Flag)
     {
       fprintf(stderr, "%s: Opened socket number %d\n",
                  progname,master_soc);
     }

    if((setsockopt(master_soc,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(one)))
            == -1)
     {
       perror(progname); exit(1);
     }

    if(bind(master_soc,(struct sockaddr*)&server_soc_addr,
                       sizeof(server_soc_addr))<0)
     {
       perror(progname); exit(1);
     }

    if(listen(master_soc, LISTEN_BACKLOG)<0)
     {
       perror(progname); exit(1);
     }

    if(Trace_Flag)
     {
       time(&cur_time);
       fprintf(stderr,
         "%s: Master socket(), bind() and listen() all OK at %s",
             progname,ctime(&cur_time));
     }

    if(Select_Flag)
     { /* The master socket is one we are skulking for: */
       FD_SET(master_soc, &open_sockets);
       if((master_soc+1) > num_sockets) { num_sockets=master_soc+1; }
     }

    return(master_soc);

} /* do_bind */

 


/* This does the select (optionally) and accept.
   Timeout is given in seconds.
   If timeout occurs returns zero to calling procedure, so that
   it can do something useful...
   (accept should never return a socket with value zero, right?)
 */
PUBLIC int do_accept ARGS2(int, master_soc, int, timeout)
{
    PRIVATE SockA client_soc_addr;
    int soc_addr_len = sizeof(client_soc_addr);
    time_t cur_time;
    int com_soc;        /* inet socket number to read on */

    if(Trace_Flag)
     {
       time(&cur_time);
       fprintf(stderr,
            "%s: Entering do_accept(%d.) (As unsigned = %u.) at %s",
            progname,timeout,timeout,ctime(&cur_time));
     }

do_again_select:
    if(!Select_Flag)
     {
       if(Trace_Flag)
        fprintf(stderr, "%s: Not doing select()\n",progname);
     }
    else
     {
       fd_set              read_chans;
       fd_set              write_chans;
       fd_set              except_chans;
       struct timeval      max_wait;
       int nfound;

       read_chans = open_sockets;
       FD_ZERO(&write_chans);
       FD_ZERO(&except_chans);
       
       max_wait.tv_sec = timeout;
       max_wait.tv_usec = 0;

       if(Trace_Flag)
        {
          if(timeout == INDEF_TIMEOUT)
	   {
             fprintf(stderr,"%s: Waiting indefinitely in select.\n",progname);
           }
          else
	   {
             fprintf(stderr,
                       "%s: Next timeout after %d hours %d mins %d secs\n",
                         progname,timeout/3600, (timeout/60)%60, (timeout%60));
	   }
          time(&cur_time);
          fprintf(stderr, "%s: %s (Mask=%x hex, max=%x hex) at %s",progname,
                         "Waiting for connection",
                         *(unsigned int *)(&read_chans),
                         (unsigned int)num_sockets,
                         ctime(&cur_time));
        }

#ifdef __hpux
       nfound = select(num_sockets, (int *) &read_chans,
			    (int *) &write_chans, (int *) &except_chans,
			    ((timeout != INDEF_TIMEOUT) ? &max_wait : NULL));
#else
       nfound = select(num_sockets, &read_chans,
			    &write_chans, &except_chans,
			    ((timeout != INDEF_TIMEOUT) ? &max_wait : NULL));
#endif
       if(nfound < 0)
        {   /* Interrupted */
/*        if(errno != EINTR) */
	   { perror(progname); }
          goto do_again_select;
        }

       if(nfound == 0)
        {  /* Timeout */
          /* Time to do some daily chore. */
          if(Trace_Flag)
	   {
             time(&cur_time);
             fprintf(stderr,"%s: Timeout occurred for select at %s",
                   progname,ctime(&cur_time));
           }
          return(0); /* Well, now we return zero to calling function. */
        }
     }

    if(Trace_Flag)
     {
       time(&cur_time);
       fprintf(stderr,"%s: Doing an accept at %s",
            progname,ctime(&cur_time));
     }

    com_soc = accept(master_soc,(struct sockaddr *)&client_soc_addr,
                       &soc_addr_len);
    if(com_soc < 0)
     {
/*     if(errno != EINTR) */
	{ perror(progname); }
       goto do_again_select;
     }

    if(Trace_Flag)
     {
       time(&cur_time);
       fprintf(stderr,"%s: Accepted new socket %d at %s",
            progname,com_soc,ctime(&cur_time));
     }

    return(com_soc);
}
 

/* This calls do_accept with its arguments, and then gives the result
   of that to fdopen to create a buffered file pointer.
 */
PUBLIC FILE *sockfdopen ARGS2(int, master_soc, int, timeout)
{
    FILE *resfp;

    resfp = fdopen(do_accept(master_soc,timeout),"r+");
    if(!resfp) { perror(progname); }
    return(resfp);
}
 

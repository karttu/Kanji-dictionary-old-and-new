
#include "tcp.h"                /* The whole mess of include files */
#include "declarat.h"
#include "franzint.h"

/*
   Again some code shamelessly scavenged from CERN HTTPD server...
   (from HTSUtils.c module of Daemon)

   ORIGINAL AUTHORS:
  	AL	Ari Luotonen	luotonen@dxcern.cern.ch
  	MD	Mark Dönszelmann 	duns@vxdeop.cern.ch
  
   HISTORY:
  	10 Mar 94  AL	First written.
 */

#define Our_Soggy_uVAX_BSD4_3   YES_WE_HAVE_IT

#if defined(Mips) || defined(_AUX) || (defined(VMS) && !defined(DECC)) || defined(Our_Soggy_uVAX_BSD4_3)
PRIVATE char * weekdays[7] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};
#endif

PUBLIC char * month_names[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};




PUBLIC char * http_time ARGS1(time_t*, t)
{
    static char buf[40];

    struct tm * gmt = gmtime(t);

#if defined(Mips) || defined(_AUX) || (defined(VMS) && !defined(DECC)) || defined(Our_Soggy_uVAX_BSD4_3)
    /* Mips, VAXC doesn't have strftime() :-( */
    sprintf(buf,"%s, %02d-%s-%02d %02d:%02d:%02d GMT",
	    weekdays[gmt->tm_wday],
	    gmt->tm_mday,
	    month_names[gmt->tm_mon],
	    gmt->tm_year % 100,
	    gmt->tm_hour,
	    gmt->tm_min,
	    gmt->tm_sec);
#else
    strftime(buf, 40, "%A, %d-%b-%y %H:%M:%S GMT", gmt);
#endif

    return buf;
}

/*
(cfasl 'httpdate.o '_get_HTTP_date 'get_HTTP_date "function")
 */

PUBLIC lispval get_HTTP_date()
{
    time_t cur_time;

    time(&cur_time);

    return((lispval) inewstr((char *)http_time(&cur_time)));
}
 

#ifdef CURRENTLY_COMMENTED_OUT

PUBLIC time_t parse_http_time ARGS1(char *, str)
{
    char * s;
    struct tm tm;
    time_t t;
#ifdef ISC3   /* Lauren */
    struct tm * gorl;		/* GMT or localtime */
    long z = 0L;
#endif

    if (!str) return 0;

    if ((s = strchr(str, ','))) {	/* Thursday, 10-Jun-93 01:29:59 GMT */
	s++;				/* or: Thu, 10 Jan 1993 01:29:59 GMT */
	while (*s && *s==' ') s++;
	if (strchr(s,'-')) {		/* First format */
	    CTRACE(stderr, "Format...... Weekday, 00-Mon-00 00:00:00 GMT\n");
	    if ((int)strlen(s) < 18) {
		CTRACE(stderr,
		       "ERROR....... Not a valid time format \"%s\"\n", s);
		return 0;
	    }
	    tm.tm_mday = make_num(s);
	    tm.tm_mon = make_month(s+3);
	    tm.tm_year = make_num(s+7);
	    tm.tm_hour = make_num(s+10);
	    tm.tm_min = make_num(s+13);
	    tm.tm_sec = make_num(s+16);
	}
	else {				/* Second format */
	    CTRACE(stderr, "Format...... Wkd, 00 Mon 0000 00:00:00 GMT\n");
	    if ((int)strlen(s) < 20) {
		CTRACE(stderr,
		       "ERROR....... Not a valid time format \"%s\"\n", s);
		return 0;
	    }
	    tm.tm_mday = make_num(s);
	    tm.tm_mon = make_month(s+3);
	    tm.tm_year = (100*make_num(s+7) - 1900) + make_num(s+9);
	    tm.tm_hour = make_num(s+12);
	    tm.tm_min = make_num(s+15);
	    tm.tm_sec = make_num(s+18);

	}
    }
    else {	/* Try the other format:  Wed Jun  9 01:29:59 1993 GMT */
	CTRACE(stderr, "Format...... Wkd Mon 00 00:00:00 0000 GMT\n");
	s = str;
	while (*s && *s==' ') s++;
	CTRACE(stderr, "Trying...... The Wrong time format: %s\n", s);
	if ((int)strlen(s) < 24) {
	    CTRACE(stderr, "ERROR....... Not a valid time format \"%s\"\n", s);
	    return 0;
	}
	tm.tm_mday = make_num(s+8);
	tm.tm_mon = make_month(s+4);
	tm.tm_year = make_num(s+22);
	tm.tm_hour = make_num(s+11);
	tm.tm_min = make_num(s+14);
	tm.tm_sec = make_num(s+17);
    }
    if (tm.tm_sec  < 0  ||  tm.tm_sec  > 59  ||
	tm.tm_min  < 0  ||  tm.tm_min  > 59  ||
	tm.tm_hour < 0  ||  tm.tm_hour > 23  ||
	tm.tm_mday < 1  ||  tm.tm_mday > 31  ||
	tm.tm_mon  < 0  ||  tm.tm_mon  > 11  ||
	tm.tm_year <70  ||  tm.tm_year >120) {
	CTRACE(stderr,
	"ERROR....... Parsed illegal time: %02d.%02d.%02d %02d:%02d:%02d\n",
	       tm.tm_mday, tm.tm_mon+1, tm.tm_year,
	       tm.tm_hour, tm.tm_min, tm.tm_sec);
	return 0;
    }

    tm.tm_isdst = -1;

    /*
     *	What a pain it is to get the timezone correctly.
     */

#if defined(sun) && !defined(__svr4__)
    t = timegm(&tm);
#else /* not sun, except svr4 */

    t = mktime(&tm);

/* BSD, have tm_gmtoff */
#if defined(SIGTSTP) && !defined(AIX) && !defined(__sgi) && !defined(_AUX) && !defined(__svr4__)
    {
	time_t cur_t = time(NULL);
	struct tm * local = localtime(&cur_t);
	t += local->tm_gmtoff;
	CTRACE(stderr,"TimeZone.... %02d hours from GMT\n",
	       (int)local->tm_gmtoff / 3600);
    }
#else /* SysV or VMS */
    {
#ifdef VMS
	CTRACE(stderr,"TimeZone.... undefined\n");
#else /* SysV */
#ifdef ISC3   /* Lauren */
	time_t cur_t = time(NULL);
	gorl = localtime(&cur_t);
	if (daylight && gorl->tm_isdst)	/* daylight time? */
	    z = altzone;	/* yes */
	else
	    z = timezone;	/* no */

	z /= 60;		/* convert to minutes */
	z = -z;			/* ISC 3.0 has it vice versa */
	t += z * 60;	
	CTRACE(stderr,"TimeZone.... %02d hours from GMT\n", z / 60);
#else
	int dst = 0;
	/*
	 * The following assumes a fixed DST offset of 1 hour,
	 * which is probably wrong.
	 */
	if (tm.tm_isdst > 0)
	    dst = -3600;
	t -= (timezone + dst);
	CTRACE(stderr,"TimeZone.... %02d hours from GMT\n", 
	       (timezone + dst) / 3600);
#endif
#endif /* SysV */
    }
#endif /* SysV or VMS */

#endif /* not sun, except svr4 */

    CTRACE(stderr, "Time string. %s", str);
    CTRACE(stderr, "Parsed...... to %ld seconds, %s", (long)t, ctime(&t));
    return t;
}


#endif /* COMMENTED OUT */

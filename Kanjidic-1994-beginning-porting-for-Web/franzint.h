 
/* ================================================================ */
/*
   franzint.h  --  an interface to the internal entrails and data
                   types of the Franz Lisp. Used by various c-modules
   I have written which are to be loaded in as foreign-functions into
   Franz Lisp.
   Shamelessly scavenged by karttu from various header files
   (at least global.h and lispval.h) found from Franz Lisp sources.
   I can add later here more definitions and declarations when needed...

 */


typedef union lispobj *lispval ;

/* ================================================================= */
  
/* 
   Some useful macros and definitions taken from the file global.h
   in Franz Lisp's sources.
 */

/* Let's assume that PORTABLE should be left undefined in our MicroVax
   BSD4.3 versions of Franz Lisp.
 */

#ifdef PORTABLE
extern struct atom nilatom, eofatom;
#define nil	((lispval) &nilatom)
#define eofa	((lispval) &eofatom)
#else
#define nil	((lispval) 0)
#define eofa	((lispval) 20)
#endif

/* In MicroVax BSD4.3 it seems to work better when this is defined: */
#define RTPORTS YES

#ifndef RTPORTS
extern FILE *xports[];		/* page of file *'s for lisp	*/
#define P(p)		((lispval) (xports +((p)-_iob)))
#define PN(p)		((int) ((p)-_iob))
#else
extern lispval P();
extern FILE **xports;
#define PN(p) (((FILE **)P(p))-xports)
#endif

extern lispval ioname[];	/* names of open files */
char *inewstr();

#define NOTNIL(a)	(nil!=((lispval)(a)))
#define ISNIL(a)	(nil==((lispval)(a)))
 
/* These four were defined by karttu, might be handy... */
#define MYCAR(a)        ((a)->d.car)
#define MYCDR(a)        ((a)->d.cdr)
#define MYRPLACA(a,x)   (MYCAR(a) = ((lispval)(x)))
#define MYRPLACD(a,x)   (MYCDR(a) = ((lispval)(x)))

#define GET_PORT(a)     ((a)->p)
/* ================================================================= */

/* These (as well as the typedef of lispval) are taken from lispval.h
   of Franz Lisp sources:
 */
 
struct dtpr {
	lispval	cdr, car;
};

struct sdot {
	int 	I;
	lispval	CDR;
};


struct	atom	{
	lispval		clb;		/* current level binding*/
	lispval 	plist;		/* pointer to prop list	*/
#ifndef WILD
	lispval		fnbnd;		/* function binding	*/
#endif
	struct	atom	*hshlnk;	/* hash link to next	*/
	char		*pname;		/* print name	*/
	};
#ifdef WILD
#define fnbnd clb
#endif

struct array {
	lispval accfun,		/*  access function--may be anything  */
		aux;		/*  slot for dimensions or auxilliary data  */
	char *data;		/*  pointer to first byte of array    */
	lispval length, delta;	/* length in items and length of one item */
};

struct bfun {
	lispval (*start)();	/*  entry point to routine  */
	lispval	discipline,	/*  argument-passing discipline  */
		language,	/*  language coded in	*/
		params,		/*  parameter list if relevant  */
		loctab;		/*  local table  */
};

struct Hunk {
	lispval hunk[1];
};

struct Vector {
        lispval vector[1];
};

/* the vectori types */
struct Vectorb {
    	char vectorb[1];
};

struct Vectorw {
       short  vectorw[1];
};

struct Vectorl {
    long vectorl[1];
};

union lispobj {
	struct atom a;
	FILE *p;
	struct dtpr d;
	long int i;
	long int *j;
	double r;
	lispval (*f)();
	struct array ar;
	struct sdot s;
	char c;
	lispval l;
	struct bfun bcd;
	struct Hunk h;
	struct Vector v;
	struct Vectorb vb;
	struct Vectorw vw;
	struct Vectorl vl;
};


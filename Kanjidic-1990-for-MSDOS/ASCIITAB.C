

#include "asciitab.h"


static int asc_to_jis_table[]=
 {
   UNKNOWN,		/*     0 */
   UNKNOWN,		/*     1 */
   UNKNOWN,		/*     2 */
   UNKNOWN,		/*     3 */
   UNKNOWN,		/*     4 */
   UNKNOWN,		/*     5 */
   UNKNOWN,		/*     6 */
   UNKNOWN,		/*     7 */
   UNKNOWN,		/*     8 */
   UNKNOWN,		/*     9 */
   NL,			/*    10 */
   MALE,		/*    11 */
   FEMALE,		/*    12 */
   UNKNOWN,		/*    13 */
   UNKNOWN,		/*    14 */
   UNKNOWN,		/*    15 */
   UNKNOWN,		/*    16 */
   UNKNOWN,		/*    17 */
   UNKNOWN,		/*    18 */
   UNKNOWN,		/*    19 */
   UNKNOWN,		/*    20 */
   UNKNOWN,		/*    21 */
   UNKNOWN,		/*    22 */
   UNKNOWN,		/*    23 */
   UNKNOWN,		/*    24 */
   UNKNOWN,		/*    25 */
   UNKNOWN,		/*    26 */
   UNKNOWN,		/*    27 */
   UNKNOWN,		/*    28 */
   UNKNOWN,		/*    29 */
   UNKNOWN,		/*    30 */
   UNKNOWN,		/*    31 */
   BLANKO,		/*    32 */
   EXCLAMATION,		/* !  33 */
   DOUBLEQUOTE,		/* "  34 */
   NUMBERSIGN,		/* #  35 */
   DOLLAR,		/* $  36 */
   PERCENT,		/* %  37 */
   AMPERSAND,		/* &  38 */
   QUOTE,		/* '  39 */
   LPAR,		/* (  40 */
   RPAR,		/* )  41 */
   ASTERISK,		/* *  42 */
   PLUS,		/* +  43 */
   COMMA,		/* ,  44 */
   MINUS,		/* -  45 */
   DOT,			/* .  46 */
   SLASH,		/* /  47 */
   ZERO,		/* 0  48 */
   ONE,			/* 1  49 */
   TWO,			/* 2  50 */
   THREE,		/* 3  51 */
   FOUR,		/* 4  52 */
   FIVE,		/* 5  53 */
   SIX,			/* 6  54 */
   SEVEN,		/* 7  55 */
   EIGHT,		/* 8  56 */
   NINE,		/* 9  57 */
   COLON,		/* :  58 */
   SEMICOLON,		/* ;  59 */
   SMALLERTHAN,		/* <  60 */
   EQUAL,		/* =  61 */
   GREATERTHAN,		/* >  62 */
   QUESTION,		/* ?  63 */
   MIAU,		/* @  64 */
   _A,			/* A  65 */
   _B,			/* B  66 */
   _C,			/* C  67 */
   _D,			/* D  68 */
   _E,			/* E  69 */
   _F,			/* F  70 */
   _G,			/* G  71 */
   _H,			/* H  72 */
   _I,			/* I  73 */
   _J,			/* J  74 */
   _K,			/* K  75 */
   _L,			/* L  76 */
   _M,			/* M  77 */
   _N,			/* N  78 */
   _O,			/* O  79 */
   _P,			/* P  80 */
   _Q,			/* Q  81 */
   _R,			/* R  82 */
   _S,			/* S  83 */
   _T,			/* T  84 */
   _U,			/* U  85 */
   _V,			/* V  86 */
   _W,			/* W  87 */
   _X,			/* X  88 */
   _Y,			/* Y  89 */
   _Z,			/* Z  90 */
   LBRACKET,		/* [  91 */
   BACKSLASH,		/* \  92 */
   RBRACKET,		/* ]  93 */
   UPWARDARROW,		/* ^  94 */
   UNDERSCORE,		/* _  95 */
   BACKQUOTE,		/* `  96 */
   _a,			/* a  97 */
   _b,			/* b  98 */
   _c,			/* c  99 */
   _d,			/* d 100 */
   _e,			/* e 101 */
   _f,			/* f 102 */
   _g,			/* g 103 */
   _h,			/* h 104 */
   _i,			/* i 105 */
   _j,			/* j 106 */
   _k,			/* k 107 */
   _l,			/* l 108 */
   _m,			/* m 109 */
   _n,			/* n 110 */
   _o,			/* o 111 */
   _p,			/* p 112 */
   _q,			/* q 113 */
   _r,			/* r 114 */
   _s,			/* s 115 */
   _t,			/* t 116 */
   _u,			/* u 117 */
   _v,			/* v 118 */
   _w,			/* w 119 */
   _x,			/* x 120 */
   _y,			/* y 121 */
   _z,			/* z 122 */
   LBRACE,		/* { 123 */
   VERTICALLINE,	/* | 124 */
   RBRACE,		/* } 125 */
   TILDE,		/* ~ 126 */
   UNKNOWN,		/*   127 */
 };


int asctojis(c)
int c;
{
        if(c & ~0x7F) { return(UNKNOWN); }
        else { return(asc_to_jis_table[c]); }
}



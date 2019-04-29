
#include "kdinclu.h"

/* 3395, 574, 2649, etc. are old indexes to fontfile KANJI24.JIS.
   Now they are made proper jis indexes by adding 911, because we use
   currently fontfiles JIS24LE1.FNT & JIS24LE2.FNT (two pieces of JIS24.FNT)
   where characters are just in same order as in JIS and Shift-JIS code.
 */

#define K_ZERO               3395+911 /*  0 */

#define K_ONE                 574+911 /*  1 */
#define K_TWO                2649+911 /*  2 */
#define K_THREE              1548+911 /*  3 */
#define K_FOUR               1577+911 /*  4 */
#define K_FIVE               1312+911 /*  5 */
#define K_SIX                3439+911 /*  6 */
#define K_SEVEN              1649+911 /*  7 */
#define K_EIGHT              2766+911 /*  8 */
#define K_NINE               1131+911 /*  9 */
#define K_TEN                1749+911 /* 10 */

#define K_TWENTY             2657+911 /* 20 */

#define K_HUNDRED            2868+911 /*              100 = 10^2 */
#define K_THOUSAND           2075+911 /*            1,000 = 10^3 */
#define K_TEN_THOUSAND       3128+911 /*           10,000 = 10^4 */
#define K_HUNDRED_MILLION     701+911 /*      100,000,000 = 10^8 */
#define K_TRILLION           2375+911 /* 1000,000,000,000 = 10^12 (biljoona)*/


#define OFL ((ULI) 10000) /* One Factor Limit */
/* Below OFL  factor 1 is implicit.
   i.e: 10, 100 and 1000 are represented as 10, 100 and 1000 (single kanji),
    but 10000 and 100,000,000 (and trillion) are represented as
     one + that kanji
   (Katso  Tadaaki Kawata: Japanin kieli (4. painos), s. 32).
 */

#define NUMS 15 /* Number of number kanjis in table */

/* Numbers which have their own character:
   (except K_TRILLION (= biljoona, 1,000,000,000,000) but that's over 2^32)
   (should I implement BIGNUMS to C ? Eh...)
 */
static ULI basic_nums[NUMS] =
 {
   0L, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L, 9L,
    10L, 100L, 1000L, 10000L, 100000000L
 };

/* Corresponding kanji-indexes: */
static int num_kanjis[NUMS] =
 {
   K_ZERO, K_ONE, K_TWO, K_THREE, K_FOUR, K_FIVE, K_SIX, K_SEVEN,
    K_EIGHT, K_NINE,
   K_TEN, K_HUNDRED, K_THOUSAND, K_TEN_THOUSAND, K_HUNDRED_MILLION
 };


/* Pointers to last elements of both arrays: */
/* This alternative doesn't work with overlays:
static ULI *lastnum = (basic_nums + (NUMS-1));
static int *lastk   = (num_kanjis + (NUMS-1));
 */

/* But this works: */
#define lastnum  (basic_nums + (NUMS-1))
#define lastk    (num_kanjis + (NUMS-1))


TOB negate_number();
TOB getjisentry();


/* int _NUMCON() */ /* Number Conversion */
ovmain()
{
        int ovmain();

        TOB ltok(),l;
        ULI atol(),num;
        register char *s;
        char buf[81],buf2[81];

        adr_of_ovmain = ovmain;

        if(!(gen_stringp(arg1) || intp(arg1)))
         { numcon_help(); return(-1); }

        if(intp(arg1)) /* Supplied 16-bit number */
         { num = to_uli(tob_uint(arg1)); }
        else /* Supplied symbol or string */
         {
           strcpy(buf,pname(arg1));
           s = buf;
           if(*s == '-') { s++; }
           while(*s) /* Delete non-digit characters, like commas: */
            { if(!isdigit(*s)) { strcpy(s,(s+1)); } else { s++; } }
           num = atol(buf);
         }

        if(rawprintflag) { knewlines(1); }
        else { kcls(); }

        sprintf(buf2,"%lu",num);

/* If argument was symbol or string and not negative nor too big,
    then print its original version (possibly containing commas which
    make big number clearer to understand). In other cases use
    buf2, which has been created with sprintf:
 */
        special_screen =
         temp_stringsave((!intp(arg1) && !strcmp(buf,buf2)) ? pname(arg1)
                                                             : buf2);
        putexpr(special_screen);
        putexpr(temp_stringsave("->"));
        knewlines(2);

/* Don't use kanji TWENTY unless number to be converted is exactly 20: */
        if(num == 20L) { l = list1(int_tob(K_TWENTY)); }
        else { l = ltok(num); }

        /* Change jis indexes to corresponding references: (negate them) */
        maprplaca(negate_number,l);

        printkanjis(l);

        free_list(l);
}




/* Choose number which is less or equivalent to x from array basic_nums,
    and let kp point to corresponding kanji-index in array num_kanjis.
 */
ULI lower_or_same(x,kp)
ULI x;
int *kp;
{
        register ULI *ptr;
        int *kptr;

        ptr = lastnum;
        kptr = lastk;

        while(*ptr > x) { --ptr; --kptr; }
        *kp = *kptr;
        return(*ptr);
}


/* Convert unsigned long integer x to Chinese/Japanese numerical notation.
    (Returns list of kanji-indexes equivalent to that number.)
   Note that kanji zero is never used except when x is 0L.
 */
TOB ltok(x)
ULI x;
{
        TOB ltok();
        ULI lower_or_same();

        int k;
        ULI y,factor,remainder;
        TOB result;

        y = lower_or_same(x,&k);

        result = list1(int_tob(k));
        if(y == x) /* x is some of the basic numbers */
         {
           /* put one to the front if needed: */
           if(x >= OFL) { result = cons(int_tob(K_ONE),result); }
           return(result);
         }
        else /* composite case */
         {
           factor     =  (x / y);
           remainder  =  (x % y);

           /* Don't put factor if it's 1 and number is below OFL: */
           if((factor != 1) || (y >= OFL))
            { result = nconc(ltok(factor),result); }
           /* Shouldn't put zero to end (that's superfluous): */
           if(remainder != 0)
            { result = nconc(result,ltok(remainder)); }

           return(result);
           /* Eh, quite nice recursive procedure ? */
         }
}


TOB negate_number(x)
TOB x;
{
        if(intp(x)) { return(int_tob(-(tob_int(x)))); }
        else { return(x); }
}



/* This is now unnecessary
TOB getjisentry(x)
TOB x;
{
        return(nth_entry(tob_int(x),_JI_));
}
 */


numcon_help()
{
        FILE *HELP_FP;

        HELP_FP = NOTICE_FP;
        w_clear(HELP_FP);
	fprintf(HELP_FP,
"Use # command to convert arabic number to number expressed with kanjis.\n");
        fprintf(HELP_FP,
"Examples: # 0   # 123,456,789   # -1 (= 65535)  # -1. (= 4,294,967,295)\n");
        fprintf(HELP_FP,
"Note that you can make big number clearer if you put commas between.\n");

}


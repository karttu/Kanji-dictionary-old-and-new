
/*
   KANA.C - Routines to convert romaji to hira/katakana.
   Coded by Antti Karttunen at 198? and 1990.
 */


#include "stdio.h"
#include "ctype.h"
/* #include "mydefs.h"
   #include "kanjifun.h"
 */
#include "kana.h"

typedef unsigned char BYTE;
typedef unsigned int UINT;

#define TO_ROMAJI 0
#define TO_HIRA   1
#define TO_KATA   2
#define TO_BOTH   3
/* According to case: lowercase -> hiragana, uppercase -> katakana */

BYTE conv_flag = TO_BOTH;
BYTE part_flag = 0; /* If this is 1 then particles are converted */



#define jis_code(C1,C2) (((94 * (C1)) + (C2)) - 3135) /* 94*(C1-33)+(C2-33) */

#define __       255 /* Exception or Error (not possible) */


#define HIRAGANA 0x3041 /* jis_code('$','!') */
#define KATAKANA 0x30A1 /* jis_code('%','!') */
#define H_TO_K   (KATAKANA - HIRAGANA)
 
 
 
int output_ucs_as_utf8(FILE *out, unsigned long int ucs)
{

    if(ucs < 128)
     {
       putc(ucs, out);
     }
    else if(ucs < 2048)
     {
 /* 110xxxxx,  bits 6-10 */
       putc(((0xC0) + ((ucs) >> 6)), out);
 /* 10xxxxxx, 6 lowest bits */
       putc(((0x80) + ((ucs)&077)), out);
     }
    else
     {
 /* 1110xxxx,  bits 12-15 */
       putc(((0xE0) + ((ucs)>> 12)), out);
 /* 10xxxxxx, bits 11-6 */
       putc(((0x80) + ((ucs>>6) & 077)), out);
 /* 10xxxxxx, 6 lowest bits */
       putc(((0x80)+((ucs) & 077)), out);
     }
}



/* When there is NOTHING in *secondkana, then getkana returned only one kana */
#define NOTHING  -1

/* Indexes to string in getkana: */
#define FIRST    0
#define SECOND   1
#define THIRD    2
#define FOURTH   3


#define isvowel(X) ((X) < 5) /* X must be fetched from lettertab. */

#define choosekana(X,Y) ((X) + ((Y) ? KATAKANA : HIRAGANA))

/* These are same as lettertab('I') and lettertab('Y') */
#define AVALUE 0
#define IVALUE 1
#define UVALUE 2
#define EVALUE 3
#define OVALUE 4
#define YBASE  100

#define smallvowel(X) (kanatab[X]-1)
/* Macro smallvowel:
 *  X is some "vowel-value" 0-4 gotten from lettertab, corresponding to
 *   some vowel A, I, U, E or O
 *  Get "relative kana-index" of small form of that vowel.
 *   (that's why -1. Small-sized variants are before ordinary-sized ones)
 */


#define smally_orvowel(Z,X)\
 ((((Z) = kanatab[YBASE+(X)]) != __) ? (--(Z)) : ((Z) = smallvowel(X)));
/* This is like smallvowel, but set Z to small form of y<vowel> if
 *  vowel is A, U or O.
 */







/* For consonants this contains indexes to kanatab to that 'row' where
    starts syllables beginning with that consonant.
   For wovels this contains also indexes to kanatab ranging from 0 to 4
 */
static BYTE _lettertab[]=
 {
   /*  0 a */          0, /* vowel */
   /*  1 b */    5,
   /*  2 c */   10,
   /*  3 d */   15,
   /*  4 e */          3, /* vowel */
   /*  5 f */   20,
   /*  6 g */   25,
   /*  7 h */   30,
   /*  8 i */          1, /* vowel */
   /*  9 j */   35,
   /* 10 k */   40,
   /* 11 l */   45,
   /* 12 m */   50,
   /* 13 n */   55,
   /* 14 o */          4, /* vowel */
   /* 15 p */   60,
   /* 16 q */   65,
   /* 17 r */   70,
   /* 18 s */   75,
   /* 19 t */   80,
   /* 20 u */          2, /* vowel */
   /* 21 v */   85,
   /* 22 w */   90,
   /* 23 x */   95,
   /* 24 y */  100,
   /* 25 z */  105
 };



static BYTE kanatab[]=
 {
    A,  I,  U,  E,  O, /* vowels */
   BA, BI, BU, BE, BO, /*  1 B*   5 */
   KA, KI, KU, KE, KO, /*  2 C*  10 */
   DA, __, __, DE, DO, /*  3 D*  15 */
   __, __, FU, __, __, /*  4 F*  20 */
   GA, GI, GU, GE, GO, /*  5 G*  25 */
   HA, HI, FU, HE, HO, /*  6 H*  30 */
   __, JI, __, __, __, /*  7 J*  35 */
   KA, KI, KU, KE, KO, /*  8 K*  40 */
   RA, RI, RU, RE, RO, /*  9 L*  45 */
   MA, MI, MU, ME, MO, /* 10 M*  50 */
   NA, NI, NU, NE, NO, /* 11 N*  55 */
   PA, PI, PU, PE, PO, /* 12 P*  60 */
   KA, KI, KU, KE, KO, /* 13 Q*  65 */
   RA, RI, RU, RE, RO, /* 14 R*  70 */
   SA,SHI, SU, SE, SO, /* 15 S*  75 */
   TA, __, __, TE, TO, /* 16 T*  80 */
   __, __, __, __, __, /* 17 V*  85 */
   WA, WI,  U, WE, WO, /* 18 W*  90 */
   __, __, __, __, __, /* 19 X*  95 */
   YA, __, YU, __, YO, /* 20 Y* 100 */
   ZA, JI, ZU, ZE, ZO  /* 21 Z* 105 */
 };

/* It is waste of time to subtract 65 (= 'A') from index at runtime,
 *  when you can subtract it from array at compile time !
 */
static BYTE *lettertab=_lettertab-'A';



/* Check if *string is vowel or letter at all,
 *  and set ptrtoc and ptrtoq to it.
 */
int isnextvowel(ptrtoc,ptrtoq,string)
int *ptrtoc,*ptrtoq;
char *string;
{
        *ptrtoc = toupper(*string);
        return(isalpha(*ptrtoc) &&
                isvowel(*ptrtoq = lettertab[*ptrtoc]));
}


/*
 In Japanese, particles e, o, and wa are written in hiragana as he, wo and ha:
 */
int handleparticles(kana)
int kana;
{
        switch(kana)
         {
           case  E: { return(HE);   }
           case  O: { return(WO);   }
           case WA: { return(HA);   }
           default: { return(kana); }
         }
}


UINT getkana(secondkana,rest,string)
UINT *secondkana;
char **rest; /* of string */
char *string;
{
        int c1,c2,c3;
        int q1,q2,q3;
        int kana;
        

        /* When this (getkana) is called, it is guaranteed that first
         *  character of string is always letter. (but only first !)
         */
        if(isnextvowel(&c1,&q1,string))
         { /* If first letter is single vowel */
           *rest = (string+SECOND);
           return(kanatab[q1]);
         }
         

        if(c1 == 'X') /* Handle X* as if it were KUS* */
         {
           *string = (isupper(*string) ? 'S' : 's');
           *rest = string;
           return(KU);
         }

        /* If first letter wasn't vowel, then it MUST (see above) be consonant.
         *  So check second character:
         */

        if(!isnextvowel(&c2,&q2,(string+SECOND)))
        /* If second character is not letter at all, or not vowel */
         {
           int is_c3vowel; /* flag */
           if(is_c3vowel = isnextvowel(&c3,&q3,(string+THIRD)))
            { *rest = (string+FOURTH); }
           else { *rest = (string+THIRD); }
           
           if((c1 == 'T') && (c2 == 'S'))
            { /* TSU, et cetera... */
              if(!is_c3vowel || (c3 == 'U')) { *secondkana = NOTHING; }
              else { *secondkana = smallvowel(q3); }
              return(TSU);
            }
           if(((c1 == 'C') || (c1 == 'S')) && (c2 == 'H'))
            { /* CHI or SHI et variants */
              if(!is_c3vowel || (c3 == 'I')) { *secondkana = NOTHING; }
              else { smally_orvowel(*secondkana,q3); }
              return(((c1 == 'C') ? CHI : SHI));
            }
           /* Liudennus: */
           if(((kana = kanatab[q1+IVALUE]) != __) && (c2 == 'Y'))
            { /* KYA, GYA, NYA, HYA, BYA, PYA, MYA, RYA et others */
              if(!is_c3vowel) { *secondkana = NOTHING; }
              else { smally_orvowel(*secondkana,q3); }
              return(kana);
            }
              
           *rest = (string+SECOND);
           if(c1 == 'N') { return(N); }
           if(c1 == 'V') { return(V); }
           /* konsonantin kahdennus, pieni tsu: */
           if(c1 == c2)  { return(tsu); }
           
           /* Otherwise, handle single consonant as it were consonant+U: */
           q2 = (((c1 == 'T') || (c1 == 'D')) ? OVALUE : UVALUE);
           if(c1 == 'H') { q2 = IVALUE; }
           else if(c1 == 'W') { q2 = AVALUE; }
           goto consplusvowel;
         }
        else /* second letter is vowel */
         {
           *rest = (string+THIRD);
consplusvowel:
           kana = kanatab[q1 + q2];
           if(kana != __) { return(kana); } /* No troubles */
           else /* It's not so easy, two kanas are needed */
            {
              *secondkana = smallvowel(q2);
              switch(c1)
               {
                 case 'D':
                  {
                    if(c2 == 'I') { kana = DE; }
                    else { kana = DO; }
                    break;
                  }
                 case 'T':
                  {
                    if(c2 == 'I') { kana = TE; }
                    else { kana = TO; }
                    break;
                  }
                 case 'F': { kana = FU; break; }
                 case 'J':
                  {
                    kana = JI;
                    smally_orvowel(*secondkana,q2);
                    break;
                  }
                 case 'V': { kana = V; break; }
                 case 'Y': { kana = YU; break; }
                 default:
                  {
                    fprintf(stderr,"\nInternal error in getkana !\n");
                    fprintf(stderr,"string: %d/%s\n*rest: %d/%s\n",
                             strlen(string),string,strlen(*rest),rest);
                    fprintf(stderr,"c1,c2,c3: <%c,%c,%c> (%xh %xh %xh)\n",
                                     c1,c2,c3,c1,c2,c3);
                    fprintf(stderr,
                     "q1,q2,q3: (%d %d %d)  kana: %d  *secondkana: %d\n",
                       q1,q2,q3,kana,*secondkana);
                    *rest = (string+SECOND);
                    *secondkana = NOTHING;
                    return(A);
                  }
               }
              return(kana);
            }
         }
}


UINT get_next_kana_as_ucs(secondkana,rest,string)
UINT *secondkana;
char **rest;
char *string;
{
        int kkflag; /* If non-zero then use KataKana, otherwise hiragana */
        int result;

        kkflag = 0;

        if(conv_flag == TO_KATA) { kkflag = 1; }
        else if(conv_flag == TO_BOTH)
         {
           if(*(string+1) && isalpha(*(string+1))) /* If second char. is letter and it's */
            { if(isupper(*(string+1))) { kkflag = 1; } } /* uppercase */
           /* Also if just one letter, and it's uppercase: */
           else if(isupper(*string)) { kkflag = 1; }
         }
        
        *secondkana = NOTHING;
 
        /* If conv_flag is TO_ROMAJI or first of string is not letter,
            then convert it directly to corresponding Unicode code: */
        if((conv_flag == TO_ROMAJI) || !isalpha(*string))
         {
           *rest = (string+1);
           *secondkana = 0;
           return(*string); /* atoj(*string)); */
         }
        else /* It is letter(s) which should be converted to kana */
         {
           result = getkana(secondkana,rest,string);
           /* If in hiragana-mode and this kana is not part of any word,
               then it may be some particle: */
           if(part_flag && !kkflag
               && !isalpha(*(string-1)) && !isalpha(**rest))
            {
              result = handleparticles(result);
            }
            
           if(*secondkana == NOTHING) { *secondkana = 0; }
           else { *secondkana = choosekana(*secondkana,kkflag); }
           
           if(result == V) { kkflag = 1; } /* V is only in katakana */
           return(choosekana(result,kkflag));
         }
}

#ifdef OLD_RELIC
/*
   atojk - main conversion routine.

   vec is destination pointer to one-dimensional array of integers, where
    resulting JIS-codes are stored.
   str is source string of ascii stuff.
   Note that str-1 should always contain blanko or some other non-letter
    character.
 */
UINT atojk(vec,str) /* Ascii to Jis & Kana */
UINT *vec;
char *str;
{
        UINT vec_start=vec;
        
        while(*str)
         {
           *vec = get_next_kana_as_ucs((vec+1),&str,str);
           if(*++vec) { vec++; } /* If got two kana-syllabics */
         }
         
        *vec = 0;

        return(vec - vec_start);
}
#endif

int output_romaji_as_kana(char *src_str, FILE *out, int use_html_entities)
{
        UINT res_space[3], *res = res_space;
        
        while(*src_str)
         {
           res = res_space;
           *res = get_next_kana_as_ucs((res+1),&src_str,src_str);
           if(use_html_entities) { fprintf(out,"&#%u;", *res); }
           else { output_ucs_as_utf8(out, *res); }
           if(*++res)    /* Got the second kana-syllabic also? */
            {
              if(use_html_entities) { fprintf(out,"&#%u;", *res); }
              else { output_ucs_as_utf8(out, *res); }
            }
         }
}

/* ====================================================================== */


#define K_ZERO                0x96F6  /*  0 */

#define K_ONE                 0x4E00  /*  1 */
#define K_TWO                 0x4E8C  /*  2 */
#define K_THREE               0x4E09  /*  3 */
#define K_FOUR                0x56DB  /*  4 */
#define K_FIVE                0x4E94  /*  5 */
#define K_SIX                 0x516D  /*  6 */
#define K_SEVEN               0x4E03  /*  7 */
#define K_EIGHT               0x516B  /*  8 */
#define K_NINE                0x4E5D  /*  9 */
#define K_TEN                 0x5341  /* 10 */

#define K_TWENTY              0x5EFF  /* 20 */ /* 0x5EFE is another version */

#define K_HUNDRED             0x767E  /*              100 = 10^2 */
#define K_THOUSAND            0x5343  /*            1,000 = 10^3 */
#define K_TEN_THOUSAND        0x4E07  /*           10,000 = 10^4 */
#define K_HUNDRED_MILLION     0x5104  /*      100,000,000 = 10^8 */
#define K_TRILLION            0x5146  /* 1000,000,000,000 = 10^12 (biljoona)*/
 
#define K_HUNDRED_QUINTILLION 0x5793  /* That's too much */
 
 
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
   (It's another thing on Alpha...)
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
        ULI lower_or_same();

        int k;
        ULI y,factor,remainder;
        TOB result;

        if(20 == x) { l = list1(int_tob(K_TWENTY)); }

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
           /* Shouldn't put zero to the end (that's superfluous): */
           if(remainder != 0)
            { result = nconc(result,ltok(remainder)); }

           return(result);
         }
}

/* ====================================================================== */

#define toUstr(X) ((BYTE*) (X))
#define toStr(X) ((char *) (X))

BYTE cgi_option=0; /* If used via the Web. */
BYTE entities_option=0;

BYTE *parse_url_value(BYTE *url_piece)
{
    BYTE *s,*t;
    BYTE save_char;
    unsigned int tmp;

    s = t = url_piece;

    while(*s)
     {
       switch(*s)
        {
          case '+': { *t++ = ' '; s++; break; } /* Plus signs to spaces */
          case '%':
           {
             if(strlen((char *)s) >= 3) /* There are at least two chars more */
              {
                save_char = *(s+3);
                *(s+3) = '\0';
                /* Convert two hex digits to int */
                sscanf(((const char *)s+1),"%x",&tmp);
                *(s+3) = save_char;
                *t++ = tmp;
                s += 3;
              }
             else { *t++ = *s++; } /* Copy the percent sign literally. */
             break;
           }
          default:  { *t++ = *s++; break; }
        }
     }

    *t = '\0';
    return(url_piece);
}


int parse_url_query_string(BYTE **src_string, BYTE *query_string)
{
    BYTE *ptr1,*ptr2,*ptr3;
    unsigned int mask=0,mask2=0;

    *src_string = NULL;
    ptr1 = query_string;
    while(ptr1)
     {
       if(!(ptr2 = toUstr(strchr(((char *)ptr1),'=')))) { break; }
       if((ptr3 = toUstr(strchr(((char *)ptr2),'&'))))
        { *ptr3++ = '\0'; } /* Overwrite the ampersand, and skip past. */
       *ptr2++ = '\0'; /* Overwrite the equal sign, and skip past. */
/* ptr1 points to the beginning of the variable name.
   ptr2 points two characters past the end of the variable name, i.e. one
    past the equal sign (=) which has been overwritten by zero '\0',
    that is to the beginning of the variable value, corresponding to
    variable name where ptr1 points to.
   ptr3 points two characters past the end of variable value, i.e. one
    past the ampersand (&) which has been overwritten by zero, that is
    to the beginning of the next variable name. Or if we have found the
    last name=value pair, then it contains the NULL.
 */
       if(!strcasecmp(((char *)ptr1),"src"))
        {
          if(*ptr2)
           { *src_string = parse_url_value(toUstr(ptr2)); }
        }
       else if(!strcasecmp(((char *)ptr1),"options"))
        {
          BYTE *s;
          for(s=parse_url_value(toUstr(ptr2)); *s; s++)
           {
             switch(tolower(*s))
              {
                case 'e': { entities_option = 1; break; }
                default: { break; }
              }
           }
        }
       ptr1 = ptr3;
     }

    return(NULL != *src_string);
}


void main(UINT argc,BYTE **argv)
{
    BYTE *src_str=NULL;
    BYTE *s;

    if((s = toUstr(getenv("QUERY_STRING")))
        && parse_url_query_string(&src_str,s))
     {
       cgi_option = 1;
       printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");

/* Note that parse_url_query_string puts to src_str the pointer to
   a substring of s, returned by getenv. If I remember right, getenv
   returns pointers to static string buffer, so if you call getenv second
   time, its result will overwrite the old s, and probably also the src_str
 */

       printf("<HTML><HEAD><TITLE>\"%s\" in kana</TITLE></HEAD><BODY>\n",
              src_str);

       output_romaji_as_kana(src_str, stdout, entities_option);

       printf("</BODY></HTML>\n");
       fflush(stdout);
     }
    else
     {
       printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");

       printf("<HTML><HEAD><TITLE>Please give something!</TITLE></HEAD><BODY>");
       printf("<FORM ACTION=\"kanaize.cgi\" METHOD=GET>\n"
              "<H4>Source String</H4>\n"
              "<INPUT TYPE=TEXT NAME=SRC>\n"
              "<BR>\n"
              "<INPUT TYPE=RADIO NAME=OPTIONS VALUE=\"-e\"> Use numeric HTML entities\n"
              "</FORM>\n"
              "</BODY></HTML>\n");
       fflush(stdout);
     }
}



/*
   KANA.C - Routines to convert romaji to hira/katakana.
   Coded by Antti Karttunen at 198? and 1990.
 */


#include "stdio.h"
#include "ctype.h"
#include "mydefs.h"
#include "kanjifun.h"
#include "kana.h"


#define TO_ROMAJI 0
#define TO_HIRA   1
#define TO_KATA   2
#define TO_BOTH   3
/* According to case: lowercase -> hiragana, uppercase -> katakana */

BYTE conv_flag = TO_ROMAJI;
BYTE part_flag = 0; /* If this is 1 then particles are converted */



#define jis_code(C1,C2) (((94 * (C1)) + (C2)) - 3135) /* 94*(C1-33)+(C2-33) */

#define __       255 /* Exception or Error (not possible) */

/* These are indexes to font file KANJI24.JIS which is not used anymore: */
#define HIRAGANA 170
#define KATAKANA 253
#define H_TO_K   83
 */

#define HIRAGANA jis_code('$','!')
#define KATAKANA jis_code('%','!')
#define H_TO_K   (KATAKANA - HIRAGANA)

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




 
/*
   atojk - main conversion routine.

   vec is destination pointer to one-dimensional array of integers, where
    resulting JIS-codes are stored.
   str is source string of ascii stuff.
   Note that str-1 should always contain blanko or some other non-letter
    character.
 */
UINT atojk(vec,str) /* Ascii to Jis & Kana */
register UINT *vec;
register char *str;
{
        UINT getjis();

        UINT vec_start=vec;
        
        while(*str)
         {
           *vec = getjis((vec+1),&str,str);
           if(*++vec) { vec++; } /* If got two kana-syllabics */
         }
         
        *vec = 0;

        return(vec - vec_start);
}



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


char *getdigits(z,string)
char *z;
char *string;
{
        while(isdigit(*string)) { *z++ = *string++; }
        *z = '\0';
        return(string);
}




UINT getjis(secondkana,rest,string)
UINT *secondkana;
char **rest;
char *string;
{
        int getkana();
        int kkflag; /* If non-zero then use KataKana, otherwise hiragana */
        int result;

        kkflag = 0;

        if(conv_flag == TO_KATA) { kkflag = 1; }
        else if(conv_flag == TO_BOTH)
         {
           if(isletter(*(string+1)) /* If second char. is letter and it's */
            { if(isupper(*(string+1))) { kkflag = 1; } } /* uppercase */
           /* Also if just one letter, and it's uppercase: */
           else if(isupper(*string)) { kkflag = 1; }
         }
        
        *secondkana = NOTHING;

/* This kind of bullshit is not supported anymore:        
        (* If there is "escape-sequence" @\num then return as result num: *)
        if((*string == '@') && (*(string+1) == '\\'))
         {
           char *getdigits();
           char z[11];
           *rest = getdigits(z,(string+2));
           if(*z) { *secondkana = 0; return(atoi(z)); }
           (* However, if there is no digits after @\ then just return @ *)
           (* (And \ next time when called) */
         }
 */

 
        /* If conv_flag is TO_ROMAJI or first of string is not letter,
            then convert it directly to corresponding JIS-code: */
        if((conv_flag == TO_ROMAJI) || !isalpha(*string))
         {
           *rest = (string+1);
           *secondkana = 0;
           return(atoj(*string));
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


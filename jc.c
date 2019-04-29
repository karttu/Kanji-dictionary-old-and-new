/*
   Unix version.
 */

#include "stdio.h"

char *getenv();

#define JIS     0
#define SHIFT   1
#define EUC     2

#define JIN     'B' /* B for new JIS, @ for old. */
#define JOUT    'J'
#define ESC     '\033'

#define kanjiIn()  ((mode == JIS) ? fprintf(output,"\033$%c",jin) : 0)
#define kanjiOut()  ((mode == JIS) ? fprintf(output,"\033(%c",jout) : 0)

#define KT_JIN  0x01
#define KT_JOUT 0x02
#define KT_ESC  0x04
#define KT_JIS  0x08
#define KT_SFT1 0x10
#define KT_SFT2 0x20
#define KT_EUC1 0x40
#define KT_EUC2 0x80

/* FILE *output=NULL; */
#define output stdout

int mode  = SHIFT; /* Default output mode, originally JIS, now SHIFT */
char jin  = JIN;
char jout = JOUT;
/* This is moved here from jc, now global variable: */
int  k_default = KT_EUC1;
 


int ktype[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*00*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*10*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, /*  !"#$%&' */
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, /* ()*+,-./ */
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, /* 01234567 */
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, /* 89:;<=>? */
    0x29, 0x28, 0x2b, 0x28, 0x28, 0x28, 0x28, 0x28, /* @ABCDEFG */
    0x2a, 0x28, 0x2a, 0x28, 0x28, 0x28, 0x28, 0x28, /* HIJKLMNO */
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, /* PQRSTUVW */
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, /* XYZ[\]^_ */
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, /* `abcdefg */
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, /* hijklmno */
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, /* pqrstuvw */
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x00, /* xyz{\}~  */
    0x20, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, /*80*/
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, /*90*/
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x20, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, /*A0*/
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, /*B0*/
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, /*C0*/
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, /*D0*/
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, /*E0*/
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, /*F0*/
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xc0, 0xc0, 0x00,
};


main(argc,argv)
int argc;
char **argv;
{
    FILE *fp;
    char *buf, *getenv();

    /* Bug fix for Turbo-C: "reopen" stdout in binary mode,
        so that CR+LF's won't get converted to CR+CR+LF:
       But this doesn't work, so commented out, compile with Aztec-C !
    if(!(output = fdopen(fileno(stdout),"wb")))
     {
       fprintf(stderr,"\njc: Cannot reopen output in binary mode !\n");
       exit(1);
     }
     */

    if (buf = getenv("JIN"))  jin = *buf;
    if (buf = getenv("JOUT")) jout = *buf;

    while(*++argv)
     if((**argv == '-') || (**argv == '+'))
      {
        switch(*(*argv+1))
         {
           case 'j': case 'J':
               mode = JIS;
               if (*(*argv+2))
                {
                  jin = *(*argv+2);
                  if (*(*argv+3)) jout = *(*argv+3);
                }
               break;
           case 's': case 'S':
               if(**argv == '-') { mode = SHIFT; }
               else { k_default = KT_SFT1; }
               break;
           case 'e': case 'E': case 'u': case 'U':
               if(**argv == '-') { mode = EUC; }
               else { k_default = KT_EUC1; }
               break;
           default:
               fprintf(stderr, "jc: Unknown switch: %c.\n",*(*argv+1));
               fprintf(stderr, "Usage: jc [-{j[@J]se}] [+{se}] [file ...]\n");
               exit(1);
         }
        argc--;
      }
     else
      {
        if(fp = fopen(*argv, "r")) /* "rb" is not good mode in Aztec C */
         {
           jc(fp);
           fclose(fp);
         }
        else perror(*argv);
      }

    if(argc == 1) jc(stdin);
}     

jc(fp)
FILE *fp;
{
    char ch;
    int flag;
    register unsigned int ch1, ch2=0;
    int kanji = 0;

    flag = 0;

    while ((ch = getc(fp)) != EOF)
     {
        ch1 = (unsigned char) ch;
        if (ch == '\033')
        {
            flag = KT_ESC;
            continue;
        }
        if (flag & KT_ESC)
        {
            if (ch == '$')
                flag = KT_JIN;
            else if (ch == '(')
                flag = KT_JOUT;
            else
            {
                flag = 0;
                putc('\033',output);
                putc(ch,output);
            }
            continue;
        }
        if (flag & KT_JIN)
        {
            if (ktype[ch1] & KT_JIN)
            {
                kanji = KT_JIN | KT_JOUT;
                kanjiIn();
                flag = 0;
                continue;
            }
        }
        else if (flag & KT_JOUT)
        {
            if (ktype[ch1] & KT_JOUT)
            {
                kanji = 0;
                kanjiOut();
                flag = 0;
                continue;
            }
        } 
        if (flag & KT_SFT1)
        {
            flag = 0;
            if (ktype[ch1] & KT_SFT2)
            {
                ch2 -= (ch2 <= 0x9f) ? 0x71 : 0xb1;
                ch2 = ch2 * 2 + 1;
                if (ch1 > 0x7f)
                    ch1 -= 1;
                if (ch1 >= 0x9e)
                {
                    ch1 -= 0x7d;
                    ch2++;
                }
                else
                    ch1 -= 0x1f;

                if (!(kanji & KT_JOUT))
                {
                    kanjiIn();
                    kanji |= KT_JOUT;
                }
                putKanji(ch2, ch1);
                k_default = KT_SFT1;
                continue;
            }
        }
        if (flag & KT_EUC1)
        {
            flag = 0;
            if (ktype[ch1] & KT_EUC2)
            {
                if (!(kanji & KT_JOUT))
                {
                    kanjiIn();
                    kanji |= KT_JOUT;
                }
                putKanji(ch2 & 0x7f, ch1 & 0x7f);
                k_default = KT_EUC1;
                continue;
            }
        } 
        if (ktype[ch1] & KT_JIS)
        {
            if (kanji & KT_JIN)
            {
                if (flag & KT_JIS)
                {
                    putKanji(ch2, ch1);
                    flag = 0;
                }
                else
                {
                    ch2 = ch1;
                    flag = KT_JIS;
                }
            }
            else
            {
                if (kanji & KT_JOUT)
                {
                    kanjiOut();
                    kanji &= ~ KT_JOUT;
                }
                putc(ch,output);
            }
            continue;
        }
        if (ktype[ch1] & KT_EUC1)
        {
            if (k_default == KT_EUC1 || !(ktype[ch1] & KT_SFT1))
            {
                flag = KT_EUC1;
                ch2 = ch1;
                continue;
            }
        }
        if (ktype[ch1] & KT_SFT1)
        {
            flag = KT_SFT1;
            ch2 = ch1;
            continue;
        }
        if (ch == '\n' && mode == JIS && kanji & KT_JOUT)
        {
            kanjiOut();
            putc('\n',output);
            kanjiIn();
        }
        else
        {
            if (kanji)
            {
                kanjiOut();
                kanji = 0;
            }
            putc(ch,output);
        }
     } /* while */

    if (kanji) kanjiOut();
}


putKanji(hi, lo)
unsigned int hi, lo;
{
    switch (mode)
     {
       case EUC:
           putc((hi | 0x80),output);
           putc((lo | 0x80),output);
           break;
       case SHIFT:    
           if (hi & 1)
               lo += 0x1f;
           else
               lo += 0x7d;
           if (lo >= 0x7f)
               lo++;
           hi = (hi - 0x21 >> 1) + 0x81;
           if (hi > 0x9f)
               hi += 0x40;
           /* THROUGH DOWN */
       case JIS:
           putc(hi,output);
           putc(lo,output);
           break;
     }
}


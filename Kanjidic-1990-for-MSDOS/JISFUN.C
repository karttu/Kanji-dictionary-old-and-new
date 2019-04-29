

/* Functions to convert various JIS codes. By A.K. at 19. MARCH 1990 */


#include "stdio.h"
#include "ctype.h"
#include "mydefs.h"


/* Default invalid_kanji sign for those cases when code is out of range:
   Three dots, like these:         .
                                  . .
 */
static UINT _invalid_kanji = 71;

/* How many codes out of range has been encountered ? */
static UINT jis_errcnt=0;

/*
   Use functions set_invalid_kanji, get_invalid_kanji, set_jis_errcnt
    & get_jis_errcnt to change and access these variables.
 */


UINT set_invalid_kanji(x)
int x;
{
        UINT y;
        y = _invalid_kanji;
        _invalid_kanji = x;
        return(y);
}

UINT get_invalid_kanji()
{ return(_invalid_kanji); }


UINT set_jis_errcnt(x)
int x;
{
        UINT y;
        y = jis_errcnt;
        jis_errcnt = x;
        return(y);
}


UINT get_jis_errcnt()
{ return(jis_errcnt); }


/* Function to convert normal jis font index to the index into font file
    K16JIS1.FNT (Another strange order in this file... ?)
   This was modified from function jis2index in module KJIS.C of KD
   kanjidriver by Izumi Ohzawa & Koichi Mori, which was in turn ported from
   JIS.PAS module of READER program by Seiichi Nomura & Seke Wei.
   This kind of function (named Font_Index) is also in SJIS.C module of
    FK program by Mark Edwards, translated also from JIS.PAS.
   (So you got picture of this genealogical tree, eh... ?)
 */
UINT index_to_k16index(i)
UINT i;
{
    /*...........................range.....notation..JIS code.....index... */

    if(i<= 107)               /* 0..93               $2121-$217e, 0  -93   */
     {                        /* 94..107             $2221-$222e, 94 -107  */
 /* Do nothing, because punctuation marks are in right place at font file. */
     }
    else if(i>=203 && i<=212)
     i -= 56;                 /* 203..212   0 .. 9,  $2330-$2339, 147-156  */
    else if(i>=220 && i<=245)
     i -= 63;                 /* 220..245   A .. Z,  $2341-$235a, 157-182  */
    else if(i>=252 && i<=277)
     i -= 69;                 /* 252..277   a .. z,  $2361-$237a, 183-208  */
    else if(i>=282 && i <=364)
     i -= 73;                 /* 282..364  hiragana, $2421-$2473, 209-291  */
    else if(i>=376 && i<=461)
     i -= 84;                 /* 376..461  katakana, $2521-$2576, 292-377  */
    else if(i>=470 && i<=493)
     i -= 92;                 /* 470..493   GREEK,   $2621-$2638  378-401  */
    else if(i>=502 && i<=525)
     i -= 100;                /* 502..525   greek,   $2641-$2658  402-425  */
    else if(i>=564 && i<=596)
     i -= 138;                /* 564..596   RUSSIAN, $2721-$2741  426-458  */
    else if(i>=612 && i<=644)
     i -= 153;                /* 612..644   russian, $2751-$2771  459-491  */

                              /*  n/a       linedraw,  n/a        492-523  */

    else if(i>=1410 && i<=4374)
     i -= 886;                /* 1410..4374 kanji-1, $3021-$4f53, 524-3488 */
    else if(i>=4418 && i<=7805) /* Do nothing for level 2 kanjis.          */
     { }                      /* 4418..7805 kanji-2, $5021-$737e, 0000-3387 */
    else
     {
       i = _invalid_kanji;   /* non printable */
       jis_errcnt++;
     }

    return(i);
}



/* Shift-Jis to Index */
/* I'm not sure whether this works correctly with JIS 2 level kanjis.
   (Haven't read enough documentation about that...) And JIS 3 level
   in Shift-JIS code (is there something like that ?) is completely
   mysterious for me.
 */
UINT sj_to_index(sj_code)
UINT sj_code;
{
        register UINT hi,lo;

        hi = gethighbyte(sj_code);
        lo = getlowbyte(sj_code);

        if(((hi < 0x81) || (hi > 0xFC)) ||
            ((lo < 0x40) || (lo > 0xFC) || (lo == 0x7F)))
         { /* If characters out of range. */
           jis_errcnt++;
           return(_invalid_kanji);
         }

/* There is never DEL in Shift-JIS code, so decrement values above it: */
        if(lo > 0x7F) { lo--; }

        hi -= 0x81;
        lo -= 0x40;

        return((hi * 188) + lo);
}


/* Index to Shift-JIS: */
UINT index_to_sj(index)
UINT index;
{
        register UINT hi,lo;
/* Count for times been in invalid kanji code, to ensure that program doesn't
    go to idiot loop: */
        int cnt=0;

loop:
        hi = (index / 188);
        lo = (index % 188);

        hi += 0x81;
        lo += 0x40;

        if(lo > 0x7E) { lo++; }


        if(((hi < 0x81) || (hi > 0xFC)) ||
            ((lo < 0x40) || (lo > 0xFC) /* || (lo == 0x7F) */))
         { /* If invalid Shift-JIS code was made, then return _invalid_kanji
               converted to Shift-JIS code. However, if user has changed
               _invalid_kanji to index which is itself invalid, then
               return default invalid kanji when second time to come here:
            */
           if(cnt) { return(0x8188); } /* Shift-JIS code of 71 (three dots) */
           cnt++;
           jis_errcnt++;
           index = _invalid_kanji;
           goto loop;
         }

        return((hi << 8) + lo);
}



/* JIS code to index: */
/* This is also from function jis2index from <- KJIS.C <- JIS.PAS */
UINT jis_to_index(jis_code)
UINT jis_code;
{
        register UINT hi,lo;
        hi = gethighbyte(jis_code);
        lo = getlowbyte(jis_code);

	if((lo > 0x20) && (lo < 0x7F) && (hi > 0x20) && (hi < 0x74))
         { hi = ((94 * hi) + lo - 3135); } /* hi=94*(hi-33)+(lo-33) */
	else /* code out of range */
         {
           jis_errcnt++;
           hi = _invalid_kanji;
         }

        return(hi);
}


/* Index to JIS code: */
UINT index_to_jis(index)
UINT index;
{
        register UINT hi,lo;

        if(index > 8835) /*   94 * 94 = 8836   */
         {
           jis_errcnt++;
           index = _invalid_kanji;
         }

        hi  = (index / 94);
        lo  = (index % 94);

        hi  += 0x21;
        lo  += 0x21;

        return((hi << 8) + lo);
}


/* These could be macros as well, but I don't care to write n+1 header files.
   Note that 188 = 2 * 94. Maybe that fact could be used to invent some
   speedier conversing algorithm ? At least in direction JIS -> Shift JIS,
   there wouldn't be any need to multiplicate and divide, just subtracting,
   shifting and adding, I think. I'll write it one day.
 */
UINT sj_to_jis(code)
UINT code;
{
        return(index_to_jis(sj_to_index(code)));
}


UINT jis_to_sj(code)
UINT code;
{
        return(index_to_sj(jis_to_index(code)));
}





#include "ctype.h"
#include "stdio.h"
#include "math.h" /* For sqrt() */

#define progname "getslice"


/* static */ unsigned char _revbyte_table[256] =
 {
/* 00 */  0x00,
/* 01 */  0x80,
/* 02 */  0x40,
/* 03 */  0xC0,
/* 04 */  0x20,
/* 05 */  0xA0,
/* 06 */  0x60,
/* 07 */  0xE0,
/* 08 */  0x10,
/* 09 */  0x90,
/* 0A */  0x50,
/* 0B */  0xD0,
/* 0C */  0x30,
/* 0D */  0xB0,
/* 0E */  0x70,
/* 0F */  0xF0,
/* 10 */  0x08,
/* 11 */  0x88,
/* 12 */  0x48,
/* 13 */  0xC8,
/* 14 */  0x28,
/* 15 */  0xA8,
/* 16 */  0x68,
/* 17 */  0xE8,
/* 18 */  0x18,
/* 19 */  0x98,
/* 1A */  0x58,
/* 1B */  0xD8,
/* 1C */  0x38,
/* 1D */  0xB8,
/* 1E */  0x78,
/* 1F */  0xF8,
/* 20 */  0x04,
/* 21 */  0x84,
/* 22 */  0x44,
/* 23 */  0xC4,
/* 24 */  0x24,
/* 25 */  0xA4,
/* 26 */  0x64,
/* 27 */  0xE4,
/* 28 */  0x14,
/* 29 */  0x94,
/* 2A */  0x54,
/* 2B */  0xD4,
/* 2C */  0x34,
/* 2D */  0xB4,
/* 2E */  0x74,
/* 2F */  0xF4,
/* 30 */  0x0C,
/* 31 */  0x8C,
/* 32 */  0x4C,
/* 33 */  0xCC,
/* 34 */  0x2C,
/* 35 */  0xAC,
/* 36 */  0x6C,
/* 37 */  0xEC,
/* 38 */  0x1C,
/* 39 */  0x9C,
/* 3A */  0x5C,
/* 3B */  0xDC,
/* 3C */  0x3C,
/* 3D */  0xBC,
/* 3E */  0x7C,
/* 3F */  0xFC,
/* 40 */  0x02,
/* 41 */  0x82,
/* 42 */  0x42,
/* 43 */  0xC2,
/* 44 */  0x22,
/* 45 */  0xA2,
/* 46 */  0x62,
/* 47 */  0xE2,
/* 48 */  0x12,
/* 49 */  0x92,
/* 4A */  0x52,
/* 4B */  0xD2,
/* 4C */  0x32,
/* 4D */  0xB2,
/* 4E */  0x72,
/* 4F */  0xF2,
/* 50 */  0x0A,
/* 51 */  0x8A,
/* 52 */  0x4A,
/* 53 */  0xCA,
/* 54 */  0x2A,
/* 55 */  0xAA,
/* 56 */  0x6A,
/* 57 */  0xEA,
/* 58 */  0x1A,
/* 59 */  0x9A,
/* 5A */  0x5A,
/* 5B */  0xDA,
/* 5C */  0x3A,
/* 5D */  0xBA,
/* 5E */  0x7A,
/* 5F */  0xFA,
/* 60 */  0x06,
/* 61 */  0x86,
/* 62 */  0x46,
/* 63 */  0xC6,
/* 64 */  0x26,
/* 65 */  0xA6,
/* 66 */  0x66,
/* 67 */  0xE6,
/* 68 */  0x16,
/* 69 */  0x96,
/* 6A */  0x56,
/* 6B */  0xD6,
/* 6C */  0x36,
/* 6D */  0xB6,
/* 6E */  0x76,
/* 6F */  0xF6,
/* 70 */  0x0E,
/* 71 */  0x8E,
/* 72 */  0x4E,
/* 73 */  0xCE,
/* 74 */  0x2E,
/* 75 */  0xAE,
/* 76 */  0x6E,
/* 77 */  0xEE,
/* 78 */  0x1E,
/* 79 */  0x9E,
/* 7A */  0x5E,
/* 7B */  0xDE,
/* 7C */  0x3E,
/* 7D */  0xBE,
/* 7E */  0x7E,
/* 7F */  0xFE,
/* 80 */  0x01,
/* 81 */  0x81,
/* 82 */  0x41,
/* 83 */  0xC1,
/* 84 */  0x21,
/* 85 */  0xA1,
/* 86 */  0x61,
/* 87 */  0xE1,
/* 88 */  0x11,
/* 89 */  0x91,
/* 8A */  0x51,
/* 8B */  0xD1,
/* 8C */  0x31,
/* 8D */  0xB1,
/* 8E */  0x71,
/* 8F */  0xF1,
/* 90 */  0x09,
/* 91 */  0x89,
/* 92 */  0x49,
/* 93 */  0xC9,
/* 94 */  0x29,
/* 95 */  0xA9,
/* 96 */  0x69,
/* 97 */  0xE9,
/* 98 */  0x19,
/* 99 */  0x99,
/* 9A */  0x59,
/* 9B */  0xD9,
/* 9C */  0x39,
/* 9D */  0xB9,
/* 9E */  0x79,
/* 9F */  0xF9,
/* A0 */  0x05,
/* A1 */  0x85,
/* A2 */  0x45,
/* A3 */  0xC5,
/* A4 */  0x25,
/* A5 */  0xA5,
/* A6 */  0x65,
/* A7 */  0xE5,
/* A8 */  0x15,
/* A9 */  0x95,
/* AA */  0x55,
/* AB */  0xD5,
/* AC */  0x35,
/* AD */  0xB5,
/* AE */  0x75,
/* AF */  0xF5,
/* B0 */  0x0D,
/* B1 */  0x8D,
/* B2 */  0x4D,
/* B3 */  0xCD,
/* B4 */  0x2D,
/* B5 */  0xAD,
/* B6 */  0x6D,
/* B7 */  0xED,
/* B8 */  0x1D,
/* B9 */  0x9D,
/* BA */  0x5D,
/* BB */  0xDD,
/* BC */  0x3D,
/* BD */  0xBD,
/* BE */  0x7D,
/* BF */  0xFD,
/* C0 */  0x03,
/* C1 */  0x83,
/* C2 */  0x43,
/* C3 */  0xC3,
/* C4 */  0x23,
/* C5 */  0xA3,
/* C6 */  0x63,
/* C7 */  0xE3,
/* C8 */  0x13,
/* C9 */  0x93,
/* CA */  0x53,
/* CB */  0xD3,
/* CC */  0x33,
/* CD */  0xB3,
/* CE */  0x73,
/* CF */  0xF3,
/* D0 */  0x0B,
/* D1 */  0x8B,
/* D2 */  0x4B,
/* D3 */  0xCB,
/* D4 */  0x2B,
/* D5 */  0xAB,
/* D6 */  0x6B,
/* D7 */  0xEB,
/* D8 */  0x1B,
/* D9 */  0x9B,
/* DA */  0x5B,
/* DB */  0xDB,
/* DC */  0x3B,
/* DD */  0xBB,
/* DE */  0x7B,
/* DF */  0xFB,
/* E0 */  0x07,
/* E1 */  0x87,
/* E2 */  0x47,
/* E3 */  0xC7,
/* E4 */  0x27,
/* E5 */  0xA7,
/* E6 */  0x67,
/* E7 */  0xE7,
/* E8 */  0x17,
/* E9 */  0x97,
/* EA */  0x57,
/* EB */  0xD7,
/* EC */  0x37,
/* ED */  0xB7,
/* EE */  0x77,
/* EF */  0xF7,
/* F0 */  0x0F,
/* F1 */  0x8F,
/* F2 */  0x4F,
/* F3 */  0xCF,
/* F4 */  0x2F,
/* F5 */  0xAF,
/* F6 */  0x6F,
/* F7 */  0xEF,
/* F8 */  0x1F,
/* F9 */  0x9F,
/* FA */  0x5F,
/* FB */  0xDF,
/* FC */  0x3F,
/* FD */  0xBF,
/* FE */  0x7F,
/* FF */  0xFF
 };


#define reverse_byytti(byytti) (_revbyte_table[(byytti)&0xFF])

char Complementing_Flag=0,Debugging_Flag=0,PBM_Flag=0,Reverse_Flag=0;
char Swap_Flag=0;

char bufferi[BUFSIZ+2];

main(argc,argv)
int argc;
char **argv;
{
    unsigned long int atol();
    unsigned long int offset,length,factor=1,result;
    size_t bytes_read,bytes_written,chunk;
    char c,*s;
    char **anchor;

    if(argc < 3)
     {
       fprintf(stderr,
"Usage: %s Offset Length [xFactor] [C|D|P|R|S] < inputfile > outputfile\n",
           progname);
       fprintf(stderr,
"where Offset, Length and Factor can be specified in:\n");
       fprintf(stderr,
"   Octal        (if the number begins with 0, e.g. 0377777)\n");
       fprintf(stderr,
"   Hexadecimal  (if the number begins with $ or 0x, e.g. 0x003BEEF7)\n");
       fprintf(stderr,
"   Decimal      (if the number begins with digits 1-9 or sign - or +)\n");
       fprintf(stderr,
"If Factor is given (e.g. x72 or *512) then Offset is multiplied by it.\n");
       fprintf(stderr,
"Use the option C for complementing the bytes read,\n");
       fprintf(stderr,
"R for reversing them, and S for swapping the adjacent bytes.\n");
       fprintf(stderr,
"And option P to output the PBM header (format P4) before the actual data.\n");
       exit(1);
     }

    anchor = argv;
    while(s = *++argv)
     {
       result = 0;
luuppi_heijaa:
       switch(c = *s)
	{
	  case '\0': { break; }
          /* There's a factor following an asterisk or minuscule x: */
          case '*': case 'x': { s++; goto luuppi_heijaa; }
	  case '0':
	   {
             if((*(s+1) == 'x') || (*(s+1) == 'X'))
	      { s++; /* And fall through */ }
             else
	      {
                sscanf(s,"%lo",&result);
                break;
	      }
           }
	  case '$':
	   {
             sscanf((s+1),"%lx",&result);
             break;
	   }
	  case 'c':
	  case 'C':
	   { Complementing_Flag = 1; s++; goto luuppi_heijaa; }
	  case 'd':
	  case 'D':
	   { Debugging_Flag = 1; s++; goto luuppi_heijaa; }
	  case 'p':
	  case 'P':
	   { PBM_Flag = 1;  s++; goto luuppi_heijaa; }
          case 'r':
          case 'R':
	   { Reverse_Flag = 1; s++; goto luuppi_heijaa; }
          case 's':
          case 'S':
	   { Swap_Flag = 1; s++; goto luuppi_heijaa; }
          case '-':
	   { /* If there is next character and it's not digit then we
                 assume that this is a beginning dash of an option:
              */
             if(*(s+1) && !isdigit(*(s+1))) { s++; goto luuppi_heijaa; }
/* Else fall through as we assume that this is a minus sign of the decimal
    number: */
           }
	  default:
	   {
             sscanf(s,"%ld",&result);
             break;
           }
	}

       if((argv-anchor) == 1) { offset = result; }
       else if((argv-anchor) == 2)
        {
          if(!(length = result))
	   {
             fprintf(stderr,
"%s: please specify a non-zero length for the slice to be cut!\n",progname);
             exit(1);
           }
        }
       else if((**argv == '*') || (**argv == 'x'))
	{
          if(!(factor = result))
	   {
             fprintf(stderr,
"%s: please specify a non-zero factor for the length of slice to be cut!\n",
                      progname);
             exit(1);
           }
	}
       else
	{
/* Do nothing. */
        }
     }

    offset *= factor;
    
    if(Debugging_Flag)
     {
       fprintf(stderr,
"%s: offset = %lu./0%lo/0x%lx   length = %lu./0%lo/0x%lx   factor = %lu./0%lo/0x%lx\n",
         progname,offset,offset,offset,
                  length,length,length,
                  factor,factor,factor);
       fprintf(stderr,
"P=%d   C=%d   R=%d   S=%d\n",
            PBM_Flag,Complementing_Flag,Reverse_Flag,Swap_Flag);
     }

    if(fseek(stdin,offset,SEEK_SET))
     {
       perror(progname);
       exit(1);
     }

    if(PBM_Flag) /* Print PBM header */
     { /* If length is 72 we get 24x24, if it is 32 we get 16x16 */
       double size;
       int width,heigth;

       size = (length * 8); /* 8 pixels in one byte */
       width = heigth = ((int) sqrt(size));
       printf("P4\n%u %u\n",width,heigth);
     }

/* We go through this loop (length/BUFSIZ)+1 times.
   (Only once if length <= BUFSIZ).
 */
    while(length)
     {
       if(length > BUFSIZ) { chunk = BUFSIZ; }
       else                { chunk = length; }

       if((bytes_read = fread(bufferi,1,chunk,stdin)) != chunk)
	{
          unsigned char *p;

          fprintf(stderr,
"%s: read only %lu bytes (%lu bytes missing from the end?) (filling rest with zeros)\n",
               progname,((unsigned long int)bytes_read)
                       ,((unsigned long int)(length-bytes_read)));

          p = ((unsigned char *) bufferi) + chunk;
          while(p > (((unsigned char *) bufferi) + bytes_read))
           { *--p = '\0'; }
	}

/* Of course Swapping, Complementing and Reversing bytes should all work
   simultaneously...
 */
       if(Complementing_Flag || Reverse_Flag || Swap_Flag)
	{ /* Process the bytes in buffer. */
          unsigned char *p,tmp;
          unsigned int ind;
/*
          p = ((unsigned char *) bufferi) + chunk;
          while(--p >= ((unsigned char *) bufferi))
 */
          ind = 0;
          p = ((unsigned char *) bufferi);
          while(p < ((unsigned char *) (bufferi+chunk)))
           {
             if(Swap_Flag && !(ind&1)) /* Do this only when ind is even. */
              { tmp = *(p+1); *(p+1) = *p; *p = tmp; }
             if(Complementing_Flag) { *p = ~*p; }
             if(Reverse_Flag) { *p = reverse_byytti(*p); }
             p++;
             ind++;
           }
	}
       if((bytes_written = fwrite(bufferi,1,chunk,stdout)) != chunk)
	{
          fprintf(stderr,
"%s: wrote only %lu bytes (output file full?)\n",
               progname,((unsigned long int)bytes_written));
	}
       /* Stop processing if an anomalous condition was met: */
       if((bytes_read != chunk) || (bytes_written != chunk)) { break; }

       length -= chunk;
     }
}


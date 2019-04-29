 
 
#include "kana.h"
 
 
/* This macro converts kana name defined in kana.h
    to two byte Shift-JIS katakana code:
 */
#define q(x) (((x) > 62) ? 0x8341+(x) : 0x8340+(x))
 
 
/* Conversion table from single-byte katakana codes A0-DF
    to two-byte Shift-JIS codes
 */
 
unsigned int katakana_tab[] =
 {
   /* A0 */ 0x0000, /* Blanko or something */
   /* A1 */ 0x8142, /* Japanese period */
   /* A2 */ 0x8175, /* Kind of quote */
   /* A3 */ 0x8176, /* Kind of quote, closing one */
   /* A4 */ 0x8141, /* Japanese comma */
   /* A5 */ 0x8145, /* Kind of dot */
   /* A6 */ q(WO),
   /* A7 */ q(a),
   /* A8 */ q(i),
   /* A9 */ q(u),
   /* AA */ q(e),
   /* AB */ q(o),
   /* AC */ q(ya),
   /* AD */ q(yu),
   /* AE */ q(yo),
   /* AF */ q(tsu),
   /* B0 */ 0x815B, /* Prolongation of wovel - dash ? */
   /* B1 */ q(A),
   /* B2 */ q(I),
   /* B3 */ q(U),
   /* B4 */ q(E),
   /* B5 */ q(O),
   /* B6 */ q(KA),
   /* B7 */ q(KI),
   /* B8 */ q(KU),
   /* B9 */ q(KE),
   /* BA */ q(KO),
   /* BB */ q(SA),
   /* BC */ q(SHI),
   /* BD */ q(SU),
   /* BE */ q(SE),
   /* BF */ q(SO),
   /* C0 */ q(TA),
   /* C1 */ q(CHI),
   /* C2 */ q(TSU),
   /* C3 */ q(TE),
   /* C4 */ q(TO),
   /* C5 */ q(NA),
   /* C6 */ q(NI),
   /* C7 */ q(NU),
   /* C8 */ q(NE),
   /* C9 */ q(NO),
   /* CA */ q(HA),
   /* CB */ q(HI),
   /* CC */ q(FU),
   /* CD */ q(HE),
   /* CE */ q(HO),
   /* CF */ q(MA),
   /* D0 */ q(MI),
   /* D1 */ q(MU),
   /* D2 */ q(ME),
   /* D3 */ q(MO),
   /* D4 */ q(YA),
   /* D5 */ q(YU),
   /* D6 */ q(YO),
   /* D7 */ q(RA),
   /* D8 */ q(RI),
   /* D9 */ q(RU),
   /* DA */ q(RE),
   /* DB */ q(RO),
   /* DC */ q(WA),
   /* DD */ q(N),
   /* DE */ 0x814A, /* Nigori */
   /* DF */ 0x814B, /* Maru */
};



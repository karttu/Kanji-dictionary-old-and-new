
/* Some commented out shit from lisp.c: */
/* Commented out non-existent functions
   { "set-abrev-char", &_LAMBDA_, 1, set_abrev_char },
   { "set-getvalue-char", &_LAMBDA_, 1, set_getvalue_char },
   { "set-ibm-scand",  &_LAMBDA_, 1, set_ibm_scand },
   { "set-dins-flag",  &_LAMBDA_, 1, set_dins_flag },
   { "switch-compact-read", &_LAMBDA_, 1, L_switch_compact_read },
   { "set-c-mode",     &_LAMBDA_, 0, set_c_mode },
   { "set-asm-mode",   &_LAMBDA_, 0, set_asm_mode },
   { "setlispmode",    &_LAMBDA_, 0, setlispmode },
   { "setkdmode",      &_LAMBDA_, 0, setkdmode },
   { "setsentmode",    &_LAMBDA_, 0, setsentmode },
   { "restoremode",    &_LAMBDA_, 0, restore_mode },
   { "switch-unsigned-printing", &_LAMBDA_, 1, switch_unsigned_printing },
 */


/* Some unnecessary shit from lists1.c */
#define DOUBLEQUOTE   '"'
#define QUOTE         '\''
#define BACKQUOTE     '`'
#define SLASH         '/'
#define SEMICOLON     ';'

#define LPAR          '(' /* +1 */
#define RPAR          ')'
#define LBRACE        '{' /* +2 */
#define RBRACE        '}'
#define LBRACKET      '[' /* +2 */
#define RBRACKET      ']'
#define LANGLE        '<' /* +2 */
#define RANGLE        '>'

TOB set_c_mode()
{
        savefuns_et_flags();
        c_mode = 1;
        isquotefun=Cisquote;
        islbegcharfun=Cislbegchar;
        islendcharfun=Cislendchar;
        iscontinuousfun=Ciscontinuous;
        islbegsymfun=Cislbegsym;
        skipcommentchar = '\0';
        dtpr_flag = 0;
        esc_flag  = 1;
        sentmode = 0;
        charquote = '\'';
        lispmode = 0;
        *quote_flag=1;
        return(NIL);
}

TOB set_asm_mode()
{
        set_c_mode();
        charquote = '\0';
        asm_mode = 1;
        skipcommentchar = ';';
        esc_flag = 0;
        sentmode = 0;
        *quote_flag=1;
        return(NIL);
}


TOB setlispmode()
{
         savefuns_et_flags();
         lispmode = 1;
         dtpr_flag = 1;
         charquote = '`';
         esc_flag = 1;
         sentmode = 0;
         plist_flag = 1;
         *quote_flag=1;

/* Set the read macros for various characters: */

         return(NIL);
}


/* Read mode what KanjiDictionary uses: */
TOB setkdmode()
{
         savefuns_et_flags();
         dtpr_flag = 1;
         charquote = '`';
	 skipcommentchar = '\0';
         esc_flag = 1;
         *quote_flag=1;
         /* Chase bugs of obscure nature which may corrupt the database: */
         *warning_flag = WENABLED;
         sentmode = 0;
         return(NIL);
}

/* Read mode good for reading human language text sentences.
    (with scandinavian letters too (i.e: {,},| and [,],\): */
TOB setsentmode()
{
         savefuns_et_flags();
         isquotefun    = ALWAYSnull;
      /* Don't read the lists in the sentence mode: */
         islbegcharfun = ALWAYSnull;
         islendcharfun = ALWAYSnull;
         islbegsymfun  = ALWAYSnull;

         iscontinuousfun    = SENTiscontinuous;

         sentmode = 1;
         dtpr_flag = 0;
         charquote = '\0';
	 skipcommentchar = '\0';
         lispmode   = 0;
         esc_flag = 0;
         *quote_flag=1;
         return(NIL);
}





/* kanjidic.c  --  Electronic Kanji Dictionary and Teaching Software using
    some hypertext & other peculiar ideas.
   Idea: by Antti Karttunen at autumn 1988.
   Coding & Data entering: by -- " -- at winter 1988-89 & autumn 1989 &
    winter 1990.
   17-SEP-89: Now uses conanized TOB's developed for Lisp
 */

/*
#include "stdio.h"
#include "ctype.h"
#include "signal.h"
#include "setjmp.h"
#include "mydefs.h"
#include "lists.h"
#include "fundefs.h"
#include "globals.h"
#include "grafuns.h"
#include "ansiesc.h"
#include "kanjidic.h"
#include "kdfundef.h"

#include "keycodes.h"
 */

#include "kdinclu.h"


/* TOB save_vks(),restore_vks(),getvks_entry(),getvks_expr(); */
char *getenv();
FILE *myfopen();


BYTE changes_made=NOT_AT_ALL;

TOB hivec;
TOB jivec;
TOB nivec;

TOB entrypointer; /* Pointer to hivec, jivec or nivec */
TOB current_entry; /* Car of entrypointer */
TOB current_mode; /* :HI, :JI or :NI */

/* Index to hivec, jivec or nivec
    (current_entry is current_index:th entry in vector): */
UINT current_index;

TOB special_screen=NIL;
TOB previous_entry=NIL;

TOB separate_entry=NIL;

TOB entry_stack=NIL;

TOB _FIXED_FIELDS_=NIL;
TOB _USE_PRIMPREF_CMDS_=NIL;
TOB _DINS_CMDS_=NIL;

TOB _JI_;    /* :JI */
TOB _NI_;    /* :NI */
TOB _NK_;    /* :NK */
TOB _HI_;    /* :HI */
TOB _HK_;    /* :HK */
TOB _E_;     /* :E  */
TOB _EX_;    /* :EX */
TOB _EXE_;   /* :EXE */
TOB _R_;     /* :R */
TOB _S_;     /* :S  */
TOB _P_;     /* :P  */
TOB _G_;     /* :G  */
TOB _SEE_;   /* :SEE */
TOB _STR_;   /* :STR */
TOB _OTHER_;     /* :OTHER */
TOB _COMMENT_;   /* :COMMENT */
TOB _COMPOUNDS_; /* :COMPOUNDS */

TOB _HIRAGANA_;
TOB _KATAKANA_;
TOB _G_HIRAGANA_;
TOB _G_KATAKANA_;

TOB _DONT_EXPAND_; /* !DONT_EXPAND */
TOB _NIOL_;        /* !NIOL */
TOB _SOMETHING_;   /* !SOMETHING */
TOB _PAIR_;        /* !PAIR */

TOB _NOT_;
TOB _AND_;
TOB _OR_;

TOB _QM_;         /* ? Question Mark */
TOB _ASTERISK_;   /* * */
TOB _EQ_;         /* = */
TOB _PLUS_;       /* + */
TOB _MINUS_;      /* - */
TOB _SLASH_;      /* / */
TOB _DOLLAR_;     /* $ */
TOB _COMMA_;      /* , */
TOB _SEMICOLON_;  /* ; */
TOB _MIAU_;       /* @ */
TOB _CARET_;      /* ^ */
TOB _LT_;         /* < */
TOB _GT_;         /* > */


/* Difference between kata- and hiragana, calculated in init2, probably 83. */
int hira_to_kata=0;

int maxfilelines_in_vks = 50; /* Default */

int maxentries,readentries;

char querybuf[MAXBUF+4]; /* ="(#"; (Now done dynamically) */
char sparebuf[MAXBUF+4];
int argcount;
TOB arglist,cmd,arg1,arg2,arg3,arg4,arg5;

int repcnt=0;
int permrepcnt=0;

BYTE goto_exprs_too=0; /* Flag for move cursor functions in VKS-FUNS.C */
BYTE debug_flag=0;

UINT loc_counter;
BYTE loc_counter_flag=0;
BYTE _E_flag=0;
BYTE printtempflag=1;
BYTE rawprintflag=1;

BYTE _no_checks=0,_ovldebug=0; /* Flags for ovloader */

PFI adr_of_ovmain=NULLFP;  /* Address of ovmain() of latest loaded overlay */
char *last_overlay=NULL; /* Name of latest loaded overlay */

int G_argc;
char **G_argv;

/* Body odor is the window to the soul. */

static jmp_buf envbuf;
static void sighandler();

FILE *ERROR_FP;
FILE *KANJI_FP;
FILE *CMD_FP;
FILE *sidewindow;

int promptline=0;

extern TOB saved_extents;

TOB sparecons=NIL;

main(argc,argv)
int argc;
char **argv;
{
        G_argc = argc; /* So that INIT can access command line arguments... */
        G_argv = argv;

	ovlhandler("init1"); /* Load module INIT1.OVR and execute it */
	ovlhandler("init2"); /* Load module INIT2.OVR and execute it */
        loop(); /* Transfer control to main command reading loop */
        EXITHR; /* Turn HIRES off */
}



/* ---------------------------------------------------------------------- */
/* Functions to access database: (see also macros in kanjidic.h) */

/* Returns pointer to vector corresponding to mode (= :HI, :JI or :NI),
    that is, return index:th cdr of that compact list. If index is
     out of the limits NIL is returned.
 */
TOB get_ptr_to_nth_entry(index,mode)
UINT index;
TOB mode;
{
        if(isoutoflimits(index,mode)) { return(NIL); }
        else { return(rawqnthcdr(index,getvector(mode))); }
}


/* Tries to set entrypointer to index:th entry. However if it goes out
    of the limits NIL is returned.
 */
TOB set_entrypointer(index)
UINT index;
{
        TOB x;

        if(nilp(x=get_ptr_to_nth_entry(index,current_mode))) { return(NIL); }
        current_index = index;
        current_entry = car(x);
        return(entrypointer = x);
}

/* Like previous, but prints error message if fails. */
TOB chkset_entrypointer(index)
UINT index;
{
        TOB x;

        if(nilp(x = set_entrypointer(index)))
         {
           pr_index_error();
         }
        return(x);
}


/* Tries to forward entrypointer n steps: */
TOB forward_entrypointer(n)
UINT n;
{
        TOB x;
        UINT i=0;

        x = getentrypointer();
        while(n--)
         {
           x = cdr(x);
           i++;
           if(nilp(x)) { return(NIL); }
         }

        current_index += i;
        current_entry = car(x);
        return(entrypointer = x);
}


/* Changes indexing mode: */
TOB switch_mode(mode)
TOB mode;
{
        if(!eq(mode,_HI_) && !eq(mode,_JI_) && !eq(mode,_NI_))
         { return(NIL); }
        else
         {
           current_mode = mode;
	   set_entrypointer(!(eq(mode,_JI_))); /* 0 if :JI, 1 if :HI or :NI */
           return(mode);
         }
}



/* Fetches field from entry: */
TOB getfield(key,entry)
TOB key,entry;
{
        register TOB loc;
        TOB origentry;

        origentry = entry;

        /* If key is for fixed location field, then take loc:th
           item from start of permanent part: */
        if(intp(loc = getfieldloc(key)))
         {
           entry = getpermpart(entry);
           if(!consp(entry)) { goto ertzu; }
/* However, if this is special entry starting with * then it doesn't
    contain any fixed location fields: */
           if(eq(car(entry),_ASTERISK_)) { return(NIL); }
           else { return(cxr(tob_int(loc),entry)); }
         }
        else /* It's normal variable location key which actually */
         {   /*  is there in entry. */
           /* Flags by itself connotate something: */
           if(flagp(key)) { return(car(memq(key,entry))); }
           /* Keys proper has some field after them: */
           else { return(cadr(memq(key,entry))); }
	 }

ertzu:
        erprintf((ER,
         "**WARNING, getfield: detected invalid entry, key: "));
        printexpr(key,ER);
        erprintf((ER,"\nEntry: "));
        printexprnl(origentry,ER);
        return(NIL);
}


/* Like getfield, but returns always list. I.e. if field contains something
 *  non-list then list of one element is made from that.
 */
TOB getlfield(key,entry)
TOB key,entry;
{
        TOB result;

        result = getfield(key,entry);
        if(length1p(result))
         {
           erprintf((ER,
            "**getlfield: detected field of length 1 ! key: %s   entry: ",
            pname(key)));
           printexprnl(entry,ER);
           result = car(entry);
/*         return(NIL); */
         }
        if(singlereferp(result)) { result = list1(result); }
        return(result);
}



/* More complex functions to get entry pointed by reference, etc.: */

#ifndef _QUESTION_MARK
#define _QUESTION_MARK 8 /* (?) Index to font-file */
#endif


TOB getentry_et_prsymb(ptrtosym,refer)
TOB *ptrtosym;
TOB refer;
{
        TOB entry,word;
        int i;

        word = NIL;

        if(consp(refer)) { word = car(refer); refer = cdr(refer); }
        entry = get_entry(refer);

        if(nilp(word)) /* If word not set yet... */
         {
           if(nonnilsymbolp(refer)) { word = refer; }
           else { word = getkeyword(entry); }
         }

        if(intp(refer) && ((i = tob_int(refer)) < 1) && nilp(entry))
         { entry = int_tob(-i); }

        *ptrtosym = word;
        return(entry);
}


/* This gets entry corresponding to refer, which can be:
    keyword
    NI-index (positive number)
    JI-index (negative number)
    (word . one-of-previous-three)
 */
TOB get_entry(refer)
TOB refer;
{
        TOB result;
        int i;

        if(consp(refer)) { refer = cdr(refer); }

        if(nonnilsymbolp(refer)) /* It's HK-keyword (or primitive-name) */
         {
	   result = value(solve_alias(refer));
/* If there is more than one entry in value of symbol, then take first one: */
	   if(!single_entryp(result)) { result = car(result); }
         }
        else if(intp(refer))
         {
           i = tob_int(refer);
           if(i < 1) /* It's JIS-INDEX */
            {
              /* Negate i: */
              result = (nth_entry((-i),_JI_));
            }
           else /* It's NI-INDEX */
            {
              result = (nth_entry(i,_NI_));
            }
         }
        else
         {
           return(NIL);
         }

        return(result);
}

TOB chkget_entry(refer)
register TOB refer;
{
        TOB result;

        result = get_entry(refer);
        if(!consp(result))
         {
           erprintf((ER,"**chkget_entry: INVALID REFERENCE: "));
           printexpr(refer,ER);
           erprintf((ER,"   Resulted: "));
           printexprnl(result,ER);
	   return(NIL);
         }
        else { return(result); }
}


/* Goes to location where entry is. Tries to keep current_mode same.
    However, if that is not possible, then tries :NI, then :HI and
     finally :JI mode, before giving up.
 */
TOB goto_entry(entry)
TOB entry;
{
        TOB ind,save_mode;

        save_mode = getcurrent_mode();

        if(nilp(entry)) { goto ertzu; }
        else if(intp(ind = getfield(save_mode,entry))) { }
        else if(intp(ind = getfield(_NI_,entry))) { current_mode = _NI_; }
        else if(intp(ind = nthmemq(entry,nivec))) { current_mode = _NI_; }
        else if(intp(ind = getfield(_HI_,entry))) { current_mode = _HI_; }
        else if(intp(ind = getfield(_JI_,entry))) { current_mode = _JI_; }
        else
         {
ertzu:
           erprintf((ER,"**Cannot go anywhere, because entry is   "));
           printexprnl(entry,ER);
           return(NIL);
         }

        if(nilp(chkset_entrypointer(tob_int(ind))))
         { current_mode = save_mode; return(NIL); }
        return(getcurrent_mode());
}



/* Returns some symbol from entry "as keyword". First tries :HK field. */
/* If nothing appropriate is found, returns NIL */
TOB getkeyword(entry)
TOB entry;
{
	TOB get_first_word();
        register TOB x;

        if(nilp(entry)) { return(NIL); }
        else if(!nilp(x = getfield(_HK_,entry)))    {}
        else if(!nilp(x = getfield(_NK_,entry)))    {}
        else if(!nilp(x = getfield(_HIRAGANA_,entry)))    {}
        else if(!nilp(x = getfield(_KATAKANA_,entry)))    {}
        else if(!nilp(x = getfield(_P_,entry)))     {}
        else if(!nilp(x = getfield(_OTHER_,entry))) {}

/* If contents of field was list, then just choose first one: */
        return(get_first_word(x));
}

TOB get_first_word(arg)
TOB arg;
{
        while(consp(arg)) { arg = car(arg); }
        return(arg);
}



/* Checks whether there exists symbol with same print name as elem,
    but preceded by ^ -sign, and return that if there is,
    otherwise return just elem itself.
   Esimerkki valaiskoon asiaa:
        getelem(sun) palauttaa symbolin ^sun
        mutta getelem(fish) palauttaa vain fish:in takaisin, koska
        ei ole symbolia ^fish
   I think this is unnecessary now, because lookup automatically
    returns symbol preceded by ^ if there is one, and if prefix_char
    is set to '^'.
 */
TOB getelem(elem)
TOB elem;
{
	TOB tmp;
        char buf[81];

        if(!nonnilsymbolp(elem) || primitivep(elem)) { return(elem); }
        else
         {
  	   buf[0] = '^';
	   strcpy((buf+1),pname(elem));
	   if(!nilp(tmp = lookup(buf))) { return(tmp); }
	   else { return(elem); }
         }
}


/* If sym is primitive then return "alias-root" of it, i.e. its value: */
TOB solve_alias(sym)
TOB sym;
{
        TOB tmp;

        if(nonnilsymbolp(sym) && nonnilsymbolp(tmp = value(sym)))
         { return(tmp); }
	else { return(sym); }
}



/* This is auxiliary function for commands which use prefix
    & abbreviation read modes, to detect unmatched ones, etc.
   (Used by EF & EA)
 */
/* Check that there is no errors in lista. Return NIL if there is,
    otherwise return lista, possible little bit manipulated one.
 */
TOB check_ambiguities(lista)
TOB lista;
{
	register TOB l;
        TOB ambig_list;

        /* Check for unmatched and ambiguous symbols/abreviations: */
        for(l = lista; !nilp(l); l = cdr(l))
         {
           if(nilp(ambig_list = car(l)))
            {
              erprintf((ER,
               "**Symbol not found or unmatched abreviation.\n"));
              return(NIL);
            }
           else if(consp(ambig_list))
            { /* If more than one symbol matched abreviation */
              TOB lalli;

	 /* Check that there is at least one primitive in ambiguous cases: */
              if(nilp(lalli = mempred(f_primitivep,ambig_list)))
               { goto ertzulus; }
         /* However, use that primitive only if it is only one in list: */
              if(nilp(mempred(f_primitivep,cdr(lalli))))
               {
                 rplaca(l,car(lalli));
                 free_list(ambig_list);
               }
              else
               {
ertzulus:
                 erprintf((ER,"**Ambiguous abreviation.\n"));

/* Don't free it, because EA command prints pattern list after this,
    and it would be contain _freelist after this.
    (Because free_list just nconcs its argument to _freelist */
/*               free_list(ambig_list); */
                 return(NIL);
               }
            }
         }

        return(lista);
}



/* ---------------------------------------------------------------------- */
/* Functions to print entries & kanjis: */


printkanjiandlist(entry)
register TOB entry;
{
        extern TOB last_ptr;

        TOB tmp;

        if(!nilp(last_ptr))
         {
           clear_vks();
           kanji_et_text_newlines(1);
           knewlines(1);
         }

        if(!nilp(tmp = getfield(_JI_,entry)))
         {
           kanjiput(tob_int(tmp));
           kanjinewlines(1);
         }
        else { knewlines(1); }
	if(loc_counter_flag == 2) { fprintf(KANJI_FP,"%d ",loc_counter); }
        printexpr((printtempflag ? gettemppart(entry) : getpermpart(entry)),
                     KANJI_FP);
        knewlines(2);
}



/* ---------------------------------------------------------------------- */

/* Function versions of commandp, flagp, keyp and primitivep for
    "functional argument" usage: */

UINT f_commandp(x)
TOB x;
{
        return(commandp(x));
}

UINT f_flagp(x)
TOB x;
{
        return(flagp(x));
}

UINT f_keyp(x)
TOB x;
{
        return(keyp(x));
}


UINT f_primitivep(x)
TOB x;
{
        return(primitivep(x));
}

/* ---------------------------------------------------------------------- */



/* ---------------------------------------------------------------------- */

/* Functions to handle overlay loading, signal handling, etc...   */



/* Like ovloader, but don't load overlay to memory if it is in there already,
    instead go directly to it: */
ovlhandler(ovlname)
char *ovlname;
{
        if(ovlname == last_overlay) { ((*adr_of_ovmain)()); }
	else
	 {
	   last_overlay = ovlname;
	   ovloader(ovlname);
	 }
}


/* This catches CTRL-C */
void sighandler(sig)
int sig;
{
        char c;

        w_clear(ER);
        erprintf((ER,
         "**sighandler: received signal %d.\n",sig));
        signal(SIGINT,sighandler); /* Reassign signal SIGINT */
        erprintf((ER,
"C continue,  R return to start,  X exit to operating system>"));
        while(!key_pressed()) { } c = _toupper(scr_getc());
        putc(c,ER); terpri(ER);
/*      c = _toupper(getchar()); terpri(stderr); */
        if(c == 'R') { longjmp(envbuf,-1); }
        if(c == 'X') { myexit(0); }
        else { return(); }
}


int argerr()
{
        erprintf((ER,"**Argument missing or invalid !\n"));
}

int pr_index_error()
{
        erprintf((ER,"**Index is out of the limits.\n"));
}


/* ---------------------------------------------------------------------- */

/* These functions can be used by overlays, if there is many commands
    in one overlay (like restcmds & editcmds):
 */

static TOB search_cmd_arg=NIL;

/* Checks whether x or car of x is equivalent to search_cmd_arg: */
int search_cmd_name_aux(x)
TOB x;
{
        return(eq(x,search_cmd_arg) ||
                (consp(x) && eq(car(x),search_cmd_arg)));
}



int set_command_adr(cmd_name,cmd_adr)
char *cmd_name;
PFI cmd_adr;
{
        TOB sym,l;
        char buf[81];

        sprintf(buf,"#%s",cmd_name);
        if(nilp(sym = lookup(buf)))
         {
           fprintf(ERROR_FP,
"**set_command_adr: Encountered undefined command: %d/%s\n",
            strlen(buf),buf);
            return(-1);
         }

        search_cmd_arg = sym;
        l = mempred(search_cmd_name_aux,value(value(cmd)));
        if(nilp(l))
         {
           fprintf(ERROR_FP,
"**set_command_adr: command %s not found from list: ",pname(sym));
           printexprnl(value(value(cmd)),ERROR_FP);
           return(-1);
         }

        rplaca(cdr(l),fun_tob(cmd_adr));

        return(0);
}


PFI get_command_adr(sym)
TOB sym;
{
        TOB l;

        search_cmd_arg = sym;
        l = mempred(search_cmd_name_aux,value(value(cmd)));

        if(nilp(l))
         {
           fprintf(ERROR_FP,
"**get_command_adr: command %s not found from list: ",pname(sym));
           printexprnl(value(value(cmd)),ERROR_FP);
           return(NULLFP);
         }
        l = cadr(l);
        if(!bcdp(l))
         {
           fprintf(ERROR_FP,
"**get_command_adr: command %s has not been associated with proper fun-adr:     ",
               pname(sym));
           printexprnl(l,ERROR_FP);
           return(NULLFP);
         }

        return((PFI) tob_fun(l));
}



/* ---------------------------------------------------------------------- */


/* The main command loop: */

int loop()
{
        TOB fake_entry;

        /* Fake entry for those cases in :JI-mode where is no entry: */
/*      fake_entry = list1(NIL); */ /* (NIL) */
        fake_entry = clistn(NIL,NIL,NIL,NIL,NIL,NIL,
                       _COMMENT_,stringsave("Unknown."),ENDMARK);
        fake_entry = cons(fake_entry,fake_entry); /* ((NIL) . (NIL)) */
/*      fake_entry = list1(list1(NIL)); */ /* ((NIL)) */


        signal(SIGINT,sighandler); /* Catch CTRL-C:s */

        /* CTRL-C returns program to this location: */
        if(setjmp(envbuf))
         {
/*         signal(SIGINT,sighandler); */
           erprintf((ER,"**Returning from sighandler.\n"));
           kcls();
         }

        if(setjmp(to_toplevel_buf))
         {
           erprintf((ER,
"**Back to toplevel from an internal error. Press any key.\n"));
           scr_getc(); /* Wait for any key. */
           kcls();
         }

        switch_mode(_NI_);
        special_screen = previous_entry = separate_entry = arglist = NIL;
/*
        erprintf((ER,"Note: use command ? for information.\n"));
 */
        print_prompt();

        do {
           rplaca(car(fake_entry),int_tob(getcurrent_index()));
           if(rawprintflag)
            {
             if(!nilp(separate_entry))
              { terpri(stdout); print(separate_entry); terpri(stdout); }
             else
              {
/* If in JI-mode and there is no entry, then use fake-entry, so that
    JI-index replaces NIL or previous JI-index in it. */
                printkanjiandlist(
/* If */         ((eq(getcurrent_mode(),_JI_) && nilp(getcurrent_entry()))
/* Then */       ? fake_entry
/* Else */       : getcurrent_entry())); /* Use current_entry */
              }
             }
            else
             {
               if(eq(getcurrent_mode(),_JI_) && nilp(getcurrent_entry()))
                { print_entry(fake_entry); }

               /* Don't print screen unless current entry has changed: */
               else if(getcurrent_entry() != previous_entry)
                {
                  special_screen = NIL;
                  print_entry(getcurrent_entry());
                }
             }

             previous_entry = getcurrent_entry();

/*
             if(permrepcnt)
              {
                if(!repcnt) { repcnt = permrepcnt; }
                else
                 {
                   printexprnl(arglist,ER);
                   repcnt--;
                   goto skip_it;
                 }
              }
*/

             if(rawprintflag)
              { /* Kuntoudu - Osta tossut */
              }
             else
              {
                TOB handlecursorkeys();
                TOB iso_paha_city;

                if(!nilp(iso_paha_city = handlecursorkeys()))
                 { goto_entry(iso_paha_city); continue; }
                else
                 {
                   /* Dead cockroaches, good cockroaches */
                 }
              }

             print_prompt();
             read_and_execute_command();

           } while(1);
}

print_prompt()
{
        /* Clear the command line: */
        scr_curs(promptline,0);
        fprintf(CMD_FP,"%80s","");
        scr_curs(promptline,0);
/*              w_clear(CMD_FP); */
        /* Print the prompt: */
        printexpr((!nilp(special_screen) ? special_screen : getcurrent_mode())
                   ,CMD_FP);
/*
        fprintf(CMD_FP," %d",getcurrent_index());
 */
        if(!nilp(separate_entry))
         { fprintf(CMD_FP," [SEP] %d>",length(separate_entry)); }
        else { fprintf(CMD_FP,">"); }
}


read_and_execute_command()
{
	TOB value_of_cmd;
        PFI cmd_adr;

        DINS_flag = 0; /* Zero this if someone has turned it on */
        kd_prefix_char = '\0'; /* This too */
        querybuf[0] = ' '; querybuf[1] = '#';
        querybuf[2] = '\0'; querybuf[3] = '\0';

        myfgets((querybuf+2),MAXBUF,stdin);

        if(isdigit(*(querybuf+2)))
         {
           separate_entry = NIL;
           chkset_entrypointer(atoi(querybuf+2));
           return;
         }
        else if(*(querybuf+2) == ',')
         { /* If command like ,fish then put blanko between,
              so that it will be (#, fish) and read the whole
              thing in "Don't Intern New Symbols" mode. */
           strcpy(sparebuf,(querybuf+3)); /* Put...       */
           *(querybuf+3) = ' ';           /* ...blanko... */
           strcpy((querybuf+4),sparebuf); /* ...between.  */
/*         DINS_flag = 1; */
         }
        else if(*(querybuf+2) == '!') /* DOS command */
         {
           char *getenv();
           char *s; /* Name of shell */

           /* If env.var. COMSPEC hasn't been defined, then use command.com */
           if(!(s = getenv("COMSPEC"))) { s = "command"; }
/* If user typed just ! without arguments, then start the new shell,
    otherwise execute stuff after ! in shell. */
           if(system(('\0' == *(querybuf+3)) ? s : querybuf+3)
               == -1)
            {
              erprintf((ER,
"**system returned -1. Not enough memory or %s not found ?\n",s));
            }
           return;
         }

        /* Don't intern any mis-spellings of commands: */
        DINS_flag = 1;
        cmd = sreadexpr(querybuf+1);
        DINS_flag = 0;
        if(!commandp(cmd))
         {
           erprintf((ER,"**UNRECOGNIZED COMMAND !\n"));
           return;
         }

/* Surround arguments with parentheses so that they are read as list: */
        *querybuf = '('; /* Put parentheses to beginning of querybuf. */
        strcat(querybuf,")"); /* Add final parentheses */

        if(!nilp(memq(cmd,_USE_PRIMPREF_CMDS_)))
         { kd_prefix_char = '^'; }
        if(!nilp(memq(cmd,_DINS_CMDS_)))
         { DINS_flag = 1; }

        /* Free previous arglist: */
        if(consp(arglist)) { free_list(arglist); }
        arglist = sreadexpr(querybuf); /* Read argumentlist from string */
        if(endmarkp(arglist))
         { /* Unbalanced parentheses, or something */
           erprintf((ER,
             "**sreadexpr returned SPECMARK, querybuf is:\n"));
           erprintf((ER,"%s/%d\n",querybuf,strlen(querybuf)));
           return;
         }
        argcount = (length(arglist));
        arg1 = nth(0,arglist);
        arg2 = nth(1,arglist);
        arg3 = nth(2,arglist);
        arg4 = nth(3,arglist);
        arg5 = nth(4,arglist);

        value_of_cmd = NIL;

        if(!nonnilsymbolp(cmd)) { goto ertzu; }

        value_of_cmd = value(cmd);

        if(ovlfilep(value_of_cmd))
         { ovlhandler(pname(value_of_cmd)+1); return; }

        if(!(otherp(value_of_cmd) && bcdp(value_of_cmd)))
         {
ertzu:
           erprintf((ER,"**Invalid command: "));
           printexprnl(cmd,ER);
           erprintf((ER,"Value of it: "));
           printexprnl(value_of_cmd,ER);
           return;
         }

	cmd_adr = (PFI) tob_fun(value_of_cmd);

 /* If some function is not found in linking process, then Aztec Linker
     lets address of that function be zero, i.e. null function pointer: */
	if(!cmd_adr)
	 {
	   erprintf((ER,"**Command  ")); printexpr(cmd,ER);
	   erprintf((ER,"  is not linked to this executable.\n"));
	   return;
	 }
/* skip_it: */
        ((*(cmd_adr))()); /* Execute command */
}




TOB handlecursorkeys()
{
    TOB _RETURN();
    int c;
    TOB result;

    do {
         c = blinkcursor_et_pollchar();
/*
         if(nilp(saved_extents)) { w_clear(ER); }
 */
         if(!isascii(c) && (c != _F3))
          { scr_getc(); } /* Read away polled character */
         goto_exprs_too = 0;
         switch(c)
          {
            case _SHIFT_UP:    { scr_getc(); goto_exprs_too = 1; }
            case _UP:
             { move_up(); break; }
            case _SHIFT_DOWN:  { scr_getc(); goto_exprs_too = 1; }
            case _DOWN:
             { move_down(); break; }
            case _SHIFT_LEFT:  { scr_getc(); goto_exprs_too = 1; }
            case _LEFT:
             { move_left(); break; }
            case _SHIFT_RIGHT: { scr_getc(); goto_exprs_too = 1; }
            case _RIGHT:
             { move_right(); break; }
            case _SHIFT_HOME:  { scr_getc(); goto_exprs_too = 1; }
            case _HOME:
             { move_home(); break; }
            case _SHIFT_END:   { scr_getc(); goto_exprs_too = 1; }
            case _END:
             { move_end();  break; }
            case _SHIFT_PGUP:  { scr_getc(); goto_exprs_too = 1; }
            case _PGUP:
             { move_pgup(); break; }
            case _SHIFT_PGDN:  { scr_getc(); goto_exprs_too = 1; }
            case _PGDN:
             { move_pgdn(); break; }

            case _CTRL+'D': /* On-Screen Debug */
             {
               TOB previ,nexti;

               scr_getc();
               previ = getptr_to_prev_filemarker(cursorpointer);
               nexti = getptr_to_next_filemarker(cursorpointer);

               printexpr(car(vks_vector),NOTICE_FP);
               putc(' ',NOTICE_FP);
               printexpr(car(last_ptr),NOTICE_FP);
               putc(' ',NOTICE_FP);
               printexpr(previ,NOTICE_FP);
               putc(' ',NOTICE_FP);
               printexpr(nexti,NOTICE_FP);
               fprintf(NOTICE_FP," col: %6d   line: %6d\n",
                 getvks_column(car(cursorpointer)),
                   getvks_line(car(cursorpointer)));

               ptr_status(nexti);

               break;
             }

            case _INS: /* General "goto there" or "show that" key */
             {
               extern TOB cursorpointer;
               TOB kylma_iho,expr;

               kylma_iho = getvks_entry(car(cursorpointer));

               if(intp(kylma_iho))
                {
                  if(tob_int(kylma_iho) >= JIS2LEVEL_LIM)
                   {
                     fprintf(ERROR_FP,
"\007**JIS level 2 kanji !\n");
                   }
                  else
                   {
                     fprintf(ERROR_FP,
"\007**Unknown character for me !\n");
                   }
                  break;
                }

               if(eq(kylma_iho,_T_))
                {
                  expr = getvks_expr(car(cursorpointer));
                }
               else { expr = NIL; }

               /* If tries to go to entry/screen which is at the top of the
                    entry stack, then go to there, but pop that entry also: */
               if(eq(kylma_iho,car(entry_stack)) ||
                   (!nilp(expr) && screenp(car(entry_stack)) &&
                     eq(nth(1,car(entry_stack)),expr)))
                { goto pop_entry; }

               /* Do nothing if tries to go to the same entry:
                  (However, if is at special screen, then goes to location) */
               if(eq(kylma_iho,getcurrent_entry()) && nilp(special_screen))
                { break; }

               /* If we are currently in some special screen, then it should
                   be saved before we go to any new entry/screen: */
               if(!nilp(special_screen))
                {
                  push_entrystack(save_vks(special_screen));
                  previous_entry = NIL;
                }
               else { push_entrystack(getcurrent_entry()); }

 /* If cursor is at reference to special screen, probably a groupname: */
               if(groupnamep(expr))
                {
                  kcls();
                  print_group(expr);
                  /* Save the group name to this variable: */
                  special_screen = expr;
                  break;
                }
 
               special_screen = NIL;
               result = kylma_iho; goto re_turn;
             }
pop_entry:  case _DEL: /* Return back to previous entry/screen */
             {
               result = _RETURN();
               if(!nilp(result)) { return(result); }
               break;
             }
            default:
             {
               result = NIL; goto re_turn;
             }
          }
;
       } while(1);

re_turn:
        if(!nilp(saved_extents))
         {
           if(nilp(result)) { result = (getcurrent_entry()); }
           restore_extents();
         }
        return(result);
}


restore_extents()
{
           special_screen = previous_entry = NIL;
           restore_kanjiwindow();
           w_clear(sidewindow);
}


TOB _RETURN()
{
        TOB liha;

        pop_entrystack(liha);
        if(nilp(liha))
         {
           erprintf((ER,"\007**No more entries in return stack !\n"));
           return(NIL);
         }
        if(screenp(liha))
         {
           special_screen = cadr(liha);
           restore_vks(liha);
           return(NIL);
         }
        else
         {
           special_screen = previous_entry = NIL;
           if(!nilp(saved_extents))
            { restore_extents(); }
           goto_entry(liha);
           return(liha);
         }
}



/* ---------------------------------------------------------------------- */
/* Internal Commands: */


/* Do nothing, used for going back to "cursor mode" when
accidentally pressed PgDn or PgUp or something else in screen mode. */
int _NULL()
{
}


/* This is good to be in internal commands, in case that module restcmds
    can't be loaded for some reason.
 */
/* Write database */
int _W()
{
        FILE *fp;

        fp = myfopen(getenv("NEWDATABASE"),"w");
        erprintf((ER,"Wrote %d lines.\n",write_database(nivec,fp)));
        fclose(fp);
        changes_made = NO;
}


int write_database(lista,fp)
TOB lista;
FILE *fp;
{
        register int z=0;

        while(lista)
         {
           printexpr(getpermpart(car(lista)),fp); terpri(fp);
           lista = cdr(lista); z++;
         }

        return(z);
}


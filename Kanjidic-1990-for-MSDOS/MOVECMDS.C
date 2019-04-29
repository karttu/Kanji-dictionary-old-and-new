

#include "kdinclu.h"


int _MINUS(),_PLUS(),_CHANGEMODE(),_GOVAL(),_NE();
int _SHLIST();



ovmain()
{
        int ovmain();
        PFI fun;

        adr_of_ovmain = ovmain;

        /* If command name & address list hasn't been initialized yet,
            (first time in this module), then initialize it: */
        if(nilp(cadr(value(value(cmd)))))
         { set_command_adrs(); }

        if(!(fun = get_command_adr(cmd))) { /* Do nothing */ }
        else { ((*fun)()); }
}



set_command_adrs()
{
        set_command_adr("NE",_NE);
	set_command_adr("-NE",_NE);
        set_command_adr("-",_MINUS);
        set_command_adr("+",_PLUS);
        set_command_adr("",_PLUS);
        set_command_adr("HI",_CHANGEMODE);
        set_command_adr("JI",_CHANGEMODE);
        set_command_adr("NI",_CHANGEMODE);
        set_command_adr("HI.",_CHANGEMODE);
        set_command_adr("JI.",_CHANGEMODE);
        set_command_adr("NI.",_CHANGEMODE);

        set_command_adr(",",_GOVAL);
        set_command_adr("$",_SHLIST);
}



/* Commands for changing place: -   +   NE   ,   HI, JI and NI */


int _MINUS() /* Go backward */
{
        separate_entry = NIL;
        if(!intp(arg1)) { arg1 = int_tob(1); }
        if(nilp(set_entrypointer(getcurrent_index()-tob_int(arg1))))
         { set_entrypointer(getuplim(getcurrent_mode())); }
}



int _PLUS() /* Go forward */
{
        separate_entry = NIL;
        if(nilp(forward_entrypointer((intp(arg1) ? tob_int(arg1) : 1))))
         { set_entrypointer(0); }
}



int _CHANGEMODE() /* hi,ji and ni */
{
        TOB x,sym;
        BYTE dotflag = 0;
        char cmd_name[12];

        separate_entry = NIL;
        /* Make new copy of string, otherwise results would be disastrous ! */
        strcpy(cmd_name,pname(cmd));
        /* If it is #HI. or like, then remove dot: */
        if(*(cmd_name+3) == '.')
        { dotflag = 1; *(cmd_name+3) = '\0'; }
        /* No need to make cmd_name uppercase, because that's done already. */
        *cmd_name = ':'; /* Change # to : */
        sym = lookup(cmd_name); /* Get corresponding index-key */
        x = NIL;
        if(intp(arg1)) { x = arg1; }
        else if(dotflag) { x = getfield(sym,getcurrent_entry()); }
        switch_mode(sym);

        if(!nilp(x)) { chkset_entrypointer(tob_int(x)); }
}        



int _GOVAL() /* COMMA, Go to entry indicated by Value */
{
        special_screen = separate_entry = NIL;

        if(argcount < 1) { commahelp(); return(-1); }
        if(nilp(arg1))
         {
           erprintf((ER,"**Symbol not found or unmatched abreviation.\n"));
           return(-1);
         }
        else if(consp(arg1))
         {
           erprintf((ER,"**Ambiguous abreviation <%d>: ",length(arg1)));
           printexprnl(arg1,ER);
           arg1 = car(arg1);
         }

/*
        if(!nonnilsymbolp(arg1)) { arg_err(); }
 */

        if(keyp(arg1)) /* If entered ,:S or like */
         {
           TOB sym;

           if(nilp(sym = getfield(arg1,getcurrent_entry()))) { return(-1); }
           if(!singlereferp(sym)) { sym = car(last(sym)); }
           arg1 = sym;
         }
        else if(nonnilsymbolp(arg1))
         {
           TOB v,x;

           arg1 = solve_alias(arg1);

           v = value(arg1);
	   if(nilp(v)) { goto_entry(NIL); return(-1); } /* Error message */
           /* If there is only one entry in value, then make list of one
	       entry from it: */
	   if(single_entryp(v)) { v = rplaca(sparecons,v); }
           /* If we are not currently at some of those entries containing the
               symbol, then start from beginning of values: */
           if(nilp(x = memq(getcurrent_entry(),v)))
            { x = car(v); }
           else if(nilp(x = cadr(x)))/* Get the next entry, and check if NIL*/
            { /* If we were at last entry contained in values of symbol: */
              fprintf(NOTICE_FP,"\007**No more found, try from beginning.\n");
              return(-1);
            }
           goto_entry(x); /* Goto the first or next entry */
           return(1);
         }

        goto_entry(chkget_entry(arg1));

}



commahelp()
{
        FILE *HELP_FP;

        HELP_FP = NOTICE_FP;
        w_clear(HELP_FP);
	fprintf(HELP_FP,
"Use   ,word   for going to first entry where is word. If you are already\n");
        fprintf(HELP_FP,
"at such entry, goes to the next one, if there is any.\n");
        fprintf(HELP_FP,
"Examples: ,fish   ,^angel   ,%neko   ,~KA   ,$SEASONS   ,:S   ,11   ,-700\n");
}




/* Go to the next one where is(n't) field given as argument */
int _NE()
{
	BYTE negate_flag=0;

        separate_entry = NIL;
        if(argcount < 1)
         {
           fprintf(NOTICE_FP,
"ne :key    Go to next entry which contain field indicated by :key.\n");
           fprintf(NOTICE_FP,
"ne ()      Go to next entry which is NIL.\n");
           fprintf(NOTICE_FP,
"-ne :key / -ne ()  Go to next entry which doesn't contain :key / isn't NIL.\n");
           return(-1);
         }
        if(*(pname(cmd)+1) == '-') { negate_flag = 1; }

        if(nilp(arg1))
         { /* Search next entry which is NIL (or not nil if -NE) */
           while(!nilp(forward_entrypointer(1)))
            {
	      if(nilp(getcurrent_entry()) ^ negate_flag)
               { break; }
            }
         }                            
        else
         {
           while(!nilp(forward_entrypointer(1)))
            {
	      if(!nilp(getcurrent_entry()) &&
                 (!nilp(getfield(arg1,getcurrent_entry())) ^ negate_flag))
                  { break ; }
            }
         }
}




int _SHLIST()
{
	char c;

        if(rawprintflag)
         { knewlines(1); }
        else { kcls(); }

        if(nonnilsymbolp(arg1))
         {
           if(eq(arg1,_COMPOUNDS_))
            {
              print_compounds(getfield(arg1,getcurrent_entry()));
	    }
           else if(keyp(arg1)) /* It's some other key */
            { arg1 = getlfield(arg1,getcurrent_entry()); }
           else if(groupnamep(arg1))
            { /* If entered $ $GROUP_NAME then show that group */
              special_screen = arg1;
              if(eq(arg1,_G_HIRAGANA_) || eq(arg1,_G_KATAKANA_))
               { print_group(arg1); return(1); }
              
              arg1 = list1(arg1); /* Make things to look like user had */
              goto groups; /* entered $ ($GROUP_NAME) and goto groups: */
            }
           else
            {
              arg1 = value(arg1);
            }
         }
        /* If entered $ $/ then show all groups: */
        else if(consp(arg1) && groupnamep(car(arg1)))
         {
           TOB fabian;
           special_screen = _T_;
groups:
           for(fabian = arg1; !nilp(fabian); fabian = cdr(fabian))
            { /* If not proper group name (e.g. single $) then skip it: */
              if(nilp(value(car(fabian)))) { continue; }
              /* Also don't print GOJUUON tables because they don't fit to
                  normal kanji window: (with 24x24 font) */
              if(eq(car(fabian),_G_HIRAGANA_) || eq(car(fabian),_G_KATAKANA_))
               { continue; }

              print_group(car(fabian));
            }
           return(1);
         }

        if(!consp(arg1)) { shlist_help(); return(-1); }

	/* This fix made at 10-SEP-89, so that
            $ fish   doesn't make "invalid arg. for car" error: */

        /* If there is keys or commands in list, then it is wrong one: */
        if(!nilp(mempred(f_keyp,arg1)) || !nilp(mempred(f_commandp,arg1)))
         { erprintf((ER,"**Ayk, ayk, saanko kalan ?\n")); return(-1); }


        printkanjis_with_keywords(arg1);

}




shlist_help()
{
        FILE *HELP_FP;

        HELP_FP = NOTICE_FP;
        w_clear(HELP_FP);
	fprintf(HELP_FP,
"Use $ command to show a list of kanjis. Examples:\n");
        fprintf(HELP_FP,
"$ (fish cat dog)     $ :E      $ $NUMBERS      $ $kata/\n");
	fprintf(HELP_FP,
"$ $/   will show all the groups except $HIRAGANA & $KATAKANA.\n");
}


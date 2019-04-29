
#include "kdinclu.h"


TOB addtofield(),delfield(),replfield();
TOB attach();



ovmain()
{
        int ovmain();
	register char *s;

        adr_of_ovmain = ovmain;

        s = (pname(cmd)+1);

/*
(#A #ADDE #AF #C (#CHPN ()) #CAR #CDR #D #DE #DF #DN (#EA () ^) #RF #SETSEP
 #SUB (#SUBCHR ()) #SUBSTR)
*/

              if(!strcmp(s,"A")) { _A(); }
         else if(!strcmp(s,"ADDE")) { _ADDE(); }
         else if(!strcmp(s,"AF")) { _AF(); }
         else if(!strcmp(s,"C")) { _C(); }
         else if(!strcmp(s,"CHPN")) { _CHPN(); }
	 else if(!strcmp(s,"CAR")) { _CAR(); }
	 else if(!strcmp(s,"CDR")) { _CDR(); }
         else if(!strcmp(s,"D")) { _D(); }
         else if(!strcmp(s,"DE")) { _DE(); }
         else if(!strcmp(s,"DF")) { _DF(); }
         else if(!strcmp(s,"DN")) { _DN(); }
         else if(!strcmp(s,"EA")) { _EA(); }
         else if(!strcmp(s,"RF")) { _RF(); }
         else if(!strcmp(s,"SETSEP"))   { _SETSEP(); }
         else if(!strcmp(s,"SUB")) { _SUBST(); }
         else if(!strcmp(s,"SUBCHR")) { _SUBCHR(); }
         else if(!strcmp(s,"SUBSTR")) { _SUBSTR(); }
	 else
          {
            erprintf((ER,"**EDITCMDS: Unknown command: %s\n",
	             pname(cmd)));
          }
}




/* Add Entry to database */
int _ADDE()
{
        if(!consp(arg1)) { arg_err(); }
        if(intp(arg2))
         {
           list_insert(list1(cons(arg1,arg1)),nivec,tob_int(arg2));
         }
        else { putlast(nivec,cons(arg1,arg1)); }
        fprintf(NOTICE_FP,"length(nivec): %d\n",length(nivec));
        changes_made = YES;
}




/* Concatenate stuff to current_entry */
int _C()
{
        if(!consp(arg1)) { arg_err(); }
        nconc(getcurperm_or_separate(),arg1);
        changes_made = YES;
}



/* Change Print Name of symbol */
int _CHPN()
{
        register char *s1;
        char s2[82];

        if(!nonnilsymbolp(arg1)) { arg_err(); }

        /* Clear the command line: */
        scr_curs(promptline,0);
        fprintf(CMD_FP,"%80s","");
        scr_curs(promptline,0);
        fprintf(CMD_FP,
"New print-name of symbol (longer name -> no change): ");
        myfgets(s2,80,stdin);
        
        s1 = pname(arg1);

        if(strlen(s2) > strlen(s1))
         {
           fprintf(NOTICE_FP,"**Cannot change %s to %s ! (%d < %d)\n",
                       s1,s2,strlen(s1),strlen(s2));
         }
        else { strcpy(s1,s2); changes_made = YES; }
        printexprnl(arg1,NOTICE_FP);
}



int _EA() /* elements add */
{
        char c;

        if(!consp(arg1)) { arg_err(); }

        /* Check ambiguities, and abort if any: */
        if(nilp(check_ambiguities(arg1)))
         {
           printexprnl(arg1,ER);
           return(-1);
         }

/*      maprplaca(getelem,arg1); Unnecessary now */

        fprintf(NOTICE_FP,"Put this list:   ");
        printexpr(arg1,NOTICE_FP);
        fprintf(NOTICE_FP,"   to :E field ? (y/n): ");

        c = toupper(scr_getc()); putc(c,NOTICE_FP);
        terpri(NOTICE_FP);
        if(c == 'Y')
         {
           replfield(_E_,getcurrent_entry(),arg1);
           changes_made = YES;
         }
        else { free_list(arg1); }
}






/* Attach one element to the front of current_entry: */
int _A()
{
        if(argcount < 1) { arg_err(); }
        attach(arg1,getcurperm_or_separate());
        changes_made = YES;
}



int _AF() /* Add to Field */
{
        if(argcount < 2) { arg_err(); }
        addtofield(arg1,getcurentry_or_separate(),
          ((argcount == 2) ? arg2 : topcopy(cdr(arglist))));
        changes_made = YES;
}


/* Delete field */
int _DF()
{
        if(argcount < 1) { arg_err(); }
        delfield(arg1,getcurentry_or_separate());
        changes_made = YES;
}



/* Replace a Field */
int _RF()
{
        if(argcount < 2) { arg_err(); }
        replfield(arg1,getcurentry_or_separate(),arg2);
        changes_made = YES;
}



int _CAR()
{
        if(nilp(separate_entry)) { separate_entry = getcurrent_entry(); }
        if(listp(car(separate_entry)))
         { separate_entry = car(separate_entry); }
}


int _CDR()
{
        if(nilp(separate_entry)) { separate_entry = getcurrent_entry(); }
        if(listp(cdr(separate_entry)))
         { separate_entry = cdr(separate_entry); }
}




/* Delete */
int _D()
{
        if(!argcount) { arg_err(); }
        deepdel(arg1,getcurperm_or_separate());
        changes_made = YES;
}


/* Delete Entry from database */
int _DE()
{
        if(!intp(arg1)) { arg_err(); }
        delnth(tob_int(arg1),nivec);
        fprintf(NOTICE_FP,"length(nivec): %d\n",length(nivec));
        changes_made = YES;
}



/* Delete Nth one */
int _DN()
{
        if(!intp(arg1)) { arg_err(); }
        delnth((tob_int(arg1)-1),getcurperm_or_separate());
        changes_made = YES;
}


/* Sets separate entry to be argument if it is list, or it's value: */
/* However, if argument is NIL , i.e. () then separate_entry is set
    to current entrypointer. This allows for example inserting new
    entries to somewhere between other entries with A (attach) command,
    not just to end as with AE command.
 */
int _SETSEP()
{
        if(argcount < 1) { arg_err(); }
        if(nilp(arg1)) { separate_entry = getentrypointer(); }
        else { separate_entry = arg1; }
}




/* Substitute arg1 with arg2 */
int _SUBST()
{
        UINT cases=0;

        if(argcount < 2) { arg_err(); }

        if(eq(arg3,_ASTERISK_))
         {
	   register TOB l;
           for(l = getentrypointer(); !nilp(l); l = cdr(l))
            {
              if(i_dsubst(arg2,arg1,car(l)))
               {
                 printexprnl(car(l),KANJI_FP); terpri(KANJI_FP);
                 changes_made = YES;
                 cases++;
               }
            }
           fprintf(NOTICE_FP,"Substituted %d cases.\n",cases);
         }
        else
         {
           if(i_dsubst(arg2,arg1,getcurperm_or_separate()))
            { changes_made = YES; }
         }
}


/* SUBCHR `c1`  `c2`  [(list of symbols)] [*] */
/* Substitutes from every symbol printname all characters c1 to c2
   (deletes c1's if c2 is 0). If list of symbols is not specified
   then _oblist is used. If there is fourth argument (which can be anything)
   then substitutions are made automatically, without asking user.
 */
int _SUBCHR()
{
        register TOB lista;
        BYTE dont_ask_flag;
        char c,c1,c2;
        char *s;
        UINT cases=0;
        char buf[165],buf2[165];

        if(!intp(arg1) || !intp(arg2))
         {
           fprintf(NOTICE_FP,
"Usage: SUBCHR c1 c2 [(list of symbols)] [*]\n");
           fprintf(NOTICE_FP,
"Where c1 and c2 are chars, inside backquotes (as `c`) or just plain integers.\n");
           fprintf(NOTICE_FP,
"If list is not specified, _oblist is used. If 4th arg, then don't ask.\n");
           return;
         }


        if(consp(arg3)) { lista = arg3; }
        else { lista = _oblist; }

        if(argcount > 3) { dont_ask_flag = 1; }
        else { dont_ask_flag = 0; }

        c1 = tob_int(arg1);
        c2 = tob_int(arg2);

        for(; !nilp(lista); lista = cdr(lista))
         {
           if(!nonnilsymbolp(car(lista)))
            {
              fprintf(NOTICE_FP,"**Skipping nonsymbol item in list: ");
              printexprnl(car(lista),NOTICE_FP);
              continue;
            }

           s = pname(car(lista));
           if(index(s,c1))
            {
              strncpy(buf2,s,163); /* Save unchanged version */
              if(dont_ask_flag) { goto do_it; }
              strncpy(buf,s,163);
              substchars(buf,c1,c2);
              fprintf(NOTICE_FP,
               "Change %s (%d) to %s (%d) ? (y/n): ",
                 s,strlen(s),buf,strlen(buf));

              c = toupper(scr_getc()); putc(c,NOTICE_FP);
              putc(' ',NOTICE_FP);
              if(c == 'Y')
               {
                 if(2 == 3) /* Really twisted code... */
                  { /* Gotoing into blocks is very nice, eh... ? */
do_it: /* Under the hood */
                    fprintf(NOTICE_FP,
                     "%s (%d) changed to %s (%d)",
                       buf2,strlen(buf2),s,strlen(s));
                  }
                 substchars(s,c1,c2);
                 cases++;
                 changes_made = YES;
               }
              terpri(NOTICE_FP);
            }
         }

        fprintf(NOTICE_FP,"Substituted %d cases.\n",cases);
}


int _SUBSTR()
{
        TOB string,stringloc;
        char newbuf[515];

        if(!(gen_stringp(arg1) && gen_stringp(arg2)))
         {
           fprintf(NOTICE_FP,
"usage: SUBSTR \"str1\" \"str2\" [:key]\n");
           fprintf(NOTICE_FP,
"Changes every str1 to str2 in the first string in the current list.\n");
           return(-1);
         }

        if(!nilp(arg3))
         {
           stringloc = NIL;
           string = getfield(arg3,getcurentry_or_separate());
         }
        else
         {
           /* Get the first string in current list: */
           stringloc = mempred(i_stringp,getcurperm_or_separate());
           string = car(stringloc);
         }

        if(!gen_stringp(string)) { return(-1); }

        /* If substitutions made: */
        if(subst_string(newbuf,pname(string),pname(arg1),pname(arg2)))
         {
           TOB new_item;

           changes_made = YES;
           /* If new string is shorter or equilength to old string,
               then copy it over the old string: */
           if(strlen(newbuf) <= strlen(pname(string)))
            { strcpy(pname(string),newbuf); }
           else /* Otherwise new string/symbol must be allocated. */
            {
              new_item =
               (stringp(string) ? stringsave(newbuf) : intern(newbuf));
              if(!nilp(arg3)) /* If used arg3 */
               { replfield(arg3,getcurentry_or_separate(),new_item); }
              else { rplaca(stringloc,new_item); }
            }
         }
}


/* Replaces field of the key with item in entry: */
TOB replfield(key,entry,item)
TOB key,entry,item;
{
        register TOB loc;
	TOB origentry;

        origentry = entry;

        if(intp(loc = getfieldloc(key)))
         {
           entry = getpermpart(entry);
/* However, if this is special entry starting with * then it doesn't
    contain any fixed location fields: */
           if(eq(car(entry),_ASTERISK_)) { goto ertzu; }
           else { loc = rawqnthcdr(tob_int(loc),entry); }
         }
        else /* It's normal variable location key which actually */
         {   /*  is there in entry. */
           loc = memq(key,entry);
           if(keyp(key)) { loc = cdr(loc); }
	 }

        if(!consp(loc))
         {
ertzu:
           erprintf((ER,"**WARNING, replfield: Can't replace field "));
           printexpr(key,ER);
           erprintf((ER," with item ")); printexpr(item,ER);
           erprintf((ER,"\nin entry ")); printexpr(origentry,ER);
           terpri(ER);
         }
        else { rplaca(loc,item); }
        return(entry);
}




TOB addtofield(key,entry,item)
TOB key,entry,item;
{
        TOB tmp;
        BYTE singleflag = 0;

        if(singlereferp(item)) { singleflag = 1; item = list1(item); }

        /* If there is something already in that field: */
        if(!nilp(tmp = getfield(key,entry)))
         {
           if(singlereferp(tmp)) /* Single item */
            {
              replfield(key,entry,cons(tmp,item));
            }
           else { nconc(tmp,item); }
         }
        else /* There is not yet that field in entry,     */
         {   /*  or if it is fixloc field then it is NIL  */
           if(singleflag) { item = car(item); } /* Uh huh nalle... */
           if(fixlockeyp(key)) { replfield(key,entry,item); }
           else { nconc(entry,clist2(key,item)); }
         }
        return(entry);
}


/* Deletes key and it's contents (= item immediately following) or flag
    from entry. If key is for fixed location field, then that field
    is just replaced with NIL.
 */
TOB delfield(key,entry)
TOB key,entry;
{
        TOB n;

        if(fixlockeyp(key))
         { return(replfield(key,entry,NIL)); }
        /* First check that there is key in entry (and get its index) */
        if(!intp(n = nthmemq(key,entry))) { return(NIL); }

        delnth(tob_int(n),entry);
        if(keyp(key)) { delnth(tob_int(n),entry); }
        return(entry);
}




int conc_query(lista1,lista2)
TOB lista1,lista2;
{
/*      char buf[82]; */
        char c;

        fprintf(NOTICE_FP,"Concatenate   ");
        printexpr(lista2,NOTICE_FP);
        fprintf(NOTICE_FP,"   to   ");
        printexpr(lista1,NOTICE_FP);
        fprintf(NOTICE_FP," ? (y/n): ");

        c = toupper(scr_getc()); putc(c,NOTICE_FP);
        terpri(NOTICE_FP);
        if(c == 'Y')
         {
           nconc(lista1,lista2);
           return(1);
         }
        else { return(0); }
}



#include "kdinclu.h"



#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


TOB memqdel();
PFI elem_get_match_function();

PFI matchfun,miau_matchfun,el_matchfun;
PFI spectestfun = NULLFP;

int el_match(),miau_match();
int deep1find(),key_or_symbfind(),and_search(),or_search();


int comfe();
int setmatch(),always_true(),ef_memq(),ef_equal();
int equal(),less_thanp(),greater_thanp();

TOB _pattern=NIL;

TOB fixed_E_field=NIL;

BYTE asterflag;
BYTE match_expanded_flag=1;

UINT negate_flag=0;


/* int _F() */ /* Find command (F F. G and G.) */
ovmain()
{
        int ovmain();

	int cases;
        int how_often_num_printed;
	TOB l,lista;
        BYTE eg_flag,hoptonextflag=0;
	char cmd_string[81];

        adr_of_ovmain = ovmain;
        separate_entry = NIL;

	cases = 0;

	if((argcount < 1) || !consp(arg1))
         { f_help(); return(-1); }

        strcpy(cmd_string,(pname(cmd)+1));
        /* If this is F. or G. command, not F  then don't examine expanded
            element lists at all:
         */
        if(LASTCHAR(cmd_string) == '.')
         {
           match_expanded_flag = 0;
           LASTCHAR(cmd_string) = '\0';
         }

        if(LASTCHAR(cmd_string) == 'G') { eg_flag = 1; }
        else
         {
           eg_flag = 0;
           if(loc_counter_flag) { loc_counter_flag = 2; }
         }

        w_clear(NOTICE_FP);
        if((argcount > 1))
         {
           how_often_num_printed = (intp(arg2) ? tob_int(arg2) : 0);
         }
        else { how_often_num_printed = 10; }

	_pattern = arg1;

        /* Print pattern for curious user: (user is very curious case) */
        terpri(NOTICE_FP); printexprnl(_pattern,NOTICE_FP);

/* If printing in screen mode and EF command, then put to upper left hand
    corner command name and argument: */
        if(!eg_flag && !rawprintflag)
         {
           kcls();
           putexpr(cmd); putexpr(topcopy(_pattern));
           knewlines(2);
         }

        /* If set_match_function failed, then return immediately: */
	if(!set_match_function()) { return(-1); }

/* If G command and this entry matches, then try to find next one: */
        if(eg_flag &&
            (((*matchfun)(_pattern,getcurrent_entry())) ^ negate_flag))
         {
           hoptonextflag = 1;
   	   lista = cdr(getentrypointer()); /* Start from next one */
           loc_counter = (getcurrent_index()+1);
         }
        else /* Otherwise search always all entries in this vector: */
         { /* Start from beginning: */
           lista = getvector(getcurrent_mode()); loc_counter = 0;
         }

	while(!nilp(lista))
	 {
           /* Print the location counter if it is right time to do it: */
           if(how_often_num_printed && !(loc_counter % how_often_num_printed))
            {
/* May be this is neat, but not so fast as the other alternative:
              w_home(NOTICE_FP); fprintf(NOTICE_FP,"%-6d",loc_counter);
 */
              scr_curs(EW_UPPEREDGE,EW_LEFTEDGE);
              fprintf(WHOLE_SCREEN,"%-6d",loc_counter);
            }
           if(((*matchfun)(_pattern,car(lista))) ^ negate_flag)
            {
              if(eg_flag) { goto archaeptoryx; }
              if(rawprintflag)
               { printkanjiandlist(car(lista)); }
              else
               {
                 putkanji_et_expr(car(lista),getkeyword(car(lista)));
               }
	      cases++;
            }
           lista = cdr(lista);
	   loc_counter++;
	 }


        w_clear(NOTICE_FP);
        if(hoptonextflag)
         {
           fprintf(NOTICE_FP,"\007**No more found, try from beginning.\n");
         }
        else
         { fprintf(NOTICE_FP,"Found %d cases.\n",cases); }

        if(!cases) /* If nothing found, then tell the main loop that it should
               show the previous screen again: (instead of this empty one) */
         { previous_entry = NIL; }
        else /* Else there is something special in screen now */
         { if(!eg_flag && !rawprintflag) { special_screen = cmd; } }


archaeptoryx:
        if(loc_counter_flag) { loc_counter_flag = 1; }
/* I think this is safe after all: (but anyway, now it is commented out) */
/*	if(consp(_pattern)) { free_list(_pattern); } */
/* Don't try to set entrypointer if didn't find anything (if lista is NIL): */
        if(eg_flag && !nilp(lista)) { chkset_entrypointer(loc_counter); }
}


int miau_match(pattern,entry)
TOB pattern,entry;
{
        return((*miau_matchfun)(pattern,entry));
}



/* Note that if there is no :E field, then
    there is surely no :EX field either.
 */

int el_match(pattern,entry)
TOB pattern,entry;
{
        TOB elements;

        if(!nilp(elements = getfield(_E_,entry)))
         {
/* Check if matches with original elements list (in :E field): */
           if((*el_matchfun)(pattern,elements)) { return(1); }
/* If didn't match with that, then check whether it matches with
    expanded elements list, which is stored into EX field in temp part: */
           else if(match_expanded_flag
                    && !nilp(elements = getfield(_EX_,entry)))
            {
              if((*el_matchfun)(pattern,elements))
               { return(1); }
            }
	 }

        return(0);
}






/* memqdel -- (Member using EQ & delete) by A.K. at 7-OCT-1989

   Checks whether atom is in lista, and if that is case, then delete it
    physically from lista (deleted node is free'ed to _freelist),
    and return T.
   Otherwise, return NIL.
   If lista is only one elem. long containing atom (which should be removed),
    then it is NOT free'ed and ZERO is returned signaling that
    there is no more elements.
   If you want this to work with COMPACT LISTS, then add some checks so that
    it doesn't try to free compact list cells. (remove some comments).
 */

TOB memqdel(atom,lista)
register TOB atom;
register TOB lista;
{
	extern TOB _freelist;
        register TOB previous,cdr_of_lista;

        previous = NIL;

        while(lista)
         {
           if(eq(car(lista),atom))
            { /* Found element (which should be removed) */

              if(nilp(previous)) /* Special case, first element removed. */
               {
	         cdr_of_lista = cdr(lista);

                 if(nilp(cdr_of_lista))
                  { /* And if lista is only one element long: */
/* Don't free it, because it would easily mess up everything:
		    rplacd(lista,_freelist);
		    _freelist = lista;
 */
                /* return ZERO indicating this very special case: */
                    return(ZERO);
                  }

		 /* Else it is longer than 1 */

                 rplaca(lista,cadr(lista));
                 rplacd(lista,cddr(lista));
 /* There should be some check for compact lists but I don't care now: */
/*		 if(!compactp(lista)) */
		  {
		    rplacd(cdr_of_lista,_freelist);
		    _freelist = cdr_of_lista;
		  }
               }
              else /* it is not first element */
               {
	         rplacd(previous,cdr(lista));
/*		 if(!compactp(previous) && !compactp(lista)) */
		  {
		    rplacd(lista,_freelist);
		    _freelist = lista;
		  }
	       }
              return(_T_);
            }

           else { previous = lista; lista = cdr(lista); }
         }
        return(NIL);
}


/* Checks whether pattern matches to set datum, and returns non-zero
    if so happens, otherwise zero.
    Normally, if pattern contains just ordinary elements, i.e.
    no wildcards or factors, it is just tested whether pattern and
    datum are equal sets (like samesetp). Note that elements can be
    in different order. If there is asterisk * in pattern, then
    there can be in datum those normal elements of pattern PLUS 
    arbitrary (including zero) amount of other elements.
   There can be as many additional elements in pattern as there is question
    marks.
   Element can be preceded with factor, i.e. positive number,
    so (3 mouth) means same as (mouth mouth mouth), although latter is
    faster to search (using equal).
   However, (3 ?) is not same as (? ? ?), because former means
    that there must be 3 SAME elements in datum, and latter
    matches to any datum whose cardinal number is three.
  */

int setmatch(pattern,datum)
/* register */ TOB pattern,datum;
{
        UINT result;
	UINT questcount,unknowns,count,factor;
        register TOB car_of_pat;
	UINT jump_back_flag;

/* Make new copy of elements, and substitute aliases: */
/* NOTE: when examining :EX field, it is unnecessary to substitute aliases,
    because that has been done at expansion phase. So invent some kludge
     for this:
 */
        datum = maprplaca(solve_alias,topcopy(datum));

	jump_back_flag = questcount = unknowns = count = factor = 0;

        if(nilp(datum)) { result = nilp(pattern); goto paluu; }
loop:

        if(nilp(pattern))
         {
	   UINT at_least,len;

       /* There must be in datum at least
          unknowns + questcount elements still left: */ 
	   at_least = unknowns + questcount;
	   len = length(datum);

        /* Asterisk allows datum to contain MORE elements than in pattern: */
	   if(asterflag)
            { if(len <  at_least) { result = FALSE; goto paluu; } }
        /* But if no asterisk, then it must be exactly: */
	   else
            { if(len != at_least) { result = FALSE; goto paluu; } }

           if(unknowns && (unknowns != comfe(datum)))
            { result = FALSE; goto paluu; }

	   result = TRUE;
           goto paluu;

         }

	car_of_pat = car(pattern);

	if(eq(car_of_pat,_QM_)) /* If Question Mark (?) */
	 { questcount++;  goto proceed; }

	if(intp(car_of_pat)) /* There is factor */
	 {
	   factor = tob_int(car_of_pat);

           pattern = cdr(pattern);
	   car_of_pat = car(pattern);

	   if(nilp(pattern)) { goto loop; }

	   if(eq(car_of_pat,_QM_))
            { unknowns = factor; goto proceed; }
	   else /* It is some normal symbol */
	    {
	      count = factor;
	      while(count)
	       {
	         count--;
              /* "Gosub" to after else (clear & beautiful ???): */
	         jump_back_flag = 1;
		 goto banaani;
jump_back_label: ;
	       }
	      jump_back_flag = 0;
	      goto proceed;
	    }
	 }


        else /* car of pattern is normal symbol */
	 {
	   register TOB zx;
banaani:
/* Check whether car_of_pat is in datum, and if it is then delete it
    from datum: */
	   zx = memqdel(car_of_pat,datum);
	   if(nilp(zx)) { result = FALSE; goto paluu; } /* Not found */

	   else if(zerop(zx))
	    { /* Found and datum ended */
	      if(unknowns || questcount || count)
               { result = FALSE; goto paluu; }
/*	      return(nilp(pattern)); */
	      result = nilp(cdr(pattern)); goto paluu; /* Fix */
            }

           /* There was such element and it was deleted, so let's continue: */

           if(jump_back_flag) { goto jump_back_label; }

          }


proceed:
	pattern = cdr(pattern);
	goto loop;
paluu:
        free_list(datum); /* Free copy of datum (what remains from it) */
        return(result);

}



/* Return the Count Of the Most Frequent Element */
/* (Palauttaa niiden atomien m{{r{n joita on kaikkein eniten listassa) */
int comfe(lista)
register TOB lista;
{
	register TOB lastu;
	register UINT count,maxcount;

	count = maxcount = 0;

	while(lista)
	 {
           lastu = lista;
	   count = 1;
           do {
                lastu = memq(car(lastu),cdr(lastu));
		if(nilp(lastu)) { break; }
		else { count++; }
	      } while(1);
	   maxcount = max(maxcount,count);
	   lista = cdr(lista);
	 }

	return(maxcount);
}

/* Check that x is not anything like (- . hiba), or like. */
int properlistp(x)
TOB x;
{
        return(consp(x) && (consp(cdr(x)) || !eq(car(x),_MINUS_)));
}


set_match_function()
{
        int properlistp();

        spectestfun = NULLFP;
alku:
        if(eq(car(_pattern),_NOT_) || eq(car(_pattern),_MINUS_))
         {
           negate_flag = 1;
           _pattern = cdr(_pattern);
           goto alku;
	 }

        if(eq(car(_pattern),_MIAU_))
         {
           _pattern = cdr(_pattern);

           if(consp(_pattern) && mempred(properlistp,_pattern))
            { spectestfun = equal; }


           if(length1p(_pattern))
            {
               _pattern = car(_pattern);
               goto jurmala;
            }
           /* If user typed   f  (@ . *whale?/)  and there is only one
               symbol containing word whale, namely whale, then
               that list is expanded as dotted pair (@ . whale)
            */
           else if(!consp(_pattern))
            {
jurmala:
              miau_matchfun = key_or_symbfind;
            }

           else if(eq(car(_pattern),_AND_))
            { _pattern = cdr(_pattern); miau_matchfun = and_search; }
           else if(eq(car(_pattern),_OR_))
            { _pattern = cdr(_pattern); goto kerala; }
           else { kerala: miau_matchfun = or_search; }

        /* matchfun = miau_match; */
           matchfun = miau_matchfun;
         }
/*
        else if(!strcmp(pname(car(_pattern)),"<>"))
         {
	   _pattern = cdr(_pattern);
           matchfun = el_match;
           el_matchfun = elem_get_match_function(_pattern);
           if((el_matchfun == setmatch) || (el_matchfun == ef_memq))
            { el_matchfun = normal_match; }
         }
 */
        else /* Normal element find */
         {
           /* Check ambiguities, and return if any: */
           if(nilp(check_ambiguities(_pattern))) { return(0); }

           /* Substitute common primitive names for alias primitives: */
	   maprplaca(solve_alias,_pattern);

           matchfun = el_match;
           el_matchfun = elem_get_match_function(_pattern);
         }

        return(1);
}


/* Returns the most economical match function for _EF()
    (i.e. fastest one for finding pattern) and checks whether
    there is asterisk in pattern.
 */
PFI elem_get_match_function(pattern)
TOB pattern;
{
	int ef_equal(),always_true(),ef_memq(),setmatch();
	int len;
	TOB zx;

	/* Check if there is asterisk in pattern, and if so then delete it */
	zx = memqdel(_ASTERISK_,pattern);
	len = length(pattern);

	if(zerop(zx)) /* pattern was (*)    (i.e. match everything) */
         { return(always_true); }
	if(nilp(zx)) /* No asterisk in pattern */
         {
           asterflag = 0;
/* If pattern is composed from one and same element (which is not ?)
    (i.e. (mouth mouth mouth)) then ef_equal will do well: */
	   if((comfe(pattern) == len) && !eq(car(pattern),_QM_))
            { return(ef_equal); }
         }
	else /* zx = _T_ */
         {
           asterflag = 1;
 /* If pattern is (elem *) or (* elem) then it is sufficient just to
    check whether elem is somewhere in datum, and ef_memq will do that: */
	   if((len == 1) && !eq(car(pattern),_QM_))
            {
              _pattern = car(pattern);
              /* The pattern list itself is not needed anymore: */
/*            free_list(pattern); */
              return(ef_memq);
            }
         }

        return(setmatch); /* Otherwise use normal setmatch */
}



int always_true(pattern,datum)
TOB pattern,datum;
{
	return(TRUE);
}


/* Checks whether element item or one of its possible aliases is in lista: */
int ef_memq(item,lista)
TOB item;
register TOB lista;
{
        while(!nilp(lista))
         {
           if(eq(item,solve_alias(car(lista)))) { return(1); }
           lista = cdr(lista);
         }
        return(0);
}



int ef_equal(pattern,datum)
TOB pattern,datum;
{
        UINT result;

/* Make new copy of elements, and substitute aliases: */
        datum = maprplaca(solve_alias,topcopy(datum));
        result = equal(pattern,datum);
        free_list(datum);
        return(result);
}


int deep1find(item,lista)
TOB item;
register TOB lista;
{
        while(!nilp(lista))
         {
           register TOB tmp;

           if((spectestfun != NULLFP) && ((*spectestfun)(lista,item)))
            { return(1); }

           /* Fix for dotted-pairs: */
           if(!consp(lista)) { tmp = lista; lista = NIL; }
           else { tmp = car(lista); }
/*
           if(equal_flag && equal(tmp,car(lista))) { return(1); }
 */
           if(atom(tmp))
            {
              if(eq(tmp,item)) { return(1); }
              if((spectestfun != NULLFP) && ((*spectestfun)(tmp,item)))
               { return(1); }
              /* Skip the :E,:EX & :EXE fields if _E_flag is not on: */
              if(!_E_flag && (eq(tmp,_E_) || eq(tmp,_EX_) || eq(tmp,_EXE_)))
               { lista = cddr(lista); continue; }
            }
           else
            { /* Don't search from tmp if it is :E-field, and _E_flag is 0: */
              if(_E_flag || !eq(tmp,fixed_E_field))
               { if(deep1find(item,tmp)) { return(1); } }
            }
           lista = cdr(lista);
         }
        return(0);
}


/* If item is key then it is just checked whether there is that field in entry
   (Non-NIL one), but if it is something other, then it is searched everywhere
   from entry, except from element-fields if _E_flag is zero.
 */
int key_or_symbfind(item,entry)
TOB item,entry;
{
        TOB result;
        UINT negateflag_=0;
        PFI savespectestfun;

        savespectestfun = spectestfun;

        if(listp(item))
         {
           TOB car_of_item;
           car_of_item = car(item);

           if(eq(car_of_item,_MINUS_))
            { negateflag_ = 1; goto occam; }
           else if(eq(car_of_item,_LT_))
            {
              spectestfun = less_thanp;
              goto occam;
            }
           else if(eq(car_of_item,_GT_))
            {
              spectestfun = greater_thanp;
occam:
              item = cdr(item);
            }
         }

        if(keyp(item))
         { result = (negateflag_ ^ !nilp(getfield(item,entry))); }
        else
         { /* Find fixed_E_field, so that deep1find can skip it if
               _E_flag is zero */
           fixed_E_field = getfield(_E_,entry);
           result = (negateflag_ ^ deep1find(item,gettemppart(entry)));
         }

        { spectestfun = savespectestfun; }
        return(result);
}



/* Returns 1 only if ALL the items in itemlist are found from entry:
 */
int and_search(itemlist,entry)
TOB itemlist,entry;
{
        while(!nilp(itemlist))
         { /* First item not found spoils the whole thing: */
           if(!key_or_symbfind(car(itemlist),entry)) { return(0); }
           itemlist = cdr(itemlist);
         }
        return(1);
}


/* Returns 1 if SOME of the items in itemlist is found from entry: */
int or_search(itemlist,entry)
TOB itemlist,entry;
{
        while(!nilp(itemlist))
         { /* First item found confirms the result: */
           if(key_or_symbfind(car(itemlist),entry)) { return(1); }
           itemlist = cdr(itemlist);
         }
        return(0);
}


int less_thanp(stuff1,stuff2)
TOB stuff1,stuff2;
{
        if(intp(stuff1))
         { return(intp(stuff2) && (tob_int(stuff1) < tob_int(stuff2))); }
        else if(gen_stringp(stuff1))
         {
           return(gen_stringp(stuff2) &&
                   (strcmp(pname(stuff1),pname(stuff2)) < 0));
         }
        return(0);
}


int greater_thanp(stuff1,stuff2)
TOB stuff1,stuff2;
{
        if(intp(stuff1))
         { return(intp(stuff2) && (tob_int(stuff1) > tob_int(stuff2))); }
        else if(gen_stringp(stuff1))
         {
           return(gen_stringp(stuff2) &&
                   (strcmp(pname(stuff1),pname(stuff2)) > 0));
         }
        return(0);
}




f_help()
{
/*         w_clear(KANJI_FP); */
           kcls();
           fprintf(KANJI_FP,"Usage:\n");
           fprintf(KANJI_FP,
"f/f./g/g. ([-] el1 el2 ...) [pause]   To find entries with those elements.\n");
           fprintf(KANJI_FP,
"f/g ([-] @ [!AND / !OR] expr1 expr2 ...) [pause]\n");
           fprintf(KANJI_FP,
"                                      To find entries which contain those\n");
           fprintf(KANJI_FP,
"                                      fields or symbols. If there is no !AND\n");
           fprintf(KANJI_FP,
"                                      or !OR specifier, then !OR is used.\n");
           fprintf(KANJI_FP,
"Examples of the search patterns:\n");
           fprintf(KANJI_FP,
" (fish *)       All entries where is fish in elements, equivalent to (* fish)\n");
           fprintf(KANJI_FP,
" (clock mouth)  Entries with elements (^clock mouth) or (mouth ^clock)\n");
           fprintf(KANJI_FP,
" (3 ?)          Entries with :E-field containing exactly three same elements\n");
           fprintf(KANJI_FP,
" (@ !or :S planet) Entries where is :S field or word planet somewhere.\n");
           fprintf(KANJI_FP,
" (@ . *school*/)   Entries which contain words which contain string school.\n");
           fprintf(KANJI_FP,
" (- @ !and !NIOL :HI) All other entries, except those which have both\n");
           fprintf(KANJI_FP,
"                      !NIOL and :HI present at the same time.\n");
           fprintf(KANJI_FP,
"The optional pause argument after pattern tells how often location counter\n");
           fprintf(KANJI_FP,
"is printed at corner of notice window. Default is 10. Use 0 to disable it.\n");
           fprintf(KANJI_FP,
"f command shows all the matching entries, g goes to the first or the next\n");
           fprintf(KANJI_FP,
"matching one. If there is dot (.) after the command, then expanded element\n");
           fprintf(KANJI_FP,
"lists (:EX-fields) are not examined.\n");
}


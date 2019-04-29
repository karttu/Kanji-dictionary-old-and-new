
#include "kdinclu.h"



/* These are just statistical debug variables for handle_S_field: */
UINT S_count=0,made_connections=0,reject_count=0;

int pass(),pass1(),pass2(),pass3();

TOB getnextfield(),getkeyfieldpairlist(),getvarfields();
TOB expand_elements(),get_JI_refer(),getrefer(),get_ptr_to_tmpfield();


FILE *initwindow(),*initkanjiwindow();
FILE *myfopen();
char *getenv();

ULI old_clistsavecnt  = 0L;
ULI old_consalloc_cnt = 0L;

int level2cnt=0;


ovmain()
{
        int ovmain();
        UINT l1,l2;
        TOB hira_a,kata_a;

        adr_of_ovmain = ovmain;

        mem_status(0);
        pass(nivec,pass1); mem_status(1);
        pass(nivec,pass2); mem_status(2);

        /* Count the difference between hiragana & katakana: */
             if(nilp(hira_a = lookup("~a"))) { hira_to_kata = 0; }
        else if(nilp(kata_a = lookup("~A"))) { hira_to_kata = 0; }
        else
         {
           hira_to_kata = (tob_int(getfield(_JI_,value(kata_a))) -
                            tob_int(getfield(_JI_,value(hira_a))));
         }
        pass(nivec,pass3); mem_status(3);
        fprintf(KANJI_FP,"level2cnt: %d\n",level2cnt);

        check_primitives();

/* ======================================================================== */

	if((l2 = length(_oblist)) != interncount)
	 {
	   fprintf(stderr,
      "**WARNING, interncount: %d   different than length of oblist: %d !\n",
                    interncount,l2);
           scr_getc();
         }
}

mem_status(pass)
int pass;
{
        extern ULI _consalloc_cnt,_clistsavecnt,_residueconscnt;

        fprintf(NOTICE_FP,(pass ? "Pass%d" : "     "),pass);
        fprintf(NOTICE_FP,
", free %6ld  clist %6ld (+ %6ld)  cons %6ld (+ %6ld)  res %5ld\n",
         lptrdiff(_uplim,_allocp),
          _clistsavecnt,
           (old_clistsavecnt ? (_clistsavecnt - old_clistsavecnt) : 0L),
          _consalloc_cnt,
           (old_consalloc_cnt ? (_consalloc_cnt - old_consalloc_cnt) : 0L),
            _residueconscnt);

        old_clistsavecnt  = _clistsavecnt;
        old_consalloc_cnt = _consalloc_cnt;
}


/* General 'pass-function'. Pass is made through vec (usually nivec)
    and for every field of every entry is fun called (which is pass1,
    pass2 or pass3)
 */
pass(vec,fun)
TOB vec;
PFI fun;
{
        TOB getnextfield();
        TOB entry,origl,key;
	register TOB l;

        for(; !nilp(vec); vec = cdr(vec))
         {
           entry = car(vec);
/* If entry is special entry (starting with *) then skip it, unless
    it is equ entry (starting like (* = $groupname etc..)), and is
    pass 3 going on, then make :G links to temporary part of those
    kanjis which are listed in groups:
 */
           if(specentryp(entry))
            {
              if((fun == pass3) && equentryp(entry))
               { /* Entry is of the form (* = $SYMBOL (list of something)) */
                 TOB name;

                 setvalue((name = getgroup_name(entry)),entry);
                 handle_G(name);
               }
            }
           else
            {
              origl = l = getpermpart(entry);
	      while(!nilp(l))
               {
                 l = getnextfield(&key,l,origl);
	         if(!nilp(car(l))) { ((*fun)(entry,key,car(l))); }
  	         l = cdr(l);
               }
            }
         }
}


TOB getnextfield(ptr_to_key,entry,origentry)
TOB *ptr_to_key;
TOB entry,origentry;
{
      int i;

      i = clist_diff(entry,origentry);
      if(i <= E_LOC)
       {
         *ptr_to_key = qnth(i,_FIXED_FIELDS_);
	 return(entry);
       }
      else
       {
         *ptr_to_key = car(entry);
	 return(keyp(*ptr_to_key) ? cdr(entry) : entry);
       }
}

/*
TOB getkeyfieldpairlist(keys,entry)
TOB keys,entry;
{
        TOB getvarkeys();
	TOB key,tmp,result,varkeys;

        while(!nilp(keys))
         {
           key = car(keys);
           if(fixlockeyp(key))
            {
              if(!nilp(tmp = getfield(key,entry)))
               {
                 result = cons(cons(key,tmp),result);
               }
            }
           else
            { varkeys = cons(key,varkeys); }
           key = cdr(keys);
         }

        return(nconc(result,getvarfields(varkeys,entry)));
}




TOB getvarfields(keys,entry)
TOB keys;
register TOB entry;
{
        TOB result;

        result = NIL;

        while(!nilp(entry))
         {
           if(keyp(car(entry)) && !nilp(memq(car(entry),keys)))
            {
              result = cons(cons(car(entry),cadr(entry)),result);
	      entry = cdr(entry);
            }
           entry = cdr(entry);
         }

        return(result);
}
*/

#define brake goto seitikki


pass1(entry,key,field)
TOB entry,key,field;
{
/*
    switch(key)
     {
 */

       if(eq(key,_HK_)) { setvalue(field,entry); brake; }
       else if(eq(key,_HI_) || eq(key,_JI_))
        {
          TOB ptr,tmp;
          int num;

/*        if(eq(field,_QM_)) { brake; } */ /* Ignore the ?-mark */
          if(!intp(field))
           {
             fprintf(stderr,
              "\nERROR in database: No numeric index after key: ");
             printexpr(key,stderr);
             fprintf(stderr,"\nin entry: ");
             printexpr(entry,stderr);
             fprintf(stderr,"\n");
  	     return;
           }
          num = tob_int(field);
          if(eq(key,_JI_) && (num > JI_UPLIM))
           {
             level2cnt++;
             brake;
           }
          ptr = get_ptr_to_nth_entry(num,key);
          /* Check that index is not out of bounds: */
          if(nilp(ptr))
           {
             fprintf(stderr,
              "\nERROR in database: Index is out of bounds: %d\n",num);
             fprintf(stderr,"key: %s   entry: ",pname(key));
             printexprnl(entry,stderr);
  	     return;
           }
/* Check that there is no multiple entries in database with same index: */
          if(!nilp(tmp = car(ptr)))
           {
             fprintf(stderr,
              "\nERROR in database: %s<%d> is occupied already:\n",
	          pname(key),num);
             printexpr(tmp,stderr);
             fprintf(stderr,"\nTried to replace with: ");
             printexprnl(entry,stderr);
   	     return;
           }
          /* However, if everything is OK, then do what is needed:
            (put that entry to index vector to its proper place) */
          rplaca(ptr,entry);
          brake;
        }
/*
       default: { brake; }
     }
 */
seitikki: ;
}


pass2(entry,key,field)
TOB entry,key,field;
{
/*
    switch(key)
     {
 */
          

       if(eq(key,_NK_) || eq(key,_R_) || eq(key,_OTHER_) ||
             eq(key,_HIRAGANA_) || eq(key,_KATAKANA_))
        { assign_values(field,entry); brake; }
       else if(eq(key,_COMPOUNDS_))
        {
          TOB compounds;

          compounds = getfield(_COMPOUNDS_,entry);
          for(; !nilp(compounds); compounds = cdr(compounds))
           {
             assign_values(getcompound_meanings(car(compounds)),entry);
             assign_values(getcompound_readings(car(compounds)),entry);
           }
          brake;
        }
       else if(eq(key,_P_))
        {
          TOB sym;
          register TOB primlist;

          if(!consp(field)) { field = rplaca(sparecons,field); }

          primlist = field;
          /* Check whether there is :HK field in this entry
              which could be used as "alias-root" where primitive names
              is assigned to. */
	  if(nilp(sym = getfield(_HK_,entry)))
 	   { /* If there is no HK-keyword then first primitivename
                 has to be "alias-root": */
 	     sym = car(primlist);
 	     setvalue(sym,entry);
	     primlist = cdr(primlist);
           }
/*	  else { primlist = field; } */

          for(; !nilp(primlist); primlist = cdr(primlist))
           {
             if(!primitivep(car(primlist)))
  	      { invalid_entry(entry); brake; }
             setvalue(car(primlist),sym);
           }

          brake;
        }
/*
       default: { brake; }
     }
 */
seitikki: ;
}


pass3(entry,key,field)
TOB entry,key,field;
{
/*
    switch(key)
     {
 */
       if(eq(key,_E_)) /* Expand element lists */
        {
          TOB expanded;

           if(!nilp(getfield(_DONT_EXPAND_,entry))) { brake; }
           expanded = expand_elements(field);
/* Currently expanded elements list is put to :EX field of temppart 
    only if it is LONGER than original elements list. This is checked
     because memory space is limited.
   And compact list version is made from expanded, because that takes
    less memory. (Original expanded is free'ed after that).
 */
           if(length(expanded) > length(field))
            {
              addtotemppart(entry,clist2(_EX_,ctopcopy(expanded)));
	    }
	   free_list(expanded); /* Don't let it waste memory */
	   brake;
	}
       else if(eq(key,_S_)) /* Make :S connections */
        {
          if(singlereferp(field)) { field = rplaca(sparecons,field); }
          handle_S_fields(entry,field);
	  brake;
        }
       /* Make :SEE connections between HIRAGANA <-> KATAKANA */
       else if(eq(key,_HIRAGANA_) || eq(key,_KATAKANA_))
        {
          int tolower(),toupper();
          TOB x;
          char buf[82];

          strncpy(buf,pname(field),80); /* Get the copy of print name */
          /* If hiragana then make kana name uppercase (to kata),
             but if katakana then make kana name lowercase (to hira): */
          convert_string(buf,(eq(key,_HIRAGANA_) ? toupper : tolower));
          /* If not found corresponding kana name from other set, then
              break. For example there is no character in hiragana
              corresponding to ~V (U + nigori) in katakana: */
          if(nilp(x = lookup(buf))) { brake; }
          /* If this is unique kana-name, then use it as reference,
             but if it is not (e.g. ~ji, ~zu, etc.), then
             take also JI-index to make reference unambiguous: */
          if(!single_entryp(value(x)))
           {
             int lisma;
             lisma = tob_int(getfield(_JI_,entry));
             /* If this is hiragana, then add hira_to_kata to ji_index
                 of this to get katakana ji-index. But if this is
                 katakana, then subtract hira_to_kata to get hiragana
                 ji_index: */
             x = cons(x,
              int_tob(-(lisma + (eq(key,_HIRAGANA_) ? hira_to_kata
                                                    : -hira_to_kata))));
             /* Not too hard expression for you, eh ? */
           }
          addtotempfield(_SEE_,entry,x);
          brake;
        }

/*
       default: { brake; }
     }
 */
seitikki: ;
}



assign_values(field,entry)
TOB field,entry;
{
        if(nilp(field)) { return; }
        if(!listp(field)) { addtovalues(field,entry); }
        else
         {
           assign_values(car(field),entry);
           assign_values(cdr(field),entry);
         }
}


addtovalues(symbol,entry)
TOB symbol,entry;
{
        register TOB previous_values,workvar;

        if(nilp(previous_values = value(symbol)))
         { /* If no value set to symbol before this */
           setvalue(symbol,entry);
         }
        else if(single_entryp(previous_values))
         { /* If only one value set before this */
           /* Don't put same stuff twice to values: */
           if(eq(previous_values,entry)) { return; }
           /* Allocate compact list of length MULT_VAL_CONST.
	       All elements will be initially NIL: */
           workvar = new_clist(MULT_VAL_CONST);
	   rplaca(workvar,previous_values);
	   rplacx(1,workvar,entry);
           setvalue(symbol,workvar);
         }
        else /* There is at least two values already */
         { /* Don't add entry to values if it is there already: */
           if(!nilp(memq(entry,previous_values))) { return; }
           /* If there is still empty locations in this compact list: */
           if(!nilp(workvar = memq(NIL,previous_values)))
            { /* Put new value into first empty location (which is NIL): */
              rplaca(workvar,entry);
            }
           else /* Concatenate to the end of previous values: */
            { nconc(previous_values,list1(entry)); }
         }
}



int invalid_entry(entry)
TOB entry;
{
        fprintf(stderr,"\n**INVALID ENTRY: ");
	printexpr(entry,stderr); terpri(stderr);
	return(1);
}



/* If original element list contains elements which are kanjis which
    themselves are constructed from smaller elements, then expand_elements
    returns longer element list composed of basic elements only.
    ("compound" elements are decomposed recursively.)
 */

TOB expand_elements(sym)
TOB sym;
{
        TOB els,tmp;

        if(nilp(sym)) { return(NIL); }
        else if(nonnilsymbolp(sym))
         {
	   tmp = value(sym = solve_alias(sym));
/* Choose HK-entry (first one) if there is more than one in value: */
           if(!single_entryp(tmp)) { tmp = car(tmp); }
 	   /* If this keyword or primitive-name has no elements, then it is
	       naturally a leaf of the element-tree. (i.e. basic-element)
              However, if there is flag !DONT_EXPAND in entry, then it is
               understood as leaf even if there are some elements.
            */
/* If there is :EXE field in entry then use that for expanding instead
    of ordinary :E field. (For those kanjis like tall & old_man which
    have different elements when part of another kanji than in single case).
   First however check that there is no !DONT_EXPAND flag in entry:
 */
           if(nilp(getfield(_DONT_EXPAND_,tmp)) &&
                (!nilp(els = getfield(_EXE_,tmp)) ||
                   (!nilp(els = getfield(_E_,tmp)))))
            { }
           else { return(list1(sym)); }
/*
   	   if((nilp(els = getfield(_E_,tmp))) ||
               !nilp(getfield(_DONT_EXPAND_,tmp)))
            { return(list1(sym)); }
 */
         }
        else if(consp(sym)) { els = sym; }
	else
	 {
           fprintf(stderr,"\n**expand_elements: sym is of wrong type: ");
	   printexpr(sym,stderr);
	   terpri(stderr);
	   return(NIL);
	 }

        return(nconc(expand_elements(car(els)),
	              expand_elements(cdr(els))));

}


handle_G(groupname)
TOB groupname;
{
        TOB referlist,entry,old_field,new_field;

        if(!groupnamep(groupname))
         {
           fprintf(stderr,
            "**handle_G: groupname is not legal groupname: ");
           eprint(groupname);
           return;
         }

/* Currently do nothing for compoundgroups: */
        if(compoundgroupnamep(groupname)) { return; }

        for(referlist = getgroup_reflist(value(groupname)); !nilp(referlist);
              referlist = cdr(referlist))
         {
           if(specialreferp(car(referlist))) { continue; }

           /* If reference points to something nonexisting then skip it: */
           if(nilp(entry = get_entry(car(referlist)))) { continue; }

           addtotempfield(_G_,entry,groupname);
         }
}


/* This adds item to field in temporary part of the entry. If there is not
    yet that field in temporary part, it is created there.
 */
addtotempfield(key,entry,item)
TOB key,entry,item;
{
         TOB old_field_loc,old_field,new_field;

         old_field = getlfield(key,entry);

         if(!nilp(memq(item,old_field)))
          {
            fprintf(stderr,"\n**addtotempfield: Warning, entry:  ");
            eprint(gettemppart(entry));
            fprintf(stderr,"already contains ");
            printexpr(key,stderr);
            fprintf(stderr,"-reference:   ");
            eprint(item);
          }

/* If there was no key-field already, then new field is just single
    reference, not list. Otherwise, cons new reference to the front of
     old references: (So key-field of permanent part doesn't get changed.)
 */
         new_field = (nilp(old_field) ? item : cons(item,old_field));

         if(nilp(old_field_loc = get_ptr_to_tmpfield(key,entry)))
          { /* There is no key-field in temp part yet */
            addtotemppart(entry,clist2(key,new_field));
          } /* Else just replace old field contents with new: */
         else { rplaca(old_field_loc,new_field); }
}



handle_S_fields(entry,S_field)
TOB entry,S_field;
{
        TOB other_entry,refertothis,loc,new_field,other_S_field;

        /* Get reference to this entry */
        refertothis = getrefer(entry);

        /* Go through S-references: */
        for(; !nilp(S_field) ; S_field = cdr(S_field))
         { /* Get entry which is pointed by this reference: */
           other_entry = chkget_entry(car(S_field));

           /* Don't do "connecting" if this entry is already
               referenced by other entry's :E or :SEE fields: */
           if(!refby_fieldp(_E_,other_entry,entry) &&
               !refby_fieldp(_SEE_,other_entry,entry))
            { made_connections++;

              addtotempfield(_S_,other_entry,refertothis);
            }
           else { reject_count++; }
         }

}


/* Construct reference which references to this entry: */
TOB getrefer(entry)
TOB entry;
{
        register TOB x,y;

        y = get_JI_refer(entry);

             if(nonnilsymbolp(x = getfield(_HK_,entry))) { return(x); }
        else if(intp(x = getfield(_NI_,entry))) { return(x); }
        else if(nonnilsymbolp(x = getfield(_HIRAGANA_,entry)) ||
                nonnilsymbolp(x = getfield(_KATAKANA_,entry)))
         { /* If this is unique kana-name, then use it as reference: */
           if(single_entryp(value(x))) { return(x); }
           /* However, if it is not unique (e.g. ~ji, ~zu, etc.), then
               take also JI-index to make reference unambiguous: */
/*         else { return(cons(x,y)); } */
/* It suffices to return only JI-index:
   (i.e. -287 is enough, no (~QU . -287) is needed) */
           else { return(y); }
         }
        else if(!nilp(x = getfield(_P_,entry))) { return(get_first_word(x)); }
/*       { return(listp(x) ? car(x) : x); } */
        /* If there is OTHER field in entry, and keyword of that is
            unambiguous (it has only one entry as value), then use
            that as reference: */
        else if(!nilp(x = get_first_word(getfield(_OTHER_,entry))) &&
                  single_entryp(value(x)))
         { return(x); }
/* Use JI-index as last change: (because, for instance, JI-references
         of level2 jis don't work properly. */
        else { return(y); }
/*
        else if(!nilp(y)) { return(y); }
        else { return(NIL); }
 */
}

TOB get_JI_refer(entry)
TOB entry;
{
        if(intp(entry = getfield(_JI_,entry)))
         {
           return(int_tob(-(tob_int(entry))));
         }
        else return(NIL);
}



/* Returns "pointer" to location in the temporary part of entry where
    is field corresponding to key. (So you must do car for result to
    get field's contents.) If there is no that field in entry,
    (or it is only in permanent part) then NIL is returned.
 */
TOB get_ptr_to_tmpfield(key,entry)
TOB key;
register TOB entry;
{
        TOB orig_entry,stopper;

        orig_entry = entry;
        stopper = getpermpart(entry); /* Stop searching when found this. */
        entry =   gettemppart(entry);

        while(!nilp(entry) && !eq(entry,stopper))
         {
           if(eq(car(entry),key)) { return(cdr(entry)); }
           entry = cdr(entry);
         }

        if(nilp(entry))
         {
           fprintf(stderr,
"\n**Curious bug in get_ptr_to_tmpfield: entry is NIL ! key: %s   stopper: ",
             pname(key));
           eprint(stopper);
           fprintf(stderr,"entry was: "); eprint(orig_entry);
           return(NIL);
         }
        return(NIL);
}


/* If in field of other_entry is reference to entry, then return 1,
    otherwise 0.
 */
int refby_fieldp(field,other_entry,entry)
TOB field,other_entry,entry;
{
        TOB origfield;
        int result=0;

        if(nilp(origfield = field = getlfield(field,other_entry)))
         { return(0); }

        while(!nilp(field))
         {
           if(!eq(car(field),_SOMETHING_) &&
                eq(chkget_entry(car(field)),entry)) { result = 1; break; }
           field = cdr(field);
         }

/* If getlfield constructed list of one item, then free it: */
        if(length1p(origfield)) { free_list(origfield); }
        return(result);
}



/* This function just checks that every symbol starting
    with ^ (i.e. primitives) has some non-nil value, so that there
     doesn't hang around any "ghost-primitives". (They should be eradicated)
 */
check_primitives()
{
        int streq();
        TOB orig,l;

        /* Get all symbols starting with caret (^): */
        orig = l = findsymbols("^",streq);
        fprintf(NOTICE_FP,"%d primitives.\n",length(l));

        while(!nilp(l))
         {
           if(!eq(car(l),_CARET_) && nilp(value(car(l))))
            {
              printf("Warning: primitive %s has no value !\n",pname(car(l)));
            }
           l = cdr(l);
         }

        free_list(orig);
}

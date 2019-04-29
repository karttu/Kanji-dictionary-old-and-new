

#include "kdinclu.h"


FILE *initwindow(),*initkanjiwindow();
FILE *myfopen();
char *getenv();

ovmain()
{
        int ovmain();


        int _NULL(),_RETURN(),_W();

        ULI atol();
        UINT l1,l2;
	char *s,*s_f16,*s_f24;
        ULI maxmem=270000;

        adr_of_ovmain = ovmain;

        /* Assign these to stderr until own windows are opened for both: */
        KANJI_FP = ERROR_FP = stderr;

	if(!(s = getenv("MAXMEM")) || !(maxmem = atol(s)))
	 {
	   fprintf(stderr,
"\nEnvironment variable MAXMEM must have numerical value above zero\n");
	   myexit(1);
	 }


	if(!(s = getenv("MAXENTRIES")) || !(maxentries = atoi(s)))
	 {
	   fprintf(stderr,
"\nEnvironment variable MAXENTRIES must have numerical value above zero\n");
	   myexit(1);
	 }

	if((s = getenv("MAXVKSENTRIES")) && isdigit(*s))
	 {
           extern int vks_entries;

	   vks_entries = atoi(s);
	 }


	if((s = getenv("FILELINES_IN_VKS")) && isdigit(*s))
	 {
	   maxfilelines_in_vks = atoi(s);
	 }

        if((s = getenv("PLIST")) && (*s != '0')) { plist_flag = 1; }
	else { plist_flag = 0; }


        if(s = getenv("CURSORPAUSE")) { setcursorpause(atoi(s)); }
        else { setcursorpause(DEFAULTCURSORPAUSE); }

        initlists(maxmem);

        setkdmode();

        nivec = new_clist(maxentries+1);
        hivec = new_clist(HI_UPLIM+1);
	jivec = new_clist(JI_UPLIM+1);

        initsymbols();

        sparecons = list1(NIL);


        s_f16 = "FONT16";
        s_f24 = "FONT24";

        if(!((s = getenv(s_f24)) && setfontfile(string_tob(s_f24))))
         {
           if(!((s = getenv(s_f16)) && setfontfile(string_tob(s_f16))))
            {
              fprintf(stderr,
"Environ. variables FONT16 or FONT24 must be assigned to existing fontfiles !\n"
                );
              myexit(1);
            }
         }

        initcommands(
         "WR",_W,
         ";",_NULL,
         ".",_RETURN,
         NULL);

/*
        _DINS_CMDS_         = sreadexpr("(#, #&)");
 */
        init_external_cmds();


        l1 = length(_oblist);
	fprintf(stderr,
         "length(_oblist): %d\n",
           l1);
	readtovec(myfopen(getenv("DATABASE"),"r"),maxentries);
	l2 = length(_oblist);
	fprintf(stderr,
         "length(_oblist): %d    difference: %d\n",
           l2,(l2-l1));

        fprintf(stderr,"length(nivec): %d.\n",
          length(nivec));

        rplacx(1,value(_NI_),int_tob(readentries));

        { TOB veba;
          veba = clist3(_ASTERISK_,intern("HEISIG-INDEXING-MODE"),_ASTERISK_);
          rplacx(0,hivec,cons(veba,veba));
        }


/* Make final I/O configurations: */

  /* Set those prefix characters which make symbol uppercase when read in: */
        _TOUPPER_PREFIXES_ = intern("*TOUPPER_PREFIXES*");
        setvalue(_TOUPPER_PREFIXES_,
           listn(char_tob(':'),char_tob('#'),
                  char_tob('!'),char_tob('$'),ENDMARK));

        abrevchar    = '/';
        getvaluechar = ',';

        switch_compact_read(0); /* Read lists in normal mode */

        INITHR; /* Initialize HiRes Graphics in CGA */

        KANJI_FP =
         initkanjiwindow(KW_UPPEREDGE,KW_HEIGTH,KW_LEFTEDGE,KW_WIDTH);
        assign_to_window(stdout,KANJI_FP);
        assign_to_window(stderr,KANJI_FP);
        promptline = PROMPTLINE;
/*      CMD_FP = initwindow(promptline,1,0,80); */
        CMD_FP = myfopen("CON","w");
        ERROR_FP =
         initwindow(EW_UPPEREDGE,EW_HEIGTH,EW_LEFTEDGE,EW_WIDTH);
        sidewindow = initwindow(0,25,0,2);

}


initcommands(args)
PTR args;
{
        register char *argptr;
        TOB tmp;
        char s[41];
        int count=0;

        argptr = ((char *) &args);

        while((*((char **)argptr)) != NULL)
         {
           sprintf(s,"#%s",*((char **)argptr));
           argptr += sizeof(char *);
           tmp = intern(s);
           setfvalue(tmp,*((PFI *) (argptr)));
           argptr += sizeof(PFI);
           count++;
         }

        fprintf(stderr,"initcommands: %d commands initialized.\n",count);
}


initsymbols()
{
/* Keys: */
        _JI_    = intern(":JI");
        _HI_    = intern(":HI");
        _HK_    = intern(":HK");
        _NI_    = intern(":NI");
        _NK_    = intern(":NK");
        _E_     = intern(":E");
        _EX_    = intern(":EX");
        _EXE_   = intern(":EXE");
        _R_     = intern(":R");
        _S_     = intern(":S");
        _P_     = intern(":P");
        _G_     = intern(":G");
        _SEE_   = intern(":SEE");
        _STR_   = intern(":STR");
        _OTHER_ = intern(":OTHER");
        _COMMENT_   = intern(":COMMENT");
        _COMPOUNDS_ = intern(":COMPOUNDS");
        _HIRAGANA_  = intern(":HIRAGANA");
        _KATAKANA_  = intern(":KATAKANA");


        _G_HIRAGANA_ = intern("$HIRAGANA");
        _G_KATAKANA_ = intern("$KATAKANA");
/* Flags: */
        _DONT_EXPAND_   = intern("!DONT_EXPAND");
        _NIOL_          = intern("!NIOL");
        _SOMETHING_     = intern("!SOMETHING");
        _PAIR_          = intern("!PAIR");

        _NOT_		= intern("!NOT");
	_AND_		= intern("!AND");
	_OR_		= intern("!OR");

/* Special character symbols: */
        _QM_	    = intern("?");
        _ASTERISK_  = intern("*");
        _EQ_        = intern("=");
	_PLUS_      = intern("+");
	_MINUS_	    = intern("-");
	_SLASH_     = intern("/");
        _DOLLAR_    = intern("$");
        _COMMA_     = intern(",");
        _SEMICOLON_ = intern(";");
	_MIAU_	    = intern("@");
        _CARET_     = intern("^");
        _LT_        = intern("<");
        _GT_        = intern(">");


        _FIXED_FIELDS_ =
         clistn(_JI_,_HI_,_HK_,_NI_,_NK_,_E_,ENDMARK);

        setvalue(_JI_,
         clist4(int_tob(JI_LOC),int_tob(JI_UPLIM),
                  stringsave("Jis index:"),jivec));
        setvalue(_HI_,
         clist4(int_tob(HI_LOC),int_tob(HI_UPLIM),
                  stringsave("Heisig:"),hivec));
        setvalue(_NI_,
         clist4(int_tob(NI_LOC),int_tob(NI_UPLIM),
                  stringsave("NDKU:"),nivec));

        setvalue(_HK_,list1(int_tob(HK_LOC)));
        setvalue(_NK_,list1(int_tob(NK_LOC)));
        setvalue(_E_,
         clist3(int_tob(E_LOC),NIL,stringsave("Elements:")));
        setvalue(_S_,
         clist3(NIL,NIL,stringsave("Similars:")));
/*
        setvalue(_R_,
         clist3(NIL,NIL,stringsave("Readings:")));
 */
        setvalue(_SEE_,
         clist3(NIL,NIL,stringsave("See also:")));
        setvalue(_OTHER_,
         clist3(NIL,NIL,stringsave("Other:")));
        setvalue(_COMMENT_,
         clist3(NIL,NIL,stringsave("Comment:")));
        setvalue(_COMPOUNDS_,
         clist3(NIL,NIL,stringsave("Compounds:")));
        setvalue(_P_,
         clist3(NIL,NIL,stringsave("Primitive names:")));
        setvalue(_G_,
         clist3(NIL,NIL,stringsave("Groups:")));
        setvalue(_HIRAGANA_,
         clist3(NIL,NIL,stringsave("Hiragana:")));
        setvalue(_KATAKANA_,
         clist3(NIL,NIL,stringsave("Katakana:")));
}



/* ======================================================================== */


static int extcmdcnt=0;


init_external_cmds()
{
        char *scdir();
        TOB path;
        int extcmdovlfilecnt=0;
        char c,*namefilename,*s;
        char search_pattern[201];
        BYTE debug_flag=0;

        if(s = getenv("OVLPATH")) { path = sreadexpr(s); }
        else { path = NIL; }

        if(!listp(path))
         {
           fprintf(stderr,
"**init_external_cmds: ENVIRONMENT VARIABLE OVLPATH SHOULD BE A LIST, NOT:\n");           eprint(path);
           path = NIL; /* Etsi nyt silti edes lokaali dirikka */
         }

        /* NIL in beginning stands for local directory: */
        path = cons(NIL,path);

        for(; !nilp(path) ; path = cdr(path))
         {
           if(nilp(car(path))) { *search_pattern = '\0'; }
           else if(gen_stringp(car(path)))
            { strncpy(search_pattern,pname(car(path)),190); }
           else
            {
              debug_flag = 1;
              fprintf(stderr,
"**init_external_cmds: INVALID DIRECTORY IN ENVIRONMENT VARIABLE OVLPATH:\n");
              eprint(car(path));
              continue;
            }

/* If last char is something else than '\0', backslash, colon or slash, then
    add backslash: */
           if(*search_pattern &&
              ((c = LASTCHAR(search_pattern)) != '\\') && 
               (c != ':') && (c != '/'))
            { strcat(search_pattern,"\\"); }

           strcat(search_pattern,"*.kdx");

           if(debug_flag)
            {
               fprintf(stderr,"search_pattern: %d/%s\n",
                 strlen(search_pattern),search_pattern);
            }

           /* Search all files with KDX extension, and do the job for them: */
           while(namefilename = scdir(search_pattern))
            {
              if(debug_flag)
               {
                 fprintf(stderr,"search_pattern: %d/%s    ",
                   strlen(search_pattern),search_pattern);
                 fprintf(stderr,"namefilename: %d/%s\n",
                   strlen(namefilename),namefilename);
               }
              assign_names_of_xcmds(namefilename);
              extcmdovlfilecnt++;
            }

         }

        fprintf(stderr,
         "%d external commands in %d overlay files.\n",
            extcmdcnt,extcmdovlfilecnt);

}



/* Read the list of command names from KDX-file and
    assign them to overlayfile name: (i.e. name without any extension) */
assign_names_of_xcmds(namefilename)
char *namefilename;
{
        TOB pair_with_nils();
        FILE *imu,*myfopen();
        char *index(),*rindex();
        int toupper(),tolower();
        TOB lista,sym,origlista,commandname,orig_commandname;
	char *s,buf[202];

        imu = myfopen(namefilename,"r");
        /* Read command lists in normal mode so that they can be free'ed: */
        switch_compact_read(0);
        origlista = lista = readexpr(imu);
        fclose(imu);
        switch_compact_read(1); /* Switch compact read mode back */

        if(!consp(lista)) /* Should be a list (of command names) */
         {
           fprintf(stderr,"\n**Contents of file %s are invalid: ",
            namefilename);
           eprint(lista);
           return(0);
         }

        /* Precede the filename with ? and make it uppercase: */
        strncpy((buf+1),namefilename,199);
        *buf = '?';
        convert_string(buf,toupper);

        /* Zero the dot (i.e. remove the extension): */
        if(s = rindex(buf,'.')) { *s = '\0'; }
/*      (*(rindex(buf,'.'))) = '\0'; */

        sym = intern(buf); /* Make symbol with that printname */

        while(lista)
         {
           orig_commandname = NIL;
           commandname = car(lista);
           if(consp(commandname))
            { /* There is some qualifiers with command name */
              orig_commandname = commandname;
              commandname = car(commandname);
            }

           if(!commandp(commandname))/* Every commandname must start with # */
            {
              fprintf(stderr,
               "\n**File %s contains an illegal command name: ",
                    namefilename);
              eprint(nilp(orig_commandname) ? commandname : orig_commandname);
            }
           else if(!nilp(value(commandname)))
            {
              fprintf(stderr,
        "\n**File %s contains command name which is used already: %s\n",
               namefilename,pname(commandname));
              fprintf(stderr,"Value of it: ");
              eprint(value(commandname));
            }
           else
            {
              setvalue(commandname,sym); extcmdcnt++;
              if(!nilp(orig_commandname))
               { /* There was some qualifiers accompanying command name. */
                 orig_commandname = cdr(orig_commandname);
                 if(!consp(orig_commandname))
                  { orig_commandname = rplaca(sparecons,orig_commandname); }

                 /* Handle those qualifiers.
                  ^    =  read args of command with kd_prefix_char set to '^'
                  NIL  =  read arguments of command in DINS-mode.
                  */
                 while(!nilp(orig_commandname))
                  {
                    if(eq(car(orig_commandname),_CARET_))
                     {
                       _USE_PRIMPREF_CMDS_ =
                          cons(commandname,_USE_PRIMPREF_CMDS_);
                     }
                    else if(nilp(car(orig_commandname)))
                     {
                       _DINS_CMDS_ = cons(commandname,_DINS_CMDS_);
                     }
                    orig_commandname = cdr(orig_commandname);
                  }
               }
            }
           lista = cdr(lista);
         }

/*
        free_list(origlista);
 */
        setvalue(sym,pair_with_nils(origlista));
}


/* Currently handles only normal lists: */
TOB pair_with_nils(lista)
TOB lista;
{
        TOB orig_lista,next;

        orig_lista = lista;

        while(!nilp(lista))
         {
           next = cdr(lista);
           rplacd(lista,cons(NIL,next));
           lista = next;
         }

        return(orig_lista);
}



/* Read database file to nivec: */

readtovec(fp,n)
FILE *fp;
int n;
{
	TOB readexpr();
	TOB vec,entry;
	int i;

/*	vec = new_clist(n+1); */
        vec = nivec;
	i = 0;
	entry = NIL;

/*      rplacx(0,vec,NIL); Unnecessary */

        while(!endmarkp(entry = readexpr(fp)))
         {
	   if(i > n)
	    {
	      fprintf(stderr,
 "**readtovec: database contains more than %d entries, truncated before:\n",
	        n);
	      print(entry);
	      goto loppu;
            }
/* car points to permanent part, and cdr to temporary + permanent part,
   but now they are still same because nothing is yet added to temporary part:
 */
	   rplacx(i,vec,cons(entry,entry));
	   i++;
         }

        fprintf(stderr,"readvec: read %d entries\n",i-1);
loppu:
	/* Clear compact bit of last entry, so that after it vec ends: */
	rplacc(qnthcdr((i-1),vec),0);
	rplacc(qnthcdr(i,vec),0); /* Clear C-bit of following one too */
        readentries = (i-1);
	return(vec);
}


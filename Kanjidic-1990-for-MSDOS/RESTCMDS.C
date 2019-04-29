

#include "kdinclu.h"


int _Q(),_DEB();
int _help();
int _SETREP();
int _TOGGLEFLAGS(),_SETCURSORPAUSE(),_SETCOLUMNS(),_FONT(),_DOUBLE();
int _SET(),_AMPERSAND();



ovmain()
{
        int ovmain();
        PFI fun;

        adr_of_ovmain = ovmain;
/*
        if(!(fun = search_command(pname(cmd)+1)))
         {
           fprintf(ERROR_FP,
"**MODULE RESTCMDS: Unknown command: %s !\n",s);
           return(-1);
         }
 */
        /* If command name & address list hasn't been initialized yet,
            (first time in this module), then initialize it: */
        if(nilp(cadr(value(value(cmd)))))
         { set_command_adrs(); }

        if(!(fun = get_command_adr(cmd))) { /* Do nothing */ }
        else { ((*fun)()); }
}


/*
struct kinnas
 {
   char *cmd_name;
   PFI  cmd_adr;
 };
 */


/* This scheme doesn't work with @#$%^! fucking overlays !
   Linker says something like: "segment fixups allowed only on EXE files".
   That's because absolute addresses of those functions in table cannot be
   known at that stage.
 */
/*
struct kinnas cmd_name_table[] =
 {
        { "Q",_Q },
        { "NE",_NE },
	{ "-NE",_NE },
        { "-",_MINUS },
        { "+",_PLUS },
        { "",_PLUS },
(* Index-mode changing commands: *)
        { "HI",_CHANGEMODE },
        { "JI",_CHANGEMODE },
        { "NI",_CHANGEMODE },
        { "HI.",_CHANGEMODE },
        { "JI.",_CHANGEMODE },
        { "NI.",_CHANGEMODE },

        { "DEB",_DEB },
        { "HELP",_help },
        { "?",_help },
        { "SET",_SET },
        { "&",_AMPERSAND },
        { ",",_GOVAL },
        { "$",_SHLIST },
        { "CURPAUSE",_SETCURSORPAUSE },
        { "COLUMNS",_SETCOLUMNS },
        { "FONT",_FONT },
        { "DOUBLE",_DOUBLE },
	{ "@0",_TOGGLEFLAGS },
	{ "@1",_TOGGLEFLAGS },
	{ "@2",_TOGGLEFLAGS },
        { "@3",_TOGGLEFLAGS },
        {  NULL,NULLFP }
 };
*/


set_command_adrs()
{
        set_command_adr("Q",_Q);
        set_command_adr("DEB",_DEB);
        set_command_adr("HELP",_help);
        set_command_adr("?",_help);
        set_command_adr("SET",_SET);
        set_command_adr("&",_AMPERSAND);
        set_command_adr("CURPAUSE",_SETCURSORPAUSE);
        set_command_adr("COLUMNS",_SETCOLUMNS);
        set_command_adr("FONT",_FONT);
        set_command_adr("DOUBLE",_DOUBLE);
	set_command_adr("@0",_TOGGLEFLAGS);
	set_command_adr("@1",_TOGGLEFLAGS);
	set_command_adr("@2",_TOGGLEFLAGS);
        set_command_adr("@3",_TOGGLEFLAGS);
}
        


/* Not needed currently:
PFI search_command(command_name)
char *command_name;
{
        register struct kinnas *ptr;

        for(ptr = cmd_name_table; ;ptr++)
         {
           if(!(ptr->cmd_name)) { return(NULLFP); } (* Not found *)
           if(!strcmp((ptr->cmd_name),command_name))
            { return(ptr->cmd_adr); }
         }
}
 */





/* These were formerly internal commands, but then memory ended. */




int _help()
{
        kcls();

        fprintf(KANJI_FP,
",             Go to first/next entry which contain word.\n");
        fprintf(KANJI_FP,
"number        Go to entry number (depends from indexing mode).\n");
        fprintf(KANJI_FP,
"+ or <enter>  Go to next entry in this indexing mode.\n");
        fprintf(KANJI_FP,
"-             Go to previous entry in this indexing mode.\n");
        fprintf(KANJI_FP,
"f / g         Find or go to entries which contain elements given as argument.\n");
        fprintf(KANJI_FP,
"ne / -ne      Go to next entry with some condition.\n");
        fprintf(KANJI_FP,
"$             Show group, etc.        &             Show various things.\n");

        fprintf(KANJI_FP,
"#             Show 32-bit number in kanji notation.\n");
        fprintf(KANJI_FP,
"v             View file.\n");
        fprintf(KANJI_FP,
"@3            Toggle between raw mode & screen mode.\n");
        fprintf(KANJI_FP,
"hi / ji / ni / hi. / ni. / ji.        Changes indexing mode.\n");
        fprintf(KANJI_FP,
"wr            Writes new database.    q       Quits to operating system.\n");
/* "wr            Writes new database.\n"); */
        fprintf(KANJI_FP,
"curpause      Change cursor pause.    font    Change font.\n");
/* "curpause      Change cursor pause.\n"); */
        fprintf(KANJI_FP,
"columns       Change columns.         double  Set doubling of pixels.\n");
/*
        fprintf(KANJI_FP,
"font          Change font.\n");
*/
        fprintf(KANJI_FP,
"deb           Debugging commands, use   deb @   for memory status.\n");
/*
        fprintf(KANJI_FP,
"q             Quit to operating system.\n");
 */
        fprintf(KANJI_FP,
"!cmd          Spawn command to operating system, single ! goes to command.com\n");
        fprintf(KANJI_FP,
"? or help     Give this help list. Use & #/ to get list of commands.\n");
        fprintf(KANJI_FP,
";   Restart cursor blinking in screen mode if pressed accidentally something.\n");
        fprintf(KANJI_FP,
". or DEL-key in screen mode   Return to entry/screen we are coming from.\n");
        fprintf(KANJI_FP,
"Keys in screen mode: HOME, END, Cursor keys, DEL, INS go to entry / show group.\n");
        fprintf(KANJI_FP,
"Edit cmds: a adde af c chpn car cdr d de df dn ea rf setsep sub subchr substr\n");

/*
        erprintf((ER,
         "To see list of commands, enter:   $w #*\n"));
        erprintf((ER,
         "Commands are entered without that #-sign."));
	erprintf((ER,
         " More help in file KANJIDIC.DOK\n"));
 */
}


int _Q()
{
        char c;

        if(changes_made == YES)
         {
           erprintf((ER,"Changes not saved. Exit ? [n]"));
           c = toupper(scr_getc()); putc(c,ER); terpri(ER);
           if(c == 'Y') { myexit(0); }
           else { return(-1); }
         }
        else /* changes_made == NO (since last write) */
         {
           erprintf((ER,"Exit ? [y]"));
           c = toupper(scr_getc()); putc(c,ER); terpri(ER);
           if(c == 'N') { return(-1); }
           else { myexit(0); }
         }
}


/* Commands to toggle various flags: */
int _TOGGLEFLAGS()
{
    switch(*(pname(cmd)+2))
     {
       case '0': { loc_counter_flag = !loc_counter_flag; break; }
       case '1': { _E_flag = !_E_flag; break; }
       case '2': { printtempflag = !printtempflag; break; }
       case '3': /* Set previous entry NIL so that current entry will be
                     printed in new mode at the top of loop(): */
        {
          kcls();
          rawprintflag  = !rawprintflag;
          previous_entry = NIL;
          break;
        }
       default:  { break; }
     }
 
   w_clear(ER);
   erprintf((ER,"@0: loc_counter_flag   =   %d           ",loc_counter_flag));
   erprintf((ER,"@1: _E_flag            =   %d\n",_E_flag));
   erprintf((ER,"@2: printtempflag      =   %d           ",printtempflag));
   erprintf((ER,"@3: rawprintflag       =   %d\n",rawprintflag));
}




int _SET()
{
        if(!nonnilsymbolp(arg1))
         {
           fprintf(NOTICE_FP,
"Use set symbol expr   to set value of symbol to expr.\n");
           return(-1);
         }

        setvalue(arg1,((argcount < 2) ? getcurentry_or_separate() : arg2));
        printexprnl(arg2,NOTICE_FP);
}


int _AMPERSAND()
{
        FILE *show_fp;

        if(argcount < 1)
         {
           fprintf(NOTICE_FP,
"Use & fish/       to see all the symbols starting with fish.\n");
           fprintf(NOTICE_FP,
"Use & $?*/        to see all the groups,  & #/  for all commands.\n");
           fprintf(NOTICE_FP,
"Use & *school*/   to see all the symbols which contain the word school.\n");
           return(-1);
         }

/* If arg1 doesn't fit to ERROR/NOTICE_FP when printed, then print it
    to KANJI_FP: */
        if(sprintnexpr(arg1,NULL,(SIZEOF_EW+5)) > (SIZEOF_EW - 10))
         {
           kcls();
           show_fp = KANJI_FP;
         }
        else { show_fp = NOTICE_FP; }

        if(listp(arg1)) { fprintf(show_fp,"%u ",length(arg1)); }
        printexprnl(arg1,show_fp);
}




int _DEB()
{
                if(argcount < 1) { goto mandrake; }
                switch(*pname(arg1))
                 {
		   case '!':
                    {
                      _debugprintflag = !_debugprintflag;
		      erprintf((ER,
		       "**_debugprintflag = %d\n",_debugprintflag));
		      break;
		    }
                   case '@':
                    {
                      if(!rawprintflag) { kcls(); }
                      prstat(stderr); break;
                    }
                   case '#':
                    {
                      if(!rawprintflag) { kcls(); }
                      printexpr(_oblist,stdout);
                      fprintf(stdout,
                       "\nlength(_oblist): %d.\n",length(_oblist));
                      fprintf(stdout,
                       "\nlength(hivec): %d.\n",length(hivec));
                      fprintf(stdout,
                       "\nlength(jivec): %d.\n",length(jivec));
                      fprintf(stdout,
                       "\nlength(nivec): %d.\n",length(nivec));
                      break;
                    }
                   case '$':
                    {
                      extern int vks_entries;
                      erprintf((ER,
                       "last_overlay: %s   adr_of_ovmain: 0x%x   %u\n",
		         last_overlay,adr_of_ovmain,adr_of_ovmain));
                      fprintf(NOTICE_FP,"current_mode: ");
                      printexpr(getcurrent_mode(),NOTICE_FP);
                      fprintf(NOTICE_FP,
"   current_index: %d   maxentries: %d\n   readentries: %d\n",
                         getcurrent_index(),maxentries,readentries);
                      fprintf(NOTICE_FP,
"hira_to_kata: %d   vks_entries: %d   length(entry_stack): %d\n",
                        hira_to_kata,vks_entries,length(entry_stack));
		      break;
                    }
                   case '?':
                    {
                      if(!rawprintflag) { kcls(); }
                      print(getcurrent_entry());
                      break;
                    }
                   case 'x': case 'X':
                    {
                      char *lto2hex();
                      TOB x;
                      char buf[83];
                      if(!rawprintflag) { kcls(); }
                      if(gen_stringp(arg2))
                       { strncpy(buf,pname(arg2),80); }
                      else
                       {
                         fprintf(stdout,
"\nGive long hexadecimal address in TOB-format (e.g. 3400:ABCD): ");
                         myfgets(buf,80,stdin);
                       }
                      /* Delete : from middle, if there is any: */
                      substchars(buf,':','\0');
                      sscanf(buf,"%lx",&x);
                      printf("\n%s ",lto2hex(sparebuf,x,':'));
                      print(x);
                      break;
                    }
                   default: 
                    {
mandrake:
                      fprintf(NOTICE_FP,
"Try one of the following:   !   @   #   $   ?  X   as argument.\n");
                      break;
                    }
                 }
}




int _SETCURSORPAUSE()
{
        extern UINT cursorpause;

        fprintf(NOTICE_FP,"cursorpause was %u\n",cursorpause);
        if(!intp(arg1))
         {
           fprintf(NOTICE_FP,
"usage: curpause unsigned-16-bit-number\n");
           fprintf(NOTICE_FP,
"Don't use very small values, because input may not work properly then.\n");
           return(-1);
         }
        setcursorpause(tob_int(arg1));
        fprintf(NOTICE_FP,"cursorpause set to %u\n",cursorpause);
}

int _SETCOLUMNS()
{
        extern UINT fieldstartcolumn,meanings_column,readings_column;

        fprintf(NOTICE_FP,"Old columns: %d %d %d\n",
          fieldstartcolumn,meanings_column,readings_column);

        if(argcount < 1) { return; }

        if(intp(arg1)) { fieldstartcolumn = tob_int(arg1); }
        if(intp(arg2)) { meanings_column =  tob_int(arg2); }
        if(intp(arg3)) { readings_column =  tob_int(arg3); }

        fprintf(NOTICE_FP,"New columns: %d %d %d\n",
          fieldstartcolumn,meanings_column,readings_column);
}


_DOUBLE()
{
        fprintf(NOTICE_FP,"double_flag was: %d\n",get_double_flag());
        if((argcount < 1) || !gen_stringp(arg1))
         { goto info; }

        if(monostrequ(pname(arg1),"on"))
         { set_double_flag(1); }
        else if(monostrequ(pname(arg1),"off"))
         { set_double_flag(0); }
        else
         {
info:
           fprintf(NOTICE_FP,
"usage: double on     to show kanjis widened in X-direction.\n");
           fprintf(NOTICE_FP,
"or:    double off    to show kanjis in narrow mode.\n");
         }
}




_FONT()
{
        register TOB argh;
        char env_var_name[81];

        print_font_settings("Old");

        if((argcount < 1))
         {
           fprintf(NOTICE_FP,
"Usage: FONT font1file font2file size doubflag swap_flag rev_flag adj_flag skip\n");
           fprintf(NOTICE_FP,
"Or just: FONT 16   or   FONT 24   or   FONT ,,ENV_VAR_NAME\n");
           return;
         }

        if(intp(arg1))
         {
           sprintf(env_var_name,"FONT%d",tob_int(arg1));
           argh = string_tob(env_var_name);
         }
        else if(consp(arg1)) { argh = arg1; }
        else
         {
           argh = arglist;
         }

        if(setfontfile(argh))
         {
           previous_entry = special_screen = NIL;
	   kcls();
           print_font_settings("New");
         }

}



print_font_settings(luumu)
char *luumu;
{
        extern char *current_font1file,*current_font2file;
        extern int fontsize;
        extern BYTE double_flag,swap_flag,reverse_flag,adjust_flag;
        extern UINT skip_bytes;

        fprintf(NOTICE_FP,
"%s: %s %s Size %d Dbl %d Swap %d Rev %d Adj %d Skip %d\n",
            luumu,current_font1file,current_font2file,fontsize,
             double_flag,swap_flag,reverse_flag,adjust_flag,skip_bytes);
}


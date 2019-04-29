

#include "kdinclu.h"


ovmain()
{
        int ovmain();
        adr_of_ovmain = ovmain;

        if(!strcmp(pname(cmd),"#DEBFLAG")) /* Toggle debug flag */
         { debug_flag = !debug_flag; }
        else if(!strcmp(pname(cmd),"#DEBSTR"))
         { dump_free_strings_list(); }
        else
         { dump_contents_of_vks(); }
}


dump_free_strings_list()
{
        TOB lista,item;
        UINT orava;

        fprintf(NOTICE_FP,
"length(_free_strings_list): %d\n",(orava = length(_free_strings_list)));


        if(!orava) { return; }

        clear_realscreen();
        w_home(KANJI_FP);


        for(lista = _free_strings_list; !nilp(lista); lista = cdr(lista))
         {
           item = car(lista);
           if(!nonnilsymbolp(item))
            {
              fprintf(KANJI_FP,
"**ERROR: wrong item in _free_strings_list: ");
              printexprnl(item,KANJI_FP);
              continue;
            }
           printexpr(value(item),KANJI_FP);
           fprintf(KANJI_FP,"\t %3d ",strlen(pname(item)));
           printexprnl(symbol2string(item),KANJI_FP);
         }

        getchar();
        clear_realscreen();
        redraw_realscreen();
}



dump_contents_of_vks()
{
        TOB lista;
        int muna;


        fprintf(NOTICE_FP,
"length(vks_vector): %d   ",length(vks_vector));

        if(nilp(last_ptr))
         { muna = 0; fprintf(NOTICE_FP,"last_ptr is NIL.\n"); }
        else
         {
           muna = (last_ptr - vks_vector);

           fprintf(NOTICE_FP,
"(last_ptr - vks_vector): %d  /3  = %d\n",
             muna,(muna / 3));
         }

        if(!muna) { return; }

        clear_realscreen();
        w_home(KANJI_FP);

        for(lista = vks_vector; !nilp(lista); lista = cdr(lista))
         {
           printexprnl(car(lista),KANJI_FP);
           if(eq(lista,last_ptr)) { break; }
         }

        getchar();
        clear_realscreen();
        redraw_realscreen();
}


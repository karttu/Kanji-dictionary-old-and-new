

#include "kdinclu.h"



char *index(),*rindex(),*sindex();

extern UINT jis1level_cnt,jis2level_cnt;


ovmain()
{
        int ovmain();
        TOB read_and_show_one_line();
        char *myfgets();
        FILE *fopen();
        FILE *input;
        TOB filename;
        UINT lines=0;
        TOB offset_marker;
        char *s;
        char bufferi[81];

        adr_of_ovmain = ovmain;

        if(!gen_stringp(arg1))
         {
           fprintf(NOTICE_FP,
/*        arg1         arg2        arg3        arg4          arg5          */
"usage: v filename.ext [hor_space] [ver_space] [double_flag] [SJ_FLAG]\n");
           fprintf(NOTICE_FP,
"If optional argument is specified as () then default value is used.\n");
           return(-1);
         }

        if(!(input = fopen(pname(arg1),"r")))
         {
           fprintf(NOTICE_FP,
"v: can't open file: %s !\n",pname(arg1));
           return(-1);
         }

        kcls();

        /* If SJ_FLAG (arg5) is given explicitly as 0, then it overrides
            the .SJ? extension of filename: */
        if((s = rindex(pname(arg1),'.')) && monocase_streq("sj",(s+1)))
         { if(!eq(arg5,ZERO)) { arg5 = int_tob(1); } }


        sprintf(bufferi,"?%s",pname(arg1));
        filename = intern(bufferi);
        setvalue(filename,
            clistn(fp_tob(input),arg2,arg3,arg4,arg5,ENDMARK));

        special_screen = filename;

        set_jis_errcnt(0);
        jis1level_cnt = jis2level_cnt = 0;

        offset_marker =
          makefileoffset_marker(filename,getlow(ftell(input)),1);

        add_to_vks(offset_marker);

        while(1)
         {
           if(nilp(read_and_show_one_line(offset_marker))) { break; }
           lines++;
           if(lines >= maxfilelines_in_vks)
            {
              goto not_whole;
            }
         }

        fclose(input);

not_whole: ;

        fprintf(NOTICE_FP,
"File %s: read %u lines.\n",(pname(filename)+1),lines);
        file_status();
}


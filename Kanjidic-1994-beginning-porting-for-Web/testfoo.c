
#include "stdio.h"

struct listcell
{
     struct listcell *cdr;
     int *car;
};

 
int cfoo(list)
struct listcell *list;
{
    int i=0;
    int *prev,*tmp;

    prev = NULL;

    for(; list ; list = list->cdr)
     {
       printf("%9u",list->car); fflush(stdout);
       printf("   %9u\n",*(list->car)); fflush(stdout);
       tmp = list->car;
       list->car = prev;
       prev = tmp;
       i++;
     }

    return(i);
}



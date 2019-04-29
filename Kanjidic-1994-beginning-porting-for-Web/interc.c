

#define TABSIZE 256

int *intptr_table[TABSIZE]; /* For each byte. */
 

struct listcell
{
     struct listcell *cdr;
     int *car;
};


/* This should be called with a list of integers from
    (0 1 2 ... to ... 254 255)
    (or something else, if we want to do some read-conversion...)
   so that the internal pointers Franz Lisp uses for storing those
   integers can be stored to intrptr_table.
 */

int init_table(list)
struct listcell *list;
{
    int i;

    for(i=0; (list && (i < TABSIZE)); list = list->cdr)
     {
       intptr_table[i] = (list->car);
       i++;
     }

    return(i);
}


int read_a_line(soc,list)
int soc;
register struct listcell *list;
{
    register struct listcell *next;
    int read_bytes;
    int len=0;
    unsigned char bufferi[1];

    if(!list || !(list->cdr)) { return(0); }
    list->car = intptr_table['('];
    list = list->cdr;

    while((next = list->cdr) &&
          ((read_bytes = read(soc,bufferi,sizeof(char))) > 0) &&
                (*bufferi != '\n'))
     {
       list->car = intptr_table[*bufferi];
       list = next;
       len++;
     }

    list->car = intptr_table[')'];

    return(len);
}



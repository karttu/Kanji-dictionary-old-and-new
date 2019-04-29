
/* Macros to extract information from the entries of _windows list: */
#define getptr_w_column(x)    icar_to_intptr(x)
#define getptr_w_line(x)      icar_to_intptr(cdr(x))
#define getw_upperedge(x)     tob_int(qnth(2,(x)))
#define getw_heigth(x)        tob_int(qnth(3,(x)))
#define getw_leftedge(x)      tob_int(qnth(4,(x)))
#define getw_width(x)         tob_int(qnth(5,(x)))
#define getw_fps(x)           qnthcdr(6,(x))


#define setw_upperedge(x,y)   rplaca(qnthcdr(2,(x)),int_tob((y)))
#define setw_heigth(x,y)      rplaca(qnthcdr(3,(x)),int_tob((y)))
#define setw_leftedge(x,y)    rplaca(qnthcdr(4,(x)),int_tob((y)))
#define setw_width(x,y)       rplaca(qnthcdr(5,(x)),int_tob((y)))


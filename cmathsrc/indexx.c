/* indexx.c
   Heap-sort for an array of double precision numbers.
*/

/************************************************/
/*                                              */
/*  CMATH.  Copyright (c) 1989 Design Software  */
/*                                              */
/************************************************/

#include "cmath.h"

/*-----------------------------------------------------------------*/

#if (PROTOTYPE)

int indexx (int nn, double arrin[], int indx[])

#else

int indexx (nn, arrin, indx)
int    nn;
double arrin[];
int    indx[];

#endif

/* Purpose ...
   -------
   Index the elements of the array arrin[] such that
   arrin[indx[j]] is in ascending order for j = 0, 1, ... nn-1.

   Input ...
   -----
   arrin[]  : array of numbers to be sorted
   nn       : number of elements in arrin[]

   Output ...
   ------
   indx[]   : integer array containing index of elements

   This C code written by ...  Peter & Nigel,
   ----------------------      Design Software,
                               42 Gubberley St,
                               Kenmore, 4069,
                               Australia.

   Version ... 2.0, 12-Feb-89 now uses zero subscript element
   -------     1.0, 23-Feb-88

   Notes ...
   -----
   (1) The input quantities nn and arrin are unchanged on exit.

   (2) This routine has been adapted from that published in the book
       W.H. Press et al
       Numerical Recipes: The art of scientific computing.
       Cambridge University Press 1986
*/

/*-------------------------------------------------------------------*/

{  /* --- start of indexx --- */
int jj, ii, ir, indxt, L;
double q;

/* Initialize the index array with consecutive integers */
for (jj = 0; jj < nn; ++jj) indx[jj] = jj;

L = nn / 2;
ir = nn - 1;

do
   {
   if (L > 0)
      {
      --L;
      indxt = indx[L];
      q = arrin[indxt];
      }
   else
      {
      indxt = indx[ir];
      q = arrin[indxt];
      indx[ir] = indx[0];
      --ir;
      if (ir == 0)
         {
	 indx[0] = indxt;
         goto Finish;
         }
      }

   ii = L;
   jj = L + L + 1;

   while (jj <= ir)
      {
      if (jj < ir)
         {
         if (arrin[indx[jj]] < arrin[indx[jj+1]]) ++jj;
         }
      if (q < arrin[indx[jj]])
         {
         indx[ii] = indx[jj];
         ii = jj;
	 jj += jj + 1;
         }
      else
         jj = ir + 1;
      }

   indx[ii] = indxt;
} while (1);

Finish:
return 0;
}   /* --- end of indexx() --- */

/*-----------------------------------------------------------------*/

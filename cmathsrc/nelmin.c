/* nelmin.c
   The Nelder-Mead Simplex Minimization Procedure.
*/

/************************************************/
/*                                              */
/*  CMATH.  Copyright (c) 1989 Design Software  */
/*                                              */
/************************************************/

#include "cmath.h"
#if (STDLIBH)
#include <stdlib.h>
#endif
#include <stdio.h>
#include <math.h>

#ifndef NULL
#define  NULL  0
#endif


/*-----------------------------------------------------------------*/

#if (PROTOTYPE)

int nelmin (double (*Fnelmin)(int n, double x[]),
            int n,
            double xmin[], double *ynewlo, double reqmin,
            double step[],
            int konvge, int *icount, int kcount,
            int *numres, int *ifault,
            double reltol, double abstol)

#else

int nelmin (Fnelmin, n, xmin, ynewlo, reqmin, step, konvge,
	    icount, kcount, numres, ifault, reltol, abstol)

int    n, konvge, kcount;
int    *icount, *numres, *ifault;
double (*Fnelmin) ();
double step[], xmin[];
double reqmin, *ynewlo;
double reltol, abstol;

#endif

/*
   Purpose ....
   -------
   Find the minimum value of a user-specified function.

   Input ...
   -----
   Fnelmin: User specified function to be minimized. NOTE that
	    the function name must be passed.
	    double Fnelmin (n, x)
            int    n;
	    double x[];
   n      : INTEGER. The number of variables over which we are
	    minimizing.
   xmin   : Array [n] of double; contains the coordinates
	    of the starting point.  (see note 5)
   reqmin : The terminating limit for the variance of function values.
   step   : Array [n] of double; determines the
	    size and shape of the initial simplex.
	    The relative magnitudes of its N elements
	    should reflect the units of the N variables.
   konvge : The convergence check is carried out every KONVGE
	    iterations.
   kcount : Maximum number of function evaluations.
   reltol : relative tolerance on minimum check  (see note 6)
   abstol : absolute tolerance on minimum check  (see note 6)

   Output ...
   ------
   xmin   : Array [id] of double; contains the
	    coordinates of the minimum.
   ynewlo : The minimum value of the function.
   icount : Function evaluations performed.
   numres : Number of restarts.
   ifault : return status flag ...
	    = 0 No error.
	    = 1 REQMIN, N or KONVGE illegal value.
	    = 2 Termination because KCOUNT exceeded before convergence.
            = 3 could not allocate memory for workspace

   This C code written by ...  Peter & Nigel,
   ----------------------      Design Software,
                               42 Gubberley St,
                               Kenmore, 4069,
                               Australia.

   Version ... 1.0 November 1987
   -------     2.0 March    1989      reltol, abstol added
				      zero subscripts implemented
               2.1 15 April 1989      workspace allocated
                                      start[] removed from arguements
               2.2 28 April 1989      n added to Fnelmin()
               2.3 7  August  89      fixed true min. check
               2.4 12 Dec   1989      fixed memory (de)allocation

   Notes ...
   -----
   1. This algorithm is a modified version of ..
      Algorithm AS 47 Applied Statistics (J. R. Statist. Soc. C),
      (1971) VOL.20. NO.3

   2. The data values of RCOEFF (reflection), ECOEFF (extension),
      and CCOEFF (contraction) can be changed by rewriting the
      assignment statements below.  The values currently set are 1.0,
      2.0 and 0.5 respectively.  These values have been recommended
      by Nelder and Mead as being best for a general situation.

   3. The value of REQMIN must be set larger than the rounding
      error in computing the variance at the minimum.  This holds
      for both Double and Single precision calculations.
      Chambers and Ertel recommend that REQMIN be set to a value
      somewhere between the required accuracy of the function at
      the minimum and the square of this value.

   4. Note that the elements [0 .. n-1] are utilized in this
      version of the routine.

   6. reltol and abstol should be set to zero for well behaved
      functions where restarts are not a problem.

   7. References ...
      Nelder, J.A. and Mead, R. (1965) A simplex method for function
      minimization. Computer J. 7,308-313

      O'Neill, R. (1971) Algorithm AS47. Function minimization
      using a simplex algorithm. Appl. Statist. 20,338-345.

      Chambers, J.M. and Ertel, J.E. (1974) Remark AS R11.
      Appl. Statist. 23,250-251.

      Olsson, D.M. and Nelson, L.S. (1975) The Nelder - Mead
      simplex procedure for function minimization.
      Technometrics 17,45-51. (Examples of use.)

      Benyon, P.R. (1976) Remark AS R15. Appl. Statist. 25,97.

      Hill, I.D. (1978) Remark AS R28. Appl. Statist. 27,380-382.

----------------------------------------------------------------------
*/

{  /* begin nelmin() ... */

int true, false;
double zero, half, one, delta;

double *pstar, *p2star, *pbar;
double **p;
double *y, *start;
double y2star, ccoeff;
double z, sum, ylo, rcoeff, ystar, ecoeff;
double curmin, del, x, yhi;
double small;

int jcount, np1, i, j;
int ilo, ihi, L;

int ok, reflok, extnok, contok, quit, found;  /* boolean variables */

/* label First, Finish; */

true  = 1;
false = 0;
zero  = 0.0;
half  = 0.5;
one   = 1.0;
delta = 1.0e-03;

/* ---- Validity checks on input parameters. ---- */

*ifault = 0;
if (reqmin <= zero || n < 1 || konvge < 1 || reltol < zero ||
    abstol < zero || xmin == NULL || ynewlo == NULL || step == NULL)
   {
   *ifault = 1;
   return (0);
   }

/* allocate workspace ...
   We want start[0..n-1], y[0..n], pstar[0..n-1], p2star[0..n-1]
           pbar[0..n-1], p[0..n-1][0..n]  */
start  = (double *) NULL;
pstar  = (double *) NULL;
p2star = (double *) NULL;
pbar   = (double *) NULL;
y      = (double *) NULL;
p      = (double **) NULL;

start = (double *) malloc(n * sizeof(double));
if (start == NULL)
   {
   *ifault = 3;
   goto Finish;
   }
pstar = (double *) malloc(n * sizeof(double));
if (pstar == NULL)
   {
   *ifault = 3;
   goto Finish;
   }
p2star = (double *) malloc(n * sizeof(double));
if (p2star == NULL)
   {
   *ifault = 3;
   goto Finish;
   }
pbar = (double *) malloc(n * sizeof(double));
if (pbar == NULL)
   {
   *ifault = 3;
   goto Finish;
   }
y = (double *) malloc((n+1) * sizeof(double));
if (y == NULL)
   {
   *ifault = 3;
   goto Finish;
   }
/* allocate the matrix p as an array of pointers to rows of the matrix */
p = (double **) malloc(n * sizeof(double *));
if (p == NULL)
   {
   *ifault = 3;
   goto Finish;
   }
for (i = 0; i < n; ++i) p[i] = (double *) NULL;
for (i = 0; i < n; ++i)
   {
   p[i] = (double *) malloc((n+1) * sizeof(double));
   if (p[i] == NULL)
      {
      *ifault = 3;
      goto Finish;
      }
   }

/* ---- reflection, extension and contraction coefficients. ---- */
rcoeff = 1.0;
ecoeff = 2.0;
ccoeff = 0.5;

quit    = false;
found   = false;
*ifault = 2;
*icount = 0;
*numres = 0;

np1 = n + 1;
del = one;

/* copy start point */
for (i = 0; i < n; ++i) start[i] = xmin[i];

First:

/* ---- Construct the initial simplex. ---- */
for (i = 0; i < n; ++i) p[i][n] = start[i];
z = (*Fnelmin) (n, start);
y[n] = z;
for (j = 0; j < n; ++j)
  {
  x = start[j];
  start[j] += (step[j] * del);  /* -- move a little in one
                                      dimension only -- */
  for (i = 0; i < n; ++i) p[i][j] = start[i];
  z = (*Fnelmin) (n, start);
  y[j] = z;
  start[j] = x;
  }
*icount = *icount + np1;

/* ---- Simplex construction complete; now do some work. ---- */

while (!found && !quit)
  {  /* ---- take some steps ---- */

  for (jcount = 1; jcount <= konvge; ++jcount)
    {  /* ---- take a single step ---- */

    /* ---- find highest and lowest y values.  yhi ( =y[ihi] ) indicates
           the vertex of the simplex to be replaced.   */

    ylo = y[0];
    yhi = ylo;
    ilo = 0;
    ihi = 0;
    for (i = 1; i < np1; ++i)
      {
      if(y[i] < ylo)     /* -- find the lowest value of the objective  */
        {
        ylo = y[i];
        ilo = i;
        }
      if(y[i] > yhi)    /* -- find the highest value of the objective -- */
        {
        yhi = y[i];
        ihi = i;
        }
      }

    /* ---- Calculate pbar, the centroid of the simplex vertices
            excepting that with the y value yhi. (largest objective value)  */

    for (i = 0; i < n; ++i)
      {    /* -- calculate the average in each dimension -- */
      z = zero;
      for (j = 0; j < np1; ++j) z += p[i][j];
      z -= p[i][ihi];
      pbar[i] = z / n;
      }

    /* ---- reflection through the centroid. ---- */

    for (i = 0; i < n; ++i)
      pstar[i] = (one + rcoeff) * pbar[i] - rcoeff * p[i][ihi];
    ystar = (*Fnelmin) (n, pstar);
    (*icount)++;
    reflok = (ystar < ylo);    /* -- true if we have moved downhill -- */

    if (reflok) /* then */

      {   /* ---- successful reflection, so be bold and try extension */
      for (i = 0; i < n; ++i)
        p2star[i] = ecoeff * pstar[i] + (one - ecoeff) * pbar[i];
      y2star = (*Fnelmin) (n, p2star);
      (*icount)++;
      extnok = (y2star < ystar);  /* -- true if we continue to
                                     move downhill -- */

      if (extnok)
        {   /* ---- retain extension ---- */
	for (i = 0; i < n; ++i) p[i][ihi] = p2star[i];
        y[ihi] = y2star;
        }
      else    /* ---- retain the first reflection. ---- */
        {
	for (i = 0; i < n; ++i) p[i][ihi] = pstar[i];
        y[ihi] = ystar;
        }
      }   /* successful reflection */

    else    /* if reflok then else */
      {
      /* ---- reflection has not been successful.  ystar is the function
      value at the extended point.  now look at the other points
      of the simplex ...
      if there are none > ystar then contract to a point within the simplex
      if there is one   > ystar then reduce the size of the extension
      if there are many > ystar then retain the reflection as is.  */

      L = 0;
      for (i = 0; i < np1; ++i) { if (y[i] > ystar) L++; }

      if (L == 1) /* then */
        {
        /* ---- reflection was not successful but we can try reducing the
          extension by contracting on the reflection side of the centroid.
          Record the reflection for the contraction below. */
	for (i = 0; i < n; ++i) p[i][ihi] = pstar[i];
        y[ihi] = ystar;
        }

      if (L == 0 || L == 1) /* then */

        {
        /* L == 0 : there are no other points with higher objectives so
          try a contraction on the y(ihi) side of the centroid.
          i.e. within the simplex.
          L == 1 : there is one other point with a higher objective so
          try a contraction on the reflection side of the centroid.  */
	for (i = 0; i < n; ++i)
          p2star[i] = ccoeff * p[i][ihi] + (one - ccoeff) * pbar[i];
        y2star = (*Fnelmin) (n, p2star);
        (*icount)++;
        contok = (y2star <= y[ihi]);  /* -- true if we have not
                                         gone uphill -- */

        if (contok) /* then */
          {    /* ---- retain contraction ---- */
	  for (i = 0; i < n; ++i) p[i][ihi] = p2star[i];
          y[ihi] = y2star;
          }
        else
          {
          /* ---- contract whole simplex about a point within itself
            but close to the current minimum. ---- */
	  for (j = 0; j < np1; ++j)
            {
	    for (i = 0; i < n; ++i)
              {
              p[i][j] = (p[i][j] + p[i][ilo]) * half;
              xmin[i] = p[i][j];
              }
            y[j] = (*Fnelmin) (n, xmin);
            }
          *icount += np1;
          }  /* if (contok) */
        }  /* contraction */

      else   /* if (L == 0) or (L == 1) then else */

        {
        /* ---- Retain the first reflection as there are many points in
          the simplex that have higher objective values than
          this new point.  A change is better than nothing. ---- */
	for (i = 0; i < n; ++i) p[i][ihi] = pstar[i];
        y[ihi] = ystar;
        }

      }   /* if Reflok then else .. */

    }   /* ---- procedure TakeAStep ----*/


  /* are we over the limit for number of function evaluations ? */
  quit = (*icount > kcount);
  if (!quit) /* then */
    {  /* ---- check to see if minimum reached.
               calculation of the variance must be done in the highest
               precision available.  */
    /* mean */
    sum = zero;
    for (i = 0; i < np1; ++i) sum += y[i];
    sum /= np1;
    /* variance */
    curmin = zero;
    for (i = 0; i < np1; ++i) curmin += ((y[i] - sum) * (y[i] - sum));
    curmin /= n;

    /* ---- curmin is the variance of the n+1 Fnelmin values at the
	    vertices.  If we haven't reached the minimum to the
	    required accuracy then take a few more steps.    */
    found = (curmin < reqmin);
    }
  }   /* while not found and not quit ... */

/* ---- At this point we either
        1. think that we have found the minimum
     or 2. have exceeded the allowable number of steps
       so, save the current minimum  */

if (y[ihi] > y[ilo]) ihi = ilo;
for (i = 0; i < n; ++i) xmin[i] = p[i][ihi];
*ynewlo = y[ihi];

/* **** bail out if necessary **** */
if (quit)  goto Finish;

/* ---- Check around the currently selected point to see if it is
	a local minimum.  */
small = fabs(*ynewlo) * reltol + abstol;
ok = true;   /* innocent until proven guilty */
for (i = 0; i < n; ++i)
  {    /* ---- check along each dimension ---- */
  if (ok) /* then */
    {
    del = step[i] * delta;
    xmin[i] += del;         /* -- check along one direction -- */
    z = (*Fnelmin) (n, xmin);
    (*icount)++;
    if ((z - *ynewlo) < -small) ok = false;
    xmin[i] -= (del + del);   /* -- now check the other way -- */
    z = (*Fnelmin) (n, xmin);
    (*icount)++;
    if ((z - *ynewlo) < -small) ok = false;
    xmin[i] += del;                     /* -- back to start -- */
    }
  }

/* ---- return on finding a local minimum to the desired accuracy. ---- */

if (ok) /* then */
  {
  *ifault = 0;
  goto Finish;
  }

/* ---- Reduce the size of the simplex and restart the procedure. ---- */

found = false;   /* -- we did not find a true minimum -- */
for (i = 0; i < n; ++i) start[i] = xmin[i];
del = delta;
(*numres)++;
goto First;

Finish:  /* end of procedure */
/* free workspace ...
   start[0..n-1], y[0..n], pstar[0..n-1], p2star[0..n-1]
   pbar[0..n-1], p[0..n-1][0..n]  */
if (p != NULL)
   {
   for (i = 0; i < n; ++i)
      {
      if (p[i] != NULL) { free (p[i]); p[i] = NULL; }
      }
   free (p);
   p = NULL;
   }
if (y      != NULL) { free (y);      y = NULL; }
if (pbar   != NULL) { free (pbar);   pbar = NULL; }
if (p2star != NULL) { free (p2star); p2star = NULL; }
if (pstar  != NULL) { free (pstar);  pstar = NULL; }
if (start  != NULL) { free (start);  start = NULL; }

return (0);
}  /* ---- end of nelmin() ---- */

/*-----------------------------------------------------------------*/

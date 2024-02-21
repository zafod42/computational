#include <math.h>
#include <stdio.h>
#include <float.h>

#include "cmath.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define SIZE 7

/*
 * adaptive Newton-Cotes quadrature usage
 * int quanc8 (double (*f)(double x), double a, double b,
 *      double epsabs, double epsrel,
 *      double *result, double *errest,
 *      int *nfe, double *posn, int *flag);
 */

/*
 * spline usage
 * int spline (int n, int end1, int end2,
 *      double slope1, double slope2,
 *      double x[], double y[],
 *      double b[], double c[], double d[],
 *      int *iflag);
 */

/*
 * seval usage
 * double seval (int n, double u,
 *         double x[], double y[],
 *         double b[], double c[], double d[],
 *         int *last)
 */


double phi(double t)
{
    return sin(t * t);
}

/*
 *  ====================
 *  f(x) = int phi(t)dt form 0 to x
 *  =====================
 *
 *  double result;
 *  quanc8(phi, 0, x, ..., &result, ...);
 *  f(x) = result;
 *
 */


int main(int argc, char* argv[])
{
    double result = 0;      // out
    double errest = 0;      // out
    double epsabs = 0.0001;  // in
    double epsrel = 0; //1e-10;      // in
    int nfe = 0;
    double posn = 0;
    int flag = 0;
    double x[SIZE] = {1.5, 1.7, 1.9, 2.1, 2.3, 2.5, 2.7};
    double f[SIZE] = { 0 };
    double err[SIZE] = { 0 };
    for (int i = 0; i < SIZE; ++i)
    {
        /* zeroing outputs */
        result = 0;
        errest = 0;
        nfe = 0;
        posn = 0;
        flag = 0;
        /* evaluate fx */
        quanc8(phi, 0, x[i], epsabs, epsrel, &result, &errest, &nfe, &posn, &flag);
        f[i] = result;
        err[i] = errest;
    }

    for(int i = 0; i < SIZE; ++i)
    {
        printf("f(%.*g) = %.*f", x[i], DBL_DIG,  f[i], 13);
        printf("\t err: %f\n", err[i]);
    }

    double b[SIZE] = { 0 };
    double c[SIZE] = { 0 };
    double d[SIZE] = { 0 };
    int iflag = 0;

    spline(SIZE, 1, 1, 1, 1, x, f, b, c, d, &iflag);

    int last = 0;
    double xx = 1.8;
    double Svalue = seval(SIZE, xx, x, f, b, c, d, &last);
    double Fvalue = 0;
    quanc8(phi, 0, xx, epsabs, epsrel, &Fvalue, &errest, &nfe, &posn, &flag);
    printf("%f, %f\n", Fvalue, Svalue); 
        
    return EXIT_SUCCESS;
}

#include "solver.h"
#include "thread_funcs.h"
#include "msr_matrix.h"

void  matrix_mult_vector (double *A, int *I, int n, double *x, double *y, int p, int k)
{
  int i1, i2, i, j, offset, len;
  double s;
  i1 = k * n;
  i1 /= p;
  i2 = (k + 1) * n;
  i2 = i2 / p - 1;
  for (i = i1; i <= i2; i++)
  {
    s = A[i] * x[i];
    len = I[i + 1] - I[i];
    offset = I[i];
    for (j = 0; j < len; j++)
      {
        s += A[offset + j] * x[I[offset + j]];
      }
    y[i] = s;
  }
  reduce_sum (p);
}

void apply_preconditioner (double *a, int n, double *u, double *r, int p, int k)
{
  int i1 = k * n; i1 /= p;
  int i2 = (k + 1) * n; i2 =i2 / p - 1;
  int i;

  for (i = i1; i <= i2; i++)
    {
      u[i] = r[i] / a[i];
    }
  reduce_sum (p);
}


void lin_comb (double *x, double *y, double t, int n, int p, int k)
{
  int i1 = k * n; i1 /= p;
  int i2 = (k + 1) * n; i2 = i2 / p - 1;
  int i;

  for (i = i1; i <= i2; i++)
    {
      x[i] -= y[i] * t;
    }
  reduce_sum (p);
}

double scalar_product (double *x, double *y, int n, int p, int k)
{
  int i1 = k * n; i1 /= p;
  int i2 = (k + 1) * n; i2 = i2 / p - 1;
  int i;
  double *s = new double[1];
  *s = 0;
  for (i = i1; i <= i2; i++)
    {
      *s += x[i] * y[i];
    }
  reduce_sum (p, s, 1);
  double ret = *s;
  delete[] s;
  return ret;
}

int solve (double *a, int *I, int n, double *x/*начальное приближение*/, double *b, double *r, double *u, double *v, double eps, int maxit, int p, int k)
{
  //u = Ax
  matrix_mult_vector (a, I, n, x, r, p, k);
  //r -= b
  lin_comb (r, b, 1, n, p, k);
  double res = scalar_product (r, r, n, p, k);
  if (res < eps * eps)
    return 0;
  int it;
  double c1;
  double c2;
  double t;
  for (it = 0; it < maxit; it++)
    {
      //Mu = r
      apply_preconditioner (a, n, u, r, p, k);
      //v = Au
      matrix_mult_vector (a, I, n, u, v, p, k);
      c1 = scalar_product (v, r, n, p, k);
      if (c1 < eps * eps)
        break;
      c2 = scalar_product (v, v, n, p, k);
      if (c2 < eps * eps)
        break;
      t = c1 / c2;
      //x -= tu;
      lin_comb (x, u, t, n, p, k);
      //r -= tv
      lin_comb (r, v, t, n, p, k);
      if (k == 0)
       {
         printf ("iter:%d c1:%e c2:%e\n", it, c1, c2);
       }
    }
  if (k == 0)
    printf ("iters: %d\n", it);
  if (it >= maxit)
    return -1;
  return 0;
}

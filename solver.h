#ifndef SOLVER_H
#define SOLVER_H

void matrix_mult_vector (double *A, int *I, int n, double *x, double *y, int p, int k);
int solve (double *a, int *I, int n, double *x/*начальное приближение*/, double *b, double *r, double *u, double *v, double eps, int maxit, int p, int k);
void lin_comb (double *x, double *y, double t, int n, int p, int k);
double scalar_product (double *x, double *y, int n, int p, int k);
void apply_preconditioner (double *a, int n, double *u, double *r, int p, int k);
#endif // SOLVER_H

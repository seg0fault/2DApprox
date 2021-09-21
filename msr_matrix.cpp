#include <pthread.h>
#include <cstring>
#include <cmath>
#include "msr_matrix.h"
#include "thread_funcs.h"
#include <functional>
#define OUTPUT 16
#define MAX_NZ 6
#define EPS 1e-8

double get_func_value_by_i_j (double i, double j, int n, int m, std::function <double(double, double)> &f)
{
  double hx = 1. / n;
  double hy = 1. / m;
  double x = hx * i;
  double y = hy * j;
  return f (x, y);
}
int get_links (int n, int n_cut, int m, int m_cut, int i, int j, int x[], int y[])
{
  if ((m_cut < j && j < m && 0 < i && i < n) ||
    (n_cut < i && i < n && 0 < j && j < m))
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i + 1; y[1] = j + 1;
      x[2] = i + 1; y[2] = j;
      x[3] = i; y[3] = j - 1;
      x[4] = i - 1; y[4] = j - 1;
      x[5] = i - 1; y[5] = j;
      return 6;
    }
  if ((i == 0 && m_cut < j && j < m) ||
      (i == n_cut && 0 < j && j < m_cut))
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i + 1; y[1] = j + 1;
      x[2] = i + 1; y[2] = j;
      x[3] = i ; y[3] = j - 1;
      return 4;
    }
  if ((0 < i && i < n_cut && j == m_cut) ||
      (n_cut < i && i < n && j == 0))
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i + 1; y[1] = j + 1;
      x[2] = i + 1; y[2] = j;
      x[3] = i - 1; y[3] = j;
      return 4;
    }
  if (0 < i && i < n && j == m)
    {
      x[0] = i + 1; y[0] = j;
      x[1] = i; y[1] = j - 1;
      x[2] = i - 1; y[2] = j - 1;
      x[3] = i - 1; y[3] = j;
      return 4;
    }
  if (i == n && 0 < j && j < m)
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i; y[1] = j - 1;
      x[2] = i - 1; y[2] = j - 1;
      x[3] = i - 1; y[3] = j;
      return 4;
    }
  if ((i == 0 && j == m_cut) || (i == n_cut && j == 0))
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i + 1; y[1] = j + 1;
      x[2] = i + 1; y[2] = j;
      return 3;
    }
  if (i == n && j == m)
    {
      x[0] = i; y[0] = j - 1;
      x[1] = i - 1; y[1] = j - 1;
      x[2] = i - 1; y[2] = j;
      return 3;
    }

  if (i == 0 && j == m)
    {
      x[0] = i + 1; y[0] = j;
      x[1] = i; y[1] = j - 1;
      return 2;
    }

  if (j == 0 && i == n)
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i - 1; y[1] = j;
      return 2;
    }
  if (i == n_cut && j == m_cut)
    {
      x[0] = i; y[0] = j + 1;
      x[1] = i + 1; y[1] = j + 1;
      x[2] = i + 1; y[2] = j;
      x[3] = i; y[3] = j - 1;
      x[4] = i - 1; y[4] = j;
      return 5;
    }
  return -1000;
}

int get_num_links (int n, int m)
{
  int i, j;
  int sum = 0;
  for (i = 0; i <= n; i++)
    {
      for (j = 0; j <= m; j++)
        sum += get_num_links (n, m, i, j);
    }
  return sum;
}

int get_num_links (int n, int m, int i, int j)
{
  if ((i == 0 &&  m_cut < j && j < m) ||
      (j == m && 0 < i && i < n) ||
      (i == n && 0 < j && j < m) ||
      (j == 0 && n_cut < i && i < n) ||
      (i == n_cut && 0 < j && j < m_cut) ||
      (j == m_cut && 0 < i && i < n_cut))
      return 4;
   if ((j == m_cut && i == 0) ||
       (i == n && j == m) ||
       (i == n_cut && j == 0))
      return 3;
   if ((i == 0 && j == m) ||
       (j == 0 && i == n))
      return 2;
   if (i == n_cut && j == m_cut)
      return 5;
   if ((m_cut < j && j < m && 0 < i && i < n) ||
       (n_cut < i && i < n && 0 < j && j < m))
      return 6;
   return -1000;
}

int get_matrix_size (int n, int m)
{
   return (m + 1) * (n + 1)
}
int get_matrix_len (int n, int m)
{
  int diag = get_matrix_size (n, m);
  int offdiag = get_num_links (n, m);
  return diag + offdiag + 1;
}

int get_index_by_i_j (int i, int j, int /*n*/, int m)
{
  return i * (m + 1) + j;
}

int create_matrix_structure (int n, int n_cut, int m, int m_cut, int *res_I)
{
  int N = get_matrix_size (n, n_cut, m, m_cut);
  int N_l = get_num_links (n, n_cut, m, m_cut);
  int len = N + 1 + N_l;

  int x[MAX_NZ], y[MAX_NZ];
  int pos = N + 1;
  int i, j;
  int q, l;
  int k = 0;
  int index;
  for (i = 0; i <= n; i++)
    {
      j = (i < n_cut)? m_cut: 0;
      for (; j <= m; j++)
        {
          res_I[k] = pos;
          l = get_links (n, n_cut, m, m_cut, i, j, x, y);
          for (q = 0; q < l; q++)
            {
              index = get_index_by_i_j (x[q], y[q], n, n_cut, m, m_cut);
              res_I[pos + q] = index;
            }
          k++;
          pos += l;
        }
    }
  res_I[N] = len;
  if (pos != len)
    return -1000;
  return 0;
}

void print_pattern (int *I, int n, int n_cut, int m, int m_cut)
{
  int N = get_matrix_size (n, n_cut, m, m_cut);
  printf ("numb_diag: %d\n", N);

  for (int i = 0; i < N; i++)
    {
      int l = I[i + 1] - I[i];
      int offset = I[i];
      printf ("elem %d row: ", i);
      for (int j = 0; j < l; j++)
        {
          printf ("%d ", I[offset + j]);
        }
      printf ("\n");
    }
}

int create_matrix_values (int n, int n_cut, int m, int m_cut, int matrix_size, double *a, int *I, int p, int k)
{
  int i, j, l1, l2;
  l1 = matrix_size * k;
  l1 /= p;
  l2 = matrix_size * (k + 1);
  l2 = l2 / p - 1;

  double err = 0;
  int pos;
  for (int l = l1; l <= l2; l++)
    {
      //по элементу l находим i, j
      get_i_j_by_index (l, i, j, n, n_cut, m, m_cut);
      a[l] = 1.;
      int num_neighbours = get_num_links (n, n_cut, m, m_cut, i, j);
#ifdef CHECK
      if (I[l + 1] - I[l] != num_neighbours)
        {
          err = -1;
          break;
        }
#endif
      pos = I[l];
      for (int q = 0; q < num_neighbours; q++)
        {
          a[pos + q] = 1. / 6;
        }
    }
  if (err < 0)
    return -1;
  return 0;
}

void get_i_j_by_index (int index, int &i, int &j, int /*n*/, int n_cut, int m, int m_cut)
{
  int m_up = m - m_cut;
  int left_bl_s = n_cut * (m_up + 1);
  if (index < left_bl_s)
    {
      i = index / (m_up + 1);
      j = index - i * (m_up + 1) + m_cut;
    }
  else
    {
      i = (index - left_bl_s) / (m + 1);
      j = (index - left_bl_s) - i * (m + 1);
      i += n_cut;
    }
}

void print_matrix (int matrix_size, double *a, int *I)
{
  int pos;
  int q;
  int pr_n = matrix_size > OUTPUT? OUTPUT: matrix_size;
  for (int i = 0; i < pr_n; i++)
    {
      pos = I[i];
      int raw_size = I[i+1] - I[i];
      for (int j = 0; j < pr_n; j++)
        {
          if (i != j)
            {
              for (q = 0; q < raw_size; q++)
                {
                  if (I[pos + q] == j)
                    {
                      printf ("%.3f ", a[pos + q]);
                      break;
                    }
                }


              if (q == raw_size)
                {
                  printf ("%.3f ", 0.);
                }
            }
          else
            {
              printf ("%.3f ", a[i]);
            }
        }
      printf ("\n");
    }
}

int check_matrix (double *a, int *I, int n, int p, int k)
{
  int i1, i2, i, j, J, error, len, l;
  i1 = k * n; i1 /= p;
  i2 = (k + 1) * n; i2 = i2 / p - 1;
  double val;
  for (i = i1; i <= i2; i++)
    {
      len = I[i + 1] - I[i];
      int offset = I[i];
      for (j = 0; j < len; j++)
        {
          J = I[offset + j];
          val = a[offset + j];
          int lenJ = I[J + 1] - I[J];
          int offsetJ = I[J];
          for (l = 0; l < lenJ; l++)
            {
              if (I[offsetJ + l] == i)
                {
                  if (fabs (val - a[offsetJ + l]) > EPS)
                    {
                      error = -2;
                    }
                  break;
                }
            }
          if (l >= lenJ)
            {
              error = -1;
              break;
            }
        }
      if (error < 0)
        break;
    }
  if (error < 0)
    return -1;
  return 0;
}

double extrapolate_func (double x, double y, double z)
{
  return y + z - x;
}

int get_func_values_neighbourhood (int i, int j, int n, int n_cut, int m, int m_cut, std::function <double(double, double)> &func, double close_vals[], double edge_vals[], double vals[])
{
  if ((m_cut < j && j < m && 0 < i && i < n) ||
  (n_cut < i && i < n && 0 < j && j < m))
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[1] = get_func_value_by_i_j (i + 1, j + 1, n, m, func);
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[3] = get_func_value_by_i_j (i,j - 1, n, m, func);
    vals[4] = get_func_value_by_i_j (i - 1, j - 1, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);
    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[1] = get_func_value_by_i_j (i + 0.5, j + 0.5, n, m, func);
    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);
    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);
    close_vals[4] = get_func_value_by_i_j (i - 0.5, j - 0.5, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);
    edge_vals[0] = get_func_value_by_i_j (i + 0.5, j + 1, n, m, func);
    edge_vals[1] = get_func_value_by_i_j (i + 1, j + 0.5, n, m, func);
    edge_vals[2] = get_func_value_by_i_j (i + 0.5, j - 0.5, n, m, func);
    edge_vals[3] = get_func_value_by_i_j (i - 0.5, j - 1, n, m, func);
    edge_vals[4] = get_func_value_by_i_j (i - 1, j - 0.5, n, m, func);
    edge_vals[5] = get_func_value_by_i_j (i - 0.5, j + 0.5, n, m, func);
    return 6;
  }
if ((i == 0 && m_cut < j && j < m) ||
    (i == n_cut && 0 < j && j < m_cut)) // II
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[1] = get_func_value_by_i_j (i + 1, j + 1, n, m, func);
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[3] = get_func_value_by_i_j (i,j - 1, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[5] = extrapolate_func (vals[1], vals[0], vals[6]) * 0;
    vals[4] = extrapolate_func (vals[2], vals[6], vals[3]) * 0;

    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[1] = get_func_value_by_i_j (i + 0.5, j + 0.5, n, m, func);
    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);
    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);

    close_vals[4] = (vals[4] + vals[6]) / 2;
    close_vals[5] = (vals[5] + vals[6]) / 2;

    edge_vals[0] = get_func_value_by_i_j (i + 0.5, j + 1, n, m, func);
    edge_vals[1] = get_func_value_by_i_j (i + 1, j + 0.5, n, m, func);
    edge_vals[2] = get_func_value_by_i_j (i + 0.5, j - 0.5, n, m, func);

    edge_vals[3] = (vals[3] + vals[4]) / 2;
    edge_vals[4] = (vals[4] + vals[5]) / 2;
    edge_vals[5] = (vals[5] + vals[0]) / 2;
    return 4;
  }
if ((0 < i && i < n_cut && j == m_cut) ||
    (n_cut < i && i < n && j == 0)) //III
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[1] = get_func_value_by_i_j (i + 1, j + 1, n, m, func);
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[3] = extrapolate_func (vals[1], vals[2], vals[6]) * 0;
    vals[4] = extrapolate_func (vals[0], vals[6], vals[5]) * 0;

    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[1] = get_func_value_by_i_j (i + 0.5, j + 0.5, n, m, func);
    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);

    close_vals[3] = (vals[6] + vals[3]) / 2;
    close_vals[4] = (vals[6] + vals[4]) / 2;

    edge_vals[0] = get_func_value_by_i_j (i + 0.5, j + 1, n, m, func);
    edge_vals[1] = get_func_value_by_i_j (i + 1, j + 0.5, n, m, func);
    edge_vals[5] = get_func_value_by_i_j (i - 0.5, j + 0.5, n, m, func);

    edge_vals[2] = (vals[2] + vals[3]) / 2;
    edge_vals[3] = (vals[3] + vals[4]) / 2;
    edge_vals[4] = (vals[4] + vals[5]) / 2;
    return 4;
  }
if (0 < i && i < n && j == m) //IV
  {
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[3] = get_func_value_by_i_j (i,j - 1, n, m, func);
    vals[4] = get_func_value_by_i_j (i - 1, j - 1, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[0] = extrapolate_func (vals[4], vals[5], vals[6]) * 0;
    vals[1] = extrapolate_func (vals[3], vals[6], vals[2]) * 0;

    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);
    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);
    close_vals[4] = get_func_value_by_i_j (i - 0.5, j - 0.5, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);

    close_vals[0] = (vals[6] + vals[0]) / 2;
    close_vals[1] = (vals[6] + vals[1]) / 2;

    edge_vals[2] = get_func_value_by_i_j (i + 0.5, j - 0.5, n, m, func);
    edge_vals[3] = get_func_value_by_i_j (i - 0.5, j - 1, n, m, func);
    edge_vals[4] = get_func_value_by_i_j (i - 1, j - 0.5, n, m, func);

    edge_vals[0] = (vals[0] + vals[1]) / 2;
    edge_vals[1] = (vals[1] + vals[2]) / 2;
    edge_vals[5] = (vals[5] + vals[0]) / 2;

    return 4;
  }
if (i == n && 0 < j && j < m) //V
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[3] = get_func_value_by_i_j (i,j - 1, n, m, func);
    vals[4] = get_func_value_by_i_j (i - 1, j - 1, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[1] = extrapolate_func (vals[5], vals[6], vals[0]) * 0;
    vals[2] = extrapolate_func (vals[4], vals[3], vals[6]) * 0;

    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);
    close_vals[4] = get_func_value_by_i_j (i - 0.5, j - 0.5, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);

    close_vals[1] = (vals[6] + vals[1]) / 2;
    close_vals[2] = (vals[6] + vals[2]) / 2;

    edge_vals[3] = get_func_value_by_i_j (i - 0.5, j - 1, n, m, func);
    edge_vals[4] = get_func_value_by_i_j (i - 1, j - 0.5, n, m, func);
    edge_vals[5] = get_func_value_by_i_j (i - 0.5, j + 0.5, n, m, func);

    edge_vals[0] = (vals[0] + vals[1]) / 2;
    edge_vals[1] = (vals[1] + vals[2]) / 2;
    edge_vals[2] = (vals[2] + vals[3]) / 2;
    return 4;
  }
if ((i == 0 && j == m_cut) || (i == n_cut && j == 0)) //VI
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[1] = get_func_value_by_i_j (i + 1, j + 1, n, m, func);
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[3] = extrapolate_func (vals[1], vals[2], vals[6]) * 0;
    vals[5] = extrapolate_func (vals[1], vals[0], vals[6]) * 0;
    vals[4] = (2 * vals[6] - vals[1]) * 0;

    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[1] = get_func_value_by_i_j (i + 0.5, j + 0.5, n, m, func);
    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);

    close_vals[3] = (vals[6] + vals[3]) / 2;
    close_vals[4] = (vals[6] + vals[4]) / 2;
    close_vals[5] = (vals[6] + vals[5]) / 2;

    edge_vals[0] = get_func_value_by_i_j (i + 0.5, j + 1, n, m, func);
    edge_vals[1] = get_func_value_by_i_j (i + 1, j + 0.5, n, m, func);

    edge_vals[2] = (vals[2] + vals[3]) / 2;
    edge_vals[3] = (vals[3] + vals[4]) / 2;
    edge_vals[4] = (vals[4] + vals[5]) / 2;
    edge_vals[5] = (vals[5] + vals[0]) / 2;
    return 3;
  }
if (i == n && j == m) // VII
  {
    vals[3] = get_func_value_by_i_j (i,j - 1, n, m, func);
    vals[4] = get_func_value_by_i_j (i - 1, j - 1, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[0] = extrapolate_func (vals[4], vals[5], vals[6]) * 0;
    vals[2] = extrapolate_func (vals[4], vals[3], vals[6]) * 0;
    vals[1] = (2 * vals[6] - vals[4]) * 0;

    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);
    close_vals[4] = get_func_value_by_i_j (i - 0.5, j - 0.5, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);

    close_vals[0] = (vals[6] + vals[0]) / 2;
    close_vals[1] = (vals[6] + vals[1]) / 2;
    close_vals[2] = (vals[6] + vals[2]) / 2;

    edge_vals[3] = get_func_value_by_i_j (i - 0.5, j - 1, n, m, func);
    edge_vals[4] = get_func_value_by_i_j (i - 1, j - 0.5, n, m, func);

    edge_vals[0] = (vals[0] + vals[1]) / 2;
    edge_vals[1] = (vals[1] + vals[2]) / 2;
    edge_vals[2] = (vals[2] + vals[3]) / 2;
    edge_vals[5] = (vals[5] + vals[0]) / 2;
    return 3;
  }

if (i == 0 && j == m) // VIII
  {
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[3] = get_func_value_by_i_j (i, j - 1, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[1] = extrapolate_func (vals[3], vals[6], vals[2]) * 0;
    vals[4] = extrapolate_func (vals[2], vals[6], vals[3]) * 0;
    vals[0] = vals[2] * 0;
    vals[5] = vals[3] * 0;

    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);
    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);

    close_vals[0] = (vals[6] + vals[0]) / 2;
    close_vals[1] = (vals[6] + vals[1]) / 2;
    close_vals[4] = (vals[6] + vals[4]) / 2;
    close_vals[5] = (vals[6] + vals[5]) / 2;

    edge_vals[2] = get_func_value_by_i_j (i + 0.5, j - 0.5, n, m, func);

    edge_vals[0] = (vals[0] + vals[1]) / 2;
    edge_vals[1] = (vals[1] + vals[2]) / 2;
    edge_vals[3] = (vals[3] + vals[4]) / 2;
    edge_vals[4] = (vals[4] + vals[5]) / 2;
    edge_vals[5] = (vals[5] + vals[0]) / 2;
    return 2;
  }

if (j == 0 && i == n)
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[1] = extrapolate_func (vals[5], vals[6], vals [0]) * 0;
    vals[4] = extrapolate_func (vals[0], vals[6], vals [5]) * 0;
    vals[2] = vals[0] * 0;
    vals[3] = vals[5] * 0;

    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);

    close_vals[1] = (vals[6] + vals[1]) / 2;
    close_vals[2] = (vals[6] + vals[2]) / 2;
    close_vals[3] = (vals[6] + vals[3]) / 2;
    close_vals[4] = (vals[6] + vals[4]) / 2;

    edge_vals[5] = get_func_value_by_i_j (i - 0.5, j + 0.5, n, m, func);
    edge_vals[0] = (vals[0] + vals[1]) / 2;
    edge_vals[1] = (vals[1] + vals[2]) / 2;
    edge_vals[2] = (vals[2] + vals[3]) / 2;
    edge_vals[3] = (vals[3] + vals[4]) / 2;
    edge_vals[4] = (vals[4] + vals[5]) / 2;
    return 2;
  }
if (i == n_cut && j == m_cut)
  {
    vals[0] = get_func_value_by_i_j (i, j + 1, n, m, func);
    vals[1] = get_func_value_by_i_j (i + 1, j + 1, n, m, func);
    vals[2] = get_func_value_by_i_j (i + 1, j, n, m, func);
    vals[3] = get_func_value_by_i_j (i,j - 1, n, m, func);
    vals[5] = get_func_value_by_i_j (i - 1, j, n, m, func);
    vals[6] = get_func_value_by_i_j (i, j, n, m, func);

    vals[4] = extrapolate_func (vals[2], vals[6], vals[3]) * 0;

    close_vals[0] = get_func_value_by_i_j (i, j + 0.5, n, m, func);
    close_vals[1] = get_func_value_by_i_j (i + 0.5, j + 0.5, n, m, func);
    close_vals[2] = get_func_value_by_i_j (i + 0.5, j, n, m, func);
    close_vals[3] = get_func_value_by_i_j (i, j - 0.5, n, m, func);
    close_vals[5] = get_func_value_by_i_j (i - 0.5, j, n, m, func);

    close_vals[4] = (vals[6] + vals[4]) / 2.;

    edge_vals[0] = get_func_value_by_i_j (i + 0.5, j + 1, n, m, func);
    edge_vals[1] = get_func_value_by_i_j (i + 1, j + 0.5, n, m, func);
    edge_vals[2] = get_func_value_by_i_j (i + 0.5, j - 0.5, n, m, func);
    edge_vals[5] = get_func_value_by_i_j (i - 0.5, j + 0.5, n, m, func);

    edge_vals[3] = (vals[3] + vals[4]) / 2;
    edge_vals[4] = (vals[4] + vals[5]) / 2;
    return 5;
  }
return -1000;
}

double func_scalar_product_basis (double close_vals[], double edge_vals[], double vals[])
{
  double sum_close = 0;
  double sum_edges = 0;
  double sum = 0;
  for (int i = 0; i < MAX_NZ; i++)
    {
      sum_close += close_vals[i];
      sum_edges += edge_vals[i];
      sum += vals[i];
    }
  return 0.25 * (3. / 2 * vals[MAX_NZ] + 5. / 6 * sum_close + 1. / 6 * sum_edges + 1. / 12 * sum);
}

void create_rhs (int n, int n_cut, int m, int m_cut, int matrix_size, std::function<double(double, double)> &func,  double *rhs, int p, int k)
{
  int i, j;
  double vals[MAX_NZ + 1];
  double close_vals[MAX_NZ];
  double edge_vals[MAX_NZ];
  int l1 = k * matrix_size;
  l1 /= p;
  int l2 = (k + 1) * matrix_size;
  l2 = l2 / p - 1;

  for (int l = l1; l <= l2; l++)
    {
      get_i_j_by_index (l, i, j, n, n_cut, m, m_cut);
      get_func_values_neighbourhood (i, j, n, n_cut, m, m_cut, func, close_vals, edge_vals, vals);
      rhs[l] = func_scalar_product_basis (close_vals, edge_vals, vals);
    }
  reduce_sum (p);
}

void print_vector (double *x, int matrix_size)
{
  int np = matrix_size > OUTPUT? OUTPUT: matrix_size;
  for (int i = 0; i < np; i++)
    {
      printf ("%.3f ", x[i]);
    }
  printf ("\n");
}

void vector_product (double v[], double u[], double r[])
{
  r[0] = u[1] * v[2] - u[2] * v[1];
  r[1] = u[2] * v[0] - u[0] * v[2];
  r[2] = u[0] * v[1] - u[1] * v[0];
}

int get_lin_func_value (double x, double y, int n, int n_cut, int m, int m_cut, double *vals, double &res)
{
   int i_l = (int)(floor (x * n));
   int j_d = (int)(floor (y * m));
   if (i_l == n)
     {
       i_l = n - 1;
     }
   if (j_d == m)
     {
       j_d = m - 1;
     }
   if (i_l >= n || j_d >= m || (i_l < n_cut && j_d < m_cut))
     {
      return -1;
     }

   double hx = 1. / n;
   double hy = 1. / m;
   double delt_x = (x - i_l * 1. / n);
   double delt_y = (y - j_d * 1. / m);

   int i;
   int j;

   double a[3];
   double b[3];
   double c[3];

   double prod[3];
   int index;
   i = i_l; j = j_d;
   index = get_index_by_i_j (i, j, n, n_cut, m, m_cut);
   if (delt_y > delt_x) // upper triangle
     {
       a[0] = 0; a[1] = 0; a[2] = vals [index];

       j++; index++;
       b[0] = 0; b[1] = hy; b[2] = vals [index];

       i++; index = get_index_by_i_j (i, j, n, n_cut, m, m_cut);
       c[0] = hx; c[1] = hy; c[2] = vals [index];
     }
   else
     {
       a[0] = 0; a[1] = 0; a[2] = vals [index];

       i++; j++; index = get_index_by_i_j (i, j, n, n_cut, m, m_cut);
       b[0] = hx; b[1] = hy; b[2] = vals [index];

       j--; index--;
       c[0] = hx; c[1] = 0; c[2] = vals [index];
     }
   b[0] = b[0] - a[0];
   b[1] = b[1] - a[1];
   b[2] = b[2] - a[2];

   c[0] = c[0] - a[0];
   c[1] = c[1] - a[1];
   c[2] = c[2] - a[2];

   vector_product (b, c, prod);

   res = 1. / prod[2] * (prod[0] * (a[0] - delt_x) + prod[1] * (a[1] - delt_y)) + a[2];
   return 0;
}

double discrepancy (int n, int n_cut, int m, int m_cut, double *vals, std::function<double (double, double)> &f, double &max, int p, int k)
{
  int matrix_size = get_matrix_size (n, n_cut, m, m_cut);
  int l1 = k * matrix_size;
  l1 /= p;
  int l2 = (k + 1) * matrix_size;
  l2 = l2 / p - 1;

  int i, j;
  double x, y;
  double dx = 1. / n;
  double dy = 1. / m;
  max = 0;

  double x_c, y_c;
  for (int l = l1; l <= l2; l++)
    {
      get_i_j_by_index (l, i, j, n, n_cut, m, m_cut);

      if (i == n || j == m)
        {
          continue;
        }
      x = i * dx;
      y = j * dy;

      double approx_val;

      x_c = x;
      y_c = y + dy / 2.;
      get_lin_func_value (x_c, y_c, n, n_cut, m, m_cut, vals, approx_val);
      max = (fabs (approx_val - f (x_c, y_c)) > max)? fabs (approx_val - f (x_c, y_c)): max;

      x_c = x + dx / 2.;
      y_c = y + dy;
      get_lin_func_value (x_c, y_c, n, n_cut, m, m_cut, vals, approx_val);
      max = (fabs (approx_val - f (x_c, y_c)) > max)? fabs (approx_val - f (x_c, y_c)): max;

      x_c = x + dx / 2.;
      y_c = y + dy / 2.;
      get_lin_func_value (x_c, y_c, n, n_cut, m, m_cut, vals, approx_val);
      max = (fabs (approx_val - f (x_c, y_c)) > max)? fabs (approx_val - f (x_c, y_c)): max;

      if ((i < n_cut && j == m_cut) || (i >= n_cut && j == 0))
        {
          x_c = x + dx / 2.;
          y_c = y;
          get_lin_func_value (x_c, y_c, n, n_cut, m, m_cut, vals, approx_val);
          max = (fabs (approx_val - f (x_c, y_c)) > max)? fabs (approx_val - f (x_c, y_c)): max;
        }

      if (i == n - 1)
        {
          x_c = x + dx;
          y_c = y + dy / 2.;
          get_lin_func_value (x_c, y_c, n, n_cut, m, m_cut, vals, approx_val);
          max = (fabs (approx_val - f (x_c, y_c)) > max)? fabs (approx_val - f (x_c, y_c)): max;
        }
    }
  reduce_max (p, &max, 1);
  return max;
}

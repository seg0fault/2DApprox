#include <pthread.h>
#include <cstdio>
#include "thread_info.h"
#include <unistd.h>
#include <window.h>
#include "thread_funcs.h"
#include "msr_matrix.h"
#include "solver.h"
#include <cmath>
#include <functional>
#include <iostream>
#include "surface.h"
#include <QVector4D>
#define EPS 1e-6
#define MAXIT 50

void fill_surface_with_vals (surface *buffer_surface, grid_info &grid_calc, int p, int k, double *vals, std::function<double (double, double)> *func = 0)
{
  int N = buffer_surface->get_point_numb ();
  int i1 = k * N; i1 /= p;
  int i2 = (k + 1) * N; i2 =i2 / p - 1;

  grid_info *grid = buffer_surface->get_grid ();
  int n = grid->n;
  int m = grid->m;
  int n_cut = grid->n_cut;
  int m_cut = grid->m_cut;

  int n_calc = grid_calc.n;
  int m_calc = grid_calc.m;
  int n_cut_calc = grid_calc.n_cut;
  int m_cut_calc = grid_calc.m_cut;

  float dx = 1./ n;
  float dy = 1. / m;
  QVector4D vec;
  for (int l = i1; l <= i2; l++)
    {
      int i, j;
      get_i_j_by_index(l, i, j, n, n_cut, m, m_cut);
      if (j >= m || i >= n)
        continue;
      float x = i * dx;
      float y = j * dy;

      double f0, f1, f2, f3;
      get_lin_func_value(x, y, n_calc, n_cut_calc, m_calc, m_cut_calc, vals, f0);
      get_lin_func_value(x + dx, y, n_calc, n_cut_calc, m_calc, m_cut_calc, vals, f1);
      get_lin_func_value(x + dx, y + dy, n_calc, n_cut_calc, m_calc, m_cut_calc, vals, f2);
      get_lin_func_value(x, y + dy, n_calc, n_cut_calc, m_calc, m_cut_calc, vals, f3);
      if (!func)
        {
          vec.setX((float) f0);
          vec.setY((float) f1);
          vec.setZ((float) f2);
          vec.setW((float) f3);
        }
      else
        {
          vec.setX(fabsf ((float) (f0 - (*func)(x, y))));
          vec.setY(fabsf ((float) (f1 - (*func)(x + dx, y))));
          vec.setZ(fabsf ((float) (f2 - (*func)(x + dx, y + dy))));
          vec.setW(fabsf ((float) (f3 - (*func)(x, y + dy))));
        }

      buffer_surface->change_triangle(i, j, vec);
      //f0 = get_lin_func_value()
    }
  reduce_sum(p);
}

void *thread_func (void *arg)
{
  thread_info* s_arg = (thread_info *) arg;
  printf ("I'm p: %d k: %d\n", s_arg->p, s_arg->k);
  while (s_arg->make_iteration)
    {
      int *structure = s_arg->structure;
      double *matrix = s_arg->matrix;
      double *rhs = s_arg->rhs;

      double *x = s_arg->x;
      double *r = s_arg->r;
      double *u = s_arg->u;
      double *v = s_arg->v;

      double *max = &(s_arg->max);

      int n = s_arg->grid.n;
      int n_cut = s_arg->grid.n_cut;
      int m = s_arg->grid.m;
      int m_cut = s_arg->grid.m_cut;
      int matrix_size = s_arg->matr_size;
      std::function <double (double, double)> *f = s_arg->f;

      int k = s_arg->k;
      int p = s_arg->p;
      int *error = &(s_arg->error);
      surface *buffer_surface = s_arg->buffer_surface;
      surface *buffer_surface_disc = s_arg->buffer_surface_disc;
      if (k == 0)
        {
          *error = create_matrix_structure (n, n_cut, m, m_cut, structure);
          //print_pattern (structure, n, n_cut, m, m_cut);
        }

      reduce_sum (p, error, 1);
      if (*error >= 0)
        {
          *error = create_matrix_values (n, n_cut, m, m_cut, matrix_size, matrix, structure, p, k);
          reduce_sum (p, error, 1);
          if (*error >= 0)
            {
              create_rhs (n, n_cut, m, m_cut, matrix_size, *f, rhs, p, k);
              *error = solve (matrix, structure, matrix_size, x, rhs, r, u, v, EPS, MAXIT, p, k);
                if (*error < 0)
                  {
                    if (k == 0) printf ("Cannot solve system!\n");
                  }
                else
                  {
                    fill_surface_with_vals(buffer_surface, (s_arg->grid), p, k, x);
                    fill_surface_with_vals(buffer_surface_disc, (s_arg->grid), p, k, x, f);
                    double disc = discrepancy (n, n_cut, m, m_cut, x, *f, *max, p, k);

                    buffer_surface_disc->set_max (float (disc));
                    buffer_surface_disc->set_min (0);
                    if (k == 0)
                      {
                        printf ("disc: %e\n", disc);
                      }
                  }
            }
        }
      synchronize(s_arg->p, s_arg->main_window, *(s_arg->c_out), *(s_arg->p_out));
    }
  return 0;
}

void reduce_sum (int p, int *a, int n)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static int *p_a;
  int i = 0;

  if (p <= 1)
    return;
  pthread_mutex_lock (&m);

  if (!p_a) p_a = a;
  else
    if (a)
      {
        for (i = 0; i < n; i++)
          p_a[i] += a[i];
      }

  t_in++;

  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);

  if (p_a && p_a != a)
    {
      for (int i = 0; i < n; i++)
        {
          a[i] = p_a[i];
        }
    }
  t_out++;


  if (t_out >= p)
    {
      p_a = 0;
      t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}

void synchronize (int p, Window *main_window, pthread_cond_t &c_out, int &t_out)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;

  pthread_mutex_lock (&m);

  t_in++;

  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);
  t_out++;


  if (t_out >= p)
    {
      t_in = 0;
      main_window->calculation_completed_emit_func ();
      //pthread_cond_broadcast (&c_out);
    }

  while (t_out <= p)
    pthread_cond_wait (&c_out, &m);

  pthread_mutex_unlock (&m);
}

void reduce_sum (int p, double *a, int n)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static double *p_a;
  int i = 0;

  if (p <= 1)
    return;
  pthread_mutex_lock (&m);

  if (!p_a) p_a = a;
  else
    if (a)
      {
        for (i = 0; i < n; i++)
          p_a[i] += a[i];
      }

  t_in++;

  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);

  if (p_a && p_a != a)
    {
      for (int i = 0; i < n; i++)
        {
          a[i] = p_a[i];
        }
    }
  t_out++;


  if (t_out >= p)
    {
      p_a = 0;
      t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}

void reduce_max (int p, double *a, int n)
{
  static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t c_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t c_out = PTHREAD_COND_INITIALIZER;

  static int t_in = 0;
  static int t_out = 0;
  static double *p_a;
  int i = 0;

  if (p <= 1)
    return;
  pthread_mutex_lock (&m);

  if (!p_a) p_a = a;
  else
    if (a)
      {
        for (i = 0; i < n; i++)
          if (p_a[i] < a[i])
            p_a[i] = a[i];
      }

  t_in++;

  if (t_in >= p)
    {
      t_out = 0;
      pthread_cond_broadcast (&c_in);
    }
  else
    while (t_in < p)
      pthread_cond_wait (&c_in, &m);

  if (p_a && p_a != a)
    {
      for (int i = 0; i < n; i++)
        {
          a[i] = p_a[i];
        }
    }
  t_out++;


  if (t_out >= p)
    {
      p_a = 0;
      t_in = 0;
      pthread_cond_broadcast (&c_out);
    }
  else
    {
      while (t_out < p)
        pthread_cond_wait (&c_out, &m);
    }
  pthread_mutex_unlock (&m);
}

#ifndef THREAD_INFO_H
#define THREAD_INFO_H
#include <pthread.h>
#include <functional>
#include "grid.h"
class Window;
class surface;
struct thread_info
{
  int p;
  int k;
  int error;
  Window *main_window;
  pthread_cond_t *c_out;
  bool make_iteration;
  int *p_out;

  grid_info grid;

  double *matrix;
  double *rhs;
  int *structure;

  double *x;
  double *r;
  double *u;
  double *v;
  double max;

  int matr_size, matr_len;

  std::function <double (double, double)> *f;
  surface *buffer_surface;
  surface *buffer_surface_disc;

};

#endif // THREAD_INFO_H

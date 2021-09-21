#ifndef THREAD_FUNCS_H
#define THREAD_FUNCS_H
#include "pthread.h"
class Window;
void *thread_func (void *arg);
void reduce_sum (int p, int *a = 0, int n = 0);
void reduce_sum (int p, double *a, int n);
void reduce_max (int p, double *a, int n);
void synchronize(int p, Window *main_window, pthread_cond_t &c_out, int &t_out);

#endif

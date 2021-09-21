#include "thread_func.h"
#include "thread_info.h"
#include <cstdio>
void *thread_func (void *arg)
{
  thread_info* s_arg = (thread_info *) arg;
  printf ("I'm p: %d k: %d\n", s_arg->p, s_arg->k);
  return 0;
}

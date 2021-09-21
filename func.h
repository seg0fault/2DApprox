#ifndef FUNC_H
#define FUNC_H

#include <cmath>

static
double f_0 (double, double)
{
 return 1;
}

static
double f_1 (double x, double)
{
  return x;
}

static
double f_2 (double , double y)
{
  return y;
}

static
double f_3 (double x, double y)
{
  return x + y;
}

static
double f_4 (double x, double y)
{
  return sqrt (x * x + y * y);
}

static
double f_5 (double x, double y)
{
  return x * x + y * y;
}

static
double f_6 (double x, double y)
{
 return exp (x * x - y * y);
}

static
double f_7 (double x, double y)
{
 return 1. / (25 * (x * x + y * y) + 1);
}

#endif // FUNC_H

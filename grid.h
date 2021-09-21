#ifndef GRID_H
#define GRID_H
#include  <QPointF>

struct grid_info
{
  int nx;
  int ny;
  std::pair<double, double> left_corner;
  std::pair<double, double> right_corner;
  QPointF u, v;

  grid_info () = default;
  grid_info (int nx_arg, int ny_arg, std::pair<double, double> left_corner_arg, std::pair<double, double> right_corner_arg)
    : nx (nx_arg), ny (ny_arg), left_corner (left_corner_arg), right_corner (right_corner_arg)
  {}
};

#endif // GRID_H

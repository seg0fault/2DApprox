#ifndef SURFACE_H
#define SURFACE_H
#include "grid.h"
#include <functional>
#include <QVector4D>

class geometry;

class surface
{
public:
  surface(const grid_info &grid, std::function <double (double, double)> &f);
  surface(const surface& obj);

  int get_point_numb ();
  grid_info *get_grid ();
  void change_triangle (int i, int j, QVector4D &vals);
  ~surface();
  geometry *get_geom ();
  void draw ();

void set_max(float max);
void set_min(float max);

float get_max ();

float get_min ();

private:
  geometry *m_geom_ptr; //owner
  grid_info m_grid;

  float max_val;
  float min_val;
  int point_numb;
};

#endif // SURFACE_H

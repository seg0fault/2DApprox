#include <QGLWidget>
#include "surface.h"
#include "geometry.h"
#include <cmath>
#include "msr_matrix.h"

#define RESOLUTION 100

# include <QPointF>
# include <QVector3D>
# include <QVector4D>

int get_triangle_pos_in_buffer_by_i_j (grid_info &grid, int i, int j)
{
  int m = grid.m;
  int n_cut = grid.n_cut;
  int m_cut = grid.m_cut;

  int pos = 0;
  if (i < n_cut)
    {
      pos = (m - m_cut) * i + (j - m_cut);
    }
  else
    {
      pos = (m - m_cut) * n_cut;
      pos += (i - n_cut) * m;
      pos += j;
    }
  return pos * 6; //up and down triangles, each consists of three points
}

int surface::get_point_numb ()
{
  return point_numb;
}

grid_info *surface::get_grid ()
{
  return &m_grid;
}

float surface::get_max ()
{
  return max_val;
}

float surface::get_min ()
{
  return min_val;
}
void surface::change_triangle (int i, int j, QVector4D &vals)
{
  QVector3D *a, *b, *c;
  int pos = get_triangle_pos_in_buffer_by_i_j(m_grid, i, j);
  a = m_geom_ptr->get_point_at_index(pos);
  b = m_geom_ptr->get_point_at_index(pos + 1);
  c = m_geom_ptr->get_point_at_index(pos + 2);
  a->setZ (vals.x ());
  b->setZ (vals.y ());
  c->setZ (vals.z ());
  QVector3D normal = QVector3D::normal(*a, *b, *c);
  *(m_geom_ptr->get_normal_at_index(pos)) = normal;
  *(m_geom_ptr->get_normal_at_index(pos + 1)) = normal;
  *(m_geom_ptr->get_normal_at_index(pos + 2)) = normal;

  a = m_geom_ptr->get_point_at_index(pos + 3);
  b = m_geom_ptr->get_point_at_index(pos + 4);
  c = m_geom_ptr->get_point_at_index(pos + 5);
  a->setZ (vals.x ());
  b->setZ (vals.z ());
  c->setZ (vals.w ());
    normal = QVector3D::normal(*a, *b, *c);
  *(m_geom_ptr->get_normal_at_index(pos + 3)) = normal;
  *(m_geom_ptr->get_normal_at_index(pos + 4)) = normal;
  *(m_geom_ptr->get_normal_at_index(pos + 5)) = normal;
}

surface::surface(const grid_info &grid, std::function <double (double, double)> &f): m_grid (grid)
{
  m_geom_ptr = new geometry ();

  double ratio, log_ratio;
  if (grid.n > RESOLUTION)
    {
      m_grid.n = grid.n;
      m_grid.n_cut = grid.n_cut;
    }
  else
    {
      ratio = (RESOLUTION * 1.) / grid.n;
      log_ratio = log2 (ratio);
      m_grid.n = grid.n * (int) (pow (2., ceil (log_ratio)));
      m_grid.n_cut = grid.n_cut * (int) (pow (2., ceil (log_ratio)));
    }

  if (grid.m > RESOLUTION)
    {
      m_grid.m = grid.m;
      m_grid.m_cut = grid.m_cut;
    }
  else
    {
      ratio = (RESOLUTION * 1.) / grid.m;
      log_ratio = log2 (ratio);
      m_grid.m = grid.m * (int) (pow (2., ceil (log_ratio)));
      m_grid.m_cut = grid.m_cut * (int) (pow (2., ceil (log_ratio)));
    }

  point_numb = get_matrix_size(m_grid.n, m_grid.n_cut, m_grid.m, m_grid.m_cut);

  int i, j;
  float dx_i = m_grid.u.x() / m_grid.n;
  float dy_i = m_grid.u.y() / m_grid.m;
  float dx_j = m_grid.v.x() / m_grid.n;
  float dy_j = m_grid.v.y() / m_grid.m;

  bool is_first = true;
  float f0, f1, f2, f3;
  for (i = 0; i < m_grid.n; i++)
    {
      if (i < m_grid.n_cut)
        j = m_grid.m_cut;
      else
        j = 0;
      for (; j < m_grid.m; j++)
        {
          float x = i * dx_i + j * dx_j;
          float y = i * dy_i + j * dy_j;
          f0 = (float) f (x, y);
          f1 = (float) f (x + dx_i, y + dy_i);
          f2 = (float) f (x + dx_i + dx_j, y + dy_i + dy_j);
          f3 = (float) f (x + dx_j, y + dy_j);

          if (is_first)
            {
              max_val = f0;
              min_val = f0;
              is_first = false;
            }


          if (f1 > max_val)
            max_val = f1;
          if (f1 < min_val)
            min_val = f1;

          if (f2 > max_val)
            max_val = f2;
          if (f2 < min_val)
            min_val = f2;

          if (f3 > max_val)
            max_val = f3;
          if (f3 < min_val)
            min_val = f3;

          if (f0 > max_val)
            max_val = f0;
          if (f0 < min_val)
            min_val = f0;

          m_geom_ptr->add_triangle ({x, y, f0}, {x + dx_i, y + dy_i, f1}, {x + dx_i + dx_j, y + dy_i + dy_j, f2});
          m_geom_ptr->add_triangle ({x, y, f0}, {x + dx_i + dx_j, y + dy_i + dy_j, f2}, {x + dx_j, y + dy_j, f3});
        }
    }
}

surface::~surface()
{
  if (m_geom_ptr)
    delete m_geom_ptr;
}

void surface::draw()
{
  if (m_geom_ptr)
    m_geom_ptr->draw ();
}

geometry *surface::get_geom ()
{
  return m_geom_ptr;
}

void surface::set_max (float max)
{
  max_val = max;
}

void surface::set_min (float min)
{
  min_val = min;
}

surface::surface(const surface& obj)
{
  max_val = obj.max_val;
  min_val = obj.min_val;
  m_grid = obj.m_grid;
  m_geom_ptr = new geometry(*(obj.m_geom_ptr));
  point_numb = obj.point_numb;
}

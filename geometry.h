#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cstdlib>

class QVector3D;

// CHECKED
class geometry
{
public:
  geometry () = default;
  ~geometry () = default;

  // Viz
  void draw () const;

  // Triangulation
  void add_triangle (const QVector3D &a, const QVector3D &b, const QVector3D &c);

  // Getters
  QVector3D *get_normal (int index);
  QVector3D *get_point (int index);

private:
  void push_back (const QVector3D &vertice, const QVector3D &normal);

private:
  std::vector<unsigned> m_indices;
  std::vector<QVector3D> m_normals;
  std::vector<QVector3D> m_vertices;
};

#endif // GEOMETRY_H

#include "geometry.h"

#include <memory>
#include <QGLWidget>

QVector3D *geometry::get_point (int index)
{
  return &m_vertices[index];
}

QVector3D *geometry::get_normal (int index)
{
  return &m_normals[index];
}

void geometry::draw () const
{
  glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
  glVertexPointer (3, GL_FLOAT, 0, m_vertices.data ());
  glNormalPointer (GL_FLOAT, 0, m_normals.data ());
  glDrawElements(GL_TRIANGLES, m_indices.size (), GL_UNSIGNED_INT, m_indices.data ());
}
void geometry::push_back (const QVector3D &vertice, const QVector3D &normal)
{
  int vertices_size = m_vertices.size ();
  m_vertices.push_back (vertice);
  m_normals.push_back (normal);
  m_indices.push_back (vertices_size);
}

void geometry::add_triangle (const QVector3D &a, const QVector3D &b, const QVector3D &c)
{
  QVector3D norm = QVector3D::normal (a, b, c);
  push_back (a, norm);
  push_back (b, norm);
  push_back (c, norm);
}

#include "glwidget.h"

#include <QtOpenGL>
#include <QWidget>
#include <QGLWidget>
#include <QPointF>
#include <memory>

#include "surface.h"
#include "grid.h"
#include "geometry.h"
#include "window.h"

#define SMALL_PAINT_AREA_SIZE 400
#define BIG_PAINT_AREA_SIZE 1000
#define SCALE_FACTOR 1.1
#define PROPORTION 1.5
#define DEPTH_EPS 1e-16

float max(float a, float b, float c )
{
   float max = ( a < b ) ? b : a;
   return ( ( max < c ) ? c : max );
}

glwidget::glwidget(grid_info grid, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_surface.reset (new surface (grid, Window::func));
    m_buffer_surface.reset (new surface (*m_surface));

    std::function<double (double, double)> null_func = [](double, double) {return 0;};
    m_surface_disc.reset (new surface (grid, null_func));
    m_buffer_surface_disc.reset (new surface (*m_surface_disc));
    current_surface = m_surface.get ();
    xRot = 0;
    yRot = 0;
    zRot = 0;
    center = QPointF ((grid.u.x () + grid.v.x ()) / 2., (grid.u.y () + grid.v.y ()) / 2.);
    scaleCoef = 1.0f;
    width = grid.u.x() + grid.v.x();
    height = grid.u.y() + grid.v.y();
    width *= PROPORTION;
    height *= PROPORTION;
    depth = m_surface->get_max () - m_surface->get_min ();
    depth *= 5;
}

glwidget::~glwidget()
{
}
surface* glwidget::get_buffer_surface ()
{
  return m_buffer_surface.get ();
}

surface* glwidget::get_buffer_surface_disc ()
{
  return m_buffer_surface_disc.get ();
}

void glwidget::swap_surfaces()
{
  m_surface.swap (m_buffer_surface);
  m_surface_disc.swap (m_buffer_surface_disc);
  if (state == FUNC)
    {
      current_surface = m_surface.get ();
    }
  else
    {
      current_surface = m_surface_disc.get ();
    }
}


QSize glwidget::minimumSizeHint() const
{
    return QSize(SMALL_PAINT_AREA_SIZE, SMALL_PAINT_AREA_SIZE);
}

void glwidget::change_cur_to_func ()
{
  current_surface = m_surface.get ();
  set_gl_ortho ();
  state = FUNC;
  updateGL ();
}

void glwidget::change_cur_to_disc ()
{
  current_surface = m_surface_disc.get ();
  set_gl_ortho ();
  state = DISC;
  updateGL ();
}

QSize glwidget::sizeHint() const
{
    return QSize(BIG_PAINT_AREA_SIZE, BIG_PAINT_AREA_SIZE);
}

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360)
        angle -= 360 * 16;
}

void glwidget::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void glwidget::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void glwidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void glwidget::initializeGL()
{
  qglClearColor(Qt::white); //clear Color buffer

  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_MULTISAMPLE);
  GLfloat lightPosition0[4] = {0, 0, current_surface->get_max () + 1, 1.0};
  glLightfv (GL_LIGHT0, GL_POSITION, lightPosition0);
  /*GLfloat lightPosition1[4] = {0, 0, m_surface->get_min () - 1, 1.0};
  glLightfv (GL_LIGHT0, GL_POSITION, lightPosition1);*/
}

void glwidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    float max_z = fabsf (current_surface -> get_max ());
    if (fabsf (current_surface->get_min ()) > max_z)
      max_z = fabsf (current_surface->get_min ());

    grid_info *grid;
    grid = m_surface->get_grid ();

    float max_w = max (fabsf (grid->u.x()), fabsf (grid->u.x() + grid->v.x ()), fabsf (grid->v.x ()));
    float max_h = max (fabsf (grid->u.y()), fabsf (grid->u.y() + grid->v.y ()), fabsf (grid->v.y ()));
    glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
    glScalef (scaleCoef / max_w, scaleCoef / max_h, scaleCoef / max_z);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_NORMAL_ARRAY);

    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable (GL_COLOR_MATERIAL);

    glDisable (GL_CULL_FACE);
    draw_axis ();
    qglColor (Qt::red);
    current_surface->draw ();

    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
}

void glwidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    set_gl_ortho ();
}

void glwidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void glwidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }

    lastPos = event->pos();
}

void glwidget::set_gl_ortho ()
{
  float depth = current_surface->get_max () - current_surface->get_min ();
  if (depth < DEPTH_EPS)
    {
      depth = 1;
    }
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#ifdef QT_OPENGL_ES_1
  glOrtho(-2, 2, -1, 1, -1, 1);
#else
  glOrtho(-2, 2, -2, 2, -2, 2);
#endif
  glMatrixMode(GL_MODELVIEW);
}

void glwidget::wheelEvent(QWheelEvent* pe)
{
  if ((pe->delta()) > 0)
    scale_inc ();
  else if ((pe->delta()) < 0)
    scale_dec ();
  updateGL();
}

void glwidget::scale_inc()
{
  scaleCoef *= SCALE_FACTOR;
}

void glwidget::scale_dec()
{
  scaleCoef /= SCALE_FACTOR;
}

void glwidget::draw_axis ()
{
  glLineWidth (3.0f);


  qglColor (Qt::green);
  glBegin (GL_LINES);
  glVertex3f ( 20.0f,  0.0f,  0.0f);
  glVertex3f (-20.0f,  0.0f,  0.0f);
  glEnd ();
  //qglColor (Green);
  qglColor (Qt::yellow);
  glBegin (GL_LINES);
  glVertex3f (0.0f,   20.0f,  0.0f);
  glVertex3f (0.0f, -20.0f,  0.0f);
  glEnd ();
  //glColor4f (0.00f, 0.00f, 1.00f, 1.0f);
  qglColor (Qt::blue);
  glBegin (GL_LINES);
  glVertex3f ( 0.0f,  0.0f,  20.0f);
  glVertex3f ( 0.0f,  0.0f, -20.0f);
  glEnd ();

}


#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <cstdlib>
#include <QObject>

class surface;
struct grid_info;

class glwidget: public QGLWidget
{
  Q_OBJECT
  enum STATE
  {
     FUNC,
     DISC,
  };
public:
    explicit glwidget(grid_info grid, QWidget *parent = 0);
    surface* get_buffer_surface ();
    surface* get_buffer_surface_disc ();
    void swap_surfaces();
    ~glwidget();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* pe);

public slots:
    // slots for xyz-rotation slider
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void change_cur_to_func ();
    void change_cur_to_disc ();

signals:
    // signaling rotation from mouse movement
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);


private:
    void draw();
    void draw_axis ();

    void scale_inc ();
    void scale_dec ();
    void set_gl_ortho ();

    int xRot;
    int yRot;
    int zRot;

    STATE state = FUNC;

    float scaleCoef;
    QPoint lastPos;

    QVector<unsigned> m_faces;
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;

    QPointF center;
    float width;
    float height;
    float depth;

    surface* current_surface;

    std::unique_ptr<surface> m_surface;
    std::unique_ptr<surface> m_buffer_surface;

    std::unique_ptr<surface> m_surface_disc;
    std::unique_ptr<surface> m_buffer_surface_disc;

};

#endif // GLWIDGET_H

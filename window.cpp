#include "window.h"
#include <QGLWidget>
#include <QtOpenGL>

#include <cstdio>
#include <cstring>
#include <atomic>
#include <functional>

#include "grid.h"
#include "pthread.h"
#include "thread_info.h"
#include "thread_funcs.h"
#include "msr_matrix.h"
#include "glwidget.h"

std::atomic <bool> is_busy (true);

int Window::init_data_for_calc ()
{
  int n = m_grid.nx;
  int m = m_grid.ny;

  double *matrix;
  double *rhs;
  int *structure;

  double *x;
  double *r;
  double *u;
  double *v;

  int matr_size = get_matrix_size (n, m);
  int matr_len = get_matrix_len (n, m);

  if (!(matrix = new double[matr_len])||
      !(structure = new int[matr_len])||
      !(x = new double[matr_size])||
      !(r = new double[matr_size])||
      !(u = new double[matr_size])||
      !(v = new double[matr_size])||
      !(rhs = new double[matr_size]))
    {
      return -1;
    }

  printf ("system size: %d\n", matr_size);
  memset (x, 0, matr_size * sizeof (double));
  memset (r, 0, matr_size * sizeof (double));
  memset (u, 0, matr_size * sizeof (double));
  memset (v, 0, matr_size * sizeof (double));
  for (int i = 0; i < p; i++)
    {
      m_infos[i].x = x;
      m_infos[i].r = r;
      m_infos[i].u = u;
      m_infos[i].v = v;

      m_infos[i].matrix = matrix;
      m_infos[i].rhs = rhs;
      m_infos[i].structure = structure;
      m_infos[i].grid = grid;

      m_infos[i].matr_len = matr_len;
      m_infos[i].matr_size = matr_size;
      m_infos[i].error = 0;
      m_infos[i].buffer_surface = buffer_surface;
      m_infos[i].buffer_surface_disc = buffer_surface_disc;
    }
  return 0;
}

void del_data_after_calc (thread_info *infos)
{
  delete [] infos[0].x;
  delete [] infos[0].r;
  delete [] infos[0].u;
  delete [] infos[0].v;
  delete [] infos[0].matrix;
  delete [] infos[0].rhs;
  delete [] infos[0].structure;
}

void Window::init_defaults ()
{
  init_args (nullptr, DEFAULT_NX, DEFAULT_NY, DEFAULT_FUNC, DEFAULT_PREC, DEFAULT_THREAD_COUNT);
}

int Window::init_args (char *filename, int nx, int ny, int func_num, double eps, int thread_count)
{
  double x1 = 0., y1 = 0., x2 = 1., y2 = 1.;

  if (filename)
    {
      FILE *fp;
      char buf[BUF_LEN];
      bool left_read = false, right_read = false;
      fp = fopen(filename, "r");

      if (!fp)
        {
        printf("Cannot open file %s\n", filename);
        return 1;
        }

      while (fgets(buf, BUF_LEN, fp))
        {
          int i = 0;
          for (; buf[i] == ' ' || buf[i] == '\t'; i++);

          if (buf[i] == '#')
            continue;

          if (!left_read && sscanf(buf, "%lf %lf", &x1, &y1) != 2)
            return 1;
          else
            left_read = true;

          if (!right_read && sscanf(buf, "%lf %lf", &x2, &y2) != 2)
            return 2;
          else
            right_read = true;
      }
    }

  m_grid.reset (new grid_info (nx, ny, {x1, y1}, {x2, y2}));

  setWindowTitle ("Kirill Yurievich please postav'te zachet");

  this->m_thread_count = thread_count;
  m_infos = new thread_info [thread_count];
  m_threads = nullptr;
  cond_out_ptr = new pthread_cond_t [1];
  cond_out_ptr[0] = PTHREAD_COND_INITIALIZER;
  m_thread_out = new int [1];
  *m_thread_out = 0;


}

Window::Window (grid_info grid, int p, QWidget *parent)
  : QWidget (parent), grid_for_calc (grid)
{
  widget = new glwidget (grid, this);
  mult_button = new QPushButton ("*=2");
  div_button = new QPushButton ("/=2");
  func_button = new QRadioButton ("func");
  disc_button = new QRadioButton ("disc");
  QVBoxLayout *v_layout = new QVBoxLayout (this);
  QHBoxLayout *h_layout = new QHBoxLayout (this);
  v_layout->addLayout (h_layout);
  h_layout->addWidget (mult_button);
  h_layout->addWidget (div_button);
  h_layout->addWidget (func_button);
  h_layout->addWidget (disc_button);
  v_layout->addWidget (widget);

  connect(mult_button, SIGNAL (pressed ()), this, SLOT (mult_button_handler ()));
  connect(div_button, SIGNAL (pressed ()), this, SLOT (div_button_handler ()));
  connect(this, SIGNAL (calculation_completed ()), this, SLOT (change_data ()));

  connect(func_button, SIGNAL (clicked ()), this, SLOT (select_func ()));
  connect(disc_button, SIGNAL (clicked ()), this, SLOT (select_disc ()));

  connect(this, SIGNAL (func_selected ()), widget, SLOT (change_cur_to_func ()));
  connect(this, SIGNAL (disc_selected ()), widget, SLOT (change_cur_to_disc ()));

  setWindowTitle ("2D approximation");

  this->p = p;
  infos = new thread_info [p];
  threads = 0;
  c_out_ptr = new pthread_cond_t [1];
  c_out_ptr[0] = PTHREAD_COND_INITIALIZER;
  p_out = new int [1];
  *p_out = 0;

  norm_func =
      new std::function <double (double, double)> ([grid]  (double u, double v) {return func (u * grid.u.x() + v * grid.v.x (), u * grid.u.y () + v * grid.v.y ());});

  for (int i = 0; i < p; i++)
    {
      infos[i].k = i;
      infos[i].p = p;
      infos[i].main_window = this;
      infos[i].c_out = c_out_ptr;
      infos[i].make_iteration = true;
      infos[i].p_out = p_out;
      infos[i].f = norm_func;
    }

  mult_button->setEnabled (false);
  div_button->setEnabled (false);

  if (init_data_for_calc(grid, infos, widget->get_buffer_surface (), widget->get_buffer_surface_disc(), p) < 0)
    {
      printf ("Cannot build initial interpolation!\n");
      is_busy = false;
    }
  else
    {
      threads = new pthread_t [p];
      for (int i = 0; i < p; i++)
        {
          pthread_create (threads + i, 0, &thread_func, infos + i);
        }
    }
}

void Window::mult_button_handler ()
{
  if (!is_busy)
    {
      mult_button->setEnabled (false);
      div_button->setEnabled (false);
      grid_for_calc.n *= 2;
      grid_for_calc.n_cut *= 2;
      grid_for_calc.m *= 2;
      grid_for_calc.m_cut *= 2;
      counter++;
      init_data_for_calc(grid_for_calc, infos, widget->get_buffer_surface(), widget->get_buffer_surface_disc (), p);
      is_busy = true;
      (*p_out)++;
      pthread_cond_broadcast (c_out_ptr);
    }

  //is_busy = true;
}

void Window::select_func ()
{
  emit func_selected ();
}

void Window::select_disc ()
{
  emit disc_selected ();
}

void Window::calculation_completed_emit_func ()
{
  emit calculation_completed ();
}

void Window::change_data ()
{
  mult_button->setEnabled (true);
  div_button->setEnabled (true);
  del_data_after_calc (infos);
  if (infos[0].error >= 0)
    {
      widget->swap_surfaces();
    }
  widget->updateGL();
  is_busy = false;
}

void Window::div_button_handler ()
{
  if (!is_busy)
    {
      if (counter > 0)
        {
          mult_button->setEnabled (false);
          div_button->setEnabled (false);
          grid_for_calc.n /= 2;
          grid_for_calc.n_cut /= 2;
          grid_for_calc.m /= 2;
          grid_for_calc.m_cut /= 2;
          counter--;
          init_data_for_calc(grid_for_calc, infos, widget->get_buffer_surface(), widget->get_buffer_surface_disc (), p);
          is_busy = true;
          (*p_out)++;
          pthread_cond_broadcast (c_out_ptr);
        }
    }
}

void Window::closeEvent(QCloseEvent *event)
{
  if (is_busy)
      event->ignore ();
  else
    {
      for (int i = 0; i < p; i++)
        {
          infos[i].make_iteration = false;
        }
      (*p_out)++;
      pthread_cond_broadcast (c_out_ptr);
      event->accept ();
    }

}

Window::~Window ()
{
  if (m_threads)
    {
     for (int i = 0; i < thread_count; i++)
       pthread_join (m_threads[i], 0);

     delete[] threads;
    }

  delete[] m_infos;
  delete[] cond_out_ptr;
  delete[] m_thread_out;
}

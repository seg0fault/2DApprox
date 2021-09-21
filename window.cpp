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
#include "window_ui.h"

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
  for (int i = 0; i < m_thread_count; i++)
    {
      m_infos[i].x = x;
      m_infos[i].r = r;
      m_infos[i].u = u;
      m_infos[i].v = v;

      m_infos[i].matrix = matrix;
      m_infos[i].rhs = rhs;
      m_infos[i].structure = structure;
      m_infos[i].grid = m_grid;

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

  m_grid = grid_info (nx, ny, {x1, y1}, {x2, y2});

  setWindowTitle ("Kirill Yurievich please postav'te zachet");

  m_thread_count = thread_count;
  m_infos = new thread_info [thread_count];
  m_threads = nullptr;
  cond_out_ptr = new pthread_cond_t [1];
  cond_out_ptr[0] = PTHREAD_COND_INITIALIZER;
  m_thread_out = new int [1];
  *m_thread_out = 0;

  norm_func = new std::function <double (double, double)> ([this]  (double u, double v)
  {
    return func (u * m_grid.u.x() + v * m_grid.v.x (), u * m_grid.u.y () + v * m_grid.v.y ());
  });

  for (int i = 0; i < m_thread_count; i++)
    {
      m_infos[i].k = i;
      m_infos[i].p = m_thread_count;
      m_infos[i].main_window = this;
      m_infos[i].c_out = cond_out_ptr;
      m_infos[i].make_iteration = true;
      m_infos[i].p_out = m_thread_out;
      m_infos[i].f = norm_func;
    }

  if (init_data_for_calc () < 0)
    {
      printf ("Cannot build initial interpolation!\n");
      is_busy = false;
    }
  else
    {
      m_threads = new pthread_t [m_thread_count];
      for (int i = 0; i < m_thread_count; i++)
        {
          pthread_create (m_threads + i, 0, &thread_func, m_infos + i);
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
      for (int i = 0; i < m_thread_count; i++)
        {
          m_infos[i].make_iteration = false;
        }
      (*m_thread_out)++;
      pthread_cond_broadcast (cond_out_ptr);
      event->accept ();
    }

}

Window::~Window ()
{
  if (m_threads)
    {
     for (int i = 0; i < m_thread_count; i++)
       pthread_join (m_threads[i], 0);

     delete[] m_threads;
    }

  delete[] m_infos;
  delete[] cond_out_ptr;
  delete[] m_thread_out;
}

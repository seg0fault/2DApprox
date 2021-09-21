#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QWidget>
#include <cstdlib>
#include <memory>

#define DEFAULT_NX 50
#define DEFAULT_NY 50
#define DEFAULT_FUNC 0
#define DEFAULT_PREC 1e-8
#define DEFAULT_THREAD_COUNT 1
#define BUF_LEN 1234

class glwidget;
class QPushButton;
class QRadioButton;
class window_ui;

struct grid_info;
struct thread_info;

class Window : public QWidget
{
  Q_OBJECT
public:
  static std::function <double (double, double)> func;
  std::function <double (double, double)> *norm_func;
public:
  Window (QWidget *parent) : QWidget (parent) {}
  ~Window ();

  // Init
  void init_defaults ();
  int init_args (char *filename, int nx, int ny, int func_num, double eps, int thread_count);

  void calculation_completed_emit_func ();

private:
  //Calculation data
  std::unique_ptr<grid_info> m_grid;

  //Threads
  pthread_t *m_threads;
  thread_info *m_infos;
  int m_thread_count;
  pthread_cond_t *cond_out_ptr;
  int *m_thread_out;

  //Interface
  std::unique_ptr<window_ui> m_ui;

private:
  int init_data_for_calc ();

  void update_menu ();
  void update_calc ();
  void update_all ();

public slots:
  void change_func (int id = -1);
  void change_method (int id = -1);
  void increase_scale ();
  void decrease_scale ();
  void increase_n ();
  void decrease_n ();
  void increase_err ();
  void decrease_err ();

  void apply_menu ();
  void reset_menu ();

signals:
  void calculation_completed ();
  void func_selected ();
  void disc_selected ();
public:
  void closeEvent(QCloseEvent *event) override;
};

#endif // WINDOW_H

#include "window_ui.h"
#include "window.h"

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QDockWidget>
#include <QGLWidget>
#include <QtOpenGL>
#include "glwidget.h"

window_ui::window_ui (Window *parent_arg)
{
  parent = parent_arg;
  menu = new QDockWidget (parent);

  menu->setFeatures (QDockWidget::NoDockWidgetFeatures);
  QWidget* multiWidget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout();

  QLabel *area_text_label = new QLabel ();
  area_text_label->setText ("Area:");
  layout->addWidget (area_text_label);

  area_label = new QLabel ();
  layout->addWidget (area_label);

  QLabel *func_label = new QLabel ();
  func_label->setText (QString ("Function:"));
  layout->addWidget (func_label);

  func_box = new QComboBox ();
  func_box->addItem (QString ("1"));
  func_box->addItem (QString ("x"));
  func_box->addItem (QString ("y"));
  func_box->addItem (QString ("x + y"));
  func_box->addItem (QString ("sqrt (x^2 + y^2)"));
  func_box->addItem (QString ("x^2 + y^2"));
  func_box->addItem (QString ("exp (x^2 - y^2)"));
  func_box->addItem (QString ("1 / (25(x^2 + y^2) + 1)"));
  layout->addWidget (func_box);

  QLabel *method_label = new QLabel ();
  method_label->setText (QString ("Method:"));
  layout->addWidget (method_label);

  method_box = new QComboBox ();
  method_box->addItem (QString ("Graph"));
  method_box->addItem (QString ("Approximation"));
  method_box->addItem (QString ("Discrepancy"));
  layout->addWidget (method_box);

  nx_box = new QSpinBox ();
  layout->addWidget (nx_box);

  ny_box = new QSpinBox ();
  layout->addWidget (ny_box);

  QLabel *err_label = new QLabel ();
  err_label->setText ("Err param:");
  layout->addWidget (err_label);

  err_box = new QSpinBox ();
  err_box->setRange (-1000000, 1000000);
  layout->addWidget (err_box);

  QLabel *f_max_text_label = new QLabel ();
  f_max_text_label->setText (QString ("|f_max|:"));
  layout->addWidget (f_max_text_label);

  f_max_label = new QLabel ();
  layout->addWidget (f_max_label);

  QLabel *elapsed_text_label = new QLabel ();
  elapsed_text_label->setText (QString ("Elapsed:"));
  layout->addWidget (elapsed_text_label);

  elapsed_label = new QLabel ();
  layout->addWidget (elapsed_label);

  layout->addStretch ();

  QPushButton *reset_button = new QPushButton ();
  reset_button->setText ("Reset");
  layout->addWidget (reset_button);

  QPushButton *apply_button = new QPushButton ();
  apply_button->setText ("Apply");
  layout->addWidget (apply_button);

  QObject::connect (apply_button, SIGNAL (released ()), this, SLOT (parent->apply_menu ()));
  QObject::connect (reset_button, SIGNAL (released ()), this, SLOT (parent->reset_menu ()));

  multiWidget->setLayout (layout);
  menu->setWidget (multiWidget);

  QVBoxLayout *graph_layout = new QVBoxLayout (parent);
  geometry_widget = new glwidget (parent_arg);
  graph_layout->addWidget (geometry_widget);
}

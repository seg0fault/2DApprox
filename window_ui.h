#ifndef WINDOW_UI_H
#define WINDOW_UI_H

class Window;

class glwidget;
class QDockWidget;
class QComboBox;
class QSpinBox;
class QLabel;

class window_ui
{
public:
  window_ui (Window *parent_arg);
  ~window_ui () = default;

  friend class Window;
private:
  Window *parent;

  glwidget *geometry_widget;
  QDockWidget *menu;
  QComboBox *func_box;
  QComboBox *method_box;
  QSpinBox *nx_box;
  QSpinBox *ny_box;
  QSpinBox *err_box;
  QLabel *area_label;
  QLabel *precision_label;
  QLabel *elapsed_label;
  QLabel *f_max_label;
};

#endif // WINDOW_UI_H

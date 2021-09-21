#include "window.h"
#include <QApplication>
#include <QMessageBox>
#include "grid.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <cmath>

int main(int argc, char *argv[])
{
  QApplication application (argc, argv);
  Window main_window;
  QMessageBox error (QMessageBox::Warning, "Input warning",
               "Mandatory arguments:\nGrid description filename (string)\nnx "
               "(int)\n ny (int)\nfunc num (int)\nprecision (real)\nthread "
               "count (int)\nDefault argument will be used.");
  if (argc != 7)
    {
      error.exec ();
      main_window.init_defaults ();
    }
  else
    {
      int nx, ny, func_num, thread_count;
      double eps;

      if (sscanf (argv[2], "%d", &nx) != 1
          || sscanf (argv[3], "%d", &ny) != 1
          || sscanf (argv[4], "%d", &func_num) != 1
          || sscanf (argv[5], "%d", &thread_count) != 1
          || sscanf (argv[6], "%lf", &eps) != 1)
        {
          error.exec ();
          main_window.init_defaults ();
        }
      else if (nx <= 0 || ny <= 0 || func_num <= 0 || thread_count <= 0 || eps <= 0.)
        {
          QMessageBox (QMessageBox::Warning, "Input error",
                      "All input numbers must be positive.\nShutting down due "
                      "to incorrect input.");
          return 1;
        }
      else if (main_window.init_args (argv[0], nx, ny, func_num, eps, thread_count))
        {
          QMessageBox (QMessageBox::Warning, "Input error",
                       "Could not open or read grid description file.\nShutting "
                       "down due to incorrect input.");
          return 2;
        }
    }

  main_window.show ();
  application.exec ();
  return 0;
}

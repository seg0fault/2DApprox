#include <cstdlib>
#include <functional>
#include <memory>

int get_links (int n, int n_cut, int m, int m_cut, int i, int j, int x[], int y[]);
int get_num_links (int n, int m, int i, int j);
int get_matrix_len (int n, int m);
int get_matrix_size (int n, int m);

int create_matrix_structure (int n, int n_cut, int m, int m_cut, int *res_I);
int create_matrix_values (int n, int n_cut, int m, int m_cut, int matrix_size, double *a, int *I, int p, int k);

int get_num_links (int n, int m);
int get_index_by_i_j (int i, int j, int /*n*/, int m);
void get_i_j_by_index (int index, int &i, int &j, int /*n*/, int n_cut, int m, int m_cut);

void print_pattern (int *I, int n, int n_cut, int m, int m_cut);
void print_matrix (int matrix_size, double *a, int *I);
void print_vector (double *x, int matrix_size);

double extrapolate_func (double x, double y, double z);
int get_func_values_neighbourhood (int i, int j, int n, int n_cut, int m, int m_cut, std::function <double(double, double)> &func, double close_vals[], double edge_vals[], double vals[]);
int get_lin_func_value (double x, double y, int n, int n_cut, int m, int m_cut, double *vals, double &res);
void create_rhs (int n, int n_cut, int m, int m_cut, int matrix_size, std::function <double(double, double)> &f, double *rhs, int p, int k);
double func_scalar_product_basis (double close_vals[], double edge_vals[], double vals[]);
void vector_product (double v[], double u[], double r[]);

int check_matrix (double *a, int *I, int n, int p, int k);

double get_func_value_by_i_j (double i, double j, int n, int m, double (*f)(double, double));
double discrepancy (int n, int n_cut, int m, int m_cut, double *val, std::function <double(double, double)> &f, double &max, int p, int k);
double get_func_value_by_i_j (double i, double j, int n, int m, double (*f)(double, double));

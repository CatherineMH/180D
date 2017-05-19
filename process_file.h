#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

void clear_buffer(float *arr, float val, int n);

double sum_arr(float *arr, int n, int i_start);

double sum_arrs(float *arr1, float *arr2, int n, int i_start);

void find_max_min_ratio(float* arr, int n_P, int* index_period_gz, float* arr_max, float* arr_min, float* ratio);

void calculate_mean(float *arr, int n_P, int* index_period_gz, float* arr_mean);

void calculate_deviation(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_variance);

void calculate_skewness(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_skewness);

void calculate_kurtosis(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_kurtosis);

void calculate_correlation_coefficient(float *arr_x, float *arr_y, int n_P, int* index_period_gz, float* correlation);

void process_file(const char *ifile_name);

#endif //PROCESS_FILE_H

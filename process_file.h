#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

/* Statistics will be normalized to the following constants in the training file */
const float PERIOD_NORM; //2 times max period
const float ACCEL_NORM; //accelerometer max
const float GYRO_NORM; //gyro max
const float RATIO_NORM; //ratio max/min
const float RATIO_NORM_AY; //ratio max/min
const float MEAN_NORM; // mean
const float DEVIATION_NORM_A; //accelerometer
const float DEVIATION_NORM_G; //gyro
const float SKEWNESS_NORM_A; //accelerometer
const float SKEWNESS_NORM_G; //gyro
const float VARIANCE_NORM_A; //accelerometer
const float VARIANCE_NORM_G; //gyro
const float KURTOSIS_NORM;
const float CORRELATION_NORM;
const float RATIO_NORM_GX_GY;

void clear_buffer(float *arr, float val, int n);

double sum_arr(float *arr, int n, int i_start);

double sum_arrs(float *arr1, float *arr2, int n, int i_start);

void find_max_min_ratio(float* arr, int n_P, int* index_period_gz, float* arr_max, float* arr_min, float* ratio);

void calculate_mean(float *arr, int n_P, int* index_period_gz, float* arr_mean);

void calculate_deviation(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_variance);

void calculate_skewness(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_skewness);

void calculate_kurtosis(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_kurtosis);

void calculate_correlation_coefficient(float *arr_x, float *arr_y, int n_P, int* index_period_gz, float* correlation);

void process_file(const char *ifile_name, int not_training, int is_turn, const char user_name[]);

#endif //PROCESS_FILE_H

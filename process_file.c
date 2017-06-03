/*
 * Contains methods for extracting features from an input .csv file
 * */

/* for file and terminal I/O */
#include <stdio.h>
/* for string manip */
#include <string.h>
/* for exit() */
#include <stdlib.h>
/* for fabsf() */
#include <math.h>
#include <stdbool.h>               //WARNING ALLOCATE MEMORY
//for real time system
#include <sys/types.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/time.h>
#include "floatfann.h"
#include <unistd.h>
#include <signal.h>

#include "ex_find_maxima_rig_zgyro.h" //find_index_maxima_gz
#include "process_file.h"
//#include "extract_test2.h"
#include "neural_nets.h"

#define BUFF_SIZE 1024

/* Statistics will be normalized to the following constants in the training file */
const float PERIOD_NORM = 3.5; //2 times max period
const float ACCEL_NORM = 16.0; //accelerometer max
const float GYRO_NORM = 2000.0; //gyro max
const float RATIO_NORM = 100; //ratio max/min
const float RATIO_NORM_AY = 10000; //ratio max/min
const float MEAN_NORM = 100; // mean
const float DEVIATION_NORM_A = 100; //accelerometer
const float DEVIATION_NORM_G = 10000; //gyro
const float SKEWNESS_NORM_A = 100; //accelerometer
const float SKEWNESS_NORM_G = 10; //gyro
const float VARIANCE_NORM_A = 100; //accelerometer
const float VARIANCE_NORM_G = 100000; //gyro
const float KURTOSIS_NORM = 100;
const float CORRELATION_NORM = 100;
const float RATIO_NORM_GX_GY = 1000;
/*
 * sets first <n> values in <*arr> to <val>
 */
void clear_buffer(float *arr, float val, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        arr[i] = val;
    }
}
/*
 * Caculates sum of first <n> samples in <*arr>
 */
double sum_arr(float *arr, int n, int i_start)
{
    int i;
    float sum;

    sum = 0.0;
    for (i=0; i<n; i++)
    {
        sum += arr[i_start];
        i_start++;
    }
    return(sum);
}

double sum_arrs(float *arr1, float *arr2, int n, int i_start)
{
    int i;
    float sum;

    sum = 0.0;
    for (i=0; i<n; i++)
    {
        sum += arr1[i_start] * arr2[i_start];
        i_start++;
    }
    return(sum);
}

void find_max_min_ratio(int segment, float* arr, int n_P, int* index_period_gz, float* arr_max, float* arr_min, float* ratio)
{
    /* find max and min values in x (accelerometer x axis) and their ratios */
    float max_v, min_v;
    int i, k;
    int start = 0; int end = 0; //the start and end indices of our segment on each stride
    int no_samples_in_stride = 0;
    int no_samples_each_segment = 0;
    for (i=0; i < n_P-1; i++)
    {
        /*initialize max and min starting values to first value in shifted index of accelerometer x axis */
        max_v = arr[index_period_gz[i]];
        min_v = max_v;

        /*find number of samples in each stride*/
        no_samples_in_stride = index_period_gz[i+1] - index_period_gz[i];
        no_samples_each_segment = no_samples_in_stride/4;

        switch(segment)
        {
            case 0:
                start = index_period_gz[i]; end = index_period_gz[i+1];
                break;
            case 1:
                start = index_period_gz[i]; end = start + no_samples_each_segment;
                break;
            case 2:
                start = index_period_gz[i]+no_samples_each_segment; end = start + no_samples_each_segment;
                break;
            case 3:
                start = index_period_gz[i]+2*no_samples_each_segment; end = start + no_samples_each_segment;
                break;
            case 4:
                start = index_period_gz[i]+2*no_samples_each_segment; end = index_period_gz[i+1];
                break;
        }

        for (k = index_period_gz[i]; k < index_period_gz[i+1]; k++) // search within one stride
        {
            if (arr[k] < min_v)
                min_v = arr[k];
            if (arr[k] > max_v)
                max_v = arr[k];
        }
        arr_max[i] = max_v;
        arr_min[i] = min_v;
        // if (min_v !=0)
        ratio[i] = max_v/min_v;
        if (ratio[i] == INFINITY) {
            ratio[i] = RATIO_NORM_GX_GY;
        }
        // else
        //   printf("Error: Division by zero in ratio");
    }

}

void calculate_mean(float *arr, int n_P, int* index_period_gz, float* arr_mean)
{
    int i, k;
    double dif =0.0;
    for (i=0; i < n_P-1; i++)
    {
        arr_mean[i] = 0.0;
        for (k = index_period_gz[i]; k < index_period_gz[i+1]; k++) // search within one stride
        {
            arr_mean[i] += arr[k];
        }
        dif = index_period_gz[i+1]-index_period_gz[i];
        //  if (dif != 0)
        arr_mean[i] = arr_mean[i]/dif;
        // else
        //   printf("Error: Division by zero in mean");
    }
}

/*
 * Caculates standard deviation of first <n> samples in <*arr>
 */
void calculate_deviation(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_variance)
{
    int i, k;
    double sum =0;
    double dif =0;
    for (i=0; i < n_P-1; i++)
    {
        sum = 0.0;
        for (k = index_period_gz[i]; k < index_period_gz[i+1]; k++) // search within one stride
        {
            sum += (arr[k]-arr_mean[i])*(arr[k]-arr_mean[i]);
        }
        dif = index_period_gz[i+1]-index_period_gz[i];
        //if (dif != 0)
        arr_variance[i] = sum/dif;
        // else
        //   printf("Error: Division by zero in deviation");
        dif = sqrt(sum/dif);
        arr_deviation[i] = dif;
    }
}

/*
 * Caculates skewness
 */
void calculate_skewness(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_skewness)
{

    int i, k;
    double sum =0.0;
    double dif =0.0;
    double result =0.0;
    for (i=0; i < n_P-1; i++)
    {
        sum = 0.0;
        for (k = index_period_gz[i]; k < index_period_gz[i+1]; k++) // search within one stride
        {
            sum += (arr[k]-arr_mean[i])*(arr[k]-arr_mean[i])*(arr[k]-arr_mean[i]);
        }
        dif = index_period_gz[i+1]-index_period_gz[i];
        result = arr_deviation[i]*arr_deviation[i]*arr_deviation[i];
        // if (dif*result != 0)
        arr_skewness[i] = sum/(dif*result);
        //else
        //  printf("Error: Division by zero in deviation");
    }
}
/*
 * Caculates kurtosis of first <n> samples in <*arr>
 */
void calculate_kurtosis(float *arr, int n_P, int* index_period_gz, float* arr_mean, float* arr_deviation, float* arr_kurtosis)
{
    //double mean = calculate_mean(arr,n, i_start);
    //double deviation = calculate_deviation(arr,n, i_start);
    int i, k;
    double sum = 0.0;
    double dif =0.0;
    double result =0.0;
    for (i=0; i < n_P-1; i++)
    {
        sum = 0.0;
        for (k = index_period_gz[i]; k < index_period_gz[i+1]; k++) // search within one stride
        {
            sum += (arr[k]-arr_mean[i])*(arr[k]-arr_mean[i])*(arr[k]-arr_mean[i])*(arr[k]-arr_mean[i]);
        }
        dif = index_period_gz[i+1]-index_period_gz[i];
        result = arr_deviation[i]*arr_deviation[i]*arr_deviation[i]*arr_deviation[i];
        //  if (dif != 0)
        arr_kurtosis[i] = sum/(dif*result);
        // else
        //   printf("Error: Division by zero in deviation");

    }
}
//correlation ax and ay
void calculate_correlation_coefficient(float *arr_x, float *arr_y, int n_P, int* index_period_gz, float* correlation)
{
    //correlation coefficient
    int num,i; //number of samples per stride
    //double *r; //correlation coefficient perstride between ax and ay axes
    double numerator, denominator,sumx, sumy, sumxx, sumyy ;

    //r = (double *) malloc(sizeof(double) * n_max_gz);
    for (i=0; i < n_P-1; i++)
    {
        num = index_period_gz[i+1] - index_period_gz[i];
        sumx = sum_arr(arr_x,num, index_period_gz[i]);
        sumy = sum_arr(arr_y,num, index_period_gz[i]);

        sumxx = sum_arrs(arr_x,arr_x,num, index_period_gz[i]);
        sumyy = sum_arrs(arr_y,arr_y,num, index_period_gz[i]);

        numerator = num*sum_arrs(arr_x,arr_y,num,index_period_gz[i]) - sumx * sumy;
        denominator = sqrt((num * sumxx - sumx*sumx)*(num * sumyy - sumy*sumy));
        correlation[i] = numerator/denominator;

        //"d_value %lf\n", correlation[i]);
    }

}

//not_training: 0 - input file is not for training; 1 - input file for training
void process_file(const char *ifile_name, int not_training, int is_turn, const char user_name[])
{
    /* Generic variables */
    int i, fd, rv;
    int activity_type = -1; //activity type returned by ANN prediction
    /*array containing the input features corresponding to each stride in file*/
    //float* input = malloc(sizeof(float) * no_input_features);
    // char *ifile_name, *integration_file_name, *ofile_period_name, *ofile_shifted_period, *normalized_period_max_min, * ofile_as_name, *ofile_ast_name ;
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    //char delete_command[1024];

    int N_SAMPLES;

    /* Variables for storing the data and storing the return values */
    float  *x, *y, *z, *gx, *gy, *gz; 	// variables for data collected from input file
    double * ta, *t;

    /* Variables for peak-trough detection */
    int *P_i_gz;
    //int *T_i_gz;    // indicies of the start of each stride
    int n_max_gz =0; 	// number of peaks
    //int n_T_gz =0;	// number of troughs

    //clear buffer
    //memset(delete_command, 0 , 1024);

    /*
     * to visualize processing of turn data later: copy input file to a different file
     * */
    char *copy_command;
    copy_command = malloc(sizeof(char) * 1024);
    memset(copy_command, 0, BUFF_SIZE);
    snprintf(copy_command, BUFF_SIZE, "cp %s test_turn.csv", ifile_name);
    if(!not_training && is_turn)
        printf("copy command: %s\n", copy_command);

    /* open the input file */
    //printf("Opening file %s\n", ifile_name);
    fp = fopen(ifile_name, "r");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                ifile_name
        );
        exit(EXIT_FAILURE);
    }
    //lock the file so no one else can use it
    fd = fileno(fp);
    flock(fd, LOCK_EX);

    /* count the number of lines in the file */
    read = getline(&line, &len, fp); //discard header of file
    N_SAMPLES = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        N_SAMPLES++;
    }

    /* go back to the start of the file so that the data can be read */
    rewind(fp);
    read = getline(&line, &len, fp); //discard header of file


    /* start reading the data from the file into the data structures */
    i = 0;
    t = (double *) malloc(sizeof(double) * N_SAMPLES);//t before
    ta = (double *) malloc(sizeof(double)* N_SAMPLES);// t after
    x = (float *) malloc(sizeof(float) * N_SAMPLES);
    y = (float *) malloc(sizeof(float) * N_SAMPLES);
    z = (float *) malloc(sizeof(float) * N_SAMPLES);
    gx = (float *) malloc(sizeof(float) * N_SAMPLES); //gyro
    gy = (float *) malloc(sizeof(float) * N_SAMPLES);
    gz = (float *) malloc(sizeof(float) * N_SAMPLES);
    int* turn_col = (int *) malloc(sizeof(int) * N_SAMPLES); //for storing 0s and 50s in 9th column of turn data

    if(N_SAMPLES < 2)
        exit(EXIT_FAILURE); //if there is no meaningful data in file, exit
    int limit = N_SAMPLES - 2;


    while ((read = getline(&line, &len, fp)) != -1 && i < limit)
    {
        /* parse the data */
        if (is_turn) {
            rv = sscanf(line, "%lf,%lf,%f,%f,%f,%f,%f,%f,%d\n", &t[i], &ta[i],
                        &x[i], &y[i], &z[i], &gx[i], &gy[i], &gz[i], &turn_col[i]);
        } else
            rv = sscanf(line, "%lf,%lf,%f,%f,%f,%f,%f,%f\n", &t[i], &ta[i],
                        &x[i], &y[i], &z[i], &gx[i], &gy[i], &gz[i]);

        if (0) //rv != 8
        {
            fprintf(stderr,
                    "%s %d \'%s\'. %s.\n",
                    "Failed to read line",
                    i,
                    line,
                    "Exiting"
            );
            exit(EXIT_FAILURE);
        }
        i++;
    }
    //close and remove the file once it is processed
    fclose(fp);

    // allocate array of indexes of peaks and troughs in gz
    P_i_gz = (int *) malloc(sizeof(int) * N_SAMPLES);
    //printf("N_SAMPLES: %d", N_SAMPLES);

    float* copy_gz = (float *) malloc(sizeof(float) * N_SAMPLES);
    float* junk_t = (float *) malloc(sizeof(float) * N_SAMPLES);
    float* junk_gz = (float *) malloc(sizeof(float) * N_SAMPLES);
    for(i = 0; i < N_SAMPLES; i++ )
    {
        copy_gz[i] = gz[i];
    }

    /*
     * segment the data into strides based on z-gyro maxima:
     * */
    find_index_maxima_gz(ifile_name, N_SAMPLES, copy_gz,t, P_i_gz, &n_max_gz);
    //printf("exited find_index_maxima\n");

    system("rm maxima.csv");
    //SEGFAULT AROUND HERE
    //printf("Leaving function find_index_maxima_gz\n");
    //printf("N_SAMPLES: %d", N_SAMPLES);
    //printf("n_max_gz: %d", n_max_gz);

    /* step5 find tau (period) and shifted indexes for period start(index_period_gz) based on gz and n_max_gz */
    int k =0;
    /* allocate array for tau (period in sec of each stride) */
    double* period_time_gz = (double *) malloc(sizeof(double) *n_max_gz);
    /* the shift in index of peaks in AS to make period start from zero */
    int index_shift_gz;
    /* an array of final SHIFTED INDEXES for the period */
    int* index_period_gz = (int *) malloc(sizeof(int) *n_max_gz);

    for (k = 0; k < n_max_gz; k++)
    {
        if (k != n_max_gz-1)
        {
            period_time_gz[k] = t[(int)P_i_gz[k+1]] - t[(int)P_i_gz[k]]; //tau (period duration)
            index_shift_gz = 0.5*((int)P_i_gz[k+1] - (int) P_i_gz[k]); // shift in period index = difference between 2 neighboring indexes of peaks divided by half
            index_period_gz[k] = (int)P_i_gz[k] + index_shift_gz;    //shift right
            //printf("\nshifted period time: %lf", t[index_period_gz[k]]);
        }
        else if (k == n_max_gz-1) //last point
        {
            index_period_gz[k] = (int)P_i_gz[k] + index_shift_gz;  //shift right
            //printf("\nshifted period time: %lf", t[index_period_gz[k]]);
        }
    }

    /*
     * create a variable to indicate whether or not current stride has a turn:
     * */
    int* is_turn_stride = (int *) malloc(sizeof(int) *n_max_gz);

    int count_50 = 0;
    int count_0 = 0;
    int j = 0;
    float percent_50 = 0; //percentage of stride that contains 50s
    if(!not_training && is_turn)
    {
        system(copy_command); //copy turn data to a different file

        //look at column with 0s and 50s to see which strides contain turns and which do not:
        for (i = 0; i < n_max_gz-1; i++) {
            count_50 = 0; count_0 = 0;
            for(j = index_period_gz[i]; j < index_period_gz[i+1]; j++)
            {
                count_50 += turn_col[j]? 1:0;
                count_0 += turn_col[j]? 0:1;
            }
            //printf("period start: %f\n", t[index_period_gz[i]]);
            percent_50 = (float)count_50/((float)count_0+(float)count_50);
            //printf("count 50: %d\n", count_50);
            //printf("count 0: %d\n", count_0);
            //printf("percent turning: %f\n", percent_50);
            is_turn_stride[i] = (percent_50 > 0.5) ? 1:0;
            //printf("is_turn_stride: %d\n", is_turn_stride[i]);
        }
    }

    //extracting features from ax, ay, az gx, gy, gz
    // period (one -index_period_gz), max (all but az, gx), min (all but az, gx), max/min (all but gx), mean (all but az and gx), st. dev (all but az, gx)
    // variance (all but az, gx), kurtosis (all but gx, az), skewness (all but gx, az), corr coefficient (one ay & ax), max_gy/max_gx (one)

    /* step6 find max and min values in x (accelerometer x axis) and their ratios */
    /* max and min values and arrays based on n_max_gz */
    //float max_v_gz, min_v_gz;



    //allocate array for max, min, and ratio max/min
    float * max_ar_ax, * min_ar_ax, * ratio_ax;
    float * max_ar_ay, * min_ar_ay, * ratio_ay;
    float * max_ar_az, * min_ar_az, * ratio_az;

    float * max_ar_gx, * min_ar_gx, * ratio_gx;
    float * max_ar_gy, * min_ar_gy, * ratio_gy;
    float * max_ar_gz, * min_ar_gz, * ratio_gz;


    /* allocate array for max and min values and ratio of max/min per stride */
    max_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    min_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    ratio_ax = (float *) malloc(sizeof(float) * n_max_gz);

    max_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    min_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    ratio_ay = (float *) malloc(sizeof(float) * n_max_gz);

    max_ar_az = (float *) malloc(sizeof(float) * n_max_gz);
    min_ar_az = (float *) malloc(sizeof(float) * n_max_gz);
    ratio_az = (float *) malloc(sizeof(float) * n_max_gz);

    max_ar_gx = (float *) malloc(sizeof(float) * n_max_gz);
    min_ar_gx = (float *) malloc(sizeof(float) * n_max_gz);
    ratio_gx = (float *) malloc(sizeof(float) * n_max_gz);

    max_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    min_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    ratio_gy = (float *) malloc(sizeof(float) * n_max_gz);

    max_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);
    min_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);
    ratio_gz = (float *) malloc(sizeof(float) * n_max_gz);

    find_max_min_ratio(0, x, n_max_gz, index_period_gz, max_ar_ax, min_ar_ax, ratio_ax);
    find_max_min_ratio(0, y, n_max_gz, index_period_gz, max_ar_ay, min_ar_ay, ratio_ay);
    find_max_min_ratio(0, z, n_max_gz, index_period_gz, max_ar_az, min_ar_az, ratio_az);

    find_max_min_ratio(0, gx, n_max_gz, index_period_gz, max_ar_gx, min_ar_gx, ratio_gx);
    find_max_min_ratio(0, gy, n_max_gz, index_period_gz, max_ar_gy, min_ar_gy, ratio_gy);
    find_max_min_ratio(0, gz, n_max_gz, index_period_gz, max_ar_gz, min_ar_gz, ratio_gz);

    //mean
    float * mean_ar_ax, * mean_ar_ay;
    float * mean_ar_gy, * mean_ar_gz;

    mean_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    mean_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    mean_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    mean_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);

    calculate_mean(x, n_max_gz, index_period_gz, mean_ar_ax);
    calculate_mean(y, n_max_gz, index_period_gz, mean_ar_ay);
    calculate_mean(gy, n_max_gz, index_period_gz, mean_ar_gy);
    calculate_mean(gz, n_max_gz, index_period_gz, mean_ar_gz);

    //deviation & variance
    float * deviation_ar_ax, * deviation_ar_ay;
    float * deviation_ar_gy, * deviation_ar_gz;
    float * variance_ar_ax, * variance_ar_ay;
    float * variance_ar_gy, * variance_ar_gz;

    deviation_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    deviation_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    deviation_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    deviation_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);

    variance_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    variance_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    variance_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    variance_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);


    calculate_deviation(x, n_max_gz, index_period_gz, mean_ar_ax, deviation_ar_ax, variance_ar_ax);
    calculate_deviation(y, n_max_gz, index_period_gz, mean_ar_ay, deviation_ar_ay, variance_ar_ay);
    calculate_deviation(gy, n_max_gz, index_period_gz, mean_ar_gy, deviation_ar_gy, variance_ar_gy);
    calculate_deviation(gz, n_max_gz, index_period_gz, mean_ar_gz, deviation_ar_gz, variance_ar_gz);

    //skewness
    float * skewness_ar_ax, * skewness_ar_ay;
    float * skewness_ar_gy, * skewness_ar_gz;

    skewness_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    skewness_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    skewness_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    skewness_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);

    calculate_skewness(x, n_max_gz, index_period_gz, mean_ar_ax, deviation_ar_ax,skewness_ar_ax);
    calculate_skewness(y, n_max_gz, index_period_gz, mean_ar_ay, deviation_ar_ay,skewness_ar_ay);
    calculate_skewness(gy, n_max_gz, index_period_gz, mean_ar_gy, deviation_ar_gy,skewness_ar_gy);
    calculate_skewness(gz, n_max_gz, index_period_gz, mean_ar_gz, deviation_ar_gz,skewness_ar_gz);

    //kurtosis
    float * kurtosis_ar_ax, * kurtosis_ar_ay;
    float * kurtosis_ar_gy, * kurtosis_ar_gz;

    kurtosis_ar_ax = (float *) malloc(sizeof(float) * n_max_gz);
    kurtosis_ar_ay = (float *) malloc(sizeof(float) * n_max_gz);
    kurtosis_ar_gy = (float *) malloc(sizeof(float) * n_max_gz);
    kurtosis_ar_gz = (float *) malloc(sizeof(float) * n_max_gz);

    calculate_kurtosis(x, n_max_gz, index_period_gz, mean_ar_ax, deviation_ar_ax,kurtosis_ar_ax);
    calculate_kurtosis(y, n_max_gz, index_period_gz, mean_ar_ay, deviation_ar_ay,kurtosis_ar_ay);
    calculate_kurtosis(gy, n_max_gz, index_period_gz, mean_ar_gy, deviation_ar_gy,kurtosis_ar_gy);
    calculate_kurtosis(gz, n_max_gz, index_period_gz, mean_ar_gz, deviation_ar_gz,kurtosis_ar_gz);

    //correlation coefficient
    float * correlation_ar_ax_ay;
    correlation_ar_ax_ay = (float *) malloc(sizeof(float) * n_max_gz);
    calculate_correlation_coefficient(x, y, n_max_gz, index_period_gz, correlation_ar_ax_ay);

    /* for (i = 0; i < n_max_gz-1; i++)
     {
         printf("%d,%f,%f,%f,%f,%f\n",
                i,
                period_time_gz[i],
                min_ar_ax[i],
                max_ar_ax[i],
                ratio_ax[i],
                correlation_ar_ax_ay[i]
                );
     }
     */

    //creating a test file with 43 fetures
    char* test_file;
    test_file = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(test_file, 0, BUFF_SIZE);

    char result[100];   // array to hold the result.
    char *out_file = "ann1_file.txt";
    strcpy(result,ifile_name); // copy string one into the result.
    strcat(result,out_file); // append string two to the result.

    snprintf(test_file, BUFF_SIZE, result);
    //snprintf(test_file, BUFF_SIZE, "Katya_test_file_%ld.txt", time(NULL));
    fp = fopen(test_file, "w");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                test_file
        );
        exit(EXIT_FAILURE);
    }
    //printf("CHECK: %d\n", n_max_gz);

    //for testing

    for (i = 0; i < n_max_gz-1; i++)
    {
        /*
     * Feed input features to neural network:
         *
         * If the file contains turn data, only save strides corresponding to turns
     * */
        if((!not_training&& is_turn && is_turn_stride[i]) || !is_turn) {
            fprintf(fp,
                    "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                    period_time_gz[i] / PERIOD_NORM,
                    min_ar_ax[i] / ACCEL_NORM, min_ar_ay[i] / ACCEL_NORM, min_ar_az[i] / ACCEL_NORM,
                    min_ar_gx[i] / GYRO_NORM, min_ar_gy[i] / GYRO_NORM, min_ar_gz[i] / GYRO_NORM,
                    max_ar_ax[i] / ACCEL_NORM, max_ar_ay[i] / ACCEL_NORM, max_ar_az[i] / ACCEL_NORM,
                    max_ar_gx[i] / GYRO_NORM, max_ar_gy[i] / GYRO_NORM, max_ar_gz[i] / GYRO_NORM,
                    ratio_ax[i] / RATIO_NORM, ratio_ay[i] / RATIO_NORM_AY, ratio_az[i] / RATIO_NORM,
                    ratio_gx[i] / RATIO_NORM, ratio_gy[i] / RATIO_NORM, ratio_gz[i] / RATIO_NORM,
                    mean_ar_ax[i] / MEAN_NORM, mean_ar_ay[i] / MEAN_NORM, mean_ar_gy[i] / MEAN_NORM,
                    mean_ar_gz[i] / MEAN_NORM,
                    deviation_ar_ax[i] / DEVIATION_NORM_A, deviation_ar_ay[i] / DEVIATION_NORM_A,
                    deviation_ar_gy[i] / DEVIATION_NORM_G, deviation_ar_gz[i] / DEVIATION_NORM_G,
                    variance_ar_ax[i] / VARIANCE_NORM_A, variance_ar_ay[i] / VARIANCE_NORM_A,
                    variance_ar_gy[i] / VARIANCE_NORM_G, variance_ar_gz[i] / VARIANCE_NORM_G,
                    skewness_ar_ax[i] / SKEWNESS_NORM_A, skewness_ar_ay[i] / SKEWNESS_NORM_A,
                    skewness_ar_gy[i] / SKEWNESS_NORM_G, skewness_ar_gz[i] / SKEWNESS_NORM_G,
                    kurtosis_ar_ax[i] / KURTOSIS_NORM, kurtosis_ar_ay[i] / KURTOSIS_NORM,
                    kurtosis_ar_gy[i] / KURTOSIS_NORM, kurtosis_ar_gz[i] / KURTOSIS_NORM,
                    correlation_ar_ax_ay[i] / CORRELATION_NORM,
                    (max_ar_gx[i] / max_ar_ay[i]) / RATIO_NORM_GX_GY,
                    (max_ar_gy[i] / max_ar_gz[i]) / RATIO_NORM,
                    (min_ar_gy[i] / min_ar_gz[i]) / RATIO_NORM
            );
        }
    }

    /*
     * feed most recently processed set of strides to the neural network to perform
     * motion and speed classification
     * */
    fclose(fp);

    //check whether the input file is the training file so that we avoid
    //performing actions specific to the real-time system:

    if(not_training)
    {
        global_neural_network(test_file, user_name);
    }

    char delete_command[1024];

    if(not_training) {
        //printf("Not training! \n");

        //clear buffer
        memset(delete_command, 0 , 1024);

        sprintf(delete_command, "rm %s", test_file);

        system(delete_command);
    }

    free(t);
    free(ta);
    free(x);
    free(y);
    free(z);
    free(gx);
    free(gy);
    free(gz);
}

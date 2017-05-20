/*For testing the data real-time*/

/* for file and terminal I/O */
#include <stdio.h>
/* for string manip */
#include <string.h>
/* for exit() */
#include <stdlib.h>
/* for fabsf() */
#include <math.h>
#include <stdbool.h>               //WARNING ALLOCATE MEMORY, WARNING REMOVING OUTPUT FILES (ann1_file.txt, description_file.txt) BEFORE LAUNCHING BECAUSE OF APPENDING
//for real time system
#include <sys/types.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/time.h>
//#include "floatfann.h"
#include <unistd.h>
#include <signal.h>

#define BUFF_SIZE 1024
#define ACTIVITY_COUNT 6
#define SPEED_COUNT 3

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

sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
        run_flag = 0;
}

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

void find_max_min_ratio(float* arr, int n_P, int* index_period_gz, float* arr_max, float* arr_min, float* ratio)
{
    float max_v, min_v;
    int i, k;
    for (i=0; i < n_P-1; i++)
    {
        /*initialize max and min starting values to first value in shifted index of accelerometer x axis */
        max_v = arr[index_period_gz[i]];
        min_v = max_v;
        
        for (k = index_period_gz[i]; k < index_period_gz[i+1]; k++) // search within one stride
        {
            if (arr[k] < min_v)
                min_v = arr[k];
            if (arr[k] > max_v)
                max_v = arr[k];
        }
        arr_max[i] = max_v;
        arr_min[i] = min_v;
            ratio[i] = max_v/min_v;
        if (ratio[i] == INFINITY) {
            ratio[i] = RATIO_NORM_GX_GY;
        }
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
            arr_mean[i] = arr_mean[i]/dif;
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
        arr_variance[i] = sum/dif;

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
            arr_skewness[i] = sum/(dif*result);
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
        arr_kurtosis[i] = sum/(dif*result);
    }
}
//correlation ax and ay
void calculate_correlation_coefficient(float *arr_x, float *arr_y, int n_P, int* index_period_gz, float* correlation)
{
    int num,i; //num - number of samples per stride
    double numerator, denominator,sumx, sumy, sumxx, sumyy ;
    
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
    }

}

 void find_index_maxima_gz(const char *ifile_name, int N_SAMPLES, float* gz, double* t, int* new_maxima_idx, int* n_max_gz)
{
    FILE *fp;
    /*find maximum values of specified data*/
    int input = -1;
    int i;
    //printf("Which data? (0 = a_x, 1 = a_y, 2 = a_z, 3 = g_x, 4 = g_y, 5 = g_z) : ");
    //input = getchar();
    //input -= '0';
    
    float *data = gz;
    
    float threshold = 0;
    
    int *maxima_idx = (int*) malloc(sizeof(int)*N_SAMPLES);
    double *maxima_time = (double *) malloc(sizeof(double) * N_SAMPLES);
    float *maxima = (float *) malloc(sizeof(float) * N_SAMPLES);
    //int *maxima_idx = (int *) malloc(sizeof(int) * N_SAMPLES);
    int num_maxima = 0; /*variable to keep track of the number of maxima*/
    
    float curr_minimum = threshold;
    int count = 0;
    int curr_min_idx = 0;
    int curr_min_idx_wave = 0;
    int new_minima_found = 0; /*boolean*/
    int first_in_region_found = 0; /*boolean*/
    int num_extrema = 0;
    
    double first_in_region_time = 0;
    float first_in_region_val = 0;
    
    float neg_slope = 0; //linear approximation of slope preceding local extrema
    float pos_slope = 0; //linear approx of slope following local extrema
    int end_count = 150; //a parameter that determines how long we go for before looking for a new maximum
    int idx = 0;
    for(; idx < N_SAMPLES; idx ++) {
        data[idx] = -1 * data[idx]; //flip data
    }
    
    for(idx = 0; idx < N_SAMPLES; idx ++)
    {
        if(data[idx]<curr_minimum && !first_in_region_found)
        {
            first_in_region_found = 1; //indicates that overall slope is changing
            curr_minimum = data[idx];
            curr_min_idx_wave = idx;
            new_minima_found = 1;
            first_in_region_time = t[idx];
            first_in_region_val = data[idx];
        }
        else if(data[idx]< curr_minimum && first_in_region_found)
        {
            curr_minimum = data[idx];
            curr_min_idx_wave = idx;
            new_minima_found = 1;
            neg_slope = (neg_slope+(data[idx]-first_in_region_val)/(t[idx]-first_in_region_time))/2;
        }
        else if(new_minima_found && data[idx] > curr_minimum && count <= end_count) //records that the function has been increasing
        {
            pos_slope = (pos_slope+(data[idx]-data[curr_min_idx_wave])/(t[idx]-t[curr_min_idx_wave]))/2;
            count = count + 1;
        }
        else if(data[idx] > curr_minimum && new_minima_found && count > end_count) //if function has been increasing, we hit the minimum
        {
            first_in_region_found = 0; //reset this for the next search region
            if(!((neg_slope > -0.007) || (pos_slope < 0.007)))
            {
                maxima_idx[curr_min_idx] = curr_min_idx_wave;
                maxima_time[curr_min_idx] = t[curr_min_idx_wave];
                maxima[curr_min_idx] = curr_minimum;
                curr_min_idx = curr_min_idx + 1;
            }
            new_minima_found = 0; //indicates that we will begin looking for a new local min
            curr_minimum = threshold;
            count = 0;
            neg_slope = 0;
            pos_slope = 0;
        }
    }
    num_extrema = curr_min_idx;
    
    //flip extrema to get maximum points:
    for(idx = 0; idx < num_extrema; idx++)
    {
        maxima[idx] = -maxima[idx];
        //printf("%f, %f \n", maxima_time[idx], maxima[idx]);
    }
    
    /*find average of all maxima*/
    float sum = 0;
    float average = 0;
    for(idx = 0; idx < num_extrema; idx++)
    {
        sum = sum + maxima[idx];
    }
    average = sum/num_extrema;
    
    //remove maximum points that are closer to zero than the average:
    //int *new_maxima_idx = (int*) malloc(sizeof(int) * N_SAMPLES);
    double *new_maxima_time = (double *) malloc(sizeof(double) * N_SAMPLES);
    float *new_maxima = (float *) malloc(sizeof(float) * N_SAMPLES);
    int new_idx = 0;
    
    //printf("Inside function find_maxima_gz. Value of num_extrema%d\n", num_extrema);
    
    for(idx = 0; idx < num_extrema; idx++)
    {
        
        //printf("average %f, %f\n", average, maxima[idx]);
        if (fabs(maxima[idx] - average) < fabs(maxima[idx] - 0))
        {
            
            new_maxima_idx[new_idx] = maxima_idx[idx];
            
            new_maxima_time[new_idx] = maxima_time[idx];
            new_maxima[new_idx] = maxima[idx];
            //printf("TEST %d, %lf" , new_maxima_idx[new_idx],new_maxima_time[new_idx] );
            new_idx = new_idx + 1;
        }
    }
  
    num_extrema = new_idx; //update number of maximum points
    
    /*
     for(idx = 0; idx < num_extrema; idx++)
     {
     printf("%f, %f \n", new_maxima_time[idx], new_maxima[idx]);
     }
     */
    char output_file_name[]= "maxima.csv";
    //printf("Attempting to write to file \'%s\'.\n", output_file_name);
    fp = fopen(output_file_name, "w");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                output_file_name
                );
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "t, max_val\n");
    for (i = 0; i < num_extrema; i++) {
        fprintf(fp, "%d,%1f,%lf\n",
                new_maxima_idx[i],
                new_maxima[i],
                new_maxima_time[i]
                );
    }
    fclose(fp);
    
    /*free memory*/
    free(maxima);
    free(maxima_time);
    free(maxima_idx);
    
    free(new_maxima);
    free(new_maxima_time);
    free(new_maxima_idx);
    
    (*n_max_gz) = num_extrema;
    //printf("Exiting find_index_maxima_gz(). Value of n_P is: %d", num_extrema);
    
}

void extract_features(const char *ifile_name, int activity_id, const int ACTIVITY_COUNT_USER)
{
	/* Generic variables */
	int i, j, idx, fd, rv;
    int no_input_features = 43; //number of input features being fed to neural network
 
    FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	
    int N_SAMPLES;

	// Variables for storing the data
	float  *x, *y, *z, *gx, *gy, *gz;
	double * ta, *t;
	
	/* Variables for peak-trough detection */
    int *P_i_gz;
    int *T_i_gz;    // indicies of the start of each stride
	int n_max_gz =0; 	// number of peaks
    int n_T_gz =0;	// number of troughs
    
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
    
    if(N_SAMPLES < 2)
        exit(EXIT_FAILURE); //if there is no meaningful data in file, exit
    int limit = N_SAMPLES - 2;
    while ((read = getline(&line, &len, fp)) != -1 && i < limit)
    {
		/* parse the data */
		rv = sscanf(line, "%lf,%lf,%f,%f,%f,%f,%f,%f\n", &t[i], &ta[i],
			       	&x[i], &y[i], &z[i], &gx[i], &gy[i], &gz[i]);
		
		if (rv != 8)
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
    
    //Catherine's code for stride detection

    //printf("Going into function find_index_maxima_gz\n");
    find_index_maxima_gz(ifile_name, N_SAMPLES, gz,t, P_i_gz, &n_max_gz);

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
        if (k == n_max_gz-1) //last point
        {
            index_period_gz[k] = (int)P_i_gz[k] + index_shift_gz;  //shift right
            //printf("\nshifted period time: %lf", t[index_period_gz[k]]);
        }
    }
    
    //extracting features from ax, ay, az gx, gy, gz
    // period (one -index_period_gz), max (all but az, gx), min (all but az, gx), max/min (all but gx), mean (all but az and gx), st. dev (all but az, gx)
    // variance (all but az, gx), kurtosis (all but gx, az), skewness (all but gx, az), corr coefficient (one ay & ax), max_gy/max_gx (one)
  
    
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

    find_max_min_ratio(x, n_max_gz, index_period_gz, max_ar_ax, min_ar_ax, ratio_ax);
    find_max_min_ratio(y, n_max_gz, index_period_gz, max_ar_ay, min_ar_ay, ratio_ay);
    find_max_min_ratio(z, n_max_gz, index_period_gz, max_ar_az, min_ar_az, ratio_az);

    find_max_min_ratio(gx, n_max_gz, index_period_gz, max_ar_gx, min_ar_gx, ratio_gx);
    find_max_min_ratio(gy, n_max_gz, index_period_gz, max_ar_gy, min_ar_gy, ratio_gy);
    find_max_min_ratio(gz, n_max_gz, index_period_gz, max_ar_gz, min_ar_gz, ratio_gz);
    
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
    
    //creating a test file with 43 fetures
    char* test_file;
    test_file = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(test_file, 0, BUFF_SIZE);

    char result[100];   // array to hold the result.
    char *out_file = "ann_training_file.txt";
    strcpy(result, out_file); // copy string one into the result.
  //  strcat(result,out_file); // append string two to the result.

    snprintf(test_file, BUFF_SIZE, result);
    //snprintf(test_file, BUFF_SIZE, "Katya_test_file_%ld.txt", time(NULL));
    fp = fopen(test_file, "a"); //"a"
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
     * */

        fprintf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                period_time_gz[i]/PERIOD_NORM,
                min_ar_ax[i]/ACCEL_NORM, min_ar_ay[i]/ACCEL_NORM, min_ar_az[i]/ACCEL_NORM, min_ar_gx[i]/GYRO_NORM, min_ar_gy[i]/GYRO_NORM, min_ar_gz[i]/GYRO_NORM,
                max_ar_ax[i]/ACCEL_NORM, max_ar_ay[i]/ACCEL_NORM, max_ar_az[i]/ACCEL_NORM, max_ar_gx[i]/GYRO_NORM, max_ar_gy[i]/GYRO_NORM, max_ar_gz[i]/GYRO_NORM,
                ratio_ax[i]/RATIO_NORM,  ratio_ay[i]/RATIO_NORM_AY,  ratio_az[i]/RATIO_NORM,  ratio_gx[i]/RATIO_NORM,  ratio_gy[i]/RATIO_NORM,  ratio_gz[i]/RATIO_NORM,
                mean_ar_ax[i]/MEAN_NORM,       mean_ar_ay[i]/MEAN_NORM,       mean_ar_gy[i]/MEAN_NORM,       mean_ar_gz[i]/MEAN_NORM,
                deviation_ar_ax[i]/DEVIATION_NORM_A,  deviation_ar_ay[i]/DEVIATION_NORM_A,  deviation_ar_gy[i]/DEVIATION_NORM_G,  deviation_ar_gz[i]/DEVIATION_NORM_G,
                variance_ar_ax[i]/VARIANCE_NORM_A, variance_ar_ay[i]/VARIANCE_NORM_A, variance_ar_gy[i]/VARIANCE_NORM_G, variance_ar_gz[i]/VARIANCE_NORM_G,
                skewness_ar_ax[i]/SKEWNESS_NORM_A,   skewness_ar_ay[i]/SKEWNESS_NORM_A,   skewness_ar_gy[i]/SKEWNESS_NORM_G,   skewness_ar_gz[i]/SKEWNESS_NORM_G,
                kurtosis_ar_ax[i]/KURTOSIS_NORM, kurtosis_ar_ay[i]/KURTOSIS_NORM, kurtosis_ar_gy[i]/KURTOSIS_NORM, kurtosis_ar_gz[i]/KURTOSIS_NORM,
                correlation_ar_ax_ay[i]/CORRELATION_NORM,
                (max_ar_gx[i]/max_ar_ay[i])/RATIO_NORM_GX_GY,
                (max_ar_gy[i]/max_ar_gz[i])/RATIO_NORM,
                (min_ar_gy[i]/min_ar_gz[i])/RATIO_NORM
                );
		//-1 -1 1 -1 -1 -1
		//activity_id = 2, ACTIVITY_COUNT = 6
		for ( j = 0; j < activity_id; j++) {
			fprintf(fp, "-1 ");
		}
		fprintf(fp, "1");
		if (activity_id != ACTIVITY_COUNT_USER - 1) {
			fprintf(fp, " ");
		}
		for ( j = activity_id + 1; j < ACTIVITY_COUNT_USER; j++) {
			if (j == ACTIVITY_COUNT_USER - 1) {
				fprintf(fp, "-1");
			} else {
				fprintf(fp, "-1 ");
			}
		}
        fprintf(fp, "\n");
    }
    /*
     * feed most recently processed set of strides to the neural network to perform
     * motion and speed classification
     * */
    fclose(fp);

	char desc_file_name[] = "description_file.txt";
    FILE * desc_file;
	desc_file = fopen(desc_file_name, "a");
	if (desc_file == NULL) {
		fprintf(stderr,
				"Failed to write to file \'%s\'.\n",
				desc_file_name
				);
		exit(EXIT_FAILURE);
	}
	fprintf(desc_file, "%d %s %d\n", activity_id, ifile_name, n_max_gz);
	fclose(desc_file);

    //global_neural_network(test_file);

    char delete_command[1024];
    //clear buffer
    memset(delete_command, 0 , 1024);

    
    sprintf(delete_command, "rm %s", test_file);
    //system(delete_command);

    free(t);
    free(ta);
    free(x);
    free(y);
    free(z);
    free(gx);
    free(gy);
    free(gz);

    //testing stride detection t_gz_AV_AS_file
    /*char* t_gz_AV_AS_file;
    t_gz_AV_AS_file = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(t_gz_AV_AS_file, 0, BUFF_SIZE);
    snprintf(t_gz_AV_AS_file, BUFF_SIZE, "t_gz_AV_AS_file.csv");

    fp = fopen(t_gz_AV_AS_file, "w");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                t_gz_AV_AS_file
                );
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < N_SAMPLES; i++)
    {
        fprintf(fp, "%f,%f,%f,%f,%d\n",
                t[i],
                x[i],
                gz[i]);
    }
    fclose(fp); */


    //file for troughs t_AS_file
    /*char* t_AS_file;
    t_AS_file = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(t_AS_file, 0, BUFF_SIZE);
    snprintf(t_AS_file, BUFF_SIZE, "t_AS_file.csv");
    
    fp = fopen(t_AS_file, "w");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                t_AS_file
                );
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < N_SAMPLES; i++)
    {
        fprintf(fp, "%f,%f,%f,%f\n",
                t[i],
                gz[i]);
    }
    fclose(fp);
    */

    //printf("extract_stride_data completed successfuly. Exiting.\n");
}
//create a function for making training files fo any ANN
void make_training_file(const int ACTIVITY_NUM)
{
    char ** inputFiles[ACTIVITY_NUM];
    int inputCounts[ACTIVITY_NUM];
    int i, j;
    for ( i = 0; i < ACTIVITY_NUM; i++)
    {
        printf("How many files do you have for acitivity type number %d? ", i);
        scanf("%d", &(inputCounts[i]));
        while(getchar() != '\n');
        inputFiles[i] = (char **) malloc(sizeof(void*) * inputCounts[i]);
        //printf("inputCounts %d\n", inputCounts[i]);
        for ( j = 0; j < inputCounts[i]; j++)
        {
            size_t len = 1024;
            inputFiles[i][j] = (char *) malloc(sizeof(char) * len);
            memset(inputFiles[i][j], 0, len);
            len = getline(&(inputFiles[i][j]), &len, stdin);
            inputFiles[i][j][len - 1] = 0;
            //printf("@inputFile %s@\n", inputFiles[i][j]);
        }
    }
    //printf("HELLO\n");
    for ( i = 0; i < ACTIVITY_NUM; i++) {
        for ( j = 0; j < inputCounts[i]; j++) {
            extract_features(inputFiles[i][j], i, ACTIVITY_NUM); // return the features
        }
    }
    //printf("HELLO2\n");
    for ( i = 0; i < ACTIVITY_NUM; i++) {
        for ( j = 0; j < inputCounts[i]; j++) {
            free(inputFiles[i][j]);
        }
        free(inputFiles[i]);
    }
    
}
int main()
{
    //Global ANN training files
    /*char ** inputFiles[ACTIVITY_COUNT];
    int inputCounts[ACTIVITY_COUNT];
    for ( i = 0; i < ACTIVITY_COUNT; i++) {
        printf("How many files do you have for acitivity type number %d? ", i);
        scanf("%d", &(inputCounts[i]));
		while(getchar() != '\n');
        inputFiles[i] = (char **) malloc(sizeof(void*) * inputCounts[i]);
		//printf("inputCounts %d\n", inputCounts[i]);
        for ( j = 0; j < inputCounts[i]; j++) {
            size_t len = 1024;
            inputFiles[i][j] = (char *) malloc(sizeof(char) * len);
            memset(inputFiles[i][j], 0, len);
            len = getline(&(inputFiles[i][j]), &len, stdin);
			inputFiles[i][j][len - 1] = 0;
			//printf("@inputFile %s@\n", inputFiles[i][j]);
        }
    }
    //printf("HELLO\n");
    for ( i = 0; i < ACTIVITY_COUNT; i++) {
        for ( j = 0; j < inputCounts[i]; j++) {
            extract_features(inputFiles[i][j], i); // return the features
        }
    }
      //printf("HELLO2\n");
    for ( i = 0; i < ACTIVITY_COUNT; i++) {
        for ( j = 0; j < inputCounts[i]; j++) {
            free(inputFiles[i][j]);
        }
        free(inputFiles[i]);
    }
    */
    //create a function for making training files fo any ANN
    make_training_file(SPEED_COUNT);
    
    
    //activity type: 0-Walk, 1-Stair Ascend, 2-Stairs Descend, 3-Jump, 4-Turn Left, 5-Turn Right
    
    //ask if need train files
    //how many for walking , create an array of files
    //get their names from the user
    //continue through alll activities
    //process all files
    //create a single training file
    //create a .txt description file with activity, file name, number of strides
    return EXIT_SUCCESS;
}

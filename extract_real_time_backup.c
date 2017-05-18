/*For testing the data real-time*/

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
 * SPEED DETECTION NEURAL NETWORKS FOR EACH OF THE DIFFERENT MOTION TYPES:
 * ann_1 - walking
 * ann_2 - upstairs
 * ann_3 - downstairs
 * ann_4 - jumping
 *
 * */

/*
 * ann_1: walking speeds
 * 1 = slow pace, 2 = medium pace, 3 = fast pace
 * */
int ann_1(float* input)
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    ann = fann_create_from_file("ANN_1.net");

    calc_out = fann_run(ann, input);
    max = -100;
    for (i = 0; i < 3; i++) {
        if (calc_out[i] > max) {
            max = calc_out[i];
            motion = i;
        }
    }

//    printf(" 0 - Walk Slow, 1 - Walk Medium, 2 - Walk Fast. PREDICTED-%d (%f, %f, %f)\n", motion,
//           calc_out[0], calc_out[1], calc_out[2]);

    switch(motion)
    {
        case 0:
            printf("SLOW WALK\n");
            return 0;
        case 1:
            printf("MEDIUM WALK\n");
            return 1;
        case 2:
            printf("FAST WALK\n");
            return 2;
        default:
            printf("Error!");
    }

    fann_destroy(ann);
    return -1;
}


/*
 * ann_2: upstairs speeds
 * 1 = slow pace, 2 = medium pace, 3 = fast pace
 * */
int ann_2(float* input)
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    ann = fann_create_from_file("ANN_2.net");

    calc_out = fann_run(ann, input);
    max = -100;
    for (i = 0; i < 3; i++) {
        if (calc_out[i] > max) {
            max = calc_out[i];
            motion = i;
        }
    }

//    printf(" 0 - Stair Ascent Slow, 1 - Stair Ascent Medium, 2 - Stair Ascent Fast. PREDICTED-%d (%f, %f, %f)\n", motion,
//           calc_out[0], calc_out[1], calc_out[2]);

    switch(motion)
    {
        case 0:
            printf("SLOW UPSTAIRS\n");
            return 0;
        case 1:
            printf("MEDIUM UPSTAIRS\n");
            return 1;
        case 2:
            printf("FAST UPSTAIRS\n");
            return 2;
        default:
            printf("Error!");
    }

    fann_destroy(ann);
    return -1;
}

/*
 * ann_3: downstairs speeds
 * 1 = slow pace, 2 = medium pace, 3 = fast pace
 * */
int ann_3(float* input)
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    ann = fann_create_from_file("ANN_3.net");

    calc_out = fann_run(ann, input);
    max = -100;
    for (i = 0; i < 3; i++) {
        if (calc_out[i] > max) {
            max = calc_out[i];
            motion = i;
        }
    }

    switch(motion)
    {
        case 0:
            printf("SLOW DOWNSTAIRS\n");
            return 0;
        case 1:
            printf("MEDIUM DOWNSTAIRS\n");
            return 1;
        case 2:
            printf("FAST DOWNSTAIRS\n");
            return 2;
        default:
            printf("Error!");
    }

    fann_destroy(ann);
    return -1;
}

/*
 * ann_4: jump heights
 * 1 = short jump, 2 = medium jump, 3 = high jump
 * */
ann_4(float* input)
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    ann = fann_create_from_file("ANN_4.net");

    calc_out = fann_run(ann, input);
    max = -100;
    for (i = 0; i < 3; i++) {
        if (calc_out[i] > max) {
            max = calc_out[i];
            motion = i;
        }
    }

    switch(motion)
    {
        case 0:
            printf("SHORT JUMP\n");
            return 0;
        case 1:
            printf("MEDIUM JUMP\n");
            return 1;
        case 2:
            printf("HIGH JUMP\n");
            return 2;
        default:
            printf("Error!");
    }

    fann_destroy(ann);
    return -1;
}

int global_neural_network(const char* ifile_name)
{
    int i, activity_type,rv,fd;
    float max;
    fann_type *calc_out;
    fann_type input[43]; //43
    struct fann *ann;

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    /* open the input file */
  printf("Opening file %s\n", ifile_name);
    fp = fopen(ifile_name, "r");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                ifile_name
                );
        exit(EXIT_FAILURE);
    }
    
    //lock the file so no one else can use it
    //fd = fileno(fp);
    //flock(fd, LOCK_EX);
    //ifile_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
    ann = fann_create_from_file("RESULT_ANN1.net");
   
    
    //double a_inputs[43];
    
    while ((read = getline(&line, &len, fp)) != -1)
    {
        max = -100;
        rv = sscanf(line, "%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f\n", &input[0], &input[1], &input[2], &input[3], &input[4], &input[5], &input[6], &input[7], &input[8], &input[9], &input[10], &input[11], &input[12], &input[13], &input[14], &input[15], &input[16], &input[17], &input[18], &input[19], &input[20], &input[21], &input[22], &input[23], &input[24], &input[25], &input[26], &input[27], &input[28], &input[29], &input[30], &input[31], &input[32], &input[33], &input[34], &input[35], &input[36], &input[37], &input[38], &input[39], &input[40], &input[41], &input[42]); //&a_inputs[1] 43 times
       
        calc_out = fann_run(ann, input);
        
        for (i = 0; i < 6; i++) {
            if (calc_out[i] > max) {
                max = calc_out[i];
                activity_type = i; //type of motion detected = i;
            }
        }
        /*
         * 0 - Walking, 1 - Upstairs, 2 - Downstairs, 3 - Jump, 4 - Left Turn, 5 - Right Turn
         *
         * */
       printf("PREDICTED-%d (%f, %f, %f, %f, %f, %f)\n", activity_type,
               calc_out[0], calc_out[1], calc_out[2], calc_out[3], calc_out[4], calc_out[5]);

        switch(activity_type)
        {
            case 0:
                ann_1(input);
                break;
            case 1:
                ann_2(input);
                break;
            case 2:
                ann_3(input);
                break;
            case 3:
                ann_4(input);
                break;
            case 4:
                printf("Left turn\n");
                break;
            case 5:
                printf("Right turn\n");
                break;
            default:
                printf("Error!");
        }
        sleep(1); // might need to reduce
    }
    fann_destroy(ann);
    return activity_type;
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
    /* find max and min values in x (accelerometer x axis) and their ratios */
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
        
        printf("d_value %lf\n", correlation[i]);
    }

}
//find_index_maxima_gz(ifile_name, N_SAMPLES, gz,t, P_i_gz, n_max_gz)

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
        printf("%f, %f \n", maxima_time[idx], maxima[idx]);
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
    
    printf("Inside function find_maxima_gz. Value of num_extrema%d\n", num_extrema);
    
    for(idx = 0; idx < num_extrema; idx++)
    {
        
        printf("average %f, %f\n", average, maxima[idx]);
        if (fabs(maxima[idx] - average) < fabs(maxima[idx] - 0))
        {
            
            new_maxima_idx[new_idx] = maxima_idx[idx];
            
            new_maxima_time[new_idx] = maxima_time[idx];
            new_maxima[new_idx] = maxima[idx];
            printf("TEST %d, %lf" , new_maxima_idx[new_idx],new_maxima_time[new_idx] );
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
    printf("Attempting to write to file \'%s\'.\n", output_file_name);
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

/*
 * Finds max and min in <*arr> of <n> samples. <*arr> is the second derivative of <*gz_arr>
 */
/*
void local_max_min(float * gz_arr, //gyro z signal
                   float * arr,    //second integral of gyro z in which max and min are found
                   int n_samples,
                   int *T, int *P,
                   int *n_T, int *n_P, // number of throughs and peaks
                   bool *binary_sig) // <*arr> coded in binary (empty)
{
    int i = 0;
    int counter =0;
    int slope_change_ind = -1;
    int MIN_NUMBER = 100; //min number of points
    float dif; //diference between values
    
    int ind_P, ind_T;
    ind_P = ind_T = 0;
    *n_P = *n_T = 0;
    
    
   while (i != n_samples-1) // slope positive = 1, negative = 0
    {
        dif = arr[i+1] - arr[i];
        if (dif >= 0)
            binary_sig[i] = true;
        else
            binary_sig[i] = false;
        printf("AS[i+1]:%f\n", arr[i+1]);
        printf("AS[i]:%f\n", arr[i]);
        printf("DIF %f:\n",dif);
        printf("BINARY_SIGNAL %d:\n", binary_sig[i]);
        i++;
    }
  
   i = 0;
    do
    {
        if (binary_sig[i] == true) // first encounter 1's
        {
            while( (binary_sig[i] == true) && (i != n_samples))
            {
                counter++; // needed for noise to make sure # of 1 is greater than 100
                i++;
            }
            slope_change_ind = i-1; //index
            if ((counter > MIN_NUMBER) && (binary_sig[i] == false) && (i != n_samples)) //found max
            {
                if ( ((*n_P) == 0) || ( ((*n_P) >= 1) && (gz_arr[i]/gz_arr[P[ind_P-1]]> 0.5) ))
                {
                    P[ind_P] = slope_change_ind;
                    ind_P++;
                    (*n_P)++;
                }
            }
           // printf("\nn_P: ,%d", *n_P);
        }
        if ((binary_sig[i] == false) && (i != n_samples)) //first encounter 0's
        {
            counter = 0;
            slope_change_ind = 0;
            while ((binary_sig[i] == false) && (i != n_samples))
            {
                counter++;
                slope_change_ind = i;
                i++;
            }
           // printf("\ncounter: ,%d", counter);

            if ((counter > MIN_NUMBER) && (binary_sig[i] == true) && (i != n_samples)) //found min
            {
                if ( ((*n_T) == 0) || ( ((*n_T) >= 1) && (gz_arr[i]/gz_arr[T[ind_T-1]]> 0.5) ))
                {
                   // printf("\nindex: ,%d", T[ind_T]);
                    //printf("\ngz: ,%f", gz_arr[T[8]]);
                    T[ind_T] = slope_change_ind;
                    ind_T++;
                    (*n_T)++;
                }
            }
          //  printf("\nn_T: ,%d", *n_T);

        }
    }while (i != n_samples);
}


*/

void process_file(const char *ifile_name)
{
	/* Generic variables */
	int i, j, idx, fd, rv;
    int no_input_features = 43; //number of input features being fed to neural network

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
    int *T_i_gz;    // indicies of the start of each stride
	int n_max_gz =0; 	// number of peaks
    int n_T_gz =0;	// number of troughs
    
    //clear buffer
    //memset(delete_command, 0 , 1024);
    

	/* open the input file */
    printf("Opening file %s\n", ifile_name);
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
   
    
    /*gz_positive = (float *) malloc(sizeof(float) * N_SAMPLES);
    for ( j = 1; j < N_SAMPLES; j++)
    {
        if ( gz[j] < 0)
            gz_positive[j] = 0;
        else
            gz_positive[j] = gz[j];
           
    }
    
    /* integration of gz
    j =0;
    /* allocate array for first and second derivatives
    float *AV = (float *) malloc(sizeof(float) * N_SAMPLES);
    float *AS = (float *) malloc(sizeof(float) * N_SAMPLES);
    AV[0] = AS[0] = 0;
    /* step1 first integration of gz gyroscope
    for ( j = 1; j < N_SAMPLES; j++)
    {
        AV[j] += AV[j-1] + gz[j]*(t[j]-t[j-1]);
    }
     /* step2 second integration of gz gyroscope
    for (j = 1; j < N_SAMPLES; j++)
    {
        AS[j] += AS[j-1] + AV[j]*(t[j]-t[j-1]);
    }
    /* step3 amplification of both signals for better visualization by some constants
    for ( j = 1; j < N_SAMPLES; j++)
    {
        AV[j] = 20* AV[j]; //amplify signal
        AS[j] = 50* AS[j]; //amplify signal
    }
    */
    /* allocate a signal array for lacal_max_min function that stores 1 for positive slope and 0 for negative slope
    bool binary_sig[N_SAMPLES]; */
    // allocate array of indexes of peaks and troughs in gz
    P_i_gz = (int *) malloc(sizeof(int) * N_SAMPLES);
    printf("N_SAMPLES: %d", N_SAMPLES);
   // T_i_gz = (int *) malloc(sizeof(int) * N_SAMPLES);
    /* step4 find number(n_max_gz, n_T_gz) and indexes(P_i_gz, T_i_gz) of peaks and throughs in AS
    local_max_min(
                  gz,
                  AV,
                  N_SAMPLES,
                  P_i_gz, T_i_gz,
                  &n_max_gz, &n_T_gz,
                  binary_sig);
    
    
    
    */
    //Catherine's code for stride detection
    printf("Going into function find_index_maxima_gz\n");
    find_index_maxima_gz(ifile_name, N_SAMPLES, gz,t, P_i_gz, &n_max_gz);
    printf("Going from function find_index_maxima_gz\n");
    
    
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
    printf("CHECK: %d\n", n_max_gz);
    
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
    }

    /*
     * feed most recently processed set of strides to the neural network to perform
     * motion and speed classification
     * */
    fclose(fp);
    global_neural_network(test_file);

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

int main()
{
    FILE *fp;
    int fd;

    char *ifile_name;
    ifile_name = malloc(sizeof(char) * 1024);
    //printf("enter input file name: ");
    //scanf("%s", ifile_name);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char delete_command[1024];
    //clear buffer
    memset(delete_command, 0 , 1024);
    int activity_type = -1;

    //global_neural_network(ifile_name);
    //process_file(ifile_name);
    
    while(1)
    {
        /*
         * check for files created by producer:
         * search directory for files starting with 'data_14', which indicates
         * a newly generated data file
         */
        system("ls -1 data_14*.csv > files_to_process.txt");


        //open file with file names we need to process:
        printf("Opening file: files_to_process.txt to read each file\n");
        fp = fopen("files_to_process.txt", "r");
        if (fp == NULL) {
            fprintf(stderr,"Failed to read from file names in 'files_to_process.txt'");
            exit(EXIT_FAILURE);
        }

        /*
         * One at a time, process each of the files found in files_to_process.txt:
         * */
        while((read = getline(&line, &len, fp) != -1)){
            //set ifile_name to null:
            memset(ifile_name, 0, 1024);

            //printf("Discovered file %s", line);

            //set ifile_name to the current line of files_to_process.txt:
            sprintf(ifile_name, line);

            //remove newline character:
            ifile_name[strlen(ifile_name)-1] = '\0';

            //Extract features from the first file
            //printf("Processing file %s\n", ifile_name);
            process_file(ifile_name); // return the features
            //delete processed file
            //printf("Deleting file %s\n", ifile_name);
            sprintf(delete_command, "rm %s", ifile_name);
            system(delete_command);

            //activity type: 0-Walk, 1-Stair Ascend, 2-Stairs Descend, 3-Jump, 4-Turn Left, 5-Turn Right
        }
        fclose(fp);
        //printf("system(\"rm files_to_process.txt\");\n");
        system("rm files_to_process.txt");
        sleep(2);

    }
   
    return EXIT_SUCCESS;
}


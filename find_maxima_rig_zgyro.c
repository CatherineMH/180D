//
// Created by Hempen on 4/8/2017.
//

/* for file and terminal I/O */
#include <stdio.h>
/* for string manip */
#include <string.h>
/* for exit() */
#include <stdlib.h>
/* for fabsf() */
#include <math.h>
#include "Moving_Avg_Filter.h"

#define BUFF_SIZE 1024

/*finds the peaks of a z-gyro signal above zero from z-gyroscope data of a given input file*/
int main(void)
{
    char *ifile_name = "walk_data.csv";
    //printf("Input file : ");
    //gets(ifile_name);

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int N_SAMPLES;
    int read_value;

    /*open input file*/
    printf("Attempting to read from file \'%s\'.\n", ifile_name);
    fp = fopen(ifile_name, "r");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                ifile_name
        );
        exit(EXIT_FAILURE);
    }

    /* count the number of lines in the file */
    read = getline(&line, &len, fp); //discard header of file
    N_SAMPLES = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        N_SAMPLES++;
    }
    //printf("Number of samples: %d \n", N_SAMPLES);

    /*store the data into array for further analysis*/
    float *t, *t2, *ax, *ay, *az, *gx, *gy, *gz;

    t = (float *) malloc(sizeof(float) * N_SAMPLES);
    t2 = (float *) malloc(sizeof(float) * N_SAMPLES);
    ax = (float *) malloc(sizeof(float) * N_SAMPLES);
    ay = (float *) malloc(sizeof(float) * N_SAMPLES);
    az = (float *) malloc(sizeof(float) * N_SAMPLES);
    gx = (float *) malloc(sizeof(float) * N_SAMPLES);
    gy = (float *) malloc(sizeof(float) * N_SAMPLES);
    gz = (float *) malloc(sizeof(float) * N_SAMPLES);

    read_value = 0;
    int i = 0;

    /* go back to the start of the file so that the data can be read */
    rewind(fp);
    read = getline(&line, &len, fp); //discard header of file

    if(N_SAMPLES < 2)
        exit(EXIT_FAILURE); //if there is no meaningful data in file, exit
    int limit = N_SAMPLES - 2;

    while ((read = getline(&line, &len, fp)) != -1 && i < limit)
    {
        read_value = sscanf(line, "%f,%f,%f,%f,%f,%f,%f,%f\n", &t[i], &t2[i], &ax[i], &ay[i],
                    &az[i], &gx[i], &gy[i], &gz[i]);

        if (read_value != 8) {
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
    fclose(fp);

    /*find maximum values of specified data*/
    int input = -1;
    //printf("Which data? (0 = a_x, 1 = a_y, 2 = a_z, 3 = g_x, 4 = g_y, 5 = g_z) : ");
    //input = getchar();
    //input -= '0';

    float *orig_data = gz;
    float *data = gz;

    //smooth the data with moving average filter:
    moving_average(N_SAMPLES, 50, t, orig_data, data);
    float threshold = 0;

    float *maxima_time = (float *) malloc(sizeof(float) * N_SAMPLES);
    float *maxima = (float *) malloc(sizeof(float) * N_SAMPLES);
    int *maxima_idx = (int *) malloc(sizeof(int) * N_SAMPLES);

    int num_maxima = 0; /*variable to keep track of the number of maxima*/

    float curr_minimum = threshold;
    int count = 0;
    int curr_min_idx = 0;
    int curr_min_idx_wave = 0;
    int new_minima_found = 0; /*boolean*/
    int first_in_region_found = 0; /*boolean*/
    int num_extrema = 0;

    float first_in_region_time = 0;
    float first_in_region_val = 0;

    float neg_slope = 0; //linear approximation of slope preceding local extrema
    float pos_slope = 0; //linear approx of slope following local extrema
    int end_count = 50; //a parameter that determines how long we go for before looking for a new maximum
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
                maxima_time[curr_min_idx] = t[curr_min_idx_wave];
                maxima[curr_min_idx] = curr_minimum;
                maxima_idx[curr_min_idx] = curr_min_idx_wave; //store original index of maximum
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

    /*find max of all maxima*/
    float sum = 0;
    float max = 0;
    int points_incl = 0;
    for(idx = 0; idx < num_extrema; idx++)
    {
        if(maxima[idx] > max)
            max = maxima[idx];
    }

    /*find average of all maxima*/
    float average = 0;
    for(idx = 0; idx < num_extrema; idx++)
    {
        //only include a point in avg. if it is greater than half the biggest maximum:
        if(maxima[idx] > max/2) {
            sum += maxima[idx];
            points_incl += 1;
        }
    }
    average = sum/points_incl;

    //remove maximum points that are closer to zero than the average:
    float *new_maxima_time = (float *) malloc(sizeof(float) * num_extrema);
    int *new_maxima_idx = (int *) malloc(sizeof(int) * num_extrema);
    float *new_maxima = (float *) malloc(sizeof(float) * num_extrema);
    float *min_between = (float *) malloc(sizeof(float) * num_extrema); //the abs minimum pts beteen successive maxima
    int new_idx = 0;

    //clean data and store in new arrays:
    for(idx = 0; idx < num_extrema; idx++)
    {
        //only include datapoint if it is closer to the maximum than 1/4 the maximum:
        if (abs(maxima[idx] - average) < abs(maxima[idx] - max/4))
        {
            new_maxima_time[new_idx] = maxima_time[idx];
            new_maxima[new_idx] = maxima[idx];
            new_maxima_idx[new_idx] = maxima_idx[idx];
            new_idx = new_idx + 1;
        }
    }
    num_extrema = new_idx; //update number of maximum points

    /*
     * find the absolute minimum points between successive maxima:
     * */
    int idx_data = 0; //the index in the raw dataset
    limit = 0;
    float min = 0;
    int no_min = num_extrema-1;

    for(idx = 0; idx < num_extrema-1; idx++) //iterate through maxima
    {
        limit = new_maxima_idx[idx+1];
        for(idx_data = new_maxima_idx[idx]; idx_data < limit; idx_data++)
        {
            if(data[idx_data] < min)
                min = data[idx_data];
        }
        min_between[idx] = min; //store the absolute minimum between two successive maxima
    }

    //clean data of bad peaks and store to original maxima array:
    int curr_max = 0;
    int keep_last_max = 1;
    for(idx = 0; idx < num_extrema-1; idx++) //iterate through maxima
    {
        //if the minimum in between the two maxima is greater than half the height of either maxima,
        //something is wrong:
        if(min_between[idx] > .5*new_maxima[idx] || min_between[idx] > .5*new_maxima[idx+1])
        {
            //check whether or not we should keep the last maximum:
            if(idx == num_extrema-2 && new_maxima[idx]>new_maxima[idx+1])
            {
                keep_last_max = 0;
            }


            //keep the larger maximum and discard the other:
            maxima_time[curr_max] = (new_maxima[idx]>new_maxima[idx+1])?
                                    new_maxima_time[idx]:new_maxima_time[idx+1];
            maxima[curr_max] = (new_maxima[idx]>new_maxima[idx+1])?
                               new_maxima[idx]:new_maxima[idx+1];

            //increment curr_max:
            curr_max = (new_maxima[idx]>new_maxima[idx+1])? curr_max+2 : curr_max+1;
        }
        else
        {
            maxima_time[curr_max] = new_maxima_time[idx];
            maxima[curr_max] = new_maxima[idx];
            curr_max += 1;
        }
    }
    //account for last maximum point:
    if(keep_last_max)
    {
        maxima_time[curr_max] = new_maxima_time[idx];
        maxima[curr_max] = new_maxima[curr_max];
        curr_max += 1;
    }

    //update array sizes:
    num_extrema = curr_max;
    no_min = curr_max - 1;

    for(idx = 0; idx < num_extrema; idx++)
    {
        printf("%f, %f \n", maxima_time[idx], maxima[idx]);
    }


    char* output_file_name = "maxima.csv";
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
        fprintf(fp, "%20.10lf,%1f\n",
                maxima_time[i],
                maxima[i]
        );
    }
    fclose(fp);

    /*free memory*/
    free(t);
    free(t2);
    free(ax);
    free(ay);
    free(az);
    free(gx);
    free(gy);
    free(gz);

    free(maxima);
    free(maxima_time);
    free(maxima_idx);

    free(new_maxima);
    free(new_maxima_time);
    free(new_maxima_idx);

    return(0);
}
//gcc -o collect_data collect_data.c LSM9DS0.c -lmraa -lm
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <mraa/i2c.h>
#include <sys/time.h>
#include "LSM9DS0.h"

/*open directory */
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>

#define MILLION 1000000.0f
#define BUFF_SIZE 1024

sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
        run_flag = 0;
}

int main()
{
    mraa_i2c_context 	accel, gyro;
    accel_scale_t 		a_scale = A_SCALE_16G; //Acceleration scale (16G = Max)
    gyro_scale_t 		g_scale = G_SCALE_2000DPS;
    
    data_t  accel_data, gyro_data;
    float   a_res, g_res;
    int     fd;
    
    DIR     *dp;
    char    *file_name;
    FILE    *fp;
    struct timeval 		tv_before, tv_after, tv_start, tv_current;
    unsigned int integer_time, integer_time2; //integer part of time
    
    signal(SIGINT, &do_when_interrupted);
    //create directory
   
    //dp = opendir ("/Users/katyakoplenko/Desktop/Data_Directory"); //open directory
    // DELETE ALL FILES IN DIRECTORY possibly manually
    
    /*file_name = "my";
    fp = fopen(file_name, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file \'%s\'. Exiting.\n",
                file_name
                );
        exit(EXIT_FAILURE);
     
    }
    */
    file_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
    //initialize sensors, set scale, and calculate resolution.
    accel = accel_init();
    set_accel_scale(accel, a_scale);
    a_res = calc_accel_res(a_scale);
    
    gyro = gyro_init();
    set_gyro_scale(gyro, g_scale);
    g_res = calc_gyro_res(g_scale);

    time_t timestamp_sec; /* timestamp in seconds */
    time(&timestamp_sec);  /* get current time */

    int collection_duration = 5; //seconds of data collection. Human activity at ~15Hz = 4 seconds/activity
    collection_duration -=1; //to account for rounding that takes place with integer times
    //ex) if we want under 2 seconds of data collection, the collection duration should be set to 1
    time_t start_time_sec; /* loop start timestamp in seconds */
    time_t stop_time_sec; /* loop end timestamp in seconds */

    /*Initialize timestamp variables for loop:*/

    struct timeval tv;
    double timestart_usec; /* timestamp in microsecond */
    double timeafter_usec;
    double initial_time_usec; //timestamp to record when data collection starts

    //Read the sensor data and print it to file
    while (run_flag)
    {
        /* get time corresponding to start of data collection for current file */
        time(&start_time_sec);
        //open new file
        memset(file_name,0, BUFF_SIZE);
        sprintf(file_name,"data_%ld.csv", start_time_sec);
        fp = fopen(file_name, "w");
        if (fp == NULL)
        {
            fprintf(stderr, "Failed to open file \'%s\'. Exiting.\n",
                    file_name
                    );
            exit(EXIT_FAILURE);
        }
        //lock file
        fd = fileno(fp);
        flock(fd, LOCK_EX);
        
        fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s\n",
                    "timestamp_before",
                    "timestamp_after",
                    "accel_x", "accel_y", "accel_z",
                    "gyro_x", "gyro_y", "gryo_z"
                    );
        //compute initial time:
        gettimeofday(&tv, NULL);
        initial_time_usec = tv.tv_sec + tv.tv_usec/MILLION;

        stop_time_sec = start_time_sec + collection_duration; //stop time based on how long we want to collect data for

        while (time(NULL) <= stop_time_sec && run_flag)
        {
            gettimeofday(&tv, NULL);
            timestart_usec = tv.tv_sec + tv.tv_usec/MILLION;
            timestart_usec -= initial_time_usec;

            accel_data = read_accel(accel, a_res);
            gyro_data = read_gyro(gyro, g_res);

            gettimeofday(&tv, NULL);
            timeafter_usec = tv.tv_sec + tv.tv_usec/MILLION;
            timeafter_usec -= initial_time_usec;

            //output collected data to file:
            fprintf(fp, "%10.6lf,%10.6lf,%+f,%+f,%+f,%+f,%+f,%+f\n",
                    timestart_usec,
                    timeafter_usec,
                    accel_data.x,
                    accel_data.y,
                    accel_data.z,
                    gyro_data.x,
                    gyro_data.y,
                    gyro_data.z
            );
            usleep(1500);

            gettimeofday(&tv_current, NULL);
        }
        //close file:
        fclose(fp);
        usleep(300);
    }
    return 0;
}

#include <stdio.h>
#include <mraa/i2c.h>
#include "LSM9DS0.h"

#define MILLION 1000000.0;
#define BUFF_SIZE 1024

float calculate_magnitude(data_t data)
{
	return sqrt(data.x*data.x+data.y*data.y+data.z*data.z);
}

int main(){
	mraa_i2c_context accel, gyro, mag;
	float a_res, g_res, m_res;
	accel_scale_t a_scale;
	gyro_scale_t g_scale;
	mag_scale_t m_scale;
	data_t ad, gd, md;
	data_t Go;

	a_scale = A_SCALE_16G; //make sure to catch all features in stride!
	g_scale = G_SCALE_2000DPS;
	m_scale = M_SCALE_2GS;

	accel = accel_init();
	set_accel_ODR(accel, A_ODR_1600); //set to max bandwidth!
	set_accel_scale(accel, a_scale);
	a_res = calc_accel_res(a_scale);

	gyro = gyro_init();	
	set_gyro_ODR(gyro, G_ODR_760_BW_100); //set to maximum!
	set_gyro_scale(gyro, g_scale);
	g_res = calc_gyro_res(g_scale);

	mag = mag_init();
set_mag_ODR(mag, M_ODR_125);
	set_mag_scale(mag, m_scale);
	m_res = calc_mag_res(m_scale);

	//Go = calc_gyro_offset(gyro, g_res); //gyroscope offset

	//get output file name from user:
    char* output_file = (char *) malloc(sizeof(char) * BUFF_SIZE);
	printf("Please enter file name: ");
	scanf("%s", output_file);
	printf("\n");
    FILE *fp = fopen(output_file, "ab+");

    printf("Attempting to write to file \'%s\'.\n", output_file);
	fp = fopen(output_file, "w");
	if (fp == NULL) {
		fprintf(stderr,
				"Failed to write to file \'%s\'.\n",
				output_file
		       );
		exit(EXIT_FAILURE);
	}

    fprintf(fp, "timestamp_before,timestamp_after,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z\n");
    time_t timestamp_sec; /* timestamp in seconds */
    time(&timestamp_sec);  /* get current time; same as: timestamp_sec = time(NULL)  */

    time_t start_time_sec; /* loop start timestamp in seconds */
    time(&start_time_sec);  /* get current time; same as: timestamp_sec = time(NULL)  */
    int collection_duration = 25; //how many seconds of data collection
    time_t stop_time_sec = start_time_sec + collection_duration; //stop time based on how long we want to collect data for
    /*Initialize timestamp variables for loop:*/

    struct timeval tv;
    double timestart_usec; /* timestamp in microsecond */
    double timeafter_usec;
    double initial_time_usec; //timestamp to record when data collection starts

    //compute initial time:
    gettimeofday(&tv, NULL);
    initial_time_usec = tv.tv_sec + tv.tv_usec/MILLION;
    //while we haven't reached data collection end time:
	while(time(NULL)<stop_time_sec){
	    gettimeofday(&tv, NULL);
	    timestart_usec = tv.tv_sec + tv.tv_usec/MILLION;
		timestart_usec -= initial_time_usec;
	    //time(&timestamp_before); //record time before acceleration data is read
		ad = read_accel(accel, a_res);
		gd = read_gyro(gyro, g_res);
		//time(&timestamp_after); //record time after angular velocity data is read
        gettimeofday(&tv, NULL);
        timeafter_usec = tv.tv_sec + tv.tv_usec/MILLION;
		timeafter_usec -= initial_time_usec;
		//md = read_mag(mag, m_res);

		//discard gyro offset for now
		gd.x = gd.x; //- Go.x;
		gd.y = gd.y; //- Go.y;
		gd.z = gd.z; //- Go.z;

        //output data to file:
		fprintf(fp, "%10.6lf,%10.6lf,%+f,%+f,%+f,%+f,%+f,%+f\n",
                        timestart_usec,
                        timeafter_usec,
                        ad.x,
                        ad.y,
                        ad.z,
                        gd.x,
                        gd.y,
                        gd.z
                       );

		usleep(1500); //write file at .0015 second intervals??
	}

	fclose(fp);

	return 0;
}

#include <unistd.h>
//#include <mraa/aio.h>
#include <stdio.h>
#include "floatfann.h"
#include <string.h>


#define BUFF_SIZE 1024

int main()
{
    int i, speed,rv;
   // int temp0, temp1, temp2, location;
    //uint16_t value0, value1, value2;
    float max;
    fann_type *calc_out;
    fann_type input[3];
    struct fann *ann;
    char *ifile_name;
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    //int inp,outp;
    //int N_SAMPLES = 106;
    //mraa_aio_context lightsensor0, lightsensor1, lightsensor2;
    
    ann = fann_create_from_file("RESULT.net");
    
    ifile_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(ifile_name, 0, BUFF_SIZE);
    snprintf(ifile_name,
             BUFF_SIZE,
             "Katya_test_file35.txt"//
             );
    /* open the input file */
    printf("Attempting to read from file \'%s\'.\n", ifile_name);
    fp = fopen(ifile_name, "r");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                ifile_name
                );
        exit(EXIT_FAILURE);
    }
    //read = getline(&line, &len, fp); //discard header of file
    //rv = sscanf(line, "%d,%d,%d\n", &N_SAMPLES, &inp, &outp);

    //lightsensor0 = mraa_aio_init(0);
    //lightsensor1 = mraa_aio_init(1);
    //lightsensor2 = mraa_aio_init(2);
    double a1,a2,a3;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        max = -100;
        rv = sscanf(line, "%lf,%lf,%lf\n", &a1, &a2, &a3);
	//printf("Values: %lf, %lf, %lf", a1, a2, a3);
	input[0] = a1;
	input[1] = a2;
	input[2] = a3;
        calc_out = fann_run(ann, input);
        
        for (i = 0; i < 4; i++) {
            if (calc_out[i] > max) {
                max = calc_out[i];
                speed = i;
            }
    }
   /* while (1) {
        temp0 = 0;
        temp1 = 0;
        temp2 = 0;
        max = -100;

        for (i = 0; i < 50; i++) {
            temp0 += mraa_aio_read(lightsensor0);
            temp1 += mraa_aio_read(lightsensor1);
            temp2 += mraa_aio_read(lightsensor2);
            usleep(10000);
        }
	*/
      /*  value0 = temp0 / 50;
        value1 = temp1 / 50;
        value2 = temp2 / 50;

        input[0] = (float) value0 / 1000;
        input[1] = (float) value1 / 1000;
        input[2] = (float) value2 / 1000;
      
        calc_out = fann_run(ann, input);

        for (i = 0; i < 4; i++) {
            if (calc_out[i] > max) {
                max = calc_out[i];
                location = i;
            }
        }*/

	printf("sensor values: %f, %f, %f -> speed is %d\n", input[0], input[1], input[2], speed);
        sleep(1);
    }
    fann_destroy(ann);
    return 0;
}

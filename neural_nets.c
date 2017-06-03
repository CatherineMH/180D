//
// Created by Hempen on 5/19/2017.
//

/*
 * A collection of all neural networks needed
 * for testing the data real-time
 *
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

#define BUFF_SIZE 1024
#define NO_FEATURES 43


#define SLOW_WALK 1
#define MED_WALK 2
#define FAST_WALK 3
#define JUMP_SHORT 4
#define JUMP_MED 5
#define JUMP_HIGH 6
#define UP_S 7
#define UP_M 9
#define UP_F 11
#define DOWN_S 8
#define DOWN_M 10
#define DOWN_F 12
#define T_RIGHT 13
#define T_LEFT 14

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
int ann_1(float* input, const char user_name[])
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    char net_path[300];
    strcpy(net_path, "/home/root/database/");
    strcat(net_path, user_name);
    strcat(net_path, "/RESULT_ANN_2_1.net");
    ann = fann_create_from_file(net_path);

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
            return SLOW_WALK;
        case 1:
            printf("MEDIUM WALK\n");
            return MED_WALK;
        case 2:
            printf("FAST WALK\n");
            return FAST_WALK;
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
int ann_2(float* input, const char user_name[])
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    char net_path[300];
    strcpy(net_path, "/home/root/database/");
    strcat(net_path, user_name);
    strcat(net_path, "/RESULT_ANN_2_2.net");
    ann = fann_create_from_file(net_path);

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
            return UP_S;
        case 1:
            printf("MEDIUM UPSTAIRS\n");
            return UP_M;
        case 2:
            printf("FAST UPSTAIRS\n");
            return UP_F;
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
int ann_3(float* input, const char user_name[])
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    char net_path[300];
    strcpy(net_path, "/home/root/database/");
    strcat(net_path, user_name);
    strcat(net_path, "/RESULT_ANN_2_3.net");
    ann = fann_create_from_file(net_path);

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
            return DOWN_S;
        case 1:
            printf("MEDIUM DOWNSTAIRS\n");
            return DOWN_M;
        case 2:
            printf("FAST DOWNSTAIRS\n");
            return DOWN_F;
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
int ann_4(float* input, const char user_name[])
{
    int i, speed,rv, motion;
    float max;
    fann_type *calc_out;
    struct fann *ann;

    size_t len = 0;
    char net_path[300];
    strcpy(net_path, "/home/root/database/");
    strcat(net_path, user_name);
    strcat(net_path, "/RESULT_ANN_2_4.net");
    ann = fann_create_from_file(net_path);

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
            return JUMP_SHORT;
        case 1:
            printf("MEDIUM JUMP\n");
            return JUMP_MED;
        case 2:
            printf("HIGH JUMP\n");
            return JUMP_HIGH;
        default:
            printf("Error!");
    }

    fann_destroy(ann);
    return -1;
}

int global_neural_network(const char* ifile_name, const char user_name[]) //changed
{
    int i, activity_type,rv,fd;
    int return_val = -1;
    float max;
    fann_type *calc_out;
    fann_type input[43]; //43
    struct fann *ann;

    FILE *f_out;
    f_out = fopen("ANN_output.csv", "a");
    if (f_out == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                "ANN_output.csv"
        );
        exit(EXIT_FAILURE);
    }

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

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
    ifile_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
    char net_path[300];
    strcpy(net_path, "/home/root/database/");
    strcat(net_path, user_name);
    strcat(net_path, "/RESULT_ANN_1.net");
    ann = fann_create_from_file(net_path);

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
        //printf("PREDICTED-%d (%f, %f, %f, %f, %f, %f)\n", activity_type,
        //        calc_out[0], calc_out[1], calc_out[2], calc_out[3], calc_out[4], calc_out[5]);

        switch(activity_type)
        {
            case 0:
                return_val = ann_1(input, user_name);
                break;
            case 1:
                return_val = ann_2(input, user_name);
                break;
            case 2:
                return_val = ann_3(input, user_name);
                break;
            case 3:
                return_val = ann_4(input, user_name);
                break;
            case 4:
                return_val = T_LEFT; printf("LEFT TURN\n");
                break;
            case 5:
                return_val = T_RIGHT; printf("RIGHT TURN\n");
                break;
            default:
                printf("Error!");
        }
        sleep(1); // might need to reduce

        fprintf(f_out, "%d\n", return_val);
    }
    fann_destroy(ann);
    fclose(f_out);
    return activity_type;
}

/*
 *
 * Main file for real-time
 * feature extraction and motion detection
 * gcc -o extract_real_time extract_real_time.c process_file.c ex_find_maxima_rig_zgyro.c Moving_Avg_Filter.c extract_test2.c  -lmraa -lm -lfann
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
#include "neural_nets.h"

#define BUFF_SIZE 1024
#define NO_FEATURES 43

sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
    {
        run_flag = 0;
    }
}

int main(int argc, char * argv[])
{
    char user_name[30];
    strcpy(user_name, argv[1]);
    
    FILE *fp;
    //int fd;

    signal(SIGINT, &do_when_interrupted);

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
    //int activity_type = -1;
    
    while(run_flag)
    {
        /*
         * check for files created by producer:
         * search directory for files starting with 'data_14', which indicates
         * a newly generated data file
         */
        system("ls -1 data_14*.csv > files_to_process.txt");


        //open file with file names we need to process:
        //printf("Opening file: files_to_process.txt to read each file\n");
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
            process_file(ifile_name, 1, 0, user_name); // return the features
            //system("clear");
            //delete processed file
            //printf("Deleting file %s\n", ifile_name);
            sprintf(delete_command, "rm %s", ifile_name);
            system(delete_command);

            //activity type: 0-Walk, 1-Stair Ascend, 2-Stairs Descend, 3-Jump, 4-Turn Left, 5-Turn Right
        }
        fclose(fp);

        system("rm files_to_process.txt");
        sleep(2);

    }
   
    return EXIT_SUCCESS;
}


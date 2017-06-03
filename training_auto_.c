/*
 *
 * Takes as input multiple raw data files
 * extracts features from input files
 * and generates a complete ANN training file from these files
 *
 *
 * */

//activity type: 0-Walk, 1-Stair Ascend, 2-Stairs Descend, 3-Jump, 4-Turn Left, 5-Turn Right

//ask if need train files
//how many for walking , create an array of files
//get their names from the user
//continue through alll activities
//process all files
//create a single training file
//create a .txt description file with activity, file name, number of strides

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
#include <unistd.h>
#include <signal.h>

#include "process_file.h"
#include "neural_nets.h"

#define BUFF_SIZE 1024
#define NO_INPUTS 43 //number of input neurons ( = number of features extracted)
#define ACTIVITY_COUNT 6
#define SPEED_COUNT 3

sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
        run_flag = 0;
}

int extract_features(const char *ifile_name, int activity_id, const int ACTIVITY_COUNT_USER)
{
    int i, j;
    int N_SAMPLES = 0; //number of samples in input file
    FILE *fp, *fr;

    /*
     * extract features from file
     * and save features in a .txt file
     * using process_file method:
     *
     * */
    process_file(ifile_name, 0);

    //creating a test file with NO_INPUTS number of features:
    char* test_file;
    test_file = (char *) malloc(sizeof(char) * BUFF_SIZE);
    memset(test_file, 0, BUFF_SIZE);

    char result[100];   // array to hold the result.
    char *out_file = "training_file_ANN.txt";
    strcpy(result, out_file); // copy string one into the result.

    snprintf(test_file, BUFF_SIZE, result);
    fp = fopen(test_file, "a"); //"a"
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                test_file
                );
        exit(EXIT_FAILURE);
    }

    //open file written to by process_file, which contains extracted features:
    char* line = NULL;
    size_t len;
    ssize_t read;

    char ff_name[100];   // array to hold the result.
    char *feature_file = "ann1_file.txt";
    strcpy(ff_name,ifile_name); // copy string one into the result.
    strcat(ff_name,feature_file); // append string two to the result.

    fr = fopen(ff_name, "r");
    if (fr == NULL)
    {
        fprintf(stderr,
                "Failed to read file \'%s\'.\n",
                ff_name
        );
        exit(EXIT_FAILURE);
    }
    else
    {
        //printf("Opened %s\n", ff_name);
    }

    /* count the number of lines in the file */
    N_SAMPLES = 0;
    while ((read = getline(&line, &len, fr)) != -1) {
        N_SAMPLES++;
    }

    /* go back to the start of the file so that the data can be read */
    rewind(fr);
    //printf("N_SAMPLES: %d\n", N_SAMPLES);

    for (i = 0; i < N_SAMPLES; i++)
    {
        //print features extracted by process_file to training file:
        if(getline(&line, &len, fr) != -1)
        {
            fprintf(fp, line);
        }

        //print -1 and 1s indicating activity type to training file:
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
	fprintf(desc_file, "%d %s %d\n", activity_id, ifile_name, N_SAMPLES);
	fclose(desc_file);

    fclose(fr);

    char delete_command[1024];
    //clear buffer
    memset(delete_command, 0 , 1024);

//    free(t);
//    free(ta);
//    free(x);
//    free(y);
//    free(z);
//    free(gx);
//    free(gy);
//    free(gz);

    return N_SAMPLES;
}

//function for making training files fo any ANN
//returns the total number of strides found in all input files
int make_training_file(const int ACTIVITY_NUM)
{
    char ** inputFiles[ACTIVITY_NUM];
    int inputCounts[ACTIVITY_NUM];
    int total_no_strides = 0; //total number of strides found in files; return value
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
            total_no_strides += extract_features(inputFiles[i][j], i, ACTIVITY_NUM); // return the features
        }
    }
    //printf("HELLO2\n");
    for ( i = 0; i < ACTIVITY_NUM; i++) {
        for ( j = 0; j < inputCounts[i]; j++) {
            free(inputFiles[i][j]);
        }
        free(inputFiles[i]);
    }
    return total_no_strides; //returnt the total number of strides found in all input files
}
int main()
{
    //function for making training files fo any ANN
    int total_n_samples = make_training_file(SPEED_COUNT);
    FILE* fp, *fr;

    size_t len;
    char* line = NULL;

    char result[100];   // array to hold the result.
    char *out_file = "training_file_ANN.txt";
    strcpy(result, out_file); // copy string one into the result.

    system("cp training_file_ANN.txt temp.txt");

    fp = fopen(result, "w");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                result
        );
        exit(EXIT_FAILURE);
    }

    /*
     * print no. strides (N_SAMPLES), no. input neurons, and no. output neurons (ACTIVITY_COUNT_USER)
     * to header of training file:
     *
     * */
    fprintf(fp, "%d %d %d\n", total_n_samples, NO_INPUTS, SPEED_COUNT);
    fclose(fp);

    fr = fopen("temp.txt", "r");
    if (fr == NULL)
    {
        fprintf(stderr,
                "Failed to read file \'%s\'.\n",
                "temp.txt"
        );
        exit(EXIT_FAILURE);
    }
//
    fp = fopen(result, "a");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to append to file \'%s\'.\n",
                result
        );
        exit(EXIT_FAILURE);
    }
//
    //restore data from temp file to training file:
    while ((getline(&line, &len, fr)) != -1) {
        fprintf(fp, line);
    }

    fclose(fr);
    fclose(fp);

    return EXIT_SUCCESS;
}

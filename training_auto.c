//TODO: Make use of the 0's and 50's from turn files=> in frocess_file funtion
//TODO: Make sure the left turn data is included in trainign file for ANN1
//TODO: Before starting the code remove the existing training files for the existing user from "database" folder if the user wanted to use his previous data
//
//gcc -o training_auto_new-2 training_auto_new-2.c ex_find_maxima_rig_zgyro.c Moving_Avg_Filter.c process_file.c neural_nets.c -lm -lmraa -lfann
//
/*
 *
 * Automatically generates a set of training files for real time system
 * from datasets collected by users present in database
 *
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
#include <stdbool.h>               //WARNING ALLOCATE MEMORY, WARNING REMOVING OUTPUT FILES (ann1_file.txt, description_file.txt) BEFORE LAUNCHING BECAUSE OF APPENDING
//for real time system
#include <sys/types.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/time.h>
//#include "floatfann.h"
#include <unistd.h>
#include <signal.h>
#include "ex_find_maxima_rig_zgyro.h"
#include "process_file.h"
#include "training_auto.h"

#define BUFF_SIZE 1024
#define ACTIVITY_COUNT 6
#define SPEED_COUNT 3
#define NO_INPUTS 43 //number of input neurons (number of stride features extracted)

/*sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
        run_flag = 0;
}
*/

/*
 * extract features:
 * INPUTS:
 * ifile_name = raw data file
 * activity_id =  0 - Walking, 1 - Upstairs, 2 - Downstairs, 3 - Jump, 4 - Left Turn, 5 - Right Turn
 * ACTIVITY_COUNT_USER = total number of different activities
 * NAME = user's name
 * ANN = the name of the ANN this file will train
 * is_turn: 1 = turn dataset, 0 = not turn dataset
 *
 * */
int extract_features(const char *ifile_name, int activity_id, const int ACTIVITY_COUNT_USER, const char NAME[], char ANN[], int is_turn /*NB1*/)
{
	/* Generic variables */
	int i, j, idx, fd, rv;
    int no_input_features = 43; //number of input features being fed to neural network
 
    FILE *fp, *fr;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	
    int N_SAMPLES;

    process_file(ifile_name, 0, is_turn, 0);

    //open file written to by process_file, which contains extracted features:
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

    //creating a training file with NO_FEATURES fetures
    char train_file[100];   // array to hold the result.
    char out_file[5] = ".txt";
    strcpy(train_file, "/home/root/database/");
    strcat(train_file, NAME);
    strcat(train_file, "/");//NB! name of the user is the subdirectory
    strcat(train_file, ANN);
    strcat(train_file, out_file); // copy string one into the result.

//    printf("train_file: %s \n", train_file);

    fp = fopen(train_file, "a"); //"a"
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                train_file
                );
        exit(EXIT_FAILURE);
    }

    /* count the number of lines in the file */
    N_SAMPLES = 0;
    while ((read = getline(&line, &len, fr)) != -1) {
        N_SAMPLES++;
    }

    /* go back to the start of the file so that the data can be read */
    rewind(fr);

    for (i = 0; i < N_SAMPLES-1; i++)
    {
        /*
     * Feed input features to neural network:
     * */

        //print features extracted by process_file to training file:
        if(getline(&line, &len, fr) != -1)
        {
            fprintf(fp, line);
        }

        int j;
		for (j = 0; j < activity_id; j++) {
			fprintf(fp, "-1 ");
		}
		fprintf(fp, "1");
		if (activity_id != ACTIVITY_COUNT_USER - 1) {
			fprintf(fp, " ");
		}
		for (j = activity_id + 1; j < ACTIVITY_COUNT_USER; j++) {
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

    //remove junk files created by process_file:
    char remove_files[300];
    strcpy(remove_files, "rm /home/root/database/");
    strcat(remove_files, NAME);
    strcat(remove_files, "/*ann1_file.txt");
    system(remove_files);

    char delete_command[1024];
    //clear buffer
    memset(delete_command, 0 , 1024);

    return N_SAMPLES-1;
}

/*
 * add_header: adds a header in the format TOTAL_STRIDES NO_INPUT_NEURONS NO_OUTPUT NEURONS to training file
 * */
void add_header(const char NAME[], char ANN[], int total_n_samples, int no_inputs, int no_outputs)
{
    /*
     * fp = file to write to, fr = file to read from
     * */
    FILE* fp, *fr;

    size_t len;
    char* line = NULL;

    //construct the training file name:
    char train_file[100];   // array to hold the result.
    char out_file[5] = ".txt";
    strcpy(train_file, "/home/root/database/");
    strcat(train_file, NAME);
    strcat(train_file, "/");//NB!
    strcat(train_file, ANN);
    strcat(train_file, out_file); // copy string one into the result.

    //construct and execute system command to copy the training file to a temporary file:
    char command[200]; //holds the system call
    strcpy(command, "cp ");
    strcat(command, train_file);
    strcat(command, " /home/root/database/temp.txt");
    printf("command: %s \n", command);
    system(command);

    //attempt to write to training file:
    fp = fopen(train_file, "w");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                train_file
        );
        exit(EXIT_FAILURE);
    }

    /*
     * WRITE HEADER TO TRAINING FILE:
     *
     * print no. strides (N_SAMPLES), no. input neurons, and no. output neurons (ACTIVITY_COUNT_USER)
     * to header of training file:
     *
     * */
    fprintf(fp, "%d %d %d\n", total_n_samples, no_inputs, no_outputs);
    fclose(fp);

    fr = fopen("/home/root/database/temp.txt", "r");
    if (fr == NULL)
    {
        fprintf(stderr,
                "Failed to read file \'%s\'.\n",
                "/home/root/database/temp.txt"
        );
        exit(EXIT_FAILURE);
    }
//
    fp = fopen(train_file, "a");
    if (fp == NULL)
    {
        fprintf(stderr,
                "Failed to append to file \'%s\'.\n",
                train_file
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

    return;
}



void create_training_files(const char user_name[])
{
    //create a function for making training files for any ANN
  //  make_training_file(SPEED_COUNT);
   //Variables
    FILE *fp, *fn;

    //create variables to keep track of total # of strides in each training file:
    int ANN_1_COUNT = 0;
    int ANN_2_1_COUNT = 0;
    int ANN_2_2_COUNT = 0;
    int ANN_2_3_COUNT = 0;
    int ANN_2_4_COUNT = 0;

    char *ifile_name;
    ifile_name = malloc(sizeof(char) * 1024);
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    //char NAME[30]; //NB!
    int activity_number = 0;
    //int rv; //NB! undeclared rv


    /* open the input file */
    //printf("Opening file %s\n", ifile_name);
    /*
	fn = fopen("Usernames.txt", "r");
    if (fn == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                "Usernames.txt"
        );
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fn)) != -1) {//read the name
        /* get the username */
    /*    rv = sscanf(line, "%s\n", NAME); //process files for the username on the current line
        if (rv != 1) {
            fprintf(stderr,
                    "%s \'%s\'. %s.\n",
                    "Failed to read line",
                    line,
                    "Exiting"
            );
            exit(EXIT_FAILURE);
        }
    }
	*/
    memset(ifile_name, 0, BUFF_SIZE); //NB! ifile changed to ifile_name

    char csv_file_name[100];
    strcpy(csv_file_name, "/home/root/database/%s/%s.csv");
    
    //Activity Training
    for (activity_number = 0; activity_number < ACTIVITY_COUNT /* NB! */; activity_number++) {
	switch (activity_number){
            // Activity Cases: 0 - Walking, 1 - Upstairs, 2 - Downstairs, 3 - Jump, 4 - Left Turn, 5 - Right Turn
            case 0 :
                //walking speeds
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "ws");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "wm");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "wf");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                break;
            case 1:
                //upstairs speeds
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sus");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sum");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "suf");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                break;
            case 2:
                //downstairs speeds
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sds");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sdm");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sdf");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                break;
            case 3:
                //jump heights
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "js");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "jm");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "jh");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 0);// make training files
                break;
            case 4:
                //left turn
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "tl");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 1);// make training files
                break;
            case 5:
                //right turn
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "tr");
                ANN_1_COUNT += extract_features(ifile_name, activity_number, ACTIVITY_COUNT, user_name, "ANN_1", 1);// make training files
                break;

        }

    }

    //Activity Speeds
    for (activity_number = 0; activity_number < 4; activity_number++) {
	switch (activity_number){
            /*
             * * SPEED DETECTION NEURAL NETWORKS FOR EACH OF THE DIFFERENT MOTION TYPES:
             * * ANN_2_1 - walking
             * * ANN_2_2 - upstairs
             * * ANN_2_3 - downstairs
             * * ANN_2_4 - jumping
             * */
            //TODO: Check for naming of these ANN training files for the final setup
            case 0 :
                //walking speeds
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "ws");
                ANN_2_1_COUNT += extract_features(ifile_name, 0, SPEED_COUNT, user_name, "ANN_2_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "wm");
                ANN_2_1_COUNT += extract_features(ifile_name, 1, SPEED_COUNT, user_name, "ANN_2_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "wf");
                ANN_2_1_COUNT += extract_features(ifile_name, 2, SPEED_COUNT, user_name, "ANN_2_1", 0);// make training files
                break;
            case 1:
                //upstairs speeds
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sus");
                ANN_2_2_COUNT += extract_features(ifile_name, 0, SPEED_COUNT, user_name, "ANN_2_2", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sum");
                ANN_2_2_COUNT += extract_features(ifile_name, 1, SPEED_COUNT, user_name, "ANN_2_2", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "suf");
                ANN_2_2_COUNT += extract_features(ifile_name, 2, SPEED_COUNT, user_name, "ANN_2_2", 0);// make training files
                break;
            case 2:
                //downstairs speeds
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sds");
                ANN_2_3_COUNT += extract_features(ifile_name, 0, SPEED_COUNT, user_name, "ANN_2_3", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sdm");
                ANN_2_3_COUNT += extract_features(ifile_name, 1, SPEED_COUNT, user_name, "ANN_2_3", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "sdf");
                ANN_2_3_COUNT += extract_features(ifile_name, 2, SPEED_COUNT, user_name, "ANN_2_3", 0);// make training files
                break;
            case 3:
                //jump heights
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "js");
                ANN_2_4_COUNT += extract_features(ifile_name, 0, SPEED_COUNT, user_name, "ANN_2_4", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "jm");
                ANN_2_4_COUNT += extract_features(ifile_name, 1, SPEED_COUNT, user_name, "ANN_2_4", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, csv_file_name, user_name, "jh");
                ANN_2_4_COUNT += extract_features(ifile_name, 2, SPEED_COUNT, user_name, "ANN_2_4", 0);// make training files
                break;

        }

    }


    //ask if need train files
    //how many for walking , create an array of files
    //get their names from the user
    //continue through alll activities
    //process all files
    //create a single training file
    //create a .txt description file with activity, file name, number of strides
    printf("---------------NO. STRIDES---------------\n");
    printf("ANN_1: %d\n", ANN_1_COUNT);
    printf("ANN_2_1: %d \n", ANN_2_1_COUNT);
    printf("ANN_2_2: %d \n", ANN_2_2_COUNT);
    printf("ANN_2_3: %d \n", ANN_2_3_COUNT);
    printf("ANN_2_4: %d \n", ANN_2_4_COUNT);

    /*
 * ADD HEADERS TO ALL TRAINING FILES:
 *
 * */

	//NB! name is subdirectory
    add_header( user_name,"ANN_1", ANN_1_COUNT, NO_INPUTS, ACTIVITY_COUNT);
    add_header( user_name,"ANN_2_1", ANN_2_1_COUNT, NO_INPUTS, SPEED_COUNT);
    add_header( user_name,"ANN_2_2", ANN_2_2_COUNT, NO_INPUTS, SPEED_COUNT);
    add_header( user_name,"ANN_2_3", ANN_2_3_COUNT, NO_INPUTS, SPEED_COUNT);
    add_header( user_name,"ANN_2_4", ANN_2_4_COUNT, NO_INPUTS, SPEED_COUNT);
}

int main_2() {
    char NAME[30]; //NB!
    int activity_number = 0;
    
    FILE *fn;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int rv;
    
    /* open the input file */
    //printf("Opening file %s\n", ifile_name);
    
	fn = fopen("Usernames.txt", "r");
    if (fn == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                "Usernames.txt"
        );
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, fn)) != -1) {//read the name
        /* get the username */
        rv = sscanf(line, "%s\n", NAME); //process files for the username on the current line
        if (rv != 1) {
            fprintf(stderr,
                    "%s \'%s\'. %s.\n",
                    "Failed to read line",
                    line,
                    "Exiting"
            );
            exit(EXIT_FAILURE);
        }
    }
	create_training_files(NAME);
	return EXIT_SUCCESS;
}

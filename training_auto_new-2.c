//TODO: Make use of the 0's and 50's from turn files=> in frocess_file funtion
//TODO: Make sure the left turn data is included in trainign file for ANN1
//TODO: Before starting the code remove the existing training files for the existing user from "database" folder if the user wanted to use his previous data
//
//gcc -o training_auto_new-2 training_auto_new-2.c ex_find_maxima_rig_zgyro.c Moving_Avg_Filter.c process_file.c neural_nets.c -lm -lmraa -lfann
//
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
#include "ex_find_maxima_rig_zgyro.h"
#include "process_file.h"

#define BUFF_SIZE 1024
#define ACTIVITY_COUNT 6
#define SPEED_COUNT 3

sig_atomic_t volatile run_flag = 1;

void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
        run_flag = 0;
}

void extract_features(const char *ifile_name, int activity_id, const int ACTIVITY_COUNT_USER, char ANN[], int is_turn /*NB1*/)
{
	/* Generic variables */
	int i, j, idx, fd, rv;
    int no_input_features = 43; //number of input features being fed to neural network
 
    FILE *fp, *fr;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	
    int N_SAMPLES;

    process_file(ifile_name, 0, is_turn);
    //open file written to by process_file, which contains extracted features:
    printf("ifile_name: %s\n", ifile_name);


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

    //creating a training file with 43 fetures
    //char* train_file = calloc(1, sizeof(*train_file));
    //memset(train_file, 0, BUFF_SIZE);
    //Store training files in the given file
    //snprintf(train_file, sizeof(*train_file), "/home/root/database/%s.txt", ANN);

    char train_file[100];   // array to hold the result.
    char out_file[5] = ".txt";
    strcpy(train_file, "/home/root/database/");
    strcat(train_file, ANN);
    strcat(train_file, out_file); // copy string one into the result.

    printf("train_file: %s \n", train_file);

    printf("adding\n");
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
}

int main()
{
    //create a function for making training files for any ANN
  //  make_training_file(SPEED_COUNT);
   //Variables
    FILE *fp, *fn;
    char *ifile_name;
    ifile_name = malloc(sizeof(char) * 1024);
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char NAME[30]; //NB!
    int activity_number = 0;
    int rv; //NB! undeclared rv


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
        rv = sscanf(line, "%s\n", NAME);
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
    printf("step0\n");
    memset(ifile_name, 0, BUFF_SIZE); //NB! ifile changed to ifile_name

    //Activity Training
    for (activity_number = 0; activity_number < ACTIVITY_COUNT /* NB! */; activity_number++) {
        printf("activity %d\n", activity_number);
	switch (activity_number){
            // Activity Cases: 0 - Walking, 1 - Upstairs, 2 - Downstairs, 3 - Jump, 4 - Left Turn, 5 - Right Turn
            case 0 :
                //walking speeds
		printf("step1\n");
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "ws");
		printf("step2\n");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
		printf("step3\n");
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "wm");
		printf("step4\n");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
		printf("step5\n");
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "wf");
		printf("step6\n");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                break;
            case 1:
                //upstairs speeds
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sus");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sum");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "suf");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                break;
            case 2:
                //downstairs speeds
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sds");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sdm");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sdf");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                break;
            case 3:
                //jump heights
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "js");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "jm");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "jh");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 0);// make training files
                break;
            case 4:
                //left turn
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "tl");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 1);// make training files
                break;
            case 5:
                //right turn
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "tr");
                extract_features(ifile_name, activity_number, ACTIVITY_COUNT, "ANN_1", 1);// make training files
                break;

        }

    }

    //Activity Speeds
    for (activity_number = 0; activity_number < 4; activity_number++) {
        printf("activity2 %d\n", activity_number);
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
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "ws");
                extract_features(ifile_name, 0, SPEED_COUNT, "ANN_2_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "wm");
                extract_features(ifile_name, 1, SPEED_COUNT, "ANN_2_1", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "wf");
                extract_features(ifile_name, 2, SPEED_COUNT, "ANN_2_1", 0);// make training files
                break;
            case 1:
                //upstairs speeds
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sus");
                extract_features(ifile_name, 0, SPEED_COUNT, "ANN_2_2", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sum");
                extract_features(ifile_name, 1, SPEED_COUNT, "ANN_2_2", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "suf");
                extract_features(ifile_name, 2, SPEED_COUNT, "ANN_2_2", 0);// make training files
                break;
            case 2:
                //downstairs speeds
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sds");
                extract_features(ifile_name, 0, SPEED_COUNT, "ANN_2_3", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sdm");
                extract_features(ifile_name, 1, SPEED_COUNT, "ANN_2_3", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "sdf");
                extract_features(ifile_name, 2, SPEED_COUNT, "ANN_2_3", 0);// make training files
                break;
            case 3:
                //jump heights
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "js");
                extract_features(ifile_name, 0, SPEED_COUNT, "ANN_2_4", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "jm");
                extract_features(ifile_name, 1, SPEED_COUNT, "ANN_2_4", 0);// make training files
                snprintf(ifile_name, BUFF_SIZE, "/home/root/database/%s_%s.csv", NAME, "jh");
                extract_features(ifile_name, 2, SPEED_COUNT, "ANN_2_4", 0);// make training files
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
    return EXIT_SUCCESS;
}

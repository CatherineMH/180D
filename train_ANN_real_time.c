/*
 * train_ANN_real_time
 *
 * TODO: Note that all codes should be in "systemCode" folder in order for this code to work
 *
 * gcc -Wall -o train_ANN_real_time train_ANN_real_time.c ANN_1_train.c ANN_2_1_train.c ANN_2_2_train.c ANN_2_3_train.c ANN_2_4_train.c -lmraa -lfann
 *new 
 *gcc -o train_ANN_real_time train_ANN_real_time.c LSM9DS0.c Moving_Avg_Filter.c ex_find_maxima_rig_zgyro.c training_auto.c user_data_collection.c ANN_train.c process_file.c extract_test2.c -lmraa -lfann -lm
 * */

//include code for training ANNs:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/file.h>


#include <sys/stat.h>
#include <unistd.h>
#include "ANN_train.h"
#include "user_data_collection.h"
#include "training_auto.h"
#define BUFF_SIZE 1024
//param user_name = the name of the user
//result 1, if the user is new to the system
// otherwise 0.
int user_is_new(const char user_name[30]) {
    /* open the input file */
    //printf("Opening file %s\n", ifile_name);
    FILE *fn;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int rv;
    struct stat st = {0};
    char directory[100];
    strcpy(directory, "/home/root/systemCode/");

    char  *userfile = (char *)malloc(sizeof(char)* BUFF_SIZE);

    if( access( "Usernames.txt", F_OK ) != -1 ) {// file exists
        fn = fopen("Usernames.txt", "r");
    }
    else {// file doesn't exist, so creates it
        if (stat(directory, &st) == -1) {
            mkdir(directory, 0700);
        }
        memset(userfile, 0, BUFF_SIZE);
        //Store file in "database" directory Check: with system call if the file exists
        snprintf(userfile, BUFF_SIZE, "%s/Usernames.txt", directory);
        fn = fopen(userfile, "r");
    }

    /*if (fn == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                "Usernames.txt"
        );
        exit(EXIT_FAILURE);
    }*/

	//NB! verify whether the person is new
    //read all usernames to be processed from a text file: (TBD!)
	char next_name[30];
    while ((read = getline(&line, &len, fn)) != -1) {//read the name
        /* get the username */
        rv = sscanf(line, "%s\n", next_name); //process files for the username on the current line
		if (strcmp(next_name, user_name) == 0) {
			fclose(fn);
			return 0;
		}
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
	fclose(fn);
	return 1;
}

void create_ANN(const char user_name[]) {
	//call the ANN training routines to train each ANN on the user's .txt files:
	ANN_train(user_name, 6, "ANN_1");
	ANN_train(user_name, 3, "ANN_2_1");
	ANN_train(user_name, 3, "ANN_2_2");
	ANN_train(user_name, 3, "ANN_2_3");
	ANN_train(user_name, 3, "ANN_2_4");
}

void do_testing(const char user_name[]) {
    char start_motion[300];
    system("chmod 755 motion");
    strcpy(start_motion, "./motion ");
    strcat(start_motion, user_name);
    system(start_motion);
}

void do_all(const char user_name[]) {
 //   	printf("COLLECT_USER_DATA\n");
//	collect_user_data(user_name);
    printf("CREATE_TRAINING_FILES\n");
	create_training_files(user_name);
    printf("CREATE_ANN\n");
	create_ANN(user_name);
    printf("DO_TESTING\n");
	do_testing(user_name);
}

//return 1, if person want to be as NEW PERSON
//0, otherwise
int want_retrain() {
	printf("print 'y' (yes) if you want to be as NEW PERSON, otherwise enter another character: ");
    fflush(stdout);
	char ch;
	scanf(" %c", &ch);
	return ch == 'y';
}

int main()
{
    //NB
    //Ask for User name
    //
	char user_name[30];
	printf("Enter your name: ");
	scanf("%s", user_name);

	//if the person is new
	if (user_is_new(user_name)) {
		//then do all things
		do_all(user_name);
	} else {
		//if the person is not new, then ask, whether he wants to be as NEW PERSON
		if (want_retrain()) {
			//if he wants to be as NEW PERSON, then do all things
			do_all(user_name);
		} else {
			//otherwise, do only testing
			do_testing(user_name);
		}
	}
	return 0;
}

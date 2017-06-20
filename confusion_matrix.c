/*
 * A collection of all neural networks needed
 * for testing the data real-time
 *
 * */

//gcc -o confusion_matrix confusion_matrix.c

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

#define BUFF_SIZE 1024
#define NO_FEATURES 43

#define N_MOTIONS 14
#define SIZE_INT sizeof(int)

/*
 * generates a confusion matrix from ann output file
 *
 * */
int main()
{
    int N_SAMPLES = 0;
    int rv = 0;
    int i = 0; int j = 0;
    FILE* fp;
    ssize_t read;
    char *line = NULL;
    size_t len = 0;

    fp = fopen("ANN_output.csv", "r");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                "ANN_output.csv"
        );
        exit(EXIT_FAILURE);
    }

    N_SAMPLES = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        N_SAMPLES++;
    }
    int* predicted_motion = (int *) malloc(SIZE_INT * N_SAMPLES);
    int* actual_motion = (int *) malloc(SIZE_INT * N_SAMPLES);
    rewind(fp);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        rv = sscanf(line, "%d,%d\n", &predicted_motion[i], &actual_motion[i]);
        i++;
    }

    //record accuracy for each motion:
    char motion_type[N_MOTIONS][30] = {"Slow Walk", "Med Walk", "Fast Walk",
    "Short Jump", "Med Jump", "High Jump", "Up Slow", "Down Slow", "Up Med", "Down Med", "Up Fast", "Down Fast",
    "Right Turn", "Left Turn"};
    float num_samples[N_MOTIONS]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float num_correct[N_MOTIONS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float accuracy[N_MOTIONS]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    int confusion_mat[N_MOTIONS][N_MOTIONS] = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    for(i = 0; i < N_SAMPLES; i++){
        num_samples[predicted_motion[i]-1]+=1;
        if(predicted_motion[i]==actual_motion[i]) num_correct[predicted_motion[i]-1]+=1;
        confusion_mat[predicted_motion[i]-1][actual_motion[i]-1]+=1;
    }
    for(i = 0; i < N_MOTIONS; i++){
        if(num_samples[i] != 0)
            accuracy[i] = num_correct[i]/num_samples[i];
        printf("accuracy: %f\n", accuracy[i]);
    }

    fp = fopen("confusion_matrix.csv", "w");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                "confusion_matrix.csv"
        );
        exit(EXIT_FAILURE);
    }

    //output confusion matrix to file:
    for(i = 0; i < N_MOTIONS+1; i++)
    {
        for(j = 0; j < N_MOTIONS+1; j++)
        {
            if(i==0 && j==0)
                fprintf(fp, ",");
            else if(i==0)
                fprintf(fp, "P_%s,",motion_type[j-1]);
            else if(j==0)
                fprintf(fp, "A_%s,",motion_type[i-1]);
            else
                fprintf(fp, "%d,", confusion_mat[j-1][i-1]);

        }
        fprintf(fp, "\n"); //next row
    }
    fclose(fp);

    fp = fopen("performance_metrics.csv", "w");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to read from file \'%s\'.\n",
                "performance_metricx.csv"
        );
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "NO. SAMPLES, %d,\n", N_SAMPLES);
    for(i = 0; i < N_MOTIONS; i++)
    {
        fprintf(fp, "%s,", motion_type[i]);
        fprintf(fp, "%f,\n", accuracy[i]);
    }
    fclose(fp);
    free(predicted_motion);
    free(actual_motion);
    return;
}

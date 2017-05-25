#include<string.h>
#include "fann.h"
#include "ANN_1_train.h"

/*
 * NAME = username to append to .net file
 * */
int ANN_1_train(char NAME[])
{
    const unsigned int num_input = 43; //43 features
    const unsigned int num_output = 6;
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 9;
    const float desired_error = (const float) 0.0001;
    const unsigned int max_epochs = 5000;
    const unsigned int epochs_between_reports = 100;

    struct fann *ann = fann_create_standard(num_layers, num_input,
        num_neurons_hidden, num_output);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

    //construct the training file name:
    char train_file[100];   // array to hold the result.
    char out_file[5] = ".txt";
    strcpy(train_file, "/home/root/database/");
    strcat(train_file, NAME);
    strcat(train_file, "_");
    strcat(train_file, "ANN_1");
    strcat(train_file, out_file); // copy string one into the result.

    fann_train_on_file(ann, train_file, max_epochs,
        epochs_between_reports, desired_error);

    char result[100];   // array to hold the result.
    strcpy(result, "/home/root/database/");
    strcat(result, NAME);
    strcat(result, "_RESULT_ANN1.net");
    fann_save(ann, result);

    fann_destroy(ann);

    return 0;
}

#include<string.h>
#include "fann.h"
#include "ANN_train.h"

#define NO_INPUTS 43
/*
 * NAME = username to append to .net file
 * num_output = number of output for neuron network
 * ann = ANN (e.g. "ANN_1", "ANN_2_1")
 * */
void ANN_train(const char NAME[], const unsigned int num_output, const char ann_name[])
{
    const unsigned int num_input = NO_INPUTS; //43 features
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 11;
    const float desired_error = (const float) 0.0001;
    const unsigned int max_epochs = 5000;
    const unsigned int epochs_between_reports = 100;

    fann_type steepness = 0.9; //changeable

    struct fann *ann = fann_create_standard(num_layers, num_input,
        num_neurons_hidden, num_output);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

    fann_set_activation_steepness(ann, steepness, num_layers, num_neurons_hidden); //activation steepness

    //construct the training file name:
    char train_file[100];   // array to hold the result.
    char out_file[5] = ".txt";
    strcpy(train_file, "/home/root/database/");
    strcat(train_file, NAME);
    strcat(train_file, "/");//NB! NAME is subdirectory for the user
    strcat(train_file, ann_name);
    strcat(train_file, out_file); // copy string one into the result.
    fann_train_on_file(ann, train_file, max_epochs,
        epochs_between_reports, desired_error);

    char result[100];   // array to hold the result.
    strcpy(result, "/home/root/database/");
    strcat(result, NAME);
    strcat(result, "/RESULT_");//NB! NAME is subdirectory for the user
	//NB! use "ANN_1" or "ANN_2_1" and so on
	strcat(result, ann_name);
	strcat(result, ".net");
    fann_save(ann, result);

    fann_destroy(ann);
}

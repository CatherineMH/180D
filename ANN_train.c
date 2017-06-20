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
    const unsigned int num_layers = 3; //4
    const unsigned int num_neurons_hidden = 10;
    const float desired_error = (const float) 0.0001;
    const unsigned int max_epochs = 5000;
    const unsigned int epochs_between_reports = 100;
    fann_type steepness = 1.0;
    
    
    //enum fann_train_enum training_algorithm = FANN_TRAIN_RPROP;
    //float 	connection_rate 10.0;

    struct fann *ann = fann_create_standard(num_layers, num_input,
        num_neurons_hidden, num_output);


    //fann_set_training_algorithm(ann,training_algorithm);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);
    
    fann_set_activation_steepness(ann, steepness, num_layers, num_neurons_hidden);

   /* if(training_algorithm == FANN_TRAIN_QUICKPROP){
	                fann_set_learning_rate(ann, 0.35);
	                fann_randomize_weights(ann, -2.0,2.0);}    */
    

     fann_set_learning_rate(ann, 0.7);
     
     //fann_create_sparse(connection_rate, num_layers, num_input, um_neurons_hidden, num_output )

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

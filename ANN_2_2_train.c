#include "fann.h"
/*
 * ******************UPSTAIRS SPEED DETECTION ANN TRAINING FILE****************************
 * */
int main()
{
    const unsigned int num_input = 43; //43 features
    const unsigned int num_output = 3; //0-slow, 1-med, 2-fast
    const unsigned int num_layers = 3;
    const unsigned int num_neurons_hidden = 9;
    const float desired_error = (const float) 0.0001;
    const unsigned int max_epochs = 5000;
    const unsigned int epochs_between_reports = 100;

    struct fann *ann = fann_create_standard(num_layers, num_input,
        num_neurons_hidden, num_output);

    fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
    fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

    fann_train_on_file(ann, "ANN2_ANN2_up.txt", max_epochs,
        epochs_between_reports, desired_error);

    fann_save(ann, "ANN_2.net");  //ANN_1.net-walk speed, ANN_2-upstairs speed, ANN_3-downstairs speed, ANN_4-jump height

    fann_destroy(ann);

    return 0;
}

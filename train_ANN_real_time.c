/*
 * train_ANN_real_time
 *
 * gcc -Wall -o train_ANN_real_time train_ANN_real_time.c ANN_1_train.c ANN_2_1_train.c ANN_2_2_train.c ANN_2_3_train.c ANN_2_4_train.c -lmraa -lfann
 *
 * */

//include all code for training ANNs:
#include "ANN_1_train.h"
#include "ANN_2_1_train.h"
#include "ANN_2_2_train.h"
#include "ANN_2_3_train.h"
#include "ANN_2_4_train.h"

int main()
{
    //read all usernames to be processed from a text file: (TBD!)

    //call the ANN training routines to train each ANN on the user's .txt files:
    ANN_1_train("Hanna");
    ANN_2_1_train("Hanna");
    ANN_2_2_train("Hanna");
    ANN_2_3_train("Hanna");
    ANN_2_4_train("Hanna");

    return 0;
}

//smooths a signal by computing the moving average over M data points (M must be even!)
//time = time stamps corresponding to data
//data = rough input data to smooth, not modified
//returns nothing, modifies input argument smooth_data

void moving_average(int NO_SAMPLES, int M, float* time, float* data, float* smooth_data)
{
    float sum = 0;
    int i = 0, j = 0;
    for(; i< M/2; i++)
    {
        sum = 0; //refresh sum
        for(j = 0; j < M; j++)
        {
            sum += data[i+j]; //accumulate sum
        }
        smooth_data[i] = sum/M; //store averaged datapoint
    }
    for(i = M/2; i< NO_SAMPLES - M/2; i++)
    {
        sum = 0; //refresh sum
        for(j = -M/2; j < M/2; j++)
        {
            sum += data[i+j]; //accumulate sum
        }
        smooth_data[i] = sum/M; //store averaged datapoint
    }
    for(; i< NO_SAMPLES; i++)
    {
        sum = 0; //refresh sum
        for(j = -M/2; j < 0; j++)
        {
            sum += data[i+j]; //accumulate sum
        }
        smooth_data[i] = sum/M; //store averaged datapoint
    }
}



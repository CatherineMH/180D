//TODO: Turning files start with turns instead of walk sometimes
//TODO: We can make the system to create the "database" folder
//gcc -lmraa -o DataCollection_New-2 DataCollection_New-2.c -lm LSM9DS0.c

#include <stdio.h>
#include <mraa/i2c.h>
#include "LSM9DS0.h"
#include <math.h>
#include<time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>  ///NB!
#include "user_data_collection.h"
#include <sys/types.h>
#include <sys/stat.h>

#define MILLION 1000000.0f
#define BUFF_SIZE 1024
//#define T_walk_slow 50
//#define T_walk_medium 35
//#define Twalk_fast 20
//#define T_jump 20
//#define T_turn 60
//#define T_stair_slow 30
//#define T_stair_medium 15
//#define T_stair_fast 10

#define TWS 5
#define TW 3
#define T3 2
#define T4 4
#define TJS 5

#define T1 5
#define T2 3
#define T3 2
#define T4 4

#define T_WALK_SLOW 50
#define T_WALK_MED 35
#define T_WALK_FAST 30
#define T_JUMP_SHORT 20
#define T_JUMP_MED 20
#define T_JUMP_HIGH 15
#define T_UP_S 13
#define T_UP_M 8
#define T_UP_F 5
#define T_DOWN_S 13
#define T_DOWN_M 8
#define T_DOWN_F 5
#define T_RIGHT 50
#define T_LEFT 50

sig_atomic_t volatile run_flag = 1;

/***************Functions**********************************/
void do_when_interrupted(int sig)
{
    if (sig == SIGINT)
        run_flag = 0;
}

double parse_time(struct timeval *time)
{
    return (double) time->tv_sec + (double) time->tv_usec/MILLION; //NB! timeval doesn't contain member named time_sec
}

//Activity Data Collection
//append = 1 if user wants to append more data to existing file
//last_time tells the function what the last time in the file is if the user is appending more data to an existing file
float collect_data(const char Name[], char Activity[], int act_time, int append, float last_time){
	//Variables
	char  *ifile = (char *)malloc(sizeof(char)* BUFF_SIZE);
	FILE *fp;
	mraa_i2c_context accel, gyro;
	accel_scale_t    a_scale = A_SCALE_16G;
	gyro_scale_t     g_scale = G_SCALE_2000DPS;
	data_t 			 accel_data, gyro_data;//NB! changed variables accData, gyroData;
	float 			 a_res, g_res;
	int 			fd;
	struct timeval tv_before, tv_after, tv_start, tv_current;
   

	time_t timestamp_sec, timeLeft; /* timestamp in seconds */
	time(&timestamp_sec);  /* get current time */

	time_t start_time_sec; /* loop start timestamp in seconds */
	time_t stop_time_sec; /* loop end timestamp in seconds */

	/*Initialize timestamp variables for loop:*/

	struct timeval tv;
	double timestart_usec; /* timestamp in microsecond */
	double timeafter_usec;
	double initial_time_usec; //timestamp to record when data collection starts
    
    struct stat st = {0};
    char required_dir[100];
    strcpy(required_dir, "/home/root/database/");
    strcat(required_dir, Name);
    if (stat(required_dir, &st) == -1) {
        mkdir(required_dir, 0700);
    }

	memset(ifile, 0, BUFF_SIZE);
	//Store file in "database" directory Check: with system call if the file exists
	snprintf(ifile, BUFF_SIZE, "%s/%s.csv", required_dir, Activity);
	signal(SIGINT, &do_when_interrupted);

	//Collect data from sensors
	accel = accel_init();
	set_accel_scale(accel, a_scale);
	a_res = calc_accel_res(a_scale);

	gyro = gyro_init();
	set_gyro_scale(gyro, g_scale);
	g_res = calc_gyro_res(g_scale);


	//Read the sensor data and print them.
    // NB!
	//while (run_flag)
	//{
	/* get time corresponding to start of data collection for current file */
	time(&start_time_sec);
	//compute initial time:
	gettimeofday(&tv, NULL);
	initial_time_usec = tv.tv_sec + tv.tv_usec/MILLION;

	// NB! act_timen -> act_time
	stop_time_sec = start_time_sec + act_time; //stop time based on how long we want to collect data for



	//open new file
	printf("data files: %s\n", ifile); // NB! file_name does not exists, so, ifile instead of file_name
	if(append)
		fp = fopen(ifile, "a");
	else
		fp = fopen(ifile, "w"); // NB! file_name does not exist, so, ifile instead of file_name
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to open file \'%s\'. Exiting.\n",
			ifile // NB! file_name does not exists, so, ifile instead of file_name
			);
		printf("%d\n", errno);
		exit(EXIT_FAILURE);
	}
	//lock file
	fd = fileno(fp);
	flock(fd, LOCK_EX);

	if(!append) //only put the header if we are not appending to existing file
		fprintf(fp, "%s,%s,%s,%s,%s,%s,%s,%s\n",
		"time_before", "time_after",
		"accel_x", "accel_y", "accel_z",
		"gyro_x", "gyro_y", "gryo_z"
		);
	//int cur = 0; //NB! time outputs writes to the stdout too often
	while (time(NULL) <= stop_time_sec && run_flag)
	{
		//cur++; //NB!
		(timeLeft = act_time - (time(NULL) - start_time_sec));
		//if (cur >= 1000) {
		//    cur = 0;
			printf("Time Left:%d     \r", timeLeft);
		//}
		gettimeofday(&tv, NULL);
		timestart_usec = tv.tv_sec + tv.tv_usec/MILLION;
		timestart_usec -= initial_time_usec;

		accel_data = read_accel(accel, a_res); //NB!
		gyro_data = read_gyro(gyro, g_res);  //NB!
		gettimeofday(&tv_after, NULL); // NB! no variable named time_after only tv_after

		gettimeofday(&tv, NULL);
		timeafter_usec = tv.tv_sec + tv.tv_usec/MILLION;
		timeafter_usec -= initial_time_usec;
		//output collected data to file:
		fprintf(fp, "%10.6lf,%10.6lf,%+f,%+f,%+f,%+f,%+f,%+f\n",
				timestart_usec+last_time,
				timeafter_usec+last_time,
				accel_data.x,
				accel_data.y,
				accel_data.z,
				gyro_data.x,
				gyro_data.y,
				gyro_data.z
		);
		usleep(1500);
		gettimeofday(&tv_current, NULL);
	}
	printf("\n");
	fclose(fp);
	//}
	return timeafter_usec+last_time;
}

//Turning Data Collection
void collect_data_turn(const char Name[], char Activity[], int act_time){
    mraa_i2c_context accel, gyro, mag;
    float a_res, g_res, m_res;
    accel_scale_t a_scale;
    gyro_scale_t g_scale;
    mag_scale_t m_scale;
    data_t ad, gd, md;
    data_t Go;
    int fd, track;

    a_scale = A_SCALE_16G; //make sure to catch all features in stride!
    g_scale = G_SCALE_2000DPS;
    m_scale = M_SCALE_2GS;

    accel = accel_init();
    set_accel_ODR(accel, A_ODR_1600); //set to max bandwidth!
    set_accel_scale(accel, a_scale);
    a_res = calc_accel_res(a_scale);

    gyro = gyro_init();
    set_gyro_ODR(gyro, G_ODR_760_BW_100); //set to maximum!
    set_gyro_scale(gyro, g_scale);
    g_res = calc_gyro_res(g_scale);

    mag = mag_init();
    set_mag_ODR(mag, M_ODR_125);
    set_mag_scale(mag, m_scale);
    m_res = calc_mag_res(m_scale);

    //Go = calc_gyro_offset(gyro, g_res); //gyroscope offset
    struct stat st = {0};
    char required_dir[100];
    strcpy(required_dir, "/home/root/database/");
    strcat(required_dir, Name);
    if (stat(required_dir, &st) == -1) {
        mkdir(required_dir, 0700);
    }
    //get output file name from user:
	char  *output_file = (char *)malloc(sizeof(char)* BUFF_SIZE);
	memset(output_file, 0, BUFF_SIZE);
	//Store file in "database" directory Check: with system call if the file exists
	snprintf(output_file, BUFF_SIZE, "%s/%s.csv", required_dir, Activity);
    printf("Attempting to write to file \'%s\'.\n", output_file);
    FILE* fp = fopen(output_file, "w");
    if (fp == NULL) {
        fprintf(stderr,
                "Failed to write to file \'%s\'.\n",
                output_file
        );
        exit(EXIT_FAILURE);
    }

    //lock file
	  fd = fileno(fp);
	  flock(fd, LOCK_EX);
     
    fprintf(fp, "timestamp_before,timestamp_after,accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z\n");
    time_t timestamp_sec; /* timestamp in seconds */
    time(&timestamp_sec);  /* get current time; same as: timestamp_sec = time(NULL)  */

    time_t start_time_sec; /* loop start timestamp in seconds */
    time(&start_time_sec);  /* get current time; same as: timestamp_sec = time(NULL)  */
    int turn_stride = 0; /* indicates whether or not the current data point belongs to a turn */
    int turn_time = 4; /* determines how many seconds to wait between issuing 'turn' prompts to user*/
    time_t stop_time_sec = start_time_sec + act_time; //stop time based on how long we want to collect data for
    /*Initialize timestamp variables for loop:*/

    struct timeval tv;
    double timestart_usec; /* timestamp in microsecond */
    double timeafter_usec;
    double initial_time_usec; //timestamp to record when data collection starts

    //compute initial time:
    gettimeofday(&tv, NULL);
    initial_time_usec = tv.tv_sec + tv.tv_usec/MILLION;
    //while we haven't reached data collection end time:
    while(time(NULL)<stop_time_sec){
        
        if(time(NULL) != 0 && time(NULL)%turn_time == 0) //promt the user to turn
        {
            printf("\n\n\n\n\n\n\n\n\n\n");
            printf("TURN\n");
            usleep(1500);
            //system("clear");
            turn_stride = 1;
        }
        else
        {
            printf("\n\n\n\n\n\n\n\n\n\n");
            printf("WALK\n");
            if(track==0)
                printf("_ _ _ _ TURN");
            if(track==1)
                printf(" _ _ TURN");
            if(track==2) {
                printf(" _ TURN");
                track=0;
            }

            usleep(1500);
            //system("clear");
            turn_stride = 0;
            track++;
        }

        gettimeofday(&tv, NULL);
        timestart_usec = tv.tv_sec + tv.tv_usec/MILLION;
        timestart_usec -= initial_time_usec;
        //time(&timestamp_before); //record time before acceleration data is read
        ad = read_accel(accel, a_res);
        gd = read_gyro(gyro, g_res);
        //time(&timestamp_after); //record time after angular velocity data is read
        gettimeofday(&tv, NULL);
        timeafter_usec = tv.tv_sec + tv.tv_usec/MILLION;
        timeafter_usec -= initial_time_usec;
        //md = read_mag(mag, m_res);

        //discard gyro offset for now
        gd.x = gd.x; //- Go.x;
        gd.y = gd.y; //- Go.y;
        gd.z = gd.z; //- Go.z;

        //output data to file:
        fprintf(fp, "%10.6lf,%10.6lf,%+f,%+f,%+f,%+f,%+f,%+f,%+d\n",
                timestart_usec,
                timeafter_usec,
                ad.x,
                ad.y,
                ad.z,
                gd.x,
                gd.y,
                gd.z,
                turn_stride*50
        );

        usleep(1500); //write file at .0015 second intervals
        system("clear");
    }
    fclose(fp);
}
/*******************End of Functions********************************/
//Main Activity
void collect_user_data(const char USERNAME[]){
	char answer;
	char *Nfile_name = "Usernames.txt";
	FILE *np; //NB! All letter should be capital
    int Activity, ch;
	int append = 0;
	float last_time = 0;
    
	printf("Hello, Let's train neural net with a set of activities...\n");

	for (Activity = 1; Activity < 15; Activity++)
    {
        answer = 'n'; //NB! reset answer
		switch (Activity) {
		case 1:
			while ((answer != 'y') && run_flag) {
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************WALKING SLOWLY*************\n");
					printf("Please start walking slowly for 50sec\n");
					sleep(1);
					collect_data(USERNAME, "ws", T_WALK_SLOW, 0, 0); //walking slow
					printf("Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();//while((ch = getchar()) != EOF || ch != '\n');
				}
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? "); //NB! user does not want to know previous answer
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 2:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************MEDIUM WALKING*************\n");
                    printf("Please start walking in medium speed for 35sec\n");
					sleep(1);
				    collect_data(USERNAME, "wm", T_WALK_MED, 0, 0); //walking medium
                     printf("Are you satisfied with this activity (y/n)? ");
                    scanf("%c", &answer);
                    //ch = getchar();
				}
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 3:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************WALKING FAST*************\n");
                    printf("Please start walking fast for 15sec\n");
                    sleep(1);
                    collect_data(USERNAME, "wf", T_WALK_FAST, 0, 0); //walking fast
                    printf("Are you satisfied with this activity?(y/n) ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else {
                    printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
			}
			break;
		case 4:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************short jumps*************\n");
                    printf("Please start jumping with the shortest height you can\n");
                    sleep(1);
                    collect_data(USERNAME, "js", T_JUMP_SHORT, 0, 0); //jump short
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else  {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 5:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************MEDIUM JUMP*************\n");
                    printf("Please start jumping with a medium height\n");
                    sleep(1);
                    collect_data(USERNAME, "jm", T_JUMP_MED, 0, 0);//jump medium
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else{
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 6:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************HIGH JUMP*************\n");
                    printf("Please start jumping as high as you can\n");
                    sleep(1);
                    collect_data(USERNAME, "jh", T_JUMP_HIGH, 0, 0); //jump high
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 7:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n' || answer == 'm') {
					append = (answer == 'm')? 1:0;
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************SLOW STAIR ASCENT*************\n");
                    printf("Please start walking up the stairs for 50 sec (~15 stairs) as slow as possible\n");
                    sleep(1);
                    last_time = collect_data(USERNAME, "sus", T_UP_S, append, last_time);//stair up slow
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else  {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n/m)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 9:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n' || answer == 'm') {
					append = (answer == 'm')? 1:0;
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************MEDIUM STAIR ASCENT*************\n");
                    printf("Please start walking up the stairs for 35 sec (~15 stairs) with a medium speed\n");
                    sleep(1);
                    last_time = collect_data(USERNAME, "sum", T_UP_M, append, last_time);//stair up medium
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n/m)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 11:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n' || answer == 'm') {
					append = (answer == 'm')? 1:0;
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************FAST STAIR ASCENT*************\n");
                    printf("Please start walking up the stairs for 20 sec (~15 stairs) as slow as possible\n");
                    sleep(1);
                    last_time = collect_data(USERNAME, "suf", T_UP_F, append, last_time);//stair up fast
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 8:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n' || answer == 'm') {
					append = (answer == 'm')? 1:0;
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************SLOW STAIR DESCENT*************\n");
                    printf("Please start walking down the stairs for 50 sec (~15 stairs) as slow as possible\n");
                    sleep(1);
                    last_time = collect_data(USERNAME, "sds", T_DOWN_S, append, last_time);//stair down slow
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 10:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n' || answer == 'm') {
					append = (answer == 'm')? 1:0;
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************MEDIUM STAIR DESCENT*************\n");
                    printf("Please start walking down the stairs for 35 sec (~15 stairs) with a medium speed\n");
                    sleep(1);
                    last_time = collect_data(USERNAME, "sdm", T_DOWN_M, append, last_time);//stair down medium
                    printf("Are you satisfied with this activity? ");
                    scanf("%c", &answer);
                    //ch = getchar();
                }
				else {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 12:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n' || answer == 'm') {
					append = (answer == 'm')? 1:0;
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************FAST STAIR DESCENT*************\n");
					printf("Please start walking down the stairs for 20 sec (~15 stairs) as fast as possible\n");
					sleep(1);
					last_time = collect_data(USERNAME, "sdf", T_DOWN_F, append, last_time);//stair down fast
					printf("Are you satisfied with this activity? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
				else  {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();

				}
			}
			break;
		case 13:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************TURN RIGHT*************\n");
					printf("Please start turning right around a square according to the following instructions\n");
					printf("Please note that you should turn each 4 seconds\n");
					sleep(1);
					collect_data_turn(USERNAME, "tr", T_RIGHT);//turn right
					printf("Are you satisfied with this activity? ", answer);
					scanf("%c", &answer);
                    //ch = getchar();
				}
				else  {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                    //ch = getchar();
				}
			}
			break;
		case 14:
			while ((answer != 'y')&&run_flag){
				if (answer == 'n') {
                    system("clear");
                    printf("\n\n\n\n\n\n\n\n\n\n");
                    printf("***************TURN LEFT*************\n");
					printf("Please start turning left around a square according to the following instructions\n");
					printf("Please note that you should turn each 4 seconds\n");
					sleep(1);
					collect_data_turn(USERNAME, "tl", T_LEFT);//turn left
					printf("Are you satisfied with this activity? \n");
					scanf("%c", &answer);
                    //ch = getchar();
				}
				else /*if ((answer != 'n')&&(answer != 'y'))*/ {
					printf("Answer is not correct\n Are you satisfied with this activity (y/n)? ");
					scanf("%c", &answer);
                   // ch = getchar();
				}
			}
			break;
		default:
			printf("Invalid activity\n");
		}
	}

	//Now, write the name of the person to the "Username.txt" file
	printf("Attempting to write to file \'%s\'.\n", Nfile_name);
	np = fopen(Nfile_name, "a");
	if (np == NULL) {
		fprintf(stderr,
			"Failed to write to file \'%s\'.\n",
			Nfile_name // NB! changed name
			);
		exit(EXIT_FAILURE);
	}
	fprintf(np, "%s\n", USERNAME); //NB!
	fclose(np); //NB!
}

int main_() {
	char USERNAME[30];
	printf("Hello, Let's train neural net with a set of activities...\n");
	printf("Could you tell me your name please? ");
	scanf("%s", USERNAME);
	collect_user_data(USERNAME);
	return 0;
}

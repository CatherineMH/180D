CC=gcc
LDFLAGS=-lmraa -lfann
CFLAGS=-Wall
SOURCES= extract_real_time.c collect_data.c train_ANN_real_time.c
EXECUTABLES=$(SOURCES:.c=)

all: extract_real_time collect_data train_ANN_real_time

collect_data: collect_data.c
	$(CC) -lmraa -lm -o collect_data collect_data.c LSM9DS0.c

extract_real_time: extract_real_time.c
	$(CC) $(CFLAGS) -o extract_real_time extract_real_time.c ex_find_maxima_rig_zgyro.c process_file.c neural_nets.c Moving_Avg_Filter.c -lm $(LDFLAGS)

train_ANN_real_time: train_ANN_real_time.c
	$(CC) $(CFLAGS) -o train_ANN_real_time train_ANN_real_time.c LSM9DS0.c Moving_Avg_Filter.c ex_find_maxima_rig_zgyro.c training_auto.c neural_nets.c user_data_collection.c ANN_train.c process_file.c -lm $(LDFLAGS)

clean:
	rm -f  extract_real_time collect_data train_ANN_real_time
	rm -f *~
	rm -f TEST.net
	rm -f test_data.txt

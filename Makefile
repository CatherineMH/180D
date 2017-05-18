CC=gcc
LDFLAGS=-lmraa -lfann
CFLAGS=-Wall
SOURCES= ANN_2_3_train.c ANN_2_4_train.c ANN_2_1_train.c ANN_2_2_train.c  collect_neural_net_data.c examine_sensor_data.c test_neural_network.c train_neural_net.c extract_test.c ann1_train_neural_net.c collect_data.c extract_real_time.c
EXECUTABLES=$(SOURCES:.c=)

all: ANN_2_3_train ANN_2_4_train ANN_2_1_train ANN_2_2_train  collect_neural_net_data examine_sensor_data test_neural_network train_neural_net extract_test ann1_train_neural_net collect_data extract_real_time

ANN_2_1_train:
	$(CC) $(CFLAGS) -o ANN_2_1_train ANN_2_1_train.c $(LDFLAGS)

ANN_2_2_train:
	$(CC) $(CFLAGS) -o ANN_2_2_train ANN_2_2_train.c $(LDFLAGS) 

ANN_2_3_train:
	$(CC) $(CFLAGS) -o ANN_2_3_train ANN_2_3_train.c $(LDFLAGS)

ANN_2_4_train:
	$(CC) $(CFLAGS) -o ANN_2_4_train ANN_2_4_train.c $(LDFLAGS)

collect_neural_net_data: collect_neural_net_data.c
	$(CC) $(CFLAGS) -o collect_neural_net_data collect_neural_net_data.c $(LDFLAGS)

examine_sensor_data: examine_sensor_data.c
	$(CC) $(CFLAGS) -o examine_sensor_data examine_sensor_data.c $(LDFLAGS)

test_neural_network: test_neural_network.c
	$(CC) $(CFLAGS) -o test_neural_network test_neural_network.c $(LDFLAGS)

train_neural_net: train_neural_net.c
	$(CC) $(CFLAGS) -o train_neural_net train_neural_net.c $(LDFLAGS)

extract_test: extract_test.c
	$(CC) $(CFLAGS) -o extract_test extract_test.c -lm $(LDFLAGS)

ann1_train_neural_net: ann1_train_neural_net.c
	$(CC) $(CFLAGS) -o ann1_train_neural_net ann1_train_neural_net.c $(LDFLAGS) 

collect_data: collect_data.c
	$(CC) -lmraa -lm -o collect_data collect_data.c LSM9DS0.c

extract_real_time: extract_real_time.c
	$(CC) $(CFLAGS) -o extract_real_time extract_real_time.c -lm $(LDFLAGS)
clean: 
	rm -f ANN_2_3_train ANN_2_4_train ANN_2_1_train ANN_2_2_train  collect_neural_net_data examine_sensor_data test_neural_network train_neural_net extract_test ann1_train_neural_net collect_data extract_real_time
	rm -f *~
	rm -f TEST.net
	rm -f test_data.txt

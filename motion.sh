#!/bin/bash
#
#echo ""
#echo ">>>>COMPILING<<<<"
make clean
make
#
#echo ""
echo ">>>>EXECUTING collect_data <<<<"
./collect_data &
COLLECT_ID=$!
echo ""
echo ">>>>EXECUTING extract_real_time <<<<"
./extract_real_time $1 &
EXTRACT_ID=$!
read -p "Press any key to kill producer/consumer... " -n1 -s
kill $COLLECT_ID $EXTRACT_ID

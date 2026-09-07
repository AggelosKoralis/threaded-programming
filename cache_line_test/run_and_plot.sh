#!/bin/bash

rm -f ./serial.txt ./threaded.txt

gcc -o main main.c -Wall -Wextra -Werror -g3 -lpthread
if [ $(echo $?) != 0 ]
then
    echo "Error: main failed to compile"
    exit 1
else
    echo "main compiled succesfully"
fi

# starting value
value=5000000

for i in {1..7}
do
    echo "Running ./main $value"
    
    ./main $value
    if [ $(echo $?) != 0 ]
    then
        echo "Error: main failed"
        exit 1
    fi
    echo ""
    
    value=$(( value * 2 ))
done

python3 plot.py

echo "Exiting"

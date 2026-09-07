#!/bin/bash

run () {
    local degree1=$1
    local degree2=$2
    local threads=$3

    for i in {1..4}
    do
        echo "Running ./main $degree1 $degree2 $threads"
        
        ./main $degree1 $degree2 $threads
        if [ $(echo $?) != 0 ]
        then
            echo "Error: main failed"
            exit 1
        fi
        
        # increase input 
        degree2=$(( $degree2 * 2 ))

    done
}

rm -f ./results/*.txt
gcc -o main main.c  -Wall -Wextra -Werror -g3 -lpthread
if [ $(echo $?) != 0 ]
then
    echo "Error: main failed to compile"
    exit 1
else
    echo "main compiled succesfully"
fi


# start with 2 threads
degree1=1000000
degree2=1000
threads=1
run $degree1 $degree2 $threads

# redo calclations with more threads
degree1=1000000
degree2=1000
threads=2
run $degree1 $degree2 $threads 

degree1=1000000
degree2=1000
threads=4
run $degree1 $degree2 $threads 

degree1=1000000
degree2=1000
threads=8
run $degree1 $degree2 $threads 

python3 plot.py

echo ""
echo "Exiting"

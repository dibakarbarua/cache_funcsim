#!/bin/bash

echo "****************************"
echo "***Default Configuration ***"
echo "****************************"

for i in `ls ../traces/`; do
    echo ""
    echo "--- $i ---"
	./cachesim -i ../traces/$i
done;

echo ""
echo "****************************"
echo "***    C = 10, S = 0     ***"
echo "****************************"

for i in `ls ../traces/`; do
    echo ""
    echo "--- $i ---"
	./cachesim -c 10 -s 0 -i ../traces/$i
done;

echo ""
echo "****************************"
echo "***    V = 0, K = 0      ***"
echo "****************************"

for i in `ls ../traces/`; do
    echo ""
    echo "--- $i ---"
	./cachesim -v 0 -k 0 -i ../traces/$i
done;

echo ""
echo "****************************"
echo "***        V = 0         ***"
echo "****************************"

for i in `ls ../traces/`; do
    echo ""
    echo "--- $i ---"
	./cachesim -v 0 -i ../traces/$i
done;

echo ""
echo "****************************"
echo "***        K = 0         ***"
echo "****************************"

for i in `ls ../traces/`; do
    echo ""
    echo "--- $i ---"
	./cachesim -k 0 -i ../traces/$i
done;

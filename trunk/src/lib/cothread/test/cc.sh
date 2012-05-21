clear
g++ -O3 -fomit-frame-pointer -c test_timing.cpp
gcc -O3 -fomit-frame-pointer -o libco.o -c ../libco.c
g++ -O3 -fomit-frame-pointer test_timing.o libco.o -o test_timing
rm -f *.o

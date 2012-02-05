mingw32-gcc -O3 -fomit-frame-pointer -o libco.o -c ../libco.c
mingw32-g++ -O3 -fomit-frame-pointer -c test_timing.cpp
mingw32-g++ -O3 -fomit-frame-pointer test_timing.o libco.o -o test_timing.exe
@del *.o

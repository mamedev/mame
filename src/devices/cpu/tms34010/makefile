dis34010.exe : dis34010.o -lalleg
		gcc -m486 -O3 -ffast-math -fomit-frame-pointer -finline-functions dis34010.o -lalleg -o dis34010.exe

dis34010.o : dis34010.c 34010dsm.c
		gcc -c -m486 -O3 -ffast-math -fomit-frame-pointer -finline-functions -I/mame/src -I/mame/src/msdos dis34010.c


..\..\..\..\dismips.exe: dismips.c psxdasm.c ..\mips\r3kdasm.c ..\mips\mips3dsm.c ../../../lib/util/corestr.c
	gcc -O3 -x c++ -Wall -Wno-sign-compare -I../../../emu -I../../../osd -I../../../lib/util -I../../../lib/expat -DINLINE="static __inline__" -DSTANDALONE -DLSB_FIRST dismips.c psxdasm.c ..\mips\r3kdasm.c ..\mips\mips3dsm.c ../../../lib/util/corestr.c -o../../../../dismips

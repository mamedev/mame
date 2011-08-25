..\..\..\..\dismips.exe: dismips.c psxdasm.c r3kdasm.c mips3dsm.c ../../../lib/util/corestr.c
	gcc -O3 -x c++ -Wall -Wno-sign-compare -I../../../emu -I../../../osd -I../../../lib/util -DINLINE="static __inline__" -DSTANDALONE -DLSB_FIRST dismips.c psxdasm.c r3kdasm.c mips3dsm.c ../../../lib/util/corestr.c -o../../../../dismips

..\..\..\..\dismips.exe: dismips.c mipsdasm.c r3kdasm.c mips3dsm.c ../../../lib/util/corestr.c
	gcc -O3 -Wall -I../../../emu -I../../../osd -I../../../lib/util -DINLINE="static __inline__" -DSTANDALONE -DLSB_FIRST dismips.c mipsdasm.c r3kdasm.c mips3dsm.c ../../../lib/util/corestr.c -o../../../../dismips

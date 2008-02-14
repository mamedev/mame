..\..\..\..\dismips.exe: dismips.c mipsdasm.c r3kdasm.c mips3dsm.c ../../../lib/util/corestr.c
	gcc -O3 -Wall -I../../../emu -I../../../lib/util -I../../../osd -DINLINE="static __inline__" -DSTANDALONE -DMAME_DEBUG -DLSB_FIRST dismips.c mipsdasm.c r3kdasm.c mips3dsm.c ../../../lib/util/corestr.c -o../../../../dismips

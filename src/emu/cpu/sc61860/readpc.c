// license:BSD-3-Clause
// copyright-holders:Peter Trauner
#if 0
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <dos.h>

// gcc -O
// important because of inb, outb

/*
   dumps data sent form sharp pc1401 into file
   at sharp the following program must be started after
   this dos program was started
   simple dos utility
   first arg (filename of file to be written) must be specified

   ct eprop needed
   changing to parallel port should be easy

nearly all sharp pocket pc's system roms should be readable
this way.

for some early pocket pc's it would be necessary to load
this program into memory.
(they don't have poke/peek/call instructions,
so this program can't be typed in)

adapter sharp - cteprommer

1
2 6v
3 gnd                      8 bit textool gnd pin 20
4 f0    data-->            8 bit textool data 0 pin 17
5 f1    handshake -->      8 bit textool data 1 pin 18
6 save
7 load
8 ib7   <--handshake       8 bit textool address 0 pin 16
9 ib9
10
11

problems with the handshake in at the sharp
so I didn't use the handshake in the sharp side
but the pc side must not be interrupted with interrupt
(pure dos needed?)

*/


#define outb(v,adr) outportb(adr,v)
#define inb(adr) inportb(adr)

#if 0
		/* this routine dump the memory (start 0)
		   in an endless loop,
		   the pc side must be started before this
		   actual version should be in mess/machine/pocketc.c
		*/

1 restore: for i=16384 to 16455:read a: poke i,a: next i
10 call 16384
100 data
#if 1
		18,4,//lip xl
		2,0,//lia 0 startaddress low
		219,//exam
		18,5,//lip xh
		2,0,//lia 0 startaddress high
		219,//exam
//400f x:
		// dump internal rom
		18,5,//lip 4
		89,//ldm
		218,//exab
		18,4,//lip 5
		89,//ldm
		4,//ix for increasing x
		0,0,//lii,0
		18,20,//lip 20
		53, //
		18,20,// lip 20
		219,//exam
#else
		18,4,//lip xl
		2,255,//lia 0
		219,//exam
		18,5,//lip xh
		2,255,//lia 0
		219,//exam
//400f x:
		// dump external memory
		4, //ix
		87,//                ldd
#endif
		218,//exab



		0,4,//lii 4

		//a:
		218,//                exab
		90,//                 sl
		218,//                exab
		18,94,//            lip 94
		96,252,//                 anma 252
		2,2, //lia 2
		196,//                adcm
		95,//                 outf
		//b:
		204,//inb
		102,128,//tsia 0x80
#if 0
		41,4,//            jnzm b
#else
		// input not working reliable!
		// so handshake removed, PC side must run with disabled
		// interrupt to not lose data
		78,20, //wait 20
#endif

		218,//                exab
		90,//                 sl
		218,//                exab
		18,94,//            lip 94
		96,252,//anma 252
		2,0,//                lia 0
		196,//adcm
		95,//                 outf
		//c:
		204,//inb
		102,128,//tsia 0x80
#if 0
		57,4,//            jzm c
#else
		78,20, //wait 20
#endif

		65,//deci
		41,34,//jnzm a

		41,41,//jnzm x:

		55,//               rtn
// fill up with several 55
// so you don't have to calculate the exact end in the loop
#endif

#define PORT_BASE 0x3e0

int main(int argc, char *argv[])
{
	unsigned i,v,j,a;
	char buffer[32768];
	FILE *out=fopen(argv[1],"wb");

#if 0
	ioperm(PORT_BASE,8,1);
	nice(-256);
#endif
	asm { cli }

	outb(0x18,PORT_BASE+3); // VCC ON
	outb(0xc0, PORT_BASE+2);
	outb(7,PORT_BASE+6); // VPP 12.5, reset off

	outb(0,PORT_BASE);
	for(j=0;j<0x8000;j++) {
		for (i=0,v=0;i<4;i++) {
			for(;((a=inb(PORT_BASE+4))&2)==0; ) ;
			v=(v<<1)|(a&1);
			outb(1,PORT_BASE);
			for(;((a=inb(PORT_BASE+4))&2)!=0; ) ;
			v=(v<<1)|(a&1);
			outb(0,PORT_BASE);
		}
		buffer[j]=v;
//      printf("%.2x\n",v);
	}
	asm { sti }
	fwrite(buffer,2,0x4000,out);
	fclose(out);

#if 0
	outb(8,PORT_BASE+2); // led off
	outb(0,PORT_BASE+3); //VCC off
	outb(0,PORT_BASE+6); //VPP off
#endif
	return 0;
}

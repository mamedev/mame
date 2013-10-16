// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   scmpdasm.c
 *
 *   National Semiconductor SC/MP CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]

CPU_DISASSEMBLE( scmp )
{
	unsigned PC = pc;
	UINT8 op = OP(pc++);
	UINT8 ptr = op & 3;

	if (BIT(op,7)) {
		// two bytes instructions
		char as[10];
		char aspr[10];
		UINT8 arg = ARG(pc); pc++;
		if (arg==0x80) {
			sprintf(as,"E");
		} else {
			if (arg & 0x80) {
				sprintf(as,"-$%02x",0x100-arg);
			} else {
				sprintf(as,"+$%02x",arg);
			}
		}
		sprintf(aspr,"%s(%d)",as,ptr);

		switch (op)
		{
			// Memory Reference Instructions
			case 0xc0 : sprintf (buffer,"ld %s",as); break;
			case 0xc1 : case 0xc2 : case 0xc3 :
						sprintf (buffer,"ld %s",aspr);break;
			case 0xc5 : case 0xc6 : case 0xc7 :
						sprintf (buffer,"ld @%s",aspr); break;
			case 0xc8 : sprintf (buffer,"st %s",as); break;
			case 0xc9 : case 0xca : case 0xcb :
						sprintf (buffer,"st %s",aspr);break;
			case 0xcd : case 0xce : case 0xcf :
						sprintf (buffer,"st @%s",aspr); break;
			case 0xd0 : sprintf (buffer,"and %s",as); break;
			case 0xd1 : case 0xd2 : case 0xd3 :
						sprintf (buffer,"and %s",aspr);break;
			case 0xd5 : case 0xd6 : case 0xd7 :
						sprintf (buffer,"and @%s",aspr); break;
			case 0xd8 : sprintf (buffer,"or %s",as); break;
			case 0xd9 : case 0xda : case 0xdb :
						sprintf (buffer,"or %s",aspr);break;
			case 0xdd : case 0xde : case 0xdf :
						sprintf (buffer,"or @%s",aspr); break;
			case 0xe0 : sprintf (buffer,"xor %s",as); break;
			case 0xe1 : case 0xe2 : case 0xe3 :
						sprintf (buffer,"xor %s",aspr);break;
			case 0xe5 : case 0xe6 : case 0xe7 :
						sprintf (buffer,"xor @%s",aspr); break;
			case 0xe8 : sprintf (buffer,"dad %s",as); break;
			case 0xe9 : case 0xea : case 0xeb :
						sprintf (buffer,"dad %s",aspr);break;
			case 0xed : case 0xee : case 0xef :
						sprintf (buffer,"dad @%s",aspr); break;
			case 0xf0 : sprintf (buffer,"add %s",as); break;
			case 0xf1 : case 0xf2 : case 0xf3 :
						sprintf (buffer,"add %s",aspr);break;
			case 0xf5 : case 0xf6 : case 0xf7 :
						sprintf (buffer,"add @%s",aspr); break;
			case 0xf8 : sprintf (buffer,"cad %s",as); break;
			case 0xf9 : case 0xfa : case 0xfb :
						sprintf (buffer,"cad %s",aspr);break;
			case 0xfd : case 0xfe : case 0xff :
						sprintf (buffer,"cad @%s",aspr); break;
			// Memory Increment/Decrement Instructions
			case 0xa8 : case 0xa9 : case 0xaa : case 0xab :
						sprintf (buffer,"ild %s",aspr); break;
			case 0xb8 : case 0xb9 : case 0xba : case 0xbb :
						sprintf (buffer,"dld %s",aspr); break;
			// Immediate Instructions
			case 0xc4 : sprintf (buffer,"ldi $%02x",arg); break;
			case 0xd4 : sprintf (buffer,"ani $%02x",arg); break;
			case 0xdc : sprintf (buffer,"ori $%02x",arg); break;
			case 0xe4 : sprintf (buffer,"xri $%02x",arg); break;
			case 0xec : sprintf (buffer,"dai $%02x",arg); break;
			case 0xf4 : sprintf (buffer,"adi $%02x",arg); break;
			case 0xfc : sprintf (buffer,"cai $%02x",arg); break;
			// Transfer Instructions
			case 0x90 : sprintf (buffer,"jmp %s",as);break;
			case 0x91 : case 0x92 : case 0x93 :
						sprintf (buffer,"jmp %s",aspr);break;
			case 0x94 : sprintf (buffer,"jp %s",as); break;
			case 0x95 : case 0x96 : case 0x97 :
						sprintf (buffer,"jp %s",aspr); break;
			case 0x98 : sprintf (buffer,"jz %s",as); break;
			case 0x99 : case 0x9a : case 0x9b :
						sprintf (buffer,"jz %s",aspr); break;
			case 0x9c : sprintf (buffer,"jnz %s",as); break;
			case 0x9d : case 0x9e : case 0x9f :
						sprintf (buffer,"jnz %s",aspr); break;
			// Double-Byte Miscellaneous Instructions
			case 0x8f:  sprintf (buffer,"dly $%02x",arg); break;
			// Others are illegal
			default : sprintf (buffer,"illegal"); pc--; break; // Illegal we consider without param
		}
	} else {
		// one byte instructions
		switch (op)
		{
			// Extension Register Instructions
			case 0x40:  sprintf (buffer,"lde"); break;
			case 0x01:  sprintf (buffer,"xae"); break;
			case 0x50:  sprintf (buffer,"ane"); break;
			case 0x58:  sprintf (buffer,"ore"); break;
			case 0x60:  sprintf (buffer,"xre"); break;
			case 0x68:  sprintf (buffer,"dae"); break;
			case 0x70:  sprintf (buffer,"ade"); break;
			case 0x78:  sprintf (buffer,"cae"); break;
			// Pointer Register Move Instructions
			case 0x30:  case 0x31 :case 0x32: case 0x33:
						sprintf (buffer,"xpal %d",ptr); break;
			case 0x34:  case 0x35 :case 0x36: case 0x37:
						sprintf (buffer,"xpah %d",ptr); break;
			case 0x3c:  case 0x3d :case 0x3e: case 0x3f:
						sprintf (buffer,"xppc %d",ptr); break;
			// Shift, Rotate, Serial I/O Instructions
			case 0x19:  sprintf (buffer,"sio"); break;
			case 0x1c:  sprintf (buffer,"sr"); break;
			case 0x1d:  sprintf (buffer,"srl"); break;
			case 0x1e:  sprintf (buffer,"rr"); break;
			case 0x1f:  sprintf (buffer,"rrl"); break;
			// Single Byte Miscellaneous Instructions
			case 0x00:  sprintf (buffer,"halt"); break;
			case 0x02:  sprintf (buffer,"ccl"); break;
			case 0x03:  sprintf (buffer,"scl"); break;
			case 0x04:  sprintf (buffer,"dint"); break;
			case 0x05:  sprintf (buffer,"ien"); break;
			case 0x06:  sprintf (buffer,"csa"); break;
			case 0x07:  sprintf (buffer,"cas"); break;
			case 0x08:  sprintf (buffer,"nop"); break;
			// Others are illegal
			default : sprintf (buffer,"illegal"); break;
		}
	}

	return (pc - PC);
}

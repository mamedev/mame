// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]

CPU_DISASSEMBLE( ie15 )
{
	UINT32 flags = 0;
	UINT8 op;
	unsigned PC = pc;

	op = OP(pc++);
	switch (op & 0xf0)
	{
		case 0x00:
			sprintf (buffer,"add  r%d", op & 0x0f);
			break;
		case 0x10:
			sprintf (buffer,"jmp  $%04x", (((op & 0x0f) << 8) | ARG(pc)) + 1);
			pc+=1;
			break;
		case 0x20:
			sprintf (buffer,"ldc  r%d, #$%02x", (op & 0x0f), ARG(pc));
			pc+=1;
			break;
		case 0x30: switch (op)
		{
			case 0x30:
				sprintf (buffer,"lca  #$%02x", ARG(pc));
				pc+=1;
				break;
			case 0x33:
				sprintf (buffer,"ral");
				break;
			case 0x35:
				sprintf (buffer,"rar");
				break;
			default:
				sprintf (buffer,"illegal");
				break;
		};
		break;
		case 0x40:
			sprintf (buffer,"dsr  r%d", op & 0x0f);
			break;
		case 0x50: switch (op)
		{
			case 0x50:
				sprintf (buffer,"isn");
				break;
			case 0x51:
				sprintf (buffer,"inc");
				break;
			case 0x52:
				sprintf (buffer,"dsn");
				break;
			case 0x58:
				sprintf (buffer,"ise");
				break;
			case 0x5a:
				sprintf (buffer,"dse");
				break;
			case 0x5b:
				sprintf (buffer,"dec");
				break;
			case 0x5d:
				sprintf (buffer,"com");
				break;
			case 0x5f:
				sprintf (buffer,"clr");
				break;
			default:
				sprintf (buffer,"illegal");
				break;
		};
		break;
		case 0x60:
			sprintf (buffer,"lla  #$%02x", op & 0x0f);
			break;
		case 0x70:
			sprintf (buffer,"jmi  r%d", op & 0x0f);
			break;
		case 0x80: switch (op)
		{
			case 0x80: case 0x81: case 0x82: case 0x83:
			case 0x84: case 0x85: case 0x86:
				sprintf (buffer,"sfc  #%d", op & 0x07);
				break;
			case 0x87:
				sprintf (buffer,"skp");
				break;
			case 0x88: case 0x89: case 0x8a: case 0x8b:
			case 0x8c: case 0x8d: case 0x8e:
				sprintf (buffer,"sfs  #%d", op & 0x07);
				break;
			case 0x8f:
				sprintf (buffer,"nop");
				break;
		};
		break;
		case 0x90:
			sprintf (buffer,"and  r%d", op & 0x0f);
			break;
		case 0xa0:
			sprintf (buffer,"xor  r%d", op & 0x0f);
			break;
		case 0xb0:
			sprintf (buffer,"cs   r%d", op & 0x0f);
			break;
		case 0xc0:
			sprintf (buffer,"%s  #%d", BIT(op, 3) ? "sfl" : "cfl", op & 0x07);
			break;
		case 0xd0:
			sprintf (buffer,"lda  r%d", op & 0x0f);
			break;
		case 0xe0:
			sprintf (buffer,"sta  r%d", op & 0x0f);
			break;
		case 0xf0:
			sprintf (buffer,"ota  #$%02x", op & 0x0f);
			break;
	}
	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}

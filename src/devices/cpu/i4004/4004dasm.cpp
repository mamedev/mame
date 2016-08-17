// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   4004dasm.c
 *
 *   Intel 4004 CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]

CPU_DISASSEMBLE( i4004 )
{
	UINT32 flags = 0;
	UINT8 op;
	unsigned PC = pc;
	UINT16 page = PC & 0x0f00;
	switch (op = OP(pc++))
	{
		case 0x00:  sprintf (buffer,"nop");                                 break;
		case 0x11:  sprintf (buffer,"jnt $%03x",page | ARG(pc));    pc++;   break;
		case 0x12:  sprintf (buffer,"jc $%03x",page | ARG(pc));     pc++;   break;
		case 0x14:  sprintf (buffer,"jz $%03x",page | ARG(pc));     pc++;   break;
		case 0x19:  sprintf (buffer,"jt $%03x",page | ARG(pc));     pc++;   break;
		case 0x1a:  sprintf (buffer,"jnc $%03x",page | ARG(pc));    pc++;   break;
		case 0x1c:  sprintf (buffer,"jnz $%03x",page | ARG(pc));    pc++;   break;
		case 0x10: case 0x13: case 0x15: case 0x16:
		case 0x17: case 0x18: case 0x1b: case 0x1d:
		case 0x1e: case 0x1f:
					sprintf (buffer,"jcn $%01x,$%03x",op & 0x0f,page | ARG(pc)); pc++; break;
		case 0x20: case 0x22: case 0x24: case 0x26:
		case 0x28: case 0x2a: case 0x2c: case 0x2e:
					sprintf (buffer,"fim $%01x,$%02x",op & 0x0f,OP(pc)); pc++; break;
		case 0x21: case 0x23: case 0x25: case 0x27:
		case 0x29: case 0x2b: case 0x2d: case 0x2f:
					sprintf (buffer,"src $%01x",(op & 0x0f)-1); break;
		case 0x30: case 0x32: case 0x34: case 0x36:
		case 0x38: case 0x3a: case 0x3c: case 0x3e:
					sprintf (buffer,"fin $%01x",op & 0x0f); break;
		case 0x31: case 0x33: case 0x35: case 0x37:
		case 0x39: case 0x3b: case 0x3d: case 0x3f:
					sprintf (buffer,"jin $%01x",(op & 0x0f)-1); break;
		case 0x40: case 0x41: case 0x42: case 0x43:
		case 0x44: case 0x45: case 0x46: case 0x47:
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
					sprintf (buffer,"jun $%01x%02x",op & 0x0f,ARG(pc)); pc++; break;
		case 0x50: case 0x51: case 0x52: case 0x53:
		case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5a: case 0x5b:
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
					sprintf (buffer,"jms $%01x%02x",op & 0x0f,ARG(pc)); pc++; break;
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
					sprintf (buffer,"inc $%01x",op & 0x0f); break;
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
					sprintf (buffer,"isz $%01x,%03x",op & 0x0f,page | ARG(pc)); pc++; break;
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
					sprintf (buffer,"add $%01x",op & 0x0f); break;
		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
					sprintf (buffer,"sub $%01x",op & 0x0f); break;
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
					sprintf (buffer,"ld $%01x",op & 0x0f); break;
		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
					sprintf (buffer,"xch $%01x",op & 0x0f); break;
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
					sprintf (buffer,"bbl $%01x",op & 0x0f); break;
		case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		case 0xd8: case 0xd9: case 0xda: case 0xdb:
		case 0xdc: case 0xdd: case 0xde: case 0xdf:
					sprintf (buffer,"ldm $%01x",op & 0x0f); break;

		case 0xe0:  sprintf (buffer,"wrm"); break;
		case 0xe1:  sprintf (buffer,"wmp"); break;
		case 0xe2:  sprintf (buffer,"wrr"); break;
		case 0xe3:  sprintf (buffer,"wpm"); break;
		case 0xe4:  sprintf (buffer,"wr0"); break;
		case 0xe5:  sprintf (buffer,"wr1"); break;
		case 0xe6:  sprintf (buffer,"wr2"); break;
		case 0xe7:  sprintf (buffer,"wr3"); break;
		case 0xe8:  sprintf (buffer,"sbm"); break;
		case 0xe9:  sprintf (buffer,"rdm"); break;
		case 0xea:  sprintf (buffer,"rdr"); break;
		case 0xeb:  sprintf (buffer,"adm"); break;
		case 0xec:  sprintf (buffer,"rd0"); break;
		case 0xed:  sprintf (buffer,"rd1"); break;
		case 0xee:  sprintf (buffer,"rd2"); break;
		case 0xef:  sprintf (buffer,"rd3"); break;

		case 0xf0:  sprintf (buffer,"clb"); break;
		case 0xf1:  sprintf (buffer,"clc"); break;
		case 0xf2:  sprintf (buffer,"iac"); break;
		case 0xf3:  sprintf (buffer,"cmc"); break;
		case 0xf4:  sprintf (buffer,"cma"); break;
		case 0xf5:  sprintf (buffer,"ral"); break;
		case 0xf6:  sprintf (buffer,"rar"); break;
		case 0xf7:  sprintf (buffer,"tcc"); break;
		case 0xf8:  sprintf (buffer,"dac"); break;
		case 0xf9:  sprintf (buffer,"tcs"); break;
		case 0xfa:  sprintf (buffer,"stc"); break;
		case 0xfb:  sprintf (buffer,"daa"); break;
		case 0xfc:  sprintf (buffer,"kbp"); break;
		case 0xfd:  sprintf (buffer,"dcl"); break;

		default : sprintf (buffer,"illegal"); break;
	}
	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 *   pps4dasm.c
 *
 *   Rockwell PPS-4 CPU Disassembly
 *
 *****************************************************************************/

#include "emu.h"

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]

CPU_DISASSEMBLE( pps4 )
{
	UINT32 flags = 0;
	unsigned PC = pc;
	UINT8 op;
	switch (op = OP(pc++))
	{
		// Arithmetic instructions
		case 0x0b:  sprintf (buffer,"ad");  break;
		case 0x0a:  sprintf (buffer,"adc"); break;
		case 0x09:  sprintf (buffer,"adsk");    break;
		case 0x08:  sprintf (buffer,"adcsk");   break;
		case 0x60:  case 0x61:  case 0x62:  case 0x63:
		case 0x64:  case 0x66:  case 0x67:  case 0x68:
		case 0x69:  case 0x6a:  case 0x6b:  case 0x6c:
		case 0x6d:  case 0x6e:
					sprintf (buffer,"adi %01x",(op & 0x0f));    break;
		case 0x65:  sprintf (buffer,"dc");  break;
		// Logical instructions
		case 0x0d:  sprintf (buffer,"and"); break;
		case 0x0f:  sprintf (buffer,"or");  break;
		case 0x0c:  sprintf (buffer,"eor"); break;
		case 0x0e:  sprintf (buffer,"comp");    break;
		// Data transfer instructions
		case 0x20:  sprintf (buffer,"sc");  break;
		case 0x24:  sprintf (buffer,"rc");  break;
		case 0x22:  sprintf (buffer,"sf1"); break;
		case 0x26:  sprintf (buffer,"rf1"); break;
		case 0x21:  sprintf (buffer,"sf2"); break;
		case 0x25:  sprintf (buffer,"rf2"); break;
		case 0x30:  case 0x31:  case 0x32:  case 0x33:
		case 0x34:  case 0x35:  case 0x36:  case 0x37:
					sprintf (buffer,"ld %01x",(op & 0x07)); break;
		case 0x38:  case 0x39:  case 0x3a:  case 0x3b:
		case 0x3c:  case 0x3d:  case 0x3e:  case 0x3f:
					sprintf (buffer,"ex %01x",(op & 0x07)); break;
		case 0x28:  case 0x29:  case 0x2a:  case 0x2b:
		case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:
					sprintf (buffer,"exd %01x",(op & 0x07));    break;
		case 0x70:  case 0x71:  case 0x72:  case 0x73:
		case 0x74:  case 0x75:  case 0x76:  case 0x77:
					sprintf (buffer,"ldi %01x",(op & 0x0f));    break;
		case 0x12:  sprintf (buffer,"lax"); break;
		case 0x1b:  sprintf (buffer,"lxa"); break;
		case 0x11:  sprintf (buffer,"labl");    break;
		case 0x10:  sprintf (buffer,"lbmx");    break;
		case 0x04:  sprintf (buffer,"lbua");    break;
		case 0x19:  sprintf (buffer,"xabl");    break;
		case 0x18:  sprintf (buffer,"xbmx");    break;
		case 0x1a:  sprintf (buffer,"xax"); break;
		case 0x06:  sprintf (buffer,"xs");  break;
		case 0x6f:  sprintf (buffer,"cys"); break;
		case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:
		case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
		case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
		case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
					sprintf (buffer,"lb %02x",ARG(pc)); pc++;   break;
		case 0x00:  sprintf (buffer,"lbl %02x",ARG(pc)); pc++;  break;
		case 0x17:  sprintf (buffer,"incb");    break;
		case 0x1f:  sprintf (buffer,"decb");    break;
		// Control transfer instructions
		case 0x80:  case 0x81:  case 0x82:  case 0x83:
		case 0x84:  case 0x85:  case 0x86:  case 0x87:
		case 0x88:  case 0x89:  case 0x8a:  case 0x8b:
		case 0x8c:  case 0x8d:  case 0x8e:  case 0x8f:
		case 0x90:  case 0x91:  case 0x92:  case 0x93:
		case 0x94:  case 0x95:  case 0x96:  case 0x97:
		case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
		case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:
		case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:
		case 0xa4:  case 0xa5:  case 0xa6:  case 0xa7:
		case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:
		case 0xac:  case 0xad:  case 0xae:  case 0xaf:
		case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:
		case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
		case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:
		case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
					sprintf (buffer,"t %02x",(op & 0x3f));  break;
		case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:
		case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
		case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:
		case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
		case 0xe0:  case 0xe1:  case 0xe2:  case 0xe3:
		case 0xe4:  case 0xe5:  case 0xe6:  case 0xe7:
		case 0xe8:  case 0xe9:  case 0xea:  case 0xeb:
		case 0xec:  case 0xed:  case 0xee:  case 0xef:
		case 0xf0:  case 0xf1:  case 0xf2:  case 0xf3:
		case 0xf4:  case 0xf5:  case 0xf6:  case 0xf7:
		case 0xf8:  case 0xf9:  case 0xfa:  case 0xfb:
		case 0xfc:  case 0xfd:  case 0xfe:  case 0xff:
					sprintf (buffer,"tm %02x %02x",(op & 0x3f),ARG(pc)); pc++;  break;
		case 0x50:  case 0x51:  case 0x52:  case 0x53:
		case 0x54:  case 0x55:  case 0x56:  case 0x57:
		case 0x58:  case 0x59:  case 0x5a:  case 0x5b:
		case 0x5c:  case 0x5d:  case 0x5e:  case 0x5f:
					sprintf (buffer,"tl %01x %02x",(op & 0x0f),ARG(pc)); pc++;  break;
		case 0x01:  case 0x02:  case 0x03:
					sprintf (buffer,"tml %02x",ARG(pc)); pc++;  break;
		case 0x15:  sprintf (buffer,"skc"); break;
		case 0x1e:  sprintf (buffer,"skz"); break;
		case 0x40:  case 0x41:  case 0x42:  case 0x43:
		case 0x44:  case 0x45:  case 0x46:  case 0x47:
		case 0x48:  case 0x49:  case 0x4a:  case 0x4b:
		case 0x4c:  case 0x4d:  case 0x4e:  case 0x4f:
					sprintf (buffer,"skbi %01x",(op & 0x0f));   break;
		case 0x16:  sprintf (buffer,"skf1");    break;
		case 0x14:  sprintf (buffer,"skf2");    break;
		case 0x05:  sprintf (buffer,"rtn"); break;
		case 0x07:  sprintf (buffer,"rtnsk");   break;
		// Input/Output instructions
		case 0x1c:  sprintf (buffer,"iol %02x",ARG(pc)); pc++;  break;
		case 0x27:  sprintf (buffer,"dia"); break;
		case 0x23:  sprintf (buffer,"dib"); break;
		case 0x1d:  sprintf (buffer,"doa"); break;
		// Special instructions
		case 0x13:  sprintf (buffer,"sag"); break;
	}

	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}

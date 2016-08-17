// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    ccpudasm.c
    Core implementation for the portable Cinematronics CPU disassembler.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#include "emu.h"
#include "ccpu.h"


CPU_DISASSEMBLE( ccpu )
{
	unsigned startpc = pc;
	UINT8 opcode = oprom[pc++ - startpc];
	UINT8 tempval;

	switch (opcode)
	{
		/* LDAI */
		case 0x00:  case 0x01:  case 0x02:  case 0x03:
		case 0x04:  case 0x05:  case 0x06:  case 0x07:
		case 0x08:  case 0x09:  case 0x0a:  case 0x0b:
		case 0x0c:  case 0x0d:  case 0x0e:  case 0x0f:
			sprintf(buffer, "LDAI $%X", opcode & 0x0f);
			break;

		/* INP */
		case 0x10:  case 0x11:  case 0x12:  case 0x13:
		case 0x14:  case 0x15:  case 0x16:  case 0x17:
		case 0x18:  case 0x19:  case 0x1a:  case 0x1b:
		case 0x1c:  case 0x1d:  case 0x1e:  case 0x1f:
			sprintf(buffer, "INP  $%X", opcode & 0x0f);
			break;

		/* A8I */
		case 0x20:
			sprintf(buffer, "A8I  $%X", oprom[pc++ - startpc]);
			break;

		/* A4I */
		case 0x21:  case 0x22:  case 0x23:
		case 0x24:  case 0x25:  case 0x26:  case 0x27:
		case 0x28:  case 0x29:  case 0x2a:  case 0x2b:
		case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:
			sprintf(buffer, "A4I  $%X", opcode & 0x0f);
			break;

		/* S8I */
		case 0x30:
			sprintf(buffer, "S8I  $%X", oprom[pc++ - startpc]);
			break;

		/* S4I */
		case 0x31:  case 0x32:  case 0x33:
		case 0x34:  case 0x35:  case 0x36:  case 0x37:
		case 0x38:  case 0x39:  case 0x3a:  case 0x3b:
		case 0x3c:  case 0x3d:  case 0x3e:  case 0x3f:
			sprintf(buffer, "S4I  $%X", opcode & 0x0f);
			break;

		/* LPAI */
		case 0x40:  case 0x41:  case 0x42:  case 0x43:
		case 0x44:  case 0x45:  case 0x46:  case 0x47:
		case 0x48:  case 0x49:  case 0x4a:  case 0x4b:
		case 0x4c:  case 0x4d:  case 0x4e:  case 0x4f:
			tempval = oprom[pc++ - startpc];
			sprintf(buffer, "LPAI $%03X", (opcode & 0x0f) + (tempval & 0xf0) + ((tempval & 0x0f) << 8));
			break;

		/* T4K */
		case 0x50:
			sprintf(buffer, "T4K");
			break;

		/* JMIB/JEHB */
		case 0x51:
			sprintf(buffer, "JMIB/JEHB");
			break;

		/* JVNB */
		case 0x52:
			sprintf(buffer, "JVNB");
			break;

		/* JLTB */
		case 0x53:
			sprintf(buffer, "JLTB");
			break;

		/* JEQB */
		case 0x54:
			sprintf(buffer, "JEQB");
			break;

		/* JCZB */
		case 0x55:
			sprintf(buffer, "JCZB");
			break;

		/* JOSB */
		case 0x56:
			sprintf(buffer, "JOSB");
			break;

		/* SSA */
		case 0x57:
			sprintf(buffer, "SSA");
			break;

		/* JMP */
		case 0x58:
			sprintf(buffer, "JMP");
			break;

		/* JMI/JEH */
		case 0x59:
			sprintf(buffer, "JMI/JEH");
			break;

		/* JVN */
		case 0x5a:
			sprintf(buffer, "JVN");
			break;

		/* JLT */
		case 0x5b:
			sprintf(buffer, "JLT");
			break;

		/* JEQ */
		case 0x5c:
			sprintf(buffer, "JEQ");
			break;

		/* JCZ */
		case 0x5d:
			sprintf(buffer, "JCZ");
			break;

		/* JOS */
		case 0x5e:
			sprintf(buffer, "JOS");
			break;

		/* NOP */
		case 0x5f:
			sprintf(buffer, "NOP");
			break;

		/* ADD */
		case 0x60:  case 0x61:  case 0x62:  case 0x63:
		case 0x64:  case 0x65:  case 0x66:  case 0x67:
		case 0x68:  case 0x69:  case 0x6a:  case 0x6b:
		case 0x6c:  case 0x6d:  case 0x6e:  case 0x6f:
			sprintf(buffer, "ADD  $%X", opcode & 0x0f);
			break;

		/* SUB n */
		case 0x70:  case 0x71:  case 0x72:  case 0x73:
		case 0x74:  case 0x75:  case 0x76:  case 0x77:
		case 0x78:  case 0x79:  case 0x7a:  case 0x7b:
		case 0x7c:  case 0x7d:  case 0x7e:  case 0x7f:
			sprintf(buffer, "SUB  $%X", opcode & 0x0f);
			break;

		/* SETP n */
		case 0x80:  case 0x81:  case 0x82:  case 0x83:
		case 0x84:  case 0x85:  case 0x86:  case 0x87:
		case 0x88:  case 0x89:  case 0x8a:  case 0x8b:
		case 0x8c:  case 0x8d:  case 0x8e:  case 0x8f:
			sprintf(buffer, "SETP $%X", opcode & 0x0f);
			break;

		/* OUT */
		case 0x90:  case 0x91:  case 0x92:  case 0x93:
		case 0x94:  case 0x95:  case 0x96:  case 0x97:
		case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
		case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:
			sprintf(buffer, "OUT  $%X", opcode & 0x0f);
			break;

		/* LDA */
		case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:
		case 0xa4:  case 0xa5:  case 0xa6:  case 0xa7:
		case 0xa8:  case 0xa9:  case 0xaa:  case 0xab:
		case 0xac:  case 0xad:  case 0xae:  case 0xaf:
			sprintf(buffer, "LDA  $%X", opcode & 0x0f);
			break;

		/* CMP */
		case 0xb0:  case 0xb1:  case 0xb2:  case 0xb3:
		case 0xb4:  case 0xb5:  case 0xb6:  case 0xb7:
		case 0xb8:  case 0xb9:  case 0xba:  case 0xbb:
		case 0xbc:  case 0xbd:  case 0xbe:  case 0xbf:
			sprintf(buffer, "TST  $%X", opcode & 0x0f);
			break;

		/* WS */
		case 0xc0:  case 0xc1:  case 0xc2:  case 0xc3:
		case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
		case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
		case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:
			sprintf(buffer, "WS   $%X", opcode & 0x0f);
			break;

		/* STA n */
		case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:
		case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
		case 0xd8:  case 0xd9:  case 0xda:  case 0xdb:
		case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:
			sprintf(buffer, "STA  $%X", opcode & 0x0f);
			break;

		/* DV */
		case 0xe0:
			sprintf(buffer, "DV");
			break;

		/* LPAP */
		case 0xe1:
			sprintf(buffer, "LPAP");
			break;

		/* WSP */
		case 0xf1:
			sprintf(buffer, "WSP");
			break;

		/* LKP */
		case 0xe2:
		case 0xf2:
			sprintf(buffer, "LKP");
			break;

		/* MUL */
		case 0xe3:
		case 0xf3:
			sprintf(buffer, "MUL");
			break;

		/* NV */
		case 0xe4:
		case 0xf4:
			sprintf(buffer, "NV");
			break;

		/* FRM */
		case 0xe5:
		case 0xf5:
			sprintf(buffer, "FRM");
			break;

		/* STAP */
		case 0xe6:
		case 0xf6:
			sprintf(buffer, "STAP");
			break;

		/* CST */
		case 0xf7:
			sprintf(buffer, "CST");
			break;

		/* ADDP */
		case 0xe7:
			sprintf(buffer, "ADDP");
			break;

		/* SUBP */
		case 0xe8:
		case 0xf8:
			sprintf(buffer, "SUBP");
			break;

		/* ANDP */
		case 0xe9:
		case 0xf9:
			sprintf(buffer, "ANDP");
			break;

		/* LDAP */
		case 0xea:
		case 0xfa:
			sprintf(buffer, "LDAP");
			break;

		/* SHR */
		case 0xeb:
		case 0xfb:
			sprintf(buffer, "SHR");
			break;

		/* SHL */
		case 0xec:
		case 0xfc:
			sprintf(buffer, "SHL");
			break;

		/* ASR */
		case 0xed:
		case 0xfd:
			sprintf(buffer, "ASR");
			break;

		/* SHRB */
		case 0xee:
		case 0xfe:
			sprintf(buffer, "SHRB");
			break;

		/* SHLB */
		case 0xef:
		case 0xff:
			sprintf(buffer, "SHLB");
			break;

		/* IV */
		case 0xf0:
			sprintf(buffer, "IV");
			break;
	}

	return (pc - startpc) | DASMFLAG_SUPPORTED;
}

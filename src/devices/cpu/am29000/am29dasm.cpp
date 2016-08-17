// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    am29dasm.c
    Disassembler for the portable Am29000 emulator.
    Written by Phil Bennett

***************************************************************************/

#include "emu.h"
#include "am29000.h"


/***************************************************************************
    DEFINES AND MACROS
***************************************************************************/

#define OP_M_BIT    (1 << 24)
#define OP_RC       ((op >> 16) & 0xff)
#define OP_RA       ((op >> 8) & 0xff)
#define OP_RB       (op & 0xff)
#define OP_I8       (op & 0xff)
#define OP_SA       ((op >> 8) & 0xff)
#define OP_I16      (((op >> 8) & 0xff00) | (op & 0xff))
#define OP_IJMP     (OP_I16 << 2)
#define OP_VN       ((op >> 16) & 0xff)

#define OP_CE       ((op >> 23) & 1)
#define OP_CNTL     ((op >> 16) & 0x7f)

#define OP_SJMP     ((INT32)(INT16)OP_I16 << 2)


/***************************************************************************
    CODE
***************************************************************************/

static const char *dasm_type1(UINT32 op)
{
	static char buf[32];

	if (op & OP_M_BIT)
		sprintf(buf, "r%d, r%d, $%.2x", OP_RC, OP_RA, OP_I8);
	else
		sprintf(buf, "r%d, r%d, r%d", OP_RC, OP_RA, OP_RB);

	return buf;
}

static const char *dasm_type2(UINT32 op)
{
	static char buf[32];

	sprintf(buf, "r%d, r%d, r%d", OP_RC, OP_RA, OP_RB);

	return buf;
}

static const char *dasm_type3(UINT32 op)
{
	static char buf[32];

	sprintf(buf, "r%d, $%.4x", OP_RA, OP_I16);

	return buf;
}

static const char *dasm_type4(UINT32 op, UINT32 pc)
{
	static char buf[32];

	if (op & OP_M_BIT)
		sprintf(buf, "r%d, $%.4x", OP_RA, OP_IJMP);
	else
		sprintf(buf, "r%d, $%.4x", OP_RA, pc + OP_SJMP);

	return buf;
}

static const char *dasm_type5(UINT32 op)
{
	static char buf[32];

	if (op & OP_M_BIT)
		sprintf(buf, "trap%d, r%d, $%.2x", OP_VN, OP_RA, OP_I8);
	else
		sprintf(buf, "trap%d, r%d, r%d", OP_VN, OP_RA, OP_RB);

	return buf;
}

static const char *dasm_type6(UINT32 op)
{
	static char buf[32];

	if (op & OP_M_BIT)
		sprintf(buf, "%d, %x, r%d, $%.2x", OP_CE, OP_CNTL, OP_RA, OP_I8);
	else
		sprintf(buf, "%d, %x, r%d, r%d", OP_CE, OP_CNTL, OP_RA, OP_RB);

	return buf;
}

#define TYPE_1      dasm_type1(op)
#define TYPE_2      dasm_type2(op)
#define TYPE_3      dasm_type3(op)
#define TYPE_4      dasm_type4(op, pc)
#define TYPE_5      dasm_type5(op)
#define TYPE_6      dasm_type6(op)


static const char* get_spr(int spid)
{
	switch (spid)
	{
		case   0: return "VAB";
		case   1: return "OPS";
		case   2: return "CPS";
		case   3: return "CFG";
		case   4: return "CHA";
		case   5: return "CHD";
		case   6: return "CHC";
		case   7: return "RBP";
		case   8: return "TMC";
		case   9: return "TMR";
		case  10: return "PC0";
		case  11: return "PC1";
		case  12: return "PC2";
		case  13: return "MMU";
		case  14: return "LRU";
		case 128: return "IPC";
		case 129: return "IPA";
		case 130: return "IPB";
		case 131: return "Q";
		case 132: return "ALU";
		case 133: return "BP";
		case 134: return "FC";
		case 135: return "CR";
		case 160: return "FPE";
		case 161: return "INTE";
		case 162: return "FPS";
		case 164: return "EXOP";
		default:  return "????";
	}
}

CPU_DISASSEMBLE( am29000 )
{
	UINT32 op = (oprom[0] << 24) | (oprom[1] << 16) | (oprom[2] << 8) | oprom[3];
	UINT32 flags = 0;

	switch (op >> 24)
	{
		case 0x01:              sprintf(buffer, "constn  %s", TYPE_3);                          break;
		case 0x02:              sprintf(buffer, "consth  %s", TYPE_3);                          break;
		case 0x03:              sprintf(buffer, "const   %s", TYPE_3);                          break;
		case 0x04:              sprintf(buffer, "mtsrim  %s, $%.4x", get_spr(OP_SA), OP_I16);   break;
		case 0x06: case 0x07:   sprintf(buffer, "loadl   %s", TYPE_6);                          break;
		case 0x08:              sprintf(buffer, "clz     r%d, %d", OP_RC, OP_RB);               break;
		case 0x09:              sprintf(buffer, "clz     %d, %.2x", OP_RC, OP_I8);              break;
		case 0x0a: case 0x0b:   sprintf(buffer, "exbyte  %s", TYPE_1);                          break;
		case 0x0c: case 0x0d:   sprintf(buffer, "inbyte  %s", TYPE_1);                          break;
		case 0x0e: case 0x0f:   sprintf(buffer, "storel  %s", TYPE_6);                          break;
		case 0x10: case 0x11:   sprintf(buffer, "adds    %s", TYPE_1);                          break;
		case 0x12: case 0x13:   sprintf(buffer, "addu    %s", TYPE_1);                          break;
		case 0x14: case 0x15:   sprintf(buffer, "add     %s", TYPE_1);                          break;
		case 0x16: case 0x17:   sprintf(buffer, "load    %s", TYPE_6);                          break;
		case 0x18: case 0x19:   sprintf(buffer, "addcs   %s", TYPE_1);                          break;
		case 0x1a: case 0x1b:   sprintf(buffer, "addcu   %s", TYPE_1);                          break;
		case 0x1c: case 0x1d:   sprintf(buffer, "addc    %s", TYPE_1);                          break;
		case 0x1e: case 0x1f:   sprintf(buffer, "store   %s", TYPE_6);                          break;
		case 0x20: case 0x21:   sprintf(buffer, "subs    %s", TYPE_1);                          break;
		case 0x22: case 0x23:   sprintf(buffer, "subu    %s", TYPE_1);                          break;
		case 0x24: case 0x25:   sprintf(buffer, "sub     %s", TYPE_1);                          break;
		case 0x26: case 0x27:   sprintf(buffer, "loadset %s", TYPE_6);                          break;
		case 0x28: case 0x29:   sprintf(buffer, "subcs   %s", TYPE_1);                          break;
		case 0x2a: case 0x2b:   sprintf(buffer, "subcu   %s", TYPE_1);                          break;
		case 0x2c: case 0x2d:   sprintf(buffer, "subc    %s", TYPE_1);                          break;
		case 0x2e: case 0x2f:   sprintf(buffer, "cpbyte  %s", TYPE_1);                          break;
		case 0x30: case 0x31:   sprintf(buffer, "subrs   %s", TYPE_1);                          break;
		case 0x32: case 0x33:   sprintf(buffer, "subru   %s", TYPE_1);                          break;
		case 0x34: case 0x35:   sprintf(buffer, "subr    %s", TYPE_1);                          break;
		case 0x36: case 0x37:   sprintf(buffer, "loadm   %s", TYPE_6);                          break;
		case 0x38: case 0x39:   sprintf(buffer, "subrcs  %s", TYPE_1);                          break;
		case 0x3a: case 0x3b:   sprintf(buffer, "subrcu  %s", TYPE_1);                          break;
		case 0x3c: case 0x3d:   sprintf(buffer, "subrc   %s", TYPE_1);                          break;
		case 0x3e: case 0x3f:   sprintf(buffer, "storem  %s", TYPE_6);                          break;
		case 0x40: case 0x41:   sprintf(buffer, "cplt    %s", TYPE_1);                          break;
		case 0x42: case 0x43:   sprintf(buffer, "cpltu   %s", TYPE_1);                          break;
		case 0x44: case 0x45:   sprintf(buffer, "cple    %s", TYPE_1);                          break;
		case 0x46: case 0x47:   sprintf(buffer, "cpleu   %s", TYPE_1);                          break;
		case 0x48: case 0x49:   sprintf(buffer, "cpgt    %s", TYPE_1);                          break;
		case 0x4a: case 0x4b:   sprintf(buffer, "cpgtu   %s", TYPE_1);                          break;
		case 0x4c: case 0x4d:   sprintf(buffer, "cpge    %s", TYPE_1);                          break;
		case 0x4e: case 0x4f:   sprintf(buffer, "cpgeu   %s", TYPE_1);                          break;
		case 0x50: case 0x51:   sprintf(buffer, "aslt    %s", TYPE_5);                          break;
		case 0x52: case 0x53:   sprintf(buffer, "asltu   %s", TYPE_5);                          break;
		case 0x54: case 0x55:   sprintf(buffer, "asle    %s", TYPE_5);                          break;
		case 0x56: case 0x57:   sprintf(buffer, "asleu   %s", TYPE_5);                          break;
		case 0x58: case 0x59:   sprintf(buffer, "asgt    %s", TYPE_5);                          break;
		case 0x5a: case 0x5b:   sprintf(buffer, "asgtu   %s", TYPE_5);                          break;
		case 0x5c: case 0x5d:   sprintf(buffer, "asge    %s", TYPE_5);                          break;
		case 0x5e: case 0x5f:   sprintf(buffer, "asgeu   %s", TYPE_5);                          break;
		case 0x60: case 0x61:   sprintf(buffer, "cpeq    %s", TYPE_1);                          break;
		case 0x62: case 0x63:   sprintf(buffer, "cpneq   %s", TYPE_1);                          break;
		case 0x64: case 0x65:   sprintf(buffer, "mul     %s", TYPE_1);                          break;
		case 0x66: case 0x67:   sprintf(buffer, "mull    %s", TYPE_1);                          break;
		case 0x68:              sprintf(buffer, "div0    r%d, r%d", OP_RC, OP_RB);              break;
		case 0x69:              sprintf(buffer, "div0    r%d, %.2x", OP_RC, OP_I8);             break;
		case 0x6a: case 0x6b:   sprintf(buffer, "div     %s", TYPE_1);                          break;
		case 0x6c: case 0x6d:   sprintf(buffer, "divl    %s", TYPE_1);                          break;
		case 0x6e: case 0x6f:   sprintf(buffer, "divrem  %s", TYPE_1);                          break;
		case 0x70: case 0x71:   sprintf(buffer, "aseq    %s", TYPE_5);                          break;
		case 0x72: case 0x73:   sprintf(buffer, "asneq   %s", TYPE_5);                          break;
		case 0x74: case 0x75:   sprintf(buffer, "mulu    %s", TYPE_1);                          break;
		case 0x78: case 0x79:   sprintf(buffer, "inhw    %s", TYPE_1);                          break;
		case 0x7a: case 0x7b:   sprintf(buffer, "extract %s", TYPE_1);                          break;
		case 0x7c: case 0x7d:   sprintf(buffer, "exhw    %s", TYPE_1);                          break;
		case 0x7e:              sprintf(buffer, "exhws   %s", TYPE_1);                          break;
		case 0x80: case 0x81:   sprintf(buffer, "sll     %s", TYPE_1);                          break;
		case 0x82: case 0x83:   sprintf(buffer, "srl     %s", TYPE_1);                          break;
		case 0x86: case 0x87:   sprintf(buffer, "sra     %s", TYPE_1);                          break;
		case 0x88:              sprintf(buffer, "iret");                                        break;
		case 0x89:              sprintf(buffer, "halt");                                        break;
		case 0x8c:              sprintf(buffer, "iretinv");                                     break;
		case 0x90: case 0x91:   sprintf(buffer, "and     %s", TYPE_1);                          break;
		case 0x92: case 0x93:   sprintf(buffer, "or      %s", TYPE_1);                          break;
		case 0x94: case 0x95:   sprintf(buffer, "xor     %s", TYPE_1);                          break;
		case 0x96: case 0x97:   sprintf(buffer, "xnor    %s", TYPE_1);                          break;
		case 0x98: case 0x99:   sprintf(buffer, "nor     %s", TYPE_1);                          break;
		case 0x9a: case 0x9b:   sprintf(buffer, "nand    %s", TYPE_1);                          break;
		case 0x9c: case 0x9d:   sprintf(buffer, "andn    %s", TYPE_1);                          break;
		case 0x9e:              sprintf(buffer, "setip   %s", TYPE_2);                          break;
		case 0x9f:              sprintf(buffer, "inv");                                         break;
		case 0xa0:              sprintf(buffer, "jmp     $%.4x", pc + OP_SJMP);                 break;
		case 0xa1:              sprintf(buffer, "jmp     $%.4x", OP_IJMP);                      break;
		case 0xa4: case 0xa5:   sprintf(buffer, "jmpf    %s", TYPE_4);                          break;
		case 0xa8: case 0xa9:   sprintf(buffer, "call    %s", TYPE_4);                          break;
		case 0xac: case 0xad:   sprintf(buffer, "jmpt    %s", TYPE_4);                          break;
		case 0xb4: case 0xb5:   sprintf(buffer, "jmpfdec %s", TYPE_4);                          break;
		case 0xb6:              sprintf(buffer, "mftlb   r%d, r%d", OP_RC, OP_RA);              break;
		case 0xbe:              sprintf(buffer, "mttlb   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xc0:              sprintf(buffer, "jmpi    r%d", OP_RB);                          break;
		case 0xc4:              sprintf(buffer, "jmpfi   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xc6:              sprintf(buffer, "mfsr    r%d, %s", OP_RC, get_spr(OP_SA));      break;
		case 0xc8:              sprintf(buffer, "calli   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xcc:              sprintf(buffer, "jmpti   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xce:              sprintf(buffer, "mtsr    %s, r%d", get_spr(OP_SA), OP_RB);      break;
		case 0xd7:              sprintf(buffer, "emulate %s", TYPE_5);                          break;
		case 0xde:              sprintf(buffer, "multm   %s", TYPE_2);                          break;
		case 0xdf:              sprintf(buffer, "multmu  %s", TYPE_2);                          break;

		case 0xe0:              sprintf(buffer, "multiply  %s", TYPE_2);                        break;
		case 0xe1:              sprintf(buffer, "divide  %s", TYPE_2);                          break;
		default:                sprintf(buffer, "??????");                                      break;
	}
	return 4 | flags | DASMFLAG_SUPPORTED;
}

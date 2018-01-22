// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    am29dasm.c
    Disassembler for the portable Am29000 emulator.
    Written by Phil Bennett

***************************************************************************/

#include "emu.h"
#include "am29dasm.h"


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

#define OP_SJMP     ((int32_t)(int16_t)OP_I16 << 2)


/***************************************************************************
    CODE
***************************************************************************/

std::string am29000_disassembler::dasm_type1(uint32_t op)
{
	return (op & OP_M_BIT)
		? string_format("r%d, r%d, $%02x", OP_RC, OP_RA, OP_I8)
		: string_format("r%d, r%d, r%d", OP_RC, OP_RA, OP_RB);
}

std::string am29000_disassembler::dasm_type2(uint32_t op)
{
	return string_format("r%d, r%d, r%d", OP_RC, OP_RA, OP_RB);
}

std::string am29000_disassembler::dasm_type3(uint32_t op)
{
	return string_format("r%d, $%04x", OP_RA, OP_I16);
}

std::string am29000_disassembler::dasm_type4(uint32_t op, uint32_t pc)
{
	return (op & OP_M_BIT)
		? string_format("r%d, $%04x", OP_RA, OP_IJMP)
		: string_format("r%d, $%04x", OP_RA, pc + OP_SJMP);
}

std::string am29000_disassembler::dasm_type5(uint32_t op)
{
	return (op & OP_M_BIT)
		? string_format("trap%d, r%d, $%02x", OP_VN, OP_RA, OP_I8)
		: string_format("trap%d, r%d, r%d", OP_VN, OP_RA, OP_RB);
}

std::string am29000_disassembler::dasm_type6(uint32_t op)
{
	return (op & OP_M_BIT)
		? string_format("%d, %x, r%d, $%02x", OP_CE, OP_CNTL, OP_RA, OP_I8)
		: string_format("%d, %x, r%d, r%d", OP_CE, OP_CNTL, OP_RA, OP_RB);
}

#define TYPE_1      dasm_type1(op)
#define TYPE_2      dasm_type2(op)
#define TYPE_3      dasm_type3(op)
#define TYPE_4      dasm_type4(op, pc)
#define TYPE_5      dasm_type5(op)
#define TYPE_6      dasm_type6(op)


const char* am29000_disassembler::get_spr(int spid)
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

u32 am29000_disassembler::opcode_alignment() const
{
	return 4;
}

offs_t am29000_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t op = opcodes.r32(pc);
	uint32_t flags = 0;

	switch (op >> 24)
	{
		case 0x01:              util::stream_format(stream, "constn  %s", TYPE_3);                          break;
		case 0x02:              util::stream_format(stream, "consth  %s", TYPE_3);                          break;
		case 0x03:              util::stream_format(stream, "const   %s", TYPE_3);                          break;
		case 0x04:              util::stream_format(stream, "mtsrim  %s, $%04x", get_spr(OP_SA), OP_I16);   break;
		case 0x06: case 0x07:   util::stream_format(stream, "loadl   %s", TYPE_6);                          break;
		case 0x08:              util::stream_format(stream, "clz     r%d, %d", OP_RC, OP_RB);               break;
		case 0x09:              util::stream_format(stream, "clz     %d, %02x", OP_RC, OP_I8);              break;
		case 0x0a: case 0x0b:   util::stream_format(stream, "exbyte  %s", TYPE_1);                          break;
		case 0x0c: case 0x0d:   util::stream_format(stream, "inbyte  %s", TYPE_1);                          break;
		case 0x0e: case 0x0f:   util::stream_format(stream, "storel  %s", TYPE_6);                          break;
		case 0x10: case 0x11:   util::stream_format(stream, "adds    %s", TYPE_1);                          break;
		case 0x12: case 0x13:   util::stream_format(stream, "addu    %s", TYPE_1);                          break;
		case 0x14: case 0x15:   util::stream_format(stream, "add     %s", TYPE_1);                          break;
		case 0x16: case 0x17:   util::stream_format(stream, "load    %s", TYPE_6);                          break;
		case 0x18: case 0x19:   util::stream_format(stream, "addcs   %s", TYPE_1);                          break;
		case 0x1a: case 0x1b:   util::stream_format(stream, "addcu   %s", TYPE_1);                          break;
		case 0x1c: case 0x1d:   util::stream_format(stream, "addc    %s", TYPE_1);                          break;
		case 0x1e: case 0x1f:   util::stream_format(stream, "store   %s", TYPE_6);                          break;
		case 0x20: case 0x21:   util::stream_format(stream, "subs    %s", TYPE_1);                          break;
		case 0x22: case 0x23:   util::stream_format(stream, "subu    %s", TYPE_1);                          break;
		case 0x24: case 0x25:   util::stream_format(stream, "sub     %s", TYPE_1);                          break;
		case 0x26: case 0x27:   util::stream_format(stream, "loadset %s", TYPE_6);                          break;
		case 0x28: case 0x29:   util::stream_format(stream, "subcs   %s", TYPE_1);                          break;
		case 0x2a: case 0x2b:   util::stream_format(stream, "subcu   %s", TYPE_1);                          break;
		case 0x2c: case 0x2d:   util::stream_format(stream, "subc    %s", TYPE_1);                          break;
		case 0x2e: case 0x2f:   util::stream_format(stream, "cpbyte  %s", TYPE_1);                          break;
		case 0x30: case 0x31:   util::stream_format(stream, "subrs   %s", TYPE_1);                          break;
		case 0x32: case 0x33:   util::stream_format(stream, "subru   %s", TYPE_1);                          break;
		case 0x34: case 0x35:   util::stream_format(stream, "subr    %s", TYPE_1);                          break;
		case 0x36: case 0x37:   util::stream_format(stream, "loadm   %s", TYPE_6);                          break;
		case 0x38: case 0x39:   util::stream_format(stream, "subrcs  %s", TYPE_1);                          break;
		case 0x3a: case 0x3b:   util::stream_format(stream, "subrcu  %s", TYPE_1);                          break;
		case 0x3c: case 0x3d:   util::stream_format(stream, "subrc   %s", TYPE_1);                          break;
		case 0x3e: case 0x3f:   util::stream_format(stream, "storem  %s", TYPE_6);                          break;
		case 0x40: case 0x41:   util::stream_format(stream, "cplt    %s", TYPE_1);                          break;
		case 0x42: case 0x43:   util::stream_format(stream, "cpltu   %s", TYPE_1);                          break;
		case 0x44: case 0x45:   util::stream_format(stream, "cple    %s", TYPE_1);                          break;
		case 0x46: case 0x47:   util::stream_format(stream, "cpleu   %s", TYPE_1);                          break;
		case 0x48: case 0x49:   util::stream_format(stream, "cpgt    %s", TYPE_1);                          break;
		case 0x4a: case 0x4b:   util::stream_format(stream, "cpgtu   %s", TYPE_1);                          break;
		case 0x4c: case 0x4d:   util::stream_format(stream, "cpge    %s", TYPE_1);                          break;
		case 0x4e: case 0x4f:   util::stream_format(stream, "cpgeu   %s", TYPE_1);                          break;
		case 0x50: case 0x51:   util::stream_format(stream, "aslt    %s", TYPE_5);                          break;
		case 0x52: case 0x53:   util::stream_format(stream, "asltu   %s", TYPE_5);                          break;
		case 0x54: case 0x55:   util::stream_format(stream, "asle    %s", TYPE_5);                          break;
		case 0x56: case 0x57:   util::stream_format(stream, "asleu   %s", TYPE_5);                          break;
		case 0x58: case 0x59:   util::stream_format(stream, "asgt    %s", TYPE_5);                          break;
		case 0x5a: case 0x5b:   util::stream_format(stream, "asgtu   %s", TYPE_5);                          break;
		case 0x5c: case 0x5d:   util::stream_format(stream, "asge    %s", TYPE_5);                          break;
		case 0x5e: case 0x5f:   util::stream_format(stream, "asgeu   %s", TYPE_5);                          break;
		case 0x60: case 0x61:   util::stream_format(stream, "cpeq    %s", TYPE_1);                          break;
		case 0x62: case 0x63:   util::stream_format(stream, "cpneq   %s", TYPE_1);                          break;
		case 0x64: case 0x65:   util::stream_format(stream, "mul     %s", TYPE_1);                          break;
		case 0x66: case 0x67:   util::stream_format(stream, "mull    %s", TYPE_1);                          break;
		case 0x68:              util::stream_format(stream, "div0    r%d, r%d", OP_RC, OP_RB);              break;
		case 0x69:              util::stream_format(stream, "div0    r%d, %02x", OP_RC, OP_I8);             break;
		case 0x6a: case 0x6b:   util::stream_format(stream, "div     %s", TYPE_1);                          break;
		case 0x6c: case 0x6d:   util::stream_format(stream, "divl    %s", TYPE_1);                          break;
		case 0x6e: case 0x6f:   util::stream_format(stream, "divrem  %s", TYPE_1);                          break;
		case 0x70: case 0x71:   util::stream_format(stream, "aseq    %s", TYPE_5);                          break;
		case 0x72: case 0x73:   util::stream_format(stream, "asneq   %s", TYPE_5);                          break;
		case 0x74: case 0x75:   util::stream_format(stream, "mulu    %s", TYPE_1);                          break;
		case 0x78: case 0x79:   util::stream_format(stream, "inhw    %s", TYPE_1);                          break;
		case 0x7a: case 0x7b:   util::stream_format(stream, "extract %s", TYPE_1);                          break;
		case 0x7c: case 0x7d:   util::stream_format(stream, "exhw    %s", TYPE_1);                          break;
		case 0x7e:              util::stream_format(stream, "exhws   %s", TYPE_1);                          break;
		case 0x80: case 0x81:   util::stream_format(stream, "sll     %s", TYPE_1);                          break;
		case 0x82: case 0x83:   util::stream_format(stream, "srl     %s", TYPE_1);                          break;
		case 0x86: case 0x87:   util::stream_format(stream, "sra     %s", TYPE_1);                          break;
		case 0x88:              util::stream_format(stream, "iret");                                        break;
		case 0x89:              util::stream_format(stream, "halt");                                        break;
		case 0x8c:              util::stream_format(stream, "iretinv");                                     break;
		case 0x90: case 0x91:   util::stream_format(stream, "and     %s", TYPE_1);                          break;
		case 0x92: case 0x93:   util::stream_format(stream, "or      %s", TYPE_1);                          break;
		case 0x94: case 0x95:   util::stream_format(stream, "xor     %s", TYPE_1);                          break;
		case 0x96: case 0x97:   util::stream_format(stream, "xnor    %s", TYPE_1);                          break;
		case 0x98: case 0x99:   util::stream_format(stream, "nor     %s", TYPE_1);                          break;
		case 0x9a: case 0x9b:   util::stream_format(stream, "nand    %s", TYPE_1);                          break;
		case 0x9c: case 0x9d:   util::stream_format(stream, "andn    %s", TYPE_1);                          break;
		case 0x9e:              util::stream_format(stream, "setip   %s", TYPE_2);                          break;
		case 0x9f:              util::stream_format(stream, "inv");                                         break;
		case 0xa0:              util::stream_format(stream, "jmp     $%04x", pc + OP_SJMP);                 break;
		case 0xa1:              util::stream_format(stream, "jmp     $%04x", OP_IJMP);                      break;
		case 0xa4: case 0xa5:   util::stream_format(stream, "jmpf    %s", TYPE_4);                          break;
		case 0xa8: case 0xa9:   util::stream_format(stream, "call    %s", TYPE_4);                          break;
		case 0xac: case 0xad:   util::stream_format(stream, "jmpt    %s", TYPE_4);                          break;
		case 0xb4: case 0xb5:   util::stream_format(stream, "jmpfdec %s", TYPE_4);                          break;
		case 0xb6:              util::stream_format(stream, "mftlb   r%d, r%d", OP_RC, OP_RA);              break;
		case 0xbe:              util::stream_format(stream, "mttlb   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xc0:              util::stream_format(stream, "jmpi    r%d", OP_RB);                          break;
		case 0xc4:              util::stream_format(stream, "jmpfi   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xc6:              util::stream_format(stream, "mfsr    r%d, %s", OP_RC, get_spr(OP_SA));      break;
		case 0xc8:              util::stream_format(stream, "calli   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xcc:              util::stream_format(stream, "jmpti   r%d, r%d", OP_RA, OP_RB);              break;
		case 0xce:              util::stream_format(stream, "mtsr    %s, r%d", get_spr(OP_SA), OP_RB);      break;
		case 0xd7:              util::stream_format(stream, "emulate %s", TYPE_5);                          break;
		case 0xde:              util::stream_format(stream, "multm   %s", TYPE_2);                          break;
		case 0xdf:              util::stream_format(stream, "multmu  %s", TYPE_2);                          break;

		case 0xe0:              util::stream_format(stream, "multiply  %s", TYPE_2);                        break;
		case 0xe1:              util::stream_format(stream, "divide  %s", TYPE_2);                          break;
		default:                util::stream_format(stream, "??????");                                      break;
	}

	return 4 | flags | SUPPORTED;
}

/***************************************************************************

 h8disasm.c: Hitachi H8/3xx series disassembler

 Original by The_Author & DynaChicken for the ZiNc emulator.

 MAME version, portability cleanups, and bugfixes by R. Belmont

****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h8.h"

static const char *const bit_instr[8]		=	{"bset", "bnot", "bclr", "btst", "bor", "bxor", "band", "bld"};
static const char *const bit_instr2[8]		=	{"bset", "bnot", "bclr", "btst", "bior", "bixor", "biand", "bild"};
static const char *const imm32l_instr[8]	=	{"mov", "add", "cmp", "sub", "or", "xor", "and", "?"};
static const char *const branch_instr[16]	=	{"bt", "bf", "bhi", "bls", "bcc", "bcs", "bne", "beq", "bvc", "bvs", "bpl", "bmi", "bge", "blt", "bgt", "ble"};

static const char *const reg_names32[8]		=	{"ER0", "ER1", "ER2", "ER3", "ER4", "ER5", "ER6", "SP"};
static const char *const reg_names16[16]	=	{"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7"};
static const char *const reg_names8[16]		=	{"R0H", "R1H", "R2H", "R3H", "R4H", "R5H", "R6H", "R7H","R0L", "R1L", "R2L", "R3L", "R4L", "R5L", "R6L", "R7L"};

static UINT32 h8disasm_0(UINT32 address, UINT32 opcode, char *output, const UINT8 *oprom);
static UINT32 h8disasm_1(UINT32 address, UINT32 opcode, char *output, const UINT8 *oprom);
static UINT32 h8disasm_5(UINT32 address, UINT32 opcode, char *output, const UINT8 *oprom);
static UINT32 h8disasm_6(UINT32 address, UINT32 opcode, char *output, const UINT8 *oprom, UINT32 addr_mask);
static UINT32 h8disasm_7(UINT32 address, UINT32 opcode, char *output, const UINT8 *oprom, UINT32 addr_mask);

#define h8_mem_read16(x) ((oprom[x] << 8) | oprom[(x) + 1])
#define h8_mem_read32(x) ((oprom[x] << 24) | (oprom[(x) + 1] << 16) | (oprom[(x) + 2] << 8) | oprom[(x) + 3])

static offs_t h8_disasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 addr_mask)
{
	UINT32 size = 0;
	UINT16 opcode;

	opcode = h8_mem_read16(0);

	switch((opcode>>12) & 0xf)
	{
	case 0x0:
		size = h8disasm_0(pc, opcode, buffer, oprom);
		break;
	case 0x1:
		size = h8disasm_1(pc, opcode, buffer, oprom);
		break;
		// mov.b @xx:8, Rd (abs)
	case 0x2:
		sprintf(buffer, "%4.4x mov.b @%8.8x, %s", opcode, (0xffff00 | (opcode & 0xff))&addr_mask, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
		// mov.b Rs, @xx:8 (abs)
	case 0x3:
		sprintf(buffer, "%4.4x mov.b %s, @%8.8x", opcode, reg_names8[(opcode>>8) & 0xf], (0xfffff00 | (opcode & 0xff))&addr_mask);
		size = 2;
		break;
		// bcc @xx:8
	case 0x4:
		pc += 2;
		sprintf(buffer, "%4.4x %s %8.8x", opcode, branch_instr[(opcode >> 8) & 0xf], (pc + (INT8)(opcode &0xff))&addr_mask);
		size = 2;
		break;
		// group 5
	case 0x5:
		size = h8disasm_5(pc, opcode, buffer, oprom);
		break;
	case 0x6:
		size = h8disasm_6(pc, opcode, buffer, oprom, addr_mask);
		break;
	case 0x7:
		size = h8disasm_7(pc, opcode, buffer, oprom, addr_mask);
		break;
	case 0x8:
		// add.b #xx:8, Rd
		sprintf(buffer, "%4.4x add.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	case 0x9:
		// addx.b #xx:8, Rd
		sprintf(buffer, "%4.4x addx.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	case 0xa:
		// cmp.b #xx:8, Rd
		sprintf(buffer, "%4.4x cmp.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	case 0xb:
		// subx.b #xx:8, Rd
		sprintf(buffer, "%4.4x subx.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	case 0xc:
		// or.b #xx:8, Rd
		sprintf(buffer, "%4.4x or.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	case 0xd:
		// xor.b #xx:8, Rd
		sprintf(buffer, "%4.4x xor.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	case 0xe:
		// and.b #xx:8, Rd
		sprintf(buffer, "%4.4x and.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
		// mov.b #xx:8, Rd
	case 0xf:
		sprintf(buffer, "%4.4x mov.b #%2.2x, %s", opcode, opcode & 0xff, reg_names8[(opcode>>8) & 0xf]);
		size = 2;
		break;
	default:
		sprintf(buffer, "%4.4x", opcode);
		size = 2;
		break;
	}

	return size | DASMFLAG_SUPPORTED;
}

static UINT32 h8disasm_0(UINT32 pc, UINT32 opcode, char *buffer, const UINT8 *oprom)
{
	UINT32 size = 2;
	UINT16 data16;
	INT16 sdata16;
	INT32 sdata32;
	UINT16 ext16;

	switch((opcode>>8)&0xf)
	{
		// nop
	case 0:
		sprintf(buffer, "%4.4x nop", opcode);
		size = 2;
		break;
	case 0x1:
		// 0140 = LDC/STC.W
		if (opcode == 0x0140)
		{
			data16 = h8_mem_read16(2);
			switch((data16>>8) & 0xff)
			{
				case 0x69:
					// ERd
					if (data16 & 0x80)
					{
						sprintf(buffer, "%4.4x stc.w ccr, @%s", opcode, reg_names32[(data16>>4)&7]);
					}
					else
					{
						sprintf(buffer, "%4.4x ldc.w @%s, ccr", opcode, reg_names32[(data16>>4)&7]);
					}
					size = 4;
					break;

				case 0x6f:
					// @(disp, ERd)
					sdata16 = h8_mem_read16(4);
					if (data16 & 0x80)
					{
						sprintf(buffer, "%4.4x stc.w ccr, @(%x, %s)", opcode, sdata16, reg_names32[(data16>>4)&7]);
					}
					else
					{
						sprintf(buffer, "%4.4x ldc.w @(%x, %s), ccr", opcode, sdata16, reg_names32[(data16>>4)&7]);
					}
					size = 6;
					break;

				case 0x78:
					// @(disp24, ERd)
					size = 10;
					ext16 = h8_mem_read16(4);
					sdata32 = h8_mem_read32(6) & 0xffffff;
					if (ext16 == 0x6b20)
					{
						sprintf(buffer, "%4.4x ldc.w @(%x, %s), ccr", opcode, sdata32, reg_names32[(data16>>4)&7]);
					}
					else if (ext16 == 0x6ba0)
					{
						sprintf(buffer, "%4.4x stc.w ccr, @(%x, %s)", opcode, sdata32, reg_names32[(data16>>4)&7]);
					}
					else
					{
						sprintf(buffer, "Unknown stc.w CCR form, prefix 0x78, format %x\n", ext16);
					}
					break;

				case 0x6d:
					// stc.w ccr,-@ERd / ldc @ERd+, ccr
					if (data16 & 0x80)
					{
						sprintf(buffer, "%4.4x stc.w ccr, -@%s", opcode, reg_names32[(data16>>4)&7]);
					}
					else
					{
						sprintf(buffer, "%4.4x ldc.w @%s+, ccr", opcode, reg_names32[(data16>>4)&7]);
					}
					size = 4;
					break;

				case 0x6b:
					// stc.w CCR,@abs
					if ((data16&0xff) == 0x00)	// 16 bit absolute
					{
						ext16 = h8_mem_read16(4);
						sprintf(buffer, "%4.4x ldc.w @%04x, ccr", opcode, ext16);
						size = 6;
					}
					else if ((data16&0xff) == 0x20)	// 24 bit absolute
					{
						sdata32 = h8_mem_read32(4) & 0xffffff;
						sprintf(buffer, "%4.4x ldc.w @%06x, ccr", opcode, sdata32);
						size = 8;
					}
					else if ((data16&0xff) == 0x80)	// 16 bit absolute
					{
						ext16 = h8_mem_read16(4);
						sprintf(buffer, "%4.4x stc.w ccr, @%04x", opcode, ext16);
						size = 6;
					}
					else if ((data16&0xff) == 0xa0)	// 24 bit absolute
					{
						sdata32 = h8_mem_read32(4) & 0xffffff;
						sprintf(buffer, "%4.4x stc.w ccr, @%06x", opcode, sdata32);
						size = 8;
					}
					else
					{
						sprintf(buffer, "Unknown stc.w CCR form, prefix 0x6b, format %x\n", data16);
					}
					break;
			}
			break;
		}

		// 010x  where x should always be 0!
		if((opcode & 0xf) != 0)
		{
			sprintf(buffer, "%4.4x default", opcode);
			break;
		}
		switch((opcode>>4) & 0xf)
		{
		// 0100 mov.l prefix
		case 0:
			data16 = h8_mem_read16(2);
			switch((data16>>8) & 0xff)
			{
			case 0x69:
				// mov.l @rx, rx
				if((data16 & 0x80) == 0x80)
				{
					sprintf(buffer, "%4.4x mov.l %s, @%s", opcode, reg_names32[data16&7], reg_names32[(data16>>4)&7]);
				}
				else
				{
					sprintf(buffer, "%4.4x mov.l @%s, %s", opcode, reg_names32[(data16>>4)&7], reg_names32[data16&7]);
				}
				size = 4;
				break;
			case 0x6b:
				// mov.l @aa:x, Rx
				// 24 bit abs ?
				if((data16 & 0x20 )==0x20)
				{
					sdata32 = h8_mem_read32(4);
					sdata32 &= 0xffffff;
					size = 8;
				}
				else
				{
					sdata16 = h8_mem_read16(4);
					sdata32 = sdata16;
					size = 6;
				}
				if((data16 & 0x80) == 0x80)
				{
					sprintf(buffer, "%4.4x %x mov.l %s, @%x", opcode, data16, reg_names32[data16&7], sdata32);
				}
				else
				{
					sprintf(buffer, "%4.4x %x mov.l @%x, %s", opcode, data16, sdata32, reg_names32[data16&7]);
				}
				break;
			case 0x6d:
				// mov.l @(Ers+), rd
				if((data16 & 0x80) == 0x80)
				{
					sprintf(buffer, "%4.4x mov.l %s, @-%s", opcode, reg_names32[data16 & 0x7], reg_names32[(data16>>4)&7]);
				}
				else
				{
					sprintf(buffer, "%4.4x mov.l @%s+, %s", opcode, reg_names32[(data16>>4)&7], reg_names32[data16 & 0x7]);
				}
				size = 4;
				break;
			case 0x6f:
				// mov.l @(displ16 + Rs), rd
				sdata16=h8_mem_read16(4);
				if((data16 & 0x80) == 0x80)
				{
					sprintf(buffer, "%4.4x mov.l %s,@(%x, %s)", opcode, reg_names32[data16 &0x7], sdata16, reg_names32[(data16>>4)&7]);
				}
				else
				{
					sprintf(buffer, "%4.4x mov.l @(%x, %s), %s", opcode, sdata16, reg_names32[(data16>>4)&7], reg_names32[data16 &0x7]);
				}
				size = 6;
				break;
			case 0x78:
				// mov.l @(displ24 + Rs), rd
				size = 10;
				ext16 = h8_mem_read16(4);
				sdata32 = h8_mem_read32(6) & 0xffffff;
				if ( (data16 & 0x80) && ((ext16 & ~7) == 0x6ba0) )
				{
					sprintf(buffer, "%4.4x mov.l %s,@(%x, %s)", opcode, reg_names32[ext16&7], sdata32, reg_names32[(data16>>4)&7]);
				}
				else if ( (!(data16 & 0x80)) && ((ext16 & ~7) == 0x6b20) )
				{
					sprintf(buffer, "%4.4x mov.l @(%x, %s), %s", opcode, sdata32, reg_names32[(data16>>4)&7], reg_names32[ext16&7]);
				}
				else
				{
					sprintf(buffer, "%4.4x ? %x", opcode, data16);
				}
				break;

			default:
				sprintf(buffer, "%4.4x ? %x", opcode, data16);
				break;
			}
			break;
			// ldm/stm.l (ERn-ERn+1), @-SP
		case 0x1:
			data16 = h8_mem_read16(2);
			size = 4;
			if ((data16&0xfff0) == 0x6df0)
			{
				sprintf(buffer, "%4.4x stm.l (%s-%s), @-SP", opcode, reg_names32[data16 & 0x7], reg_names32[(data16 & 0x7)+1]);
			}
			else if ((data16&0xfff0) == 0x6d70)
			{
				sprintf(buffer, "%4.4x ldm.l @SP+, (%s-%s)", opcode, reg_names32[(data16 & 0x7)-1], reg_names32[(data16 & 0x7)]);
			}
			break;
			// ldm/stm.l (ERn-ERn+2), @-SP
		case 0x2:
			data16 = h8_mem_read16(2);
			size = 4;
			if ((data16&0xfff0) == 0x6df0)
			{
				sprintf(buffer, "%4.4x stm.l (%s-%s), @-SP", opcode, reg_names32[data16 & 0x7], reg_names32[(data16 & 0x7)+2]);
			}
			else if ((data16&0xfff0) == 0x6d70)
			{
				sprintf(buffer, "%4.4x ldm.l @SP+, (%s-%s)", opcode, reg_names32[(data16 & 0x7)-2], reg_names32[(data16 & 0x7)]);
			}
			break;
			// ldm/stm.l (ERn-ERn+3), @-SP
		case 0x3:
			data16 = h8_mem_read16(2);
			size = 4;
			if ((data16&0xfff0) == 0x6df0)
			{
				sprintf(buffer, "%4.4x stm.l (%s-%s), @-SP", opcode, reg_names32[data16 & 0x7], reg_names32[(data16 & 0x7)+3]);
			}
			else if ((data16&0xfff0) == 0x6d70)
			{
				sprintf(buffer, "%4.4x ldm.l @SP+, (%s-%s)", opcode, reg_names32[(data16 & 0x7)-3], reg_names32[(data16 & 0x7)]);
			}
			break;
			// ldc
		case 0x4:
			break;
			// sleep
		case 0x8:
			sprintf(buffer, "%4.4x sleep", opcode);
			size = 2;
			break;
			// mulxs
		case 0xc:
			data16 = h8_mem_read16(2);
			size = 4;
			if ((data16&0xff00) == 0x5000)
			{
				sprintf(buffer, "mulxs.b %s, %s", reg_names16[(data16>>4)&0xf], reg_names16[data16&0xf]);
			}
			else if ((data16&0xff00) == 0x5200)
			{
				sprintf(buffer, "mulxs.w %s, %s", reg_names16[(data16>>4)&0xf], reg_names16[data16&0xf]);
			}
			else
			{
				sprintf(buffer, "%04x %04x unknown\n", opcode, data16);
			}
			break;
			// divxs
		case 0xd:
			data16 = h8_mem_read16(2);
			size = 4;
			if ((data16&0xff00) == 0x5100)
			{
				sprintf(buffer, "divxs.b %s, %s", reg_names16[(data16>>4)&0xf], reg_names16[data16&0xf]);
			}
			else if ((data16&0xff00) == 0x5300)
			{
				sprintf(buffer, "divxs.w %s, %s", reg_names16[(data16>>4)&0xf], reg_names16[data16&0xf]);
			}
			else
			{
				sprintf(buffer, "%04x %04x unknown\n", opcode, data16);
			}
			break;
			// 01f0 and.l prefix
		case 0xf:
			// or.l/xor.l/and.l rs, rd
			data16 = h8_mem_read16(2);
			size = 4;
			switch((data16>>8)&0xff)
			{
			case 0x64:	// or.l ERs, ERd
				sprintf(buffer, "%4.4x or.l %s, %s", opcode, reg_names32[(data16>>4) & 0x7], reg_names32[data16 & 0x7]);
				break;
			case 0x65:	// xor.l ERs, ERd
				sprintf(buffer, "%4.4x xor.l %s, %s", opcode, reg_names32[(data16>>4) & 0x7], reg_names32[data16 & 0x7]);
				break;
			case 0x66:	// and.l ERs, ERd
				sprintf(buffer, "%4.4x and.l %s, %s", opcode, reg_names32[(data16>>4) & 0x7], reg_names32[data16 & 0x7]);
				break;
			default:
				sprintf(buffer, "%4.4x default", opcode);
				break;
			}
			break;
		default:
			sprintf(buffer, "%4.4x default", opcode);
			break;
		}
		break;
		// stc ccr, rd
	case 0x2:
		if(((opcode>>4) & 0xf) == 0)
		{
			sprintf(buffer, "%4.4x stc ccr, %s", opcode, reg_names8[opcode & 0xf]);
		}
		else if(((opcode>>4) & 0xf) == 1)   // H8S stc exr, rd
        {
			sprintf(buffer, "%4.4x stc exr, %s", opcode, reg_names8[opcode & 0xf]);
        }
		else
		{
			sprintf(buffer, "%4.4x default", opcode);
		}
		size = 2;
		break;
		// ldc rd, ccr
	case 0x3:
		if(((opcode>>4) & 0xf) == 0)
		{
			sprintf(buffer, "%4.4x ldc %s, ccr", opcode, reg_names8[opcode & 0xf]);
		}
		else if(((opcode>>4) & 0xf) == 1)   // H8S ldr rd, exr
        {
			sprintf(buffer, "%4.4x ldc %s, exr", opcode, reg_names8[opcode & 0xf]);
        }
		else
		{
			sprintf(buffer, "%4.4x default", opcode);
		}
		size = 2;
		break;
		// orc
	case 0x4:
		sprintf(buffer, "%4.4x orc #%2.2x, ccr", opcode, opcode & 0xff);
		size = 2;
		break;
		// xorc
	case 0x5:
		sprintf(buffer, "%4.4x xorc #%2.2x, ccr", opcode, opcode & 0xff);
		size = 2;
		break;
		// andc
	case 0x6:
		sprintf(buffer, "%4.4x andc #%2.2x, ccr", opcode, opcode & 0xff);
		size = 2;
		break;
		// ldc
	case 0x7:
		sprintf(buffer, "%4.4x ldc #%2.2x, ccr", opcode, opcode & 0xff);
		size = 2;
		break;
		// add.b rx, ry
	case 0x8:
		sprintf(buffer, "%4.4x add.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// add.w rx, ry
	case 0x9:
		sprintf(buffer, "%4.4x add.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// inc.b rx
	case 0xA:
		if(opcode&0x80)
		{
			if(opcode & 0x8)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x add.l %s, %s", opcode, reg_names32[(opcode>>4) & 7], reg_names32[opcode & 7]);
			}
		}
		else
		{
			if(opcode & 0xf0)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x inc.b %s", opcode, reg_names8[(opcode & 0xf)]);
			}
		}
		size = 2;
		break;
		//
	case 0xb:
		switch((opcode>>4)& 0xf)
		{
		case 0:
			sprintf(buffer, "%4.4x adds.l #1, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 5:
			sprintf(buffer, "%4.4x inc.w #1, %s", opcode, reg_names16[opcode & 0xf]);
			break;
		case 7:
			sprintf(buffer, "%4.4x inc.l #1, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 8:
			sprintf(buffer, "%4.4x adds.l #2, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 9:
			sprintf(buffer, "%4.4x adds.l #4, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 0xd:
			sprintf(buffer, "%4.4x inc.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			break;
		case 0xf:
			sprintf(buffer, "%4.4x inc.l #2, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		default:
			sprintf(buffer, "%4.4x illegal", opcode);
			break;
		}
		size = 2;
		break;
		// mov.b rx, ry
	case 0xc:
		sprintf(buffer, "%4.4x mov.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// mov.w rx, ry
	case 0xd:
		sprintf(buffer, "%4.4x mov.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// addx.b rx, ry
	case 0xe:
		sprintf(buffer, "%4.4x addx.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
	case 0xf:
		if(opcode & 0x80)
		{
			if(opcode & 8)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x mov.l %s, %s", opcode, reg_names32[(opcode>>4) & 0x7], reg_names32[opcode & 0x7]);
			}
		}
		else
		{
			if((opcode & 0xf0) !=0)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x daa.b %s", opcode, reg_names8[opcode & 0xf]);
			}
		}
		size = 2;
		break;
	default:
		sprintf(buffer, "%4.4x default", opcode);
		break;
	}
	return size;
}

static UINT32 h8disasm_1(UINT32 pc, UINT32 opcode, char *buffer, const UINT8 *oprom)
{
	UINT32 size = 2; //, mode8;
	switch((opcode>>8)&0xf)
	{
	case 0x0:
		switch((opcode>>4)&0xf)
		{
			// shll.b Rx
		case 0x0:
			sprintf(buffer, "%4.4x shll.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shll.w Rx
		case 0x1:
			sprintf(buffer, "%4.4x shll.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shll.l Rx
		case 0x3:
			sprintf(buffer, "%4.4x shll.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// shll.b #2, Rx
		case 0x4:
			sprintf(buffer, "%4.4x shll.b #2, %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shll.w Rx
		case 0x5:
			sprintf(buffer, "%4.4x shll.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shll.l #2, Rx
		case 0x7:
			sprintf(buffer, "%4.4x shll.l #2, %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// shal.b Rx
		case 0x8:
			sprintf(buffer, "%4.4x shal.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shal.w Rx
		case 0x9:
			sprintf(buffer, "%4.4x shal.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shal.l Rx
		case 0xb:
			sprintf(buffer, "%4.4x shal.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// shal.b #2, Rx
		case 0xc:
			sprintf(buffer, "%4.4x shal.b #2, %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shal.w #2, Rx
		case 0xd:
			sprintf(buffer, "%4.4x shal.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shal.l #2, Rx
		case 0xf:
			sprintf(buffer, "%4.4x shal.l #2, %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
		default:
			sprintf(buffer, "%4.4x default", opcode);
			size = 2;
			break;
		}
		break;

	case 0x1:
		switch((opcode>>4)&0xf)
		{
			// shlr.b rx
		case 0x0:
			sprintf(buffer, "%4.4x shlr.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shlr.w rx
		case 0x1:
			sprintf(buffer, "%4.4x shlr.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shlr.l rx
		case 0x3:
			sprintf(buffer, "%4.4x shlr.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// shlr.b #2, rx
		case 0x4:
			sprintf(buffer, "%4.4x shlr.b #2, %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shlr.w #2, rx
		case 0x5:
			sprintf(buffer, "%4.4x shlr.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shlr.l #2, rx
		case 0x7:
			sprintf(buffer, "%4.4x shlr.l #2, %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// shar.b rx
		case 0x8:
			sprintf(buffer, "%4.4x shar.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shar.w rx
		case 0x9:
			sprintf(buffer, "%4.4x shar.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shar.l rx
		case 0xb:
			sprintf(buffer, "%4.4x shar.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// shar.b #2, rx
		case 0xc:
			sprintf(buffer, "%4.4x shar.b #2, %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// shar.w #2, rx
		case 0xd:
			sprintf(buffer, "%4.4x shar.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// shar.l #2, rx
		case 0xf:
			sprintf(buffer, "%4.4x shar.l #2, %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
		default:
			sprintf(buffer, "%4.4x illegal ", opcode);
			size = 2;
			break;
		}
		break;
	case 0x2:
		switch((opcode>>4)&0xf)
		{
			// rotxl.b Rx
		case 0x0:
			sprintf(buffer, "%4.4x rotxl.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// rotxl.w Rx
		case 0x1:
			sprintf(buffer, "%4.4x rotxl.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// rotxl.l Rx
		case 0x3:
			sprintf(buffer, "%4.4x rotxl.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// rotl.b Rx
		case 0x8:
			sprintf(buffer, "%4.4x rotl.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// rotl.w Rx
		case 0x9:
			sprintf(buffer, "%4.4x rotl.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// rotl.l Rx
		case 0xb:
			sprintf(buffer, "%4.4x rotl.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// rotl.b #2, Rx
		case 0xc:
			sprintf(buffer, "%4.4x rotl.b #2, %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// rotl.w #2, Rx
		case 0xd:
			sprintf(buffer, "%4.4x rotl.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// rotl.l #2, Rx
		case 0xf:
			sprintf(buffer, "%4.4x rotl.l #2, %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
		default:
			sprintf(buffer, "%4.4x  ? ", opcode);
			size = 2;
			break;
		}
		break;
	case 0x3:
		switch((opcode>>4)&0xf)
		{
			// rotxr.b Rx
		case 0x0:
			sprintf(buffer, "%4.4x rotxr.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// rotxr.w Rx
		case 0x1:
			sprintf(buffer, "%4.4x rotxr.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// rotxr.l Rx
		case 0x3:
			sprintf(buffer, "%4.4x rotxr.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// rotr.b Rx
		case 0x8:
			sprintf(buffer, "%4.4x rotr.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// rotr.w Rx
		case 0x9:
			sprintf(buffer, "%4.4x rotr.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// rotr.l Rx
		case 0xb:
			sprintf(buffer, "%4.4x rotr.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// rotr.b #2, Rx
		case 0xc:
			sprintf(buffer, "%4.4x rotr.b #2, %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// rotr.w #2, Rx
		case 0xd:
			sprintf(buffer, "%4.4x rotr.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// rotr.l #2, Rx
		case 0xf:
			sprintf(buffer, "%4.4x rotr.l #2, %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
		default:
			sprintf(buffer, "%4.4x  ? ", opcode);
			size = 2;
			break;
		}
		break;
		// or.b rs, rd
	case 0x4:
		sprintf(buffer, "%4.4x or.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// xor.b rs, rd
	case 0x5:
		sprintf(buffer, "%4.4x xor.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// and.b rs, rd
	case 0x6:
		sprintf(buffer, "%4.4x and.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// not
	case 0x7:
		switch((opcode>>4)&0xf)
		{
			// not.b Rx
		case 0x0:
			sprintf(buffer, "%4.4x not.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// not.w Rx
		case 0x1:
			sprintf(buffer, "%4.4x not.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// not.l Rx
		case 0x3:
			sprintf(buffer, "%4.4x not.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// extu.b Rx
		case 0x4:
			sprintf(buffer, "%4.4x extu.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// extu.w Rx
		case 0x5:
			sprintf(buffer, "%4.4x extu.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// extu.l Rx
		case 0x7:
			sprintf(buffer, "%4.4x extu.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// neg.b Rx
		case 0x8:
			sprintf(buffer, "%4.4x neg.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// neg.w Rx
		case 0x9:
			sprintf(buffer, "%4.4x neg.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// neg.l Rx
		case 0xb:
			sprintf(buffer, "%4.4x neg.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
			// exts.b Rx
		case 0xc:
			sprintf(buffer, "%4.4x exts.b %s", opcode, reg_names8[opcode & 0xf]);
			size = 2;
			break;
			// exts.w Rx
		case 0xd:
			sprintf(buffer, "%4.4x exts.w %s", opcode, reg_names16[opcode & 0xf]);
			size = 2;
			break;
			// exts.l Rx
		case 0xf:
			sprintf(buffer, "%4.4x exts.l %s", opcode, reg_names32[opcode & 7]);
			size = 2;
			break;
		default:
			sprintf(buffer, "%4.4x  default ", opcode);
			size = 2;
			break;
		}
		break;
		// sub.b rs, rd
	case 0x8:
		sprintf(buffer, "%4.4x sub.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// sub.w rs, rd
	case 0x9:
		sprintf(buffer, "%4.4x sub.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// sub.b rx
	case 0xA:
		if(opcode&0x80)
		{
			if(opcode & 0x8)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x sub.l %s, %s", opcode, reg_names32[(opcode>>4) & 7], reg_names32[opcode & 7]);
			}
		}
		else
		{
			if(opcode & 0xf0)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x dec.b %s", opcode, reg_names8[(opcode & 0xf)]);
			}
		}
		size = 2;
		break;
		//
	case 0xb:
		switch((opcode>>4)& 0xf)
		{
		case 0:
			sprintf(buffer, "%4.4x subs.l #1, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 5:
			sprintf(buffer, "%4.4x dec.w #1, %s", opcode, reg_names16[opcode & 0xf]);
			break;
		case 7:
			sprintf(buffer, "%4.4x dec.l #1, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 8:
			sprintf(buffer, "%4.4x subs.l #2, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 9:
			sprintf(buffer, "%4.4x subs.l #4, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		case 0xd:
			sprintf(buffer, "%4.4x dec.w #2, %s", opcode, reg_names16[opcode & 0xf]);
			break;
		case 0xf:
			sprintf(buffer, "%4.4x dec.l #2, %s", opcode, reg_names32[opcode & 0x7]);
			break;
		default:
			sprintf(buffer, "%4.4x ?", opcode);
			break;
		}
		size = 2;
		break;
		// cmp.b rs, rd
	case 0xc:
		sprintf(buffer, "%4.4x cmp.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// cmp.w rx, ry
	case 0xd:
		sprintf(buffer, "%4.4x cmp.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// subx.b rx, ry
	case 0xe:
		sprintf(buffer, "%4.4x subx.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
	case 0xf:
		if(opcode & 0x80)
		{
			if(opcode & 8)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x cmp.l %s, %s", opcode, reg_names32[(opcode>>4) & 0x7], reg_names32[opcode & 0x7]);
			}
		}
		else
		{
			if((opcode & 0xf0) !=0)
			{
				sprintf(buffer, "%4.4x illegal", opcode);
			}
			else
			{
				sprintf(buffer, "%4.4x das.b %s", opcode, reg_names8[opcode & 0xf]);
			}
		}
		size = 2;
		break;
	default:
		sprintf(buffer, "%4.4x default", opcode);
		break;
	}

	return size;
}

static UINT32 h8disasm_5(UINT32 pc, UINT32 opcode, char *buffer, const UINT8 *oprom)
{
	UINT32 size = 2;
	UINT32 data32;
	INT16 data16;

	switch((opcode>>8)&0xf)
	{
		// mulxu.b
	case 0x0:
		sprintf(buffer, "%4.4x mulxu.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// divxu.b
	case 0x1:
		sprintf(buffer, "%4.4x divxu.b %s, %s", opcode, reg_names8[(opcode>>4) & 0xf], reg_names8[opcode & 0xf]);
		size = 2;
		break;
		// mulxu.w
	case 0x2:
		sprintf(buffer, "%4.4x mulxu.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names32[opcode & 7]);
		size = 2;
		break;
		// divxu.w
	case 0x3:
		sprintf(buffer, "%4.4x divxu.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names32[opcode & 7]);
		size = 2;
		break;
		// rts
	case 0x4:
		if(opcode == 0x5470)
		{
			sprintf(buffer, "%4.4x rts", opcode);
		}
		else
		{
			sprintf(buffer, "%4.4x default", opcode);
		}
			size = 2 | DASMFLAG_STEP_OUT;
		break;
		// bsr 8
	case 0x5:
		sprintf(buffer, "%4.4x bsr %x", opcode, pc+2 + ((INT8)opcode& 0xff));
		size = 2 | DASMFLAG_STEP_OVER;
		break;
		// rte
	case 0x6:
		if(opcode == 0x5670)
		{
			sprintf(buffer, "%4.4x rte", opcode);
		}
		else
		{
			sprintf(buffer, "%4.4x default", opcode);
		}
		size = 2 | DASMFLAG_STEP_OUT;
		break;
		// trapa
	case 0x7:
		if(opcode & 0xcf)
		{
			sprintf(buffer, "%4.4x illegal", opcode);
		}
		else
		{
			sprintf(buffer, "%4.4x trapa #%x", opcode, (opcode >> 4) & 3);
		}
		size = 2 | DASMFLAG_STEP_OVER;
		break;
		// bcc 16
	case 0x8:
		if(opcode & 0xf)
		{
			sprintf(buffer, "%4.4x illegal", opcode);
			size = 2;
		}
		else
		{
			data16 = h8_mem_read16(2);
			pc += 4;
			sprintf(buffer, "%4.4x %s %8.8x", opcode, branch_instr[(opcode >> 4) & 0xf], pc + (INT16)(data16));
			size = 4;
		}
		break;
		// jmp @erd
	case 0x9:
		sprintf(buffer, "%4.4x jmp @%s", opcode, reg_names32[(opcode>>4) &7]);
		size = 2;
		break;
		// jmp @aa:24
	case 0xa:
		data32=h8_mem_read32(0);
		sprintf(buffer, "%4.4x jmp %8.8x", opcode, (data32& 0xffffff));
		size = 4;
		break;
		// jmp @aa:8
	case 0xb:
		sprintf(buffer, "%4.4x jmp %8.8x", opcode, (opcode & 0xff));
		size = 2;
		break;
		// bsr @aa:16
	case 0xc:
		if(opcode & 0xff)
		{
			sprintf(buffer, "%4.4x illegal", opcode);
			size = 2;
		}
		else
		{
			data16=h8_mem_read16(2);
			sprintf(buffer, "%4.4x bsr %8.8x", opcode, data16+pc+4);
			size = 4 | DASMFLAG_STEP_OVER;
		}
		break;
		// jsr @erd
	case 0xd:
		sprintf(buffer, "%4.4x jsr @%s", opcode, reg_names32[(opcode>>4) &7]);
		size = 2 | DASMFLAG_STEP_OVER;
		break;
		// jsr @aa:24
	case 0xe:
		data32=h8_mem_read32(0);
		sprintf(buffer, "%4.4x jsr %8.8x", opcode, (data32& 0xffffff));
		size = 4 | DASMFLAG_STEP_OVER;
		break;
		// jsr @aa:8
	case 0xf:
		sprintf(buffer, "%4.4x jsr %8.8x", opcode, (opcode & 0xff));
		size = 2 | DASMFLAG_STEP_OVER;
		break;
	default:
		sprintf(buffer, "%4.4x default", opcode);
		break;
	}
	return size;
}

static UINT32 h8disasm_6(UINT32 address, UINT32 opcode, char *buffer, const UINT8 *oprom, UINT32 addr_mask)
{
	UINT32 size = 2;
	UINT32 data32;
	INT16 data16;

	switch((opcode>>8)&0xf)
	{
	case 0:case 1:case 2:case 3:
		if(((opcode>>4)&0x8) == 0)
		{
			sprintf(buffer, "%4.4x %s.b %s, %s", opcode, bit_instr[(opcode>>8)&7], reg_names8[(opcode>>4)&0xf], reg_names8[(opcode & 0xf)]);
		}
		else
		{
			sprintf(buffer, "%4.4x %s.b %s, %s", opcode, bit_instr2[(opcode>>8)&7], reg_names8[(opcode>>4)&0xf], reg_names8[(opcode & 0xf)]);
		}
		size = 2;
		break;
		// or.w rs, rd
	case 0x4:
		sprintf(buffer, "%4.4x or.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// xor.w rs, rd
	case 0x5:
		sprintf(buffer, "%4.4x xor.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// and.w rs, rd
	case 0x6:
		sprintf(buffer, "%4.4x and.w %s, %s", opcode, reg_names16[(opcode>>4) & 0xf], reg_names16[opcode & 0xf]);
		size = 2;
		break;
		// bst/bist #imm, rd
	case 0x7:
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x bist.b #%1.1x, %s", opcode, (opcode>>4)&7, reg_names8[(opcode & 0xf)]);
		}
		else
		{
			sprintf(buffer, "%4.4x bst.b #%1.1x, %s", opcode, (opcode>>4)&7, reg_names8[(opcode & 0xf)]);
		}
		size = 2;
		break;
	case 0x8:
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x mov.b %s, @%s", opcode, reg_names8[opcode & 0xf], reg_names32[(opcode >> 4) & 7]);
			size = 2;
		}
		else
		{
			sprintf(buffer, "%4.4x mov.b @%s, %s", opcode, reg_names32[(opcode >> 4) & 7], reg_names8[opcode & 0xf]);
			size = 2;
		}
		break;
	case 0x9:
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x mov.w %s, @%s", opcode, reg_names16[opcode & 0xf], reg_names32[(opcode >> 4) & 7]);
			size = 2;
		}
		else
		{
			sprintf(buffer, "%4.4x mov.w @%s, %s", opcode, reg_names32[(opcode >> 4) & 7], reg_names16[opcode & 0xf]);
			size = 2;
		}
		break;
	case 0xa:
		// mov.b rx, @xx
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			data16=h8_mem_read16(2);
			sprintf(buffer, "%4.4x mov.b @%x, %s", opcode, data16, reg_names8[opcode & 0xf]);
			size = 4;
			break;
		case 0x2:
			data32=h8_mem_read32(2);
			sprintf(buffer, "%4.4x mov.b @%8.8x, %s", opcode, data32&addr_mask, reg_names8[opcode & 0xf]);
			size = 6;
			break;
		case 0x3:	// bclr #imm, imm:32
			data32=h8_mem_read32(2);
			data16=h8_mem_read16(6);
			switch ((data16 >> 8) & 0xff)
			{
				case 0x60 :
					sprintf(buffer, "%4.4x bset %s, @%8.8x", opcode, reg_names16[(data16>>4)&0xf], data32&addr_mask);
					break;
				case 0x61 :
					sprintf(buffer, "%4.4x bnot %s, @%8.8x", opcode, reg_names16[(data16>>4)&0xf], data32&addr_mask);
					break;
				case 0x62 :
					sprintf(buffer, "%4.4x bclr %s, @%8.8x", opcode, reg_names16[(data16>>4)&0xf], data32&addr_mask);
					break;
				case 0x63:
					sprintf(buffer, "%4.4x btst %s, @%8.8x", opcode, reg_names16[(data16>>4)&0xf], data32&addr_mask);
					break;
				case 0x70 :
					sprintf(buffer, "%4.4x bset #%d, @%8.8x", opcode, (data16>>4)&7, data32&addr_mask);
					break;
				case 0x71 :
					sprintf(buffer, "%4.4x bnot #%d, @%8.8x", opcode, (data16>>4)&7, data32&addr_mask);
					break;
				case 0x72 :
					sprintf(buffer, "%4.4x bclr #%d, @%8.8x", opcode, (data16>>4)&7, data32&addr_mask);
					break;
				case 0x73 :
					sprintf(buffer, "%4.4x btst #%d, @%8.8x", opcode, (data16>>4)&7, data32&addr_mask);
					break;
			}
			size = 8;
			break;
		case 0x8:
			data16=h8_mem_read16(2);
			sprintf(buffer, "%4.4x mov.b %s, @%8.8x", opcode, reg_names8[opcode & 0xf], data16&addr_mask);
			size = 4;
			break;
		case 0xa:
			data32=h8_mem_read32(2);
			sprintf(buffer, "%4.4x mov.b %s, @%8.8x", opcode, reg_names8[opcode & 0xf], data32&addr_mask);
			size = 6;
			break;
		default:
			sprintf(buffer, "%4.4x 0x6a ? ", opcode);
			size = 2;
			break;
		}
		break;
	case 0xb:
		// mov.w rx, @xx
		switch((opcode>>4)&0xf)
		{
		case 0x0:
			data16=h8_mem_read16(2);
			sprintf(buffer, "%4.4x mov.w @%8.8x, %s", opcode, data16&addr_mask, reg_names16[opcode & 0xf]);
			size = 4;
			break;
		case 0x2:
			data32=h8_mem_read32(2);
			sprintf(buffer, "%4.4x mov.w @%8.8x, %s", opcode, data32&addr_mask, reg_names16[opcode & 0xf]);
			size = 6;
			break;
		case 0x8:
			data16=h8_mem_read16(2);
			sprintf(buffer, "%4.4x mov.w %s, @%8.8x", opcode, reg_names16[opcode & 0xf], data16&addr_mask);
			size = 4;
			break;
		case 0xa:
			data32=h8_mem_read32(2);
			sprintf(buffer, "%4.4x mov.w %s, @%8.8x", opcode, reg_names16[opcode & 0xf], data32&addr_mask);
			size = 6;
			break;
		default:
			sprintf(buffer, "%4.4x 0x6b ? ", opcode);
			size = 2;
			break;
		}
		break;
	case 0xc:
		// mov.b @erx+,rx / mov.b @-erx, rx
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x mov.b %s, @(-%s)", opcode, reg_names8[opcode &0xf], reg_names32[(opcode>>4)&7]);
		}
		else
		{
			sprintf(buffer, "%4.4x mov.b @(%s+), %s", opcode, reg_names32[(opcode>>4)&7], reg_names8[opcode &0xf]);
		}
		size = 2;
		break;
	case 0xd:
		// mov.w @ers+, rd / mov.w rs, @-erd
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x mov.w %s, @-%s", opcode, reg_names16[opcode&0xf], reg_names32[(opcode>>4)&7]);
		}
		else
		{
			sprintf(buffer, "%4.4x mov.w @%s+, %s", opcode, reg_names32[(opcode>>4)&7], reg_names16[opcode & 0xf]);
		}
		size = 2;
		break;
	case 0xe:
		// mov.b @(displ16 + Rs), rd
		data16=h8_mem_read16(2);
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x mov.b %s,@(%x, %s)", opcode, reg_names8[opcode &0xf], data16, reg_names32[(opcode>>4)&7]);
		}
		else
		{
			sprintf(buffer, "%4.4x mov.b @(%x, %s), %s", opcode, data16, reg_names32[(opcode>>4)&7], reg_names8[opcode &0xf]);
		}
		size = 4;
		break;
	case 0xf:
		// mov.w @(displ16 + Rs), rd
		data16=h8_mem_read16(2);
		if(opcode & 0x80)
		{
			sprintf(buffer, "%4.4x mov.w %s,@(%x, %s)", opcode, reg_names16[opcode &0xf], data16, reg_names32[(opcode>>4)&7]);
		}
		else
		{
			sprintf(buffer, "%4.4x mov.w @(%x, %s), %s", opcode, data16, reg_names32[(opcode>>4)&7], reg_names16[opcode &0xf]);
		}
		size = 4;
		break;
	default:
		sprintf(buffer, "%4.4x default", opcode);
		break;
	}
	return size;
}

static UINT32 h8disasm_7(UINT32 address, UINT32 opcode, char *buffer, const UINT8 *oprom, UINT32 addr_mask)
{
	UINT32 size = 2;
	UINT32 data32;
	UINT16 data16;

	switch((opcode>>8)&0xf)
	{
	case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:
		if(((opcode>>4)&0x8) == 0)
		{
			sprintf(buffer, "%4.4x %s.b #%1.1x, %s", opcode, bit_instr[(opcode>>8)&7], (opcode>>4)&7, reg_names8[(opcode & 0xf)]);
		}
		else
		{
			sprintf(buffer, "%4.4x %s.b #%1.1x, %s", opcode, bit_instr2[(opcode>>8)&7], (opcode>>4)&7, reg_names8[(opcode & 0xf)]);
		}
		size = 2;
		break;
	case 0x8:
		data16 = h8_mem_read16(2);
		data32 = h8_mem_read32(4);
		if(((data16>>8) & 0xf) == 0xa)
		{
			if(((data16>>4) & 0xf) == 0xa)
			{
				sprintf(buffer, "%4.4x mov.b %s, @(%x, %s)", opcode, reg_names8[(data16 & 0xf)], data32, reg_names32[(opcode>>4)&7]);
			}
			else
			{
				sprintf(buffer, "%4.4x mov.b @(%x, %s), %s", opcode, data32, reg_names32[(opcode>>4) & 7], reg_names8[(data16 & 0xf)]);
			}
		}
		else
		{
			if(((data16>>4) & 0xf) == 0xa)
			{
				sprintf(buffer, "%4.4x mov.w %s, @(%x, %s)", opcode, reg_names16[(data16 & 0xf)], data32, reg_names32[(opcode>>4)&7]);
			}
			else
			{
				sprintf(buffer, "%4.4x mov.w @(%x, %s), %s", opcode, data32, reg_names32[(opcode>>4) & 7], reg_names16[(data16 & 0xf)]);
			}
		}
		size = 8;
		break;
		// xxx.w #aa:16, rd
	case 0x9:
		if( ((opcode>>4) & 0xf) > 0x6)
		{
			sprintf(buffer, "%4.4x illegal", opcode);
			size = 2;
		}
		else
		{
			data16 = h8_mem_read16(2);
			sprintf(buffer, "%4.4x %s.w #%x, %s", opcode, imm32l_instr[(opcode>>4)&7], data16, reg_names16[opcode&0xf]);
			size = 4;
		}
		break;
		// xxx.l #aa:32, erd
	case 0xa:
		if( (((opcode>>4) & 0xf) > 0x6) || (opcode & 0x8))
		{
			sprintf(buffer, "%4.4x illegal", opcode);
			size = 2;
		}
		else
		{
			data32 = h8_mem_read32(2);
			sprintf(buffer, "%4.4x %s.l #%x, %s", opcode, imm32l_instr[(opcode>>4)&7], data32, reg_names32[opcode&0x7]);
			size = 6;
		}
		break;
		// eepmov
	case 0xb:
		switch (opcode & 0x00ff)
		{
			case 0x5c:
			sprintf(buffer, "%4.4x eepmov.b", opcode);
			break;
			case 0xd4:
			sprintf(buffer, "%4.4x eepmov.w", opcode);
			break;
			default:
			sprintf(buffer, "%4.4x eepmov UNK!", opcode);
			break;
		}
		size = 4;
		break;
		// bxx.b #xx:3, @rd
	case 0xc:
	case 0xd:
		data16 = h8_mem_read16(2);
		size = 4;
		switch(data16>>8)
		{
			case 0x60:	// bset/bnot/bclr.b rn, @erd
			case 0x61:
			case 0x62:
				if (((opcode & 0x8f)!=0)||((data16 & 0x0f)!=0))
				{
					sprintf(buffer, "%4.4x default", opcode);
					break;
				}
				sprintf(buffer, "%4.4x %s.b %s, @%s", opcode, bit_instr[(data16>>8)&7], reg_names16[(data16>>4)&0xf], reg_names32[(opcode>>4) & 0x7]);
				break;

			case 0x67:	// bst/bist.b #xx:3, @erd
				if (((opcode & 0x8f)!=0)||((data16 & 0x0f)!=0))
				{
					sprintf(buffer, "%4.4x default", opcode);
					break;
				}
				if ((data16 & 0x80)!=0)
					sprintf(buffer, "%4.4x bist.b #%1.1x, @%s", opcode, (data16>>4)&7, reg_names32[(opcode>>4) & 0x7]);
				else
					sprintf(buffer, "%4.4x bst.b #%1.1x, @%s", opcode, (data16>>4)&7, reg_names32[(opcode>>4) & 0x7]);
				break;

			case 0x70:	// bset/bnot/bclr.b #xx:3, @erd
			case 0x71:
			case 0x72:
				if (((opcode & 0x8f)!=0)||((data16 & 0x8f)!=0))
				{
					sprintf(buffer, "%4.4x default", opcode);
					break;
				}
				sprintf(buffer, "%4.4x %s.b #%1.1x, @%s", opcode, bit_instr[(data16>>8)&7], (data16>>4)&7, reg_names32[(opcode>>4) & 0x7]);
				break;

			case 0x77:	// bld #xx:3, @rd
				sprintf(buffer, "%4.4x bld #%d, @%s", opcode, (data16>>4)&7, reg_names32[(opcode>>4) & 0x7]);
				break;
		}
		break;
		// bxx.b #imm, @aa:8
	case 0xe:
	case 0xf:
		data16 = h8_mem_read16(2);
		if(((data16>>4)&0x8) == 0)
		{
			sprintf(buffer, "%4.4x %4.4x %s.b #%1.1x, @%x", opcode, data16, bit_instr[(data16>>8)&7], (data16>>4)&7, (0xffffff00 + (opcode & 0xff))&addr_mask);
		}
		else
		{
			sprintf(buffer, "%4.4x %4.4x %s.b #%1.1x, @%x", opcode, data16, bit_instr2[(data16>>8)&7], (data16>>4)&7, (0xffffff00 + (opcode & 0xff))&addr_mask);
		}
		size = 4;
		break;
	default:
		sprintf(buffer, "%4.4x default", opcode);
		break;
	}
	return size;
}

CPU_DISASSEMBLE(h8)
{
	return h8_disasm(buffer, pc, oprom, opram, 0xffff);
}

// disassembly hook for varients with 24-bit address bus (e.g. H8/3044)
CPU_DISASSEMBLE(h8_24)
{
	return h8_disasm(buffer, pc, oprom, opram, 0xffffff);
}

// disassembly hook for full 32-bit address bus
CPU_DISASSEMBLE(h8_32)
{
	return h8_disasm(buffer, pc, oprom, opram, 0xffffffff);
}


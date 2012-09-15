/*
    Sega Saturn SCU DSP disassembler

    Written by Angelo Salese
*/

#include "emu.h"

enum
{
	EA_A = 1,
	EA_ALU,
	EA_D0,
	EA_IMM8,
	EA_IMM18,
	EA_IMM25,
	EA_MUL,
	EA_P,
	EA_X,
	EA_Y,
	EA_SRCMEMX,
	EA_SRCMEMY,
	EA_SRCMEMD1,
	EA_DMADSTMEM,
	EA_DSTMEM,
	EA_MVIDSTMEM,
};

struct SCUDSP_OPCODE {
	char mnemonic[32];
	int address_mode_1;
	int address_mode_2;
	int address_mode_3,
};

static const SCUDSP_OPCODE alu_table[16] =
{
	{ "NOP", 0, 0, 0, }, /* 0000 */
	{ "AND", 0, 0, 0, }, /* 0001 */
	{ "OR ", 0, 0, 0, }, /* 0010 */
	{ "XOR", 0, 0, 0, }, /* 0011 */
	{ "ADD", 0, 0, 0, }, /* 0100 */
	{ "SUB", 0, 0, 0, }, /* 0101 */
	{ "AD2", 0, 0, 0, }, /* 0110 */
	{ "???", 0, 0, 0, }, /* 0111 */
	{ "SR ", 0, 0, 0, }, /* 1000 */
	{ "RR ", 0, 0, 0, }, /* 1001 */
	{ "SL ", 0, 0, 0, }, /* 1010 */
	{ "RL ", 0, 0, 0, }, /* 1011 */
	{ "???", 0, 0, 0, }, /* 1100 */
	{ "???", 0, 0, 0, }, /* 1101 */
	{ "???", 0, 0, 0, }, /* 1110 */
	{ "RL8", 0, 0, 0, }, /* 1111 */
};

static const SCUDSP_OPCODE xbus_table[] =
{
	{ "NOP", 0,          0,    0, },			/* 000 */
	{ "???", 0,          0,    0, },			/* 001 */
	{ "MOV", EA_MUL,     EA_P, 0, },	/* 010 */
	{ "MOV", EA_SRCMEMX, EA_P, 0, },		/* 011 */ //MOV %s,P
	{ "MOV", EA_SRCMEMX, EA_X, 0, },		/* 100 */ //MOV %s,X
	{ "???", 0,          0,    0, },			/* 101 */
	{ "???", 0,          0,    0, },			/* 110 */
	{ "???", 0,          0,    0, },			/* 111 */
};

static const SCUDSP_OPCODE ybus_table[] =
{
	{ "NOP", 0,          0,    0, },	/* 000 */
	{ "CLR", 0,          EA_A, 0, },	/* 001 */
	{ "MOV", EA_ALU,     EA_A, 0, },	/* 010 */
	{ "MOV", EA_SRCMEMY, EA_A, 0, },	/* 011 */ //MOV %s,A
	{ "MOV", EA_SRCMEMY, EA_Y, 0, },	/* 100 */ //MOV %s,Y
	{ "???", 0,          0,    0, },				/* 101 */
	{ "???", 0,          0,    0, },				/* 110 */
	{ "???", 0,          0,    0, },				/* 111 */
};

static const SCUDSP_OPCODE d1bus_table[] =
{
	{ "NOP", 0,          0,         0, },					/* 00 */
	{ "MOV", EA_IMM8,    EA_DSTMEM, 0, },		/* 01 */ //MOV %I8,%d
	{ "???", 0,          0,         0, },					/* 10 */
	{ "MOV", EA_SRCMEMD1,0,         0, },			/* 11 */ //MOV %S,%d
};

static const SCUDSP_OPCODE mvi_table[] =
{
	{ "MVI", EA_IMM25,   EA_MVIDSTMEM,  0, },					/* 0 */ //"MVI %I,%d"
	{ "MVI", EA_IMM18,   EA_MVIDSTMEM,  EA_FLAGS, },			/* 1 */ //"MVI %I,%d,%f"
};

static const SCUDSP_OPCODE dma_table[] =
{
	{ "DMA",  EA_D0,          EA_DMADSTMEM,  EA_IMM8, }, /* 000 */ // "DMA%H%A D0,%M,%I",
	{ "DMA",  EA_DMASRCMEM,   EA_D0,  EA_IMM8, },	/* 001 */ // "DMA%H%A %s,D0,%I",
	{ "DMA",  0,          0,  0, },	/* 010 */ // "DMA%H%A D0,%M,%s",
	{ "DMA",  0,   0,  0, },						/* 011 */ // "DMA%H%A %s,D0,%s",
	{ "DMAH", EA_D0,   EA_DMADSTMEM,  EA_IMM8, },	/* 100 */ // "DMA%H%A D0,%M,%I",
	{ "DMAH", EA_DMASRCMEM,   EA_D0,  EA_IMM8, },	/* 101 */ // "DMA%H%A %s,D0,%I",
	{ "DMAH", 0,   0,  0, },						/* 110 */ // "DMA%H%A D0,%M,%s",
	{ "DMAH", 0,   0,  0, },						/* 111 */ // "DMA%H%A %s,D0,%s",

};

static const SCUDSP_OPCODE jmp_table[] =
{
	{ "JMP", EA_IMM8, 0, 0, }, /* 0 */ // unconditional
	{ "JMP", EA_IMM8, 0, EA_FLAGS, }, /* 1 */ // conditional
};

static const SCUDSP_OPCODE loop_table[] =
{
	{ "BTM", 0,   0,  0, },					/* 00 */
	{ "LPS", 0,   0,  0, },					/* 01 */
};

static const SCUDSP_OPCODE end_table[] =
{
	{ "END", 0,   0,  0, },					/* 00 */
	{ "ENDI",0,   0,  0, },					/* 01 */
};


static const char *const src_mem[] =
{
	"M0",			/* 0000 */
	"M1",			/* 0001 */
	"M2",			/* 0010 */
	"M3",			/* 0011 */
	"MC0",			/* 0100 */
	"MC1",			/* 0101 */
	"MC2",			/* 0110 */
	"MC3",			/* 0111 */
	"???",			/* 1000 */
	"ALL",			/* 1001 */
	"ALH",			/* 1010 */
	"???",			/* 1011 */
	"???",			/* 1100 */
	"???",			/* 1101 */
	"???",			/* 1110 */
	"???",			/* 1111 */
};

static const char *const dst_mem[] =
{
	"MC0",			/* 0000 */
	"MC1",			/* 0001 */
	"MC2",			/* 0010 */
	"MC3",			/* 0011 */
	"RX",			/* 0100 */
	"PL",			/* 0101 */
	"RA0",			/* 0110 */
	"WA0",			/* 0111 */
	"???",			/* 1000 */
	"???",			/* 1001 */
	"LOP",			/* 1010 */
	"TOP",			/* 1011 */
	"CT0",			/* 1100 */
	"CT1",			/* 1101 */
	"CT2",			/* 1110 */
	"CT3",			/* 1111 */
};

static const char *const mvi_dst_mem[] =
{
	"MC0",			/* 0000 */
	"MC1",			/* 0001 */
	"MC2",			/* 0010 */
	"MC3",			/* 0011 */
	"RX",			/* 0100 */
	"PL",			/* 0101 */
	"RA0",			/* 0110 */
	"WA0",			/* 0111 */
	"???",			/* 1000 */
	"???",			/* 1001 */
	"LOP",			/* 1010 */
	"???",			/* 1011 */
	"PC",           /* 1100 */ //???
	"???",			/* 1101 */
	"???",			/* 1110 */
	"???",			/* 1111 */
};

static const char *const cond_flags[] =
{
	"??", /* 0000 */
	"Z ", /* 0001 */
	"S ", /* 0010 */
	"ZS", /* 0011 */
	"C ", /* 0100 */
	"??", /* 0101 */
	"??", /* 0110 */
	"??", /* 0111 */
	"T0", /* 1000 */
	"??", /* 1001 */
	"??", /* 1010 */
	"??", /* 1011 */
	"??", /* 1100 */
	"??", /* 1101 */
	"??", /* 1110 */
	"??", /* 1111 */
};

/*****************************************************************************/

static char *output;
static const UINT32 *rombase;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static UINT32 fetch(void)
{
	return *rombase++;
}

static UINT8 add_table(UINT32 cur_opcode)
{
	UINT8 res = (cur_opcode & 0x00038000) >> 15;

	if(res == 0)
		res = 0;
	else
		res = 1 << (res-1);

	return res;
}

static UINT32 decode_opcode(UINT32 pc, const SCUDSP_OPCODE *op_table,UINT32 cur_opcode)
{
	INT8 rel8;
	UINT32 imm32;
	UINT8 op2;
	UINT32 flags = 0;

	//if (!strcmp(op_table->mnemonic, "jsr") || !strcmp(op_table->mnemonic, "bsr"))
	//  flags = DASMFLAG_STEP_OVER;
	//else if (!strcmp(op_table->mnemonic, "rts") || !strcmp(op_table->mnemonic, "rti"))
	//  flags = DASMFLAG_STEP_OUT;

	print("%s ", op_table->mnemonic);

	switch(op_table->address_mode_1)
	{
		case EA_ALU:       print("ALU "); break;
		case EA_IMM8:      print("%02X ",cur_opcode & 0xff); break;
		case EA_IMM18:     print("%08X ",cur_opcode & 0x7ffff); break;
		case EA_IMM25:     print("%08X ",cur_opcode & 0x1ffffff); break;
		case EA_MUL:       print("MUL "); break;
		case EA_SRCMEMX:   print("%s ", src_mem[(cur_opcode & 0x00700000) >> 20]); break;
		case EA_SRCMEMY:   print("%s ", src_mem[(cur_opcode & 0x0001c000) >> 14]); break;
		case EA_SRCMEMD1:  print("%s ", src_mem[(cur_opcode & 0x0000000f) >> 0]); break;
		case EA_D0:        print("%d D0 ",add_table(cur_opcode)); break;
		case EA_DMASRCMEM: print("%d %s ",add_table(cur_opcode),src_mem[(cur_opcode & 0x00000300) >> 8]); break;

		default:
			break;
	}

	switch(op_table->address_mode_2)
	{
		case EA_A:		   print("A"); break;
		case EA_P:		   print("P"); break;
		case EA_X:		   print("X"); break;
		case EA_Y:		   print("Y"); break;
		case EA_DSTMEM:    print("%s ", dst_mem[(cur_opcode & 0x00000f00) >> 8]); break;
		case EA_DMADSTMEM: print("%s ", dst_mem[(cur_opcode & 0x00000300) >> 8]); break;
		case EA_MVIDSTMEM: print("%s ", mvi_dst_mem[(cur_opcode & 0x3c000000) >> 26]); break;
		case EA_D0:        print("D0 "); break;

		default:
			break;
	}

	switch(op_table->address_mode_3)
	{
		case EA_IMM8:     print("%02X ",cur_opcode & 0xff); break;
		case EA_FLAGS:
			if(!((cur_opcode >> 19) & 0x20))
				print("N");
			print("%s ", cond_flags[(cur_opcode & 0x0780000) >> 19]);
			break;

		default:
			break;
	}

	return flags;
}

CPU_DISASSEMBLE( scudsp )
{
	UINT32 flags = 0;
	UINT8 opcode;

	output = buffer;
	rombase = oprom;

	opcode = fetch();
	switch((opcode & 0xc0000000) >> 30)
	{
		case 0: // operation
			flags =  decode_opcode(pc, &alu_table  [(opcode & 0x3c000000) >> 26],opcode);
			flags |= decode_opcode(pc, &xbus_table [(opcode & 0x03800000) >> 23],opcode);
			flags |= decode_opcode(pc, &ybus_table [(opcode & 0x000e0000) >> 17],opcode);
			flags |= decode_opcode(pc, &d1bus_table[(opcode & 0x00003000) >> 12],opcode);
			break;
		case 1: // unknown
			print("???");
			flags = 0;
			break;
		case 2: // move immediate
			flags = decode_opcode(pc,  &mvi_table  [(opcode & 0x02000000) >> 25],opcode);
			break;
		case 3: // control
			switch((opcode & 0x30000000) >> 28)
			{
				case 0:
					flags = decode_opcode(pc,  &dma_table  [(opcode & 0x7000) >> 12],opcode);
					break;
				case 1:
					flags = decode_opcode(pc,  &jmp_table  [(opcode & 0x2000000) >> 25],opcode);
					break;
				case 2:
					flags = decode_opcode(pc,  &loop_table [(opcode & 0x8000000) >> 27],opcode);
					break;
				case 3:
					flags = decode_opcode(pc,  &end_table  [(opcode & 0x8000000) >> 27],opcode);
					break;
			}
			break;
	}

	return (rombase-oprom) | flags | DASMFLAG_SUPPORTED;
}

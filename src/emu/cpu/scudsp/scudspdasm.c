/*
	Sega Saturn SCU DSP disassembler

	Written by Angelo Salese
*/

#include "emu.h"

enum
{
	EA_PDST = 1,
	EA_XDST,
	EA_ADST,
	EA_YDST,
};

typedef struct {
	char mnemonic[32];
	int address_mode;
} SCUDSP_OPCODE;

static const SCUDSP_OPCODE alu_table[16] =
{
	{ "NOP", 0, }, /* 0000 */
	{ "AND", 0, }, /* 0001 */
	{ "OR ", 0, }, /* 0010 */
	{ "XOR", 0, }, /* 0011 */
	{ "ADD", 0, }, /* 0100 */
	{ "SUB", 0, }, /* 0101 */
	{ "AD2", 0, }, /* 0110 */
	{ "???", 0, }, /* 0111 */
	{ "SR ", 0, }, /* 1000 */
	{ "RR ", 0, }, /* 1001 */
	{ "SL ", 0, }, /* 1010 */
	{ "RL ", 0, }, /* 1011 */
	{ "???", 0, }, /* 1100 */
	{ "???", 0, }, /* 1101 */
	{ "???", 0, }, /* 1110 */
	{ "RL8", 0, }, /* 1111 */
};

static const SCUDSP_OPCODE xbus_table[] =
{
	{ "NOP", 0, },			/* 000 */
	{ "???", 0, },			/* 001 */
	{ "MOV MUL,P", 0, },	/* 010 */
	{ "MOV", EA_PDST },		/* 011 */ //MOV %s,P
	{ "MOV", EA_XDST },		/* 100 */ //MOV %s,X
	{ "???", 0, },			/* 101 */
	{ "???", 0, },			/* 110 */
	{ "???", 0, },			/* 111 */
};

static const SCUDSP_OPCODE ybus_table[] =
{
	{ "NOP", 0, },			/* 000 */
	{ "CLR A", 0, },		/* 001 */
	{ "MOV ALU,A", 0, },	/* 010 */
	{ "MOV", EA_ADST, },	/* 011 */ //MOV %s,A
	{ "MOV", EA_YDST, },	/* 100 */ //MOV %s,Y
	{ "???", 0, },			/* 101 */
	{ "???", 0, },			/* 110 */
	{ "???", 0, },			/* 111 */
};

static const SCUDSP_OPCODE ybus_table[] =
{
	{ "NOP", 0, },			/* 000 */
	{ "MOV", 0, },			/* 001 */ //MOV %I8,%d
	{ "???", 0, },			/* 010 */
	{ "MOV", 0, },			/* 011 */ //MOV %S,%d
};


static const src_mem[] =
{
	"M0",			/* 000 */
	"M1",			/* 001 */
	"M2",			/* 010 */
	"M3",			/* 011 */
	"MC0",			/* 100 */
	"MC1",			/* 101 */
	"MC2",			/* 110 */
	"MC3",			/* 111 */
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

static UINT32 decode_opcode(UINT32 pc, const SCUDSP_OPCODE *op_table,UINT32 cur_opcode)
{
	INT8 rel8;
	UINT32 imm32;
	UINT8 op2;
	UINT32 flags = 0;

	//if (!strcmp(op_table->mnemonic, "jsr") || !strcmp(op_table->mnemonic, "bsr"))
	//	flags = DASMFLAG_STEP_OVER;
	//else if (!strcmp(op_table->mnemonic, "rts") || !strcmp(op_table->mnemonic, "rti"))
	//	flags = DASMFLAG_STEP_OUT;

	switch(op_table->address_mode)
	{
		case EA_PDST:
			print("%s %s,P", op_table->mnemonic, src_mem[(cur_opcode & 0x00700000) >> 20]);
			break;

		case EA_XDST:
			print("%s %s,X", op_table->mnemonic, src_mem[(cur_opcode & 0x00700000) >> 20]);
			break;

		case EA_ADST:
			print("%s %s,A", op_table->mnemonic, src_mem[(cur_opcode & 0x0001c000) >> 14]);
			break;

		case EA_YDST:
			print("%s %s,Y", op_table->mnemonic, src_mem[(cur_opcode & 0x0001c000) >> 14]);
			break;


		#if 0
		case EA_IMM8:
			imm8 = fetch();
			print("%s 0x%02X", op_table->mnemonic, imm8);
			break;

		case EA_IMM16:
			imm16 = fetch16();
			print("%s 0x%04X", op_table->mnemonic, imm16);
			break;

		case EA_DIRECT:
			imm8 = fetch();
			print("%s (0x%04X)", op_table->mnemonic, imm8);
			break;

		case EA_EXT:
			imm16 = fetch16();
			print("%s (0x%04X)", op_table->mnemonic, imm16);
			break;

		case EA_IND_X:
			imm8 = fetch();
			print("%s (X+0x%02X)", op_table->mnemonic, imm8);
			break;

		case EA_REL:
			rel8 = fetch();
			print("%s [0x%04X]", op_table->mnemonic, pc+2+rel8);
			break;

		case EA_DIRECT_IMM8:
			imm8 = fetch();
			mask = fetch();
			print("%s (0x%04X), 0x%02X", op_table->mnemonic, imm8, mask);
			break;

		case EA_IND_X_IMM8:
			imm8 = fetch();
			mask = fetch();
			print("%s (X+0x%02X), 0x%02X", op_table->mnemonic, imm8, mask);
			break;

		case EA_DIRECT_IMM8_REL:
			imm8 = fetch();
			mask = fetch();
			rel8 = fetch();
			print("%s (0x%04X), 0x%02X, [0x%04X]", op_table->mnemonic, imm8, mask, pc+4+rel8);
			break;

		case EA_IND_X_IMM8_REL:
			imm8 = fetch();
			mask = fetch();
			rel8 = fetch();
			print("%s (X+0x%02X), 0x%02X, [0x%04X]", op_table->mnemonic, imm8, mask, pc+4+rel8);
			break;

		case EA_IND_Y:
			imm8 = fetch();
			print("%s (Y+0x%02X)", op_table->mnemonic, imm8);
			break;

		case EA_IND_Y_IMM8:
			imm8 = fetch();
			mask = fetch();
			print("%s (Y+0x%02X), 0x%02X", op_table->mnemonic, imm8, mask);
			break;

		case EA_IND_Y_IMM8_REL:
			imm8 = fetch();
			mask = fetch();
			rel8 = fetch();
			print("%s (Y+0x%02X), 0x%02X, [0x%04X]", op_table->mnemonic, imm8, mask, pc+2+rel8);
			break;

		case PAGE2:
			op2 = fetch();
			return decode_opcode(pc, &opcode_table_page2[op2]);

		case PAGE3:
			op2 = fetch();
			return decode_opcode(pc, &opcode_table_page3[op2]);

		case PAGE4:
			op2 = fetch();
			return decode_opcode(pc, &opcode_table_page4[op2]);
		#endif

		default:
			print("%s", op_table->mnemonic);
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
	flags = decode_opcode(pc,  &alu_table  [(opcode & 0x3c000000) >> 26],opcode);
	flags |= decode_opcode(pc, &xbus_table [(opcode & 0x03800000) >> 23],opcode);
	flags |= decode_opcode(pc, &ybus_table [(opcode & 0x000e0000) >> 17],opcode);
	flags |= decode_opcode(pc, &d1bus_table[(opcode & 0x00003000) >> 12],opcode);

	return (rombase-oprom) | flags | DASMFLAG_SUPPORTED;
}

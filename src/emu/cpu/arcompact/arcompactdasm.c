/*********************************\

 ARCompact disassembler

\*********************************/

#include "emu.h"
#include <stdarg.h>

static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	vsprintf(output, fmt, vl);
	va_end(vl);
}

/*****************************************************************************/



/*****************************************************************************/


static const char *basic[0x20] =
{
	/* opcode below are 32-bit mode */
	/* 00 */ "Bcc",
	/* 01 */ "BLcc/BRcc",
	/* 02 */ "LD r+o",
	/* 03 */ "ST r+o",
	/* 04 */ "op a,b,c (basecase)", // basecase ops
	/* 05 */ "op a,b,c (05 ARC ext)", // ARC processor specific extensions
	/* 06 */ "op a,b,c (06 ARC ext)",
	/* 07 */ "op a,b,c (07 User ext)", // User speciifc extensions
	/* 08 */ "op a,b,c (08 User ext)",
	/* 09 */ "op a,b,c (09 Market ext)", // Market specific extensions
	/* 0a */ "op a,b,c (0a Market ext)",
	/* 0b */ "op a,b,c (0b Market ext)",
	/* opcodes below are 16-bit mode */
	/* 0c */ "Load/Add reg-reg",
	/* 0d */ "Add/Sub/Shft imm",
	/* 0e */ "Mov/Cmp/Add",
	/* 0f */ "op_S b,b,c", // single ops
	/* 10 */ "LD_S",
	/* 11 */ "LDB_S",
	/* 12 */ "LDW_S",
	/* 13 */ "LSW_S.X",
	/* 14 */ "ST_S",
	/* 15 */ "STB_S",
	/* 16 */ "STW_S",
	/* 17 */ "Shift/Sub/Bit",
	/* 18 */ "Stack Instr",
	/* 19 */ "GP Instr",
	/* 1a */ "PCL Instr",
	/* 1b */ "MOV_S",
	/* 1c */ "ADD_S/CMP_S",
	/* 1d */ "BRcc_S",
	/* 1e */ "Bcc_S",
	/* 1f */ "BL_S"
};

// condition codes (basic ones are the same as arc 
static const char *conditions[0x20] =
{
	/* 00 */ "AL", // (aka RA         - Always)
	/* 01 */ "EQ", // (aka Z          - Zero
	/* 02 */ "NE", // (aka NZ         - Non-Zero)
	/* 03 */ "PL", // (aka P          - Positive)
	/* 04 */ "MI", // (aka N          - Negative)
	/* 05 */ "CS", // (aka C,  LO     - Carry set / Lower than) (unsigned)
	/* 06 */ "CC", // (aka CC, NC, HS - Carry Clear / Higher or Same) (unsigned) 
	/* 07 */ "VS", // (aka V          - Overflow set)
	/* 08 */ "VC", // (aka NV         - Overflow clear)
	/* 09 */ "GT", // (               - Greater than) (signed)
	/* 0a */ "GE", // (               - Greater than or Equal) (signed)
	/* 0b */ "LT", // (               - Less than) (signed)
	/* 0c */ "LE", // (               - Less than or Equal) (signed)
	/* 0d */ "HI", // (               - Higher than) (unsigned)
	/* 0e */ "LS", // (               - Lower or Same) (unsigned)
	/* 0f */ "PNZ",// (               - Positive non-0 value)
	/* 10 */ "0x10 Reserved", // possible CPU implementation specifics
	/* 11 */ "0x11 Reserved",
	/* 12 */ "0x12 Reserved",
	/* 13 */ "0x13 Reserved",
	/* 14 */ "0x14 Reserved",
	/* 15 */ "0x15 Reserved",
	/* 16 */ "0x16 Reserved",
	/* 17 */ "0x17 Reserved",
	/* 18 */ "0x18 Reserved",
	/* 19 */ "0x19 Reserved",
	/* 1a */ "0x1a Reserved",
	/* 1b */ "0x1b Reserved",
	/* 1c */ "0x1c Reserved",
	/* 1d */ "0x1d Reserved",
	/* 1e */ "0x1e Reserved",
	/* 1f */ "0x1f Reserved"
};

#define ARCOMPACT_OPERATION ((op & 0xf800) >> 11)

CPU_DISASSEMBLE(arcompact)
{
	int size = 2;

	UINT32 op = oprom[0] | (oprom[1] << 8);
	output = buffer;

	UINT8 instruction = ARCOMPACT_OPERATION;

	if (instruction < 0x0c)
	{
		size = 4;
		op <<= 16;
		op |= oprom[2] | (oprom[3] << 8);

		switch (instruction)
		{
			case 0x00:
				if (op & 0x00010000)
				{ // Branch Unconditionally Far
				  // 00000 ssssssssss 1  SSSSSSSSSS N 0 TTTT
					UINT32 address =   (op & 0x07fe0000) >> 17;
					address |=        ((op & 0x0000ffc0) >> 6) << 10;
					address |=        ((op & 0x0000000f) >> 0) << 20;

					print("B %08x (%08x)",  address<<1, op & ~0xffffffcf );
				}
				else
				{ // Branch Conditionally
				  // 00000 ssssssssss 0 SSSSSSSSSS N QQQQQ
					UINT32 address =   (op & 0x07fe0000) >> 17;
					address |=        ((op & 0x0000ffc0) >> 6) << 10;

					UINT8 condition = op & 0x0000001f;

					print("B(%s) %08x (%08x)", conditions[condition], address<<1, op & ~0xffffffdf );

				}

				break;

			default:
				print("%s (%08x)", basic[instruction], op & ~0xf8000000 );
				break;

		}

		
	}
	else
	{
		size = 2;
		print("%s (%04x)", basic[instruction], op & ~0xf800 );
	}


	return size | DASMFLAG_SUPPORTED;
}

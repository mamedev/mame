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

#define DASM_OPS_16 char *output, offs_t pc, UINT16 op, const UINT8* oprom
#define DASM_OPS_32 char *output, offs_t pc, UINT32 op, const UINT8* oprom

int arcompact_handle18_00_dasm(DASM_OPS_16);
int arcompact_handle18_01_dasm(DASM_OPS_16);
int arcompact_handle18_02_dasm(DASM_OPS_16);
int arcompact_handle18_03_dasm(DASM_OPS_16);
int arcompact_handle18_04_dasm(DASM_OPS_16);

int arcompact_handle18_05_dasm(DASM_OPS_16);
int arcompact_handle18_05_00_dasm(DASM_OPS_16);
int arcompact_handle18_05_01_dasm(DASM_OPS_16);
int arcompact_handle18_05_02_dasm(DASM_OPS_16);
int arcompact_handle18_05_03_dasm(DASM_OPS_16);
int arcompact_handle18_05_04_dasm(DASM_OPS_16);
int arcompact_handle18_05_05_dasm(DASM_OPS_16);
int arcompact_handle18_05_06_dasm(DASM_OPS_16);
int arcompact_handle18_05_07_dasm(DASM_OPS_16);

int arcompact_handle18_06_dasm(DASM_OPS_16);
int arcompact_handle18_06_00_dasm(DASM_OPS_16);
int arcompact_handle18_06_01_dasm(DASM_OPS_16);
int arcompact_handle18_06_02_dasm(DASM_OPS_16);
int arcompact_handle18_06_03_dasm(DASM_OPS_16);
int arcompact_handle18_06_04_dasm(DASM_OPS_16);
int arcompact_handle18_06_05_dasm(DASM_OPS_16);
int arcompact_handle18_06_06_dasm(DASM_OPS_16);
int arcompact_handle18_06_07_dasm(DASM_OPS_16);
int arcompact_handle18_06_08_dasm(DASM_OPS_16);
int arcompact_handle18_06_09_dasm(DASM_OPS_16);
int arcompact_handle18_06_0a_dasm(DASM_OPS_16);
int arcompact_handle18_06_0b_dasm(DASM_OPS_16);
int arcompact_handle18_06_0c_dasm(DASM_OPS_16);
int arcompact_handle18_06_0d_dasm(DASM_OPS_16);
int arcompact_handle18_06_0e_dasm(DASM_OPS_16);
int arcompact_handle18_06_0f_dasm(DASM_OPS_16);
int arcompact_handle18_06_10_dasm(DASM_OPS_16);
int arcompact_handle18_06_11_dasm(DASM_OPS_16);
int arcompact_handle18_06_12_dasm(DASM_OPS_16);
int arcompact_handle18_06_13_dasm(DASM_OPS_16);
int arcompact_handle18_06_14_dasm(DASM_OPS_16);
int arcompact_handle18_06_15_dasm(DASM_OPS_16);
int arcompact_handle18_06_16_dasm(DASM_OPS_16);
int arcompact_handle18_06_17_dasm(DASM_OPS_16);
int arcompact_handle18_06_18_dasm(DASM_OPS_16);
int arcompact_handle18_06_19_dasm(DASM_OPS_16);
int arcompact_handle18_06_1a_dasm(DASM_OPS_16);
int arcompact_handle18_06_1b_dasm(DASM_OPS_16);
int arcompact_handle18_06_1c_dasm(DASM_OPS_16);
int arcompact_handle18_06_1d_dasm(DASM_OPS_16);
int arcompact_handle18_06_1e_dasm(DASM_OPS_16);
int arcompact_handle18_06_1f_dasm(DASM_OPS_16);

int arcompact_handle18_07_dasm(DASM_OPS_16);
int arcompact_handle18_07_00_dasm(DASM_OPS_16);
int arcompact_handle18_07_01_dasm(DASM_OPS_16);
int arcompact_handle18_07_02_dasm(DASM_OPS_16);
int arcompact_handle18_07_03_dasm(DASM_OPS_16);
int arcompact_handle18_07_04_dasm(DASM_OPS_16);
int arcompact_handle18_07_05_dasm(DASM_OPS_16);
int arcompact_handle18_07_06_dasm(DASM_OPS_16);
int arcompact_handle18_07_07_dasm(DASM_OPS_16);
int arcompact_handle18_07_08_dasm(DASM_OPS_16);
int arcompact_handle18_07_09_dasm(DASM_OPS_16);
int arcompact_handle18_07_0a_dasm(DASM_OPS_16);
int arcompact_handle18_07_0b_dasm(DASM_OPS_16);
int arcompact_handle18_07_0c_dasm(DASM_OPS_16);
int arcompact_handle18_07_0d_dasm(DASM_OPS_16);
int arcompact_handle18_07_0e_dasm(DASM_OPS_16);
int arcompact_handle18_07_0f_dasm(DASM_OPS_16);
int arcompact_handle18_07_10_dasm(DASM_OPS_16);
int arcompact_handle18_07_11_dasm(DASM_OPS_16);
int arcompact_handle18_07_12_dasm(DASM_OPS_16);
int arcompact_handle18_07_13_dasm(DASM_OPS_16);
int arcompact_handle18_07_14_dasm(DASM_OPS_16);
int arcompact_handle18_07_15_dasm(DASM_OPS_16);
int arcompact_handle18_07_16_dasm(DASM_OPS_16);
int arcompact_handle18_07_17_dasm(DASM_OPS_16);
int arcompact_handle18_07_18_dasm(DASM_OPS_16);
int arcompact_handle18_07_19_dasm(DASM_OPS_16);
int arcompact_handle18_07_1a_dasm(DASM_OPS_16);
int arcompact_handle18_07_1b_dasm(DASM_OPS_16);
int arcompact_handle18_07_1c_dasm(DASM_OPS_16);
int arcompact_handle18_07_1d_dasm(DASM_OPS_16);
int arcompact_handle18_07_1e_dasm(DASM_OPS_16);
int arcompact_handle18_07_1f_dasm(DASM_OPS_16);



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

static const char *table01_01_0x[0x10] =
{
	/* 00 */ "BREQ",
	/* 01 */ "BRNE",
	/* 02 */ "BRLT",
	/* 03 */ "BRGE",
	/* 04 */ "BRLO",
	/* 05 */ "BRHS",
	/* 06 */ "<reserved>",
	/* 07 */ "<reserved>",
	/* 08 */ "<reserved>",
	/* 09 */ "<reserved>",
	/* 0a */ "<reserved>",
	/* 0b */ "<reserved>",
	/* 0c */ "<reserved>",
	/* 0d */ "<reserved>",
	/* 0e */ "<BBIT0>",
	/* 0f */ "<BBIT1>"
};





static const char *table0f[0x20] =
{
	/* 00 */ "SOPs", // Sub Operation (another table..) ( table0f_00 )
	/* 01 */ "0x01 <illegal>",
	/* 02 */ "SUB_S",
	/* 03 */ "0x03 <illegal>",
	/* 04 */ "AND_S",
	/* 05 */ "OR_S",
	/* 06 */ "BIC_S",
	/* 07 */ "XOR_S",
	/* 08 */ "0x08 <illegal>",
	/* 09 */ "0x09 <illegal>",
	/* 0a */ "0x0a <illegal>",
	/* 0b */ "TST_S",
	/* 0c */ "MUL64_S",
	/* 0d */ "SEXB_S",
	/* 0e */ "SEXW_S",
	/* 0f */ "EXTB_S",
	/* 10 */ "EXTW_S",
	/* 11 */ "ABS_S",
	/* 12 */ "NOT_S",
	/* 13 */ "NEG_S",
	/* 14 */ "ADD1_S",
	/* 15 */ "ADD2_S>",
	/* 16 */ "ADD3_S",
	/* 17 */ "0x17 <illegal>",
	/* 18 */ "ASL_S (multiple)",
	/* 19 */ "LSR_S (multiple)",
	/* 1a */ "ASR_S (multiple)",
	/* 1b */ "ASL_S (single)",
	/* 1c */ "LSR_S (single)",
	/* 1d */ "ASR_S (single)",
	/* 1e */ "TRAP (not a5?)",
	/* 1f */ "BRK_S" // 0x7fff only?
};

static const char *table0f_00[0x8] =
{
	/* 00 */ "J_S",
	/* 01 */ "J_S.D",
	/* 02 */ "JL_S",
	/* 03 */ "JL_S.D",
	/* 04 */ "0x04 <illegal>",
	/* 05 */ "0x05 <illegal>",
	/* 06 */ "SUB_S.NE",
	/* 07 */ "ZOPs", // Sub Operations (yet another table..) ( table0f_00_07 )
};

static const char *table0f_00_07[0x8] =
{
	/* 00 */ "NOP_S",
	/* 01 */ "UNIMP_S", // unimplemented (not a5?)
	/* 02 */ "0x02 <illegal>",
	/* 03 */ "0x03 <illegal>",
	/* 04 */ "JEQ_S [BLINK]",
	/* 05 */ "JNE_S [BLINK]",
	/* 06 */ "J_S [BLINK]",
	/* 07 */ "J_S.D [BLINK]",
};

#define ARCOMPACT_OPERATION ((op & 0xf800) >> 11)


int arcompact_handle00_dasm(DASM_OPS_32)
{
	if (op & 0x00010000)
	{ // Branch Unconditionally Far
		// 00000 ssssssssss 1  SSSSSSSSSS N R TTTT
		INT32 address =   (op & 0x07fe0000) >> 17;
		address |=        ((op & 0x0000ffc0) >> 6) << 10;
		address |=        ((op & 0x0000000f) >> 0) << 20;
		if (address & 0x800000) address = -(address&0x7fffff);	

		print("B %08x (%08x)",  pc + (address *2) + 2, op & ~0xffffffcf );
	}
	else
	{ // Branch Conditionally
		// 00000 ssssssssss 0 SSSSSSSSSS N QQQQQ
		INT32 address =   (op & 0x07fe0000) >> 17;
		address |=        ((op & 0x0000ffc0) >> 6) << 10;
		if (address & 0x800000) address = -(address&0x7fffff);	

		UINT8 condition = op & 0x0000001f;

		print("B(%s) %08x (%08x)", conditions[condition], pc + (address *2) + 2, op & ~0xffffffdf );
	}
	return 4;
}

int arcompact_handle01_dasm(DASM_OPS_32)
{
	if (op & 0x00010000)
	{
		if (op & 0x00000010)
		{ // Branch on Compare / Bit Test - Register-Immediate
			// 00001 bbb sssssss 1 S BBB UUUUUU N 1 iiii
			UINT8 subinstr = op & 0x0000000f;
			INT32 address =    (op & 0x00fe0000) >> 17;
			address |=        ((op & 0x00008000) >> 15) << 7;
			if (address & 0x80) address = -(address&0x7f);	

						
			print("%s (reg-imm) %08x (%08x)", table01_01_0x[subinstr], pc + (address *2) + 4, op & ~0xf8fe800f);


		}
		else
		{
			// Branch on Compare / Bit Test - Register-Register
			// 00001 bbb sssssss 1 S BBB CCCCCC N 0 iiii
			UINT8 subinstr = op & 0x0000000f;
			INT32 address =    (op & 0x00fe0000) >> 17;
			address |=        ((op & 0x00008000) >> 15) << 7;
			if (address & 0x80) address = -(address&0x7f);	

			print("%s (reg-reg) %08x (%08x)", table01_01_0x[subinstr], pc + (address *2) + 4, op & ~0xf8fe800f);

		}

	}
	else
	{
		if (op & 0x00020000)
		{ // Branch and Link Unconditionally Far
			// 00001 sssssssss 10  SSSSSSSSSS N R TTTT
			INT32 address =   (op & 0x07fc0000) >> 17;
			address |=        ((op & 0x0000ffc0) >> 6) << 10;
			address |=        ((op & 0x0000000f) >> 0) << 20;
			if (address & 0x800000) address = -(address&0x7fffff);	

			print("BL %08x (%08x)",  pc + (address *2) + 2, op & ~0xffffffcf );
		}
		else
		{ // Branch and Link Conditionally
			// 00001 sssssssss 00 SSSSSSSSSS N QQQQQ
			INT32 address =   (op & 0x07fc0000) >> 17;
			address |=        ((op & 0x0000ffc0) >> 6) << 10;
			if (address & 0x800000) address = -(address&0x7fffff);	

			UINT8 condition = op & 0x0000001f;

			print("BL(%s) %08x (%08x)", conditions[condition], pc + (address *2) + 2, op & ~0xffffffdf );

		}

	}
	return 4;
}

int arcompact_handle02_dasm(DASM_OPS_32)
{
	print("LD r+o (%08x)", op );
	return 4;
}

int arcompact_handle03_dasm(DASM_OPS_32)
{
	print("ST r+o (%08x)", op );
	return 4;
}

int arcompact_handle04_dasm(DASM_OPS_32)
{
	print("op a,b,c (basecase) (%08x)", op );
	return 4;
}

int arcompact_handle05_dasm(DASM_OPS_32)
{
	print("op a,b,c (05 ARC ext) (%08x)", op );
	return 4;
}

int arcompact_handle06_dasm(DASM_OPS_32)
{
	print("op a,b,c (06 ARC ext) (%08x)", op );
	return 4;
}

int arcompact_handle07_dasm(DASM_OPS_32)
{
	print("op a,b,c (07 User ext) (%08x)", op );
	return 4;
}

int arcompact_handle08_dasm(DASM_OPS_32)
{
	print("op a,b,c (08 User ext) (%08x)", op );
	return 4;
}

int arcompact_handle09_dasm(DASM_OPS_32)
{
	print("op a,b,c (09 Market ext) (%08x)", op );
	return 4;
}

int arcompact_handle0a_dasm(DASM_OPS_32)
{
	print("op a,b,c (0a Market ext) (%08x)",  op );
	return 4;
}

int arcompact_handle0b_dasm(DASM_OPS_32)
{
	print("op a,b,c (0b Market ext) (%08x)",  op );
	return 4;
}




int arcompact_handle0c_dasm(DASM_OPS_16)
{
	print("Load/Add reg-reg (%04x)", op);
	return 2;
}

int arcompact_handle0d_dasm(DASM_OPS_16)
{
	print("Add/Sub/Shft imm (%04x)", op);
	return 2;
}

int arcompact_handle0e_dasm(DASM_OPS_16)
{
	print("Mov/Cmp/Add (%04x)", op);
	return 2;
}

int arcompact_handle0f_dasm(DASM_OPS_16)
{
	// General Register Instructions (16-bit)
	// 01111 bbb ccc iiiii
	UINT8 subinstr = (op & 0x01f) >> 0;
	//print("%s (%04x)", table0f[subinstr], op & ~0xf81f);
		
	switch (subinstr)
	{
	
		default:
			print("%s (%04x)", table0f[subinstr], op & ~0xf81f);
			break;

		case 0x00:
		{
			// General Operations w/ Register
			// 01111 bbb iii 00000
			UINT8 subinstr2 = (op & 0x00e0) >> 5;

			switch (subinstr2)
			{
				default:
					print("%s (%04x)", table0f_00[subinstr2], op & ~0xf8ff);
					return 2;

				case 0x7:
				{
					// General Operations w/o Register
					// 01111 iii 111 00000
					UINT8 subinstr3 = (op & 0x0700) >> 8;

					print("%s (%04x)", table0f_00_07[subinstr3], op & ~0xffff);

					return 2;
				}
			}
		}
	}
	
	return 2;
}

int arcompact_handle10_dasm(DASM_OPS_16)
{
	print("LD_S (%04x)",  op);
	return 2;
}

int arcompact_handle11_dasm(DASM_OPS_16)
{
	print("LDB_S (%04x)", op);
	return 2;
}

int arcompact_handle12_dasm(DASM_OPS_16)
{
	print("LDW_S (%04x)", op);
	return 2;
}

int arcompact_handle13_dasm(DASM_OPS_16)
{
	print("LSW_S.X (%04x)", op);
	return 2;
}

int arcompact_handle14_dasm(DASM_OPS_16)
{
	print("ST_S (%04x)", op);
	return 2;
}

int arcompact_handle15_dasm(DASM_OPS_16)
{
	print("STB_S (%04x)", op);
	return 2;
}

int arcompact_handle16_dasm(DASM_OPS_16)
{
	print("STW_S (%04x)",  op);
	return 2;
}

int arcompact_handle17_dasm(DASM_OPS_16)
{
	print("Shift/Sub/Bit (%04x)",  op);
	return 2;
}

int arcompact_handle18_dasm(DASM_OPS_16)
{
	int size = 2;
	// Stack Pointer Based Instructions (16-bit)
	// 11000 bbb iii uuuuu
	UINT8 subinstr = (op & 0x00e0) >> 5;
	op &= ~0x00e0;

	switch (subinstr)
	{
		case 0x00: size = arcompact_handle18_00_dasm(output, pc, op, oprom); break; // LD_S (SP)
		case 0x01: size = arcompact_handle18_01_dasm(output, pc, op, oprom); break; // LDB_S (SP)
		case 0x02: size = arcompact_handle18_02_dasm(output, pc, op, oprom); break; // ST_S (SP)
		case 0x03: size = arcompact_handle18_03_dasm(output, pc, op, oprom); break; // STB_S (SP)
		case 0x04: size = arcompact_handle18_04_dasm(output, pc, op, oprom); break; // ADD_S (SP)
		case 0x05: size = arcompact_handle18_05_dasm(output, pc, op, oprom); break; // subtable 18_05
		case 0x06: size = arcompact_handle18_06_dasm(output, pc, op, oprom); break; // subtable 18_06
		case 0x07: size = arcompact_handle18_07_dasm(output, pc, op, oprom); break; // subtable 18_07
	}

	return size;
}

// op bits remaining for 0x18_xx subgroups 0x071f 

int arcompact_handle18_00_dasm(DASM_OPS_16) 
{
	print("LD_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_01_dasm(DASM_OPS_16) 
{
	print("LDB_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_02_dasm(DASM_OPS_16) 
{
	print("ST_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_03_dasm(DASM_OPS_16) 
{
	print("STB_S (SP) (%04x)",  op);
	return 2;
}

int arcompact_handle18_04_dasm(DASM_OPS_16) 
{
	print("ADD_S (SP) (%04x)",  op);
	return 2;
}





int arcompact_handle18_05_dasm(DASM_OPS_16) 
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x0700) >> 8;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle18_05_00_dasm(output, pc, op, oprom); break; // ADD_S (SP)
		case 0x01: size = arcompact_handle18_05_01_dasm(output, pc, op, oprom); break; // SUB_S (SP)
		case 0x02: size = arcompact_handle18_05_02_dasm(output, pc, op, oprom); break; // <illegal 0x18_05_02> 
		case 0x03: size = arcompact_handle18_05_03_dasm(output, pc, op, oprom); break; // <illegal 0x18_05_03>
		case 0x04: size = arcompact_handle18_05_04_dasm(output, pc, op, oprom); break; // <illegal 0x18_05_04>
		case 0x05: size = arcompact_handle18_05_05_dasm(output, pc, op, oprom); break; // <illegal 0x18_05_05>
		case 0x06: size = arcompact_handle18_05_06_dasm(output, pc, op, oprom); break; // <illegal 0x18_05_06>
		case 0x07: size = arcompact_handle18_05_07_dasm(output, pc, op, oprom); break; // <illegal 0x18_05_07>
	}

	return size;
}
// op bits remaining for 0x18_05_xx subgroups 0x001f
int arcompact_handle18_05_00_dasm(DASM_OPS_16)
{
	int u = op & 0x001f;
	op &= ~0x001f; // all bits now used

	print("ADD_S %02x (SP)", u);
	return 2;

}

int arcompact_handle18_05_01_dasm(DASM_OPS_16)
{
	int u = op & 0x001f;
	op &= ~0x001f; // all bits now used

	print("SUB_S %02x (SP)", u);
	return 2;
}


int arcompact_handle18_05_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_02> (%04x)", op); return 2;}
int arcompact_handle18_05_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_03> (%04x)", op); return 2;}
int arcompact_handle18_05_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_04> (%04x)", op); return 2;}
int arcompact_handle18_05_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_05> (%04x)", op); return 2;}
int arcompact_handle18_05_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_06> (%04x)", op); return 2;}
int arcompact_handle18_05_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_05_07> (%04x)", op); return 2;}


int arcompact_handle18_06_dasm(DASM_OPS_16) 
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x001f) >> 0;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle18_06_00_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_00>
		case 0x01: size = arcompact_handle18_06_01_dasm(output, pc, op, oprom); break; // POP_S b
		case 0x02: size = arcompact_handle18_06_02_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_02>
		case 0x03: size = arcompact_handle18_06_03_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_03>
		case 0x04: size = arcompact_handle18_06_04_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_04>
		case 0x05: size = arcompact_handle18_06_05_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_05>
		case 0x06: size = arcompact_handle18_06_06_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_06>
		case 0x07: size = arcompact_handle18_06_07_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_07>
		case 0x08: size = arcompact_handle18_06_08_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_08>
		case 0x09: size = arcompact_handle18_06_09_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_09>
		case 0x0a: size = arcompact_handle18_06_0a_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_0a>
		case 0x0b: size = arcompact_handle18_06_0b_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_0b>
		case 0x0c: size = arcompact_handle18_06_0c_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_0c>
		case 0x0d: size = arcompact_handle18_06_0d_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_0d>
		case 0x0e: size = arcompact_handle18_06_0e_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_0e>
		case 0x0f: size = arcompact_handle18_06_0f_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_0f>
		case 0x10: size = arcompact_handle18_06_10_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_10>
		case 0x11: size = arcompact_handle18_06_11_dasm(output, pc, op, oprom); break; // POP_S blink
		case 0x12: size = arcompact_handle18_06_12_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_12>
		case 0x13: size = arcompact_handle18_06_13_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_13>
		case 0x14: size = arcompact_handle18_06_14_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_14>
		case 0x15: size = arcompact_handle18_06_15_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_15>
		case 0x16: size = arcompact_handle18_06_16_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_16>
		case 0x17: size = arcompact_handle18_06_17_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_17>
		case 0x18: size = arcompact_handle18_06_18_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_18>
		case 0x19: size = arcompact_handle18_06_19_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_19>
		case 0x1a: size = arcompact_handle18_06_1a_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_1a>
		case 0x1b: size = arcompact_handle18_06_1b_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_1b>
		case 0x1c: size = arcompact_handle18_06_1c_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_1c>
		case 0x1d: size = arcompact_handle18_06_1d_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_1d>
		case 0x1e: size = arcompact_handle18_06_1e_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_1e>
		case 0x1f: size = arcompact_handle18_06_1f_dasm(output, pc, op, oprom); break; // <illegal 0x18_06_1f>
	}

	return size;
}


// op bits remaining for 0x18_06_xx subgroups 0x0700 
int arcompact_handle18_06_00_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_00> (%04x)",  op); return 2;}

int arcompact_handle18_06_01_dasm(DASM_OPS_16) 
{
	int b = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	print("POP_S [%02x]", b);

	return 2;
}

int arcompact_handle18_06_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_02> (%04x)", op); return 2;}
int arcompact_handle18_06_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_03> (%04x)", op); return 2;}
int arcompact_handle18_06_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_04> (%04x)", op); return 2;}
int arcompact_handle18_06_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_05> (%04x)", op); return 2;}
int arcompact_handle18_06_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_06> (%04x)", op); return 2;}
int arcompact_handle18_06_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_07> (%04x)", op); return 2;}
int arcompact_handle18_06_08_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_08> (%04x)", op); return 2;}
int arcompact_handle18_06_09_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_09> (%04x)", op); return 2;}
int arcompact_handle18_06_0a_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0a> (%04x)", op); return 2;}
int arcompact_handle18_06_0b_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0b> (%04x)", op); return 2;}
int arcompact_handle18_06_0c_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0c> (%04x)", op); return 2;}
int arcompact_handle18_06_0d_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0d> (%04x)", op); return 2;}
int arcompact_handle18_06_0e_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0e> (%04x)", op); return 2;}
int arcompact_handle18_06_0f_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_0f> (%04x)", op); return 2;}
int arcompact_handle18_06_10_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_10> (%04x)", op); return 2;}

int arcompact_handle18_06_11_dasm(DASM_OPS_16) 
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		print("POP_S [BLINK] (Reserved Bits set %04x)", op);
	else
		print("POP_S [BLINK]");

	return 2;
}

int arcompact_handle18_06_12_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_12> (%04x)",  op); return 2;}
int arcompact_handle18_06_13_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_13> (%04x)",  op); return 2;}
int arcompact_handle18_06_14_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_14> (%04x)",  op); return 2;}
int arcompact_handle18_06_15_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_15> (%04x)",  op); return 2;}
int arcompact_handle18_06_16_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_16> (%04x)",  op); return 2;}
int arcompact_handle18_06_17_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_17> (%04x)",  op); return 2;}
int arcompact_handle18_06_18_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_18> (%04x)",  op); return 2;}
int arcompact_handle18_06_19_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_19> (%04x)",  op); return 2;}
int arcompact_handle18_06_1a_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1a> (%04x)",  op); return 2;}
int arcompact_handle18_06_1b_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1b> (%04x)",  op); return 2;}
int arcompact_handle18_06_1c_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1c> (%04x)",  op); return 2;}
int arcompact_handle18_06_1d_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1d> (%04x)",  op); return 2;}
int arcompact_handle18_06_1e_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1e> (%04x)",  op); return 2;}
int arcompact_handle18_06_1f_dasm(DASM_OPS_16)  { print("<illegal 0x18_06_1f> (%04x)",  op); return 2;}




int arcompact_handle18_07_dasm(DASM_OPS_16) 
{
	int size = 2;
	UINT8 subinstr2 = (op & 0x001f) >> 0;
	op &= ~0x001f;

	switch (subinstr2)
	{
		case 0x00: size = arcompact_handle18_07_00_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_00>
		case 0x01: size = arcompact_handle18_07_01_dasm(output, pc, op, oprom); break; // PUSH_S b
		case 0x02: size = arcompact_handle18_07_02_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_02>
		case 0x03: size = arcompact_handle18_07_03_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_03>
		case 0x04: size = arcompact_handle18_07_04_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_04>
		case 0x05: size = arcompact_handle18_07_05_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_05>
		case 0x06: size = arcompact_handle18_07_06_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_06>
		case 0x07: size = arcompact_handle18_07_07_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_07>
		case 0x08: size = arcompact_handle18_07_08_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_08>
		case 0x09: size = arcompact_handle18_07_09_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_09>
		case 0x0a: size = arcompact_handle18_07_0a_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_0a>
		case 0x0b: size = arcompact_handle18_07_0b_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_0b>
		case 0x0c: size = arcompact_handle18_07_0c_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_0c>
		case 0x0d: size = arcompact_handle18_07_0d_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_0d>
		case 0x0e: size = arcompact_handle18_07_0e_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_0e>
		case 0x0f: size = arcompact_handle18_07_0f_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_0f>
		case 0x10: size = arcompact_handle18_07_10_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_10>
		case 0x11: size = arcompact_handle18_07_11_dasm(output, pc, op, oprom); break; // PUSH_S blink
		case 0x12: size = arcompact_handle18_07_12_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_12>
		case 0x13: size = arcompact_handle18_07_13_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_13>
		case 0x14: size = arcompact_handle18_07_14_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_14>
		case 0x15: size = arcompact_handle18_07_15_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_15>
		case 0x16: size = arcompact_handle18_07_16_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_16>
		case 0x17: size = arcompact_handle18_07_17_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_17>
		case 0x18: size = arcompact_handle18_07_18_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_18>
		case 0x19: size = arcompact_handle18_07_19_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_19>
		case 0x1a: size = arcompact_handle18_07_1a_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_1a>
		case 0x1b: size = arcompact_handle18_07_1b_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_1b>
		case 0x1c: size = arcompact_handle18_07_1c_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_1c>
		case 0x1d: size = arcompact_handle18_07_1d_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_1d>
		case 0x1e: size = arcompact_handle18_07_1e_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_1e>
		case 0x1f: size = arcompact_handle18_07_1f_dasm(output, pc, op, oprom); break; // <illegal 0x18_07_1f>
	}

	return size;
}


// op bits remaining for 0x18_07_xx subgroups 0x0700 
int arcompact_handle18_07_00_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_00> (%04x)",  op); return 2;}

int arcompact_handle18_07_01_dasm(DASM_OPS_16) 
{
	int b = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	print("PUSH_S [%02x]", b);

	return 2;
}

int arcompact_handle18_07_02_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_02> (%04x)", op); return 2;}
int arcompact_handle18_07_03_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_03> (%04x)", op); return 2;}
int arcompact_handle18_07_04_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_04> (%04x)", op); return 2;}
int arcompact_handle18_07_05_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_05> (%04x)", op); return 2;}
int arcompact_handle18_07_06_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_06> (%04x)", op); return 2;}
int arcompact_handle18_07_07_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_07> (%04x)", op); return 2;}
int arcompact_handle18_07_08_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_08> (%04x)", op); return 2;}
int arcompact_handle18_07_09_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_09> (%04x)", op); return 2;}
int arcompact_handle18_07_0a_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0a> (%04x)", op); return 2;}
int arcompact_handle18_07_0b_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0b> (%04x)", op); return 2;}
int arcompact_handle18_07_0c_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0c> (%04x)", op); return 2;}
int arcompact_handle18_07_0d_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0d> (%04x)", op); return 2;}
int arcompact_handle18_07_0e_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0e> (%04x)", op); return 2;}
int arcompact_handle18_07_0f_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_0f> (%04x)", op); return 2;}
int arcompact_handle18_07_10_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_10> (%04x)", op); return 2;}

int arcompact_handle18_07_11_dasm(DASM_OPS_16) 
{
	int res = (op & 0x0700) >> 8;
	op &= ~0x0700; // all bits now used

	if (res)
		print("PUSH_S [BLINK] (Reserved Bits set %04x)", op);
	else
		print("PUSH_S [BLINK]");

	return 2;
}

int arcompact_handle18_07_12_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_12> (%04x)",  op); return 2;}
int arcompact_handle18_07_13_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_13> (%04x)",  op); return 2;}
int arcompact_handle18_07_14_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_14> (%04x)",  op); return 2;}
int arcompact_handle18_07_15_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_15> (%04x)",  op); return 2;}
int arcompact_handle18_07_16_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_16> (%04x)",  op); return 2;}
int arcompact_handle18_07_17_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_17> (%04x)",  op); return 2;}
int arcompact_handle18_07_18_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_18> (%04x)",  op); return 2;}
int arcompact_handle18_07_19_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_19> (%04x)",  op); return 2;}
int arcompact_handle18_07_1a_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1a> (%04x)",  op); return 2;}
int arcompact_handle18_07_1b_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1b> (%04x)",  op); return 2;}
int arcompact_handle18_07_1c_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1c> (%04x)",  op); return 2;}
int arcompact_handle18_07_1d_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1d> (%04x)",  op); return 2;}
int arcompact_handle18_07_1e_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1e> (%04x)",  op); return 2;}
int arcompact_handle18_07_1f_dasm(DASM_OPS_16)  { print("<illegal 0x18_07_1f> (%04x)",  op); return 2;}


int arcompact_handle19_dasm(DASM_OPS_16)
{
	print("GP Instr (%04x)",  op);
	return 2;
}


int arcompact_handle1a_dasm(DASM_OPS_16)
{
	print("PCL Instr (%04x)", op);
	return 2;
}

int arcompact_handle1b_dasm(DASM_OPS_16)
{
	print("MOV_S (%04x)", op);
	return 2;
}

int arcompact_handle1c_dasm(DASM_OPS_16)
{
	print("ADD_S/CMP_S (%04x)", op);
	return 2;
}

int arcompact_handle1d_dasm(DASM_OPS_16)
{
	print("BRcc_S (%04x)", op);
	return 2;
}

int arcompact_handle1e_dasm(DASM_OPS_16)
{
	print("Bcc_S (%04x)", op);
	return 2;
}

int arcompact_handle1f_dasm(DASM_OPS_16)
{
	print("BL_S (%04x)", op);
	return 2;
}

CPU_DISASSEMBLE(arcompact)
{
	int size = 2;

	UINT32 op = oprom[2] | (oprom[3] << 8);
	output = buffer;

	UINT8 instruction = ARCOMPACT_OPERATION;

	if (instruction < 0x0c)
	{
		size = 4;
		op <<= 16;
		op |= oprom[0] | (oprom[1] << 8);

		op &= ~0xf8000000;

		switch (instruction) // 32-bit instructions (with optional extra dword for immediate data)
		{
			case 0x00: size = arcompact_handle00_dasm(output, pc, op, oprom);	break; // Bcc
			case 0x01: size = arcompact_handle01_dasm(output, pc, op, oprom);	break; // BLcc/BRcc
			case 0x02: size = arcompact_handle02_dasm(output, pc, op, oprom);	break; // LD r+o
			case 0x03: size = arcompact_handle03_dasm(output, pc, op, oprom);	break; // ST r+o
			case 0x04: size = arcompact_handle04_dasm(output, pc, op, oprom);	break; // op a,b,c (basecase)
			case 0x05: size = arcompact_handle05_dasm(output, pc, op, oprom);	break; // op a,b,c (05 ARC ext)
			case 0x06: size = arcompact_handle06_dasm(output, pc, op, oprom);	break; // op a,b,c (06 ARC ext)
			case 0x07: size = arcompact_handle07_dasm(output, pc, op, oprom);	break; // op a,b,c (07 User ext)
			case 0x08: size = arcompact_handle08_dasm(output, pc, op, oprom);	break; // op a,b,c (08 User ext)
			case 0x09: size = arcompact_handle09_dasm(output, pc, op, oprom);	break; // op a,b,c (09 Market ext)
			case 0x0a: size = arcompact_handle0a_dasm(output, pc, op, oprom);	break; // op a,b,c (0a Market ext)
			case 0x0b: size = arcompact_handle0b_dasm(output, pc, op, oprom);	break; // op a,b,c (0b Market ext)
		}
	}
	else
	{	
		size = 2;
		op &= ~0xf800;


		switch (instruction) // 16-bit instructions
		{
			case 0x0c: size = arcompact_handle0c_dasm(output, pc, op, oprom);	break; // Load/Add reg-reg
			case 0x0d: size = arcompact_handle0d_dasm(output, pc, op, oprom);	break; // Add/Sub/Shft imm
			case 0x0e: size = arcompact_handle0e_dasm(output, pc, op, oprom);	break; // Mov/Cmp/Add
			case 0x0f: size = arcompact_handle0f_dasm(output, pc, op, oprom);	break; // op_S b,b,c (single 16-bit ops)
			case 0x10: size = arcompact_handle10_dasm(output, pc, op, oprom);	break; // LD_S
			case 0x11: size = arcompact_handle11_dasm(output, pc, op, oprom);	break; // LDB_S
			case 0x12: size = arcompact_handle12_dasm(output, pc, op, oprom);	break; // LDW_S
			case 0x13: size = arcompact_handle13_dasm(output, pc, op, oprom);	break; // LSW_S.X
			case 0x14: size = arcompact_handle14_dasm(output, pc, op, oprom);	break; // ST_S
			case 0x15: size = arcompact_handle15_dasm(output, pc, op, oprom);	break; // STB_S
			case 0x16: size = arcompact_handle16_dasm(output, pc, op, oprom);	break; // STW_S
			case 0x17: size = arcompact_handle17_dasm(output, pc, op, oprom);	break; // Shift/Sub/Bit
			case 0x18: size = arcompact_handle18_dasm(output, pc, op, oprom);	break; // Stack Instr
			case 0x19: size = arcompact_handle19_dasm(output, pc, op, oprom);	break; // GP Instr
			case 0x1a: size = arcompact_handle1a_dasm(output, pc, op, oprom);	break; // PCL Instr
			case 0x1b: size = arcompact_handle1b_dasm(output, pc, op, oprom);	break; // MOV_S
			case 0x1c: size = arcompact_handle1c_dasm(output, pc, op, oprom);	break; // ADD_S/CMP_S
			case 0x1d: size = arcompact_handle1d_dasm(output, pc, op, oprom);	break; // BRcc_S
			case 0x1e: size = arcompact_handle1e_dasm(output, pc, op, oprom);	break; // Bcc_S
			case 0x1f: size = arcompact_handle1f_dasm(output, pc, op, oprom);	break; // BL_S
		}
	}

	return size | DASMFLAG_SUPPORTED;
}

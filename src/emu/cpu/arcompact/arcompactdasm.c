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

static const char *table18[0x8] =
{
	/* 00 */ "LD_S (SP)",
	/* 01 */ "LDB_S (SP)",
	/* 02 */ "ST_S (SP)",
	/* 03 */ "STB_S (SP)",
	/* 04 */ "ADD_S (SP)",
	/* 05 */ "ADD_S/SUB_S (SP)",
	/* 06 */ "POP_S (SP)",
	/* 07 */ "PUSH_S (SP)",

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

		switch (instruction)
		{
			case 0x00:
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

				break;

			case 0x01:
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
				break;

			default:
				print("%s (%08x)", basic[instruction], op & ~0xf8000000 );
				break;

		}

		
	}
	else
	{	
		size = 2;

		switch (instruction)
		{
			case 0x0f:
			{
				// General Register Instructions (16-bit)
				// 01111 bbb ccc iiiii
				UINT8 subinstr = (op & 0x01f) >> 0;
				//print("%s (%04x)", table0f[subinstr], op & ~0xf81f);

#if 1			
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
								break;

							case 0x7:
							{
								// General Operations w/o Register
								// 01111 iii 111 00000
								UINT8 subinstr3 = (op & 0x0700) >> 8;

								print("%s (%04x)", table0f_00_07[subinstr3], op & ~0xffff);

								break;
							}
						}
					}
				}
#endif				
			
				break;

			}


			case 0x18:
			{
				// Stack Pointer Based Instructions (16-bit)
				// 11000 bbb iii uuuuu
				UINT8 subinstr = (op & 0x00e0) >> 5;
				print("%s (%04x)", table18[subinstr], op & ~0xf8e0);
				break;

			}

			default:
				print("%s (%04x)", basic[instruction], op & ~0xf800);
				break;
		}
	}


	return size | DASMFLAG_SUPPORTED;
}

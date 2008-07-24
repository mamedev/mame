/***************************************************************************

    dsp56dsm.c
    Disassembler for the portable Motorola/Freescale dsp56k emulator.
    Written by Andrew Gardner

***************************************************************************/

#include "dsp56k.h"

/* Main opcode categories */
static unsigned assemble_x_memory_data_move_ALU_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_dual_x_memory_data_read_ALU_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_TCC_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_bitfield_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_no_parallel_move_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_immediate_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_movec_opcodes(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_misc_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);
static unsigned assemble_unique_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc);

/* Sub-opcode decoding */
static void decode_data_ALU_opcode(const UINT16 op_byte, char* opcode_str, char* arg_str);
static void decode_data_ALU_opcode_dual_move(const UINT16 op_byte, char* opcode_str, char* arg_str);

/* Direct opcode decoding */
static unsigned decode_TCC_opcode(const UINT16 op, char* opcode_str, char* arg_str);
static unsigned decode_bitfield_opcode(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static unsigned decode_no_parallel_move_opcode(const UINT16 op, const UINT16 op2, const UINT16 pc, char* opcode_str, char* arg_str);
static unsigned decode_immediate_opcode(const UINT16 op, const UINT16 pc, char* opcode_str, char* arg_str);
static unsigned decode_movec_opcodes(const UINT16 op, const UINT16 op2, const UINT16 pc, char* opcode_str, char* arg_str);
static unsigned decode_misc_opcode(const UINT16 op, const UINT16 pc, char* opcode_str, char* arg_str);
static unsigned decode_unique_opcode(const UINT16 op, const UINT16 op2, const UINT16 pc, char* opcode_str, char* arg_str);

/* Parallel operation decoding */
static void decode_x_memory_data_move(const UINT16 op_byte, char* parallel_move_str);
static void decode_dual_x_memory_data_read(const UINT16 op, char* parallel_move_str, char* parallel_move_str2);


/* Helper functions */
#define BITS(CUR,MASK) (dsp56k_op_mask(CUR,MASK))
static UINT16 dsp56k_op_mask(UINT16 op, UINT16 mask);

enum bbbType  { BBB_UPPER, BBB_MIDDLE, BBB_LOWER };

/* Table decoder functions */
static int  decode_BBB_table  (UINT16 BBB);
static void decode_cccc_table (UINT16 cccc, char *mnemonic);
static void decode_DDDDD_table(UINT16 DDDDD, char *SD);
static void decode_DD_table   (UINT16 DD, char *SD);
static void decode_DDF_table  (UINT16 DD, UINT16 F, char *S, char *D);
static void decode_EE_table   (UINT16 EE, char *D);
static void decode_F_table    (UINT16 F, char *SD);
static void decode_h0hF_table (UINT16 h0h, UINT16 F, char *S, char *D);
static void decode_HH_table   (UINT16 HH, char *SD);
static void decode_HHH_table  (UINT16 HHH, char *SD);
/* static void decode_IIII_table (UINT16 IIII, char *S, char *D); */
static void decode_JJJF_table (UINT16 JJJ, UINT16 F, char *S, char *D);
static void decode_JJF_table  (UINT16 JJ, UINT16 F, char *S, char *D);
static void decode_JF_table   (UINT16 J, UINT16 F, char *S, char *D);
/* static void decode_k_table    (UINT16 k, char *Dnot); */
static void decode_kSign_table(UINT16 k, char *plusMinus);
static void decode_KKK_table  (UINT16 KKK, char *D1, char *D2);
/* static int  decode_NN_table   (UINT16 NN); */
static void decode_QQF_table  (UINT16 QQ, UINT16 F, char *S1, char *S2, char *D);
static void decode_QQF_special_table(UINT16 QQ, UINT16 F, char *S1, char *S2, char *D);
static void decode_QQQF_table (UINT16 QQQ, UINT16 F, char *S1, char *S2, char *D);
static int  decode_RR_table   (UINT16 RR);
static int  decode_rr_table   (UINT16 rr);
static void decode_s_table    (UINT16 s, char *arithmetic);
static void decode_ss_table   (UINT16 ss, char *arithmetic);
static void decode_uuuuF_table(UINT16 uuuu, UINT16 F, char *arg, char *S, char *D);
static void decode_Z_table    (UINT16 Z, char *ea);

static void assemble_ea_from_m_table (UINT16 m, int n, char *ea);
static void assemble_eas_from_m_table(UINT16 mm, int n1, int n2, char *ea1, char *ea2);
static void assemble_ea_from_MM_table(UINT16 MM, int n, char *ea);
static void assemble_ea_from_t_table (UINT16 t, UINT16 val, char *ea);
static void assemble_ea_from_q_table (UINT16 q, int n, char *ea);
/* static void assemble_ea_from_z_table (UINT16 z, int n, char *ea); */

static void assemble_D_from_P_table(UINT16 P, UINT16 ppppp, char *D);
static void assemble_arguments_from_W_table(UINT16 W, char *args, char ma, char *SD, char *ea);
static void assemble_reg_from_W_table(UINT16 W, char *args, char ma, char *SD, UINT8 xx);

static void assemble_address_from_IO_short_address(UINT16 pp, char *ea);
static INT8 get_6_bit_relative_value(UINT16 bits);


/*****************************/
/* Main disassembly function */
/*****************************/
offs_t dsp56k_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	unsigned size = 0;
	const UINT16 op  = oprom[0] | (oprom[1] << 8);
	const UINT16 op2 = oprom[2] | (oprom[3] << 8);

	/* FILTER FOR PARALLEL DATA MOVES */
	if (BITS(op,0x8000))
	{
		/* ALU opcode with a single parallel data move */
		size = assemble_x_memory_data_move_ALU_opcode(buffer, op, op2, pc);
	}
	else if (BITS(op,0xe000) == 0x3)
	{
		/* ALU opcode with two parallel data moves */
		size = assemble_dual_x_memory_data_read_ALU_opcode(buffer, op, op2, pc);
	}
	else if (BITS(op,0xff00) == 0x4a)	/* TODO */
	{
		/* ALU opcode without any parallel data move */
		sprintf(buffer, "No parallel data move unimplemented.");
		size = 1;
	}
	else if (BITS(op,0xf000) == 0x4)	/* TODO */
	{
		/* ALU opcode with a parallel register to register move */
		sprintf(buffer, "Parallel register to register data move unimplemented.");
		size = 1;
	}
	else if (BITS(op,0xf800) == 0x6)	/* TODO */
	{
		/* ALU opcode with an address register update */
		sprintf(buffer, "Parallel address register update unimplemented.");
		size = 1;
	}
	else if (BITS(op,0xf000) == 0x5)	/* TODO */
	{
		/* ALU opcode with an cooperative x data memory move */
		sprintf(buffer, "Parallel coorperative x data memory move unimplemented.");
		size = 1;
	}
	else if (BITS(op,0xff00) == 0x05)	/* TODO */
	{
		/* ALU opcode with x memory data move + short displacement */
		sprintf(buffer, "Parallel x memory data move + short displacement unimplemented.");
		size = 2;
	}
	else if (BITS(op,0xfe00) == 0xb)	/* TODO */
	{
		/* ALU opcode with x memory data write and register data move */
		/* Don't forget these two, unique cases */
		/* MPY             X       0001 0110 xxxx xxxx */
		/* MAC             X       0001 0111 xxxx xxxx */
		sprintf(buffer, "Parallel x memory data write and register data move unimplemented.");
		size = 1;
	}

	/* Tcc is a unique critter */
	if (BITS(op,0xfc00) == 0x4)
	{
		size = assemble_TCC_opcode(buffer, op, op2, pc);
	}

	/* Operations that do not allow a parallel move */
	if (BITS(op,0xff00) == 0x14)
	{
		size = assemble_bitfield_opcode(buffer, op, op2, pc);
	}
	if (BITS(op,0xff00) == 0x15)
	{
		size = assemble_no_parallel_move_opcode(buffer, op, op2, pc);
	}
	if (BITS(op,0xf800) == 0x3)
	{
		size = assemble_immediate_opcode(buffer, op, op2, pc);
	}
	if (BITS(op,0xf800) == 0x7)
	{
		size = assemble_movec_opcodes(buffer, op, op2, pc);
	}
	if (BITS(op,0xf000) == 0x2)
	{
		size = assemble_misc_opcode(buffer, op, op2, pc);
	}

	if (BITS(op,0xf000) == 0x0)
	{
		size = assemble_unique_opcode(buffer, op, op2, pc);
	}

	/* Not recognized?  Nudge debugger onto the next opcode. */
	if (size == 0)
	{
		sprintf(buffer, "unknown");
		size = 1;
	}

	return size | DASMFLAG_SUPPORTED;
}


static unsigned assemble_x_memory_data_move_ALU_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* All operations are of length 1 */
	unsigned opSize = 1;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";
	char parallel_move_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* First, decode the Data ALU opcode */
	decode_data_ALU_opcode(BITS(op,0x00ff), opcode_str, arg_str);

	/* Next, decode the X Memory Data Move */
	decode_x_memory_data_move(BITS(op,0xff00), parallel_move_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s    %s", opcode_str, arg_str, parallel_move_str);

	return opSize;
}

/* Working save for a couple of opcode oddities */
static unsigned assemble_dual_x_memory_data_read_ALU_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* All operations are of length 1 */
	unsigned opSize = 1;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";
	char parallel_move_str[128] = "";
	char parallel_move_str2[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* First, decode the Data ALU opcode */
	decode_data_ALU_opcode_dual_move(BITS(op,0x00ff), opcode_str, arg_str);

	/* Next, decode the Dual X Memory Data Read */
	decode_dual_x_memory_data_read(op, parallel_move_str, parallel_move_str2);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s    %s %s", opcode_str, arg_str, parallel_move_str, parallel_move_str2);

	return opSize;
}

static unsigned assemble_TCC_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* TCC is of length 1 */
	unsigned opSize = 1;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Simply decode the opcode and its arguments */
	opSize = decode_TCC_opcode(op, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}

static unsigned assemble_bitfield_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* All bitfield ops are length 2 */
	unsigned opSize = 2;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Simply decode the opcode and its arguments */
	opSize = decode_bitfield_opcode(op, op2, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}

static unsigned assemble_no_parallel_move_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* Length is variable */
	unsigned opSize = 0;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Simply decode the opcode and its arguments */
	opSize = decode_no_parallel_move_opcode(op, op2, pc, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}

static unsigned assemble_immediate_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* Length is variable */
	unsigned opSize = 0;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Decode the 4 opcode and their arguments */
	opSize = decode_immediate_opcode(op, pc, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}

static unsigned assemble_movec_opcodes(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* Length is variable */
	unsigned opSize = 0;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Simply decode the opcode and its arguments */
	opSize = decode_movec_opcodes(op, op2, pc, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}

static unsigned assemble_misc_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* Length is variable */
	unsigned opSize = 0;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Simply decode the opcode and its arguments */
	opSize = decode_misc_opcode(op, pc, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}

/* Working except for a TODO */
static unsigned assemble_unique_opcode(char* buffer, const UINT16 op, const UINT16 op2, const unsigned pc)
{
	/* Length is variable */
	unsigned opSize = 0;

	/* Recovered strings */
	char arg_str[128] = "";
	char opcode_str[128] = "";

	/* Init */
	sprintf(buffer, " ");

	/* Simply decode the opcode and its arguments */
	opSize = decode_unique_opcode(op, op2, pc, opcode_str, arg_str);

	/* Finally, assemble the full opcode */
	sprintf(buffer, "%s    %s", opcode_str, arg_str);

	return opSize;
}


/**************************/
/* Actual opcode decoding */
/**************************/
static void decode_data_ALU_opcode(const UINT16 op_byte, char* opcode_str, char* arg_str)
{
	char D[128];
	char S1[128];

	if (!BITS(op_byte, 0x80))
	{
		switch(BITS(op_byte,0x70))
		{
			case 0x0:
				if (BITS(op_byte,0x07) == 0x1)
				{
					/* CLR - 1mRR HHHW 0000 F001 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "clr");
					sprintf(arg_str, "%s", D);
				}
				else
				{
					/* ADD - 1mRR HHHW 0000 FJJJ */
					decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "add");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				break;

			case 0x1:
				if (BITS(op_byte,0x0f) == 0x1)
				{
					/* MOVE - 1mRR HHHW 0001 0001 */
					/* Equivalent to a NOP (+ parallel move) */
					sprintf(opcode_str, "move");
					sprintf(arg_str, " ");
				}
				else
				{
					/* TFR - 1mRR HHHW 0001 FJJJ */
					decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "tfr");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				break;

			case 0x2:
				if (BITS(op_byte,0x07) == 0x0)
				{
					/* RND - 1mRR HHHW 0010 F000 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "rnd");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x1)
				{
					/* TST - 1mRR HHHW 0010 F001 */
		            decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "tst");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x2)
				{
					/* INC - 1mRR HHHW 0010 F010 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "inc");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x3)
				{
					/* INC24 - 1mRR HHHW 0010 F011 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "inc24");
					sprintf(arg_str, "%s", D);
				}
				else
				{
					/* OR - 1mRR HHHW 0010 F1JJ */
					decode_JJF_table(BITS(op_byte,0x03),BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "or");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				break;

			case 0x3:
				if (BITS(op_byte,0x07) == 0x0)
				{
					/* ASR - 1mRR HHHW 0011 F000 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "asr");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x1)
				{
	                /* ASL - 1mRR HHHW 0011 F001 */
	                decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "asl");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x2)
				{
					/* LSR - 1mRR HHHW 0011 F010 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "lsr");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x3)
				{
	                /* LSL - 1mRR HHHW 0011 F011 */
	                decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "lsl");
					sprintf(arg_str, "%s", D);
				}
				else
				{
					/* EOR - 1mRR HHHW 0011 F1JJ */
					decode_JJF_table(BITS(op_byte,0x03),BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "eor");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				break;

			case 0x4:
				if (BITS(op_byte,0x07) == 0x1)
				{
					/* SUBL - 1mRR HHHW 0100 F001 */
					sprintf(opcode_str, "subl");

					/* Only one option for the F table */
					if (!BITS(op_byte,0x08))
						sprintf(arg_str, "B,A");
					else
						sprintf(arg_str, "!");
				}
				else
				{
					/* SUB - 1mRR HHHW 0100 FJJJ */
					decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "sub");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				break;

			case 0x5:
				if (BITS(op_byte,0x07) == 0x1)
				{
					/* CLR24 - 1mRR HHHW 0101 F001 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "clr24");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x06) == 0x1)
				{
					/* SBC - 1mRR HHHW 0101 F01J */
					decode_JF_table(BITS(op_byte,0x01), BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "sbc");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				else
				{
					/* CMP - 1mRR HHHW 0101 FJJJ */
					decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
					/* TODO: This is a JJJF limited */
					sprintf(opcode_str, "cmp");
					sprintf(arg_str, "%s,%s", S1,D);
				}
				break;

			case 0x6:
				if (BITS(op_byte,0x07) == 0x0)
				{
					/* NEG - 1mRR HHHW 0110 F000 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "neg");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x1)
				{
					/* NOT - 1mRR HHHW 0110 F001 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "not");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x2)
				{
					/* DEC - 1mRR HHHW 0110 F010 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "dec");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x3)
				{
					/* DEC24 - 1mRR HHHW 0110 F011 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "dec24");
					sprintf(arg_str, "%s", D);
				}
				else
				{
					/* AND - 1mRR HHHW 0110 F1JJ */
					decode_JJF_table(BITS(op_byte,0x03),BITS(op_byte,0x08), S1, D);
					sprintf(opcode_str, "and");
					sprintf(arg_str, "%s,%s", S1, D);
				}
				break;

			case 0x7:
				if (BITS(op_byte,0x07) == 0x1)
				{
					/* ABS - 1mRR HHHW 0111 F001 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "abs");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x2)
				{
					/* ROR - 1mRR HHHW 0111 F010 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "ror");
					sprintf(arg_str, "%s", D);
				}
				else if (BITS(op_byte,0x07) == 0x3)
				{
					/* ROL - 1mRR HHHW 0111 F011 */
					decode_F_table(BITS(op_byte,0x08), D);
					sprintf(opcode_str, "rol");
					sprintf(arg_str, "%s", D);
				}
				else
				{
					/* CMPM - 1mRR HHHW 0111 FJJJ */
					decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
					/* TODO: This is JJJF limited */
					sprintf(opcode_str, "cmpm");
					sprintf(arg_str, "%s,%s", S1,D);
				}
				break;
		}
	}
	else
	{
		char S2[128];
		char SIGN[128];

        decode_QQQF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, S2, D);
        decode_kSign_table(BITS(op_byte,0x40), SIGN);

        switch(BITS(op_byte,0x30))
        {
			/* MPY - 1mRR HHHH 1k00 FQQQ */
            case 0x0:
				sprintf(opcode_str, "mpy");
				sprintf(arg_str, "(%s)%s,%s,%s", SIGN, S2, S1, D);
				break;

            /* MPYR - 1mRR HHHH 1k01 FQQQ */
            case 0x1:
				sprintf(opcode_str, "mpyr");
				sprintf(arg_str, "(%s)%s,%s,%s", SIGN, S2, S1, D);
				break;

            /* MAC - 1mRR HHHH 1k10 FQQQ */
            case 0x2:
				sprintf(opcode_str, "mac");
				sprintf(arg_str, "(%s)%s,%s,%s", SIGN, S2, S1, D);
				break;

            /* MACR - 1mRR HHHH 1k11 FQQQ */
            case 0x3:
				sprintf(opcode_str, "macr");
	            /* TODO: It's a little odd that macr is S1,S2 while everyone else is S2,S1.  Check! */
				sprintf(arg_str, "(%s)%s,%s,%s", SIGN, S1, S2, D);
				break;
        }
	}
}

/* TODO: Triple-check these.  There's weirdness around TFR & MOVE */
static void decode_data_ALU_opcode_dual_move(const UINT16 op_byte, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	char S2[32];
	char arg[32];

	if (!BITS(op_byte,0x80))
	{
		if (BITS(op_byte,0x14) != 0x2)
		{
			/* ADD - 011m mKKK 0rru Fuuu */
			/* SUB - 011m mKKK 0rru Fuuu */
			/* These two opcodes are conflated */
			decode_uuuuF_table(BITS(op_byte,0x17), BITS(op_byte,0x08), arg, S1, D);
			sprintf(opcode_str, "%s", arg);
			sprintf(arg_str, "%s,%s", S1, D);
		}
		else if (BITS(op_byte,0x14) == 0x2)
		{
			/* TFR - 011m mKKK 0rr1 F0DD */
			/* MOVE - 011m mKKK 0rr1 0000 */
			/* TODO: This MOVE opcode is .identical. to the TFR one.  Investigate. */
			decode_DDF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, D);
			sprintf(opcode_str, "tfr");
			sprintf(arg_str, "%s,%s", S1, D);
		}
	}
	else
	{
		decode_QQF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, S2, D);
		sprintf(arg_str, "%s,%s,%s", S1, S2, D);	/* Oddly, these 4 have identical operand ordering as */
													/* compared to single memory move equivalents */
		switch (BITS(op_byte,0x14))
		{
			case 0x0:
				/* MPY - 011m mKKK 1rr0 F0QQ */
				sprintf(opcode_str, "mpy");
				break;

			case 0x1:
				/* MAC - 011m mKKK 1rr0 F1QQ */
				sprintf(opcode_str, "mac");
				break;

			case 0x2:
				/* MPYR - 011m mKKK 1rr1 F0QQ */
				sprintf(opcode_str, "mpyr");
				break;

			case 0x3:
				/* MACR - 011m mKKK 1rr1 F1QQ */
				sprintf(opcode_str, "macr");
				break;
		}
	}
}

static unsigned decode_TCC_opcode(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum = -1;
	char M[32];
	char D[32];
	char S[32];

	decode_cccc_table(BITS(op,0x03c0), M);
	sprintf(opcode_str, "t.%s", M);

	Rnum = decode_RR_table(BITS(op,0x0030));
	decode_h0hF_table(BITS(op,0x0007),BITS(op,0x0008), S, D);
	sprintf(arg_str, "%s,%s  R0,R%d", S, D, Rnum);

	/* TODO: Investigate
    if (S1[0] == D[0] && D[0] == 'A')
        sprintf(buffer, "t.%s  %s,%s", M, S1, D);
    else
        sprintf(buffer, "t.%s  %s,%s  R0,R%d", M, S1, D, Rnum);
    */

	return 1;
}

/* TODO: Check all (dual-world) */
static unsigned decode_bitfield_opcode(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	char D[32];
	int upperMiddleLower = -1;
	UINT16 iVal = 0x0000;
	UINT16 rVal = 0x0000;

	/* Decode the common parts */
	upperMiddleLower = decode_BBB_table(BITS(op2,0xe000));
	iVal = BITS(op2,0x00ff);

	switch(upperMiddleLower)
	{
		case BBB_UPPER:  iVal <<= 8; break;
		case BBB_MIDDLE: iVal <<= 4; break;
		case BBB_LOWER:  iVal <<= 0; break;
	}

	switch(BITS(op,0x00e0))
	{
		case 0x6: case 0x7: case 0x2: case 0x3:
			assemble_D_from_P_table(BITS(op,0x0020), BITS(op,0x001f), D);
			break;

		case 0x5: case 0x1:
			rVal = decode_RR_table(BITS(op,0x0003));
			sprintf(D, "X:(R%d)", rVal);
			break;

		case 0x4: case 0x0:
			decode_DDDDD_table(BITS(op,0x001f), D);
			break;
	}
	sprintf(arg_str, "#%04x,%s", iVal, D);

	switch(BITS(op2,0x1f00))
	{
		case 0x12: sprintf(opcode_str, "bfchg");  break;
		case 0x04: sprintf(opcode_str, "bfclr");  break;
		case 0x18: sprintf(opcode_str, "bfset");  break;
		case 0x10: sprintf(opcode_str, "bftsth"); break;
		case 0x00: sprintf(opcode_str, "bftstl"); break;
	}

	return 2;
}

static unsigned decode_no_parallel_move_opcode(const UINT16 op, const UINT16 op2, const UINT16 pc, char* opcode_str, char* arg_str)
{
	unsigned retSize = 1;

	int Rnum = -1;
	char A[32];
	char D[32];
	char S1[32];
	char S2[32];

	switch(BITS(op,0x0074))
	{
		case 0x0:
			if (BITS(op,0x0006) == 0x0)
			{
				/* TFR(2) - 0001 0101 0000 F00J */
				decode_JF_table(BITS(op,0x0001),BITS(op,0x0008), D, S1);
				sprintf(opcode_str, "tfr2");
				sprintf(arg_str, "%s,%s", S1, D);
			}
			else if (BITS(op,0x0006) == 0x1)
			{
				/* ADC - 0001 0101 0000 F01J */
				decode_JF_table(BITS(op,0x0001),BITS(op,0x0008), S1, D);
				sprintf(opcode_str, "adc");
				sprintf(arg_str, "%s,%s", S1, D);
			}
			break;

		case 0x3:
			/* TST(2) - 0001 0101 0001 -1DD */
			decode_DD_table(BITS(op,0x0003), S1);
			sprintf(opcode_str, "tst2");
			sprintf(arg_str, "%s", S1);
			break;

		case 0x4:
			/* NORM - 0001 0101 0010 F0RR */
			decode_F_table(BITS(op,0x0008), D);
			Rnum = decode_RR_table(BITS(op,0x0003));
			sprintf(opcode_str, "norm");
			sprintf(arg_str, "R%d,%s", Rnum, D);
			break;

		case 0x6:
			if (BITS(op,0x0003) == 0x0)
			{
				/* ASR4 - 0001 0101 0011 F000 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "asr4");
				sprintf(arg_str, "%s", D);
			}
			else if (BITS(op,0x0003) == 0x1)
			{
				/* ASL4 - 0001 0101 0011 F001 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "asl4");
				sprintf(arg_str, "%s", D);
			}
			break;

		case 0x1: case 0x5: case 0x9: case 0xd:
			/* DIV - 0001 0101 0--0 F1DD */
			decode_DDF_table(BITS(op,0x0003), BITS(op,0x0008), S1, D);
			sprintf(opcode_str, "div");
			sprintf(arg_str, "%s,%s", S1, D);
			break;

		case 0xa:
			if (BITS(op,0x0003) == 0x0)
			{
				/* ZERO - 0001 0101 0101 F000 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "zero");
				sprintf(arg_str, "%s", D);
			}
			else if (BITS(op,0x0003) == 0x2)
			{
				/* EXT - 0001 0101 0101 F010 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "ext");
				sprintf(arg_str, "%s", D);
			}
			break;

		case 0xc:
			if (BITS(op,0x0003) == 0x0)
			{
				/* NEGC - 0001 0101 0110 F000 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "negc");
				sprintf(arg_str, "%s", D);
			}
			break;

		case 0xe:
			if (BITS(op,0x0003) == 0x0)
			{
				/* ASR16 - 0001 0101 0111 F000 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "asr16");
				sprintf(arg_str, "%s", D);
			}
			else if (BITS(op,0x0003) == 0x1)
			{
				/* SWAP - 0001 0101 0111 F001 */
				decode_F_table(BITS(op,0x0008), D);
				sprintf(opcode_str, "swap");
				sprintf(arg_str, "%s", D);
			}
			break;
	}

	switch(BITS(op,0x00f0))
	{
		case 0x8:
			/* IMPY - 0001 0101 1000 FQQQ */
			decode_QQQF_table(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D);
			sprintf(opcode_str, "impy");
			sprintf(arg_str, "%s,%s,%s", S1, S2, D);
			break;
		case 0xa:
			/* IMAC - 0001 0101 1010 FQQQ */
			decode_QQQF_table(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D);
			sprintf(opcode_str, "imac");
			sprintf(arg_str, "%s,%s,%s", S1, S2, D);
			break;
		case 0x9: case 0xb:
			/* DMAC(ss,su,uu) - 0001 0101 10s1 FsQQ */
			decode_ss_table(BITS(op,0x0024), A);
			decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D);     /* Special QQF */
			sprintf(opcode_str, "dmac(%s)", A);
			sprintf(arg_str, "%s,%s,%s", S1, S2, D);
			break;
		case 0xc:
			/* MPY(su,uu) - 0001 0101 1100 FsQQ */
			decode_s_table(BITS(op,0x0004), A);
			decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D);     /* Special QQF */
			sprintf(opcode_str, "mpy(%s)", A);
			sprintf(arg_str, "%s,%s,%s", S1, S2, D);
			break;
		case 0xe:
			/* MAC(su,uu) - 0001 0101 1110 FsQQ */
			decode_s_table(BITS(op,0x0004), A);
			decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D);		/* Special QQF */
			sprintf(opcode_str, "mac(%s)", A);
			sprintf(arg_str, "%s,%s,%s", S1, S2, D);
			break;
	}

	return retSize;
}

static unsigned decode_immediate_opcode(const UINT16 op, const UINT16 pc, char* opcode_str, char* arg_str)
{
	unsigned retSize = 1;

	if (BITS(op,0x0600))
	{
		if (!BITS(op,0x0100))
		{
			/* ANDI - 0001 1EE0 iiii iiii */
			char D[32];
			decode_EE_table(BITS(op,0x0600), D);
			sprintf(opcode_str, "and(i)");
			sprintf(arg_str, "#%02x,%s", BITS(op,0x00ff), D);
		}
		else
		{
			/* ORI - 0001 1EE1 iiii iiii */
			char D[32];
            decode_EE_table(BITS(op,0x0600), D);
			sprintf(opcode_str, "or(i)");
			sprintf(arg_str, "#%02x,%s", BITS(op,0x00ff), D);
		}
	}
	else
	{
		if (!BITS(op,0x0020))
		{
			/* MOVE(S) - 0001 100W HH0a aaaa */
			char A[32];
			char SD[32];
			char args[32];
			decode_HH_table(BITS(op,0x00c0), SD);
			sprintf(A, "00%02x", BITS(op,0x001f));
			assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, A);
			sprintf(opcode_str, "move(s)");
			sprintf(arg_str, "%s", args);
		}
		else
		{
			/* MOVE(P) - 0001 100W HH1p pppp */
			char A[32];
			char SD[32];
			char args[32];
			char fullAddy[128];
			decode_HH_table(BITS(op,0x00c0), SD);
			assemble_address_from_IO_short_address(BITS(op,0x001f), fullAddy);	/* Convert Short Absolute Address to full 16-bit */
			sprintf(A, "%02x (%s)", BITS(op,0x001f), fullAddy);
			assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, A);
			sprintf(opcode_str, "move(p)");
			sprintf(arg_str, "%s", args);
		}
	}

	return retSize;
}

static unsigned decode_movec_opcodes(const UINT16 op, const UINT16 op2, const UINT16 pc, char* opcode_str, char* arg_str)
{
	unsigned retSize = 1;

	char SD[32];
	char ea[32];
	char args[64];
	int Rnum = -1;

	if (BITS(op,0x0010) == 0x0)
	{
		/* MOVE(C) - 0011 1WDD DDD0 MMRR */
		decode_DDDDD_table(BITS(op,0x03e0), SD);
		Rnum = decode_RR_table(BITS(op,0x0003));
		assemble_ea_from_MM_table(BITS(op,0x000c), Rnum, ea);
		assemble_arguments_from_W_table(BITS(op,0x0400), args, 'X', SD, ea);
		sprintf(opcode_str, "move(c)");
		sprintf(arg_str, "%s", args);
	}
	else
	{
		if (BITS(op,0x0004) == 0x0)
		{
			/* MOVE(C) - 0011 1WDD DDD1 q0RR */
			decode_DDDDD_table(BITS(op,0x03e0), SD);
			Rnum = decode_RR_table(BITS(op,0x0003));
			assemble_ea_from_q_table(BITS(op,0x0008), Rnum, ea);
			assemble_arguments_from_W_table(BITS(op,0x0400), args, 'X', SD, ea);
			sprintf(opcode_str, "move(c)");
			sprintf(arg_str, "%s", args);
		}
		else
		{
			switch(BITS(op,0x0006))
			{
				/* MOVE(C) - 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx */
				case 0x2:
					decode_DDDDD_table(BITS(op,0x03e0), SD);
					assemble_ea_from_t_table(BITS(op,0x0008), op2, ea);
					/* !!! I'm pretty sure this is  in the right order - same issue as the WTables !!! */
					if (BITS(op,0x0400))												/* fixed - 02/03/05 */
						sprintf(args, "%s,%s", ea, SD);
					else
						sprintf(args, "%s,%s", SD, ea);
					sprintf(opcode_str, "move(c)");
					sprintf(arg_str, "%s", args);
					retSize = 2;
					break;

				/* MOVE(C) - 0011 1WDD DDD1 Z11- */
				case 0x3:
					decode_DDDDD_table(BITS(op,0x03e0), SD);
					decode_Z_table(BITS(op,0x0008), ea);
					assemble_arguments_from_W_table(BITS(op,0x0400), args, 'X', SD, ea);
					sprintf(opcode_str, "move(c)");
					sprintf(arg_str, "%s", args);
					break;
			}
		}
	}

	return retSize;
}

static unsigned decode_misc_opcode(const UINT16 op, const UINT16 pc, char* opcode_str, char* arg_str)
{
	unsigned retSize = 1;

	char M[32];
	char S1[32];
	char SD[32];
	char D1[32];
	char ea[32];
	char args[64];
	int Rnum = -1;
	int relativeInt = 666;

	switch(BITS(op,0x0c00))
	{
		/* MOVE(I) - 0010 00DD BBBB BBBB */
		case 0x0:
			decode_DD_table(BITS(op,0x0300), D1);
			sprintf(opcode_str, "move(i)");
			sprintf(arg_str, "#%02x,%s", BITS(op,0x00ff), D1);
			break;

		/* TFR(3) - 0010 01mW RRDD FHHH */
		case 0x1:
            decode_DDF_table(BITS(op,0x0030), BITS(op,0x0008), D1, S1);          /* Intentionally switched */
			decode_HHH_table(BITS(op,0x0007), SD);
			Rnum = decode_RR_table(BITS(op,0x00c0));
			assemble_ea_from_m_table(BITS(op,0x0200), Rnum, ea);
			assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea);
			sprintf(opcode_str, "tfr3");
			sprintf(arg_str, "%s,%s %s", S1, D1, args);
			break;

		/* MOVE(C) - 0010 10dd dddD DDDD */
		case 0x2:
			decode_DDDDD_table(BITS(op,0x03e0), S1);
			decode_DDDDD_table(BITS(op,0x001f), D1);
			sprintf(opcode_str, "move(c)");
			sprintf(arg_str, "%s,%s", S1, D1);
			break;

		/* B.cc - 0010 11cc ccee eeee */
		case 0x3:
			decode_cccc_table(BITS(op,0x3c0), M);
			relativeInt = get_6_bit_relative_value(BITS(op,0x003f));
			sprintf(opcode_str, "b.%s", M);
			sprintf(arg_str, "%04x (%d)", (int)pc + relativeInt, relativeInt);
			break;
	}

	return retSize;
}

/* TODO: Check all dual-word guys */
static unsigned decode_unique_opcode(const UINT16 op, const UINT16 op2, const UINT16 pc, char* opcode_str, char* arg_str)
{
	unsigned retSize = 1;

	char ea[32];
	char ea2[32];
	char S1[32];
	char SD[32];
	char args[32];

	if (BITS(op,0x0ff0) == 0x0)
	{
		switch (BITS(op,0x000f))
		{
			/* NOP - 0000 0000 0000 0000 */
			case 0x0:
				sprintf(opcode_str, "nop");
				sprintf(arg_str, " ");
				break;

			/* Debug - 0000 0000 0000 0001 */
			case 0x1:
				sprintf(opcode_str, "debug");
				sprintf(arg_str, " ");
				break;

			/* DO FOREVER - 0000 0000 0000 0010 xxxx xxxx xxxx xxxx */
			case 0x2:
				sprintf(opcode_str, "doForever");
				sprintf(arg_str, "%04x", op2);
				retSize = 2;
				break;

			/* chkaau - 0000 0000 0000 0100 */
			case 0x4:
				sprintf(opcode_str, "chkaau");
				sprintf(arg_str, " ");
				break;

			/* SWI - 0000 0000 0000 0101 */
			case 0x5:
				sprintf(opcode_str, "swi");
				sprintf(arg_str, " ");
				break;

			/* RTS - 0000 0000 0000 0110 */
			case 0x6:
				sprintf(opcode_str, "rts");
				sprintf(arg_str, " ");
				retSize |= DASMFLAG_STEP_OUT;
				break;

			/* RTI - 0000 0000 0000 0111 */
			case 0x7:
				sprintf(opcode_str, "rti");
				sprintf(arg_str, " ");
                retSize |= DASMFLAG_STEP_OUT;
				break;

			/* RESET - 0000 0000 0000 1000 */
			case 0x8:
				sprintf(opcode_str, "reset");
				sprintf(arg_str, " ");
				break;

			/* Enddo - 0000 0000 0000 1001 */
			case 0x9:
				sprintf(opcode_str, "enddo");
				sprintf(arg_str, " ");
				break;

			/* STOP - 0000 0000 0000 1010 */
			case 0xa:
				sprintf(opcode_str, "stop");
				sprintf(arg_str, " ");
				break;

			/* WAIT - 0000 0000 0000 1011 */
			case 0xb:
				sprintf(opcode_str, "wait");
				sprintf(arg_str, " ");
				break;

			/* ILLEGAL - 0000 0000 0000 1111 */
			case 0xf:
				sprintf(opcode_str, "illegal");
				sprintf(arg_str, " ");
				break;
		}
	}
	else if (BITS(op,0x0f00) == 0x0)
	{
		int Rnum = -1;
		char M[32] = "";

		switch(BITS(op,0x00e0))
		{
			case 0x2:
				/* DEBUG.cc - 0000 0000 0101 cccc */
				decode_cccc_table(BITS(op,0x000f), M);
				sprintf(opcode_str, "debug.%s", M);
				sprintf(arg_str, " ");
				break;

			case 0x6:
				/* DO - 0000 0000 110- --RR xxxx xxxx xxxx xxxx */
				Rnum = decode_RR_table(BITS(op,0x0003));
				sprintf(opcode_str, "do");
				sprintf(arg_str, "X:(R%d),%02x", Rnum, op2);
                retSize = 2;
				break;

			case 0x7:
				/* REP - 0000 0000 111- --RR */
				Rnum = decode_RR_table(BITS(op,0x0003));
				sprintf(opcode_str, "rep");
				sprintf(arg_str, "X:(R%d)", Rnum);
				break;
		}
	}
	else if (BITS(op,0x0f00) == 0x1)
	{
		char M[32] = "";

		if (BITS(op,0x00f0) == 0x1)
		{
			/* BRK.cc - 0000 0001 0001 cccc */
			decode_cccc_table(BITS(op,0x000f), M);
			sprintf(opcode_str, "brk.%s", M);
			sprintf(arg_str, " ");
		}
		else if (BITS(op,0x00f0) == 0x2)
		{
			/* All 4 instructions have the same RR encoding */
			int Rnum = decode_RR_table(BITS(op,0x0003));

			switch(BITS(op,0x000c))
			{
				/* JSR - 0000 0001 0010 00RR */
				case 0x0:
                    sprintf(opcode_str, "jsr");
                    sprintf(arg_str, "R%d", Rnum);
                    retSize |= DASMFLAG_STEP_OVER;
					break;

				/* JMP - 0000 0001 0010 01RR */
				case 0x1:
                    sprintf(opcode_str, "jmp");
                    sprintf(arg_str, "R%d", Rnum);
					break;

				/* BSR - 0000 0001 0010 10RR */
				case 0x2:
                    sprintf(opcode_str, "bsr");
                    sprintf(arg_str, "R%d", Rnum);
                    retSize |= DASMFLAG_STEP_OVER;
					break;

				/* BRA - 0000 0001 0010 11RR */
				case 0x3:
                    sprintf(opcode_str, "bra");
                    sprintf(arg_str, "R%d", Rnum);
					break;
			}
		}
		else if (BITS(op,0x00f0) == 0x3)
		{
			switch(BITS(op,0x000c))
			{
				/* JSR - 0000 0001 0011 00-- xxxx xxxx xxxx xxxx */
				case 0x0:
					sprintf(opcode_str, "jsr");
					sprintf(arg_str, "%04x", op2);
					retSize = 2;
                    retSize |= DASMFLAG_STEP_OVER;
					break;

				/* JMP - 0000 0001 0011 01-- xxxx xxxx xxxx xxxx */
				case 0x1:
					sprintf(opcode_str, "jmp");
					sprintf(arg_str, "%04x", op2);
					retSize = 2;
					break;

				/* BSR - 0000 0001 0011 10-- xxxx xxxx xxxx xxxx */
				case 0x2:
					sprintf(opcode_str, "bsr");
					sprintf(arg_str, "%d (0x%04x)", op2, op2);
					retSize = 2;
					retSize |= DASMFLAG_STEP_OVER;
					break;

				/* BRA - 0000 0001 0011 11-- xxxx xxxx xxxx xxxx */
				case 0x3:
					sprintf(opcode_str, "bra");
					sprintf(arg_str, "%d (0x%04x)", op2, op2);
                    retSize = 2;
					break;
			}
		}
		else if (BITS(op,0x00f0) == 0x5)
		{
			char M[32] = "";

			/* REP.cc - 0000 0001 0101 cccc */
			decode_cccc_table(BITS(op,0x000f), M);
			sprintf(opcode_str, "rep.%s", M);
			sprintf(arg_str, " ");
			/* !!! Should I decode the next instruction and put it here ???  probably... */
		}
		else if (BITS(op,0x0080) == 0x1)
		{
			/* LEA - 0000 0001 10TT MMRR */
			/*     - 0000 0001 11NN MMRR */
			int Rnum = decode_RR_table(BITS(op,0x0030));
            assemble_ea_from_MM_table(BITS(op,0x000c), BITS(op,0x0003), ea);
			sprintf(opcode_str, "lea");
            if (BITS(op,0x0040))
				sprintf(arg_str, "%s,R%d", ea, Rnum);
            else
				sprintf(arg_str, "%s,N%d", ea, Rnum);
		}
	}
	else if (BITS(op,0x0e00) == 0x1)
	{
		sprintf(opcode_str, "move(m)");

		if (BITS(op,0x0020) == 0x0)
		{
			/* MOVE(M) - 0000 001W RR0M MHHH */
			int Rnum = decode_RR_table(BITS(op,0x00c0));
			decode_HHH_table(BITS(op,0x0007), SD);
			assemble_ea_from_MM_table(BITS(op,0x0018), Rnum, ea);
			assemble_arguments_from_W_table(BITS(op,0x0100), args, 'P', SD, ea);
			sprintf(arg_str, "%s", args);
		}
		else
		{
			/* MOVE(M) - 0000 001W RR11 mmRR */
			assemble_eas_from_m_table(BITS(op,0x000c), BITS(op,0x00c0), BITS(op,0x0003), ea, ea2);
            sprintf(SD, "P:%s", ea);
            assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea2);
			sprintf(arg_str, "%s", args);
		}
	}
	else if (BITS(op,0x0f00) == 0x4)
	{
		if (BITS(op,0x0020) == 0x0)
		{
			/* DO - 0000 0100 000D DDDD xxxx xxxx xxxx xxxx */
			decode_DDDDD_table(BITS(op,0x001f), S1);
			sprintf(opcode_str, "do");
			sprintf(arg_str, "%s,%04x", S1, op2);
			retSize = 2;
		}
		else
		{
			/* REP - 0000 0100 001D DDDD */
            decode_DDDDD_table(BITS(op,0x001f), S1);
			sprintf(opcode_str, "rep");
			sprintf(arg_str, "%s", S1);
		}
	}
	else if (BITS(op,0x0f00) == 0x5)
	{
		UINT8 B;

		if (BITS(op2,0xfe20) == 0x02)
		{
			/* MOVE(M) - 0000 0101 BBBB BBBB | 0000 001W --0- -HHH */
			B = BITS(op,0x00ff);
			decode_HHH_table(BITS(op2,0x0007), SD);
			assemble_reg_from_W_table(BITS(op2,0x0100), args, 'P', SD, B);
			sprintf(opcode_str, "move(m)");
			sprintf(arg_str, "%s", args);
			/* !!! The docs list the read/write order backwards for all move(m)'s - crackbabies ??? */
			retSize = 2;
		}
		else if (BITS(op2,0xf810) == 0x0e)
		{
			/* MOVE(C) - 0000 0101 BBBB BBBB | 0011 1WDD DDD0 ---- */
			B = BITS(op,0x00ff);
            decode_DDDDD_table(BITS(op2,0x03e0), SD);
            assemble_reg_from_W_table(BITS(op2,0x0400), args, 'X', SD, B);
			sprintf(opcode_str, "move(c)");
			sprintf(arg_str, "%s", args);
            retSize = 2;
		}
		else if (BITS(op2,0x00ff) == 0x11)
		{
			/* MOVE - 0000 0101 BBBB BBBB | ---- HHHW 0001 0001 */
			B = BITS(op,0x00ff);
            decode_HHH_table(BITS(op2,0x0e00), SD);
            assemble_reg_from_W_table(BITS(op2,0x0100), args, 'X', SD, B);
			sprintf(opcode_str, "move");
			sprintf(arg_str, "%s", args);
            retSize = 2;
		}
	}
	else if (BITS(op,0x0f00) == 0x6)
	{
		int Rnum = -1;
		char M[32] = "";

		switch(BITS(op,0x0030))
		{
			case 0x0:
				/* JS.cc - 0000 0110 RR00 cccc */
				decode_cccc_table(BITS(op,0x000f), M);
                Rnum = decode_RR_table(BITS(op,0x00c0));
				sprintf(opcode_str, "js.%s", M);
				sprintf(arg_str, "R%d", Rnum);
				retSize |= DASMFLAG_STEP_OVER;	/* probably.  What's the diff between a branch and a jump? */
				break;

			case 0x1:
				/* JS.cc - 0000 0110 --01 cccc xxxx xxxx xxxx xxxx */
				decode_cccc_table(BITS(op,0x000f), M);
				sprintf(opcode_str, "js.%s", M);
				sprintf(arg_str, "%04x", op2);
                retSize = 2;
				retSize |= DASMFLAG_STEP_OVER;	/* probably */
				break;

			case 0x2:
				/* J.cc - 0000 0110 RR10 cccc */
				decode_cccc_table(BITS(op,0x000f), M);
                Rnum = decode_RR_table(BITS(op,0x00c0));
				sprintf(opcode_str, "j.%s", M);
                sprintf(arg_str, "R%d", Rnum);
				break;

			case 0x3:
				/* J.cc - 0000 0110 --11 cccc xxxx xxxx xxxx xxxx */
				decode_cccc_table(BITS(op,0x000f), M);
				sprintf(opcode_str, "j.%s", M);
				sprintf(arg_str, "%04x", op2);
                retSize = 2;
				break;
		}
	}
	else if (BITS(op,0x0f00) == 0x7)
	{
		int Rnum = -1;
		char M[32] = "";

		switch(BITS(op,0x0030))
		{
			case 0x0:
				/* BS.cc - 0000 0111 RR00 cccc */
				decode_cccc_table(BITS(op,0x000f), M);
                Rnum = decode_RR_table(BITS(op,0x00c0));
				sprintf(opcode_str, "bs.%s", M);
				sprintf(arg_str, "R%d", Rnum);
				retSize |= DASMFLAG_STEP_OVER;	/* probably.  What's the diff between a branch and a jump? */
				break;

			case 0x1:
				/* BS.cc - 0000 0111 --01 cccc xxxx xxxx xxxx xxxx */
				decode_cccc_table(BITS(op,0x000f), M);
				sprintf(opcode_str, "bs.%s", M);
				sprintf(arg_str, "%d (0x%04x)", (INT16)(op2), op2);
				retSize = 2;
				retSize |= DASMFLAG_STEP_OVER;	/* probably */
				break;

			case 0x2:
				/* B.cc - 0000 0111 RR10 cccc */
				decode_cccc_table(BITS(op,0x000f), M);
                Rnum = decode_RR_table(BITS(op,0x00c0));
				sprintf(opcode_str, "b.%s", M);
				sprintf(arg_str, "R%d", Rnum);
				break;

			case 0x3:
				/* B.cc - 0000 0111 --11 cccc xxxx xxxx xxxx xxxx */
				decode_cccc_table(BITS(op,0x000f), M);
				sprintf(opcode_str, "b.%s", M);
				sprintf(arg_str, "%04x (%d)", op2, op2);
				retSize = 2;
				break;
		}
	}
	else if (BITS(op,0x0800))
	{
		int Rnum = -1;

		switch (BITS(op,0x0700))
		{
			/* JSR - 0000 1010 AAAA AAAA */
			case 0x2:
				sprintf(opcode_str, "jsr");
				sprintf(arg_str, "%d (0x%02x)", BITS(op,0x00ff), BITS(op,0x00ff));
                retSize |= DASMFLAG_STEP_OVER;
				break;

			/* BRA - 0000 1011 aaaa aaaa */
			case 0x3:
				sprintf(opcode_str, "bra");
				sprintf(arg_str, "%d (0x%02x)", (INT8)BITS(op,0x00ff), pc + (INT8)BITS(op,0x00ff));
				break;

			/* MOVE(P) - 0000 110W RRmp pppp */
			case 0x4: case 0x5:
			{
				char fullAddy[128];		/* Convert Short Absolute Address to full 16-bit */
				Rnum = decode_RR_table(BITS(op,0x00c0));
				assemble_ea_from_m_table(BITS(op,0x0020), Rnum, ea);
				assemble_address_from_IO_short_address(BITS(op,0x001f), fullAddy);
				sprintf(SD, "X:%02x (%s)", BITS(op,0x001f), fullAddy);
				assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea);
				sprintf(opcode_str, "move(p)");
				sprintf(arg_str, "%s", args);
				break;
			}

			/* DO - 0000 1110 iiii iiii xxxx xxxx xxxx xxxx */
			case 0x6:
				sprintf(opcode_str, "do");
				sprintf(arg_str, "#%02x,%04x", BITS(op,0x00ff), op2);
				retSize = 2;
				break;

			/* REP - 0000 1111 iiii iiii */
			case 0x7:
				sprintf(opcode_str, "rep");
				sprintf(arg_str, "%d (0x%02x)", BITS(op,0x00ff), BITS(op,0x00ff));
				break;
		}
	}

	return retSize;
}


/*******************************/
/* Parallel data move decoding */
/*******************************/
static void decode_x_memory_data_move(const UINT16 op_byte, char* parallel_move_str)
{
	int Rnum;
	char SD[32];
	char ea[32];
	char args[32];

	/* Byte: 1mRR HHHW ---- ---- */
	Rnum = decode_RR_table(BITS(op_byte,0x30));
	decode_HHH_table(BITS(op_byte,0x0e), SD);
	assemble_ea_from_m_table(BITS(op_byte,0x40), Rnum, ea);
	assemble_arguments_from_W_table(BITS(op_byte,0x01), args, 'X', SD, ea);

	sprintf(parallel_move_str, "%s", args);
}

static void decode_dual_x_memory_data_read(const UINT16 op, char* parallel_move_str, char* parallel_move_str2)
{
	int Rnum;
	char D1[32] = "";
	char D2[32] = "";
	char ea1[32] = "";
	char ea2[32] = "";

	/* 011m mKKK -rr- ---- */
	Rnum = decode_rr_table(BITS(op,0x0060));
	decode_KKK_table(BITS(op,0x0700), D1, D2);
	assemble_eas_from_m_table(BITS(op,0x1800), Rnum, 3, ea1, ea2);

	if (Rnum == -1)
	{
		sprintf(ea1, "(!!)!");
	}

	sprintf(parallel_move_str,  "X:%s,%s", ea1, D1);
	sprintf(parallel_move_str2, "X:%s,%s", ea2, D2);
}


/* MISSING MPY */
/*
        char S[32];

        // MPY - 0001 0110 RRDD FQQQ
        decode_k_table(BITS(op,0x0100), Dnot);
        Rnum = decode_RR_table(BITS(op,0x00c0));
        decode_DD_table(BITS(op,0x0030), S);
        decode_QQQF_table(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D);
        sprintf(buffer, "mpy       %s,%s,%s %s,(R%d)+N%d %s,%s", S1, S2, D, Dnot, Rnum, Rnum, S, Dnot);
        // Strange, but not entirely out of the question - this 'k' parameter is hardcoded
        // I cheat here and do the parallel memory data move above - this specific one is only used twice
        retSize = 1;
*/

/* MISSING MAC */
/*
        char S[32];

        // MAC - 0001 0111 RRDD FQQQ
        decode_k_table(BITS(op,0x0100), Dnot);
        Rnum = decode_RR_table(BITS(op,0x00c0));
        decode_DD_table(BITS(op,0x0030), S);
        decode_QQQF_table(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D);
        sprintf(buffer, "mac       %s,%s,%s %s,(R%d)+N%d %s,%s", S1, S2, D, Dnot, Rnum, Rnum, S, Dnot);
        // Strange, but not entirely out of the question - this 'k' parameter is hardcoded
        // I cheat here and do the parallel memory data move above - this specific one is only used twice
        retSize = 1;
*/



/******************/
/* Table decoding */
/******************/
static int decode_BBB_table(UINT16 BBB)
{
	switch(BBB)
	{
		case 0x4: return BBB_UPPER ; break;
		case 0x2: return BBB_MIDDLE; break;
		case 0x1: return BBB_LOWER ; break;
	}

	return BBB_LOWER;                          /* Not really safe... */
}

static void decode_cccc_table(UINT16 cccc, char *mnemonic)
{
	switch (cccc)
	{
		case 0x0: sprintf(mnemonic, "cc(hs)"); break;
		case 0x1: sprintf(mnemonic, "ge    "); break;
		case 0x2: sprintf(mnemonic, "ne    "); break;
		case 0x3: sprintf(mnemonic, "pl    "); break;
		case 0x4: sprintf(mnemonic, "nn    "); break;
		case 0x5: sprintf(mnemonic, "ec    "); break;
		case 0x6: sprintf(mnemonic, "lc    "); break;
		case 0x7: sprintf(mnemonic, "gt    "); break;
		case 0x8: sprintf(mnemonic, "cs(lo)"); break;
		case 0x9: sprintf(mnemonic, "lt    "); break;
		case 0xa: sprintf(mnemonic, "eq    "); break;
		case 0xb: sprintf(mnemonic, "mi    "); break;
		case 0xc: sprintf(mnemonic, "nr    "); break;
		case 0xd: sprintf(mnemonic, "es    "); break;
		case 0xe: sprintf(mnemonic, "ls    "); break;
		case 0xf: sprintf(mnemonic, "le    "); break;
	}
}

static void decode_DDDDD_table(UINT16 DDDDD, char *SD)
{
	switch(DDDDD)
	{
		case 0x00: sprintf(SD, "X0");  break;
		case 0x01: sprintf(SD, "Y0");  break;
		case 0x02: sprintf(SD, "X1");  break;
		case 0x03: sprintf(SD, "Y1");  break;
		case 0x04: sprintf(SD, "A");   break;
		case 0x05: sprintf(SD, "B");   break;
		case 0x06: sprintf(SD, "A0");  break;
		case 0x07: sprintf(SD, "B0");  break;
		case 0x08: sprintf(SD, "LC");  break;
		case 0x09: sprintf(SD, "SR");  break;
		case 0x0a: sprintf(SD, "OMR"); break;
		case 0x0b: sprintf(SD, "SP");  break;
		case 0x0c: sprintf(SD, "A1");  break;
		case 0x0d: sprintf(SD, "B1");  break;
		case 0x0e: sprintf(SD, "A2");  break;
		case 0x0f: sprintf(SD, "B2");  break;

		case 0x10: sprintf(SD, "R0");  break;
		case 0x11: sprintf(SD, "R1");  break;
		case 0x12: sprintf(SD, "R2");  break;
		case 0x13: sprintf(SD, "R3");  break;
		case 0x14: sprintf(SD, "M0");  break;
		case 0x15: sprintf(SD, "M1");  break;
		case 0x16: sprintf(SD, "M2");  break;
		case 0x17: sprintf(SD, "M3");  break;
		case 0x18: sprintf(SD, "SSH"); break;
		case 0x19: sprintf(SD, "SSL"); break;
		case 0x1a: sprintf(SD, "LA");  break;
		case 0x1b: sprintf(SD, "!!");  break; /* no 0x1b */
		case 0x1c: sprintf(SD, "N0");  break;
		case 0x1d: sprintf(SD, "N1");  break;
		case 0x1e: sprintf(SD, "N2");  break;
		case 0x1f: sprintf(SD, "N3");  break;
	}
}

static void decode_DD_table(UINT16 DD, char *SD)
{
	switch (DD)
	{
		case 0x0: sprintf(SD, "X0"); break;
		case 0x1: sprintf(SD, "Y0"); break;
		case 0x2: sprintf(SD, "X1"); break;
		case 0x3: sprintf(SD, "Y1"); break;
	}
}

static void decode_DDF_table(UINT16 DD, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (DD << 1) | F;

	switch (switchVal)
	{
		case 0x0: sprintf(S, "X0"); sprintf(D, "A"); break;
		case 0x1: sprintf(S, "X0"); sprintf(D, "B"); break;
		case 0x2: sprintf(S, "Y0"); sprintf(D, "A"); break;
		case 0x3: sprintf(S, "Y0"); sprintf(D, "B"); break;
		case 0x4: sprintf(S, "X1"); sprintf(D, "A"); break;
		case 0x5: sprintf(S, "X1"); sprintf(D, "B"); break;
		case 0x6: sprintf(S, "Y1"); sprintf(D, "A"); break;
		case 0x7: sprintf(S, "Y1"); sprintf(D, "B"); break;
	}
}

static void decode_EE_table(UINT16 EE, char *D)
{
	switch(EE)
	{
		case 0x1: sprintf(D, "MR");  break;
		case 0x3: sprintf(D, "CCR"); break;
		case 0x2: sprintf(D, "OMR"); break;
	}
}

static void decode_F_table(UINT16 F, char *SD)
{
	switch(F)
	{
		case 0x0: sprintf(SD, "A"); break;
		case 0x1: sprintf(SD, "B"); break;
	}
}

static void decode_h0hF_table(UINT16 h0h, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (h0h << 1) | F;

	switch (switchVal)
	{
		case 0x8: sprintf(S, "X0"); sprintf(D, "A"); break;
		case 0x9: sprintf(S, "X0"); sprintf(D, "B"); break;
		case 0xa: sprintf(S, "Y0"); sprintf(D, "A"); break;
		case 0xb: sprintf(S, "Y0"); sprintf(D, "B"); break;
		case 0x2: sprintf(S, "A") ; sprintf(D, "A"); break;
		case 0x1: sprintf(S, "A") ; sprintf(D, "B"); break;
		case 0x0: sprintf(S, "B") ; sprintf(D, "A"); break;
		case 0x3: sprintf(S, "B") ; sprintf(D, "B"); break;
	}
}

static void decode_HH_table(UINT16 HH, char *SD)
{
	switch(HH)
	{
		case 0x0: sprintf(SD, "X0"); break;
		case 0x1: sprintf(SD, "Y0"); break;
		case 0x2: sprintf(SD, "A") ; break;
		case 0x3: sprintf(SD, "B") ; break;
	}
}

static void decode_HHH_table(UINT16 HHH, char *SD)
{
	switch(HHH)
	{
		case 0x0: sprintf(SD, "X0"); break;
		case 0x1: sprintf(SD, "Y0"); break;
		case 0x2: sprintf(SD, "X1"); break;
		case 0x3: sprintf(SD, "Y1"); break;
		case 0x4: sprintf(SD, "A") ; break;
		case 0x5: sprintf(SD, "B") ; break;
		case 0x6: sprintf(SD, "A0"); break;
		case 0x7: sprintf(SD, "B0"); break;
	}
}

#ifdef UNUSED_FUNCTION
static void decode_IIII_table(UINT16 IIII, char *S, char *D)
{
	switch(IIII)
	{
		case 0x0: sprintf(S, "X0"); sprintf(D, "^F"); break;
		case 0x1: sprintf(S, "Y0"); sprintf(D, "^F"); break;
		case 0x2: sprintf(S, "X1"); sprintf(D, "^F"); break;
		case 0x3: sprintf(S, "Y1"); sprintf(D, "^F"); break;
		case 0x4: sprintf(S, "A");  sprintf(D, "X0"); break;
		case 0x5: sprintf(S, "B");  sprintf(D, "Y0"); break;
		case 0x6: sprintf(S, "A0"); sprintf(D, "X0"); break;
		case 0x7: sprintf(S, "B0"); sprintf(D, "Y0"); break;
		case 0x8: sprintf(S, "F");  sprintf(D, "^F"); break;
		case 0x9: sprintf(S, "F");  sprintf(D, "^F"); break;
		case 0xc: sprintf(S, "A");  sprintf(D, "X1"); break;
		case 0xd: sprintf(S, "B");  sprintf(D, "Y1"); break;
		case 0xe: sprintf(S, "A0"); sprintf(D, "X1"); break;
		case 0xf: sprintf(S, "B0"); sprintf(D, "Y1"); break;
	}
}
#endif

static void decode_JJJF_table(UINT16 JJJ, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (JJJ << 1) | F;

	switch(switchVal)
	{
		case 0x0: sprintf(S, "B") ; sprintf(D, "A"); break;
		case 0x1: sprintf(S, "A") ; sprintf(D, "B"); break;
		case 0x2: sprintf(S, "!") ; sprintf(D, "!"); break;
		case 0x3: sprintf(S, "!") ; sprintf(D, "!"); break;
		case 0x4: sprintf(S, "X") ; sprintf(D, "A"); break;
		case 0x5: sprintf(S, "X") ; sprintf(D, "B"); break;
		case 0x6: sprintf(S, "Y") ; sprintf(D, "A"); break;
		case 0x7: sprintf(S, "Y") ; sprintf(D, "B"); break;
		case 0x8: sprintf(S, "X0"); sprintf(D, "A"); break;
		case 0x9: sprintf(S, "X0"); sprintf(D, "B"); break;
		case 0xa: sprintf(S, "Y0"); sprintf(D, "A"); break;
		case 0xb: sprintf(S, "Y0"); sprintf(D, "B"); break;
		case 0xc: sprintf(S, "X1"); sprintf(D, "A"); break;
		case 0xd: sprintf(S, "X1"); sprintf(D, "B"); break;
		case 0xe: sprintf(S, "Y1"); sprintf(D, "A"); break;
		case 0xf: sprintf(S, "Y1"); sprintf(D, "B"); break;
	}
}

static void decode_JJF_table(UINT16 JJ, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (JJ << 1) | F;

	switch (switchVal)
	{
		case 0x0: sprintf(S, "X0"); sprintf(D, "A"); break;
		case 0x1: sprintf(S, "X0"); sprintf(D, "B"); break;
		case 0x2: sprintf(S, "Y0"); sprintf(D, "A"); break;
		case 0x3: sprintf(S, "Y0"); sprintf(D, "B"); break;
		case 0x4: sprintf(S, "X1"); sprintf(D, "A"); break;
		case 0x5: sprintf(S, "X1"); sprintf(D, "B"); break;
		case 0x6: sprintf(S, "Y1"); sprintf(D, "A"); break;
		case 0x7: sprintf(S, "Y1"); sprintf(D, "B"); break;
	}
}

static void decode_JF_table(UINT16 J, UINT16 F, char *S, char *D)
{
	UINT16 switchVal = (J << 1) | F;

	switch(switchVal)
	{
		case 0x0: sprintf(S, "X"); sprintf(D, "A"); break;
		case 0x1: sprintf(S, "X"); sprintf(D, "B"); break;
		case 0x2: sprintf(S, "Y"); sprintf(D, "A"); break;
		case 0x3: sprintf(S, "Y"); sprintf(D, "B"); break;
	}
}

#ifdef UNUSED_FUNCTION
static void decode_k_table(UINT16 k, char *Dnot)
{
	switch(k)
	{
		case 0x0: sprintf(Dnot, "B"); break;
		case 0x1: sprintf(Dnot, "A"); break;
	}
}
#endif

static void decode_kSign_table(UINT16 k, char *plusMinus)
{
	switch(k)
	{
		case 0x0: sprintf(plusMinus, "+"); break;
		case 0x1: sprintf(plusMinus, "-"); break;
	}
}

static void decode_KKK_table(UINT16 KKK, char *D1, char *D2)
{
	switch(KKK)
	{
		case 0x0: sprintf(D1, "^F"); sprintf(D2, "X0"); break;
		case 0x1: sprintf(D1, "Y0"); sprintf(D2, "X0"); break;
		case 0x2: sprintf(D1, "X1"); sprintf(D2, "X0"); break;
		case 0x3: sprintf(D1, "Y1"); sprintf(D2, "X0"); break;
		case 0x4: sprintf(D1, "X0"); sprintf(D2, "X1"); break;
		case 0x5: sprintf(D1, "Y0"); sprintf(D2, "X1"); break;
		case 0x6: sprintf(D1, "^F"); sprintf(D2, "Y0"); break;
		case 0x7: sprintf(D1, "Y1"); sprintf(D2, "X1"); break;
	}
}

#ifdef UNUSED_FUNCTION
static int decode_NN_table(UINT16 NN)
{
	return NN;
}
#endif

static void decode_QQF_table(UINT16 QQ, UINT16 F, char *S1, char *S2, char *D)
{
	UINT16 switchVal = (QQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: sprintf(S1, "X0"); sprintf(S2, "Y0"); sprintf(D, "A"); break;
		case 0x1: sprintf(S1, "X0"); sprintf(S2, "Y0"); sprintf(D, "B"); break;
		case 0x2: sprintf(S1, "X0"); sprintf(S2, "Y1"); sprintf(D, "A"); break;
		case 0x3: sprintf(S1, "X0"); sprintf(S2, "Y1"); sprintf(D, "B"); break;
		case 0x4: sprintf(S1, "X1"); sprintf(S2, "Y0"); sprintf(D, "A"); break;
		case 0x5: sprintf(S1, "X1"); sprintf(S2, "Y0"); sprintf(D, "B"); break;
		case 0x6: sprintf(S1, "X1"); sprintf(S2, "Y1"); sprintf(D, "A"); break;
		case 0x7: sprintf(S1, "X1"); sprintf(S2, "Y1"); sprintf(D, "B"); break;
	}
}

static void decode_QQF_special_table(UINT16 QQ, UINT16 F, char *S1, char *S2, char *D)
{
	UINT16 switchVal = (QQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: sprintf(S1, "Y0"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0x1: sprintf(S1, "Y0"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0x2: sprintf(S1, "Y1"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0x3: sprintf(S1, "Y1"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0x4: sprintf(S1, "X1"); sprintf(S2, "Y0"); sprintf(D, "A"); break;
		case 0x5: sprintf(S1, "X1"); sprintf(S2, "Y0"); sprintf(D, "B"); break;
		case 0x6: sprintf(S1, "X1"); sprintf(S2, "Y1"); sprintf(D, "A"); break;
		case 0x7: sprintf(S1, "X1"); sprintf(S2, "Y1"); sprintf(D, "B"); break;
	}
}

static void decode_QQQF_table(UINT16 QQQ, UINT16 F, char *S1, char *S2, char *D)
{
	UINT16 switchVal = (QQQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: sprintf(S1, "X0"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0x1: sprintf(S1, "X0"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0x2: sprintf(S1, "X1"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0x3: sprintf(S1, "X1"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0x4: sprintf(S1, "A1"); sprintf(S2, "Y0"); sprintf(D, "A"); break;
		case 0x5: sprintf(S1, "A1"); sprintf(S2, "Y0"); sprintf(D, "B"); break;
		case 0x6: sprintf(S1, "B1"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0x7: sprintf(S1, "B1"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0x8: sprintf(S1, "Y0"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0x9: sprintf(S1, "Y0"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0xa: sprintf(S1, "Y1"); sprintf(S2, "X0"); sprintf(D, "A"); break;
		case 0xb: sprintf(S1, "Y1"); sprintf(S2, "X0"); sprintf(D, "B"); break;
		case 0xc: sprintf(S1, "Y0"); sprintf(S2, "X1"); sprintf(D, "A"); break;
		case 0xd: sprintf(S1, "Y0"); sprintf(S2, "X1"); sprintf(D, "B"); break;
		case 0xe: sprintf(S1, "Y1"); sprintf(S2, "X1"); sprintf(D, "A"); break;
		case 0xf: sprintf(S1, "Y1"); sprintf(S2, "X1"); sprintf(D, "B"); break;
	}
}

static int decode_RR_table(UINT16 RR)
{
	return RR;
}

static int decode_rr_table(UINT16 rr)
{
	if (rr != 0x3)
		return rr;
	else
		return -1;
}

static void decode_s_table(UINT16 s, char *arithmetic)
{
	switch(s)
	{
		case 0x0: sprintf(arithmetic, "su"); break;
		case 0x1: sprintf(arithmetic, "uu"); break;
	}
}

static void decode_ss_table(UINT16 ss, char *arithmetic)
{
	switch(ss)
	{
		case 0x0: sprintf(arithmetic, "ss"); break;
		case 0x1: sprintf(arithmetic, "ss"); break;
		case 0x2: sprintf(arithmetic, "su"); break;
		case 0x3: sprintf(arithmetic, "uu"); break;
	}
}

static void decode_uuuuF_table(UINT16 uuuu, UINT16 F, char *arg, char *S, char *D)
{
	UINT16 switchVal = (uuuu << 1) | F;

	switch(switchVal)
	{
		case 0x00: sprintf(arg, "add"); sprintf(S, "X0"); sprintf(D, "A"); break;
		case 0x08: sprintf(arg, "sub"); sprintf(S, "X0"); sprintf(D, "A"); break;
		case 0x01: sprintf(arg, "add"); sprintf(S, "X0"); sprintf(D, "B"); break;
		case 0x09: sprintf(arg, "sub"); sprintf(S, "X0"); sprintf(D, "B"); break;
		case 0x02: sprintf(arg, "add"); sprintf(S, "Y0"); sprintf(D, "A"); break;
		case 0x0a: sprintf(arg, "sub"); sprintf(S, "Y0"); sprintf(D, "A"); break;
		case 0x03: sprintf(arg, "add"); sprintf(S, "Y0"); sprintf(D, "B"); break;
		case 0x0b: sprintf(arg, "sub"); sprintf(S, "Y0"); sprintf(D, "B"); break;
		case 0x04: sprintf(arg, "add"); sprintf(S, "X1"); sprintf(D, "A"); break;
		case 0x0c: sprintf(arg, "sub"); sprintf(S, "X1"); sprintf(D, "A"); break;
		case 0x05: sprintf(arg, "add"); sprintf(S, "X1"); sprintf(D, "B"); break;
		case 0x0d: sprintf(arg, "sub"); sprintf(S, "X1"); sprintf(D, "B"); break;
		case 0x06: sprintf(arg, "add"); sprintf(S, "Y1"); sprintf(D, "A"); break;
		case 0x0e: sprintf(arg, "sub"); sprintf(S, "Y1"); sprintf(D, "A"); break;
		case 0x07: sprintf(arg, "add"); sprintf(S, "Y1"); sprintf(D, "B"); break;
		case 0x0f: sprintf(arg, "sub"); sprintf(S, "Y1"); sprintf(D, "B"); break;
		case 0x18: sprintf(arg, "add"); sprintf(S, "B") ; sprintf(D, "A"); break;
		case 0x1a: sprintf(arg, "sub"); sprintf(S, "B") ; sprintf(D, "A"); break;
		case 0x19: sprintf(arg, "add"); sprintf(S, "A") ; sprintf(D, "B"); break;
		case 0x1b: sprintf(arg, "sub"); sprintf(S, "A") ; sprintf(D, "B"); break;
	}
}

static void decode_Z_table(UINT16 Z, char *ea)
{
	/* This is fixed as per the Family Manual errata addendum */
	switch(Z)
	{
		case 0x1: sprintf(ea, "(A1)"); break;
		case 0x0: sprintf(ea, "(B1)"); break;
	}
}

static void assemble_ea_from_m_table(UINT16 m, int n, char *ea)
{
	switch(m)
	{
		case 0x0: sprintf(ea, "(R%d)+",n)       ; break;
		case 0x1: sprintf(ea, "(R%d)+N%d", n, n); break;
	}
}

static void assemble_eas_from_m_table(UINT16 mm, int n1, int n2, char *ea1, char *ea2)
{
	switch(mm)
	{
		case 0x0: sprintf(ea1, "(R%d)+",    n1)    ;
			      sprintf(ea2, "(R%d)+",    n2)    ; break;
		case 0x1: sprintf(ea1, "(R%d)+",    n1)    ;
			      sprintf(ea2, "(R%d)+N%d", n2, n2); break;
		case 0x2: sprintf(ea1, "(R%d)+N%d", n1, n1);
			      sprintf(ea2, "(R%d)+",    n2)    ; break;
		case 0x3: sprintf(ea1, "(R%d)+N%d", n1, n1);
			      sprintf(ea2, "(R%d)+N%d", n2, n2); break;
	}
}

static void assemble_ea_from_MM_table(UINT16 MM, int n, char *ea)
{
	switch(MM)
	{
		case 0x0: sprintf(ea, "(R%d)",     n)   ; break;
		case 0x1: sprintf(ea, "(R%d)+",    n)   ; break;
		case 0x2: sprintf(ea, "(R%d)-",    n)   ; break;
		case 0x3: sprintf(ea, "(R%d)+N%d", n, n); break;
	}
}

static void assemble_ea_from_q_table(UINT16 q, int n, char *ea)
{
	switch(q)
	{
		case 0x0: sprintf(ea, "(R%d+N%d)", n, n); break;
		case 0x1: sprintf(ea, "-(R%d)",    n)   ; break;
	}
}

static void assemble_ea_from_t_table(UINT16 t,  UINT16 val, char *ea)
{
	switch(t)
	{
		/* !!! Looking at page 336 of UM, I'm not sure if 0 is X: or if 0 is just # !!! */
		case 0x0: sprintf(ea, "X:%04x", val); break;
		case 0x1: sprintf(ea, "#%04x", val);  break;
	}
}


#ifdef UNUSED_FUNCTION
static void assemble_ea_from_z_table(UINT16 z, int n, char *ea)
{
	switch(z)
	{
		case 0x0: sprintf(ea, "(R%d)-",    n)   ; break;
		case 0x1: sprintf(ea, "(R%d)+N%d", n, n); break;
	}
}
#endif

static void assemble_D_from_P_table(UINT16 P, UINT16 ppppp, char *D)
{
	char fullAddy[128];		/* Convert Short Absolute Address to full 16-bit */

	switch(P)
	{
		case 0x0: sprintf(D, "X:%02x", ppppp); break;
		case 0x1:
			assemble_address_from_IO_short_address(ppppp, fullAddy);
			sprintf(D, "X:%02x (%s)", ppppp, fullAddy);
			break;
	}
}

static void assemble_arguments_from_W_table(UINT16 W, char *args, char ma, char *SD, char *ea)
{
	switch(W)
	{
		case 0x0: sprintf(args, "%s,%c:%s", SD, ma, ea); break;
		case 0x1: sprintf(args, "%c:%s,%s", ma, ea, SD); break;
	}
}

static void assemble_reg_from_W_table(UINT16 W, char *args, char ma, char *SD, UINT8 xx)
{
	switch(W)
	{
		case 0x0: sprintf(args, "%s,%c:(R2+%02x)", SD, ma, xx); break;
		case 0x1: sprintf(args, "%c:(R2+%02x),%s", ma, xx, SD); break;
	}
}

static void assemble_address_from_IO_short_address(UINT16 pp, char *ea)
{
	UINT16 fullAddy = 0xffe0;
	fullAddy |= pp;

	sprintf(ea, "%.04x", fullAddy);
}

static INT8 get_6_bit_relative_value(UINT16 bits)
{
	UINT16 fullAddy = bits;
	if (fullAddy & 0x0020)
		fullAddy |= 0xffc0;

	return (INT8)fullAddy;
}


/********************/
/* HELPER FUNCTIONS */
/********************/

static UINT16 dsp56k_op_mask(UINT16 cur, UINT16 mask)
{
	int i;

	UINT16 retVal = (cur & mask);
	UINT16 temp = 0x0000;
	int offsetCount = 0;

	/* Shift everything right, eliminating 'whitespace'... */
	for (i = 0; i < 16; i++)
	{
		if (mask & (0x1<<i))		/* If mask bit is non-zero */
		{
			temp |= (((retVal >> i) & 0x1) << offsetCount);
			offsetCount++;
		}
	}

	return temp;
}



/*

Data ALU Ops
--------------------

CLR             X       1xxx xxxx 0000 x001
ADD             X       1xxx xxxx 0000 xxxx

MOVE            X       1xxx xxxx 0001 0001
TFR             X       1xxx xxxx 0001 xxxx

RND             X       1xxx xxxx 0010 x000
TST             X       1xxx xxxx 0010 x001
INC             X       1xxx xxxx 0010 x010
INC24           X       1xxx xxxx 0010 x011
OR              X       1xxx xxxx 0010 x1xx

ASR             X       1xxx xxxx 0011 x000
ASL             X       1xxx xxxx 0011 x001
LSR             X       1xxx xxxx 0011 x010
LSL             X       1xxx xxxx 0011 x011
EOR             X       1xxx xxxx 0011 x1xx

SUBL            X       1xxx xxxx 0100 x001
SUB             X       1xxx xxxx 0100 xxxx

CLR24           X       1xxx xxxx 0101 x001
SBC             X       1xxx xxxx 0101 x01x
CMP             X       1xxx xxxx 0101 xxxx

NEG             X       1xxx xxxx 0110 x000
NOT             X       1xxx xxxx 0110 x001
DEC             X       1xxx xxxx 0110 x010
DEC24           X       1xxx xxxx 0110 x011
AND             X       1xxx xxxx 0110 x1xx

ABS             X       1xxx xxxx 0111 x001
ROR             X       1xxx xxxx 0111 x010
ROL             X       1xxx xxxx 0111 x011
CMPM            X       1xxx xxxx 0111 xxxx

MPY             X       1xxx xxxx 1x00 xxxx
MPYR            X       1xxx xxxx 1x01 xxxx
MAC             X       1xxx xxxx 1x10 xxxx
MACR            X       1xxx xxxx 1x11 xxxx


VERY STRANGE XMDM!      0101 HHHW xxxx xxxx


DXMDR
-----------------

ADD             X       011x xxxx 0xxx xxxx
SUB             X       011x xxxx 0xxx xxxx
TFR             X       011x xxxx 0xx1 x0xx
MOVE            X       011x xxxx 0xx1 0000

MPY             X       011x xxxx 1xx0 x0xx
MAC             X       011x xxxx 1xx0 x1xx
MPYR            X       011x xxxx 1XX1 x0xx
MACR            X       011x xxxx 1XX1 x1xx

TODO: don't forget us
x memory data write and register data move - unique
-----------------
MPY             X       0001 0110 xxxx xxxx
MAC             X       0001 0111 xxxx xxxx

ODDBALL
-------
Tcc             X       0001 00xx xxxx xx0x

BITFIELD
-----------------

BFTSTL          X       0001 0100 000x xxxx . xxx0 0000 xxxx xxxx
BFTSTH          X       0001 0100 000x xxxx . xxx1 0000 xxxx xxxx
BFTSTL          X       0001 0100 001X XXxx . xxx0 0000 xxxx xxxx
BFTSTH          X       0001 0100 001X XXxx . xxx1 0000 xxxx xxxx
BFTSTL          X       0001 0100 01xx xxxx . xxx0 0000 xxxx xxxx
BFTSTH          X       0001 0100 01xx xxxx . xxx1 0000 xxxx xxxx
BFCLR           X       0001 0100 100x xxxx . xxx0 0100 xxxx xxxx
BFCHG           X       0001 0100 100x xxxx . xxx1 0010 xxxx xxxx
BFSET           X       0001 0100 100x xxxx . xxx1 1000 xxxx xxxx
BFCLR           X       0001 0100 101X XXxx . xxx0 0100 xxxx xxxx
BFCHG           X       0001 0100 101X XXxx . xxx1 0010 xxxx xxxx
BFSET           X       0001 0100 101X XXxx . xxx1 1000 xxxx xxxx
BFCHG           X       0001 0100 11xx xxxx . xxx1 0010 xxxx xxxx
BFCLR           X       0001 0100 11xx xxxx . xxx0 0100 xxxx xxxx
BFSET           X       0001 0100 11xx xxxx . xxx1 1000 xxxx xxxx


NPM
-----------------

TFR2            X       0001 0101 0000 x00x
ADC             X       0001 0101 0000 x01x

TST2            X       0001 0101 0001 X1xx

NORM            X       0001 0101 0010 x0xx

ASR4            X       0001 0101 0011 x000
ASL4            X       0001 0101 0011 x001

DIV             X       0001 0101 0XX0 x1xx

ZERO            X       0001 0101 0101 x000
EXT             X       0001 0101 0101 x010

NEGC            X       0001 0101 0110 x000

ASR16           X       0001 0101 0111 x000
SWAP            X       0001 0101 0111 x001

IMPY            X       0001 0101 1000 xxxx
IMAC            X       0001 0101 1010 xxxx
DMAC(ss,su,uu)  X       0001 0101 10x1 xxxx
MPY(su,uu)      X       0001 0101 1100 xxxx
MAC(su,uu)      X       0001 0101 1110 xxxx

IMMEDIATE
-----------------

ANDI            X       0001 1xx0 xxxx xxxx
ORI             X       0001 1xx1 xxxx xxxx


SPECIAL
-----------------

MOVE(S)         X       0001 100x xx0x xxxx
MOVE(P)         X       0001 100x xx1x xxxx

-----------------

MOVE(I)         X       0010 00xx xxxx xxxx
TFR3            X       0010 01xx xxxx xxxx
MOVE(C)         X       0010 10xx xxxx xxxx
Bcc             X       0010 11xx xxxx xxxx

-----------------

MOVE(C)         X       0011 1xxx xxx0 xxxx
MOVE(C)         X       0011 1xxx xxx1 x0xx
MOVE(C)         X       0011 1xxx xxx1 x10X xxxx xxxx xxxx xxxx
MOVE(C)         X       0011 1xxx xxx1 x11X








-----------------

NOP             X       0000 0000 0000 0000
DEBUG           X       0000 0000 0000 0001
DO FOREVER      X       0000 0000 0000 0010 xxxx xxxx xxxx xxxx
CHKAAU          X       0000 0000 0000 0100
SWI             X       0000 0000 0000 0101
RTS             X       0000 0000 0000 0110
RTI             X       0000 0000 0000 0111
RESET           X       0000 0000 0000 1000
ENDDO           X       0000 0000 0000 1001
STOP            X       0000 0000 0000 1010
WAIT            X       0000 0000 0000 1011
ILLEGAL         X       0000 0000 0000 1111

DEBUGcc         X       0000 0000 0101 xxxx
DOLoop          X       0000 0000 110X XXxx xxxx xxxx xxxx xxxx
REP             X       0000 0000 111X XXxx

BRKcc           X       0000 0001 0001 xxxx

JSR             X       0000 0001 0010 00xx
JMP             X       0000 0001 0010 01xx
BSR             X       0000 0001 0010 10xx
BRA             X       0000 0001 0010 11xx

JSR             X       0000 0001 0011 00XX xxxx xxxx xxxx xxxx
JMP             X       0000 0001 0011 01XX xxxx xxxx xxxx xxxx
BSR             X       0000 0001 0011 10XX xxxx xxxx xxxx xxxx
BRA             X       0000 0001 0011 11XX xxxx xxxx xxxx xxxx

REPcc           X       0000 0001 0101 xxxx

LEA             X       0000 0001 10xx xxxx
LEA             X       0000 0001 11xx xxxx

-----------------

MOVE(M)         X       0000 001x xx0x xxxx
MOVE(M)         X       0000 001x xx11 xxxx

-----------------

DOLoop          X       0000 0100 000x xxxx xxxx xxxx xxxx xxxx
REP             X       0000 0100 001x xxxx

-----------------

MOVE(M)         X       0000 0101 xxxx xxxx 0000 001x XX0X Xxxx
MOVE(C)         X       0000 0101 xxxx xxxx 0011 1xxx xxx0 XXXX
MOVE            X       0000 0101 xxxx xxxx XXXX xxxx 0001 0001

-----------------

JScc            X       0000 0110 xx00 xxxx
JScc            X       0000 0110 XX01 xxxx xxxx xxxx xxxx xxxx
Jcc             X       0000 0110 xx10 xxxx
Jcc             X       0000 0110 XX11 xxxx xxxx xxxx xxxx xxxx

BScc            X       0000 0111 xx00 xxxx
BScc            X       0000 0111 XX01 xxxx xxxx xxxx xxxx xxxx
Bcc             X       0000 0111 xx10 xxxx
Bcc             X       0000 0111 XX11 xxxx xxxx xxxx xxxx xxxx

-----------------

JSR             X       0000 1010 xxxx xxxx
BRA             X       0000 1011 xxxx xxxx
MOVE(P)         X       0000 110x xxxx xxxx
DOLoop          X       0000 1110 xxxx xxxx xxxx xxxx xxxx xxxx
REP             X       0000 1111 xxxx xxxx

*/

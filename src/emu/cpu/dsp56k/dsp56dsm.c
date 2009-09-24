/***************************************************************************

    dsp56dsm.c
    Disassembler for the portable Motorola/Freescale dsp56k emulator.
    Written by Andrew Gardner

***************************************************************************/

/*
  This disassembler has been 95% verified against the docs and a different disassembler.
  The docs list some conflicting and confusing behaviors.  These are mareked with an asterisk
  and need to be tested on real hardware before this disassembler is considered 100% complete.
*/

#include "dsp56k.h"

/*******************/
/* Dasm prototypes */
/*******************/
static size_t dsp56k_dasm_addsub_2	(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mac_1		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_macr_1	(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_tfr_2		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_move_1	(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mpy_1		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mpyr_1	(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mpy_2		(const UINT16 op_byte, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_mac_2		(const UINT16 op_byte, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_clr		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_add		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_move		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_tfr		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_rnd		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_tst		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_inc		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_inc24		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_or		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_asr		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_asl		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_lsr		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_lsl		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_eor		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_subl		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_sub		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_clr24		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_sbc		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_cmp		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_neg		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_not		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_dec		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_dec24		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_and		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_abs		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_ror		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_rol		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_cmpm		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mpy		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mpyr		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_mac		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_macr		(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register);
static size_t dsp56k_dasm_adc		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_andi		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_asl4		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_asr4		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_asr16		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_bfop		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_bcc		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_bcc_1		(const UINT16 op, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_bcc_2		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_bra		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_bra_1		(const UINT16 op, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_bra_2		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_brkcc		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_bscc		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_bscc_1	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_bsr		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_bsr_1		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_chkaau	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_debug		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_debugcc	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_div		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_dmac		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_do		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_do_1		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_do_2		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_doforever	(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc);
static size_t dsp56k_dasm_enddo		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_ext		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_illegal	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_imac		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_impy		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jcc		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jcc_1		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jmp		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jmp_1		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jscc		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jscc_1	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jsr		(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jsr_1		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_jsr_2		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_lea		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_lea_1		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_macsuuu	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_move_2	(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movec		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movec_1	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movec_2	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movec_3	(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movec_4	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movec_5	(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movei		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movem		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movem_1	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movem_2	(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movep		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_movep_1	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_moves		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_mpysuuu	(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_negc		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_nop		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_norm		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_ori		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_rep		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_rep_1		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_rep_2		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_repcc		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_reset		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_rti		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_rts		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_stop		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_swap		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_swi		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_tcc		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_tfr2		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_tfr3		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_tst2		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_wait		(const UINT16 op, char* opcode_str, char* arg_str);
static size_t dsp56k_dasm_zero		(const UINT16 op, char* opcode_str, char* arg_str);


/***************************/
/* Table decoder functions */
/***************************/
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
static void decode_IIII_table (UINT16 IIII, char *S, char *D);
static void decode_JJJF_table (UINT16 JJJ, UINT16 F, char *S, char *D);
static void decode_JJF_table  (UINT16 JJ, UINT16 F, char *S, char *D);
static void decode_JF_table   (UINT16 J, UINT16 F, char *S, char *D);
static void decode_k_table    (UINT16 k, char *Dnot);
static void decode_kSign_table(UINT16 k, char *plusMinus);
static void decode_KKK_table  (UINT16 KKK, char *D1, char *D2);
static int  decode_NN_table   (UINT16 NN);
static int  decode_TT_table   (UINT16 TT);
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
static void assemble_ea_from_z_table (UINT16 z, int n, char *ea);

static void assemble_D_from_P_table(UINT16 P, UINT16 ppppp, char *D);
static void assemble_arguments_from_W_table(UINT16 W, char *args, char ma, char *SD, char *ea);
static void assemble_reg_from_W_table(UINT16 W, char *args, char ma, char *SD, INT8 xx);

static void assemble_address_from_IO_short_address(UINT16 pp, char *ea);
static INT8 get_6_bit_signed_value(UINT16 bits);


/**********************************/
/* Parallel memory move functions */
/**********************************/
static void decode_x_memory_data_move(const UINT16 op, char* parallel_move_str);
static void decode_x_memory_data_move2(const UINT16 op, char* parallel_move_str, char* d_register);
static void decode_dual_x_memory_data_read(const UINT16 op, char* parallel_move_str, char* parallel_move_str2, char* d_register);
static void decode_register_to_register_data_move(const UINT16 op, char* parallel_move_str, char* d_register);
static void decode_x_memory_data_write_and_register_data_move(const UINT16 op, char* parallel_move_str, char* parallel_move_str2);
static void decode_address_register_update(const UINT16 op, char* parallel_move_str);
static void decode_x_memory_data_move_with_short_displacement(const UINT16 op, const UINT16 op2, char* parallel_move_str);


/********************/
/* Helper functions */
/********************/
#define BITS(CUR,MASK) (dsp56k_op_mask(CUR,MASK))
static UINT16 dsp56k_op_mask(UINT16 op, UINT16 mask);
static void pad_string(const int dest_length, char* string);

enum bbbType  { BBB_UPPER, BBB_MIDDLE, BBB_LOWER, BBB_INVALID };



/*****************************/
/* Main disassembly function */
/*****************************/
CPU_DISASSEMBLE( dsp56k )
{
	/* ORDER: Handle parallel types in the ALU */
	/*        Handle the rest */
	unsigned size = 0;

	char arg_str[128] = "";
	char opcode_str[128] = "";
	char parallel_move_str[128] = "";
	char parallel_move_str2[128] = "";

	const UINT16 op  = oprom[0] | (oprom[1] << 8);
	const UINT16 op2 = oprom[2] | (oprom[3] << 8);

	/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
	if ((op & 0xe000) == 0x6000)
	{
		char d_register[32] = "";

		/* Quote: (MOVE, MAC(R), MPY(R), ADD, SUB, TFR) */
		UINT16 op_byte = op & 0x00ff;

		/* ADD : 011m mKKK 0rru Fuuu : A-22 */
		/* SUB : 011m mKKK 0rru Fuuu : A-202 */
		/* Note: 0x0094 check allows command to drop through to MOVE and TFR */
		if (((op & 0xe080) == 0x6000) && ((op & 0x0094) != 0x0010))
		{
			size = dsp56k_dasm_addsub_2(op_byte, opcode_str, arg_str, d_register);
		}
		/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
		else if ((op & 0xe094) == 0x6084)
		{
			size = dsp56k_dasm_mac_1(op_byte, opcode_str, arg_str, d_register);
		}
		/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
		else if ((op & 0xe094) == 0x6094)
		{
			size = dsp56k_dasm_macr_1(op_byte, opcode_str, arg_str, d_register);
		}
		/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
		else if ((op & 0xe094) == 0x6010)
		{
			size = dsp56k_dasm_tfr_2(op_byte, opcode_str, arg_str, d_register);
		}
		/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
		else if ((op & 0xe09f) == 0x6010)
		{
			/* Note: The opcode encoding : 011x xxxx 0xx1 0000 (move + double memory read)
                     is .identical. to (tfr X0,A + two parallel reads).  This sparks the notion
                     that these 'move' opcodes don't actually exist and are just there as
                     documentation.  Real-world examples would need to be examined to come
                     to a satisfactory conclusion, but as it stands, tfr will override this
                     move operation. */
			size = dsp56k_dasm_move_1(op_byte, opcode_str, arg_str, d_register);
		}
		/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
		else if ((op & 0xe094) == 0x6080)
		{
			size = dsp56k_dasm_mpy_1(op_byte, opcode_str, arg_str, d_register);
		}
		/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
		else if ((op & 0xe094) == 0x6090)
		{
			size = dsp56k_dasm_mpyr_1(op_byte, opcode_str, arg_str, d_register);
		}

		/* Now evaluate the parallel data move */
		decode_dual_x_memory_data_read(op, parallel_move_str, parallel_move_str2, d_register);
	}
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
	else if ((op & 0xfe00) == 0x1600)
	{
		/* Quote: (MPY or MAC) */
		UINT16 op_byte = op & 0x00ff;

		/* MPY : 0001 0110 RRDD FQQQ : A-160 */
		if ((op & 0xff00) == 0x1600)
		{
			size = dsp56k_dasm_mpy_2(op_byte, opcode_str, arg_str);
		}
		/* MAC : 0001 0111 RRDD FQQQ : A-122 */
		else if ((op & 0xff00) == 0x1700)
		{
			size = dsp56k_dasm_mac_2(op_byte, opcode_str, arg_str);
		}

		/* Now evaluate the parallel data move */
		decode_x_memory_data_write_and_register_data_move(op, parallel_move_str, parallel_move_str2);
	}

	/* Handle Other parallel types */
	else
	{
		/***************************************/
		/* 32 General parallel move operations */
		/***************************************/

		enum pType { kNoParallelDataMove,
					 kRegisterToRegister,
					 kAddressRegister,
					 kXMemoryDataMove,
					 kXMemoryDataMove2,
					 kXMemoryDataMoveWithDisp };

		UINT16 op_byte = 0x0000;
		char d_register[32] = "";
		int parallelType = -1;

		/* Note: it's important that NPDM comes before RtRDM here */
		/* No Parallel Data Move : 0100 1010 .... .... : A-131 */
		if ((op & 0xff00) == 0x4a00)
		{
			op_byte = op & 0x00ff;
			parallelType = kNoParallelDataMove;
		}
		/* Register to Register Data Move : 0100 IIII .... .... : A-133 */
		else if ((op & 0xf000) == 0x4000)
		{
			op_byte = op & 0x00ff;
			parallelType = kRegisterToRegister;
		}
		/* Address Register Update : 0011 0zRR .... .... : A-135 */
		else if ((op & 0xf800) == 0x3000)
		{
			op_byte = op & 0x00ff;
			parallelType = kAddressRegister;
		}
		/* X Memory Data Move : 1mRR HHHW .... .... : A-137 */
		else if ((op & 0x8000) == 0x8000)
		{
			op_byte = op & 0x00ff;
			parallelType = kXMemoryDataMove;
		}
		/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
		else if ((op & 0xf000) == 0x5000)
		{
			op_byte = op & 0x00ff;
			parallelType = kXMemoryDataMove2;
		}
		/* X Memory Data Move with short displacement : 0000 0101 BBBB BBBB ---- HHHW .... .... : A-139 */
		else if ((op & 0xff00) == 0x0500)
		{
			/* Now check it against all the other potential collisions */
			/* This is necessary because "don't care bits" get in the way. */
			/*
            MOVE(M) :   0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152
            MOVE(C) :   0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144
            MOVE :      0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128
            */
			if (((op2 & 0xfe20) != 0x0200) &&
				((op2 & 0xf810) != 0x3800) &&
				((op2 & 0x00ff) != 0x0011))
			{
				op_byte = op2 & 0x00ff;
				parallelType = kXMemoryDataMoveWithDisp;
			}
		}


		if (parallelType != -1)
		{
			/* Note: There is much overlap between opcodes down here */
			/*       To this end, certain ops must come before others in the list */

			/* CLR : .... .... 0000 F001 : A-60 */
			if ((op_byte & 0x00f7) == 0x0001)
			{
				size = dsp56k_dasm_clr(op_byte, opcode_str, arg_str, d_register);
			}
			/* ADD : .... .... 0000 FJJJ : A-22 */
			else if ((op_byte & 0x00f0) == 0x0000)
			{
				size = dsp56k_dasm_add(op_byte, opcode_str, arg_str, d_register);
			}


			/* MOVE : .... .... 0001 0001 : A-128 */
			else if ((op_byte & 0x00ff) == 0x0011)
			{
				size = dsp56k_dasm_move(op_byte, opcode_str, arg_str, d_register);
			}
			/* TFR : .... .... 0001 FJJJ : A-212 */
			else if ((op_byte & 0x00f0) == 0x0010)
			{
				size = dsp56k_dasm_tfr(op_byte, opcode_str, arg_str, d_register);
			}


			/* RND : .... .... 0010 F000 : A-188 */
			else if ((op_byte & 0x00f7) == 0x0020)
			{
				size = dsp56k_dasm_rnd(op_byte, opcode_str, arg_str, d_register);
			}
			/* TST : .... .... 0010 F001 : A-218 */
			else if ((op_byte & 0x00f7) == 0x0021)
			{
				size = dsp56k_dasm_tst(op_byte, opcode_str, arg_str, d_register);
			}
			/* INC : .... .... 0010 F010 : A-104 */
			else if ((op_byte & 0x00f7) == 0x0022)
			{
				size = dsp56k_dasm_inc(op_byte, opcode_str, arg_str, d_register);
			}
			/* INC24 : .... .... 0010 F011 : A-106 */
			else if ((op_byte & 0x00f7) == 0x0023)
			{
				size = dsp56k_dasm_inc24(op_byte, opcode_str, arg_str, d_register);
			}
			/* OR : .... .... 0010 F1JJ : A-176 */
			else if ((op_byte & 0x00f4) == 0x0024)
			{
				size = dsp56k_dasm_or(op_byte, opcode_str, arg_str, d_register);
			}


			/* ASR : .... .... 0011 F000 : A-32 */
			else if ((op_byte & 0x00f7) == 0x0030)
			{
				size = dsp56k_dasm_asr(op_byte, opcode_str, arg_str, d_register);
			}
			/* ASL : .... .... 0011 F001 : A-28 */
			else if ((op_byte & 0x00f7) == 0x0031)
			{
				size = dsp56k_dasm_asl(op_byte, opcode_str, arg_str, d_register);
			}
			/* LSR : .... .... 0011 F010 : A-120 */
			else if ((op_byte & 0x00f7) == 0x0032)
			{
				size = dsp56k_dasm_lsr(op_byte, opcode_str, arg_str, d_register);
			}
			/* LSL : .... .... 0011 F011 : A-118 */
			else if ((op_byte & 0x00f7) == 0x0033)
			{
				size = dsp56k_dasm_lsl(op_byte, opcode_str, arg_str, d_register);
			}
			/* EOR : .... .... 0011 F1JJ : A-94 */
			else if ((op_byte & 0x00f4) == 0x0034)
			{
				size = dsp56k_dasm_eor(op_byte, opcode_str, arg_str, d_register);
			}


			/* SUBL : .... .... 0100 F001 : A-204 */
			else if ((op_byte & 0x00f7) == 0x0041)
			{
				size = dsp56k_dasm_subl(op_byte, opcode_str, arg_str, d_register);
			}
			/* SUB : .... .... 0100 FJJJ : A-202 */
			else if ((op_byte & 0x00f0) == 0x0040)
			{
				size = dsp56k_dasm_sub(op_byte, opcode_str, arg_str, d_register);
			}


			/* CLR24 : .... .... 0101 F001 : A-62 */
			else if ((op_byte & 0x00f7) == 0x0051)
			{
				size = dsp56k_dasm_clr24(op_byte, opcode_str, arg_str, d_register);
			}
			/* SBC : .... .... 0101 F01J : A-198 */
			else if ((op_byte & 0x00f6) == 0x0052)
			{
				size = dsp56k_dasm_sbc(op_byte, opcode_str, arg_str, d_register);
			}
			/* CMP : .... .... 0101 FJJJ : A-64 */
			else if ((op_byte & 0x00f0) == 0x0050)
			{
				size = dsp56k_dasm_cmp(op_byte, opcode_str, arg_str, d_register);
			}


			/* NEG : .... .... 0110 F000 : A-166 */
			else if ((op_byte & 0x00f7) == 0x0060)
			{
				size = dsp56k_dasm_neg(op_byte, opcode_str, arg_str, d_register);
			}
			/* NOT : .... .... 0110 F001 : A-174 */
			else if ((op_byte & 0x00f7) == 0x0061)
			{
				size = dsp56k_dasm_not(op_byte, opcode_str, arg_str, d_register);
			}
			/* DEC : .... .... 0110 F010 : A-72 */
			else if ((op_byte & 0x00f7) == 0x0062)
			{
				size = dsp56k_dasm_dec(op_byte, opcode_str, arg_str, d_register);
			}
			/* DEC24 : .... .... 0110 F011 : A-74 */
			else if ((op_byte & 0x00f7) == 0x0063)
			{
				size = dsp56k_dasm_dec24(op_byte, opcode_str, arg_str, d_register);
			}
			/* AND : .... .... 0110 F1JJ : A-24 */
			else if ((op_byte & 0x00f4) == 0x0064)
			{
				size = dsp56k_dasm_and(op_byte, opcode_str, arg_str, d_register);
			}


			/* ABS : .... .... 0111 F001 : A-18 */
			if ((op_byte & 0x00f7) == 0x0071)
			{
				size = dsp56k_dasm_abs(op_byte, opcode_str, arg_str, d_register);
			}
			/* ROR : .... .... 0111 F010 : A-192 */
			else if ((op_byte & 0x00f7) == 0x0072)
			{
				size = dsp56k_dasm_ror(op_byte, opcode_str, arg_str, d_register);
			}
			/* ROL : .... .... 0111 F011 : A-190 */
			else if ((op_byte & 0x00f7) == 0x0073)
			{
				size = dsp56k_dasm_rol(op_byte, opcode_str, arg_str, d_register);
			}
			/* CMPM : .... .... 0111 FJJJ : A-66 */
			else if ((op_byte & 0x00f0) == 0x0070)
			{
				size = dsp56k_dasm_cmpm(op_byte, opcode_str, arg_str, d_register);
			}


			/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
			else if ((op_byte & 0x00b0) == 0x0080)
			{
				size = dsp56k_dasm_mpy(op_byte, opcode_str, arg_str, d_register);
			}
			/* MPYR : .... .... 1k01 FQQQ : A-162 */
			else if ((op_byte & 0x00b0) == 0x0090)
			{
				size = dsp56k_dasm_mpyr(op_byte, opcode_str, arg_str, d_register);
			}
			/* MAC : .... .... 1k10 FQQQ : A-122 */
			else if ((op_byte & 0x00b0) == 0x00a0)
			{
				size = dsp56k_dasm_mac(op_byte, opcode_str, arg_str, d_register);
			}
			/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
			else if ((op_byte & 0x00b0) == 0x00b0)
			{
				size = dsp56k_dasm_macr(op_byte, opcode_str, arg_str, d_register);
			}


			/* Now evaluate the parallel data move */
			switch (parallelType)
			{
			case kNoParallelDataMove:
				/* DO NOTHING */
				size = 1;
				break;
			case kRegisterToRegister:
				decode_register_to_register_data_move(op, parallel_move_str, d_register);
				size = 1;
				break;
			case kAddressRegister:
				decode_address_register_update(op, parallel_move_str);
				size = 1;
				break;
			case kXMemoryDataMove:
				decode_x_memory_data_move(op, parallel_move_str);
				size = 1;
				break;
			case kXMemoryDataMove2:
				decode_x_memory_data_move2(op, parallel_move_str, d_register);
				size = 1;
				break;
			case kXMemoryDataMoveWithDisp:
				decode_x_memory_data_move_with_short_displacement(op, op2, parallel_move_str);
				size = 2;
				break;
			}
		}
	}

	/* Assemble and return parallel move operation */
	if (size > 0)
	{
		char space[32] = "";

		pad_string(11, opcode_str);
		if (strlen(parallel_move_str) != 0)
			pad_string(15, arg_str);
		if (strlen(parallel_move_str2) != 0)
			sprintf(space, " ");

		sprintf(buffer, "%s%s%s%s%s", opcode_str, arg_str, parallel_move_str, space, parallel_move_str2);

		return (size | DASMFLAG_SUPPORTED);
	}


	/******************************/
	/* Remaining non-parallel ops */
	/******************************/

	/* ADC : 0001 0101 0000 F01J : A-20 */
	if ((op & 0xfff6) == 0x1502)
	{
		size = dsp56k_dasm_adc(op, opcode_str, arg_str);
	}
	/* ANDI : 0001 1EE0 iiii iiii : A-26 */
	/* (MoveP sneaks in here if you don't check 0x0600) */
	else if (((op & 0xf900) == 0x1800) & ((op & 0x0600) != 0x0000))
	{
		size = dsp56k_dasm_andi(op, opcode_str, arg_str);
	}
	/* ASL4 : 0001 0101 0011 F001 : A-30 */
	else if ((op & 0xfff7) == 0x1531)
	{
		size = dsp56k_dasm_asl4(op, opcode_str, arg_str);
	}
	/* ASR4 : 0001 0101 0011 F000 : A-34 */
	else if ((op & 0xfff7) == 0x1530)
	{
		size = dsp56k_dasm_asr4(op, opcode_str, arg_str);
	}
	/* ASR16 : 0001 0101 0111 F000 : A-36 */
	else if ((op & 0xfff7) == 0x1570)
	{
		size = dsp56k_dasm_asr16(op, opcode_str, arg_str);
	}
	/* BFCHG : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFCHG : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFCHG : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFCLR : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFCLR : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFCLR : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFSET : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFSET : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFSET : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_dasm_bfop(op, op2, opcode_str, arg_str);
	}
	/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
	else if (((op & 0xff30) == 0x0730) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_bcc(op, op2, opcode_str, arg_str, pc);
	}
	/* Bcc : 0010 11cc ccee eeee : A-48 */
	else if ((op & 0xfc00) == 0x2c00)
	{
		size = dsp56k_dasm_bcc_1(op, opcode_str, arg_str, pc);
	}
	/* Bcc : 0000 0111 RR10 cccc : A-48 */
	else if ((op & 0xff30) == 0x0720)
	{
		size = dsp56k_dasm_bcc_2(op, opcode_str, arg_str);
	}
	/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
	else if (((op & 0xfffc) == 0x013c) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_bra(op, op2, opcode_str, arg_str, pc);
	}
	/* BRA : 0000 1011 aaaa aaaa : A-50 */
	else if ((op & 0xff00) == 0x0b00)
	{
		size = dsp56k_dasm_bra_1(op, opcode_str, arg_str, pc);
	}
	/* BRA : 0000 0001 0010 11RR : A-50 */
	else if ((op & 0xfffc) == 0x012c)
	{
		size = dsp56k_dasm_bra_2(op, opcode_str, arg_str);
	}
	/* BRKc : 0000 0001 0001 cccc : A-52 */
	else if ((op & 0xfff0) == 0x0110)
	{
		size = dsp56k_dasm_brkcc(op, opcode_str, arg_str);
	}
	/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
	else if (((op & 0xff30) == 0x0710) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_bscc(op, op2, opcode_str, arg_str, pc);
	}
	/* BScc : 0000 0111 RR00 cccc : A-54 */
	else if ((op & 0xff30) == 0x0700)
	{
		size = dsp56k_dasm_bscc_1(op, opcode_str, arg_str);
	}
	/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
	else if (((op & 0xfffc) == 0x0138) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_bsr(op, op2, opcode_str, arg_str, pc);
	}
	/* BSR : 0000 0001 0010 10RR : A-56 */
	else if ((op & 0xfffc) == 0x0128)
	{
		size = dsp56k_dasm_bsr_1(op, opcode_str, arg_str);
	}
	/* CHKAAU : 0000 0000 0000 0100 : A-58 */
	else if ((op & 0xffff) == 0x0004)
	{
		size = dsp56k_dasm_chkaau(op, opcode_str, arg_str);
	}
	/* DEBUG : 0000 0000 0000 0001 : A-68 */
	else if ((op & 0xffff) == 0x0001)
	{
		size = dsp56k_dasm_debug(op, opcode_str, arg_str);
	}
	/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
	else if ((op & 0xfff0) == 0x0050)
	{
		size = dsp56k_dasm_debugcc(op, opcode_str, arg_str);
	}
	/* DIV : 0001 0101 0--0 F1DD : A-76 */
	else if ((op & 0xff94) == 0x1504)
	{
		size = dsp56k_dasm_div(op, opcode_str, arg_str);
	}
	/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
	else if ((op & 0xffd0) == 0x1590)
	{
		size = dsp56k_dasm_dmac(op, opcode_str, arg_str);
	}
	/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x00c0) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_do(op, op2, opcode_str, arg_str, pc);
	}
	/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xff00) == 0x0e00) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_do_1(op, op2, opcode_str, arg_str, pc);
	}
	/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x0400) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_do_2(op, op2, opcode_str, arg_str, pc);
	}
	/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
	else if (((op & 0xffff) == 0x0002) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_doforever(op, op2, opcode_str, arg_str, pc);
	}
	/* ENDDO : 0000 0000 0000 1001 : A-92 */
	else if ((op & 0xffff) == 0x0009)
	{
		size = dsp56k_dasm_enddo(op, opcode_str, arg_str);
	}
	/* EXT : 0001 0101 0101 F010 : A-96 */
	else if ((op & 0xfff7) == 0x1552)
	{
		size = dsp56k_dasm_ext(op, opcode_str, arg_str);
	}
	/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
	else if ((op & 0xffff) == 0x000f)
	{
		size = dsp56k_dasm_illegal(op, opcode_str, arg_str);
	}
	/* IMAC : 0001 0101 1010 FQQQ : A-100 */
	else if ((op & 0xfff0) == 0x15a0)
	{
		size = dsp56k_dasm_imac(op, opcode_str, arg_str);
	}
	/* IMPY : 0001 0101 1000 FQQQ : A-102 */
	else if ((op & 0xfff0) == 0x1580)
	{
		size = dsp56k_dasm_impy(op, opcode_str, arg_str);
	}
	/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
	else if (((op & 0xff30) == 0x0630) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_jcc(op, op2, opcode_str, arg_str);
	}
	/* Jcc : 0000 0110 RR10 cccc : A-108 */
	else if ((op & 0xff30) == 0x0620 )
	{
		size = dsp56k_dasm_jcc_1(op, opcode_str, arg_str);
	}
	/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
	else if (((op & 0xfffc) == 0x0134) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_jmp(op, op2, opcode_str, arg_str);
	}
	/* JMP : 0000 0001 0010 01RR : A-110 */
	else if ((op & 0xfffc) == 0x0124)
	{
		size = dsp56k_dasm_jmp_1(op, opcode_str, arg_str);
	}
	/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
	else if (((op & 0xff30) == 0x0610) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_jscc(op, op2, opcode_str, arg_str);
	}
	/* JScc : 0000 0110 RR00 cccc : A-112 */
	else if ((op & 0xff30) == 0x0600)
	{
		size = dsp56k_dasm_jscc_1(op, opcode_str, arg_str);
	}
	/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
	else if (((op & 0xfffc) == 0x0130) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_jsr(op, op2, opcode_str, arg_str);
	}
	/* JSR : 0000 1010 AAAA AAAA : A-114 */
	else if ((op & 0xff00) == 0x0a00)
	{
		size = dsp56k_dasm_jsr_1(op, opcode_str, arg_str);
	}
	/* JSR : 0000 0001 0010 00RR : A-114 */
	else if ((op & 0xfffc) == 0x0120)
	{
		size = dsp56k_dasm_jsr_2(op, opcode_str, arg_str);
	}
	/* LEA : 0000 0001 11TT MMRR : A-116 */
	else if ((op & 0xffc0) == 0x01c0)
	{
		size = dsp56k_dasm_lea(op, opcode_str, arg_str);
	}
	/* LEA : 0000 0001 10NN MMRR : A-116 */
	else if ((op & 0xffc0) == 0x0180)
	{
		size = dsp56k_dasm_lea_1(op, opcode_str, arg_str);
	}
	/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
	else if ((op & 0xfff0) == 0x15e0)
	{
		size = dsp56k_dasm_macsuuu(op, opcode_str, arg_str);
	}
	/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0x00ff) == 0x0011))
	{
		size = dsp56k_dasm_move_2(op, op2, opcode_str, arg_str);
	}
	/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
	else if ((op & 0xf810) == 0x3800)
	{
		size = dsp56k_dasm_movec(op, opcode_str, arg_str);
	}
	/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
	else if ((op & 0xf814) == 0x3810)
	{
		size = dsp56k_dasm_movec_1(op, opcode_str, arg_str);
	}
	/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
	else if ((op & 0xf816) == 0x3816)
	{
		size = dsp56k_dasm_movec_2(op, opcode_str, arg_str);
	}
	/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
	else if (((op & 0xf816) == 0x3814) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_dasm_movec_3(op, op2, opcode_str, arg_str);
	}
	/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
	else if ((op & 0xfc00) == 0x2800)
	{
		size = dsp56k_dasm_movec_4(op, opcode_str, arg_str);
	}
	/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xf810) == 0x3800))
	{
		size = dsp56k_dasm_movec_5(op, op2, opcode_str, arg_str);
	}
	/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
	else if ((op & 0xfc00) == 0x2000)
	{
		size = dsp56k_dasm_movei(op, opcode_str, arg_str);
	}
	/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
	else if ((op & 0xfe20) == 0x0200)
	{
		size = dsp56k_dasm_movem(op, opcode_str, arg_str);
	}
	/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
	else if ((op & 0xfe30) == 0x0230)
	{
		size = dsp56k_dasm_movem_1(op, opcode_str, arg_str);
	}
	/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xfe20) == 0x0200))
	{
		size = dsp56k_dasm_movem_2(op, op2, opcode_str, arg_str);
	}
	/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
	else if ((op & 0xfe20) == 0x1820)
	{
		size = dsp56k_dasm_movep(op, opcode_str, arg_str);
	}
	/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
	else if ((op & 0xfe00) == 0x0c00)
	{
		size = dsp56k_dasm_movep_1(op, opcode_str, arg_str);
	}
	/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
	else if ((op & 0xfe20) == 0x1800)
	{
		size = dsp56k_dasm_moves(op, opcode_str, arg_str);
	}
	/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
	else if ((op & 0xfff0) == 0x15c0)
	{
		size = dsp56k_dasm_mpysuuu(op, opcode_str, arg_str);
	}
	/* NEGC : 0001 0101 0110 F000 : A-168 */
	else if ((op & 0xfff7) == 0x1560)
	{
		size = dsp56k_dasm_negc(op, opcode_str, arg_str);
	}
	/* NOP : 0000 0000 0000 0000 : A-170 */
	else if ((op & 0xffff) == 0x0000)
	{
		size = dsp56k_dasm_nop(op, opcode_str, arg_str);
	}
	/* NORM : 0001 0101 0010 F0RR : A-172 */
	else if ((op & 0xfff4) == 0x1520)
	{
		size = dsp56k_dasm_norm(op, opcode_str, arg_str);
	}
	/* ORI : 0001 1EE1 iiii iiii : A-178 */
	else if ((op & 0xf900) == 0x1900)
	{
		size = dsp56k_dasm_ori(op, opcode_str, arg_str);
	}
	/* REP : 0000 0000 111- --RR : A-180 */
	else if ((op & 0xffe0) == 0x00e0)
	{
		size = dsp56k_dasm_rep(op, opcode_str, arg_str);
	}
	/* REP : 0000 1111 iiii iiii : A-180 */
	else if ((op & 0xff00) == 0x0f00)
	{
		size = dsp56k_dasm_rep_1(op, opcode_str, arg_str);
	}
	/* REP : 0000 0100 001D DDDD : A-180 */
	else if ((op & 0xffe0) == 0x0420)
	{
		size = dsp56k_dasm_rep_2(op, opcode_str, arg_str);
	}
	/* REPcc : 0000 0001 0101 cccc : A-184 */
	else if ((op & 0xfff0) == 0x0150)
	{
		size = dsp56k_dasm_repcc(op, opcode_str, arg_str);
	}
	/* RESET : 0000 0000 0000 1000 : A-186 */
	else if ((op & 0xffff) == 0x0008)
	{
		size = dsp56k_dasm_reset(op, opcode_str, arg_str);
	}
	/* RTI : 0000 0000 0000 0111 : A-194 */
	else if ((op & 0xffff) == 0x0007)
	{
		size = dsp56k_dasm_rti(op, opcode_str, arg_str);
	}
	/* RTS : 0000 0000 0000 0110 : A-196 */
	else if ((op & 0xffff) == 0x0006)
	{
		size = dsp56k_dasm_rts(op, opcode_str, arg_str);
	}
	/* STOP : 0000 0000 0000 1010 : A-200 */
	else if ((op & 0xffff) == 0x000a)
	{
		size = dsp56k_dasm_stop(op, opcode_str, arg_str);
	}
	/* SWAP : 0001 0101 0111 F001 : A-206 */
	else if ((op & 0xfff7) == 0x1571)
	{
		size = dsp56k_dasm_swap(op, opcode_str, arg_str);
	}
	/* SWI : 0000 0000 0000 0101 : A-208 */
	else if ((op & 0xffff) == 0x0005)
	{
		size = dsp56k_dasm_swi(op, opcode_str, arg_str);
	}
	/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
	else if ((op & 0xfc02) == 0x1000)
	{
		size = dsp56k_dasm_tcc(op, opcode_str, arg_str);
	}
	/* TFR(2) : 0001 0101 0000 F00J : A-214 */
	else if ((op & 0xfff6) == 0x1500)
	{
		size = dsp56k_dasm_tfr2(op, opcode_str, arg_str);
	}
	/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
	else if ((op & 0xfc00) == 0x2400)
	{
		size = dsp56k_dasm_tfr3(op, opcode_str, arg_str);
	}
	/* TST(2) : 0001 0101 0001 -1DD : A-220 */
	else if ((op & 0xfff4) == 0x1514)
	{
		size = dsp56k_dasm_tst2(op, opcode_str, arg_str);
	}
	/* WAIT : 0000 0000 0000 1011 : A-222 */
	else if ((op & 0xffff) == 0x000b)
	{
		size = dsp56k_dasm_wait(op, opcode_str, arg_str);
	}
	/* ZERO : 0001 0101 0101 F000 : A-224 */
	else if ((op & 0xfff7) == 0x1550)
	{
		size = dsp56k_dasm_zero(op, opcode_str, arg_str);
	}


	/* Assemble opcode string buffer */
	if (size >= 1)
	{
		pad_string(11, opcode_str);
		sprintf(buffer, "%s%s", opcode_str, arg_str);
	}
	/* Not recognized?  Nudge debugger onto the next word */
	else if (size == 0)
	{
		sprintf(buffer, "unknown");
		size = 1;
	}

	return (size | DASMFLAG_SUPPORTED);
}


/*******************************/
/* 32 Parallel move operations */
/*******************************/

/* ADD : 011m mKKK 0rru Fuuu : A-22 */
/* SUB : 011m mKKK 0rru Fuuu : A-202 */
static size_t dsp56k_dasm_addsub_2(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* TODO: How strange.  Same as SUB?  Investigate */
	char D[32];
	char S1[32];
	char arg[32];
	decode_uuuuF_table(BITS(op_byte,0x17), BITS(op_byte,0x08), arg, S1, D);
	sprintf(opcode_str, "%s", arg);
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
static size_t dsp56k_dasm_mac_1(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, S2, D);
	sprintf(opcode_str, "mac");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
static size_t dsp56k_dasm_macr_1(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, S2, D);
	sprintf(opcode_str, "macr");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
static size_t dsp56k_dasm_tfr_2(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* Note: This opcode collides with move_1 when F0DD is 0000.  Needs verifying on a real CPU. */
	char D[32];
	char S1[32];
	decode_DDF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "tfr");
	if (BITS(op_byte,0x0f) == 0x00)
		strcat(opcode_str, "*");	/* This asterisk is to denote that this opcode may have an error with its disassembly */
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
static size_t dsp56k_dasm_move_1(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* Note: This opcode collides with tfr_2 & will never be disassembled.  Needs verifying on a real CPU. */
	char D[32];
	char S1[32];
	decode_DDF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "tfr");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
static size_t dsp56k_dasm_mpy_1(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, S2, D);
	sprintf(opcode_str, "mpy");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
static size_t dsp56k_dasm_mpyr_1(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQF_table(BITS(op_byte,0x03), BITS(op_byte,0x08), S1, S2, D);
	sprintf(opcode_str, "mpyr");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MPY : 0001 0110 RRDD FQQQ : A-160 */
static size_t dsp56k_dasm_mpy_2(const UINT16 op_byte, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQQF_table(BITS(op_byte,0x0007), BITS(op_byte,0x0008), S1, S2, D);
	sprintf(opcode_str, "mpy");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* MAC : 0001 0111 RRDD FQQQ : A-122 */
static size_t dsp56k_dasm_mac_2(const UINT16 op_byte, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQQF_table(BITS(op_byte,0x0007), BITS(op_byte,0x0008), S1, S2, D);
	sprintf(opcode_str, "mac");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* CLR : .... .... 0000 F001 : A-60 */
static size_t dsp56k_dasm_clr(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "clr");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* ADD : .... .... 0000 FJJJ : A-22 */
static size_t dsp56k_dasm_add(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "add");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MOVE : .... .... 0001 0001 : A-128 */
static size_t dsp56k_dasm_move(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* Equivalent to a NOP (+ parallel move) */
	sprintf(opcode_str, "move");
	sprintf(arg_str, " ");
	sprintf(d_register, " ");
	return 1;
}

/* TFR : .... .... 0001 FJJJ : A-212 */
static size_t dsp56k_dasm_tfr(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "tfr");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* RND : .... .... 0010 F000 : A-188 */
static size_t dsp56k_dasm_rnd(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "rnd");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* TST : .... .... 0010 F001 : A-218 */
static size_t dsp56k_dasm_tst(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "tst");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* INC : .... .... 0010 F010 : A-104 */
static size_t dsp56k_dasm_inc(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "inc");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* INC24 : .... .... 0010 F011 : A-106 */
static size_t dsp56k_dasm_inc24(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "inc24");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* OR : .... .... 0010 F1JJ : A-176 */
static size_t dsp56k_dasm_or(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JJF_table(BITS(op_byte,0x03),BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "or");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* ASR : .... .... 0011 F000 : A-32 */
static size_t dsp56k_dasm_asr(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "asr");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* ASL : .... .... 0011 F001 : A-28 */
static size_t dsp56k_dasm_asl(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "asl");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* LSR : .... .... 0011 F010 : A-120 */
static size_t dsp56k_dasm_lsr(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "lsr");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* LSL : .... .... 0011 F011 : A-118 */
static size_t dsp56k_dasm_lsl(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "lsl");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* EOR : .... .... 0011 F1JJ : A-94 */
static size_t dsp56k_dasm_eor(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JJF_table(BITS(op_byte,0x03),BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "eor");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* SUBL : .... .... 0100 F001 : A-204 */
static size_t dsp56k_dasm_subl(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	sprintf(opcode_str, "subl");

	/* There is only one option for the F table.  This is a very strange opcode. */
	if (!BITS(op_byte,0x08))
	{
		sprintf(arg_str, "B,A");
		sprintf(d_register, "A");
	}
	else
	{
		sprintf(arg_str, "!,!");
		sprintf(d_register, "!");
	}

	return 1;
}

/* SUB : .... .... 0100 FJJJ : A-202 */
static size_t dsp56k_dasm_sub(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "sub");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* CLR24 : .... .... 0101 F001 : A-62 */
static size_t dsp56k_dasm_clr24(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "clr24");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* SBC : .... .... 0101 F01J : A-198 */
static size_t dsp56k_dasm_sbc(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JF_table(BITS(op_byte,0x01), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "sbc");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* CMP : .... .... 0101 FJJJ : A-64 */
static size_t dsp56k_dasm_cmp(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	/* Note: This is a JJJF limited in the docs, but other opcodes sneak
             in before cmp, so the same decode function can be used. */
	decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "cmp");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* NEG : .... .... 0110 F000 : A-166 */
static size_t dsp56k_dasm_neg(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "neg");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* NOT : .... .... 0110 F001 : A-174 */
static size_t dsp56k_dasm_not(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "not");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* DEC : .... .... 0110 F010 : A-72 */
static size_t dsp56k_dasm_dec(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "dec");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* DEC24 : .... .... 0110 F011 : A-74 */
static size_t dsp56k_dasm_dec24(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "dec24");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* AND : .... .... 0110 F1JJ : A-24 */
static size_t dsp56k_dasm_and(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	decode_JJF_table(BITS(op_byte,0x03),BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "and");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* ABS : .... .... 0111 F001 : A-18 */
static size_t dsp56k_dasm_abs(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "abs");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* ROR : .... .... 0111 F010 : A-192 */
static size_t dsp56k_dasm_ror(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "ror");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* ROL : .... .... 0111 F011 : A-190 */
static size_t dsp56k_dasm_rol(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	decode_F_table(BITS(op_byte,0x08), D);
	sprintf(opcode_str, "rol");
	sprintf(arg_str, "%s", D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* CMPM : .... .... 0111 FJJJ : A-66 */
static size_t dsp56k_dasm_cmpm(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	/* Note: This is a JJJF limited in the docs, but other opcodes sneak
             in before cmp, so the same decode function can be used. */
	decode_JJJF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, D);
	sprintf(opcode_str, "cmpm");
	sprintf(arg_str, "%s,%s", S1, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
static size_t dsp56k_dasm_mpy(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* There are inconsistencies with the S1 & S2 operand ordering in the docs,
       but since it's a multiply it doesn't matter */
	char D[32];
	char S1[32];
	char S2[32];
	char sign[32];
	decode_QQQF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, S2, D);
	decode_kSign_table(BITS(op_byte,0x40), sign);
	sprintf(opcode_str, "mpy");
	if (sign[0] == '-')
		sprintf(arg_str, "-%s,%s,%s", S1, S2, D);
	else
		sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MPYR : .... .... 1k01 FQQQ : A-162 */
static size_t dsp56k_dasm_mpyr(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* There are inconsistencies with the S1 & S2 operand ordering in the docs,
       but since it's a multiply it doesn't matter */
	char D[32];
	char S1[32];
	char S2[32];
	char sign[32];
	decode_QQQF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, S2, D);
	decode_kSign_table(BITS(op_byte,0x40), sign);
	sprintf(opcode_str, "mpyr");
	if (sign[0] == '-')
		sprintf(arg_str, "-%s,%s,%s", S1, S2, D);
	else
		sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MAC : .... .... 1k10 FQQQ : A-122 */
static size_t dsp56k_dasm_mac(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	char D[32];
	char S1[32];
	char S2[32];
	char sign[32];
	decode_QQQF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, S2, D);
	decode_kSign_table(BITS(op_byte,0x40), sign);
	sprintf(opcode_str, "mac");
	if (sign[0] == '-')
		sprintf(arg_str, "-%s,%s,%s", S1, S2, D);
	else
		sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}

/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
static size_t dsp56k_dasm_macr(const UINT16 op_byte, char* opcode_str, char* arg_str, char* d_register)
{
	/* There are inconsistencies with the S1 & S2 operand ordering in the docs,
       but since it's a multiply it doesn't matter */
	char D[32];
	char S1[32];
	char S2[32];
	char sign[32];
	decode_QQQF_table(BITS(op_byte,0x07), BITS(op_byte,0x08), S1, S2, D);
	decode_kSign_table(BITS(op_byte,0x40), sign);
	sprintf(opcode_str, "macr");
	if (sign[0] == '-')
		sprintf(arg_str, "-%s,%s,%s", S1, S2, D);
	else
		sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	sprintf(d_register, "%s", D);
	return 1;
}


/******************************/
/* Remaining non-parallel ops */
/******************************/

/* ADC : 0001 0101 0000 F01J : A-20 */
static size_t dsp56k_dasm_adc(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	decode_JF_table(BITS(op,0x0001), BITS(op,0x0008), S1, D);
	sprintf(opcode_str, "adc");
	sprintf(arg_str, "%s,%s", S1, D);
	return 1;
}

/* ANDI : 0001 1EE0 iiii iiii : A-26 */
static size_t dsp56k_dasm_andi(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_EE_table(BITS(op,0x0600), D);
	sprintf(opcode_str, "and(i)");
	sprintf(arg_str, "#$%02x,%s", BITS(op,0x00ff), D);
	return 1;
}

/* ASL4 : 0001 0101 0011 F001 : A-30 */
static size_t dsp56k_dasm_asl4(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "asl4");
	sprintf(arg_str, "%s", D);
	return 1;
}

/* ASR4 : 0001 0101 0011 F000 : A-34 */
static size_t dsp56k_dasm_asr4(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "asr4");
	sprintf(arg_str, "%s", D);
	return 1;
}

/* ASR16 : 0001 0101 0111 F000 : A-36 */
static size_t dsp56k_dasm_asr16(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "asr16");
	sprintf(arg_str, "%s", D);
	return 1;
}

/* BFCHG  : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
/* BFCHG  : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
/* BFCHG  : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
/* BFCLR  : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
/* BFCLR  : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
/* BFSET  : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
/* BFSET  : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_dasm_bfop(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
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

	if (upperMiddleLower != BBB_INVALID)
	{
		sprintf(arg_str, "#$%04x,%s", iVal, D);
	}
	else
	{
		sprintf(arg_str, "[[invalid]],%s", D);
	}

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

/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
static size_t dsp56k_dasm_bcc(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "b.%s", M);
	sprintf(arg_str, "$%04x (%d)", pc + 2 + (INT16)op2, (INT16)op2);
	return 2;
}

/* Bcc : 0010 11cc ccee eeee : A-48 */
static size_t dsp56k_dasm_bcc_1(const UINT16 op, char* opcode_str, char* arg_str, const offs_t pc)
{
	char M[32];
	INT8 relativeInt;
	decode_cccc_table(BITS(op,0x3c0), M);
	relativeInt = get_6_bit_signed_value(BITS(op,0x003f));
	sprintf(opcode_str, "b.%s", M);
	sprintf(arg_str, "$%04x (%d)", pc + 1 + relativeInt, relativeInt);
	return 1;
}

/* Bcc : 0000 0111 RR10 cccc : A-48 */
static size_t dsp56k_dasm_bcc_2(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	Rnum = decode_RR_table(BITS(op,0x00c0));
	sprintf(opcode_str, "b.%s", M);
	sprintf(arg_str, "R%d", Rnum);
	return 1;
}

/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
static size_t dsp56k_dasm_bra(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	sprintf(opcode_str, "bra");
	sprintf(arg_str, "$%04x (%d)", pc + 2 + op2, (INT16)op2);
	return 2;
}

/* BRA : 0000 1011 aaaa aaaa : A-50 */
static size_t dsp56k_dasm_bra_1(const UINT16 op, char* opcode_str, char* arg_str, const offs_t pc)
{
	INT8 iVal = (INT8)BITS(op,0x00ff);
	sprintf(opcode_str, "bra");
	sprintf(arg_str, "$%04x (%d)", pc + 1 + iVal, iVal);
	return 1;
}

/* BRA : 0000 0001 0010 11RR : A-50 */
static size_t dsp56k_dasm_bra_2(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "bra");
	sprintf(arg_str, "R%d", Rnum);
	return 1;
}

/* BRKc : 0000 0001 0001 cccc : A-52 */
static size_t dsp56k_dasm_brkcc(const UINT16 op, char* opcode_str, char* arg_str)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "brk.%s", M);
	sprintf(arg_str, " ");
	return 1;
}

/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
static size_t dsp56k_dasm_bscc(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "bs.%s", M);
	sprintf(arg_str, "$%04x (%d)", pc + 2 + (INT16)op2, (INT16)op2);
	return (2 | DASMFLAG_STEP_OVER);
}

/* BScc: 0000 0111 RR00 cccc : A-54 */
static size_t dsp56k_dasm_bscc_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	Rnum = decode_RR_table(BITS(op,0x00c0));
	sprintf(opcode_str, "bs.%s", M);
	sprintf(arg_str, "R%d", Rnum);
	return (1 | DASMFLAG_STEP_OVER);
}

/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
static size_t dsp56k_dasm_bsr(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	sprintf(opcode_str, "bsr");
	sprintf(arg_str, "$%04x (%d)", pc + 2 + (INT16)op2, (INT16)op2);
	return (2 | DASMFLAG_STEP_OVER);
}

/* BSR : 0000 0001 0010 10RR : A-56 */
static size_t dsp56k_dasm_bsr_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "bsr");
	sprintf(arg_str, "R%d", Rnum);
	return (1 | DASMFLAG_STEP_OVER);
}

/* CHKAAU : 0000 0000 0000 0100 : A-58 */
static size_t dsp56k_dasm_chkaau(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "chkaau");
	sprintf(arg_str, " ");
	return 1;
}

/* DEBUG : 0000 0000 0000 0001 : A-68 */
static size_t dsp56k_dasm_debug(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "debug");
	sprintf(arg_str, " ");
	return 1;
}

/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
static size_t dsp56k_dasm_debugcc(const UINT16 op, char* opcode_str, char* arg_str)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "debug.%s", M);
	sprintf(arg_str, " ");
	return 1;
}

/* DIV : 0001 0101 0--0 F1DD : A-76 */
static size_t dsp56k_dasm_div(const UINT16 op, char* opcode_str, char* arg_str)
{
	/* The docs on page A-76 claim there is potential for a parallel move here,
       but various other sources (including elsewhere in the family manual) disagree */
	char D[32];
	char S1[32];
	decode_DDF_table(BITS(op,0x0003), BITS(op,0x0008), S1, D);
	sprintf(opcode_str, "div");
	sprintf(arg_str, "%s,%s", S1, D);
	return 1;
}

/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
static size_t dsp56k_dasm_dmac(const UINT16 op, char* opcode_str, char* arg_str)
{
	char A[32];
	char D[32];
	char S1[32];
	char S2[32];
	decode_ss_table(BITS(op,0x0024), A);
	decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D);     /* Special QQF */
	sprintf(opcode_str, "dmac(%s)", A);
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_dasm_do(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "do");
	sprintf(arg_str, "X:(R%d),$%02x", Rnum, pc + 2 + op2);
	return 2;
}

/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_dasm_do_1(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	sprintf(opcode_str, "do");
	sprintf(arg_str, "#$%02x,$%04x", BITS(op,0x00ff), pc + 2 + op2);
	return 2;
}

/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_dasm_do_2(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	char S1[32];
	decode_DDDDD_table(BITS(op,0x001f), S1);
	sprintf(opcode_str, "do");
	sprintf(arg_str, "%s,$%04x", S1, pc + 2 + op2);
	return 2;
}

/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
static size_t dsp56k_dasm_doforever(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str, const offs_t pc)
{
	sprintf(opcode_str, "do forever");
	sprintf(arg_str, "$%04x", pc + 2 + op2);
	return 2;
}

/* ENDDO : 0000 0000 0000 1001 : A-92 */
static size_t dsp56k_dasm_enddo(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "enddo");
	sprintf(arg_str, " ");
	return 1;
}

/* EXT : 0001 0101 0101 F010 : A-96 */
static size_t dsp56k_dasm_ext(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "ext");
	sprintf(arg_str, "%s", D);
	return 1;
}

/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
static size_t dsp56k_dasm_illegal(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "illegal");
	sprintf(arg_str, " ");
	return 1;
}

/* IMAC : 0001 0101 1010 FQQQ : A-100 */
static size_t dsp56k_dasm_imac(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQQF_table(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D);
	sprintf(opcode_str, "imac");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* IMPY : 0001 0101 1000 FQQQ : A-102 */
static size_t dsp56k_dasm_impy(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	char S2[32];
	decode_QQQF_table(BITS(op,0x0007), BITS(op,0x0008), S1, S2, D);
	sprintf(opcode_str, "impy");
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
static size_t dsp56k_dasm_jcc(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "j.%s", M);
	sprintf(arg_str, "$%04x", op2);
	return 2;
}

/* Jcc : 0000 0110 RR10 cccc : A-108 */
static size_t dsp56k_dasm_jcc_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	Rnum = decode_RR_table(BITS(op,0x00c0));
	sprintf(opcode_str, "j.%s", M);
	sprintf(arg_str, "R%d", Rnum);
	return 1;
}

/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
static size_t dsp56k_dasm_jmp(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "jmp");
	sprintf(arg_str, "$%04x", op2);
	return 2;
}

/* JMP : 0000 0001 0010 01RR : A-110 */
static size_t dsp56k_dasm_jmp_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "jmp");
	sprintf(arg_str, "R%d", Rnum);
	return 1;
}

/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
static size_t dsp56k_dasm_jscc(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "js.%s", M);
	sprintf(arg_str, "$%04x", op2);
	return (2 | DASMFLAG_STEP_OVER);
}

/* JScc : 0000 0110 RR00 cccc : A-112 */
static size_t dsp56k_dasm_jscc_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	Rnum = decode_RR_table(BITS(op,0x00c0));
	sprintf(opcode_str, "js.%s", M);
	sprintf(arg_str, "R%d", Rnum);
	return (1 | DASMFLAG_STEP_OVER);
}

/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
static size_t dsp56k_dasm_jsr(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "jsr");
	sprintf(arg_str, "$%04x", op2);
	return (2 | DASMFLAG_STEP_OVER);
}

/* JSR : 0000 1010 AAAA AAAA : A-114 */
static size_t dsp56k_dasm_jsr_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "jsr");
	sprintf(arg_str, "#$%02x", BITS(op,0x00ff));
	return (1 | DASMFLAG_STEP_OVER);
}

/* JSR : 0000 0001 0010 00RR : A-114 */
static size_t dsp56k_dasm_jsr_2(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "jsr");
	sprintf(arg_str, "R%d", Rnum);
	return (1 | DASMFLAG_STEP_OVER);
}

/* LEA : 0000 0001 11TT MMRR : A-116 */
static size_t dsp56k_dasm_lea(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	int Tnum;
	char ea[32];
	Tnum = decode_TT_table(BITS(op,0x0030));
	Rnum = decode_RR_table(BITS(op,0x0003));
	assemble_ea_from_MM_table(BITS(op,0x000c), Rnum, ea);
	sprintf(opcode_str, "lea");
	sprintf(arg_str, "%s,R%d", ea, Tnum);
	return 1;
}

/* LEA : 0000 0001 10NN MMRR : A-116 */
static size_t dsp56k_dasm_lea_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Nnum;
	int Rnum;
	char ea[32];
	Nnum = decode_NN_table(BITS(op,0x0030));
	Rnum = decode_RR_table(BITS(op,0x0003));
	assemble_ea_from_MM_table(BITS(op,0x000c), Rnum, ea);
	sprintf(opcode_str, "lea");
	sprintf(arg_str, "%s,N%d", ea, Nnum);
	return 1;
}

/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
static size_t dsp56k_dasm_macsuuu(const UINT16 op, char* opcode_str, char* arg_str)
{
	char A[32];
	char D[32];
	char S1[32];
	char S2[32];
	decode_s_table(BITS(op,0x0004), A);
	decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D);		/* Special QQF */
	sprintf(opcode_str, "mac(%s)", A);
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
static size_t dsp56k_dasm_move_2(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	INT8 B;
	char SD[32];
	char args[32];
	B = BITS(op,0x00ff);
	decode_HHH_table(BITS(op2,0x0e00), SD);
	assemble_reg_from_W_table(BITS(op2,0x0100), args, 'X', SD, B);
	sprintf(opcode_str, "move");
	sprintf(arg_str, "%s", args);
	return 2;
}

/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
static size_t dsp56k_dasm_movec(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char ea[32];
	char SD[32];
	char args[32];
	decode_DDDDD_table(BITS(op,0x03e0), SD);
	Rnum = decode_RR_table(BITS(op,0x0003));
	assemble_ea_from_MM_table(BITS(op,0x000c), Rnum, ea);
	assemble_arguments_from_W_table(BITS(op,0x0400), args, 'X', SD, ea);
	sprintf(opcode_str, "move(c)");
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
static size_t dsp56k_dasm_movec_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char ea[32];
	char SD[32];
	char args[32];
	decode_DDDDD_table(BITS(op,0x03e0), SD);
	Rnum = decode_RR_table(BITS(op,0x0003));
	assemble_ea_from_q_table(BITS(op,0x0008), Rnum, ea);
	assemble_arguments_from_W_table(BITS(op,0x0400), args, 'X', SD, ea);
	sprintf(opcode_str, "move(c)");
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
static size_t dsp56k_dasm_movec_2(const UINT16 op, char* opcode_str, char* arg_str)
{
	char ea[32];
	char SD[32];
	char args[32];
	decode_DDDDD_table(BITS(op,0x03e0), SD);
	decode_Z_table(BITS(op,0x0008), ea);
	assemble_arguments_from_W_table(BITS(op,0x0400), args, 'X', SD, ea);
	sprintf(opcode_str, "move(c)");
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
static size_t dsp56k_dasm_movec_3(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	char ea[32];
	char SD[32];
	char args[32];
	decode_DDDDD_table(BITS(op,0x03e0), SD);
	assemble_ea_from_t_table(BITS(op,0x0008), op2, ea);
	if (BITS(op,0x0400))									/* order fixed - 02/03/05 */
		sprintf(args, "%s,%s", ea, SD);
	else
		sprintf(args, "%s,%s", SD, ea);
	sprintf(opcode_str, "move(c)");
	sprintf(arg_str, "%s", args);
	return 2;
}

/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
static size_t dsp56k_dasm_movec_4(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D1[32];
	char S1[32];
	decode_DDDDD_table(BITS(op,0x03e0), S1);
	decode_DDDDD_table(BITS(op,0x001f), D1);
	sprintf(opcode_str, "move(c)");
	sprintf(arg_str, "%s,%s", S1, D1);
	return 1;
}

/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
static size_t dsp56k_dasm_movec_5(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	INT8 B;
	char SD[32];
	char args[32];
	B = BITS(op,0x00ff);
	decode_DDDDD_table(BITS(op2,0x03e0), SD);
	assemble_reg_from_W_table(BITS(op2,0x0400), args, 'X', SD, B);
	sprintf(opcode_str, "move(c)");
	sprintf(arg_str, "%s", args);
	return 2;
}

/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
static size_t dsp56k_dasm_movei(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D1[32];
	decode_DD_table(BITS(op,0x0300), D1);
	sprintf(opcode_str, "move(i)");
	sprintf(arg_str, "#$%02x,%s", BITS(op,0x00ff), D1);
	return 1;
}

/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
static size_t dsp56k_dasm_movem(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char ea[32];
	char SD[32];
	char args[32];
	Rnum = decode_RR_table(BITS(op,0x00c0));
	decode_HHH_table(BITS(op,0x0007), SD);
	assemble_ea_from_MM_table(BITS(op,0x0018), Rnum, ea);
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'P', SD, ea);
	sprintf(opcode_str, "move(m)");
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
static size_t dsp56k_dasm_movem_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	char ea[32];
	char SD[32];
	char ea2[32];
	char args[32];
	assemble_eas_from_m_table(BITS(op,0x000c), BITS(op,0x00c0), BITS(op,0x0003), ea, ea2);
	sprintf(SD, "P:%s", ea);
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea2);
	sprintf(opcode_str, "move(m)*");	/* This asterisk is to denote that this opcode may have an error with its disassembly */
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
static size_t dsp56k_dasm_movem_2(const UINT16 op, const UINT16 op2, char* opcode_str, char* arg_str)
{
	INT8 B;
	char SD[32];
	char args[32];
	B = BITS(op,0x00ff);
	decode_HHH_table(BITS(op2,0x0007), SD);
	assemble_reg_from_W_table(BITS(op2,0x0100), args, 'P', SD, B);
	sprintf(opcode_str, "move(m)");
	sprintf(arg_str, "%s", args);
	return 2;
}

/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
static size_t dsp56k_dasm_movep(const UINT16 op, char* opcode_str, char* arg_str)
{
	char A[32];
	char SD[32];
	char args[32];
	char fullAddy[128];		/* Convert Short Absolute Address to full 16-bit */
	decode_HH_table(BITS(op,0x00c0), SD);
	assemble_address_from_IO_short_address(BITS(op,0x001f), fullAddy);
	sprintf(A, "$%s", fullAddy);
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, A);
	sprintf(opcode_str, "move(p)");
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
static size_t dsp56k_dasm_movep_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char ea[32];
	char SD[32];
	char args[32];
	char fullAddy[128];		/* Convert Short Absolute Address to full 16-bit */
	Rnum = decode_RR_table(BITS(op,0x00c0));
	assemble_ea_from_m_table(BITS(op,0x0020), Rnum, ea);
	assemble_address_from_IO_short_address(BITS(op,0x001f), fullAddy);
	sprintf(SD, "X:$%s", fullAddy);
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea);
	sprintf(opcode_str, "move(p)*");	/* This asterisk is to denote that this opcode may have an error with its disassembly */
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
static size_t dsp56k_dasm_moves(const UINT16 op, char* opcode_str, char* arg_str)
{
	char A[32];
	char SD[32];
	char args[32];
	decode_HH_table(BITS(op,0x00c0), SD);
	sprintf(A, "$%02x", BITS(op,0x001f));
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, A);
	sprintf(opcode_str, "move(s)");
	sprintf(arg_str, "%s", args);
	return 1;
}

/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
static size_t dsp56k_dasm_mpysuuu(const UINT16 op, char* opcode_str, char* arg_str)
{
	char A[32];
	char D[32];
	char S1[32];
	char S2[32];
	decode_s_table(BITS(op,0x0004), A);
	decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), S1, S2, D);     /* Special QQF */
	sprintf(opcode_str, "mpy(%s)", A);
	sprintf(arg_str, "%s,%s,%s", S1, S2, D);
	return 1;
}

/* NEGC : 0001 0101 0110 F000 : A-168 */
static size_t dsp56k_dasm_negc(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "negc");
	sprintf(arg_str, "%s", D);
	return 1;
}

/* NOP : 0000 0000 0000 0000 : A-170 */
static size_t dsp56k_dasm_nop(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "nop");
	sprintf(arg_str, " ");
	return 1;
}

/* NORM : 0001 0101 0010 F0RR : A-172 */
static size_t dsp56k_dasm_norm(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "norm");
	sprintf(arg_str, "R%d,%s", Rnum, D);
	return 1;
}

/* ORI : 0001 1EE1 iiii iiii : A-178 */
static size_t dsp56k_dasm_ori(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_EE_table(BITS(op,0x0600), D);
	sprintf(opcode_str, "or(i)");
	sprintf(arg_str, "#$%02x,%s", BITS(op,0x00ff), D);
	return 1;
}

/* REP : 0000 0000 111- --RR : A-180 */
static size_t dsp56k_dasm_rep(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0003));
	sprintf(opcode_str, "rep");
	sprintf(arg_str, "X:(R%d)", Rnum);
	return 1;
}

/* REP : 0000 1111 iiii iiii : A-180 */
static size_t dsp56k_dasm_rep_1(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "rep");
	sprintf(arg_str, "#$%02x (%d)", BITS(op,0x00ff), BITS(op,0x00ff));
	return 1;
}

/* REP : 0000 0100 001D DDDD : A-180 */
static size_t dsp56k_dasm_rep_2(const UINT16 op, char* opcode_str, char* arg_str)
{
	char S1[32];
	decode_DDDDD_table(BITS(op,0x001f), S1);
	sprintf(opcode_str, "rep");
	sprintf(arg_str, "%s", S1);
	return 1;
}

/* REPcc : 0000 0001 0101 cccc : A-184 */
static size_t dsp56k_dasm_repcc(const UINT16 op, char* opcode_str, char* arg_str)
{
	char M[32];
	decode_cccc_table(BITS(op,0x000f), M);
	sprintf(opcode_str, "rep.%s", M);
	sprintf(arg_str, " ");
	return 1;
}

/* RESET : 0000 0000 0000 1000 : A-186 */
static size_t dsp56k_dasm_reset(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "reset");
	sprintf(arg_str, " ");
	return 1;
}

/* RTI : 0000 0000 0000 0111 : A-194 */
static size_t dsp56k_dasm_rti(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "rti");
	sprintf(arg_str, " ");
	return (1 | DASMFLAG_STEP_OUT);
}

/* RTS : 0000 0000 0000 0110 : A-196 */
static size_t dsp56k_dasm_rts(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "rts");
	sprintf(arg_str, " ");
	return (1 | DASMFLAG_STEP_OUT);
}

/* STOP : 0000 0000 0000 1010 : A-200 */
static size_t dsp56k_dasm_stop(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "stop");
	sprintf(arg_str, " ");
	return 1;
}

/* SWAP : 0001 0101 0111 F001 : A-206 */
static size_t dsp56k_dasm_swap(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "swap");
	sprintf(arg_str, "%s", D);
	return 1;
}

/* SWI : 0000 0000 0000 0101 : A-208 */
static size_t dsp56k_dasm_swi(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "swi");
	sprintf(arg_str, " ");
	return 1;
}

/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
static size_t dsp56k_dasm_tcc(const UINT16 op, char* opcode_str, char* arg_str)
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

	/* Note: S == 'A' && D == 'A' is used by the assembler when "no Data ALU
       transfer is specified in the instruction."  The Data ALU contains the
       X,Y,A,B registers.  The AGU holds the R registers.  This comment means
       the assembler will create Tcc A,A when it wants to conditionally transfer
       only a R register to another R register. */
	return 1;
}

/* TFR(2) : 0001 0101 0000 F00J : A-214 */
static size_t dsp56k_dasm_tfr2(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	char S1[32];
	decode_JF_table(BITS(op,0x0001),BITS(op,0x0008), D, S1);
	sprintf(opcode_str, "tfr2");
	sprintf(arg_str, "%s,%s", S1, D);
	return 1;
}

/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
static size_t dsp56k_dasm_tfr3(const UINT16 op, char* opcode_str, char* arg_str)
{
	int Rnum;
	char ea[32];
	char D1[32];
	char S1[32];
	char SD[32];
	char args[32];
	decode_DDF_table(BITS(op,0x0030), BITS(op,0x0008), D1, S1);		/* Intentionally switched */
	decode_HHH_table(BITS(op,0x0007), SD);
	Rnum = decode_RR_table(BITS(op,0x00c0));
	assemble_ea_from_m_table(BITS(op,0x0200), Rnum, ea);
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea);
	sprintf(opcode_str, "tfr3");
	sprintf(arg_str, "%s,%s %s", S1, D1, args);
	return 1;
}

/* TST(2) : 0001 0101 0001 -1DD : A-220 */
static size_t dsp56k_dasm_tst2(const UINT16 op, char* opcode_str, char* arg_str)
{
	char S1[32];
	decode_DD_table(BITS(op,0x0003), S1);
	sprintf(opcode_str, "tst2");
	sprintf(arg_str, "%s", S1);
	return 1;
}

/* WAIT : 0000 0000 0000 1011 : A-222 */
static size_t dsp56k_dasm_wait(const UINT16 op, char* opcode_str, char* arg_str)
{
	sprintf(opcode_str, "wait");
	sprintf(arg_str, " ");
	return 1;
}

/* ZERO : 0001 0101 0101 F000 : A-224 */
static size_t dsp56k_dasm_zero(const UINT16 op, char* opcode_str, char* arg_str)
{
	char D[32];
	decode_F_table(BITS(op,0x0008), D);
	sprintf(opcode_str, "zero");
	sprintf(arg_str, "%s", D);
	return 1;
}



/**********************************/
/* Parallel memory move functions */
/**********************************/
/* X Memory Data Move : 1mRR HHHW ---- ---- : A-137 */
static void decode_x_memory_data_move(const UINT16 op, char* parallel_move_str)
{
	int Rnum;
	char SD[32];
	char ea[32];
	char args[32];

	Rnum = decode_RR_table(BITS(op,0x3000));
	decode_HHH_table(BITS(op,0x0e00), SD);
	assemble_ea_from_m_table(BITS(op,0x4000), Rnum, ea);
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, ea);

	sprintf(parallel_move_str, "%s", args);
}

/* X Memory Data Move : 0101 HHHW ---- ---- : A-137 */
static void decode_x_memory_data_move2(const UINT16 op, char* parallel_move_str, char* d_register)
{
	char SD[32] ;
	char args[32] ;
	char dest[32] ;

	if (d_register[0] == 'B')
		sprintf(dest, "(A1)");
	else if (d_register[0] == 'A')
		sprintf(dest, "(B1)");
	else
		sprintf(dest, "(A1)");

	decode_HHH_table(BITS(op,0x0e00), SD) ;
	assemble_arguments_from_W_table(BITS(op,0x0100), args, 'X', SD, dest) ;

	sprintf(parallel_move_str, "%s", args);
}

/* Dual X Memory Data Read : 011m mKKK -rr- ---- : A-142*/
static void decode_dual_x_memory_data_read(const UINT16 op, char* parallel_move_str, char* parallel_move_str2, char* d_register)
{
	int Rnum;
	char D1[32] = "";
	char D2[32] = "";
	char ea1[32] = "";
	char ea2[32] = "";

	Rnum = decode_rr_table(BITS(op,0x0060));
	decode_KKK_table(BITS(op,0x0700), D1, D2);
	assemble_eas_from_m_table(BITS(op,0x1800), Rnum, 3, ea1, ea2);

	/* Not documented, but extrapolated from docs on page A-133 */
	if (D1[0] == '^' && D1[1] == 'F')
	{
		if (d_register[0] == 'B')
			sprintf(D1, "A");
		else if (d_register[0] == 'A')
			sprintf(D1, "B");
		else
			sprintf(D1, "A");	/* In the case of no data ALU operation */
	}

	/* D1 and D2 may not specify the same register : A-142 */
	if (Rnum == 3)
	{
		/* Replace the R3 with !! */
		ea1[1] = ea1[2] = '!';
	}

	sprintf(parallel_move_str,  "X:%s,%s", ea1, D1);
	sprintf(parallel_move_str2, "X:%s,%s", ea2, D2);
}

/* Register to Register Data Move : 0100 IIII ---- ---- : A-133 */
static void decode_register_to_register_data_move(const UINT16 op, char* parallel_move_str, char* d_register)
{
	char S[32];
	char D[32];

	decode_IIII_table(BITS(op,0x0f00), S, D);

	if (S[0] == 'F')
	{
		S[0] = d_register[0];
		S[1] = 0x00;
	}

	if (D[0] == '^' && D[1] == 'F')
	{
		if (d_register[0] == 'B')
			sprintf(D, "A");
		else if (d_register[0] == 'A')
			sprintf(D, "B");
		else
			sprintf(D, "A");	/* In the case of no data ALU operation */
	}

	sprintf(parallel_move_str, "%s,%s", S, D);
}

/* Address Register Update : 0011 0zRR ---- ---- : A-135 */
static void decode_address_register_update(const UINT16 op, char* parallel_move_str)
{
	char ea[32];
	int Rnum;
	Rnum = decode_RR_table(BITS(op,0x0300));
	assemble_ea_from_z_table(BITS(op,0x0400), Rnum, ea);
	sprintf(parallel_move_str, "%s", ea);
}

/* X Memory Data Write and Register Data Move : 0001 011k RRDD ---- : A-140 */
static void decode_x_memory_data_write_and_register_data_move(const UINT16 op, char* parallel_move_str, char* parallel_move_str2)
{
	int Rnum;
	char S[32];
	char Dnot[32];

	decode_k_table(BITS(op,0x0100), Dnot);
	Rnum = decode_RR_table(BITS(op,0x00c0));
	decode_DD_table(BITS(op,0x0030), S);

	sprintf(parallel_move_str,  "%s,X:(R%d)+N%d", Dnot, Rnum, Rnum);
	sprintf(parallel_move_str2, "%s,%s", S, Dnot);
}

/* X Memory Data Move with short displacement : 0000 0101 BBBB BBBB ---- HHHW ---- ---- : A-139 */
static void decode_x_memory_data_move_with_short_displacement(const UINT16 op, const UINT16 op2, char* parallel_move_str)
{
	INT8 B;
	char SD[32];
	char args[32];

	B = (char)(op & 0x00ff);
	decode_HHH_table(BITS(op2,0x0e00), SD);
	assemble_reg_from_W_table(BITS(op2,0x0100), args, 'X', SD, B);
	sprintf(parallel_move_str, "%s", args);
}


/******************/
/* Table decoding */
/******************/

static int decode_BBB_table(UINT16 BBB)
{
	switch(BBB)
	{
		case 0x4: return BBB_UPPER;
		case 0x2: return BBB_MIDDLE;
		case 0x1: return BBB_LOWER;
	}

	return BBB_INVALID;
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
		case 0xa: sprintf(S, "?");  sprintf(D, "?");  break;
		case 0xb: sprintf(S, "?");  sprintf(D, "?");  break;
		case 0xc: sprintf(S, "A");  sprintf(D, "X1"); break;
		case 0xd: sprintf(S, "B");  sprintf(D, "Y1"); break;
		case 0xe: sprintf(S, "A0"); sprintf(D, "X1"); break;
		case 0xf: sprintf(S, "B0"); sprintf(D, "Y1"); break;
	}
}

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

static void decode_k_table(UINT16 k, char *Dnot)
{
	switch(k)
	{
		case 0x0: sprintf(Dnot, "B"); break;
		case 0x1: sprintf(Dnot, "A"); break;
	}
}

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

static int decode_NN_table(UINT16 NN)
{
	return NN;
}

static int decode_TT_table(UINT16 TT)
{
	return TT;
}

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
	return rr;
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

	sprintf(D, "sub?");
	sprintf(S, "add");
	sprintf(arg, "invalid");

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
		case 0x0: sprintf(ea, "X:$%04x", val); break;
		case 0x1: sprintf(ea, "#$%04x", val);  break;
	}
}

static void assemble_ea_from_z_table(UINT16 z, int n, char *ea)
{
	switch(z)
	{
		case 0x0: sprintf(ea, "(R%d)-",    n)   ; break;
		case 0x1: sprintf(ea, "(R%d)+N%d", n, n); break;
	}
}

static void assemble_D_from_P_table(UINT16 P, UINT16 ppppp, char *D)
{
	char fullAddy[128];		/* Convert Short Absolute Address to full 16-bit */

	switch(P)
	{
		case 0x0: sprintf(D, "X:$%02x", ppppp); break;
		case 0x1:
			assemble_address_from_IO_short_address(ppppp, fullAddy);
			sprintf(D, "X:$%s", fullAddy);
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

static void assemble_reg_from_W_table(UINT16 W, char *args, char ma, char *SD, INT8 xx)
{
	UINT8 abs_xx;
	char operation[32];

	if(xx < 0)
		sprintf(operation,"-");
	else
		sprintf(operation,"+");

	abs_xx = abs(xx);

	switch(W)
	{
		case 0x0: sprintf(args, "%s,%c:(R2%s$%02x)", SD, ma, operation, abs_xx); break;
		case 0x1: sprintf(args, "%c:(R2%s$%02x),%s", ma, operation, abs_xx, SD); break;
	}
}

static void assemble_address_from_IO_short_address(UINT16 pp, char *ea)
{
	UINT16 fullAddy = 0xffe0;
	fullAddy |= pp;

	sprintf(ea, "%.04x", fullAddy);
}

static INT8 get_6_bit_signed_value(UINT16 bits)
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

static void pad_string(const int dest_length, char* string)
{
	while (strlen(string) < dest_length)
	{
		strcat(string, " ");
	}
}

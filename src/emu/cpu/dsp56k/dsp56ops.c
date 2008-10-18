/***************************************************************************

    dsp56ops.c
    Core implementation for the portable Motorola/Freescale DSP56k emulator.
    Written by Andrew Gardner

***************************************************************************/

// NOTES For register setting:
// FM.3-4 : When A2 or B2 is read, the register contents occupy the low-order portion
//          (bits 7-0) of the word; the high-order portion (bits 16-8) is sign-extended. When A2 or B2
//          is written, the register receives the low-order portion of the word; the high-order portion is not used
//        : ...much more!
//        : ...shifter/limiter/overflow notes too.
//
//

/************************/
/* Datatypes and macros */
/************************/
enum dataType { DT_BYTE, 
				DT_WORD, 
				DT_DOUBLE_WORD, 
				DT_LONG_WORD };

struct _typed_pointer
{
	void* addr;
	char  data_type;
};
typedef struct _typed_pointer typed_pointer;


#define WORD(X) (X<<1)
#define BITS(CUR,MASK) (Dsp56kOpMask(CUR,MASK))



/*********************/
/* Opcode prototypes */
/*********************/
static size_t dsp56k_op_add_2	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_mac_1	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_macr_1	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_move_1	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_mpy_1	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_mpyr_1	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_sub_1	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_tfr_2	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_mpy_2	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_mac_2	 (const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_clr		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_add		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_move	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_tfr		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_rnd		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_tst		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_inc		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_inc24	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_or		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_asr		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_asl		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_lsr		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_lsl		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_eor		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_subl	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_sub		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_clr24	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_sbc		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_cmp		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_neg		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_not		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_dec		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_dec24	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_and		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_abs		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_ror		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_rol		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_cmpm	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_mpy		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_mpyr	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_mac		 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_macr	 (const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_adc		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_andi	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_asl4	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_asr4	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_asr16	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bfop	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bfop_1	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bfop_2	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bcc		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bcc_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bcc_2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bra		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bra_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bra_2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_brkc	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bscc	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bscc_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bsr		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bsr_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_chkaau	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_debug	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_debugcc	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_div		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_dmac	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_do		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_do_1	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_do_2	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_doforever(const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_enddo	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_ext		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_illegal	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_imac	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_impy	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jcc		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jcc_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jmp		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jmp_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jscc	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jscc_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jsr		 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jsr_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jsr_2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_lea		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_lea_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_macsuuu	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_move_2	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movec	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_3	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movec_4	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_5	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movei	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movem	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movem_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movem_2	 (const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movep	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movep_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_moves	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_mpysuuu	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_negc	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_nop		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_norm	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_ori		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rep		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rep_1	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rep_2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_repcc	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_reset	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rti		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rts		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_stop	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_swap	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_swi		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tcc		 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tfr2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tfr3	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tst2	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_wait	 (const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_zero	 (const UINT16 op, UINT8* cycles);


static void execute_register_to_register_data_move(const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value);
static void execute_x_memory_data_move (const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value);
static void execute_x_memory_data_move2(const UINT16 op, typed_pointer* d_register);


static UINT16 decode_BBB_bitmask(UINT16 BBB, UINT16 *iVal);
static int decode_cccc_table(UINT16 cccc);
static void decode_DDDDD_table(UINT16 DDDDD, typed_pointer* ret);
static void decode_DD_table(UINT16 DD, typed_pointer* ret);
static void decode_F_table(UINT16 F, typed_pointer* ret);
static void decode_h0hF_table(UINT16 h0h, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void decode_HH_table(UINT16 HH, typed_pointer* ret);
static void decode_HHH_table(UINT16 HHH, typed_pointer* ret);
static void decode_IIII_table(UINT16 IIII, typed_pointer* src_ret, typed_pointer* dst_ret, void *working);
static void decode_JJJF_table(UINT16 JJJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void decode_JJF_table(UINT16 JJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void decode_QQF_special_table(UINT16 QQ, UINT16 F, void **S1, void **S2, void **D);
static void decode_RR_table(UINT16 RR, typed_pointer* ret);
#ifdef UNUSED_FUNCTION
static void decode_Z_table(UINT16 Z, typed_pointer* ret);
#endif
static void execute_m_table(int x, UINT16 m);
static void execute_MM_table(UINT16 rnum, UINT16 MM);
#ifdef UNUSED_FUNCTION
static UINT16 execute_q_table(int x, UINT16 q);
static void execute_z_table(int x, UINT16 z);
static UINT16 assemble_D_from_P_table(UINT16 P, UINT16 ppppp);
#endif
static UINT16 assemble_address_from_IO_short_address(UINT16 pp);
#ifdef UNUSED_FUNCTION
static UINT16 assemble_address_from_6bit_signed_relative_short_address(UINT16 srs);
#endif

static void dsp56k_process_loop(void);
static void dsp56k_process_rep(size_t repSize);



/********************/
/* Helper Functions */
/********************/
static UINT16 Dsp56kOpMask(UINT16 op, UINT16 mask);

/* These arguments are written source->destination to fall in line with the processor's paradigm. */
static void SetDestinationValue(typed_pointer source, typed_pointer dest);

static void SetDataMemoryValue(typed_pointer source, UINT32 destinationAddr);
static void SetProgramMemoryValue(typed_pointer source, UINT32 destinationAddr);



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void execute_one(void)
{
	UINT16 op;
	UINT16 op2;
	size_t size = 0x1337;
	UINT8 cycle_count = 0;

	/* For MAME */
	debugger_instruction_hook(Machine, PC);
	OP = ROPCODE(WORD(PC));

	/* The words we're going to be working with */
	op = ROPCODE(WORD(PC));
	op2 = ROPCODE(WORD(PC) + WORD(1));


	/* DECODE */
	/* Dual X Memory Data Read : 011m mKKK -rr- ---- : A-142*/
	if ((op & 0xe000) == 0x6000)
	{
		/* Quote: (MOVE, MAC(R), MPY(R), ADD, SUB, TFR) */
		UINT16 op_byte = op & 0x00ff;

		/* ADD : 011m mKKK 0rru Fuuu : A-22 */
		if ((op & 0xe080) == 0x6000)
		{
			size = dsp56k_op_add_2(op_byte, &cycle_count);
		}
		/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
		else if ((op & 0xe094) == 0x6084)
		{
			size = dsp56k_op_mac_1(op_byte, &cycle_count);
		}
		/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
		else if ((op & 0xe094) == 0x6094)
		{
			size = dsp56k_op_macr_1(op_byte, &cycle_count);
		}
		/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
		else if ((op & 0xe09f) == 0x6010)
		{
			size = dsp56k_op_move_1(op_byte, &cycle_count);
		}
		/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
		else if ((op & 0xe094) == 0x6080)
		{
			size = dsp56k_op_mpy_1(op_byte, &cycle_count);
		}
		/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
		else if ((op & 0xe094) == 0x6090)
		{
			size = dsp56k_op_mpyr_1(op_byte, &cycle_count);
		}
		/* SUB : 011m mKKK 0rru Fuuu : A-202 */
		else if ((op & 0xe080) == 0x6000)
		{
			size = dsp56k_op_sub_1(op_byte, &cycle_count);
		}
		/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
		else if ((op & 0xe094) == 0x6010)
		{
			size = dsp56k_op_tfr_2(op_byte, &cycle_count);
		}

		/* Now evaluate the parallel data move */
		// TODO // decode_dual_x_memory_data_read(op, parallel_move_str, parallel_move_str2);
	}
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD ---- : A-140 */
	else if ((op & 0xfe00) == 0x1600)
	{
		/* Quote: (MPY or MAC) */
		UINT16 op_byte = op & 0x00ff;

		/* MPY : 0001 0110 RRDD FQQQ : A-160 */
		if ((op & 0xff00) == 0x1600)
		{
			size = dsp56k_op_mpy_2(op_byte, &cycle_count);
		}
		/* MAC : 0001 0111 RRDD FQQQ : A-122 */
		else if ((op & 0xff00) == 0x1700)
		{
			size = dsp56k_op_mac_2(op_byte, &cycle_count);
		}

		/* Now evaluate the parallel data move */
		// TODO // decode_x_memory_data_write_and_register_data_move(op, parallel_move_str, parallel_move_str2);
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

		int parallelType = -1;
		UINT16 op_byte = 0x0000;
		typed_pointer d_register = {NULL, DT_BYTE};
		UINT64 prev_accum_value = U64(0x0000000000000000);

		/* Note: it's important that NPDM comes before RtRDM here */
		/* No Parallel Data Move : 0100 1010 ---- ---- : A-131 */
		if ((op & 0xff00) == 0x4a00)
		{
			op_byte = op & 0x00ff;
			parallelType = kNoParallelDataMove;
		}
		/* Register to Register Data Move : 0100 IIII ---- ---- : A-133 */
		else if ((op & 0xf000) == 0x4000)
		{
			op_byte = op & 0x00ff;
			parallelType = kRegisterToRegister;
		}
		/* Address Register Update : 0011 0zRR ---- ---- : A-135 */
		else if ((op & 0xf800) == 0x3000)
		{
			op_byte = op & 0x00ff;
			parallelType = kAddressRegister;
		}
		/* X Memory Data Move : 1mRR HHHW ---- ---- : A-137 */
		else if ((op & 0x8000) == 0x8000)
		{
			op_byte = op & 0x00ff;
			parallelType = kXMemoryDataMove;
		}
		/* X Memory Data Move : 0101 HHHW ---- ---- : A-137 */
		else if ((op & 0xf000) == 0x5000)
		{
			op_byte = op & 0x00ff;
			parallelType = kXMemoryDataMove2;
		}
		/* X Memory Data Move with short displacement : 0000 0101 BBBB BBBB ---- HHHW ---- ---- : A-139 */
		else if ((op & 0xff00) == 0x0500)
		{
			/* See notes at top of file! */
			/* op_byte = op2 & 0x00ff; */
			/* parallelType = kXMemoryDataMoveWithDisp; */
			/* DO NOTHING FOR NOW */
		}


		if (parallelType != -1)
		{
			/* Note: There is much overlap between opcodes down here */
			/*       To this end, certain ops must come before others in the list */

			/* CLR : .... .... 0000 F001 : A-60 */
			if ((op_byte & 0x00f7) == 0x0001)
			{
				size = dsp56k_op_clr(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ADD : .... .... 0000 FJJJ : A-22 */
			else if ((op_byte & 0x00f0) == 0x0000)
			{
				size = dsp56k_op_add(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* MOVE : .... .... 0001 0001 : A-128 */
			else if ((op_byte & 0x00ff) == 0x0011)
			{
				size = dsp56k_op_move(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* TFR : .... .... 0001 FJJJ : A-212 */
			else if ((op_byte & 0x00f0) == 0x0010)
			{
				size = dsp56k_op_tfr(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* RND : .... .... 0010 F000 : A-188 */
			else if ((op_byte & 0x00f7) == 0x0020)
			{
				size = dsp56k_op_rnd(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* TST : .... .... 0010 F001 : A-218 */
			else if ((op_byte & 0x00f7) == 0x0021)
			{
				size = dsp56k_op_tst(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* INC : .... .... 0010 F010 : A-104 */
			else if ((op_byte & 0x00f7) == 0x0022)
			{
				size = dsp56k_op_inc(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* INC24 : .... .... 0010 F011 : A-106 */
			else if ((op_byte & 0x00f7) == 0x0023)
			{
				size = dsp56k_op_inc24(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* OR : .... .... 0010 F1JJ : A-176 */
			else if ((op_byte & 0x00f4) == 0x0024)
			{
				size = dsp56k_op_or(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* ASR : .... .... 0011 F000 : A-32 */
			else if ((op_byte & 0x00f7) == 0x0030)
			{
				size = dsp56k_op_asr(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ASL : .... .... 0011 F001 : A-28 */
			else if ((op_byte & 0x00f7) == 0x0031)
			{
				size = dsp56k_op_asl(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* LSR : .... .... 0011 F010 : A-120 */
			else if ((op_byte & 0x00f7) == 0x0032)
			{
				size = dsp56k_op_lsr(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* LSL : .... .... 0011 F011 : A-118 */
			else if ((op_byte & 0x00f7) == 0x0033)
			{
				size = dsp56k_op_lsl(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* EOR : .... .... 0011 F1JJ : A-94 */
			else if ((op_byte & 0x00f4) == 0x0034)
			{
				size = dsp56k_op_eor(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* SUBL : .... .... 0100 F001 : A-204 */
			else if ((op_byte & 0x00f7) == 0x0041)
			{
				size = dsp56k_op_subl(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* SUB : .... .... 0100 FJJJ : A-202 */
			else if ((op_byte & 0x00f0) == 0x0040)
			{
				size = dsp56k_op_sub(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* CLR24 : .... .... 0101 F001 : A-62 */
			else if ((op_byte & 0x00f7) == 0x0051)
			{
				size = dsp56k_op_clr24(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* SBC : .... .... 0101 F01J : A-198 */
			else if ((op_byte & 0x00f6) == 0x0052)
			{
				size = dsp56k_op_sbc(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* CMP : .... .... 0101 FJJJ : A-64 */
			else if ((op_byte & 0x00f0) == 0x0050)
			{
				size = dsp56k_op_cmp(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* NEG : .... .... 0110 F000 : A-166 */
			else if ((op_byte & 0x00f7) == 0x0060)
			{
				size = dsp56k_op_neg(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* NOT : .... .... 0110 F001 : A-174 */
			else if ((op_byte & 0x00f7) == 0x0061)
			{
				size = dsp56k_op_not(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* DEC : .... .... 0110 F010 : A-72 */
			else if ((op_byte & 0x00f7) == 0x0062)
			{
				size = dsp56k_op_dec(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* DEC24 : .... .... 0110 F011 : A-74 */
			else if ((op_byte & 0x00f7) == 0x0063)
			{
				size = dsp56k_op_dec24(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* AND : .... .... 0110 F1JJ : A-24 */
			else if ((op_byte & 0x00f4) == 0x0064)
			{
				size = dsp56k_op_and(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* ABS : .... .... 0111 F001 : A-18 */
			if ((op_byte & 0x00f7) == 0x0071)
			{
				size = dsp56k_op_abs(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ROR : .... .... 0111 F010 : A-192 */
			else if ((op_byte & 0x00f7) == 0x0072)
			{
				size = dsp56k_op_ror(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ROL : .... .... 0111 F011 : A-190 */
			else if ((op_byte & 0x00f7) == 0x0073)
			{
				size = dsp56k_op_rol(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* CMPM : .... .... 0111 FJJJ : A-66 */
			else if ((op_byte & 0x00f0) == 0x0070)
			{
				size = dsp56k_op_cmpm(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
			else if ((op_byte & 0x00b0) == 0x0080)
			{
				size = dsp56k_op_mpy(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MPYR : .... .... 1k01 FQQQ : A-162 */
			else if ((op_byte & 0x00b0) == 0x0090)
			{
				size = dsp56k_op_mpyr(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MAC : .... .... 1k10 FQQQ : A-122 */
			else if ((op_byte & 0x00b0) == 0x00a0)
			{
				size = dsp56k_op_mac(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
			else if ((op_byte & 0x00b0) == 0x00b0)
			{
				size = dsp56k_op_macr(op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* Now evaluate the parallel data move */
			switch (parallelType)
			{
			case kNoParallelDataMove:
				/* DO NOTHING */
				break;
			case kRegisterToRegister:
				execute_register_to_register_data_move(op, &d_register, &prev_accum_value);
				break;
			case kAddressRegister:
				// TODO // decode_address_register_update(op, parallel_move_str);
				break;
			case kXMemoryDataMove:
				execute_x_memory_data_move(op, &d_register, &prev_accum_value);
				break;
			case kXMemoryDataMove2:
				execute_x_memory_data_move2(op, &d_register);
				break;
			case kXMemoryDataMoveWithDisp:
				// TODO // decode_x_memory_data_move_with_short_displacement(op, op2, parallel_move_str);
				break;
			}
		}
	}

	/* Drop out if you've already completed your work. */
	if (size != 0x1337)
	{
		PC += size;
		change_pc(PC);

		dsp56k_process_loop();
		dsp56k_process_rep(size);

		dsp56k_icount -= 4;	/* Temporarily hard-coded at 4 clocks per opcode */	/* cycle_count */
		return;
	}


	/******************************/
	/* Remaining non-parallel ops */
	/******************************/

	/* ADC : 0001 0101 0000 F01J : A-20 */
	if ((op & 0xfff6) == 0x1502)
	{
		size = dsp56k_op_adc(op, &cycle_count);
	}
	/* ANDI : 0001 1EE0 iiii iiii : A-26 */
	/* (MoveP sneaks in here if you don't check 0x0600) */
	else if (((op & 0xf900) == 0x1800) & ((op & 0x0600) != 0x0000))
	{
		size = dsp56k_op_andi(op, &cycle_count);
	}
	/* ASL4 : 0001 0101 0011 F001 : A-30 */
	else if ((op & 0xfff7) == 0x1531)
	{
		size = dsp56k_op_asl4(op, &cycle_count);
	}
	/* ASR4 : 0001 0101 0011 F000 : A-34 */
	else if ((op & 0xfff7) == 0x1530)
	{
		size = dsp56k_op_asr4(op, &cycle_count);
	}
	/* ASR16 : 0001 0101 0111 F000 : A-36 */
	else if ((op & 0xfff7) == 0x1570)
	{
		size = dsp56k_op_asr16(op, &cycle_count);
	}
	/* BFCHG : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_op_bfop(op, op2, &cycle_count);
	}
	/* BFCHG : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_op_bfop_1(op, op2, &cycle_count);
	}
	/* BFCHG : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_op_bfop_2(op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_op_bfop(op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_op_bfop_1(op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_op_bfop_2(op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_op_bfop(op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_op_bfop_1(op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_op_bfop_2(op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_op_bfop(op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_op_bfop_1(op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_op_bfop_2(op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_op_bfop(op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_op_bfop_1(op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_op_bfop_2(op, op2, &cycle_count);
	}
	/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
	else if (((op & 0xff30) == 0x0730) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bcc(op, op2, &cycle_count);
	}
	/* Bcc : 0010 11cc ccee eeee : A-48 */
	else if ((op & 0xfc00) == 0x2c00)
	{
		size = dsp56k_op_bcc_1(op, &cycle_count);
	}
	/* Bcc : 0000 0111 RR10 cccc : A-48 */
	else if ((op & 0xff30) == 0x0720)
	{
		size = dsp56k_op_bcc_2(op, &cycle_count);
	}
	/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
	else if (((op & 0xfffc) == 0x013c) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bra(op, op2, &cycle_count);
	}
	/* BRA : 0000 1011 aaaa aaaa : A-50 */
	else if ((op & 0xff00) == 0x0b00)
	{
		size = dsp56k_op_bra_1(op, &cycle_count);
	}
	/* BRA : 0000 0001 0010 11RR : A-50 */
	else if ((op & 0xfffc) == 0x012c)
	{
		size = dsp56k_op_bra_2(op, &cycle_count);
	}
	/* BRKc : 0000 0001 0001 cccc : A-52 */
	else if ((op & 0xfff0) == 0x0110)
	{
		size = dsp56k_op_brkc(op, &cycle_count);
	}
	/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
	else if (((op & 0xff30) == 0x0710) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bscc(op, op2, &cycle_count);
	}
	/* BScc : 0000 0111 RR00 cccc : A-54 */
	else if ((op & 0xff30) == 0x0700)
	{
		size = dsp56k_op_bscc_1(op, &cycle_count);
	}
	/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
	else if (((op & 0xfffc) == 0x0138) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bsr(op, op2, &cycle_count);
	}
	/* BSR : 0000 0001 0010 10RR : A-56 */
	else if ((op & 0xfffc) == 0x0128)
	{
		size = dsp56k_op_bsr_1(op, &cycle_count);
	}
	/* CHKAAU : 0000 0000 0000 0100 : A-58 */
	else if ((op & 0xffff) == 0x0004)
	{
		size = dsp56k_op_chkaau(op, &cycle_count);
	}
	/* DEBUG : 0000 0000 0000 0001 : A-68 */
	else if ((op & 0xffff) == 0x0001)
	{
		size = dsp56k_op_debug(op, &cycle_count);
	}
	/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
	else if ((op & 0xfff0) == 0x0050)
	{
		size = dsp56k_op_debugcc(op, &cycle_count);
	}
	/* DIV : 0001 0101 0--0 F1DD : A-76 */
	/* WARNING : DOCS SAY THERE IS A PARALLEL MOVE HERE !!! */
	else if ((op & 0xff94) == 0x1504)
	{
		size = dsp56k_op_div(op, &cycle_count);
	}
	/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
	else if ((op & 0xffd0) == 0x1590)
	{
		size = dsp56k_op_dmac(op, &cycle_count);
	}
	/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x00c0) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_do(op, op2, &cycle_count);
	}
	/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xff00) == 0x0e00) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_do_1(op, op2, &cycle_count);
	}
	/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x0400) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_do_2(op, op2, &cycle_count);
	}
	/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
	else if (((op & 0xffff) == 0x0002) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_doforever(op, op2, &cycle_count);
	}
	/* ENDDO : 0000 0000 0000 1001 : A-92 */
	else if ((op & 0xffff) == 0x0009)
	{
		size = dsp56k_op_enddo(op, &cycle_count);
	}
	/* EXT : 0001 0101 0101 F010 : A-96 */
	else if ((op & 0xfff7) == 0x1552)
	{
		size = dsp56k_op_ext(op, &cycle_count);
	}
	/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
	else if ((op & 0xffff) == 0x000f)
	{
		size = dsp56k_op_illegal(op, &cycle_count);
	}
	/* IMAC : 0001 0101 1010 FQQQ : A-100 */
	else if ((op & 0xfff0) == 0x15a0)
	{
		size = dsp56k_op_imac(op, &cycle_count);
	}
	/* IMPY : 0001 0101 1000 FQQQ : A-102 */
	else if ((op & 0xfff0) == 0x1580)
	{
		size = dsp56k_op_impy(op, &cycle_count);
	}
	/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
	else if (((op & 0xff30) == 0x0630) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jcc(op, op2, &cycle_count);
	}
	/* Jcc : 0000 0110 RR10 cccc : A-108 */
	else if ((op & 0xff30) == 0x0620 )
	{
		size = dsp56k_op_jcc_1(op, &cycle_count);
	}
	/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
	else if (((op & 0xfffc) == 0x0134) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jmp(op, op2, &cycle_count);
	}
	/* JMP : 0000 0001 0010 01RR : A-110 */
	else if ((op & 0xfffc) == 0x0124)
	{
		size = dsp56k_op_jmp_1(op, &cycle_count);
	}
	/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
	else if (((op & 0xff30) == 0x0610) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jscc(op, op2, &cycle_count);
	}
	/* JScc : 0000 0110 RR00 cccc : A-112 */
	else if ((op & 0xff30) == 0x0600)
	{
		size = dsp56k_op_jscc_1(op, &cycle_count);
	}
	/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
	else if (((op & 0xfffc) == 0x0130) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jsr(op, op2, &cycle_count);
	}
	/* JSR : 0000 1010 AAAA AAAA : A-114 */
	else if ((op & 0xff00) == 0x0a00)
	{
		size = dsp56k_op_jsr_1(op, &cycle_count);
	}
	/* JSR : 0000 0001 0010 00RR : A-114 */
	else if ((op & 0xfffc) == 0x0120)
	{
		size = dsp56k_op_jsr_2(op, &cycle_count);
	}
	/* LEA : 0000 0001 11TT MMRR : A-116 */
	else if ((op & 0xffc0) == 0x01c0)
	{
		size = dsp56k_op_lea(op, &cycle_count);
	}
	/* LEA : 0000 0001 10NN MMRR : A-116 */
	else if ((op & 0xffc0) == 0x0180)
	{
		size = dsp56k_op_lea_1(op, &cycle_count);
	}
	/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
	else if ((op & 0xfff0) == 0x15e0)
	{
		size = dsp56k_op_macsuuu(op, &cycle_count);
	}
	/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0x00ff) == 0x0011))
	{
		size = dsp56k_op_move_2(op, op2, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
	else if ((op & 0xf810) == 0x3800)
	{
		size = dsp56k_op_movec(op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
	else if ((op & 0xf814) == 0x3810)
	{
		size = dsp56k_op_movec_1(op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
	else if ((op & 0xf816) == 0x3816)
	{
		size = dsp56k_op_movec_2(op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
	else if (((op & 0xf816) == 0x3814) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_movec_3(op, op2, &cycle_count);
	}
	/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
	else if ((op & 0xfc00) == 0x2800)
	{
		size = dsp56k_op_movec_4(op, &cycle_count);
	}
	/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xf810) == 0x3800))
	{
		size = dsp56k_op_movec_5(op, op2, &cycle_count);
	}
	/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
	else if ((op & 0xfc00) == 0x2000)
	{
		size = dsp56k_op_movei(op, &cycle_count);
	}
	/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
	else if ((op & 0xfe20) == 0x0200)
	{
		size = dsp56k_op_movem(op, &cycle_count);
	}
	/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
	else if ((op & 0xfe30) == 0x0230)
	{
		size = dsp56k_op_movem_1(op, &cycle_count);
	}
	/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xfe20) == 0x0200))
	{
		size = dsp56k_op_movem_2(op, op2, &cycle_count);
	}
	/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
	else if ((op & 0xfe20) == 0x1820)
	{
		size = dsp56k_op_movep(op, &cycle_count);
	}
	/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
	else if ((op & 0xfe00) == 0x0c00)
	{
		size = dsp56k_op_movep_1(op, &cycle_count);
	}
	/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
	else if ((op & 0xfe20) == 0x1800)
	{
		size = dsp56k_op_moves(op, &cycle_count);
	}
	/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
	else if ((op & 0xfff0) == 0x15c0)
	{
		size = dsp56k_op_mpysuuu(op, &cycle_count);
	}
	/* NEGC : 0001 0101 0110 F000 : A-168 */
	else if ((op & 0xfff7) == 0x1560)
	{
		size = dsp56k_op_negc(op, &cycle_count);
	}
	/* NOP : 0000 0000 0000 0000 : A-170 */
	else if ((op & 0xffff) == 0x0000)
	{
		size = dsp56k_op_nop(op, &cycle_count);
	}
	/* NORM : 0001 0101 0010 F0RR : A-172 */
	else if ((op & 0xfff4) == 0x1520)
	{
		size = dsp56k_op_norm(op, &cycle_count);
	}
	/* ORI : 0001 1EE1 iiii iiii : A-178 */
	else if ((op & 0xf900) == 0x1900)
	{
		size = dsp56k_op_ori(op, &cycle_count);
	}
	/* REP : 0000 0000 111- --RR : A-180 */
	else if ((op & 0xffe0) == 0x00e0)
	{
		size = dsp56k_op_rep(op, &cycle_count);
	}
	/* REP : 0000 1111 iiii iiii : A-180 */
	else if ((op & 0xff00) == 0x0f00)
	{
		size = dsp56k_op_rep_1(op, &cycle_count);
	}
	/* REP : 0000 0100 001D DDDD : A-180 */
	else if ((op & 0xffe0) == 0x0420)
	{
		size = dsp56k_op_rep_2(op, &cycle_count);
	}
	/* REPcc : 0000 0001 0101 cccc : A-184 */
	else if ((op & 0xfff0) == 0x0150)
	{
		size = dsp56k_op_repcc(op, &cycle_count);
	}
	/* RESET : 0000 0000 0000 1000 : A-186 */
	else if ((op & 0xffff) == 0x0008)
	{
		size = dsp56k_op_reset(op, &cycle_count);
	}
	/* RTI : 0000 0000 0000 0111 : A-194 */
	else if ((op & 0xffff) == 0x0007)
	{
		size = dsp56k_op_rti(op, &cycle_count);
	}
	/* RTS : 0000 0000 0000 0110 : A-196 */
	else if ((op & 0xffff) == 0x0006)
	{
		size = dsp56k_op_rts(op, &cycle_count);
	}
	/* STOP : 0000 0000 0000 1010 : A-200 */
	else if ((op & 0xffff) == 0x000a)
	{
		size = dsp56k_op_stop(op, &cycle_count);
	}
	/* SWAP : 0001 0101 0111 F001 : A-206 */
	else if ((op & 0xfff7) == 0x1571)
	{
		size = dsp56k_op_swap(op, &cycle_count);
	}
	/* SWI : 0000 0000 0000 0101 : A-208 */
	else if ((op & 0xffff) == 0x0005)
	{
		size = dsp56k_op_swi(op, &cycle_count);
	}
	/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
	else if ((op & 0xfc02) == 0x1000)
	{
		size = dsp56k_op_tcc(op, &cycle_count);
	}
	/* TFR(2) : 0001 0101 0000 F00J : A-214 */
	else if ((op & 0xfff6) == 0x1500)
	{
		size = dsp56k_op_tfr2(op, &cycle_count);
	}
	/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
	else if ((op & 0xfc00) == 0x2400)
	{
		size = dsp56k_op_tfr3(op, &cycle_count);
	}
	/* TST(2) : 0001 0101 0001 -1DD : A-220 */
	else if ((op & 0xfff4) == 0x1514)
	{
		size = dsp56k_op_tst2(op, &cycle_count);
	}
	/* WAIT : 0000 0000 0000 1011 : A-222 */
	else if ((op & 0xffff) == 0x000b)
	{
		size = dsp56k_op_wait(op, &cycle_count);
	}
	/* ZERO : 0001 0101 0101 F000 : A-224 */
	else if ((op & 0xfff7) == 0x1550)
	{
		size = dsp56k_op_zero(op, &cycle_count);
	}


	/* Not recognized?  Nudge debugger onto the next word */
	if (size == 0x1337)
	{
		logerror("DSP56k: Unimplemented opcode at 0x%04x : %04x\n", PC, op);
		size = 1 ;						// Just to get the debugger past the bad opcode
	}

	/* Must have been a good opcode */
	PC += size;
	change_pc(PC);

	dsp56k_process_loop();
	dsp56k_process_rep(size);

	dsp56k_icount -= 4;	/* Temporarily hard-coded at 4 clocks per opcode */	/* cycle_count */
}




/***************************************************************************
	Opcode implementations
***************************************************************************/

/*******************************/
/* 32 Parallel move operations */
/*******************************/

/* ADD : 011m mKKK 0rru Fuuu : A-22 */
static size_t dsp56k_op_add_2(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
static size_t dsp56k_op_mac_1(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
static size_t dsp56k_op_macr_1(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
static size_t dsp56k_op_move_1(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
static size_t dsp56k_op_mpy_1(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
static size_t dsp56k_op_mpyr_1(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* SUB : 011m mKKK 0rru Fuuu : A-202 */
static size_t dsp56k_op_sub_1(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
static size_t dsp56k_op_tfr_2(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* MPY : 0001 0110 RRDD FQQQ : A-160 */
static size_t dsp56k_op_mpy_2(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MAC : 0001 0111 RRDD FQQQ : A-122 */
static size_t dsp56k_op_mac_2(const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* CLR : .... .... 0000 F001 : A-60 */
static size_t dsp56k_op_clr(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_LONG_WORD};
	typed_pointer clear = {NULL, DT_LONG_WORD};
	UINT64 clear_val = U64(0x0000000000000000);

	decode_F_table(BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	clear.addr = &clear_val;
	clear.data_type = DT_LONG_WORD;
	SetDestinationValue(clear, D);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * 0 - */
	/* TODO - S&L */
	E_bit_set(0);
	U_bit_set(1);
	N_bit_set(0);
	Z_bit_set(1);
	V_bit_set(0);

	cycles += 2;	/* TODO: + mv oscillator clock cycles */
	return 1;
}

/* ADD : .... .... 0000 FJJJ : A-22 */
static size_t dsp56k_op_add(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT64 addVal = 0;

	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};
	decode_JJJF_table(BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((UINT64*)D.addr);

	switch(S.data_type)
	{
		case DT_WORD:        addVal = (UINT64)*((UINT16*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: addVal = (UINT64)*((UINT32*)S.addr);       break;
		case DT_LONG_WORD:   addVal = (UINT64)*((UINT64*)S.addr);       break;
	}

	/* TODO: Verify : Sign-extend everyone for proper addition op */
	if (addVal &  U64(0x0000000080000000))
		addVal |= U64(0xffffffff00000000);

	/* Operate*/
	*((UINT64*)D.addr) += addVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U, V, C */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) N_bit_set(1);
	if (*((UINT64*)D.addr) == 0) Z_bit_set(1);

	cycles += 2;		/* TODO: + mv oscillator cycles */
	return 1;
}

/* MOVE : .... .... 0001 0001 : A-128 */
static size_t dsp56k_op_move(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* TFR : .... .... 0001 FJJJ : A-212 */
static size_t dsp56k_op_tfr(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* RND : .... .... 0010 F000 : A-188 */
static size_t dsp56k_op_rnd(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* TST : .... .... 0010 F001 : A-218 */
static size_t dsp56k_op_tst(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* 0 * * * * * 0 0 */
	return 0;
}

/* INC : .... .... 0010 F010 : A-104 */
static size_t dsp56k_op_inc(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* INC24 : .... .... 0010 F011 : A-106 */
static size_t dsp56k_op_inc24(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * ? * * */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	return 0;
}

/* OR : .... .... 0010 F1JJ : A-176 */
static size_t dsp56k_op_or(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
    /* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	return 0;
}

/* ASR : .... .... 0011 F000 : A-32 */
static size_t dsp56k_op_asr(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	*((UINT64*)D.addr) = (*((UINT64*)D.addr)) >> 1;

	/* Make sure the MSB is maintained */
	if (*p_accum & U64(0x0000008000000000))
		*((UINT64*)D.addr) |= U64(0x0000008000000000);
	else
		*((UINT64*)D.addr) &= (~U64(0x0000008000000000));

	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * 0 ? */
	/* TODO: S, L, E, U */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) N_bit_set(1);
	if (*((UINT64*)D.addr) == 0) Z_bit_set(1);
	V_bit_set(0);
	if (*p_accum & U64(0x0000000000000001)) C_bit_set(1);

	cycles += 2;		/* TODO: + mv oscillator cycles */
	return 1;
}

/* ASL : .... .... 0011 F001 : A-28 */
static size_t dsp56k_op_asl(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * ? ? */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significant
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
    /* C - Set if bit 39 of source operand is set. Cleared otherwise. */
	return 0;
}

/* LSR : .... .... 0011 F010 : A-120 */
static size_t dsp56k_op_lsr(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(BITS(op_byte,0x0008), &D);
	
	*p_accum = *((UINT64*)D.addr);
	
	((PAIR64*)D.addr)->w.h = (((PAIR64*)D.addr)->w.h) >> 1;
	
	/* Make sure bit 31 gets a 0 */
	((PAIR64*)D.addr)->w.h &= (~0x8000);
	
	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;
	
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* TODO: S, L */
	N_bit_set(0);
	if (((PAIR64*)D.addr)->w.h == 0) Z_bit_set(1);
	V_bit_set(0);
	if (*p_accum & U64(0x0000000000010000)) C_bit_set(1);
	
	cycles += 2;		/* TODO: + mv oscillator cycles */
	return 1;
}

/* LSL : .... .... 0011 F011 : A-118 */
static size_t dsp56k_op_lsl(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 31 of the source operand is set. Cleared otherwise. */
	return 0;
}

/* EOR : .... .... 0011 F1JJ : A-94 */
static size_t dsp56k_op_eor(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	return 0;
}

/* SUBL : .... .... 0100 F001 : A-204 */
static size_t dsp56k_op_subl(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * ? * */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significant
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	return 0;
}

/* SUB : .... .... 0100 FJJJ : A-202 */
static size_t dsp56k_op_sub(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* CLR24 : .... .... 0101 F001 : A-62 */
static size_t dsp56k_op_clr24(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * ? 0 - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	return 0;
}

/* SBC : .... .... 0101 F01J : A-198 */
static size_t dsp56k_op_sbc(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* CMP : .... .... 0101 FJJJ : A-64 */
static size_t dsp56k_op_cmp(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT64 cmpVal = 0;
	UINT64 result = 0;

	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJJF_table(BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((UINT64*)D.addr);

	switch(S.data_type)
	{
		case DT_WORD:        cmpVal = (UINT64)*((UINT16*)S.addr);  break;
		case DT_DOUBLE_WORD: cmpVal = (UINT64)*((UINT32*)S.addr);  break;
		case DT_LONG_WORD:   cmpVal = (UINT64)*((UINT64*)S.addr);  break;
	}

	result = *((UINT64*)D.addr) - cmpVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U, N, V, C */
	if (result == 0) Z_bit_set(1);		/* TODO: Do you clear it if this isn't true? */


	cycles += 2;		/* TODO: + mv oscillator clock cycles */
	return 1;
}

/* NEG : .... .... 0110 F000 : A-166 */
static size_t dsp56k_op_neg(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* NOT : .... .... 0110 F001 : A-174 */
static size_t dsp56k_op_not(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	/* Invert bits [16:31] of D */
	((PAIR64*)D.addr)->w.h = ~(((PAIR64*)D.addr)->w.h);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S?, L */
	if ( *((UINT64*)D.addr) & U64(0x0000000080000000))		 N_bit_set(1);
	if ((*((UINT64*)D.addr) & U64(0x00000000ffff0000)) == 0) Z_bit_set(1);
	V_bit_set(0);

	cycles += 2;		/* TODO: + mv oscillator cycles */
	return 1;
}

/* DEC : .... .... 0110 F010 : A-72 */
static size_t dsp56k_op_dec(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* DEC24 : .... .... 0110 F011 : A-74 */
static size_t dsp56k_op_dec24(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT32 workBits24;

	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);

	/* TODO: I wonder if workBits24 should be signed? */
	workBits24 = ((*((UINT64*)D.addr)) & U64(0x000000ffffff0000)) >> 16;
	workBits24--;
	workBits24 &= 0x00ffffff;		/* Solves -x issues */

	/* Set the D bits with the dec result */
	*((UINT64*)D.addr) &= U64(0x000000000000ffff);
	*((UINT64*)D.addr) |= (((UINT64)(workBits24)) << 16);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * ? * * */
	/* TODO: S, L, E, U, V, C */
	if ( *((UINT64*)D.addr) & U64(0x0000008000000000))		 N_bit_set(1);
	if ((*((UINT64*)D.addr) & U64(0x000000ffffff0000)) == 0) Z_bit_set(1);

	cycles += 2;		/* TODO: + mv oscillator clock cycles */
	return 1;
}

/* AND : .... .... 0110 F1JJ : A-24 */
static size_t dsp56k_op_and(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};
	
	decode_JJF_table(BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S, &D);
	
	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);
	
	/* AND a word of S with A1|B1 */
	((PAIR64*)D.addr)->w.h = *((UINT16*)S.addr) & ((PAIR64*)D.addr)->w.h;
	
	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S, L */
	if ( *((UINT64*)D.addr) & U64(0x0000000080000000))		 N_bit_set(1);
	if ((*((UINT64*)D.addr) & U64(0x00000000ffff0000)) == 0) Z_bit_set(1);
	V_bit_set(0);

	cycles += 2;		/* TODO: + mv oscillator cycles */
	return 1;
}

/* ABS : .... .... 0111 F001 : A-18 */
static size_t dsp56k_op_abs(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* ROR : .... .... 0111 F010 : A-192 */
static size_t dsp56k_op_ror(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 16 of the source operand is set. Cleared otherwise. */
	return 0;
}

/* ROL : .... .... 0111 F011 : A-190 */
static size_t dsp56k_op_rol(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 31 of the source operand is set. Cleared otherwise. */
    return 0;
}

/* CMPM : .... .... 0111 FJJJ : A-66 */
static size_t dsp56k_op_cmpm(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
static size_t dsp56k_op_mpy(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MPYR : .... .... 1k01 FQQQ : A-162 */
static size_t dsp56k_op_mpyr(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MAC : .... .... 1k10 FQQQ : A-122 */
static size_t dsp56k_op_mac(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
static size_t dsp56k_op_macr(const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}


/******************************/
/* Remaining non-parallel ops */
/******************************/

/* ADC : 0001 0101 0000 F01J : A-20 */
static size_t dsp56k_op_adc(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * * */
	return 0;
}

/* ANDI : 0001 1EE0 iiii iiii : A-26 */
static size_t dsp56k_op_andi(const UINT16 op, UINT8* cycles)
{
	UINT16 immediate = BITS(op,0x00ff);

	/* There is not currently a good way to refer to CCR or MR.  Explicitly decode here. */
	switch(BITS(op,0x0600))
	{
		case 0x01:	/* MR */
			SR &= ((immediate << 8) | 0x00ff);
			break;
	
		case 0x02:	/* CCR */
			SR &= (immediate | 0xff00);
			break;
	
		case 0x03:	/* OMR */
			OMR &= (UINT8)(immediate);
			break;

		default:
			fatalerror("DSP56k - BAD EE value in andi operation") ;
	}

	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Cleared if the corresponding bit in the immediate data is cleared and if the operand
	   is the CCR. Not affected otherwise. */
	cycles += 2;
	return 1;
}

/* ASL4 : 0001 0101 0011 F001 : A-30 */
static size_t dsp56k_op_asl4(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - ? * * * * ? ? */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if bit 5 through 39 are
	       not the same. */
	/* C - Set if bit 36 of source operand is set. Cleared otherwise. */
	return 0;
}

/* ASR4 : 0001 0101 0011 F000 : A-34 */
static size_t dsp56k_op_asr4(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * 0 ? */
	/* C - Set if bit 3 of source operand is set. Cleared otherwise. */
	return 0;
}

/* ASR16 : 0001 0101 0111 F000 : A-36 */
static size_t dsp56k_op_asr16(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * 0 ? */
	/* C - Set if bit 15 of source operand is set. Cleared otherwise. */
	return 0;
}

/* BFCHG  : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_op_bfop(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* C - Set if all bits specified by the mask are set. Cleared otherwise. Ignore bits which are
	       not set in the mask. (BFCHG, BFSET, BFTSTH) */
	/* C - Set if all bits specified by the mask are cleared. Cleared otherwise. Ignore bits which
	       are not set in the mask. (BFCLR, BFTSTL) */
	return 0;
}

/* BFCHG  : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_op_bfop_1(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT16 workAddr = 0x0000;
	UINT16 workingWord = 0x0000;
	UINT16 previousValue = 0x0000;
	typed_pointer R = { NULL, DT_BYTE };
	typed_pointer tempTP = { NULL, DT_BYTE };

	UINT16 iVal = op2 & 0x00ff;

	decode_RR_table(BITS(op,0x0003), &R);
	decode_BBB_bitmask(BITS(op2,0xe000), &iVal);

	workAddr = *((UINT16*)R.addr);
	previousValue = data_read_word_16le(WORD(workAddr));
	workingWord = previousValue;

	switch(BITS(op2, 0x1f00))
	{
		case 0x12:	/* BFCHG */
			workingWord = ~workingWord;
			workingWord &= iVal;
			break;
		case 0x04:	/* BFCLR */
			workingWord = workingWord & (~iVal);
			break;
		case 0x18:	/* BFSET */
			workingWord = workingWord | iVal;
			break;
		case 0x10:	/* BFTSTH */
			/* Just the test below */
			break;
		case 0x00:	/* BFTSTL */
			/* Just the test below */
			break;
	}

	tempTP.addr = &workingWord;
	tempTP.data_type = DT_WORD;
	SetDataMemoryValue(tempTP, WORD(workAddr));

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:	/* BFCHG */
			if ((iVal & previousValue) == iVal) C_bit_set(1); break;
		case 0x04:	/* BFCLR */
			if ((iVal & previousValue) == iVal) C_bit_set(1); else C_bit_set(0); break;
		case 0x18:	/* BFSET */
			if ((iVal & previousValue) == iVal) C_bit_set(1); break;
		case 0x10:	/* BFTSTH */
			if ((iVal & previousValue) == iVal) C_bit_set(1); break;
		case 0x00:	/* BFTSTL */
			if ((iVal & previousValue) == 0x0000) C_bit_set(1); break;
	}

	cycles += 4; 	/* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* BFCHG  : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_op_bfop_2(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* C - Set if all bits specified by the mask are set. Cleared otherwise. Ignore bits which are
	       not set in the mask. (BFCHG, BFSET, BFTSTH) */
	/* C - Set if all bits specified by the mask are cleared. Cleared otherwise. Ignore bits which
	       are not set in the mask. (BFCLR, BFTSTL) */
	return 0;
}

/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
static size_t dsp56k_op_bcc(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Bcc : 0010 11cc ccee eeee : A-48 */
static size_t dsp56k_op_bcc_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Bcc : 0000 0111 RR10 cccc : A-48 */
static size_t dsp56k_op_bcc_2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
static size_t dsp56k_op_bra(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BRA : 0000 1011 aaaa aaaa : A-50 */
static size_t dsp56k_op_bra_1(const UINT16 op, UINT8* cycles)
{
	/* 8 bit immediate, relative offset */
	INT8 branchOffset = (INT8)BITS(op,0x00ff);

	/* "The PC Contains the address of the next instruction" */
	PC += 1;

	/* Jump */
	core.ppc = PC;
	PC += branchOffset;
	change_pc(PC);

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4; /* TODO: + jx oscillator clock cycles */
	return 0;
}

/* BRA : 0000 0001 0010 11RR : A-50 */
static size_t dsp56k_op_bra_2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BRKc : 0000 0001 0001 cccc : A-52 */
static size_t dsp56k_op_brkc(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
static size_t dsp56k_op_bscc(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	int shouldBranch = decode_cccc_table(BITS(op,0x000f));

	if (shouldBranch)
	{
		/* The PC Contains the address of the next instruction */
		PC += 2;

		/* Push */
		SP++;
		SSH = PC;
		SSL = SR;

		/* Change */
		core.ppc = PC;
		PC = PC + (INT16)op2;
		change_pc(PC);
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;		/* TODO: + jx oscillator clock cycles */
	return 2;
}

/* BScc : 0000 0111 RR00 cccc : A-54 */
static size_t dsp56k_op_bscc_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
static size_t dsp56k_op_bsr(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* The PC Contains the address of the next instruction */
	PC += 2;

	/* Push */
	SP++;
	SSH = PC;
	SSL = SR;

	/* Change */
	core.ppc = PC;
	PC = PC + (INT16)op2;
	change_pc(PC);

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;	/* TODO: + jx oscillator clock cycles */
	return 0;
}

/* BSR : 0000 0001 0010 10RR : A-56 */
static size_t dsp56k_op_bsr_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* CHKAAU : 0000 0000 0000 0100 : A-58 */
static size_t dsp56k_op_chkaau(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - ? ? ? - */
	/* V - Set if the result of the last address ALU update performed a modulo wrap. Cleared if 
	       result of the last address ALU did not perform a modulo wrap.*/
	/* Z - Set if the result of the last address ALU update is 0. Cleared if the result of the last
	       address ALU is positive. */
	/* N - Set if the result of the last address ALU update is negative. Cleared if the result of the
	       last address ALU is positive. */
	return 0;
}

/* DEBUG : 0000 0000 0000 0001 : A-68 */
static size_t dsp56k_op_debug(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
static size_t dsp56k_op_debugcc(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* DIV : 0001 0101 0--0 F1DD : A-76 */
/* WARNING : DOCS SAY THERE IS A PARALLEL MOVE HERE !!! */
static size_t dsp56k_op_div(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - ? ? */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significantst 
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	/* C - Set if bit 39 of the result is cleared. Cleared otherwise. */
	return 0;
}

/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
static size_t dsp56k_op_dmac(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	return 0;
}

/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_op_do(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	return 0;
}

/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_op_do_1(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT8 iValue = BITS(op,0x00ff);

	/* Don't execute if the loop counter == 0 */
	if (iValue != 0x00)
	{
		/* First instruction cycle */
		SP++;						/* TODO: Should i really inc here first? */
		SSH = LA;
		SSL = LC;
		LC = (UINT16)iValue;


		/* Second instruction cycle */
		SP++;						/* TODO: See above */
		SSH = PC + 2;				/* Keep these stack entries in 'word-based-index' space */
		SSL = SR;
		LA = PC + 2 + op2;			/* TODO: The docs subtract 1 from here? */


		/* Third instruction cycle */
		LF_bit_set(1);


		/* S L E U N Z V C */
		/* - * - - - - - - */
		/* TODO : L */

		cycles += 6;	/* TODO: + mv oscillator cycles */
	}
	else
	{
		cycles += 10;	/* TODO: + mv oscillator cycles */
	}

	return 2;
}

/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_op_do_2(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT16 lValue = 0x0000;
	typed_pointer S = {NULL, DT_BYTE};
	decode_DDDDD_table(BITS(op,0x001f), &S);

	/* TODO: Does not work for sources A&B - Fix per the docs */
	lValue = *((UINT16*)S.addr);

	/* TODO: Fix for special cased SP S */
	if (S.addr == &SP)
		logerror("DSP56k: do with SP as the source not properly implemented yet.\n");

	/* TODO: Fix for special cased SSSL S */
	if (S.addr == &SSL)
		logerror("DSP56k: do with SP as the source not properly implemented yet.\n");

	/* Don't execute if the loop counter == 0 */
	if (lValue != 0x00)
	{
		/* First instruction cycle */
		SP++;						/* TODO: Should i really inc here first? */
		SSH = LA;
		SSL = LC;
		LC = (UINT16)lValue;


		/* Second instruction cycle */
		SP++;						/* TODO: See above */
		SSH = PC + 2;				/* Keep these stack entries in 'word-based-index' space */
		SSL = SR;
		LA = PC + 2 + op2;			/* TODO: The docs subtract 1 from here? */


		/* Third instruction cycle */
		LF_bit_set(1);


		/* S L E U N Z V C */
		/* - * - - - - - - */
		/* TODO : L */

		cycles += 6;	/* TODO: + mv oscillator cycles */
	}
	else
	{
		cycles += 10;	/* TODO: + mv oscillator cycles */
	}

	return 2;
}

/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
static size_t dsp56k_op_doforever(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* ENDDO : 0000 0000 0000 1001 : A-92 */
static size_t dsp56k_op_enddo(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* EXT : 0001 0101 0101 F010 : A-96 */
static size_t dsp56k_op_ext(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	return 0;
}

/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
static size_t dsp56k_op_illegal(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* IMAC : 0001 0101 1010 FQQQ : A-100 */
static size_t dsp56k_op_imac(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * ? ? * ? ? - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	/* U,E - Will not be set correctly by this instruction*/
	/* V - Set to zero regardless of the overflow */
	return 0;
}

/* IMPY : 0001 0101 1000 FQQQ : A-102 */
static size_t dsp56k_op_impy(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * ? ? * ? ? - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	/* U,E - Will not be set correctly by this instruction*/
	/* V - Set to zero regardless of the overflow */
	return 0;
}

/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
static size_t dsp56k_op_jcc(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Jcc : 0000 0110 RR10 cccc : A-108 */
static size_t dsp56k_op_jcc_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
static size_t dsp56k_op_jmp(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	core.ppc = PC;
	PC = op2;
	change_pc(PC);

	/* S L E U N Z V C */
	/* - - - - - - - - */

	cycles += 4;	/* TODO: + jx */
	return 0;
}

/* JMP : 0000 0001 0010 01RR : A-110 */
static size_t dsp56k_op_jmp_1(const UINT16 op, UINT8* cycles)
{
	typed_pointer R = { NULL, DT_BYTE };
	decode_RR_table(BITS(op,0x0003), &R);

	core.ppc = PC;
	PC = *((UINT16*)R.addr);
	change_pc(PC);

	/* S L E U N Z V C */
	/* - - - - - - - - */

	cycles += 4;	/* TODO: + jx */
	return 0;
}

/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
static size_t dsp56k_op_jscc(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JScc : 0000 0110 RR00 cccc : A-112 */
static size_t dsp56k_op_jscc_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
static size_t dsp56k_op_jsr(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JSR : 0000 1010 AAAA AAAA : A-114 */
static size_t dsp56k_op_jsr_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JSR : 0000 0001 0010 00RR : A-114 */
static size_t dsp56k_op_jsr_2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* LEA : 0000 0001 11TT MMRR : A-116 */
static size_t dsp56k_op_lea(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* LEA : 0000 0001 10NN MMRR : A-116 */
static size_t dsp56k_op_lea_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
static size_t dsp56k_op_macsuuu(const UINT16 op, UINT8* cycles)
{
	UINT8 s = 0;
	INT64 result = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;
	
	decode_QQF_special_table(BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);
	
	s = BITS(op,0x0004);
	
	/* Fixed-point 2's complement multiplication requires a shift */
	if (s)
	{
		/* Unsigned * Unsigned */
		UINT32 s1 = (UINT32)(*((UINT16*)S1));
		UINT32 s2 = (UINT32)(*((UINT16*)S2));
		result = ( s1 * s2 ) << 1;
	}
	else
	{
		/* Signed * Unsigned */
		UINT32 s1 = (UINT32)((INT32)(*((UINT16*)S1)));
		UINT32 s2 = (UINT32)(*((UINT16*)S2));
		result = ( s1 * s2 ) << 1;
	}
	
	(*((UINT64*)D)) += result;
	
	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))		N_bit_set(1);
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) Z_bit_set(1);

	cycles += 2;
	return 1;
}

/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
static size_t dsp56k_op_move_2(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
static size_t dsp56k_op_movec(const UINT16 op, UINT8* cycles)
{
	UINT8 W;
	typed_pointer R = { NULL, DT_BYTE };
	typed_pointer SD = { NULL, DT_BYTE };

	W = BITS(op,0x0400);
	decode_DDDDD_table(BITS(op,0x03e0), &SD);
	decode_RR_table(BITS(op,0x0003), &R);

	if (W)
	{
		/* Write D */
		UINT16 value = data_read_word_16le(WORD(*((UINT16*)R.addr))) ;
		typed_pointer temp_src = { &value, DT_WORD };
		SetDestinationValue(temp_src, SD);
	}
	else
	{
		/* Read S */
		UINT16 dataMemOffset = *((UINT16*)R.addr);
		SetDataMemoryValue(SD, WORD(dataMemOffset));
	}

	execute_MM_table(BITS(op,0x0003), BITS(op,0x000c));

	/* S L E U N Z V C */
	/* * ? ? ? ? ? ? ? */
	/* All ? bits - If SR is specified as a destination operand, set according to the corresponding
	   bit of the source operand. If SR is not specified as a destination operand, L is set if data
	   limiting occurred. All ? bits are not affected otherwise.*/
	if (W && (SD.addr != &SR))
	{
		/* If you're writing to something other than the SR */
		/* TODO */
	}

	cycles += 2;	/* TODO: + mvc */
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
static size_t dsp56k_op_movec_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * ? ? ? ? ? ? ? */
	/* All ? bits - If SR is specified as a destination operand, set according to the corresponding
	   bit of the source operand. If SR is not specified as a destination operand, L is set if data
	   limiting occurred. All ? bits are not affected otherwise.*/
	return 0;
}

/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
static size_t dsp56k_op_movec_2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * ? ? ? ? ? ? ? */
	/* All ? bits - If SR is specified as a destination operand, set according to the corresponding
	   bit of the source operand. If SR is not specified as a destination operand, L is set if data
	   limiting occurred. All ? bits are not affected otherwise.*/
	return 0;
}

/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
static size_t dsp56k_op_movec_3(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT8 W;
	UINT8 t;
	typed_pointer SD = { NULL, DT_BYTE };

	W = BITS(op,0x0400);
	t = BITS(op,0x0008);
	decode_DDDDD_table(BITS(op,0x03e0), &SD);

	if (W)
	{
		/* Write D */
		if (t)
		{
			/* 16-bit long data */
			typed_pointer temp_src = { (void*)&op2, DT_WORD };
			SetDestinationValue(temp_src, SD);
		}
		else
		{
			/* 16-bit long address */
			UINT16 tempD = data_read_word_16le(WORD(op2));
			typed_pointer tempTP = {&tempD, DT_WORD};
			SetDestinationValue(tempTP, SD);
		}
	}
	else
	{
		/* Read S */
		if (t)
		{
			/* 16-bit long data */
			logerror("DSP56k: Movec - I don't think this exists?");
		}
		else
		{
			/* 16-bit long address */
			SetDataMemoryValue(SD, WORD(op2));
		}
	}

	/* S L E U N Z V C */
	/* * ? ? ? ? ? ? ? */
	/* All ? bits - If SR is specified as a destination operand, set according to the corresponding
	   bit of the source operand. If SR is not specified as a destination operand, L is set if data
	   limiting occurred. All ? bits are not affected otherwise.*/
	if (W && (SD.addr != &SR))
	{
		/* If you're writing to something other than the SR */
		/* TODO */
	}

	cycles += 2;	/* TODO: + mvc */
	return 2;
}

/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
static size_t dsp56k_op_movec_4(const UINT16 op, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_DDDDD_table(BITS(op,0x03e0), &S);
	decode_DDDDD_table(BITS(op,0x001f), &D);

	SetDestinationValue(S, D);

	/* S L E U N Z V C */
	/* * ? ? ? ? ? ? ? */
	/* All ? bits - If SR is specified as a destination operand, set according to the corresponding
	   bit of the source operand. If SR is not specified as a destination operand, L is set if data
	   limiting occurred. All ? bits are not affected otherwise.*/
	if (D.addr != &SR)
	{
		/* If you're writing to something other than the SR */
		/* TODO */
	}

	cycles += 2;
	return 1;
}

/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
static size_t dsp56k_op_movec_5(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * ? ? ? ? ? ? ? */
	/* All ? bits - If SR is specified as a destination operand, set according to the corresponding
	   bit of the source operand. If SR is not specified as a destination operand, L is set if data
	   limiting occurred. All ? bits are not affected otherwise.*/
	return 0;
}

/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
static size_t dsp56k_op_movei(const UINT16 op, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	typed_pointer immTP = {NULL, DT_BYTE};

	/* Typecasting to INT16 sign-extends the BBBBBBBB operand */
	UINT16 immediateSignExtended = (INT16)(op & 0x00ff);
	immTP.addr = &immediateSignExtended;
	immTP.data_type = DT_WORD;

	decode_DD_table(BITS(op,0x0300), &D);

	SetDestinationValue(immTP, D);

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
static size_t dsp56k_op_movem(const UINT16 op, UINT8* cycles)
{
	UINT8 W;
	typed_pointer R = { NULL, DT_BYTE };
	typed_pointer SD = { NULL, DT_BYTE };

	W = BITS(op,0x0100);
	decode_RR_table(BITS(op,0x00c0), &R);
	decode_HHH_table(BITS(op,0x0007), &SD);

	if (W)
	{
		/* Read from Program Memory */
		typed_pointer data;
		UINT16 ldata = program_read_word_16le(WORD(*((UINT16*)R.addr)));

		data.addr = &ldata;
		data.data_type = DT_WORD;
		SetDestinationValue(data, SD) ;
	}
	else
	{
		/* Write to Program Memory */
		SetProgramMemoryValue(SD, WORD(*((UINT16*)R.addr))) ;
	}

	execute_MM_table(BITS(op,0x00c0), BITS(op,0x0018));

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;	/* TODO: + mvm oscillator clock cycles */
	return 1;
}

/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
static size_t dsp56k_op_movem_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
static size_t dsp56k_op_movem_2(const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
static size_t dsp56k_op_movep(const UINT16 op, UINT8* cycles)
{
	UINT16 W;
	UINT16 pp;
	typed_pointer SD = {NULL, DT_BYTE};

	decode_HH_table(BITS(op,0x00c0), &SD);
	/* TODO: Special cases for A & B */

	pp = op & 0x001f;
	pp = assemble_address_from_IO_short_address(pp);

	W = BITS(op,0x0100);

	if (W)
	{
		UINT16 data = data_read_word_16le(WORD(pp));

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		SetDestinationValue(tempTP, SD);
	}
	else
	{
		SetDataMemoryValue(SD, WORD(pp));
	}

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */

	cycles += 4;		/* TODO: + mvp oscillator cycles */
	return 1;
}

/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
static size_t dsp56k_op_movep_1(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
static size_t dsp56k_op_moves(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
static size_t dsp56k_op_mpysuuu(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	return 0;
}

/* NEGC : 0001 0101 0110 F000 : A-168 */
static size_t dsp56k_op_negc(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * * */
	return 0;
}

/* NOP : 0000 0000 0000 0000 : A-170 */
static size_t dsp56k_op_nop(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* NORM : 0001 0101 0010 F0RR : A-172 */
static size_t dsp56k_op_norm(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * ? - */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significantst 
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	return 0;
}

/* ORI : 0001 1EE1 iiii iiii : A-178 */
static size_t dsp56k_op_ori(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Set if the corresponding bit in the immediate data is set and if the operand is the
	   CCR. Not affected otherwise. */
	return 0;
}

/* REP : 0000 0000 111- --RR : A-180 */
static size_t dsp56k_op_rep(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	return 0;
}

/* REP : 0000 1111 iiii iiii : A-180 */
static size_t dsp56k_op_rep_1(const UINT16 op, UINT8* cycles)
{
	/* TODO: This is non-interruptable, probably have to turn off interrupts here */
	UINT16 iVal = op & 0x00ff;
	
	if (iVal != 0)
	{
		TEMP = LC;
		LC = iVal;
		
		core.repFlag = 1;
		core.repAddr = PC + WORD(1);
		
		cycles += 4;		/* TODO: + mv oscillator clock cycles */
	}
	else
	{
		cycles += 6;		/* TODO: + mv oscillator clock cycles */
	}
	
	
	/* S L E U N Z V C */
	/* - * - - - - - - */
    /* TODO */
	return 1;
}

/* REP : 0000 0100 001D DDDD : A-180 */
static size_t dsp56k_op_rep_2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	return 0;
}

/* REPcc : 0000 0001 0101 cccc : A-184 */
static size_t dsp56k_op_repcc(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* RESET : 0000 0000 0000 1000 : A-186 */
static size_t dsp56k_op_reset(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* RTI : 0000 0000 0000 0111 : A-194 */
static size_t dsp56k_op_rti(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Set according to value pulled from the stack. */
	return 0;
}

/* RTS : 0000 0000 0000 0110 : A-196 */
static size_t dsp56k_op_rts(const UINT16 op, UINT8* cycles)
{
	/* Pop */
	core.ppc = PC;
	PC = SSH;
	change_pc(PC);

	/* SR = SSL; The status register is not affected. */

	SP--;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;	/* TODO: + rx oscillator clock cycles */
	return 0;
}

/* STOP : 0000 0000 0000 1010 : A-200 */
static size_t dsp56k_op_stop(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* SWAP : 0001 0101 0111 F001 : A-206 */
static size_t dsp56k_op_swap(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* SWI : 0000 0000 0000 0101 : A-208 */
static size_t dsp56k_op_swi(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
static size_t dsp56k_op_tcc(const UINT16 op, UINT8* cycles)
{
	int shouldTransfer = decode_cccc_table(BITS(op,0x03c0));

	if (shouldTransfer)
	{
		typed_pointer S = {NULL, DT_BYTE};
		typed_pointer D = {NULL, DT_BYTE};
		typed_pointer S2 = {&R0, DT_WORD};
		typed_pointer D2 = {NULL, DT_BYTE};

		decode_h0hF_table(BITS(op,0x0007),BITS(op,0x0008), &S, &D);
		SetDestinationValue(S, D);

		/* TODO: What's up with that A,A* thing in the docs?  Can you only ignore the R0->RX transfer if you do an A,A? */
		decode_RR_table(BITS(op,0x0030), &D2); /* TT is the same as RR */
		SetDestinationValue(S2, D2);
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* TFR(2) : 0001 0101 0000 F00J : A-214 */
static size_t dsp56k_op_tfr2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	return 0;
}

/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
static size_t dsp56k_op_tfr3(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* TST(2) : 0001 0101 0001 -1DD : A-220 */
static size_t dsp56k_op_tst2(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * 0 0 */
	/* (L,E,U should be set to 0) */
	return 0;
}

/* WAIT : 0000 0000 0000 1011 : A-222 */
static size_t dsp56k_op_wait(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* ZERO : 0001 0101 0101 F000 : A-224 */
static size_t dsp56k_op_zero(const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	return 0;
}



/***************************************************************************
	Table decoding
***************************************************************************/
static UINT16 decode_BBB_bitmask(UINT16 BBB, UINT16 *iVal)
{
	UINT16 retVal = 0x0000;

	switch(BBB)
	{
		case 0x4: retVal = 0xff00;  *iVal <<= 8;  break;
		case 0x2: retVal = 0x0ff0;  *iVal <<= 4;  break;
		case 0x1: retVal = 0x00ff;  *iVal <<= 0;  break;
	}

	return retVal;
}

static int decode_cccc_table(UINT16 cccc)
{
	int retVal = 0;

	/* Not fully tested */
	switch (cccc)
	{
		/* Arranged according to mnemonic table - not decoding table */
		case 0x0: if( C_bit() == 0)                              retVal = 1;  break;  // cc(hs)
		case 0x8: if( C_bit() == 1)                              retVal = 1;  break;  // cs(lo)
		case 0x5: if( E_bit() == 0)                              retVal = 1;  break;  // ec
		case 0xa: if( Z_bit() == 1)                              retVal = 1;  break;  // eq
		case 0xd: if( E_bit() == 1)                              retVal = 1;  break;  // es
		case 0x1: if((N_bit() ^  V_bit()) == 0)                  retVal = 1;  break;  // ge
		case 0x7: if((Z_bit() | (N_bit() ^ V_bit())) == 0)       retVal = 1;  break;  // gt
		case 0x6: if( L_bit() == 0)                              retVal = 1;  break;  // lc
		case 0xf: if((Z_bit() | (N_bit() ^ V_bit())) == 1)       retVal = 1;  break;  // le
		case 0xe: if( L_bit() == 1)                              retVal = 1;  break;  // ls
		case 0x9: if((N_bit() ^  V_bit()) == 1)                  retVal = 1;  break;  // lt
		case 0xb: if( N_bit() == 1)                              retVal = 1;  break;  // mi
		case 0x2: if( Z_bit() == 0)                              retVal = 1;  break;  // ne
		case 0xc: if((Z_bit() | ((!U_bit()) & (!E_bit()))) == 1) retVal = 1;  break;  // nr
		case 0x3: if( N_bit() == 0)                              retVal = 1;  break;  // pl
		case 0x4: if((Z_bit() | ((!U_bit()) & (!E_bit()))) == 0) retVal = 1;  break;  // nn
	}

	return retVal;
}

static void decode_DDDDD_table(UINT16 DDDDD, typed_pointer* ret)
{
	switch(DDDDD)
	{
		case 0x00: ret->addr = &X0;  ret->data_type = DT_WORD;       break; 
		case 0x01: ret->addr = &Y0;  ret->data_type = DT_WORD;       break; 
		case 0x02: ret->addr = &X1;  ret->data_type = DT_WORD;       break; 
		case 0x03: ret->addr = &Y1;  ret->data_type = DT_WORD;       break; 
		case 0x04: ret->addr = &A ;  ret->data_type = DT_LONG_WORD;  break; 
		case 0x05: ret->addr = &B ;  ret->data_type = DT_LONG_WORD;  break; 
		case 0x06: ret->addr = &A0;  ret->data_type = DT_WORD;       break; 
		case 0x07: ret->addr = &B0;  ret->data_type = DT_WORD;       break; 
		case 0x08: ret->addr = &LC;  ret->data_type = DT_WORD;       break; 
		case 0x09: ret->addr = &SR;  ret->data_type = DT_WORD;       break; 
		case 0x0a: ret->addr = &OMR; ret->data_type = DT_BYTE;       break; 
		case 0x0b: ret->addr = &SP;  ret->data_type = DT_BYTE;       break; 
		case 0x0c: ret->addr = &A1;  ret->data_type = DT_WORD;       break; 
		case 0x0d: ret->addr = &B1;  ret->data_type = DT_WORD;       break; 
		case 0x0e: ret->addr = &A2;  ret->data_type = DT_BYTE;       break; 
		case 0x0f: ret->addr = &B2;  ret->data_type = DT_BYTE;       break; 

		case 0x10: ret->addr = &R0;  ret->data_type = DT_WORD;       break; 
		case 0x11: ret->addr = &R1;  ret->data_type = DT_WORD;       break; 
		case 0x12: ret->addr = &R2;  ret->data_type = DT_WORD;       break; 
		case 0x13: ret->addr = &R3;  ret->data_type = DT_WORD;       break; 
		case 0x14: ret->addr = &M0;  ret->data_type = DT_WORD;       break; 
		case 0x15: ret->addr = &M1;  ret->data_type = DT_WORD;       break; 
		case 0x16: ret->addr = &M2;  ret->data_type = DT_WORD;       break; 
		case 0x17: ret->addr = &M3;  ret->data_type = DT_WORD;       break; 
		case 0x18: ret->addr = &SSH; ret->data_type = DT_WORD;       break; 
		case 0x19: ret->addr = &SSL; ret->data_type = DT_WORD;       break; 
		case 0x1a: ret->addr = &LA;  ret->data_type = DT_WORD;       break; 
		//no 0x1b
		case 0x1c: ret->addr = &N0;  ret->data_type = DT_WORD;       break; 
		case 0x1d: ret->addr = &N1;  ret->data_type = DT_WORD;       break; 
		case 0x1e: ret->addr = &N2;  ret->data_type = DT_WORD;       break; 
		case 0x1f: ret->addr = &N3;  ret->data_type = DT_WORD;       break; 
	}
}

static void decode_DD_table(UINT16 DD, typed_pointer* ret)
{
	switch(DD)
	{
		case 0x00: ret->addr = &X0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &Y0;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &X1;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &Y1;  ret->data_type = DT_WORD;  break;
	}
}

static void decode_F_table(UINT16 F, typed_pointer* ret)
{
	switch(F)
	{
		case 0x0: ret->addr = &A;  ret->data_type = DT_LONG_WORD;  break;
		case 0x1: ret->addr = &B;  ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_h0hF_table(UINT16 h0h, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	UINT16 switchVal = (h0h << 1) | F ;

	switch (switchVal)
	{
		case 0x8: src_ret->addr = &X0;  src_ret->data_type = DT_WORD;       dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x9: src_ret->addr = &X0;  src_ret->data_type = DT_WORD;       dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xa: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD;       dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xb: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD;       dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x2: src_ret->addr = &A;   src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x1: src_ret->addr = &A;   src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x0: src_ret->addr = &B;   src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x3: src_ret->addr = &B;   src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_HH_table(UINT16 HH, typed_pointer* ret)
{
	switch(HH)
	{
		case 0x0: ret->addr = &X0;  ret->data_type = DT_WORD;       break;
		case 0x1: ret->addr = &Y0;  ret->data_type = DT_WORD;       break;
		case 0x2: ret->addr = &A;   ret->data_type = DT_LONG_WORD;  break;
		case 0x3: ret->addr = &B;   ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_HHH_table(UINT16 HHH, typed_pointer* ret)
{
	switch(HHH)
	{
		case 0x0: ret->addr = &X0;  ret->data_type = DT_WORD;       break;
		case 0x1: ret->addr = &Y0;  ret->data_type = DT_WORD;       break;
		case 0x2: ret->addr = &X1;  ret->data_type = DT_WORD;       break;
		case 0x3: ret->addr = &Y1;  ret->data_type = DT_WORD;       break;
		case 0x4: ret->addr = &A;   ret->data_type = DT_LONG_WORD;  break;
		case 0x5: ret->addr = &B;   ret->data_type = DT_LONG_WORD;  break;
		case 0x6: ret->addr = &A0;  ret->data_type = DT_WORD;       break;
		case 0x7: ret->addr = &B0;  ret->data_type = DT_WORD;       break;
	}
}

static void decode_IIII_table(UINT16 IIII, typed_pointer* src_ret, typed_pointer* dst_ret, void *working)
{
	void *opposite = 0x00 ;

	if (working == &A) opposite = &B ;
	else               opposite = &A ;

	switch(IIII)
	{
		case 0x0: src_ret->addr = &X0;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x1: src_ret->addr = &Y0;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x2: src_ret->addr = &X1;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x3: src_ret->addr = &Y1;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x4: src_ret->addr = &A;       src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &X0;       dst_ret->data_type = DT_WORD;       break;
		case 0x5: src_ret->addr = &B;       src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &Y0;       dst_ret->data_type = DT_WORD;       break;
		case 0x6: src_ret->addr = &A0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &X0;       dst_ret->data_type = DT_WORD;       break;
		case 0x7: src_ret->addr = &B0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &Y0;       dst_ret->data_type = DT_WORD;       break;
		case 0x8: src_ret->addr = working;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x9: src_ret->addr = working;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xc: src_ret->addr = &A;       src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &X1;       dst_ret->data_type = DT_WORD;       break;
		case 0xd: src_ret->addr = &B;       src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &Y1;       dst_ret->data_type = DT_WORD;       break;
		case 0xe: src_ret->addr = &A0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &X1;       dst_ret->data_type = DT_WORD;       break;
		case 0xf: src_ret->addr = &B0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &Y1;       dst_ret->data_type = DT_WORD;       break;
	}
}

static void decode_JJJF_table(UINT16 JJJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	UINT16 switchVal = (JJJ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: src_ret->addr = &B;   src_ret->data_type = DT_LONG_WORD;    dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x1: src_ret->addr = &A;   src_ret->data_type = DT_LONG_WORD;    dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x4: src_ret->addr = &X;   src_ret->data_type = DT_DOUBLE_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x5: src_ret->addr = &X;   src_ret->data_type = DT_DOUBLE_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x6: src_ret->addr = &Y;   src_ret->data_type = DT_DOUBLE_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x7: src_ret->addr = &Y;   src_ret->data_type = DT_DOUBLE_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x8: src_ret->addr = &X0;  src_ret->data_type = DT_WORD;         dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x9: src_ret->addr = &X0;  src_ret->data_type = DT_WORD;         dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xa: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD;         dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xb: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD;         dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xc: src_ret->addr = &X1;  src_ret->data_type = DT_WORD;         dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xd: src_ret->addr = &X1;  src_ret->data_type = DT_WORD;         dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xe: src_ret->addr = &Y1;  src_ret->data_type = DT_WORD;         dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0xf: src_ret->addr = &Y1;  src_ret->data_type = DT_WORD;         dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_JJF_table(UINT16 JJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	UINT16 switchVal = (JJ << 1) | F ;

	switch (switchVal)
	{
		case 0x0: src_ret->addr = &X0;  src_ret->data_type = DT_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x1: src_ret->addr = &X0;  src_ret->data_type = DT_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x2: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x3: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x4: src_ret->addr = &X1;  src_ret->data_type = DT_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x5: src_ret->addr = &X1;  src_ret->data_type = DT_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x6: src_ret->addr = &Y1;  src_ret->data_type = DT_WORD;  dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x7: src_ret->addr = &Y1;  src_ret->data_type = DT_WORD;  dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_QQF_special_table(UINT16 QQ, UINT16 F, void **S1, void **S2, void **D)
{
	UINT16 switchVal = (QQ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: *S1 = &Y0;  *S2 = &X0;  *D = &A;  break;
		case 0x1: *S1 = &Y0;  *S2 = &X0;  *D = &B;  break;
		case 0x2: *S1 = &Y1;  *S2 = &X0;  *D = &A;  break;
		case 0x3: *S1 = &Y1;  *S2 = &X0;  *D = &B;  break;
		case 0x4: *S1 = &X1;  *S2 = &Y0;  *D = &A;  break;
		case 0x5: *S1 = &X1;  *S2 = &Y0;  *D = &B;  break;
		case 0x6: *S1 = &X1;  *S2 = &Y1;  *D = &A;  break;
		case 0x7: *S1 = &X1;  *S2 = &Y1;  *D = &B;  break;
	}
}

static void decode_RR_table(UINT16 RR, typed_pointer* ret)
{
	switch(RR)
	{
		case 0x00: ret->addr = &R0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &R1;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &R2;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &R3;  ret->data_type = DT_WORD;  break;
	}
}
#ifdef UNUSED_FUNCTION
static void decode_Z_table(UINT16 Z, typed_pointer* ret)
{
	switch(Z)
	{
		/* Fixed as per the Family Manual addendum */
		case 0x01: ret->addr = &A1;  ret->data_type = DT_WORD;  break;
		case 0x00: ret->addr = &B1;  ret->data_type = DT_WORD;  break;
	}
}
#endif
static void execute_m_table(int x, UINT16 m)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0;  nX = &N0; break;
		case 0x1: rX = &R1;  nX = &N1; break;
		case 0x2: rX = &R2;  nX = &N2; break;
		case 0x3: rX = &R3;  nX = &N3; break;
	}

	switch(m)
	{
		case 0x0: (*rX)++;             break;
		case 0x1: (*rX) = (*rX)+(*nX); break;
	}
}

static void execute_MM_table(UINT16 rnum, UINT16 MM)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(rnum)
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: rX = &R3;  nX = &N3;  break;
	}

	switch(MM)
	{
		case 0x0: /* do nothing */      break;
		case 0x1: (*rX)++ ;             break;
		case 0x2: (*rX)-- ;             break;
		case 0x3: (*rX) = (*rX)+(*nX) ; break;
	}
}
#ifdef UNUSED_FUNCTION
// Returns R address
static UINT16 execute_q_table(int x, UINT16 q)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0 ; nX = &N0 ; break ;
		case 0x1: rX = &R1 ; nX = &N1 ; break ;
		case 0x2: rX = &R2 ; nX = &N2 ; break ;
		case 0x3: rX = &R3 ; nX = &N3 ; break ;
	}

	switch(q)
	{
		case 0x0: /* No permanent changes */ ; return (*rX)+(*nX); break;
		case 0x1: (*rX)--;					   return (*rX);	   break;	// This one is special - it's a *PRE-decrement*!
	}

	exit(1);
	return 0x00;
}

static void execute_z_table(int x, UINT16 z)
{
	UINT16 *rX = 0x00 ;
	UINT16 *nX = 0x00 ;

	switch(x)
	{
		case 0x0: rX = &R0 ; nX = &N0 ; break ;
		case 0x1: rX = &R1 ; nX = &N1 ; break ;
		case 0x2: rX = &R2 ; nX = &N2 ; break ;
		case 0x3: rX = &R3 ; nX = &N3 ; break ;
	}

	if (!z)
	{
		(*rX)-- ;
	}
	else
	{
		(*rX) = (*rX)+(*nX) ;
	}
}

static UINT16 assemble_D_from_P_table(UINT16 P, UINT16 ppppp)
{
	UINT16 destAddr = 0x00 ;

	switch (P)
	{
		case 0x0: destAddr = ppppp ; break ;
		case 0x1: destAddr = assemble_address_from_IO_short_address(ppppp) ; break ;
	}

	return destAddr ;
}
#endif
static UINT16 assemble_address_from_IO_short_address(UINT16 pp)
{
	UINT16 fullAddy = 0xffe0;
	fullAddy |= pp;
	return fullAddy;
}
#ifdef UNUSED_FUNCTION
static UINT16 assemble_address_from_6bit_signed_relative_short_address(UINT16 srs)
{
	UINT16 fullAddy = srs ;
	if (fullAddy & 0x0020) fullAddy |= 0xffc0 ;

	return fullAddy ;
}
#endif
static void dsp56k_process_loop(void)
{
	if (LF_bit())
	{
		if (PC == LA)
		{
			if (LC == 1)
			{
				/* End of loop processing */
				SR = SSL;	/* TODO: A-83.  I believe only the Loop Flag comes back here. */
				SP--;
				
				LA = SSH;
				LC = SSL;
				SP--;
			}
			else
			{
				LC--;
				PC = SSH;
				change_pc(PC);
			}
		}
	}
}

static void dsp56k_process_rep(size_t repSize)
{
	if (core.repFlag)
	{
		if (PC == core.repAddr)
		{
			if (LC == 1)
			{
				/* End of rep processing */
				LC = TEMP;
				core.repFlag = 0;
				core.repAddr = 0x0000;
			}
			else
			{
				LC--;
				PC -= repSize;		/* A little strange - rewind by the size of the rep'd op */
				change_pc(PC);
			}
		}
	}
}


/***************************************************************************
	Parallel Memory Ops
***************************************************************************/
/* Register to Register Data Move : 0100 IIII ---- ---- */
static void execute_register_to_register_data_move(const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_IIII_table(BITS(op,0x0f00), &S, &D, d_register->addr);

	/* If the source is the same as the ALU destination, use the previous accumulator value */
	if (d_register->addr == S.addr)
	{
		typed_pointer tempTP;
		tempTP.addr = prev_accum_value;
		tempTP.data_type = DT_LONG_WORD;
		SetDestinationValue(tempTP, D);
	}
	else
	{
		SetDestinationValue(S, D);
	}
}

/* X Memory Data Move : 1mRR HHHW ---- ---- : A-137 */
static void execute_x_memory_data_move(const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value)
{
	UINT16 W;
	typed_pointer R = {NULL, DT_BYTE};
	typed_pointer SD = {NULL, DT_BYTE};

	W = BITS(op,0x0100);
	decode_HHH_table(BITS(op,0x0e00), &SD);
	decode_RR_table(BITS(op,0x3000),&R);

	if (W)
	{
		/* From X:<ea> to SD */
		UINT16 data = data_read_word_16le(WORD(*((UINT16*)R.addr)));

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		SetDestinationValue(tempTP, SD);
	}
	else
	{
		/* From SD to X:<ea> */
		/* If the source is the same as the ALU destination, use the previous accumulator value */
		if (d_register->addr == SD.addr)
		{
			typed_pointer tempTP;
			tempTP.addr = prev_accum_value;
			tempTP.data_type = DT_LONG_WORD;

			SetDataMemoryValue(tempTP, WORD(*((UINT16*)R.addr))) ;
		}
		else
		{
			SetDataMemoryValue(SD, WORD(*((UINT16*)R.addr))) ;
		}
	}

	execute_m_table(BITS(op,0x3000), BITS(op,0x4000));
}


/* X Memory Data Move : 0101 HHHW ---- ---- : A-137 */
/* NOTE: previous accumulator value is not needed since ^F1 is always the opposite accumulator */
static void execute_x_memory_data_move2(const UINT16 op, typed_pointer* d_register)
{
	UINT16 W;
	UINT16* mem_offset = NULL;
	typed_pointer SD = {NULL, DT_BYTE};

	W = BITS(op,0x0100);
	decode_HHH_table(BITS(op,0x0e000), &SD);

	if (d_register->addr == &A)
		mem_offset = &B1;
	else
		mem_offset = &A1;

	if (W)
	{
		/* Write D */
		UINT16 value = data_read_word_16le(WORD(*mem_offset));
		typed_pointer tempV = {&value, DT_WORD};
		SetDestinationValue(tempV, SD);
	}
	else
	{
		/* Read S */
		SetDataMemoryValue(SD, WORD(*mem_offset));
	}
}


/***************************************************************************
	Helper Functions
***************************************************************************/
static UINT16 Dsp56kOpMask(UINT16 cur, UINT16 mask)
{
	int i ;

	UINT16 retVal = (cur & mask) ;
	UINT16 temp = 0x0000 ;
	int offsetCount = 0 ;

	// Shift everything right, eliminating 'whitespace'...
	for (i = 0; i < 16; i++)
	{
		if (mask & (0x1<<i))		// If mask bit is non-zero
		{
			temp |= (((retVal >> i) & 0x1) << offsetCount) ;
			offsetCount++ ;
		}
	}

	return temp ;
}

static void SetDestinationValue(typed_pointer source, typed_pointer dest)
{
	UINT64 destinationValue = 0 ;

	switch(dest.data_type)
	{
		/* Copying to an 8-bit value */
		case DT_BYTE:
			switch(source.data_type)
			{
				/* From a ? */
				case DT_BYTE:        *((UINT8*)dest.addr) = (*((UINT8*) source.addr)) & 0xff; break;
				case DT_WORD:        *((UINT8*)dest.addr) = (*((UINT16*)source.addr)) & 0x00ff; break;
				case DT_DOUBLE_WORD: *((UINT8*)dest.addr) = (*((UINT32*)source.addr)) & 0x000000ff; break;
				case DT_LONG_WORD:   *((UINT8*)dest.addr) = (*((UINT64*)source.addr)) & U64(0x00000000000000ff); break;
			}
		break ;

		/* Copying to a 16-bit value */
		case DT_WORD:
			switch(source.data_type)
			{
				case DT_BYTE:        *((UINT16*)dest.addr) = (*((UINT8*) source.addr)) & 0xff; break;
				case DT_WORD:        *((UINT16*)dest.addr) = (*((UINT16*)source.addr)) & 0xffff; break;
				case DT_DOUBLE_WORD: *((UINT16*)dest.addr) = (*((UINT32*)source.addr)) & 0x0000ffff; break;
				case DT_LONG_WORD:   *((UINT16*)dest.addr) = (*((UINT64*)source.addr)) & U64(0x000000000000ffff); break;	/* Shift limiter action? */
			}
		break ;

		/* Copying to a 32-bit value */
		case DT_DOUBLE_WORD:
			switch(source.data_type)
			{
				case DT_BYTE:        *((UINT32*)dest.addr) = (*((UINT8*) source.addr)) & 0xff; break;
				case DT_WORD:        *((UINT32*)dest.addr) = (*((UINT16*)source.addr)) & 0xffff; break;
				case DT_DOUBLE_WORD: *((UINT32*)dest.addr) = (*((UINT32*)source.addr)) & 0xffffffff; break;
				case DT_LONG_WORD:   *((UINT32*)dest.addr) = (*((UINT64*)source.addr)) & U64(0x00000000ffffffff); break;
			}
		break ;

		/* Copying to a 64-bit value */
		case DT_LONG_WORD:
			switch(source.data_type)
			{
				case DT_BYTE:        *((UINT64*)dest.addr) = (*((UINT8*)source.addr)) & 0xff; break;

				case DT_WORD:        destinationValue = (*((INT16*)source.addr)) << 16;			/* Sign extend */
									 destinationValue &= U64(0x000000ffffff0000);
									 *((UINT64*)dest.addr) = destinationValue; break;			/* Forget not, yon shift register */

				case DT_DOUBLE_WORD: *((UINT64*)dest.addr) = (*((UINT32*)source.addr)) & 0xffffffff; break;
				case DT_LONG_WORD:   *((UINT64*)dest.addr) = (*((UINT64*)source.addr)) & U64(0x000000ffffffffff); break;
			}
		break ;
	}
}

/* TODO: Wait-state timings! */
static void SetDataMemoryValue(typed_pointer source, UINT32 destinationAddr)
{
	switch(source.data_type)
	{
		case DT_BYTE:        data_write_word_16le(destinationAddr, (UINT16)( (*((UINT8*) source.addr) & 0xff)               ) ) ; break ;
		case DT_WORD:        data_write_word_16le(destinationAddr, (UINT16)( (*((UINT16*)source.addr) & 0xffff)             ) ) ; break ;
		case DT_DOUBLE_WORD: data_write_word_16le(destinationAddr, (UINT16)( (*((UINT32*)source.addr) & 0x0000ffff)         ) ) ; break ;

		// !!! Is this universal ???
		// !!! Forget not, yon shift-limiter !!!
		case DT_LONG_WORD:   data_write_word_16le(destinationAddr, (UINT16)( ((*((UINT64*)source.addr)) & U64(0x00000000ffff0000)) >> 16) ) ; break ;
	}
}

/* TODO: Wait-state timings! */
static void SetProgramMemoryValue(typed_pointer source, UINT32 destinationAddr)
{
	switch(source.data_type)
	{
		case DT_BYTE:        program_write_word_16le(destinationAddr, (UINT16)( (*((UINT8*) source.addr) & 0xff)               ) ) ; break ;
		case DT_WORD:        program_write_word_16le(destinationAddr, (UINT16)( (*((UINT16*)source.addr) & 0xffff)             ) ) ; break ;
		case DT_DOUBLE_WORD: program_write_word_16le(destinationAddr, (UINT16)( (*((UINT32*)source.addr) & 0x0000ffff)         ) ) ; break ;

		// !!! Is this universal ???
		// !!! Forget not, yon shift-limiter !!!
		case DT_LONG_WORD:   program_write_word_16le(destinationAddr, (UINT16)( ((*((UINT64*)source.addr)) & U64(0x00000000ffff0000)) >> 16) ) ; break ;
	}
}


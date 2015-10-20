// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56ops.inc
    Core implementation for the portable Motorola/Freescale DSP56k emulator.
    Written by Andrew Gardner

***************************************************************************/

/* NOTES For register setting:
   FM.3-4 : When A2 or B2 is read, the register contents occupy the low-order portion
            (bits 7-0) of the word; the high-order portion (bits 16-8) is sign-extended. When A2 or B2
            is written, the register receives the low-order portion of the word; the high-order portion is not used
          : ...much more!
          : ...shifter/limiter/overflow notes too.

*/

/*
TODO:
    - 0x01ee: should this move sign extend?  otherwise the test-against-minus means nothing.
    - Restore only the proper bits upon loop termination!
    - BFCLR has some errata in the docs that may need to be applied.
*/

/************************/
/* Datatypes and macros */
/************************/
enum addSubOpType { OP_ADD,
					OP_SUB,
					OP_OTHER };

enum dataType { DT_BYTE,
				DT_WORD,
				DT_DOUBLE_WORD,
				DT_LONG_WORD };

struct typed_pointer
{
	void* addr;
	char  data_type;
};

//#define ADDRESS(X) (X<<1)
#define BITS(CUR,MASK) (Dsp56kOpMask(CUR,MASK))

/*********************/
/* Opcode prototypes */
/*********************/
static size_t dsp56k_op_addsub_2 (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_mac_1    (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_macr_1   (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_move_1   (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_mpy_1    (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_mpyr_1   (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_tfr_2    (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles);
static size_t dsp56k_op_mpy_2    (dsp56k_core* cpustate, const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_mac_2    (dsp56k_core* cpustate, const UINT16 op_byte, UINT8* cycles);
static size_t dsp56k_op_clr      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_add      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_move     (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_tfr      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_rnd      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_tst      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_inc      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_inc24    (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_or       (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_asr      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_asl      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_lsr      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_lsl      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_eor      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_subl     (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_sub      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_clr24    (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_sbc      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_cmp      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_neg      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_not      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_dec      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_dec24    (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_and      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_abs      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_ror      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_rol      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_cmpm     (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_mpy      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_mpyr     (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_mac      (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_macr     (dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles);
static size_t dsp56k_op_adc      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_andi     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_asl4     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_asr4     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_asr16    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bfop     (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bfop_1   (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bfop_2   (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bcc      (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bcc_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bcc_2    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bra      (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bra_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bra_2    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_brkcc    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bscc     (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bscc_1   (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_bsr      (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_bsr_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_chkaau   (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_debug    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_debugcc  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_div      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_dmac     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_do       (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_do_1     (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_do_2     (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_doforever(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_enddo    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_ext      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_illegal  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_imac     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_impy     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jcc      (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jcc_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jmp      (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jmp_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jscc     (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jscc_1   (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jsr      (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_jsr_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_jsr_2    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_lea      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_lea_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_macsuuu  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_move_2   (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movec    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_1  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_2  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_3  (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movec_4  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movec_5  (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movei    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movem    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movem_1  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movem_2  (dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles);
static size_t dsp56k_op_movep    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_movep_1  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_moves    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_mpysuuu  (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_negc     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_nop      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_norm     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_ori      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rep      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rep_1    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rep_2    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_repcc    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_reset    (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rti      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_rts      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_stop     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_swap     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_swi      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tcc      (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tfr2     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tfr3     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_tst2     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_wait     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);
static size_t dsp56k_op_zero     (dsp56k_core* cpustate, const UINT16 op, UINT8* cycles);


static void execute_register_to_register_data_move(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value);
static void execute_address_register_update(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value);
static void execute_x_memory_data_move (dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value);
static void execute_x_memory_data_move2(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register);
static void execute_dual_x_memory_data_read(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register);
static void execute_x_memory_data_move_with_short_displacement(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2);

static UINT16   decode_BBB_bitmask(dsp56k_core* cpustate, UINT16 BBB, UINT16 *iVal);
static int      decode_cccc_table(dsp56k_core* cpustate, UINT16 cccc);
static void     decode_DDDDD_table(dsp56k_core* cpustate, UINT16 DDDDD, typed_pointer* ret);
static void     decode_DD_table(dsp56k_core* cpustate, UINT16 DD, typed_pointer* ret);
static void     decode_DDF_table(dsp56k_core* cpustate, UINT16 DD, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_F_table(dsp56k_core* cpustate, UINT16 F, typed_pointer* ret);
static void     decode_h0hF_table(dsp56k_core* cpustate, UINT16 h0h, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_HH_table(dsp56k_core* cpustate, UINT16 HH, typed_pointer* ret);
static void     decode_HHH_table(dsp56k_core* cpustate, UINT16 HHH, typed_pointer* ret);
static void     decode_IIII_table(dsp56k_core* cpustate, UINT16 IIII, typed_pointer* src_ret, typed_pointer* dst_ret, void* working);
static void     decode_JJJF_table(dsp56k_core* cpustate, UINT16 JJJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_JJF_table(dsp56k_core* cpustate, UINT16 JJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_JF_table(dsp56k_core* cpustate, UINT16 JJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_KKK_table(dsp56k_core* cpustate, UINT16 KKK, typed_pointer* dst_ret1, typed_pointer* dst_ret2, void* working);
static void     decode_QQF_table(dsp56k_core* cpustate, UINT16 QQ, UINT16 F, void **S1, void **S2, void **D);
static void     decode_QQF_special_table(dsp56k_core* cpustate, UINT16 QQ, UINT16 F, void **S1, void **S2, void **D);
static void     decode_QQQF_table(dsp56k_core* cpustate, UINT16 QQQ, UINT16 F, void **S1, void **S2, void **D);
static void     decode_RR_table(dsp56k_core* cpustate, UINT16 RR, typed_pointer* ret);
static void     decode_TT_table(dsp56k_core* cpustate, UINT16 TT, typed_pointer* ret);
static void     decode_uuuuF_table(dsp56k_core* cpustate, UINT16 uuuu, UINT16 F, UINT8 add_sub_other, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_Z_table(dsp56k_core* cpustate, UINT16 Z, typed_pointer* ret);

static void     execute_m_table(dsp56k_core* cpustate, int x, UINT16 m);
static void     execute_mm_table(dsp56k_core* cpustate, UINT16 rnum, UINT16 mm);
static void     execute_MM_table(dsp56k_core* cpustate, UINT16 rnum, UINT16 MM);
static UINT16   execute_q_table(dsp56k_core* cpustate, int RR, UINT16 q);
static void     execute_z_table(dsp56k_core* cpustate, int RR, UINT16 z);

static UINT16   assemble_address_from_Pppppp_table(dsp56k_core* cpustate, UINT16 P, UINT16 ppppp);
static UINT16   assemble_address_from_IO_short_address(dsp56k_core* cpustate, UINT16 pp);
static UINT16   assemble_address_from_6bit_signed_relative_short_address(dsp56k_core* cpustate, UINT16 srs);

static void dsp56k_process_loop(dsp56k_core* cpustate);
static void dsp56k_process_rep(dsp56k_core* cpustate, size_t repSize);



/********************/
/* Helper Functions */
/********************/
static UINT16 Dsp56kOpMask(UINT16 op, UINT16 mask);

/* These arguments are written source->destination to fall in line with the processor's paradigm. */
static void SetDestinationValue(typed_pointer source, typed_pointer dest);

static void SetDataMemoryValue(dsp56k_core* cpustate, typed_pointer source, UINT32 destinationAddr);
static void SetProgramMemoryValue(dsp56k_core* cpustate, typed_pointer source, UINT32 destinationAddr);



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static void execute_one(dsp56k_core* cpustate)
{
	UINT16 op;
	UINT16 op2;
	size_t size = 0x1337;
	UINT8 cycle_count = 0;

	/* For MAME */
	cpustate->op = ROPCODE(ADDRESS(PC));
	debugger_instruction_hook(cpustate->device, PC);

	/* The words we're going to be working with */
	op = ROPCODE(ADDRESS(PC));
	op2 = ROPCODE(ADDRESS(PC) + ADDRESS(1));


	/* DECODE */
	/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
	if ((op & 0xe000) == 0x6000)
	{
		typed_pointer d_register = {NULL, DT_BYTE};

		/* Quote: (MOVE, MAC(R), MPY(R), ADD, SUB, TFR) */
		UINT16 op_byte = op & 0x00ff;

		/* ADD : 011m mKKK 0rru Fuuu : A-22 */
		/* SUB : 011m mKKK 0rru Fuuu : A-202 */
		/* Note: 0x0094 check allows command to drop through to MOVE and TFR */
		if (((op & 0xe080) == 0x6000) && ((op & 0x0094) != 0x0010))
		{
			size = dsp56k_op_addsub_2(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
		else if ((op & 0xe094) == 0x6084)
		{
			size = dsp56k_op_mac_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
		else if ((op & 0xe094) == 0x6094)
		{
			size = dsp56k_op_macr_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
		else if ((op & 0xe094) == 0x6010)
		{
			size = dsp56k_op_tfr_2(cpustate, op_byte, &d_register, &cycle_count);
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
			size = dsp56k_op_move_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
		else if ((op & 0xe094) == 0x6080)
		{
			size = dsp56k_op_mpy_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
		else if ((op & 0xe094) == 0x6090)
		{
			size = dsp56k_op_mpyr_1(cpustate, op_byte, &d_register, &cycle_count);
		}

		/* Now evaluate the parallel data move */
		execute_dual_x_memory_data_read(cpustate, op, &d_register);
	}
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
	else if ((op & 0xfe00) == 0x1600)
	{
		/* Quote: (MPY or MAC) */
		UINT16 op_byte = op & 0x00ff;

		/* MPY : 0001 0110 RRDD FQQQ : A-160 */
		if ((op & 0xff00) == 0x1600)
		{
			size = dsp56k_op_mpy_2(cpustate, op_byte, &cycle_count);
		}
		/* MAC : 0001 0111 RRDD FQQQ : A-122 */
		else if ((op & 0xff00) == 0x1700)
		{
			size = dsp56k_op_mac_2(cpustate, op_byte, &cycle_count);
		}

		/* Now evaluate the parallel data move */
		/* TODO // decode_x_memory_data_write_and_register_data_move(op, parallel_move_str, parallel_move_str2); */
		logerror("DSP56k: Unemulated Dual X Memory Data And Register Data Move @ 0x%x\n", PC);
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
				size = dsp56k_op_clr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ADD : .... .... 0000 FJJJ : A-22 */
			else if ((op_byte & 0x00f0) == 0x0000)
			{
				size = dsp56k_op_add(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* MOVE : .... .... 0001 0001 : A-128 */
			else if ((op_byte & 0x00ff) == 0x0011)
			{
				size = dsp56k_op_move(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* TFR : .... .... 0001 FJJJ : A-212 */
			else if ((op_byte & 0x00f0) == 0x0010)
			{
				size = dsp56k_op_tfr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* RND : .... .... 0010 F000 : A-188 */
			else if ((op_byte & 0x00f7) == 0x0020)
			{
				size = dsp56k_op_rnd(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* TST : .... .... 0010 F001 : A-218 */
			else if ((op_byte & 0x00f7) == 0x0021)
			{
				size = dsp56k_op_tst(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* INC : .... .... 0010 F010 : A-104 */
			else if ((op_byte & 0x00f7) == 0x0022)
			{
				size = dsp56k_op_inc(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* INC24 : .... .... 0010 F011 : A-106 */
			else if ((op_byte & 0x00f7) == 0x0023)
			{
				size = dsp56k_op_inc24(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* OR : .... .... 0010 F1JJ : A-176 */
			else if ((op_byte & 0x00f4) == 0x0024)
			{
				size = dsp56k_op_or(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* ASR : .... .... 0011 F000 : A-32 */
			else if ((op_byte & 0x00f7) == 0x0030)
			{
				size = dsp56k_op_asr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ASL : .... .... 0011 F001 : A-28 */
			else if ((op_byte & 0x00f7) == 0x0031)
			{
				size = dsp56k_op_asl(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* LSR : .... .... 0011 F010 : A-120 */
			else if ((op_byte & 0x00f7) == 0x0032)
			{
				size = dsp56k_op_lsr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* LSL : .... .... 0011 F011 : A-118 */
			else if ((op_byte & 0x00f7) == 0x0033)
			{
				size = dsp56k_op_lsl(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* EOR : .... .... 0011 F1JJ : A-94 */
			else if ((op_byte & 0x00f4) == 0x0034)
			{
				size = dsp56k_op_eor(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* SUBL : .... .... 0100 F001 : A-204 */
			else if ((op_byte & 0x00f7) == 0x0041)
			{
				size = dsp56k_op_subl(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* SUB : .... .... 0100 FJJJ : A-202 */
			else if ((op_byte & 0x00f0) == 0x0040)
			{
				size = dsp56k_op_sub(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* CLR24 : .... .... 0101 F001 : A-62 */
			else if ((op_byte & 0x00f7) == 0x0051)
			{
				size = dsp56k_op_clr24(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* SBC : .... .... 0101 F01J : A-198 */
			else if ((op_byte & 0x00f6) == 0x0052)
			{
				size = dsp56k_op_sbc(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* CMP : .... .... 0101 FJJJ : A-64 */
			else if ((op_byte & 0x00f0) == 0x0050)
			{
				size = dsp56k_op_cmp(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* NEG : .... .... 0110 F000 : A-166 */
			else if ((op_byte & 0x00f7) == 0x0060)
			{
				size = dsp56k_op_neg(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* NOT : .... .... 0110 F001 : A-174 */
			else if ((op_byte & 0x00f7) == 0x0061)
			{
				size = dsp56k_op_not(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* DEC : .... .... 0110 F010 : A-72 */
			else if ((op_byte & 0x00f7) == 0x0062)
			{
				size = dsp56k_op_dec(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* DEC24 : .... .... 0110 F011 : A-74 */
			else if ((op_byte & 0x00f7) == 0x0063)
			{
				size = dsp56k_op_dec24(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* AND : .... .... 0110 F1JJ : A-24 */
			else if ((op_byte & 0x00f4) == 0x0064)
			{
				size = dsp56k_op_and(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* ABS : .... .... 0111 F001 : A-18 */
			if ((op_byte & 0x00f7) == 0x0071)
			{
				size = dsp56k_op_abs(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ROR : .... .... 0111 F010 : A-192 */
			else if ((op_byte & 0x00f7) == 0x0072)
			{
				size = dsp56k_op_ror(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ROL : .... .... 0111 F011 : A-190 */
			else if ((op_byte & 0x00f7) == 0x0073)
			{
				size = dsp56k_op_rol(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* CMPM : .... .... 0111 FJJJ : A-66 */
			else if ((op_byte & 0x00f0) == 0x0070)
			{
				size = dsp56k_op_cmpm(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
			else if ((op_byte & 0x00b0) == 0x0080)
			{
				size = dsp56k_op_mpy(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MPYR : .... .... 1k01 FQQQ : A-162 */
			else if ((op_byte & 0x00b0) == 0x0090)
			{
				size = dsp56k_op_mpyr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MAC : .... .... 1k10 FQQQ : A-122 */
			else if ((op_byte & 0x00b0) == 0x00a0)
			{
				size = dsp56k_op_mac(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
			else if ((op_byte & 0x00b0) == 0x00b0)
			{
				size = dsp56k_op_macr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* Now evaluate the parallel data move */
			switch (parallelType)
			{
			case kNoParallelDataMove:
				/* DO NOTHING */
				break;
			case kRegisterToRegister:
				execute_register_to_register_data_move(cpustate, op, &d_register, &prev_accum_value);
				break;
			case kAddressRegister:
				execute_address_register_update(cpustate, op, &d_register, &prev_accum_value);
				break;
			case kXMemoryDataMove:
				execute_x_memory_data_move(cpustate, op, &d_register, &prev_accum_value);
				break;
			case kXMemoryDataMove2:
				execute_x_memory_data_move2(cpustate, op, &d_register);
				break;
			case kXMemoryDataMoveWithDisp:
				execute_x_memory_data_move_with_short_displacement(cpustate, op, op2);
				size = 2;
				break;
			}
		}
	}

	/* Drop out if you've already completed your work. */
	if (size != 0x1337)
	{
		PC += (UINT16)size;

		dsp56k_process_loop(cpustate);
		dsp56k_process_rep(cpustate, size);

		cpustate->icount -= 4;  /* Temporarily hard-coded at 4 clocks per opcode */ /* cycle_count */
		return;
	}


	/******************************/
	/* Remaining non-parallel ops */
	/******************************/

	/* ADC : 0001 0101 0000 F01J : A-20 */
	if ((op & 0xfff6) == 0x1502)
	{
		size = dsp56k_op_adc(cpustate, op, &cycle_count);
	}
	/* ANDI : 0001 1EE0 iiii iiii : A-26 */
	/* (MoveP sneaks in here if you don't check 0x0600) */
	else if (((op & 0xf900) == 0x1800) & ((op & 0x0600) != 0x0000))
	{
		size = dsp56k_op_andi(cpustate, op, &cycle_count);
	}
	/* ASL4 : 0001 0101 0011 F001 : A-30 */
	else if ((op & 0xfff7) == 0x1531)
	{
		size = dsp56k_op_asl4(cpustate, op, &cycle_count);
	}
	/* ASR4 : 0001 0101 0011 F000 : A-34 */
	else if ((op & 0xfff7) == 0x1530)
	{
		size = dsp56k_op_asr4(cpustate, op, &cycle_count);
	}
	/* ASR16 : 0001 0101 0111 F000 : A-36 */
	else if ((op & 0xfff7) == 0x1570)
	{
		size = dsp56k_op_asr16(cpustate, op, &cycle_count);
	}
	/* BFCHG : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFCHG : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFCHG : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56k_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56k_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56k_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56k_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56k_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
	else if (((op & 0xff30) == 0x0730) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bcc(cpustate, op, op2, &cycle_count);
	}
	/* Bcc : 0010 11cc ccee eeee : A-48 */
	else if ((op & 0xfc00) == 0x2c00)
	{
		size = dsp56k_op_bcc_1(cpustate, op, &cycle_count);
	}
	/* Bcc : 0000 0111 RR10 cccc : A-48 */
	else if ((op & 0xff30) == 0x0720)
	{
		size = dsp56k_op_bcc_2(cpustate, op, &cycle_count);
	}
	/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
	else if (((op & 0xfffc) == 0x013c) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bra(cpustate, op, op2, &cycle_count);
	}
	/* BRA : 0000 1011 aaaa aaaa : A-50 */
	else if ((op & 0xff00) == 0x0b00)
	{
		size = dsp56k_op_bra_1(cpustate, op, &cycle_count);
	}
	/* BRA : 0000 0001 0010 11RR : A-50 */
	else if ((op & 0xfffc) == 0x012c)
	{
		size = dsp56k_op_bra_2(cpustate, op, &cycle_count);
	}
	/* BRKc : 0000 0001 0001 cccc : A-52 */
	else if ((op & 0xfff0) == 0x0110)
	{
		size = dsp56k_op_brkcc(cpustate, op, &cycle_count);
	}
	/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
	else if (((op & 0xff30) == 0x0710) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bscc(cpustate, op, op2, &cycle_count);
	}
	/* BScc : 0000 0111 RR00 cccc : A-54 */
	else if ((op & 0xff30) == 0x0700)
	{
		size = dsp56k_op_bscc_1(cpustate, op, &cycle_count);
	}
	/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
	else if (((op & 0xfffc) == 0x0138) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_bsr(cpustate, op, op2, &cycle_count);
	}
	/* BSR : 0000 0001 0010 10RR : A-56 */
	else if ((op & 0xfffc) == 0x0128)
	{
		size = dsp56k_op_bsr_1(cpustate, op, &cycle_count);
	}
	/* CHKAAU : 0000 0000 0000 0100 : A-58 */
	else if ((op & 0xffff) == 0x0004)
	{
		size = dsp56k_op_chkaau(cpustate, op, &cycle_count);
	}
	/* DEBUG : 0000 0000 0000 0001 : A-68 */
	else if ((op & 0xffff) == 0x0001)
	{
		size = dsp56k_op_debug(cpustate, op, &cycle_count);
	}
	/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
	else if ((op & 0xfff0) == 0x0050)
	{
		size = dsp56k_op_debugcc(cpustate, op, &cycle_count);
	}
	/* DIV : 0001 0101 0--0 F1DD : A-76 */
	else if ((op & 0xff94) == 0x1504)
	{
		size = dsp56k_op_div(cpustate, op, &cycle_count);
	}
	/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
	else if ((op & 0xffd0) == 0x1590)
	{
		size = dsp56k_op_dmac(cpustate, op, &cycle_count);
	}
	/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x00c0) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_do(cpustate, op, op2, &cycle_count);
	}
	/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xff00) == 0x0e00) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_do_1(cpustate, op, op2, &cycle_count);
	}
	/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x0400) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_do_2(cpustate, op, op2, &cycle_count);
	}
	/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
	else if (((op & 0xffff) == 0x0002) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_doforever(cpustate, op, op2, &cycle_count);
	}
	/* ENDDO : 0000 0000 0000 1001 : A-92 */
	else if ((op & 0xffff) == 0x0009)
	{
		size = dsp56k_op_enddo(cpustate, op, &cycle_count);
	}
	/* EXT : 0001 0101 0101 F010 : A-96 */
	else if ((op & 0xfff7) == 0x1552)
	{
		size = dsp56k_op_ext(cpustate, op, &cycle_count);
	}
	/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
	else if ((op & 0xffff) == 0x000f)
	{
		size = dsp56k_op_illegal(cpustate, op, &cycle_count);
	}
	/* IMAC : 0001 0101 1010 FQQQ : A-100 */
	else if ((op & 0xfff0) == 0x15a0)
	{
		size = dsp56k_op_imac(cpustate, op, &cycle_count);
	}
	/* IMPY : 0001 0101 1000 FQQQ : A-102 */
	else if ((op & 0xfff0) == 0x1580)
	{
		size = dsp56k_op_impy(cpustate, op, &cycle_count);
	}
	/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
	else if (((op & 0xff30) == 0x0630) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jcc(cpustate, op, op2, &cycle_count);
	}
	/* Jcc : 0000 0110 RR10 cccc : A-108 */
	else if ((op & 0xff30) == 0x0620 )
	{
		size = dsp56k_op_jcc_1(cpustate, op, &cycle_count);
	}
	/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
	else if (((op & 0xfffc) == 0x0134) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jmp(cpustate, op, op2, &cycle_count);
	}
	/* JMP : 0000 0001 0010 01RR : A-110 */
	else if ((op & 0xfffc) == 0x0124)
	{
		size = dsp56k_op_jmp_1(cpustate, op, &cycle_count);
	}
	/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
	else if (((op & 0xff30) == 0x0610) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jscc(cpustate, op, op2, &cycle_count);
	}
	/* JScc : 0000 0110 RR00 cccc : A-112 */
	else if ((op & 0xff30) == 0x0600)
	{
		size = dsp56k_op_jscc_1(cpustate, op, &cycle_count);
	}
	/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
	else if (((op & 0xfffc) == 0x0130) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_jsr(cpustate, op, op2, &cycle_count);
	}
	/* JSR : 0000 1010 AAAA AAAA : A-114 */
	else if ((op & 0xff00) == 0x0a00)
	{
		size = dsp56k_op_jsr_1(cpustate, op, &cycle_count);
	}
	/* JSR : 0000 0001 0010 00RR : A-114 */
	else if ((op & 0xfffc) == 0x0120)
	{
		size = dsp56k_op_jsr_2(cpustate, op, &cycle_count);
	}
	/* LEA : 0000 0001 11TT MMRR : A-116 */
	else if ((op & 0xffc0) == 0x01c0)
	{
		size = dsp56k_op_lea(cpustate, op, &cycle_count);
	}
	/* LEA : 0000 0001 10NN MMRR : A-116 */
	else if ((op & 0xffc0) == 0x0180)
	{
		size = dsp56k_op_lea_1(cpustate, op, &cycle_count);
	}
	/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
	else if ((op & 0xfff0) == 0x15e0)
	{
		size = dsp56k_op_macsuuu(cpustate, op, &cycle_count);
	}
	/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0x00ff) == 0x0011))
	{
		size = dsp56k_op_move_2(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
	else if ((op & 0xf810) == 0x3800)
	{
		size = dsp56k_op_movec(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
	else if ((op & 0xf814) == 0x3810)
	{
		size = dsp56k_op_movec_1(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
	else if ((op & 0xf816) == 0x3816)
	{
		size = dsp56k_op_movec_2(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
	else if (((op & 0xf816) == 0x3814) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56k_op_movec_3(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
	else if ((op & 0xfc00) == 0x2800)
	{
		size = dsp56k_op_movec_4(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xf810) == 0x3800))
	{
		size = dsp56k_op_movec_5(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
	else if ((op & 0xfc00) == 0x2000)
	{
		size = dsp56k_op_movei(cpustate, op, &cycle_count);
	}
	/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
	else if ((op & 0xfe20) == 0x0200)
	{
		size = dsp56k_op_movem(cpustate, op, &cycle_count);
	}
	/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
	else if ((op & 0xfe30) == 0x0230)
	{
		size = dsp56k_op_movem_1(cpustate, op, &cycle_count);
	}
	/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xfe20) == 0x0200))
	{
		size = dsp56k_op_movem_2(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
	else if ((op & 0xfe20) == 0x1820)
	{
		size = dsp56k_op_movep(cpustate, op, &cycle_count);
	}
	/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
	else if ((op & 0xfe00) == 0x0c00)
	{
		size = dsp56k_op_movep_1(cpustate, op, &cycle_count);
	}
	/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
	else if ((op & 0xfe20) == 0x1800)
	{
		size = dsp56k_op_moves(cpustate, op, &cycle_count);
	}
	/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
	else if ((op & 0xfff0) == 0x15c0)
	{
		size = dsp56k_op_mpysuuu(cpustate, op, &cycle_count);
	}
	/* NEGC : 0001 0101 0110 F000 : A-168 */
	else if ((op & 0xfff7) == 0x1560)
	{
		size = dsp56k_op_negc(cpustate, op, &cycle_count);
	}
	/* NOP : 0000 0000 0000 0000 : A-170 */
	else if ((op & 0xffff) == 0x0000)
	{
		size = dsp56k_op_nop(cpustate, op, &cycle_count);
	}
	/* NORM : 0001 0101 0010 F0RR : A-172 */
	else if ((op & 0xfff4) == 0x1520)
	{
		size = dsp56k_op_norm(cpustate, op, &cycle_count);
	}
	/* ORI : 0001 1EE1 iiii iiii : A-178 */
	else if ((op & 0xf900) == 0x1900)
	{
		size = dsp56k_op_ori(cpustate, op, &cycle_count);
	}
	/* REP : 0000 0000 111- --RR : A-180 */
	else if ((op & 0xffe0) == 0x00e0)
	{
		size = dsp56k_op_rep(cpustate, op, &cycle_count);
	}
	/* REP : 0000 1111 iiii iiii : A-180 */
	else if ((op & 0xff00) == 0x0f00)
	{
		size = dsp56k_op_rep_1(cpustate, op, &cycle_count);
	}
	/* REP : 0000 0100 001D DDDD : A-180 */
	else if ((op & 0xffe0) == 0x0420)
	{
		size = dsp56k_op_rep_2(cpustate, op, &cycle_count);
	}
	/* REPcc : 0000 0001 0101 cccc : A-184 */
	else if ((op & 0xfff0) == 0x0150)
	{
		size = dsp56k_op_repcc(cpustate, op, &cycle_count);
	}
	/* RESET : 0000 0000 0000 1000 : A-186 */
	else if ((op & 0xffff) == 0x0008)
	{
		size = dsp56k_op_reset(cpustate, op, &cycle_count);
	}
	/* RTI : 0000 0000 0000 0111 : A-194 */
	else if ((op & 0xffff) == 0x0007)
	{
		size = dsp56k_op_rti(cpustate, op, &cycle_count);
	}
	/* RTS : 0000 0000 0000 0110 : A-196 */
	else if ((op & 0xffff) == 0x0006)
	{
		size = dsp56k_op_rts(cpustate, op, &cycle_count);
	}
	/* STOP : 0000 0000 0000 1010 : A-200 */
	else if ((op & 0xffff) == 0x000a)
	{
		size = dsp56k_op_stop(cpustate, op, &cycle_count);
	}
	/* SWAP : 0001 0101 0111 F001 : A-206 */
	else if ((op & 0xfff7) == 0x1571)
	{
		size = dsp56k_op_swap(cpustate, op, &cycle_count);
	}
	/* SWI : 0000 0000 0000 0101 : A-208 */
	else if ((op & 0xffff) == 0x0005)
	{
		size = dsp56k_op_swi(cpustate, op, &cycle_count);
	}
	/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
	else if ((op & 0xfc02) == 0x1000)
	{
		size = dsp56k_op_tcc(cpustate, op, &cycle_count);
	}
	/* TFR(2) : 0001 0101 0000 F00J : A-214 */
	else if ((op & 0xfff6) == 0x1500)
	{
		size = dsp56k_op_tfr2(cpustate, op, &cycle_count);
	}
	/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
	else if ((op & 0xfc00) == 0x2400)
	{
		size = dsp56k_op_tfr3(cpustate, op, &cycle_count);
	}
	/* TST(2) : 0001 0101 0001 -1DD : A-220 */
	else if ((op & 0xfff4) == 0x1514)
	{
		size = dsp56k_op_tst2(cpustate, op, &cycle_count);
	}
	/* WAIT : 0000 0000 0000 1011 : A-222 */
	else if ((op & 0xffff) == 0x000b)
	{
		size = dsp56k_op_wait(cpustate, op, &cycle_count);
	}
	/* ZERO : 0001 0101 0101 F000 : A-224 */
	else if ((op & 0xfff7) == 0x1550)
	{
		size = dsp56k_op_zero(cpustate, op, &cycle_count);
	}


	/* Not recognized?  Nudge debugger onto the next word */
	if (size == 0x1337)
	{
		logerror("DSP56k: Unimplemented opcode at 0x%04x : %04x\n", PC, op);
		size = 1 ;                      /* Just to get the debugger past the bad opcode */
	}

	/* Must have been a good opcode */
	PC += (UINT16)size;

	dsp56k_process_loop(cpustate);
	dsp56k_process_rep(cpustate, size);

	cpustate->icount -= 4;  /* Temporarily hard-coded at 4 clocks per opcode */ /* cycle_count */
}




/***************************************************************************
    Opcode implementations
***************************************************************************/

/*******************************/
/* 32 Parallel move operations */
/*******************************/

/* ADD : 011m mKKK 0rru Fuuu : A-22 */
/* SUB : 011m mKKK 0rru Fuuu : A-202 */
static size_t dsp56k_op_addsub_2(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	UINT64 useVal = 0;
	UINT8 op_type = OP_OTHER;
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_uuuuF_table(cpustate, BITS(op_byte,0x0017), BITS(op_byte,0x0008), op_type, &S, &D);

	/* If you gave an invalid operation type, presume it's a nop and move on with the parallel move */
	if (op_type == OP_OTHER)
	{
		d_register->addr = NULL;
		d_register->data_type = DT_BYTE;
		cycles += 2;
		return 1;
	}

	/* It's a real operation.  Get on with it. */
	switch(S.data_type)
	{
		case DT_WORD:        useVal = (UINT64)*((UINT16*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: useVal = (UINT64)*((UINT32*)S.addr);       break;
		case DT_LONG_WORD:   useVal = (UINT64)*((UINT64*)S.addr);       break;
	}

	/* Sign-extend word for proper add/sub op */
	if ((S.data_type == DT_WORD) && useVal & U64(0x0000000080000000))
		useVal |= U64(0x000000ff00000000);

	/* Operate*/
	if (op_type == OP_ADD)
		*((UINT64*)D.addr) += useVal;
	else if (op_type == OP_SUB)
		*((UINT64*)D.addr) -= useVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U, V, C */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (*((UINT64*)D.addr) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
static size_t dsp56k_op_mac_1(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	INT64 opD = 0;
	INT64 result = 0;

	INT32 s1 = 0;
	INT32 s2 = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S1, &S2, &D);

	/* Cast both values as being signed */
	s1 = *((INT16*)S1);
	s2 = *((INT16*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	result = (s1 * s2) << 1;

	/* Sign extend D into a temp variable */
	opD = (*((UINT64*)D));
	if (opD & U64(0x0000008000000000))
		opD |= U64(0xffffff0000000000);
	else
		opD &= U64(0x000000ffffffffff);

	/* Accumulate */
	opD += result;

	/* And out the bits that don't live in the register */
	opD &= U64(0x000000ffffffffff);

	(*((UINT64*)D)) = (UINT64)opD;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
static size_t dsp56k_op_macr_1(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
static size_t dsp56k_op_move_1(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
static size_t dsp56k_op_mpy_1(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	INT64 result = 0;

	INT32 s1 = 0;
	INT32 s2 = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S1, &S2, &D);

	/* Cast both values as being signed */
	s1 = *((INT16*)S1);
	s2 = *((INT16*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	result = (s1 * s2) << 1;

	/* And out the bits that don't live in the register */
	(*((UINT64*)D)) = result & U64(0x000000ffffffffff);

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
static size_t dsp56k_op_mpyr_1(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
static size_t dsp56k_op_tfr_2(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* MPY : 0001 0110 RRDD FQQQ : A-160 */
static size_t dsp56k_op_mpy_2(dsp56k_core* cpustate, const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MAC : 0001 0111 RRDD FQQQ : A-122 */
static size_t dsp56k_op_mac_2(dsp56k_core* cpustate, const UINT16 op_byte, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* CLR : .... .... 0000 F001 : A-60 */
static size_t dsp56k_op_clr(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_LONG_WORD};
	typed_pointer clear = {NULL, DT_LONG_WORD};
	UINT64 clear_val = U64(0x0000000000000000);

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	clear.addr = &clear_val;
	clear.data_type = DT_LONG_WORD;
	SetDestinationValue(clear, D);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * 0 - */
	/* TODO - S, L */
	DSP56K_E_CLEAR();
	DSP56K_U_SET();
	DSP56K_N_CLEAR();
	DSP56K_Z_SET();
	DSP56K_V_CLEAR();

	cycles += 2;    /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* ADD : .... .... 0000 FJJJ : A-22 */
static size_t dsp56k_op_add(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT64 addVal = 0;

	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};
	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((UINT64*)D.addr);

	switch(S.data_type)
	{
		case DT_WORD:        addVal = (UINT64)*((UINT16*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: addVal = (UINT64)*((UINT32*)S.addr);       break;
		case DT_LONG_WORD:   addVal = (UINT64)*((UINT64*)S.addr);       break;
	}

	/* Sign-extend word for proper add/sub op */
	if ((S.data_type == DT_WORD) && addVal & U64(0x0000000080000000))
		addVal |= U64(0x000000ff00000000);

	/* Operate*/
	*((UINT64*)D.addr) += addVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U, V, C */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (*((UINT64*)D.addr) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* MOVE : .... .... 0001 0001 : A-128 */
static size_t dsp56k_op_move(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* Equivalent to a nop with a parallel move */
	/* These can't be used later.  Hopefully compilers would pick this up. */
	*p_accum = 0;
	d_register->addr = NULL;
	d_register->data_type = DT_BYTE;

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;    /* TODO: + mv oscillator cycles */
	return 1;
}

/* TFR : .... .... 0001 FJJJ : A-212 */
static size_t dsp56k_op_tfr(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((UINT64*)D.addr);

	SetDestinationValue(S, D);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* RND : .... .... 0010 F000 : A-188 */
static size_t dsp56k_op_rnd(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	/* WARNING : ROUNDING NOT FULLY IMPLEMENTED YET! */
	if ((*((UINT64*)D.addr) & U64(0x000000000000ffff)) >= 0x8000)
		*((UINT64*)D.addr) += U64(0x0000000000010000);

	*((UINT64*)D.addr) = *((UINT64*)D.addr) & U64(0x000000ffffff0000);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, U, V */
	if ((*((UINT64*)D.addr)) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr)) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* TST : .... .... 0010 F001 : A-218 */
static size_t dsp56k_op_tst(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_LONG_WORD};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* 0 * * * * * 0 0 */
	/* TODO: S, L, E, U */
	if ((*((UINT64*)D.addr)) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr)) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();
	DSP56K_C_CLEAR();

	cycles += 2;    /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* INC : .... .... 0010 F010 : A-104 */
static size_t dsp56k_op_inc(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);

	/* Make sure the destination is a real 40-bit value */
	*((UINT64*)D.addr) &= U64(0x000000ffffffffff);

	/* Increment */
	*((UINT64*)D.addr) = *((UINT64*)D.addr) + 1;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U */
	if ( *((UINT64*)D.addr) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x000000ffffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0xffffff0000000000)) != 0) DSP56K_V_SET(); else DSP56K_V_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0xffffff0000000000)) != 0) DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;    /* TODO: +mv oscillator cycles */
	return 1;
}

/* INC24 : .... .... 0010 F011 : A-106 */
static size_t dsp56k_op_inc24(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT32 workBits24;

	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);

	/* TODO: I wonder if workBits24 should be signed? */
	workBits24 = ((*((UINT64*)D.addr)) & U64(0x000000ffffff0000)) >> 16;
	workBits24++;
	//workBits24 &= 0x00ffffff;     /* Solves -x issues - TODO: huh? */

	/* Set the D bits with the dec result */
	*((UINT64*)D.addr) &= U64(0x000000000000ffff);
	*((UINT64*)D.addr) |= (((UINT64)(workBits24)) << 16);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * ? * * */
	/* TODO: S, L, E, U */
	if ( *((UINT64*)D.addr) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x000000ffffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ((workBits24 & 0xff000000) != 0) DSP56K_V_SET(); else DSP56K_V_CLEAR();
	if ((workBits24 & 0xff000000) != 0) DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* OR : .... .... 0010 F1JJ : A-176 */
static size_t dsp56k_op_or(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S, &D);

	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);

	/* OR a word of S with A1|B1 */
	((PAIR64*)D.addr)->w.h = *((UINT16*)S.addr) | ((PAIR64*)D.addr)->w.h;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S, L */
	if ( *((UINT64*)D.addr) & U64(0x0000000080000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x00000000ffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* ASR : .... .... 0011 F000 : A-32 */
static size_t dsp56k_op_asr(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

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
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (*((UINT64*)D.addr) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();
	if (*p_accum & U64(0x0000000000000001))           DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* ASL : .... .... 0011 F001 : A-28 */
static size_t dsp56k_op_asl(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * ? ? */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significant
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	/* C - Set if bit 39 of source operand is set. Cleared otherwise. */
	return 0;
}

/* LSR : .... .... 0011 F010 : A-120 */
static size_t dsp56k_op_lsr(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

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
	DSP56K_N_CLEAR();
	if (((PAIR64*)D.addr)->w.h == 0)        DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();
	if (*p_accum & U64(0x0000000000010000)) DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* LSL : .... .... 0011 F011 : A-118 */
static size_t dsp56k_op_lsl(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 31 of the source operand is set. Cleared otherwise. */
	return 0;
}

/* EOR : .... .... 0011 F1JJ : A-94 */
static size_t dsp56k_op_eor(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	return 0;
}

/* SUBL : .... .... 0100 F001 : A-204 */
static size_t dsp56k_op_subl(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * ? * */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significant
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	return 0;
}

/* SUB : .... .... 0100 FJJJ : A-202 */
static size_t dsp56k_op_sub(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT64 useVal = 0;
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S, &D);

	/* Get on with it. */
	switch(S.data_type)
	{
		case DT_WORD:        useVal = (UINT64)*((UINT16*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: useVal = (UINT64)*((UINT32*)S.addr);       break;
		case DT_LONG_WORD:   useVal = (UINT64)*((UINT64*)S.addr);       break;
	}

	/* Sign-extend word for proper sub op */
	if ((S.data_type == DT_WORD) && useVal & U64(0x0000000080000000))
		useVal |= U64(0x000000ff00000000);

	/* Make sure they're both real 40-bit values */
	useVal &= U64(0x000000ffffffffff);
	*((UINT64*)D.addr) &= U64(0x000000ffffffffff);

	/* Operate*/
	*((UINT64*)D.addr) -= useVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U */
	if ( *((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ( *((UINT64*)D.addr) == 0)                     DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0xffffff0000000000)) != 0) DSP56K_V_SET(); else DSP56K_V_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0xffffff0000000000)) != 0) DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* CLR24 : .... .... 0101 F001 : A-62 */
static size_t dsp56k_op_clr24(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * ? 0 - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	return 0;
}

/* SBC : .... .... 0101 F01J : A-198 */
static size_t dsp56k_op_sbc(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* CMP : .... .... 0101 FJJJ : A-64 */
static size_t dsp56k_op_cmp(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT64 cmpVal = 0;
	UINT64 result = 0;

	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((UINT64*)D.addr);

	switch(S.data_type)
	{
		case DT_WORD:        cmpVal = (UINT64)*((UINT16*)S.addr) << 16;  break;
		case DT_DOUBLE_WORD: cmpVal = (UINT64)*((UINT32*)S.addr);  break;
		case DT_LONG_WORD:   cmpVal = (UINT64)*((UINT64*)S.addr);  break;
	}

	/* Sign-extend word for proper subtraction op */
	if ((S.data_type == DT_WORD) && cmpVal & U64(0x0000000080000000))
		cmpVal |= U64(0x000000ff00000000);

	/* Make sure they're both real 40-bit values */
	cmpVal &= U64(0x000000ffffffffff);
	*((UINT64*)D.addr) &= U64(0x000000ffffffffff);

	/* Operate */
	result = *((UINT64*)D.addr) - cmpVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U */
	if ( result & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ( result == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ((result & U64(0xffffff0000000000)) != 0) DSP56K_V_SET(); else DSP56K_V_CLEAR();
	if ((result & U64(0xffffff0000000000)) != 0) DSP56K_C_SET(); else DSP56K_C_CLEAR();


	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* NEG : .... .... 0110 F000 : A-166 */
static size_t dsp56k_op_neg(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* NOT : .... .... 0110 F001 : A-174 */
static size_t dsp56k_op_not(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	/* Invert bits [16:31] of D */
	((PAIR64*)D.addr)->w.h = ~(((PAIR64*)D.addr)->w.h);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S?, L */
	if ( *((UINT64*)D.addr) & U64(0x0000000080000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x00000000ffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* DEC : .... .... 0110 F010 : A-72 */
static size_t dsp56k_op_dec(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	return 0;
}

/* DEC24 : .... .... 0110 F011 : A-74 */
static size_t dsp56k_op_dec24(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT32 workBits24;

	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);

	/* TODO: I wonder if workBits24 should be signed? */
	workBits24 = ((*((UINT64*)D.addr)) & U64(0x000000ffffff0000)) >> 16;
	workBits24--;
	workBits24 &= 0x00ffffff;       /* Solves -x issues */

	/* Set the D bits with the dec result */
	*((UINT64*)D.addr) &= U64(0x000000000000ffff);
	*((UINT64*)D.addr) |= (((UINT64)(workBits24)) << 16);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * ? * * */
	/* TODO: S, L, E, U, V, C */
	if ( *((UINT64*)D.addr) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x000000ffffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* AND : .... .... 0110 F1JJ : A-24 */
static size_t dsp56k_op_and(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S, &D);

	/* Save some data for the parallel move */
	*p_accum = *((UINT64*)D.addr);

	/* AND a word of S with A1|B1 */
	((PAIR64*)D.addr)->w.h = *((UINT16*)S.addr) & ((PAIR64*)D.addr)->w.h;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S, L */
	if ( *((UINT64*)D.addr) & U64(0x0000000080000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x00000000ffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* ABS : .... .... 0111 F001 : A-18 */
static size_t dsp56k_op_abs(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	INT64 opD = 0;
	typed_pointer D = {NULL, DT_LONG_WORD};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((UINT64*)D.addr);

	/* Sign extend D into a temp variable */
	opD = *p_accum;
	if (opD &  U64(0x0000008000000000))
		opD |= U64(0xffffff0000000000);
	else
		opD &= U64(0x000000ffffffffff);

	/* Take the absolute value and clean up */
	opD = (opD < 0) ? -opD : opD;
	opD &= U64(0x000000ffffffffff);

	/* Reassign */
	*((UINT64*)D.addr) = opD;

	/* Special overflow case */
	if ((*p_accum) == U64(0x0000008000000000))
		*((UINT64*)D.addr) = U64(0x0000007fffffffff);

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, U */
	if ( *((UINT64*)D.addr) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D.addr) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ((*p_accum)         == U64(0x0000008000000000))       DSP56K_V_SET(); else DSP56K_V_CLEAR();

	cycles += 2;            /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* ROR : .... .... 0111 F010 : A-192 */
static size_t dsp56k_op_ror(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 16 of the source operand is set. Cleared otherwise. */
	return 0;
}

/* ROL : .... .... 0111 F011 : A-190 */
static size_t dsp56k_op_rol(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 31 of the source operand is set. Cleared otherwise. */
	return 0;
}

/* CMPM : .... .... 0111 FJJJ : A-66 */
static size_t dsp56k_op_cmpm(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	INT64 absS;
	INT64 absD;
	INT64 absResult;

	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((UINT64*)D.addr);

	/* Sign extend and get absolute value of the source */
	if (S.addr == &A || S.addr == &B)
	{
		absS = *((UINT64*)S.addr);
		if (absS &  U64(0x0000008000000000))
			absS |= U64(0xffffff8000000000);
	}
	else
	{
		absS = (*((UINT16*)S.addr)) << 16;
		if (absS &  U64(0x0000000080000000))
			absS |= U64(0xffffffff80000000);
	}
	absS = (absS < 0) ? -absS : absS;

	/* Sign extend and get absolute value of the destination */
	if (D.addr == &A || D.addr == &B)
	{
		absD = *((UINT64*)D.addr);
		if (absD &  U64(0x0000008000000000))
			absD |= U64(0xffffff8000000000);
	}
	else
	{
		absD = (*((UINT16*)D.addr)) << 16;
		if (absS &  U64(0x0000000080000000))
			absS |= U64(0xffffffff80000000);
	}
	absD = (absD < 0) ? -absD : absD;

	/* Compare */
	absResult = absD - absS;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U */
	if ( (absResult) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (((absResult) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ( (absResult  & U64(0xffffff0000000000)) != 0)  DSP56K_V_SET(); else DSP56K_V_CLEAR();
	if ( (absResult  & U64(0xffffff0000000000)) != 0)  DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
static size_t dsp56k_op_mpy(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT16 k = 0;
	INT64 result = 0;

	INT32 s1 = 0;
	INT32 s2 = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQQF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S1, &S2, &D);

	k = BITS(op_byte,0x0040);

	/* Cast both values as being signed */
	s1 = *((INT16*)S1);
	s2 = *((INT16*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	result = (s1 * s2) << 1;

	/* Negate the product if necessary */
	if (k)
		result *= -1;

	(*((UINT64*)D)) = result & U64(0x000000ffffffffff);

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MPYR : .... .... 1k01 FQQQ : A-162 */
static size_t dsp56k_op_mpyr(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	return 0;
}

/* MAC : .... .... 1k10 FQQQ : A-122 */
static size_t dsp56k_op_mac(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT16 k = 0;
	INT64 opD = 0;
	INT64 result = 0;

	INT32 s1 = 0;
	INT32 s2 = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQQF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S1, &S2, &D);

	k = BITS(op_byte,0x0040);

	/* Cast both values as being signed */
	s1 = *((INT16*)S1);
	s2 = *((INT16*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	result = (s1 * s2) << 1;

	/* Sign extend D into a temp variable */
	opD = (*((UINT64*)D));
	if (opD & U64(0x0000008000000000))
		opD |= U64(0xffffff0000000000);
	else
		opD &= U64(0x000000ffffffffff);

	/* Negate if necessary */
	if (k)
		result *= -1;

	/* Accumulate */
	opD += result;

	/* And out the bits that don't live in the register */
	opD &= U64(0x000000ffffffffff);

	(*((UINT64*)D)) = (UINT64)opD;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
static size_t dsp56k_op_macr(dsp56k_core* cpustate, const UINT16 op_byte, typed_pointer* d_register, UINT64* p_accum, UINT8* cycles)
{
	UINT16 k = 0;
	INT64 opD = 0;
	INT64 result = 0;

	INT32 s1 = 0;
	INT32 s2 = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQQF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S1, &S2, &D);

	k = BITS(op_byte,0x0040);

	/* Cast both values as being signed */
	s1 = *((INT16*)S1);
	s2 = *((INT16*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	result = (s1 * s2) << 1;

	/* Sign extend D into a temp variable */
	opD = (*((UINT64*)D));
	if (opD & U64(0x0000008000000000))
		opD |= U64(0xffffff0000000000);
	else
		opD &= U64(0x000000ffffffffff);

	/* Negate if necessary */
	if (k)
		result *= -1;

	/* Accumulate */
	opD += result;

	/* Round the result */
	/* WARNING : ROUNDING NOT FULLY IMPLEMENTED YET! */
	if ((opD & U64(0x000000000000ffff)) >= 0x8000)
		opD += U64(0x0000000000010000);

	opD &= U64(0x000000ffffff0000);

	/* And out the bits that don't live in the register */
	opD &= U64(0x000000ffffffffff);

	/* Store the result */
	(*((UINT64*)D)) = (UINT64)opD;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}


/******************************/
/* Remaining non-parallel ops */
/******************************/

/* ADC : 0001 0101 0000 F01J : A-20 */
static size_t dsp56k_op_adc(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * * */
	return 0;
}

/* ANDI : 0001 1EE0 iiii iiii : A-26 */
static size_t dsp56k_op_andi(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT16 immediate = BITS(op,0x00ff);

	/* There is not currently a good way to refer to CCR or MR.  Explicitly decode here. */
	switch(BITS(op,0x0600))
	{
		case 0x01:  /* MR */
			SR &= ((immediate << 8) | 0x00ff);
			break;

		case 0x02:  /* CCR */
			SR &= (immediate | 0xff00);
			break;

		case 0x03:  /* OMR */
			OMR &= (UINT8)(immediate);
			break;

		default:
			fatalerror("DSP56k - BAD EE value in andi operation\n") ;
	}

	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Cleared if the corresponding bit in the immediate data is cleared and if the operand
	   is the CCR. Not affected otherwise. */
	cycles += 2;
	return 1;
}

/* ASL4 : 0001 0101 0011 F001 : A-30 */
static size_t dsp56k_op_asl4(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT64 p_accum = 0;
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op,0x0008), &D);

	p_accum = *((UINT64*)D.addr);

	*((UINT64*)D.addr) = (*((UINT64*)D.addr)) << 4;
	*((UINT64*)D.addr) = (*((UINT64*)D.addr)) & U64(0x000000ffffffffff);

	/* S L E U N Z V C */
	/* - ? * * * * ? ? */
	/* TODO: L, E, U  */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if bit 35 through 39 are
	       not the same. */
	/* C - Set if bit 36 of source operand is set. Cleared otherwise. */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (*((UINT64*)D.addr) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	if ( (*((UINT64*)D.addr) & U64(0x000000ff00000000)) != (p_accum & U64(0x000000ff00000000)) ) DSP56K_V_SET(); else DSP56K_V_CLEAR();
	if (p_accum & U64(0x0000001000000000))            DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;
	return 1;
}

/* ASR4 : 0001 0101 0011 F000 : A-34 */
static size_t dsp56k_op_asr4(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT64 p_accum = 0;
	typed_pointer D = {NULL, DT_BYTE};
	decode_F_table(cpustate, BITS(op,0x0008), &D);

	p_accum = *((UINT64*)D.addr);

	*((UINT64*)D.addr) = (*((UINT64*)D.addr)) >> 4;
	*((UINT64*)D.addr) = (*((UINT64*)D.addr)) & U64(0x000000ffffffffff);

	/* The top 4 bits become the old bit 39 */
	if (p_accum & U64(0x0000008000000000))
		*((UINT64*)D.addr) |= U64(0x000000f000000000);
	else
		*((UINT64*)D.addr) &= (~U64(0x000000f000000000));

	/* S L E U N Z V C */
	/* - * * * * * 0 ? */
	/* TODO: E, U  */
	/* C - Set if bit 3 of source operand is set. Cleared otherwise. */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (*((UINT64*)D.addr) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();
	if (p_accum & U64(0x0000000000000008))            DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;
	return 1;
}

/* ASR16 : 0001 0101 0111 F000 : A-36 */
static size_t dsp56k_op_asr16(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT64 backupVal;
	typed_pointer D = {NULL, DT_BYTE};

	decode_F_table(cpustate, BITS(op,0x0008), &D);

	backupVal = *((UINT64*)D.addr);

	*((UINT64*)D.addr) = *((UINT64*)D.addr) >> 16;

	if(backupVal & U64(0x0000008000000000))
		*((UINT64*)D.addr) |= U64(0x000000ffff000000);
	else
		*((UINT64*)D.addr) &= U64(0x0000000000ffffff);

	/* S L E U N Z V C */
	/* - * * * * * 0 ? */
	/* TODO: E, U */
	if (*((UINT64*)D.addr) & U64(0x0000008000000000)) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if (*((UINT64*)D.addr) == 0)                      DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();
	if (backupVal & U64(0x0000000000008000))          DSP56K_C_SET(); else DSP56K_C_CLEAR();

	cycles += 2;
	return 1;
}

/* BFCHG  : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_op_bfop(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT16 workAddr = 0x0000;
	UINT16 workingWord = 0x0000;
	UINT16 previousValue = 0x0000;
	typed_pointer tempTP = { NULL, DT_BYTE };

	UINT16 iVal = op2 & 0x00ff;
	decode_BBB_bitmask(cpustate, BITS(op2,0xe000), &iVal);

	workAddr = assemble_address_from_Pppppp_table(cpustate, BITS(op,0x0020), BITS(op,0x001f));
	previousValue = cpustate->data->read_word(ADDRESS(workAddr));
	workingWord = previousValue;

	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			workingWord ^= iVal;
			break;
		case 0x04:  /* BFCLR */
			workingWord = workingWord & (~iVal);
			break;
		case 0x18:  /* BFSET */
			workingWord = workingWord | iVal;
			break;
		case 0x10:  /* BFTSTH */
			/* Just the test below */
			break;
		case 0x00:  /* BFTSTL */
			/* Just the test below */
			break;
	}

	tempTP.addr = &workingWord;
	tempTP.data_type = DT_WORD;
	SetDataMemoryValue(cpustate, tempTP, ADDRESS(workAddr));

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* TODO: L */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x04:  /* BFCLR */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x18:  /* BFSET */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x10:  /* BFTSTH */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x00:  /* BFTSTL */
			if ((iVal & previousValue) == 0x0000) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
	}

	cycles += 4;    /* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* BFCHG  : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_op_bfop_1(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT16 workAddr = 0x0000;
	UINT16 workingWord = 0x0000;
	UINT16 previousValue = 0x0000;
	typed_pointer R = { NULL, DT_BYTE };
	typed_pointer tempTP = { NULL, DT_BYTE };

	UINT16 iVal = op2 & 0x00ff;
	decode_BBB_bitmask(cpustate, BITS(op2,0xe000), &iVal);

	decode_RR_table(cpustate, BITS(op,0x0003), &R);

	workAddr = *((UINT16*)R.addr);
	previousValue = cpustate->data->read_word(ADDRESS(workAddr));
	workingWord = previousValue;

	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			workingWord ^= iVal;
			break;
		case 0x04:  /* BFCLR */
			workingWord = workingWord & (~iVal);
			break;
		case 0x18:  /* BFSET */
			workingWord = workingWord | iVal;
			break;
		case 0x10:  /* BFTSTH */
			/* Just the test below */
			break;
		case 0x00:  /* BFTSTL */
			/* Just the test below */
			break;
	}

	tempTP.addr = &workingWord;
	tempTP.data_type = DT_WORD;
	SetDataMemoryValue(cpustate, tempTP, ADDRESS(workAddr));

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* TODO: L */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x04:  /* BFCLR */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x18:  /* BFSET */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x10:  /* BFTSTH */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x00:  /* BFTSTL */
			if ((iVal & previousValue) == 0x0000) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
	}

	cycles += 4;    /* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* BFCHG  : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
static size_t dsp56k_op_bfop_2(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT16 workingWord = 0x0000;
	UINT16 previousValue = 0x0000;

	UINT16 iVal = op2 & 0x00ff;
	typed_pointer S = { NULL, DT_BYTE };

	decode_BBB_bitmask(cpustate, BITS(op2,0xe000), &iVal);
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &S);

	/* A & B are special */
	if (S.data_type == DT_LONG_WORD)
		previousValue = ((PAIR64*)S.addr)->w.h;
	else
		previousValue = *((UINT16*)S.addr);

	workingWord = previousValue;

	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			workingWord ^= iVal;
			break;
		case 0x04:  /* BFCLR */
			workingWord = workingWord & (~iVal);
			break;
		case 0x18:  /* BFSET */
			workingWord = workingWord | iVal;
			break;
		case 0x10:  /* BFTSTH */
			/* Just the test below */
			break;
		case 0x00:  /* BFTSTL */
			/* Just the test below */
			break;
	}

	/* Put the data back where it belongs (A & B are special) */
	if (S.data_type == DT_LONG_WORD)
		((PAIR64*)S.addr)->w.h = workingWord;
	else
		*((UINT16*)S.addr) = workingWord;

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* TODO: L */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x04:  /* BFCLR */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x18:  /* BFSET */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x10:  /* BFTSTH */
			if ((iVal & previousValue) == iVal) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
		case 0x00:  /* BFTSTL */
			if ((iVal & previousValue) == 0x0000) DSP56K_C_SET(); else DSP56K_C_CLEAR(); break;
	}

	cycles += 4;    /* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
static size_t dsp56k_op_bcc(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	int shouldBranch = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldBranch)
	{
		INT16 offset = (INT16)op2;

		PC += 2;

		cpustate->ppc = PC;
		PC += offset;

		cycles += 4;
		return 0;
	}
	else
	{
		cycles += 4;
		return 2;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Bcc : 0010 11cc ccee eeee : A-48 */
static size_t dsp56k_op_bcc_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	int shouldBranch = decode_cccc_table(cpustate, BITS(op,0x03c0));

	if (shouldBranch)
	{
		INT16 offset = (INT16)assemble_address_from_6bit_signed_relative_short_address(cpustate, BITS(op,0x003f));

		PC += 1;

		cpustate->ppc = PC;
		PC += offset;

		cycles += 4;
		return 0;
	}
	else
	{
		cycles += 4;
		return 1;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Bcc : 0000 0111 RR10 cccc : A-48 */
static size_t dsp56k_op_bcc_2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
static size_t dsp56k_op_bra(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BRA : 0000 1011 aaaa aaaa : A-50 */
static size_t dsp56k_op_bra_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* 8 bit immediate, relative offset */
	INT8 branchOffset = (INT8)BITS(op,0x00ff);

	/* "The PC Contains the address of the next instruction" */
	PC += 1;

	/* Jump */
	cpustate->ppc = PC;
	PC += branchOffset;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4; /* TODO: + jx oscillator clock cycles */
	return 0;
}

/* BRA : 0000 0001 0010 11RR : A-50 */
static size_t dsp56k_op_bra_2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BRKcc : 0000 0001 0001 cccc : A-52 */
static size_t dsp56k_op_brkcc(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	int shouldBreak = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldBreak)
	{
		/* TODO: I think this PC = LA thing is off-by-1, but it's working this way because its consistently so */
		cpustate->ppc = PC;
		PC = LA;

		SR = SSL;   /* TODO: A-83.  I believe only the Loop Flag and Forever Flag come back here. */
		SP--;

		LA = SSH;
		LC = SSL;
		SP--;

		cycles += 8;
		return 0;
	}
	else
	{
		cycles += 2;
		return 1;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
static size_t dsp56k_op_bscc(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	int shouldBranch = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldBranch)
	{
		/* The PC Contains the address of the next instruction */
		PC += 2;

		/* Push */
		SP++;
		SSH = PC;
		SSL = SR;

		/* Change */
		cpustate->ppc = PC;
		PC = PC + (INT16)op2;

		/* S L E U N Z V C */
		/* - - - - - - - - */
		cycles += 4;        /* TODO: + jx oscillator clock cycles */
		return 0;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;        /* TODO: + jx oscillator clock cycles */
	return 2;
}

/* BScc : 0000 0111 RR00 cccc : A-54 */
static size_t dsp56k_op_bscc_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
static size_t dsp56k_op_bsr(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* The PC Contains the address of the next instruction */
	PC += 2;

	/* Push */
	SP++;
	SSH = PC;
	SSL = SR;

	/* Change */
	cpustate->ppc = PC;
	PC = PC + (INT16)op2;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;    /* TODO: + jx oscillator clock cycles */
	return 0;
}

/* BSR : 0000 0001 0010 10RR : A-56 */
static size_t dsp56k_op_bsr_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* CHKAAU : 0000 0000 0000 0100 : A-58 */
static size_t dsp56k_op_chkaau(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
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
static size_t dsp56k_op_debug(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
static size_t dsp56k_op_debugcc(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* DIV : 0001 0101 0--0 F1DD : A-76 */
/* WARNING : DOCS SAY THERE IS A PARALLEL MOVE HERE !!! */
static size_t dsp56k_op_div(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* WARNING : THIS DOES NOT WORK.  IT DOESN'T EVEN TRY !!! */
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_DDF_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S, &D);

	/* S L E U N Z V C */
	/* - * - - - - ? ? */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significantst
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	/* C - Set if bit 39 of the result is cleared. Cleared otherwise. */
	cycles += 2;
	return 1;
}

/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
static size_t dsp56k_op_dmac(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 ss = 0;
	INT64 result = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQF_special_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);

	ss = BITS(op,0x0024);

	/* Fixed-point 2's complement multiplication requires a shift */
	if (ss == 0x00 || ss == 0x01)
	{
		/* Signed * Signed */
		INT32 s1 = ((INT32)(*((UINT16*)S1)));
		INT32 s2 = ((INT32)(*((UINT16*)S2)));
		result = ( s1 * s2 ) << 1;
	}
	else if (ss == 0x2)
	{
		/* Signed * Unsigned */
		/* WARNING : THERE IS A HUGE CHANCE THIS DOESN'T WORK RIGHT */
		INT32 s1 = ((INT32)(*((UINT16*)S1)));
		INT32 s2 = (UINT32)(*((UINT16*)S2));
		result = ( s1 * s2 ) << 1;
	}
	else if (ss == 0x3)
	{
		/* Unsigned * Unsigned */
		UINT32 s1 = (UINT32)(*((UINT16*)S1));
		UINT32 s2 = (UINT32)(*((UINT16*)S2));
		result = ( s1 * s2 ) << 1;
	}

	/* Shift right, then accumulate */
	(*((UINT64*)D)) =  (*((UINT64*)D)) >> 16;
	(*((UINT64*)D)) += result;

	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;
	return 1;
}

/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_op_do(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	return 0;
}

/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_op_do_1(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT8 retSize = 0;
	UINT8 iValue = BITS(op,0x00ff);

	/* Don't execute if the loop counter == 0 */
	if (iValue != 0x00)
	{
		/* First instruction cycle */
		SP++;                       /* TODO: Should i really inc here first? */
		SSH = LA;
		SSL = LC;
		LC = (UINT16)iValue;


		/* Second instruction cycle */
		SP++;                       /* TODO: See above */
		SSH = PC + 2;               /* Keep these stack entries in 'word-based-index' space */
		SSL = SR;
		LA = PC + 2 + op2;          /* TODO: The docs subtract 1 from here? */


		/* Third instruction cycle */
		LF_bit_set(cpustate, 1);

		/* Undocumented, but it must be true to nest Dos in DoForevers */
		FV_bit_set(cpustate, 0);


		/* S L E U N Z V C */
		/* - * - - - - - - */
		/* TODO : L */

		cycles += 6;    /* TODO: + mv oscillator cycles */
		retSize = 2;
	}
	else
	{
		/* Skip over the contents of the loop */
		cpustate->ppc = PC;
		PC = PC + 2 + op2;

		cycles += 10;   /* TODO: + mv oscillator cycles */
		retSize = 0;
	}

	return retSize;
}

/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56k_op_do_2(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT8 retSize = 0;
	UINT16 lValue = 0x0000;
	typed_pointer S = {NULL, DT_BYTE};
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &S);

	/* TODO: Does not properly shift-limit sources A&B - Fix per the docs. */
	/* TODO: There are other cases besides A&B this code won't work. */
	if      (S.addr == &A) lValue = *((UINT16*)(&A1));
	else if (S.addr == &B) lValue = *((UINT16*)(&B1));
	else                   lValue = *((UINT16*)S.addr);

	/* HACK */
	if (lValue >= 0xfff0)
	{
		logerror("Dsp56k : DO_2 operation changed %04x to 0000.\n", lValue);
		lValue = 0x0000;
	}

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
		SP++;                       /* TODO: Should i really inc here first? */
		SSH = LA;
		SSL = LC;
		LC = (UINT16)lValue;


		/* Second instruction cycle */
		SP++;                       /* TODO: See above */
		SSH = PC + 2;               /* Keep these stack entries in 'word-based-index' space */
		SSL = SR;
		LA = PC + 2 + op2;          /* TODO: The docs subtract 1 from here? */


		/* Third instruction cycle */
		LF_bit_set(cpustate, 1);


		/* S L E U N Z V C */
		/* - * - - - - - - */
		/* TODO : L */

		cycles += 6;    /* TODO: + mv oscillator cycles */
		retSize = 2;
	}
	else
	{
		/* Skip over the contents of the loop */
		cpustate->ppc = PC;
		PC = PC + 2 + op2;

		cycles += 10;   /* TODO: + mv oscillator cycles */
		retSize = 0;
	}

	return retSize;
}

/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
static size_t dsp56k_op_doforever(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* First instruction cycle */
	SP++;
	SSH = LA;
	SSL = LC;

	/* Second instruction cycle */
	SP++;
	SSH = PC + 2;
	SSL = SR;
	LA = PC + 2 + op2;

	/* Third instruction cycle */
	LF_bit_set(cpustate, 1);
	FV_bit_set(cpustate, 1);

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 6;
	return 2;
}

/* ENDDO : 0000 0000 0000 1001 : A-92 */
static size_t dsp56k_op_enddo(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* EXT : 0001 0101 0101 F010 : A-96 */
static size_t dsp56k_op_ext(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	return 0;
}

/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
static size_t dsp56k_op_illegal(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* IMAC : 0001 0101 1010 FQQQ : A-100 */
static size_t dsp56k_op_imac(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	INT64 opD = 0;
	INT64 result = 0;

	INT32 s1 = 0;
	INT32 s2 = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQQF_table(cpustate, BITS(op,0x0007), BITS(op,0x0008), &S1, &S2, &D);

	/* Cast both values as being signed */
	s1 = *((INT16*)S1);
	s2 = *((INT16*)S2);

	/* Integral multiply doesn't require the shift */
	result = (s1 * s2);

	/* Shift result 16 bits to the left before adding to destination */
	result = (result << 16) & 0xffff0000;

	/* Sign extend D into a temp variable */
	opD = (*((UINT64*)D));
	if (opD & U64(0x0000008000000000))
		opD |= U64(0xffffff0000000000);
	else
		opD &= U64(0x000000ffffffffff);

	/* Accumulate */
	opD += result;

	/* And out the bits that don't live in the register */
	opD &= U64(0x000000ffffffffff);

	(*((UINT64*)D)) = (UINT64)opD;

	/* S L E U N Z V C */
	/* - * ? ? * ? ? - */
	/* TODO: L */
	/* U,E - Will not be set correctly by this instruction*/
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffff0000)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	DSP56K_V_CLEAR();

	cycles += 2;
	return 1;
}

/* IMPY : 0001 0101 1000 FQQQ : A-102 */
static size_t dsp56k_op_impy(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * ? ? * ? ? - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	/* U,E - Will not be set correctly by this instruction*/
	/* V - Set to zero regardless of the overflow */
	return 0;
}

/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
static size_t dsp56k_op_jcc(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Jcc : 0000 0110 RR10 cccc : A-108 */
static size_t dsp56k_op_jcc_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
static size_t dsp56k_op_jmp(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	cpustate->ppc = PC;
	PC = op2;

	/* S L E U N Z V C */
	/* - - - - - - - - */

	cycles += 4;    /* TODO: + jx */
	return 0;
}

/* JMP : 0000 0001 0010 01RR : A-110 */
static size_t dsp56k_op_jmp_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	typed_pointer R = { NULL, DT_BYTE };
	decode_RR_table(cpustate, BITS(op,0x0003), &R);

	cpustate->ppc = PC;
	PC = *((UINT16*)R.addr);

	/* S L E U N Z V C */
	/* - - - - - - - - */

	cycles += 4;    /* TODO: + jx */
	return 0;
}

/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
static size_t dsp56k_op_jscc(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	int shouldJump = decode_cccc_table(cpustate, BITS(op,0x000f));

	if(shouldJump)
	{
		/* TODO: It says "signed" absolute offset.  Weird. */
		UINT16 branchOffset = op2;

		/* TODO: Verify, since it's not in the docs, but it must be true */
		PC += 2;

		SP++;
		SSH = PC;
		SSL = SR;

		cpustate->ppc = PC;
		PC = branchOffset;

		cycles += 4;    /* TODO: +jx oscillator clock cycles */
		return 0;
	}
	else
	{
		cycles += 4;    /* TODO: +jx oscillator clock cycles */
		return 2;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JScc : 0000 0110 RR00 cccc : A-112 */
static size_t dsp56k_op_jscc_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
static size_t dsp56k_op_jsr(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* TODO: It says "signed" absolute offset.  Weird. */
	UINT16 branchOffset = op2;

	/* TODO: Verify, since it's not in the docs, but it must be true */
	PC += 2;

	/* TODO: This is a hacky implementation of Long vs Fast Interrupts.  Do it right someday! */
	if (PC < ADDRESS(0x40))
	{
		/* Long interrupt gets the previous PC, not the current one */
		SP++;
		SSH = cpustate->ppc;
		SSL = SR;

		cpustate->ppc = cpustate->ppc;
		PC = branchOffset;
	}
	else
	{
		/* Normal operation */
		SP++;
		SSH = PC;
		SSL = SR;

		cpustate->ppc = PC;
		PC = branchOffset;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;        /* TODO: + jx oscillator cycles */
	return 0;
}

/* JSR : 0000 1010 AAAA AAAA : A-114 */
static size_t dsp56k_op_jsr_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* JSR : 0000 0001 0010 00RR : A-114 */
static size_t dsp56k_op_jsr_2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* LEA : 0000 0001 11TT MMRR : A-116 */
static size_t dsp56k_op_lea(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT16 ea = 0;
	UINT16 *rX = NULL;
	UINT16 *nX = NULL;
	typed_pointer D = {NULL, DT_BYTE};
	decode_TT_table(cpustate, BITS(op,0x0030), &D);

	/* TODO: change the execute_mm_functions to return values.  Maybe */
	/* Because this calculation isn't applied, do everything locally */
	/* RR table */
	switch(BITS(op,0x0003))
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: rX = &R3;  nX = &N3;  break;
	}

	/* MM table */
	switch(BITS(op,0x000c))
	{
		case 0x0: ea = *rX;       break;
		case 0x1: ea = *rX + 1;   break;
		case 0x2: ea = *rX - 1;   break;
		case 0x3: ea = *rX + *nX; break;
	}

	*((UINT16*)D.addr) = ea;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 1;
}

/* LEA : 0000 0001 10NN MMRR : A-116 */
static size_t dsp56k_op_lea_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
static size_t dsp56k_op_macsuuu(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 s = 0;
	INT64 result = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQF_special_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);

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
		/* WARNING : THERE IS A HUGE CHANCE THIS DOESN'T WORK RIGHT */
		INT32 s1 = ((INT32)(*((UINT16*)S1)));
		INT32 s2 = (UINT32)(*((UINT16*)S2));
		result = ( s1 * s2 ) << 1;
	}

	(*((UINT64*)D)) += result;

	/* And out the bits that don't live in the register */
	(*((UINT64*)D)) &= U64(0x000000ffffffffff);

	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;
	return 1;
}

/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
static size_t dsp56k_op_move_2(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
static size_t dsp56k_op_movec(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 W;
	typed_pointer R = { NULL, DT_BYTE };
	typed_pointer SD = { NULL, DT_BYTE };

	W = BITS(op,0x0400);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);
	decode_RR_table(cpustate, BITS(op,0x0003), &R);

	if (W)
	{
		/* Write D */
		UINT16 value = cpustate->data->read_word(ADDRESS(*((UINT16*)R.addr))) ;
		typed_pointer temp_src = { &value, DT_WORD };
		SetDestinationValue(temp_src, SD);
	}
	else
	{
		/* Read S */
		UINT16 dataMemOffset = *((UINT16*)R.addr);
		SetDataMemoryValue(cpustate, SD, ADDRESS(dataMemOffset));
	}

	execute_MM_table(cpustate, BITS(op,0x0003), BITS(op,0x000c));

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

	cycles += 2;    /* TODO: + mvc */
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
static size_t dsp56k_op_movec_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 W;
	UINT16 memOffset;
	typed_pointer SD = {NULL, DT_BYTE};

	W = BITS(op,0x0400);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);
	memOffset = execute_q_table(cpustate, BITS(op,0x0003), BITS(op,0x0008));

	if (W)
	{
		/* Write D */
		UINT16 tempData = cpustate->data->read_word(ADDRESS(memOffset));
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDestinationValue(temp_src, SD);
	}
	else
	{
		/* Read S */
		UINT16 tempData = *((UINT16*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDataMemoryValue(cpustate, temp_src, ADDRESS(memOffset));
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

	cycles += 2;        /* + mvc oscillator clock cycles */
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
static size_t dsp56k_op_movec_2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 W;
	UINT16 memOffset;
	typed_pointer SD = {NULL, DT_BYTE};
	typed_pointer XMemOffset = {NULL, DT_BYTE};

	W = BITS(op,0x0400);
	decode_Z_table(cpustate, BITS(op,0x0008), &XMemOffset);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);

	memOffset = *((UINT16*)XMemOffset.addr);

	if (W)
	{
		/* Write D */
		UINT16 tempData = cpustate->data->read_word(ADDRESS(memOffset));
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDestinationValue(temp_src, SD);
	}
	else
	{
		/* Read S */
		UINT16 tempData = *((UINT16*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDataMemoryValue(cpustate, temp_src, ADDRESS(memOffset));
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

	cycles += 2;        /* + mvc oscillator clock cycles */
	return 1;
}

/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
static size_t dsp56k_op_movec_3(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	UINT8 W;
	UINT8 t;
	typed_pointer SD = { NULL, DT_BYTE };

	W = BITS(op,0x0400);
	t = BITS(op,0x0008);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);

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
			UINT16 tempD = cpustate->data->read_word(ADDRESS(op2));
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
			SetDataMemoryValue(cpustate, SD, ADDRESS(op2));
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

	cycles += 2;    /* TODO: + mvc */
	return 2;
}

/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
static size_t dsp56k_op_movec_4(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &S);
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &D);

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
static size_t dsp56k_op_movec_5(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	INT8 xx;
	UINT8 W;
	UINT16 memOffset;
	typed_pointer SD = { NULL, DT_BYTE };

	xx = (INT8)(op & 0x00ff);
	W = BITS(op2,0x0400);
	decode_DDDDD_table(cpustate, BITS(op2,0x03e0), &SD);

	memOffset = R2 + (INT16)xx;

	if (W)
	{
		/* Write D */
		UINT16 tempData = cpustate->data->read_word(ADDRESS(memOffset));
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDestinationValue(temp_src, SD);
	}
	else
	{
		/* Read S */
		UINT16 tempData = *((UINT16*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDataMemoryValue(cpustate, temp_src, ADDRESS(memOffset));
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

	cycles += 2;    /* TODO: + mvc oscillator clock cycles */
	return 2;
}

/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
static size_t dsp56k_op_movei(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	typed_pointer immTP = {NULL, DT_BYTE};

	/* Typecasting to INT16 sign-extends the BBBBBBBB operand */
	UINT16 immediateSignExtended = (INT16)(op & 0x00ff);
	immTP.addr = &immediateSignExtended;
	immTP.data_type = DT_WORD;

	decode_DD_table(cpustate, BITS(op,0x0300), &D);

	SetDestinationValue(immTP, D);

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
static size_t dsp56k_op_movem(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 W;
	typed_pointer R = { NULL, DT_BYTE };
	typed_pointer SD = { NULL, DT_BYTE };

	W = BITS(op,0x0100);
	decode_RR_table(cpustate, BITS(op,0x00c0), &R);
	decode_HHH_table(cpustate, BITS(op,0x0007), &SD);

	if (W)
	{
		/* Read from Program Memory */
		typed_pointer data;
		UINT16 ldata = cpustate->program->read_word(ADDRESS(*((UINT16*)R.addr)));

		data.addr = &ldata;
		data.data_type = DT_WORD;
		SetDestinationValue(data, SD) ;
	}
	else
	{
		/* Write to Program Memory */
		SetProgramMemoryValue(cpustate, SD, ADDRESS(*((UINT16*)R.addr))) ;
	}

	execute_MM_table(cpustate, BITS(op,0x00c0), BITS(op,0x0018));

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;    /* TODO: + mvm oscillator clock cycles */
	return 1;
}

/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
static size_t dsp56k_op_movem_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
static size_t dsp56k_op_movem_2(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
static size_t dsp56k_op_movep(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT16 W;
	UINT16 pp;
	typed_pointer SD = {NULL, DT_BYTE};

	decode_HH_table(cpustate, BITS(op,0x00c0), &SD);
	/* TODO: Special cases for A & B */

	pp = op & 0x001f;
	pp = assemble_address_from_IO_short_address(cpustate, pp);

	W = BITS(op,0x0100);

	if (W)
	{
		UINT16 data = cpustate->data->read_word(ADDRESS(pp));

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		SetDestinationValue(tempTP, SD);
	}
	else
	{
		SetDataMemoryValue(cpustate, SD, ADDRESS(pp));
	}

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */

	cycles += 4;        /* TODO: + mvp oscillator cycles */
	return 1;
}

/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
static size_t dsp56k_op_movep_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* X:<Rx> and X:<pp> */
	UINT16 W;
	UINT16 pp;

	typed_pointer SD = {NULL, DT_BYTE};
	decode_RR_table(cpustate, BITS(op,0x00c0), &SD);

	pp = op & 0x001f;
	pp = assemble_address_from_IO_short_address(cpustate, pp);

	W = BITS(op,0x0100);

	/* A little different than most W if's - opposite read and write */
	if (W)
	{
		UINT16 data = cpustate->data->read_word(ADDRESS(*((UINT16*)SD.addr)));

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		SetDataMemoryValue(cpustate, tempTP, ADDRESS(pp));
	}
	else
	{
		/* TODO */
		fatalerror("dsp56k : move(p) NOTHING HERE (yet)\n") ;
	}

	/* Postincrement */
	execute_m_table(cpustate, BITS(op,0x00c0), BITS(op,0x0020));

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 4;        /* TODO: + mvp oscillator cycles */
	return 1;
}

/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
static size_t dsp56k_op_moves(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
static size_t dsp56k_op_mpysuuu(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	UINT8 s = 0;
	INT64 result = 0;

	void* D = NULL;
	void* S1 = NULL;
	void* S2 = NULL;

	decode_QQF_special_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);

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
		/* WARNING : THERE IS A HUGE CHANCE THIS DOESN'T WORK RIGHT */
		INT32 s1 = ((INT32)(*((UINT16*)S1)));
		INT32 s2 = (UINT32)(*((UINT16*)S2));
		result = ( s1 * s2 ) << 1;
	}

	(*((UINT64*)D)) = result;

	/* And out the bits that don't live in the register */
	(*((UINT64*)D)) &= U64(0x000000ffffffffff);

	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((UINT64*)D) & U64(0x0000008000000000))       DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT64*)D) & U64(0x000000ffffffffff)) == 0) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();

	cycles += 2;
	return 1;
}

/* NEGC : 0001 0101 0110 F000 : A-168 */
static size_t dsp56k_op_negc(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * * */
	return 0;
}

/* NOP : 0000 0000 0000 0000 : A-170 */
static size_t dsp56k_op_nop(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 1;
}

/* NORM : 0001 0101 0010 F0RR : A-172 */
static size_t dsp56k_op_norm(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * ? - */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significantst
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	return 0;
}

/* ORI : 0001 1EE1 iiii iiii : A-178 */
static size_t dsp56k_op_ori(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Set if the corresponding bit in the immediate data is set and if the operand is the
	   CCR. Not affected otherwise. */
	return 0;
}

/* REP : 0000 0000 111- --RR : A-180 */
static size_t dsp56k_op_rep(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	return 0;
}

/* REP : 0000 1111 iiii iiii : A-180 */
static size_t dsp56k_op_rep_1(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* TODO: This is non-interruptable, probably have to turn off interrupts here */
	UINT16 iVal = op & 0x00ff;

	if (iVal != 0)
	{
		TEMP = LC;
		LC = iVal;

		cpustate->repFlag = 1;
		cpustate->repAddr = PC + ADDRESS(1);

		cycles += 4;        /* TODO: + mv oscillator clock cycles */
	}
	else
	{
		cycles += 6;        /* TODO: + mv oscillator clock cycles */
	}


	/* S L E U N Z V C */
	/* - * - - - - - - */
	/* TODO: L */
	return 1;
}

/* REP : 0000 0100 001D DDDD : A-180 */
static size_t dsp56k_op_rep_2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* TODO: This is non-interruptable, probably have to turn off interrupts here */
	UINT16 repValue;
	typed_pointer D = {NULL, DT_BYTE};
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &D);

	/* TODO: handle special A&B source cases */
	if (D.addr == &A || D.addr == &B)
		logerror("DSP56k ERROR : Rep with A or B instruction not implemented yet!\n");

	repValue = *((UINT16*)D.addr);

	if (repValue != 0)
	{
		TEMP = LC;
		LC = repValue;

		cpustate->repFlag = 1;
		cpustate->repAddr = PC + ADDRESS(1);

		cycles += 4;        /* TODO: + mv oscillator clock cycles */
	}
	else
	{
		cycles += 6;        /* TODO: + mv oscillator clock cycles */
	}

	/* S L E U N Z V C */
	/* - * - - - - - - */
	/* TODO: L */
	return 1;
}

/* REPcc : 0000 0001 0101 cccc : A-184 */
static size_t dsp56k_op_repcc(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* RESET : 0000 0000 0000 1000 : A-186 */
static size_t dsp56k_op_reset(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* RTI : 0000 0000 0000 0111 : A-194 */
static size_t dsp56k_op_rti(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* WARNING : THERE SHOULD BE A MORE GENERAL HANDLING OF STACK ERRORS. */
	if (SP == 0)
	{
		dsp56k_add_pending_interrupt(cpustate, "Stack Error");
		return 0;
	}

	cpustate->ppc = PC;
	PC = SSH;

	SR = SSL;
	SP = SP - 1;

	/* S L E U N Z V C */
	/* ? ? ? ? ? ? ? ? */
	/* All ? bits - Set according to value pulled from the stack. */
	cycles += 4;        /* TODO: + rx oscillator clock cycles */
	return 0;
}

/* RTS : 0000 0000 0000 0110 : A-196 */
static size_t dsp56k_op_rts(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* Pop */
	cpustate->ppc = PC;
	PC = SSH;

	/* SR = SSL; The status register is not affected. */

	SP--;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;    /* TODO: + rx oscillator clock cycles */
	return 0;
}

/* STOP : 0000 0000 0000 1010 : A-200 */
static size_t dsp56k_op_stop(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* SWAP : 0001 0101 0111 F001 : A-206 */
static size_t dsp56k_op_swap(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* SWI : 0000 0000 0000 0101 : A-208 */
static size_t dsp56k_op_swi(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
static size_t dsp56k_op_tcc(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	int shouldTransfer = decode_cccc_table(cpustate, BITS(op,0x03c0));

	if (shouldTransfer)
	{
		typed_pointer S = {NULL, DT_BYTE};
		typed_pointer D = {NULL, DT_BYTE};
		typed_pointer S2 = {&R0, DT_WORD};
		typed_pointer D2 = {NULL, DT_BYTE};

		decode_h0hF_table(cpustate, BITS(op,0x0007),BITS(op,0x0008), &S, &D);
		SetDestinationValue(S, D);

		/* TODO: What's up with that A,A* thing in the docs?  Can you only ignore the R0->RX transfer if you do an A,A? */
		decode_RR_table(cpustate, BITS(op,0x0030), &D2); /* TT is the same as RR */
		SetDestinationValue(S2, D2);
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* TFR(2) : 0001 0101 0000 F00J : A-214 */
static size_t dsp56k_op_tfr2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_JF_table(cpustate, BITS(op,0x0001), BITS(op,0x0008), &S, &D);

	SetDestinationValue(S, D);

	/* S L E U N Z V C */
	/* - * - - - - - - */
	/* TODO: L */
	cycles += 2;
	return 1;
}

/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
static size_t dsp56k_op_tfr3(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	return 0;
}

/* TST(2) : 0001 0101 0001 -1DD : A-220 */
static size_t dsp56k_op_tst2(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	typed_pointer D = {NULL, DT_BYTE};
	decode_DD_table(cpustate, BITS(op,0x0003), &D);

	/* S L E U N Z V C */
	/* - * * * * * 0 0 */
	/* (L,E,U should be set to 0) */
	DSP56K_L_CLEAR();
	DSP56K_E_CLEAR();
	/* U_CLEAR(); */ /* TODO: Conflicting opinions?  "Set if unnormalized."  Documentation is weird (A&B?) */
	if ((*((UINT16*)D.addr)) &  0x8000) DSP56K_N_SET(); else DSP56K_N_CLEAR();
	if ((*((UINT16*)D.addr)) == 0x0000) DSP56K_Z_SET(); else DSP56K_Z_CLEAR();
	/* DSP56K_V_CLEAR(); */ /* Unaffected */
	DSP56K_C_CLEAR();

	cycles += 2;
	return 1;
}

/* WAIT : 0000 0000 0000 1011 : A-222 */
static size_t dsp56k_op_wait(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 0;
}

/* ZERO : 0001 0101 0101 F000 : A-224 */
static size_t dsp56k_op_zero(dsp56k_core* cpustate, const UINT16 op, UINT8* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	return 0;
}



/***************************************************************************
    Table decoding
***************************************************************************/
static UINT16 decode_BBB_bitmask(dsp56k_core* cpustate, UINT16 BBB, UINT16 *iVal)
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

static int decode_cccc_table(dsp56k_core* cpustate, UINT16 cccc)
{
	int retVal = 0;

	/* Not fully tested */
	switch (cccc)
	{
		/* Arranged according to mnemonic table - not decoding table */
		case 0x0: if( C() == 0)                         retVal = 1;  break;  /* cc(hs) */
		case 0x8: if( C() == 1)                         retVal = 1;  break;  /* cs(lo) */
		case 0x5: if( E() == 0)                         retVal = 1;  break;  /* ec */
		case 0xa: if( Z() == 1)                         retVal = 1;  break;  /* eq */
		case 0xd: if( E() == 1)                         retVal = 1;  break;  /* es */
		case 0x1: if((N() ^  V()) == 0)                 retVal = 1;  break;  /* ge */
		case 0x7: if((Z() | (N() ^ V())) == 0)          retVal = 1;  break;  /* gt */
		case 0x6: if( L() == 0)                         retVal = 1;  break;  /* lc */
		case 0xf: if((Z() | (N() ^ V())) == 1)          retVal = 1;  break;  /* le */
		case 0xe: if( L() == 1)                         retVal = 1;  break;  /* ls */
		case 0x9: if((N() ^  V()) == 1)                 retVal = 1;  break;  /* lt */
		case 0xb: if( N() == 1)                         retVal = 1;  break;  /* mi */
		case 0x2: if( Z() == 0)                         retVal = 1;  break;  /* ne */
		case 0xc: if((Z() | ((!U()) & (!E()))) == 1)    retVal = 1;  break;  /* nr */
		case 0x3: if( N() == 0)                         retVal = 1;  break;  /* pl */
		case 0x4: if((Z() | ((!U()) & (!E()))) == 0)    retVal = 1;  break;  /* nn */
	}

	return retVal;
}

static void decode_DDDDD_table(dsp56k_core* cpustate, UINT16 DDDDD, typed_pointer* ret)
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
		/*no 0x1b  */
		case 0x1c: ret->addr = &N0;  ret->data_type = DT_WORD;       break;
		case 0x1d: ret->addr = &N1;  ret->data_type = DT_WORD;       break;
		case 0x1e: ret->addr = &N2;  ret->data_type = DT_WORD;       break;
		case 0x1f: ret->addr = &N3;  ret->data_type = DT_WORD;       break;
	}
}

static void decode_DD_table(dsp56k_core* cpustate, UINT16 DD, typed_pointer* ret)
{
	switch(DD)
	{
		case 0x00: ret->addr = &X0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &Y0;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &X1;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &Y1;  ret->data_type = DT_WORD;  break;
	}
}

static void decode_DDF_table(dsp56k_core* cpustate, UINT16 DD, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	UINT16 switchVal = (DD << 1) | F;

	switch (switchVal)
	{
		case 0x0: src_ret->addr = &X0;  src_ret->data_type = DT_WORD; dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x1: src_ret->addr = &X0;  src_ret->data_type = DT_WORD; dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x2: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD; dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x3: src_ret->addr = &Y0;  src_ret->data_type = DT_WORD; dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x4: src_ret->addr = &X1;  src_ret->data_type = DT_WORD; dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x5: src_ret->addr = &X1;  src_ret->data_type = DT_WORD; dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x6: src_ret->addr = &Y1;  src_ret->data_type = DT_WORD; dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x7: src_ret->addr = &Y1;  src_ret->data_type = DT_WORD; dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_F_table(dsp56k_core* cpustate, UINT16 F, typed_pointer* ret)
{
	switch(F)
	{
		case 0x0: ret->addr = &A;  ret->data_type = DT_LONG_WORD;  break;
		case 0x1: ret->addr = &B;  ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_h0hF_table(dsp56k_core* cpustate, UINT16 h0h, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
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

static void decode_HH_table(dsp56k_core* cpustate, UINT16 HH, typed_pointer* ret)
{
	switch(HH)
	{
		case 0x0: ret->addr = &X0;  ret->data_type = DT_WORD;       break;
		case 0x1: ret->addr = &Y0;  ret->data_type = DT_WORD;       break;
		case 0x2: ret->addr = &A;   ret->data_type = DT_LONG_WORD;  break;
		case 0x3: ret->addr = &B;   ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_HHH_table(dsp56k_core* cpustate, UINT16 HHH, typed_pointer* ret)
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

static void decode_IIII_table(dsp56k_core* cpustate, UINT16 IIII, typed_pointer* src_ret, typed_pointer* dst_ret, void *working)
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

static void decode_JJJF_table(dsp56k_core* cpustate, UINT16 JJJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
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

static void decode_JJF_table(dsp56k_core* cpustate, UINT16 JJ, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
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

static void decode_JF_table(dsp56k_core* cpustate, UINT16 J, UINT16 F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	UINT16 switchVal = (J << 1) | F ;

	switch (switchVal)
	{
		case 0x0: src_ret->addr = &A;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &X;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
		case 0x1: src_ret->addr = &B;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &X;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
		case 0x2: src_ret->addr = &A;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &Y;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
		case 0x3: src_ret->addr = &B;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &Y;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
	}
}

static void decode_KKK_table(dsp56k_core* cpustate, UINT16 KKK, typed_pointer* dst_ret1, typed_pointer* dst_ret2, void* working)
{
	void *opposite = 0x00 ;

	if (working == &A) opposite = &B ;
	else               opposite = &A ;

	switch(KKK)
	{
		case 0x0: dst_ret1->addr = opposite;  dst_ret1->data_type = DT_LONG_WORD;  dst_ret2->addr = &X0;  dst_ret2->data_type = DT_WORD;  break;
		case 0x1: dst_ret1->addr = &Y0;       dst_ret1->data_type = DT_WORD;       dst_ret2->addr = &X0;  dst_ret2->data_type = DT_WORD;  break;
		case 0x2: dst_ret1->addr = &X1;       dst_ret1->data_type = DT_WORD;       dst_ret2->addr = &X0;  dst_ret2->data_type = DT_WORD;  break;
		case 0x3: dst_ret1->addr = &Y1;       dst_ret1->data_type = DT_WORD;       dst_ret2->addr = &X0;  dst_ret2->data_type = DT_WORD;  break;
		case 0x4: dst_ret1->addr = &X0;       dst_ret1->data_type = DT_WORD;       dst_ret2->addr = &X1;  dst_ret2->data_type = DT_WORD;  break;
		case 0x5: dst_ret1->addr = &Y0;       dst_ret1->data_type = DT_WORD;       dst_ret2->addr = &X1;  dst_ret2->data_type = DT_WORD;  break;
		case 0x6: dst_ret1->addr = opposite;  dst_ret1->data_type = DT_LONG_WORD;  dst_ret2->addr = &Y0;  dst_ret2->data_type = DT_WORD;  break;
		case 0x7: dst_ret1->addr = &Y1;       dst_ret1->data_type = DT_WORD;       dst_ret2->addr = &X1;  dst_ret2->data_type = DT_WORD;  break;
	}
}

static void decode_QQF_table(dsp56k_core* cpustate, UINT16 QQ, UINT16 F, void **S1, void **S2, void **D)
{
	UINT16 switchVal = (QQ << 1) | F ;

	switch(switchVal)
	{
		case 0x0: *S1 = &X0;  *S2 = &Y0;  *D = &A;  break;
		case 0x1: *S1 = &X0;  *S2 = &Y0;  *D = &B;  break;
		case 0x2: *S1 = &X0;  *S2 = &Y1;  *D = &A;  break;
		case 0x3: *S1 = &X0;  *S2 = &Y1;  *D = &B;  break;
		case 0x4: *S1 = &X1;  *S2 = &Y0;  *D = &A;  break;
		case 0x5: *S1 = &X1;  *S2 = &Y0;  *D = &B;  break;
		case 0x6: *S1 = &X1;  *S2 = &Y1;  *D = &A;  break;
		case 0x7: *S1 = &X1;  *S2 = &Y1;  *D = &B;  break;
	}
}

static void decode_QQF_special_table(dsp56k_core* cpustate, UINT16 QQ, UINT16 F, void **S1, void **S2, void **D)
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

static void decode_QQQF_table(dsp56k_core* cpustate, UINT16 QQQ, UINT16 F, void **S1, void **S2, void **D)
{
	UINT16 switchVal = (QQQ << 1) | F;

	switch(switchVal)
	{
		case 0x0: *S1 = &X0;  *S2 = &X0;  *D = &A;  break;
		case 0x1: *S1 = &X0;  *S2 = &X0;  *D = &B;  break;
		case 0x2: *S1 = &X1;  *S2 = &X0;  *D = &A;  break;
		case 0x3: *S1 = &X1;  *S2 = &X0;  *D = &B;  break;
		case 0x4: *S1 = &A1;  *S2 = &Y0;  *D = &A;  break;
		case 0x5: *S1 = &A1;  *S2 = &Y0;  *D = &B;  break;
		case 0x6: *S1 = &B1;  *S2 = &X0;  *D = &A;  break;
		case 0x7: *S1 = &B1;  *S2 = &X0;  *D = &B;  break;
		case 0x8: *S1 = &Y0;  *S2 = &X0;  *D = &A;  break;
		case 0x9: *S1 = &Y0;  *S2 = &X0;  *D = &B;  break;
		case 0xa: *S1 = &Y1;  *S2 = &X0;  *D = &A;  break;
		case 0xb: *S1 = &Y1;  *S2 = &X0;  *D = &B;  break;
		case 0xc: *S1 = &Y0;  *S2 = &X1;  *D = &A;  break;
		case 0xd: *S1 = &Y0;  *S2 = &X1;  *D = &B;  break;
		case 0xe: *S1 = &Y1;  *S2 = &X1;  *D = &A;  break;
		case 0xf: *S1 = &Y1;  *S2 = &X1;  *D = &B;  break;
	}
}

static void decode_RR_table(dsp56k_core* cpustate, UINT16 RR, typed_pointer* ret)
{
	switch(RR)
	{
		case 0x00: ret->addr = &R0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &R1;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &R2;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &R3;  ret->data_type = DT_WORD;  break;
	}
}

static void decode_TT_table(dsp56k_core* cpustate, UINT16 TT, typed_pointer* ret)
{
	switch(TT)
	{
		case 0x00: ret->addr = &R0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &R1;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &R2;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &R3;  ret->data_type = DT_WORD;  break;
	}
}


static void decode_uuuuF_table(dsp56k_core* cpustate, UINT16 uuuu, UINT16 F, UINT8 add_sub_other, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	UINT16 switchVal = (uuuu << 1) | F;

	/* Unknown uuuuFs have been seen in the wild */
	add_sub_other = OP_OTHER;
	src_ret->addr = NULL; src_ret->data_type = DT_BYTE;
	dst_ret->addr = NULL; dst_ret->data_type = DT_BYTE;

	switch(switchVal)
	{
		case 0x00: add_sub_other = OP_ADD;
					src_ret->addr = &X0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x08: add_sub_other = OP_SUB;
					src_ret->addr = &X0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x01: add_sub_other = OP_ADD;
					src_ret->addr = &X0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x09: add_sub_other = OP_SUB;
					src_ret->addr = &X0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x02: add_sub_other = OP_ADD;
					src_ret->addr = &Y0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x0a: add_sub_other = OP_SUB;
					src_ret->addr = &Y0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x03: add_sub_other = OP_ADD;
					src_ret->addr = &Y0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x0b: add_sub_other = OP_SUB;
					src_ret->addr = &Y0; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x04: add_sub_other = OP_ADD;
					src_ret->addr = &X1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x0c: add_sub_other = OP_SUB;
					src_ret->addr = &X1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x05: add_sub_other = OP_ADD;
					src_ret->addr = &X1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x0d: add_sub_other = OP_SUB;
					src_ret->addr = &X1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x06: add_sub_other = OP_ADD;
					src_ret->addr = &Y1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x0e: add_sub_other = OP_SUB;
					src_ret->addr = &Y1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x07: add_sub_other = OP_ADD;
					src_ret->addr = &Y1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x0f: add_sub_other = OP_SUB;
					src_ret->addr = &Y1; src_ret->data_type = DT_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x18: add_sub_other = OP_ADD;
					src_ret->addr = &B;  src_ret->data_type = DT_LONG_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x1a: add_sub_other = OP_SUB;
					src_ret->addr = &B;  src_ret->data_type = DT_LONG_WORD;
					dst_ret->addr = &A;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x19: add_sub_other = OP_ADD;
					src_ret->addr = &A;  src_ret->data_type = DT_LONG_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
		case 0x1b: add_sub_other = OP_SUB;
					src_ret->addr = &A;  src_ret->data_type = DT_LONG_WORD;
					dst_ret->addr = &B;  dst_ret->data_type = DT_LONG_WORD; break;
	}
}

static void decode_Z_table(dsp56k_core* cpustate, UINT16 Z, typed_pointer* ret)
{
	switch(Z)
	{
		/* Fixed as per the Family Manual addendum */
		case 0x01: ret->addr = &A1;  ret->data_type = DT_WORD;  break;
		case 0x00: ret->addr = &B1;  ret->data_type = DT_WORD;  break;
	}
}

static void execute_m_table(dsp56k_core* cpustate, int x, UINT16 m)
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

static void execute_mm_table(dsp56k_core* cpustate, UINT16 rnum, UINT16 mm)
{
	UINT16 *rX = NULL;
	UINT16 *nX = NULL;

	switch(rnum)
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: fatalerror("Dsp56k: Error. execute_mm_table specified R3 as its first source!\n");  break;
	}

	switch(mm)
	{
		case 0x0: (*rX)++;                  R3++;           break;
		case 0x1: (*rX)++;                  R3 = R3 + N3;   break;
		case 0x2: (*rX) = (*rX) + (*nX);    R3++;           break;
		case 0x3: (*rX) = (*rX) + (*nX);    R3 = R3 + N3;   break;
	}
}

static void execute_MM_table(dsp56k_core* cpustate, UINT16 rnum, UINT16 MM)
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

/* Returns R value */
static UINT16 execute_q_table(dsp56k_core* cpustate, int RR, UINT16 q)
{
	UINT16 *rX = 0x0000;
	UINT16 *nX = 0x0000;

	switch(RR)
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: rX = &R3;  nX = &N3;  break;
	}

	switch(q)
	{
		case 0x0: /* No permanent changes */ ; return (*rX)+(*nX);
		case 0x1: (*rX)--;                     return (*rX);    /* This one is special - it's a *PRE-decrement*! */
	}

	/* Should not get here */
	fatalerror("dsp56k: execute_q_table did something impossible!\n");
	return 0;
}

static void execute_z_table(dsp56k_core* cpustate, int RR, UINT16 z)
{
	UINT16 *rX = 0x00;
	UINT16 *nX = 0x00;

	switch(RR)
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: rX = &R3;  nX = &N3;  break;
	}

	switch(z)
	{
		case 0x0: (*rX)--;               break;
		case 0x1: (*rX) = (*rX) + (*nX); break;
	}
}

static UINT16 assemble_address_from_Pppppp_table(dsp56k_core* cpustate, UINT16 P, UINT16 ppppp)
{
	UINT16 destAddr = 0x00 ;

	switch (P)
	{
		case 0x0: destAddr = ppppp;  break;     /* TODO:  Does this really only address up to 0x32? */
		case 0x1: destAddr = assemble_address_from_IO_short_address(cpustate, ppppp);  break;
	}

	return destAddr ;
}

static UINT16 assemble_address_from_IO_short_address(dsp56k_core* cpustate, UINT16 pp)
{
	UINT16 fullAddy = 0xffe0;
	fullAddy |= pp;
	return fullAddy;
}

static UINT16 assemble_address_from_6bit_signed_relative_short_address(dsp56k_core* cpustate, UINT16 srs)
{
	UINT16 fullAddy = srs ;
	if (fullAddy & 0x0020)
		fullAddy |= 0xffc0 ;

	return fullAddy ;
}

static void dsp56k_process_loop(dsp56k_core* cpustate)
{
	/* TODO: This might not work for dos nested in doForevers */
	if (LF_bit(cpustate) && FV_bit(cpustate))
	{
		/* Do Forever*/
		if (PC == LA)
		{
			LC--;

			cpustate->ppc = PC;
			PC = SSH;
		}
	}
	else if (LF_bit(cpustate))
	{
		/* Do */
		if (PC == LA)
		{
			if (LC == 1)
			{
				/* End of loop processing */
				SR = SSL;   /* TODO: A-83.  I believe only the Loop Flag comes back here.  And maybe the do forever bit too. */
				SP--;

				LA = SSH;
				LC = SSL;
				SP--;
			}
			else
			{
				LC--;
				PC = SSH;
			}
		}
	}
}

static void dsp56k_process_rep(dsp56k_core* cpustate, size_t repSize)
{
	if (cpustate->repFlag)
	{
		if (PC == cpustate->repAddr)
		{
			if (LC == 1)
			{
				/* End of rep processing */
				LC = TEMP;
				cpustate->repFlag = 0;
				cpustate->repAddr = 0x0000;
			}
			else
			{
				LC--;
				PC -= repSize;      /* A little strange - rewind by the size of the rep'd op */
			}
		}
	}
}


/***************************************************************************
    Parallel Memory Ops
***************************************************************************/
/* Register to Register Data Move : 0100 IIII .... .... : A-132 */
static void execute_register_to_register_data_move(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value)
{
	typed_pointer S = {NULL, DT_BYTE};
	typed_pointer D = {NULL, DT_BYTE};

	decode_IIII_table(cpustate, BITS(op,0x0f00), &S, &D, d_register->addr);

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

/* Address Register Update : 0011 0zRR .... .... : A-135 */
static void execute_address_register_update(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value)
{
	execute_z_table(cpustate, BITS(op,0x0300), BITS(op,0x0400));
}

/* X Memory Data Move : 1mRR HHHW .... .... : A-137 */
static void execute_x_memory_data_move(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register, UINT64* prev_accum_value)
{
	UINT16 W;
	typed_pointer R = {NULL, DT_BYTE};
	typed_pointer SD = {NULL, DT_BYTE};

	W = BITS(op,0x0100);
	decode_HHH_table(cpustate, BITS(op,0x0e00), &SD);
	decode_RR_table(cpustate, BITS(op,0x3000),&R);

	if (W)
	{
		/* From X:<ea> to SD */
		UINT16 data = cpustate->data->read_word(ADDRESS(*((UINT16*)R.addr)));

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

			SetDataMemoryValue(cpustate, tempTP, ADDRESS(*((UINT16*)R.addr))) ;
		}
		else
		{
			SetDataMemoryValue(cpustate, SD, ADDRESS(*((UINT16*)R.addr))) ;
		}
	}

	execute_m_table(cpustate, BITS(op,0x3000), BITS(op,0x4000));
}

/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
/* NOTE: previous accumulator value is not needed since ^F1 is always the opposite accumulator */
static void execute_x_memory_data_move2(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register)
{
	UINT16 W;
	UINT16* mem_offset = NULL;
	typed_pointer SD = {NULL, DT_BYTE};

	W = BITS(op,0x0100);
	decode_HHH_table(cpustate, BITS(op,0x0e000), &SD);

	if (d_register->addr == &A)
		mem_offset = &B1;
	else
		mem_offset = &A1;

	if (W)
	{
		/* Write D */
		UINT16 value = cpustate->data->read_word(ADDRESS(*mem_offset));
		typed_pointer tempV = {&value, DT_WORD};
		SetDestinationValue(tempV, SD);
	}
	else
	{
		/* Read S */
		SetDataMemoryValue(cpustate, SD, ADDRESS(*mem_offset));
	}
}

/* X Memory Data Move With Short Displacement : 0000 0101 BBBB BBBB ---- HHHW .... .... : A-139 */
static void execute_x_memory_data_move_with_short_displacement(dsp56k_core* cpustate, const UINT16 op, const UINT16 op2)
{
	INT8 xx;
	UINT8 W;
	UINT16 memOffset;
	typed_pointer SD = { NULL, DT_BYTE };

	xx = (INT8)(op & 0x00ff);
	W = BITS(op2,0x0100);
	decode_HHH_table(cpustate, BITS(op2,0x0e00), &SD);

	memOffset = R2 + (INT16)xx;

	if (W)
	{
		/* Write D */
		UINT16 tempData = cpustate->data->read_word(ADDRESS(memOffset));
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDestinationValue(temp_src, SD);
	}
	else
	{
		/* Read S */
		UINT16 tempData = *((UINT16*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		SetDataMemoryValue(cpustate, temp_src, ADDRESS(memOffset));
	}
}

/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
static void execute_dual_x_memory_data_read(dsp56k_core* cpustate, const UINT16 op, typed_pointer* d_register)
{
	typed_pointer tempV;
	UINT16 srcVal1 = 0x0000;
	UINT16 srcVal2 = 0x0000;
	typed_pointer R = {NULL, DT_BYTE};
	typed_pointer D1 = {NULL, DT_BYTE};
	typed_pointer D2 = {NULL, DT_BYTE};

	decode_RR_table(cpustate, BITS(op,0x0060), &R);
	decode_KKK_table(cpustate, BITS(op,0x0700), &D1, &D2, d_register->addr);

	/* Can't do an R3 for S1 */
	if (R.addr == &R3)
		fatalerror("Dsp56k: Error. Dual x memory data read specified R3 as its first source!\n");

	/* The note on A-142 is very interesting.
	   You can effectively access external memory in the last 64 bytes of X data memory! */
	if (*((UINT16*)D2.addr) >= 0xffc0)
		fatalerror("Dsp56k: Unimplemented access to external X Data Memory >= 0xffc0 in Dual X Memory Data Read.\n");

	/* First memmove */
	srcVal1 = cpustate->data->read_word(ADDRESS(*((UINT16*)R.addr)));
	tempV.addr = &srcVal1;
	tempV.data_type = DT_WORD;
	SetDestinationValue(tempV, D1);

	/* Second memmove */
	srcVal2 = cpustate->data->read_word(ADDRESS(R3));
	tempV.addr = &srcVal2;
	tempV.data_type = DT_WORD;
	SetDestinationValue(tempV, D2);

	/* Touch up the R regs after all the moves */
	execute_mm_table(cpustate, BITS(op,0x0060), BITS(op,0x1800));
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

	/* Shift everything right, eliminating 'whitespace' */
	for (i = 0; i < 16; i++)
	{
		if (mask & (0x1<<i))        /* If mask bit is non-zero */
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
				case DT_LONG_WORD:   *((UINT16*)dest.addr) = (*((UINT64*)source.addr)) & U64(0x000000000000ffff); break;    /* TODO: Shift limiter action! A-147 */
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

				case DT_WORD:        destinationValue = (*((UINT16*)source.addr)) << 16;
										if (destinationValue & U64(0x0000000080000000))
											destinationValue |= U64(0x000000ff00000000);
										*((UINT64*)dest.addr) = (UINT64)destinationValue; break;    /* Forget not, yon shift register */

				case DT_DOUBLE_WORD: *((UINT64*)dest.addr) = (*((UINT32*)source.addr)) & 0xffffffff; break;
				case DT_LONG_WORD:   *((UINT64*)dest.addr) = (*((UINT64*)source.addr)) & U64(0x000000ffffffffff); break;
			}
		break ;
	}
}

/* TODO: Wait-state timings! */
static void SetDataMemoryValue(dsp56k_core* cpustate, typed_pointer source, UINT32 destinationAddr)
{
	switch(source.data_type)
	{
		case DT_BYTE:        cpustate->data->write_word(destinationAddr, (UINT16)( (*((UINT8*) source.addr) & 0xff)               ) ) ; break ;
		case DT_WORD:        cpustate->data->write_word(destinationAddr, (UINT16)( (*((UINT16*)source.addr) & 0xffff)             ) ) ; break ;
		case DT_DOUBLE_WORD: cpustate->data->write_word(destinationAddr, (UINT16)( (*((UINT32*)source.addr) & 0x0000ffff)         ) ) ; break ;

		/* !!! Is this universal ??? */
		/* !!! Forget not, yon shift-limiter !!! */
		case DT_LONG_WORD:   cpustate->data->write_word(destinationAddr, (UINT16)( ((*((UINT64*)source.addr)) & U64(0x00000000ffff0000)) >> 16) ) ; break ;
	}
}

/* TODO: Wait-state timings! */
static void SetProgramMemoryValue(dsp56k_core* cpustate, typed_pointer source, UINT32 destinationAddr)
{
	switch(source.data_type)
	{
		case DT_BYTE:        cpustate->program->write_word(destinationAddr, (UINT16)( (*((UINT8*) source.addr) & 0xff)               ) ) ; break ;
		case DT_WORD:        cpustate->program->write_word(destinationAddr, (UINT16)( (*((UINT16*)source.addr) & 0xffff)             ) ) ; break ;
		case DT_DOUBLE_WORD: cpustate->program->write_word(destinationAddr, (UINT16)( (*((UINT32*)source.addr) & 0x0000ffff)         ) ) ; break ;

		/* !!! Is this universal ??? */
		/* !!! Forget not, yon shift-limiter !!! */
		case DT_LONG_WORD:   cpustate->program->write_word(destinationAddr, (UINT16)( ((*((UINT64*)source.addr)) & U64(0x00000000ffff0000)) >> 16) ) ; break ;
	}
}

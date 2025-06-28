// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56ops.hxx
    Core implementation for the portable Motorola/Freescale DSP56156 emulator.
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
    - Restore only the proper SR/MR bits upon loop termination!
*/

#define LOG_OUTPUT_FUNC cpustate->device->logerror
#define LOG_UNIMPLEMENTED   (1U << 1)
#define LOG_CHECKS          (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

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

#define BITS(CUR,MASK) (dsp56156_op_mask(CUR,MASK))

static constexpr bool dsp56156_add_ovf_table[8] = { false, true, false, false, false, false, true, false };
static constexpr bool dsp56156_sub_ovf_table[8] = { false, false, false, true, true, false, false, false };

/*********************/
/* Opcode prototypes */
/*********************/
static size_t dsp56156_op_addsub_2 (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_mac_1    (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_macr_1   (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_move_1   (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_mpy_1    (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_mpyr_1   (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_tfr_2    (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles);
static size_t dsp56156_op_mpy_2    (dsp56156_core* cpustate, const uint16_t op_byte, uint8_t* cycles);
static size_t dsp56156_op_mac_2    (dsp56156_core* cpustate, const uint16_t op_byte, uint8_t* cycles);
static size_t dsp56156_op_clr      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_add      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_move     (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_tfr      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_rnd      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_tst      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_inc      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_inc24    (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_or       (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_asr      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_asl      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_lsr      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_lsl      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_eor      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_subl     (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_sub      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_clr24    (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_sbc      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_cmp      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_neg      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_not      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_dec      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_dec24    (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_and      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_abs      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_ror      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_rol      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_cmpm     (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_mpy      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_mpyr     (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_mac      (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_macr     (dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles);
static size_t dsp56156_op_adc      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_andi     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_asl4     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_asr4     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_asr16    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_bfop     (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bfop_1   (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bfop_2   (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bcc      (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bcc_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_bcc_2    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_bra      (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bra_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_bra_2    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_brkcc    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_bscc     (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bscc_1   (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_bsr      (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_bsr_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_chkaau   (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_debug    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_debugcc  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_div      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_dmac     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_do       (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_do_1     (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_do_2     (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_doforever(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_enddo    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_ext      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_illegal  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_imac     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_impy     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_jcc      (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_jcc_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_jmp      (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_jmp_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_jscc     (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_jscc_1   (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_jsr      (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_jsr_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_jsr_2    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_lea      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_lea_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_macsuuu  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_move_2   (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_movec    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movec_1  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movec_2  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movec_3  (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_movec_4  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movec_5  (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_movei    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movem    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movem_1  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movem_2  (dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles);
static size_t dsp56156_op_movep    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_movep_1  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_moves    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_mpysuuu  (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_negc     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_nop      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_norm     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_ori      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_rep      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_rep_1    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_rep_2    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_repcc    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_reset    (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_rti      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_rts      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_stop     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_swap     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_swi      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_tcc      (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_tfr2     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_tfr3     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_tst2     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_wait     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);
static size_t dsp56156_op_zero     (dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles);

static bool dsp56156_value_is_unnormalized(dsp56156_core* cpustate, const uint64_t value);
static bool dsp56156_value_is_extended(dsp56156_core* cpustate, const uint64_t value);

static void execute_register_to_register_data_move(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register, uint64_t* prev_accum_value);
static void execute_address_register_update(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register, uint64_t* prev_accum_value);
static void execute_x_memory_data_move (dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register, uint64_t* prev_accum_value);
static void execute_x_memory_data_move2(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register);
static void execute_dual_x_memory_data_read(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register);
static void execute_x_memory_data_move_with_short_displacement(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2);

static uint16_t decode_BBB_bitmask(dsp56156_core* cpustate, uint16_t BBB, uint16_t *iVal);
static int      decode_cccc_table(dsp56156_core* cpustate, uint16_t cccc);
static void     decode_DDDDD_table(dsp56156_core* cpustate, uint16_t DDDDD, typed_pointer* ret);
static void     decode_DD_table(dsp56156_core* cpustate, uint16_t DD, typed_pointer* ret);
static void     decode_DDF_table(dsp56156_core* cpustate, uint16_t DD, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_F_table(dsp56156_core* cpustate, uint16_t F, typed_pointer* ret);
static void     decode_h0hF_table(dsp56156_core* cpustate, uint16_t h0h, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_HH_table(dsp56156_core* cpustate, uint16_t HH, typed_pointer* ret);
static void     decode_HHH_table(dsp56156_core* cpustate, uint16_t HHH, typed_pointer* ret);
static void     decode_IIII_table(dsp56156_core* cpustate, uint16_t IIII, typed_pointer* src_ret, typed_pointer* dst_ret, void* working);
static void     decode_JJJF_table(dsp56156_core* cpustate, uint16_t JJJ, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_JJF_table(dsp56156_core* cpustate, uint16_t JJ, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_JF_table(dsp56156_core* cpustate, uint16_t JJ, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_KKK_table(dsp56156_core* cpustate, uint16_t KKK, typed_pointer* dst_ret1, typed_pointer* dst_ret2, void* working);
static void     decode_QQF_table(dsp56156_core* cpustate, uint16_t QQ, uint16_t F, void **S1, void **S2, void **D);
static void     decode_QQF_special_table(dsp56156_core* cpustate, uint16_t QQ, uint16_t F, void **S1, void **S2, void **D);
static void     decode_QQQF_table(dsp56156_core* cpustate, uint16_t QQQ, uint16_t F, void **S1, void **S2, void **D);
static void     decode_RR_table(dsp56156_core* cpustate, uint16_t RR, typed_pointer* ret);
static void     decode_TT_table(dsp56156_core* cpustate, uint16_t TT, typed_pointer* ret);
static void     decode_uuuuF_table(dsp56156_core* cpustate, uint16_t uuuu, uint16_t F, uint8_t add_sub_other, typed_pointer* src_ret, typed_pointer* dst_ret);
static void     decode_Z_table(dsp56156_core* cpustate, uint16_t Z, typed_pointer* ret);

static void     execute_m_table(dsp56156_core* cpustate, int x, uint16_t m);
static void     execute_mm_table(dsp56156_core* cpustate, uint16_t rnum, uint16_t mm);
static void     execute_MM_table(dsp56156_core* cpustate, uint16_t rnum, uint16_t MM);
static uint16_t execute_q_table(dsp56156_core* cpustate, int RR, uint16_t q);
static void     execute_z_table(dsp56156_core* cpustate, int RR, uint16_t z);

static uint16_t assemble_address_from_Pppppp_table(dsp56156_core* cpustate, uint16_t P, uint16_t ppppp);
static uint16_t assemble_address_from_IO_short_address(dsp56156_core* cpustate, uint16_t pp);

static void dsp56156_process_loop(dsp56156_core* cpustate);
static void dsp56156_process_rep(dsp56156_core* cpustate, size_t repSize);



/********************/
/* Helper Functions */
/********************/
static uint16_t dsp56156_op_mask(uint16_t op, uint16_t mask);

static uint64_t dsp56156_limit_long_to_long(dsp56156_core* cpustate, uint64_t val);

/* These arguments are written source->destination to fall in line with the processor's paradigm. */
static void dsp56156_set_destination_value(dsp56156_core* cpustate, typed_pointer source, typed_pointer dest);

static void dsp56156_set_data_memory_value(dsp56156_core* cpustate, typed_pointer source, uint32_t destinationAddr);
static void dsp56156_set_program_memory_value(dsp56156_core* cpustate, typed_pointer source, uint32_t destinationAddr);



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static size_t dsp56156_get_op_size(uint16_t op, uint16_t op2)
{
	/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
	if ((op & 0xe000) == 0x6000)
	{
		/* ADD : 011m mKKK 0rru Fuuu : A-22 */
		/* SUB : 011m mKKK 0rru Fuuu : A-202 */
		/* Note: 0x0094 check allows command to drop through to MOVE and TFR */
		if (((op & 0xe080) == 0x6000) && ((op & 0x0094) != 0x0010))
		{
			return 1;
		}
		/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
		else if ((op & 0xe094) == 0x6084)
		{
			return 1;
		}
		/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
		else if ((op & 0xe094) == 0x6094)
		{
			return 0; // Not yet implemented
		}
		/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
		else if ((op & 0xe094) == 0x6010)
		{
			return 0; // Not yet implemented
		}
		/* MOVE : 011m mKKK 0rr1 x111 : A-128, Note: The displayed bitfield appears to be errata. */
		else if ((op & 0xe097) == 0x6017)
		{
			return 0; // Not yet implemented
		}
		/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
		else if ((op & 0xe094) == 0x6080)
		{
			return 1;
		}
		/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
		else if ((op & 0xe094) == 0x6090)
		{
			return 0; // Not yet implemented
		}
	}
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
	else if ((op & 0xfe00) == 0x1600)
	{
		/* MPY : 0001 0110 RRDD FQQQ : A-160 */
		if ((op & 0xff00) == 0x1600)
		{
			return 0; // Not yet implemented
		}
		/* MAC : 0001 0111 RRDD FQQQ : A-122 */
		else if ((op & 0xff00) == 0x1700)
		{
			return 0; // Not yet implemented
		}
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

		/* Note: it's important that NPDM comes before RtRDM here */
		/* No Parallel Data Move : 0100 1010 .... .... : A-131 */
		if ((op & 0xff00) == 0x4a00)
		{
			parallelType = kNoParallelDataMove;
		}
		/* Register to Register Data Move : 0100 IIII .... .... : A-133 */
		else if ((op & 0xf000) == 0x4000)
		{
			parallelType = kRegisterToRegister;
		}
		/* Address Register Update : 0011 0zRR .... .... : A-135 */
		else if ((op & 0xf800) == 0x3000)
		{
			parallelType = kAddressRegister;
		}
		/* X Memory Data Move : 1mRR HHHW .... .... : A-137 */
		else if ((op & 0x8000) == 0x8000)
		{
			parallelType = kXMemoryDataMove;
		}
		/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
		else if ((op & 0xf000) == 0x5000)
		{
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
				return 2;
			}
		}

		if (parallelType != -1)
		{
			/* Note: There is much overlap between opcodes down here */
			/*       To this end, certain ops must come before others in the list */

			/* CLR : .... .... 0000 F001 : A-60 */
			if ((op & 0x00f7) == 0x0001)
			{
				return 1;
			}
			/* ADD : .... .... 0000 FJJJ : A-22 */
			else if ((op & 0x00f0) == 0x0000)
			{
				return 1;
			}
			/* MOVE : .... .... 0001 0001 : A-128 */
			else if ((op & 0x00ff) == 0x0011)
			{
				return 1;
			}
			/* TFR : .... .... 0001 FJJJ : A-212 */
			else if ((op & 0x00f0) == 0x0010)
			{
				return 1;
			}
			/* RND : .... .... 0010 F000 : A-188 */
			else if ((op & 0x00f7) == 0x0020)
			{
				return 1;
			}
			/* TST : .... .... 0010 F001 : A-218 */
			else if ((op & 0x00f7) == 0x0021)
			{
				return 1;
			}
			/* INC : .... .... 0010 F010 : A-104 */
			else if ((op & 0x00f7) == 0x0022)
			{
				return 1;
			}
			/* INC24 : .... .... 0010 F011 : A-106 */
			else if ((op & 0x00f7) == 0x0023)
			{
				return 1;
			}
			/* OR : .... .... 0010 F1JJ : A-176 */
			else if ((op & 0x00f4) == 0x0024)
			{
				return 1;
			}
			/* ASR : .... .... 0011 F000 : A-32 */
			else if ((op & 0x00f7) == 0x0030)
			{
				return 1;
			}
			/* ASL : .... .... 0011 F001 : A-28 */
			else if ((op & 0x00f7) == 0x0031)
			{
				return 1;
			}
			/* LSR : .... .... 0011 F010 : A-120 */
			else if ((op & 0x00f7) == 0x0032)
			{
				return 1;
			}
			/* LSL : .... .... 0011 F011 : A-118 */
			else if ((op & 0x00f7) == 0x0033)
			{
				return 1;
			}
			/* EOR : .... .... 0011 F1JJ : A-94 */
			else if ((op & 0x00f4) == 0x0034)
			{
				return 1;
			}
			/* SUBL : .... .... 0100 F001 : A-204 */
			else if ((op & 0x00f7) == 0x0041)
			{
				return 1;
			}
			/* SUB : .... .... 0100 FJJJ : A-202 */
			else if ((op & 0x00f0) == 0x0040)
			{
				return 1;
			}
			/* CLR24 : .... .... 0101 F001 : A-62 */
			else if ((op & 0x00f7) == 0x0051)
			{
				return 1;
			}
			/* SBC : .... .... 0101 F01J : A-198 */
			else if ((op & 0x00f6) == 0x0052)
			{
				return 1;
			}
			/* CMP : .... .... 0101 FJJJ : A-64 */
			else if ((op & 0x00f0) == 0x0050)
			{
				return 1;
			}
			/* NEG : .... .... 0110 F000 : A-166 */
			else if ((op & 0x00f7) == 0x0060)
			{
				return 1;
			}
			/* NOT : .... .... 0110 F001 : A-174 */
			else if ((op & 0x00f7) == 0x0061)
			{
				return 1;
			}
			/* DEC : .... .... 0110 F010 : A-72 */
			else if ((op & 0x00f7) == 0x0062)
			{
				return 1;
			}
			/* DEC24 : .... .... 0110 F011 : A-74 */
			else if ((op & 0x00f7) == 0x0063)
			{
				return 1;
			}
			/* AND : .... .... 0110 F1JJ : A-24 */
			else if ((op & 0x00f4) == 0x0064)
			{
				return 1;
			}
			/* ABS : .... .... 0111 F001 : A-18 */
			if ((op & 0x00f7) == 0x0071)
			{
				return 1;
			}
			/* ROR : .... .... 0111 F010 : A-192 */
			else if ((op & 0x00f7) == 0x0072)
			{
				return 1;
			}
			/* ROL : .... .... 0111 F011 : A-190 */
			else if ((op & 0x00f7) == 0x0073)
			{
				return 1;
			}
			/* CMPM : .... .... 0111 FJJJ : A-66 */
			else if ((op & 0x00f0) == 0x0070)
			{
				return 1;
			}
			/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
			else if ((op & 0x00b0) == 0x0080)
			{
				return 1;
			}
			/* MPYR : .... .... 1k01 FQQQ : A-162 */
			else if ((op & 0x00b0) == 0x0090)
			{
				return 1;
			}
			/* MAC : .... .... 1k10 FQQQ : A-122 */
			else if ((op & 0x00b0) == 0x00a0)
			{
				return 1;
			}
			/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
			else if ((op & 0x00b0) == 0x00b0)
			{
				return 1;
			}
		}
	}

	/******************************/
	/* Remaining non-parallel ops */
	/******************************/

	/* ADC : 0001 0101 0000 F01J : A-20 */
	if ((op & 0xfff6) == 0x1502)
	{
		return 1;
	}
	/* ANDI : 0001 1EE0 iiii iiii : A-26 */
	/* (MoveP sneaks in here if you don't check 0x0600) */
	else if (((op & 0xf900) == 0x1800) & ((op & 0x0600) != 0x0000))
	{
		return 1;
	}
	/* ASL4 : 0001 0101 0011 F001 : A-30 */
	else if ((op & 0xfff7) == 0x1531)
	{
		return 1;
	}
	/* ASR4 : 0001 0101 0011 F000 : A-34 */
	else if ((op & 0xfff7) == 0x1530)
	{
		return 1;
	}
	/* ASR16 : 0001 0101 0111 F000 : A-36 */
	else if ((op & 0xfff7) == 0x1570)
	{
		return 1;
	}
	/* BFCHG : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1200))
	{
		return 2;
	}
	/* BFCHG : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1200))
	{
		return 2;
	}
	/* BFCHG : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1200))
	{
		return 2;
	}
	/* BFCLR : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x0400))
	{
		return 2;
	}
	/* BFCLR : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x0400))
	{
		return 2;
	}
	/* BFCLR : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x0400))
	{
		return 2;
	}
	/* BFSET : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1800))
	{
		return 2;
	}
	/* BFSET : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1800))
	{
		return 2;
	}
	/* BFSET : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1800))
	{
		return 2;
	}
	/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x1000))
	{
		return 2;
	}
	/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x1000))
	{
		return 2;
	}
	/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x1000))
	{
		return 2;
	}
	/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x0000))
	{
		return 2;
	}
	/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x0000))
	{
		return 2;
	}
	/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x0000))
	{
		return 2;
	}
	/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
	else if (((op & 0xff30) == 0x0730) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* Bcc : 0010 11cc ccee eeee : A-48 */
	else if ((op & 0xfc00) == 0x2c00)
	{
		return 1;
	}
	/* Bcc : 0000 0111 RR10 cccc : A-48 */
	else if ((op & 0xff30) == 0x0720)
	{
		return 1;
	}
	/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
	else if (((op & 0xfffc) == 0x013c) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* BRA : 0000 1011 aaaa aaaa : A-50 */
	else if ((op & 0xff00) == 0x0b00)
	{
		return 1;
	}
	/* BRA : 0000 0001 0010 11RR : A-50 */
	else if ((op & 0xfffc) == 0x012c)
	{
		return 1;
	}
	/* BRKc : 0000 0001 0001 cccc : A-52 */
	else if ((op & 0xfff0) == 0x0110)
	{
		return 1;
	}
	/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
	else if (((op & 0xff30) == 0x0710) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* BScc : 0000 0111 RR00 cccc : A-54 */
	else if ((op & 0xff30) == 0x0700)
	{
		return 1;
	}
	/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
	else if (((op & 0xfffc) == 0x0138) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* BSR : 0000 0001 0010 10RR : A-56 */
	else if ((op & 0xfffc) == 0x0128)
	{
		return 1;
	}
	/* CHKAAU : 0000 0000 0000 0100 : A-58 */
	else if ((op & 0xffff) == 0x0004)
	{
		return 1;
	}
	/* DEBUG : 0000 0000 0000 0001 : A-68 */
	else if ((op & 0xffff) == 0x0001)
	{
		return 1;
	}
	/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
	else if ((op & 0xfff0) == 0x0050)
	{
		return 1;
	}
	/* DIV : 0001 0101 0--0 F1DD : A-76 */
	else if ((op & 0xff94) == 0x1504)
	{
		return 1;
	}
	/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
	else if ((op & 0xffd0) == 0x1590)
	{
		return 1;
	}
	/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x00c0) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xff00) == 0x0e00) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x0400) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
	else if (((op & 0xffff) == 0x0002) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* ENDDO : 0000 0000 0000 1001 : A-92 */
	else if ((op & 0xffff) == 0x0009)
	{
		return 1;
	}
	/* EXT : 0001 0101 0101 F010 : A-96 */
	else if ((op & 0xfff7) == 0x1552)
	{
		return 1;
	}
	/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
	else if ((op & 0xffff) == 0x000f)
	{
		return 1;
	}
	/* IMAC : 0001 0101 1010 FQQQ : A-100 */
	else if ((op & 0xfff0) == 0x15a0)
	{
		return 1;
	}
	/* IMPY : 0001 0101 1000 FQQQ : A-102 */
	else if ((op & 0xfff0) == 0x1580)
	{
		return 1;
	}
	/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
	else if (((op & 0xff30) == 0x0630) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* Jcc : 0000 0110 RR10 cccc : A-108 */
	else if ((op & 0xff30) == 0x0620 )
	{
		return 1;
	}
	/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
	else if (((op & 0xfffc) == 0x0134) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* JMP : 0000 0001 0010 01RR : A-110 */
	else if ((op & 0xfffc) == 0x0124)
	{
		return 1;
	}
	/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
	else if (((op & 0xff30) == 0x0610) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* JScc : 0000 0110 RR00 cccc : A-112 */
	else if ((op & 0xff30) == 0x0600)
	{
		return 1;
	}
	/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
	else if (((op & 0xfffc) == 0x0130) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* JSR : 0000 1010 AAAA AAAA : A-114 */
	else if ((op & 0xff00) == 0x0a00)
	{
		return 1;
	}
	/* JSR : 0000 0001 0010 00RR : A-114 */
	else if ((op & 0xfffc) == 0x0120)
	{
		return 1;
	}
	/* LEA : 0000 0001 11TT MMRR : A-116 */
	else if ((op & 0xffc0) == 0x01c0)
	{
		return 1;
	}
	/* LEA : 0000 0001 10NN MMRR : A-116 */
	else if ((op & 0xffc0) == 0x0180)
	{
		return 1;
	}
	/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
	else if ((op & 0xfff0) == 0x15e0)
	{
		return 1;
	}
	/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0x00ff) == 0x0011))
	{
		return 2;
	}
	/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
	else if ((op & 0xf810) == 0x3800)
	{
		return 1;
	}
	/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
	else if ((op & 0xf814) == 0x3810)
	{
		return 1;
	}
	/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
	else if ((op & 0xf816) == 0x3816)
	{
		return 1;
	}
	/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
	else if (((op & 0xf816) == 0x3814) && ((op2 & 0x0000) == 0x0000))
	{
		return 2;
	}
	/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
	else if ((op & 0xfc00) == 0x2800)
	{
		return 1;
	}
	/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xf810) == 0x3800))
	{
		return 2;
	}
	/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
	else if ((op & 0xfc00) == 0x2000)
	{
		return 1;
	}
	/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
	else if ((op & 0xfe20) == 0x0200)
	{
		return 1;
	}
	/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
	else if ((op & 0xfe30) == 0x0230)
	{
		return 1;
	}
	/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xfe20) == 0x0200))
	{
		return 2;
	}
	/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
	else if ((op & 0xfe20) == 0x1820)
	{
		return 1;
	}
	/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
	else if ((op & 0xfe00) == 0x0c00)
	{
		return 1;
	}
	/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
	else if ((op & 0xfe20) == 0x1800)
	{
		return 1;
	}
	/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
	else if ((op & 0xfff0) == 0x15c0)
	{
		return 1;
	}
	/* NEGC : 0001 0101 0110 F000 : A-168 */
	else if ((op & 0xfff7) == 0x1560)
	{
		return 1;
	}
	/* NOP : 0000 0000 0000 0000 : A-170 */
	else if ((op & 0xffff) == 0x0000)
	{
		return 1;
	}
	/* NORM : 0001 0101 0010 F0RR : A-172 */
	else if ((op & 0xfff4) == 0x1520)
	{
		return 1;
	}
	/* ORI : 0001 1EE1 iiii iiii : A-178 */
	else if ((op & 0xf900) == 0x1900)
	{
		return 1;
	}
	/* REP : 0000 0000 111- --RR : A-180 */
	else if ((op & 0xffe0) == 0x00e0)
	{
		return 1;
	}
	/* REP : 0000 1111 iiii iiii : A-180 */
	else if ((op & 0xff00) == 0x0f00)
	{
		return 1;
	}
	/* REP : 0000 0100 001D DDDD : A-180 */
	else if ((op & 0xffe0) == 0x0420)
	{
		return 1;
	}
	/* REPcc : 0000 0001 0101 cccc : A-184 */
	else if ((op & 0xfff0) == 0x0150)
	{
		return 1;
	}
	/* RESET : 0000 0000 0000 1000 : A-186 */
	else if ((op & 0xffff) == 0x0008)
	{
		return 1;
	}
	/* RTI : 0000 0000 0000 0111 : A-194 */
	else if ((op & 0xffff) == 0x0007)
	{
		return 1;
	}
	/* RTS : 0000 0000 0000 0110 : A-196 */
	else if ((op & 0xffff) == 0x0006)
	{
		return 1;
	}
	/* STOP : 0000 0000 0000 1010 : A-200 */
	else if ((op & 0xffff) == 0x000a)
	{
		return 1;
	}
	/* SWAP : 0001 0101 0111 F001 : A-206 */
	else if ((op & 0xfff7) == 0x1571)
	{
		return 1;
	}
	/* SWI : 0000 0000 0000 0101 : A-208 */
	else if ((op & 0xffff) == 0x0005)
	{
		return 1;
	}
	/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
	else if ((op & 0xfc02) == 0x1000)
	{
		return 1;
	}
	/* TFR(2) : 0001 0101 0000 F00J : A-214 */
	else if ((op & 0xfff6) == 0x1500)
	{
		return 1;
	}
	/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
	else if ((op & 0xfc00) == 0x2400)
	{
		return 1;
	}
	/* TST(2) : 0001 0101 0001 -1DD : A-220 */
	else if ((op & 0xfff4) == 0x1514)
	{
		return 1;
	}
	/* WAIT : 0000 0000 0000 1011 : A-222 */
	else if ((op & 0xffff) == 0x000b)
	{
		return 1;
	}
	/* ZERO : 0001 0101 0101 F000 : A-224 */
	else if ((op & 0xfff7) == 0x1550)
	{
		return 1;
	}

	return 1;
}

static void execute_one(dsp56156_core* cpustate)
{
	size_t size = 0x1337;
	uint8_t cycle_count = 0;

	/* For MAME */
	cpustate->ppc = PC;
	if (cpustate->device->machine().debug_flags & DEBUG_FLAG_CALL_HOOK) // FIXME: if this was a member, the helper would work
		cpustate->device->debug()->instruction_hook(PC);

	cpustate->op = ROPCODE(PC);
	/* The words we're going to be working with */
	uint16_t op = ROPCODE(PC);
	uint16_t op2 = ROPCODE(PC + 1);

	/* DECODE */
	/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
	if ((op & 0xe000) == 0x6000)
	{
		typed_pointer d_register = {nullptr, DT_BYTE};

		/* Quote: (MOVE, MAC(R), MPY(R), ADD, SUB, TFR) */
		uint16_t op_byte = op & 0x00ff;

		/* ADD : 011m mKKK 0rru Fuuu : A-22 */
		/* SUB : 011m mKKK 0rru Fuuu : A-202 */
		/* Note: 0x0094 check allows command to drop through to MOVE and TFR */
		if (((op & 0xe080) == 0x6000) && ((op & 0x0094) != 0x0010))
		{
			size = dsp56156_op_addsub_2(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
		else if ((op & 0xe094) == 0x6084)
		{
			size = dsp56156_op_mac_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
		else if ((op & 0xe094) == 0x6094)
		{
			size = dsp56156_op_macr_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
		else if ((op & 0xe094) == 0x6010)
		{
			size = dsp56156_op_tfr_2(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MOVE : 011m mKKK 0rr1 x111 : A-128, Note: The displayed bitfield appears to be errata. */
		else if ((op & 0xe097) == 0x6017)
		{
			size = dsp56156_op_move_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
		else if ((op & 0xe094) == 0x6080)
		{
			size = dsp56156_op_mpy_1(cpustate, op_byte, &d_register, &cycle_count);
		}
		/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
		else if ((op & 0xe094) == 0x6090)
		{
			size = dsp56156_op_mpyr_1(cpustate, op_byte, &d_register, &cycle_count);
		}

		/* Now evaluate the parallel data move */
		execute_dual_x_memory_data_read(cpustate, op, &d_register);
	}
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
	else if ((op & 0xfe00) == 0x1600)
	{
		/* Quote: (MPY or MAC) */
		uint16_t op_byte = op & 0x00ff;

		/* MPY : 0001 0110 RRDD FQQQ : A-160 */
		if ((op & 0xff00) == 0x1600)
		{
			size = dsp56156_op_mpy_2(cpustate, op_byte, &cycle_count);
		}
		/* MAC : 0001 0111 RRDD FQQQ : A-122 */
		else if ((op & 0xff00) == 0x1700)
		{
			size = dsp56156_op_mac_2(cpustate, op_byte, &cycle_count);
		}

		/* Now evaluate the parallel data move */
		/* TODO // decode_x_memory_data_write_and_register_data_move(op, parallel_move_str, parallel_move_str2); */
		LOGMASKED(LOG_UNIMPLEMENTED, "DSP56156: Unemulated Dual X Memory Data And Register Data Move @ 0x%x\n", PC);
		cpustate->device->machine().debug_break();
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
		uint16_t op_byte = 0x0000;
		typed_pointer d_register = {nullptr, DT_BYTE};
		uint64_t prev_accum_value = 0x0000000000000000ULL;

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
				size = dsp56156_op_clr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ADD : .... .... 0000 FJJJ : A-22 */
			else if ((op_byte & 0x00f0) == 0x0000)
			{
				size = dsp56156_op_add(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* MOVE : .... .... 0001 0001 : A-128 */
			else if ((op_byte & 0x00ff) == 0x0011)
			{
				size = dsp56156_op_move(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* TFR : .... .... 0001 FJJJ : A-212 */
			else if ((op_byte & 0x00f0) == 0x0010)
			{
				size = dsp56156_op_tfr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* RND : .... .... 0010 F000 : A-188 */
			else if ((op_byte & 0x00f7) == 0x0020)
			{
				size = dsp56156_op_rnd(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* TST : .... .... 0010 F001 : A-218 */
			else if ((op_byte & 0x00f7) == 0x0021)
			{
				size = dsp56156_op_tst(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* INC : .... .... 0010 F010 : A-104 */
			else if ((op_byte & 0x00f7) == 0x0022)
			{
				size = dsp56156_op_inc(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* INC24 : .... .... 0010 F011 : A-106 */
			else if ((op_byte & 0x00f7) == 0x0023)
			{
				size = dsp56156_op_inc24(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* OR : .... .... 0010 F1JJ : A-176 */
			else if ((op_byte & 0x00f4) == 0x0024)
			{
				size = dsp56156_op_or(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* ASR : .... .... 0011 F000 : A-32 */
			else if ((op_byte & 0x00f7) == 0x0030)
			{
				size = dsp56156_op_asr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ASL : .... .... 0011 F001 : A-28 */
			else if ((op_byte & 0x00f7) == 0x0031)
			{
				size = dsp56156_op_asl(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* LSR : .... .... 0011 F010 : A-120 */
			else if ((op_byte & 0x00f7) == 0x0032)
			{
				size = dsp56156_op_lsr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* LSL : .... .... 0011 F011 : A-118 */
			else if ((op_byte & 0x00f7) == 0x0033)
			{
				size = dsp56156_op_lsl(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* EOR : .... .... 0011 F1JJ : A-94 */
			else if ((op_byte & 0x00f4) == 0x0034)
			{
				size = dsp56156_op_eor(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* SUBL : .... .... 0100 F001 : A-204 */
			else if ((op_byte & 0x00f7) == 0x0041)
			{
				size = dsp56156_op_subl(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* SUB : .... .... 0100 FJJJ : A-202 */
			else if ((op_byte & 0x00f0) == 0x0040)
			{
				size = dsp56156_op_sub(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* CLR24 : .... .... 0101 F001 : A-62 */
			else if ((op_byte & 0x00f7) == 0x0051)
			{
				size = dsp56156_op_clr24(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* SBC : .... .... 0101 F01J : A-198 */
			else if ((op_byte & 0x00f6) == 0x0052)
			{
				size = dsp56156_op_sbc(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* CMP : .... .... 0101 FJJJ : A-64 */
			else if ((op_byte & 0x00f0) == 0x0050)
			{
				size = dsp56156_op_cmp(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* NEG : .... .... 0110 F000 : A-166 */
			else if ((op_byte & 0x00f7) == 0x0060)
			{
				size = dsp56156_op_neg(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* NOT : .... .... 0110 F001 : A-174 */
			else if ((op_byte & 0x00f7) == 0x0061)
			{
				size = dsp56156_op_not(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* DEC : .... .... 0110 F010 : A-72 */
			else if ((op_byte & 0x00f7) == 0x0062)
			{
				size = dsp56156_op_dec(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* DEC24 : .... .... 0110 F011 : A-74 */
			else if ((op_byte & 0x00f7) == 0x0063)
			{
				size = dsp56156_op_dec24(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* AND : .... .... 0110 F1JJ : A-24 */
			else if ((op_byte & 0x00f4) == 0x0064)
			{
				size = dsp56156_op_and(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* ABS : .... .... 0111 F001 : A-18 */
			if ((op_byte & 0x00f7) == 0x0071)
			{
				size = dsp56156_op_abs(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ROR : .... .... 0111 F010 : A-192 */
			else if ((op_byte & 0x00f7) == 0x0072)
			{
				size = dsp56156_op_ror(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* ROL : .... .... 0111 F011 : A-190 */
			else if ((op_byte & 0x00f7) == 0x0073)
			{
				size = dsp56156_op_rol(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* CMPM : .... .... 0111 FJJJ : A-66 */
			else if ((op_byte & 0x00f0) == 0x0070)
			{
				size = dsp56156_op_cmpm(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}


			/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
			else if ((op_byte & 0x00b0) == 0x0080)
			{
				size = dsp56156_op_mpy(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MPYR : .... .... 1k01 FQQQ : A-162 */
			else if ((op_byte & 0x00b0) == 0x0090)
			{
				size = dsp56156_op_mpyr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MAC : .... .... 1k10 FQQQ : A-122 */
			else if ((op_byte & 0x00b0) == 0x00a0)
			{
				size = dsp56156_op_mac(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
			}
			/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
			else if ((op_byte & 0x00b0) == 0x00b0)
			{
				size = dsp56156_op_macr(cpustate, op_byte, &d_register, &prev_accum_value, &cycle_count);
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
		PC += (uint16_t)size;

		dsp56156_process_loop(cpustate);
		dsp56156_process_rep(cpustate, size);

		cpustate->icount -= 4;  /* Temporarily hard-coded at 4 clocks per opcode */ /* cycle_count */
		return;
	}


	/******************************/
	/* Remaining non-parallel ops */
	/******************************/

	/* ADC : 0001 0101 0000 F01J : A-20 */
	if ((op & 0xfff6) == 0x1502)
	{
		size = dsp56156_op_adc(cpustate, op, &cycle_count);
	}
	/* ANDI : 0001 1EE0 iiii iiii : A-26 */
	/* (MoveP sneaks in here if you don't check 0x0600) */
	else if (((op & 0xf900) == 0x1800) & ((op & 0x0600) != 0x0000))
	{
		size = dsp56156_op_andi(cpustate, op, &cycle_count);
	}
	/* ASL4 : 0001 0101 0011 F001 : A-30 */
	else if ((op & 0xfff7) == 0x1531)
	{
		size = dsp56156_op_asl4(cpustate, op, &cycle_count);
	}
	/* ASR4 : 0001 0101 0011 F000 : A-34 */
	else if ((op & 0xfff7) == 0x1530)
	{
		size = dsp56156_op_asr4(cpustate, op, &cycle_count);
	}
	/* ASR16 : 0001 0101 0111 F000 : A-36 */
	else if ((op & 0xfff7) == 0x1570)
	{
		size = dsp56156_op_asr16(cpustate, op, &cycle_count);
	}
	/* BFCHG : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56156_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFCHG : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56156_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFCHG : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1200))
	{
		size = dsp56156_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56156_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56156_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFCLR : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x0400))
	{
		size = dsp56156_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffc0) == 0x14c0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56156_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x14a0) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56156_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFSET : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
	else if (((op & 0xffe0) == 0x1480) && ((op2 & 0x1f00) == 0x1800))
	{
		size = dsp56156_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56156_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56156_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x1000))
	{
		size = dsp56156_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffc0) == 0x1440) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56156_op_bfop(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1420) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56156_op_bfop_1(cpustate, op, op2, &cycle_count);
	}
	/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
	else if (((op & 0xffe0) == 0x1400) && ((op2 & 0x1f00) == 0x0000))
	{
		size = dsp56156_op_bfop_2(cpustate, op, op2, &cycle_count);
	}
	/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
	else if (((op & 0xff30) == 0x0730) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_bcc(cpustate, op, op2, &cycle_count);
	}
	/* Bcc : 0010 11cc ccee eeee : A-48 */
	else if ((op & 0xfc00) == 0x2c00)
	{
		size = dsp56156_op_bcc_1(cpustate, op, &cycle_count);
	}
	/* Bcc : 0000 0111 RR10 cccc : A-48 */
	else if ((op & 0xff30) == 0x0720)
	{
		size = dsp56156_op_bcc_2(cpustate, op, &cycle_count);
	}
	/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
	else if (((op & 0xfffc) == 0x013c) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_bra(cpustate, op, op2, &cycle_count);
	}
	/* BRA : 0000 1011 aaaa aaaa : A-50 */
	else if ((op & 0xff00) == 0x0b00)
	{
		size = dsp56156_op_bra_1(cpustate, op, &cycle_count);
	}
	/* BRA : 0000 0001 0010 11RR : A-50 */
	else if ((op & 0xfffc) == 0x012c)
	{
		size = dsp56156_op_bra_2(cpustate, op, &cycle_count);
	}
	/* BRKc : 0000 0001 0001 cccc : A-52 */
	else if ((op & 0xfff0) == 0x0110)
	{
		size = dsp56156_op_brkcc(cpustate, op, &cycle_count);
	}
	/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
	else if (((op & 0xff30) == 0x0710) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_bscc(cpustate, op, op2, &cycle_count);
	}
	/* BScc : 0000 0111 RR00 cccc : A-54 */
	else if ((op & 0xff30) == 0x0700)
	{
		size = dsp56156_op_bscc_1(cpustate, op, &cycle_count);
	}
	/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
	else if (((op & 0xfffc) == 0x0138) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_bsr(cpustate, op, op2, &cycle_count);
	}
	/* BSR : 0000 0001 0010 10RR : A-56 */
	else if ((op & 0xfffc) == 0x0128)
	{
		size = dsp56156_op_bsr_1(cpustate, op, &cycle_count);
	}
	/* CHKAAU : 0000 0000 0000 0100 : A-58 */
	else if ((op & 0xffff) == 0x0004)
	{
		size = dsp56156_op_chkaau(cpustate, op, &cycle_count);
	}
	/* DEBUG : 0000 0000 0000 0001 : A-68 */
	else if ((op & 0xffff) == 0x0001)
	{
		size = dsp56156_op_debug(cpustate, op, &cycle_count);
	}
	/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
	else if ((op & 0xfff0) == 0x0050)
	{
		size = dsp56156_op_debugcc(cpustate, op, &cycle_count);
	}
	/* DIV : 0001 0101 0--0 F1DD : A-76 */
	else if ((op & 0xff94) == 0x1504)
	{
		size = dsp56156_op_div(cpustate, op, &cycle_count);
	}
	/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
	else if ((op & 0xffd0) == 0x1590)
	{
		size = dsp56156_op_dmac(cpustate, op, &cycle_count);
	}
	/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x00c0) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_do(cpustate, op, op2, &cycle_count);
	}
	/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xff00) == 0x0e00) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_do_1(cpustate, op, op2, &cycle_count);
	}
	/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
	else if (((op & 0xffe0) == 0x0400) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_do_2(cpustate, op, op2, &cycle_count);
	}
	/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
	else if (((op & 0xffff) == 0x0002) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_doforever(cpustate, op, op2, &cycle_count);
	}
	/* ENDDO : 0000 0000 0000 1001 : A-92 */
	else if ((op & 0xffff) == 0x0009)
	{
		size = dsp56156_op_enddo(cpustate, op, &cycle_count);
	}
	/* EXT : 0001 0101 0101 F010 : A-96 */
	else if ((op & 0xfff7) == 0x1552)
	{
		size = dsp56156_op_ext(cpustate, op, &cycle_count);
	}
	/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
	else if ((op & 0xffff) == 0x000f)
	{
		size = dsp56156_op_illegal(cpustate, op, &cycle_count);
	}
	/* IMAC : 0001 0101 1010 FQQQ : A-100 */
	else if ((op & 0xfff0) == 0x15a0)
	{
		size = dsp56156_op_imac(cpustate, op, &cycle_count);
	}
	/* IMPY : 0001 0101 1000 FQQQ : A-102 */
	else if ((op & 0xfff0) == 0x1580)
	{
		size = dsp56156_op_impy(cpustate, op, &cycle_count);
	}
	/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
	else if (((op & 0xff30) == 0x0630) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_jcc(cpustate, op, op2, &cycle_count);
	}
	/* Jcc : 0000 0110 RR10 cccc : A-108 */
	else if ((op & 0xff30) == 0x0620 )
	{
		size = dsp56156_op_jcc_1(cpustate, op, &cycle_count);
	}
	/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
	else if (((op & 0xfffc) == 0x0134) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_jmp(cpustate, op, op2, &cycle_count);
	}
	/* JMP : 0000 0001 0010 01RR : A-110 */
	else if ((op & 0xfffc) == 0x0124)
	{
		size = dsp56156_op_jmp_1(cpustate, op, &cycle_count);
	}
	/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
	else if (((op & 0xff30) == 0x0610) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_jscc(cpustate, op, op2, &cycle_count);
	}
	/* JScc : 0000 0110 RR00 cccc : A-112 */
	else if ((op & 0xff30) == 0x0600)
	{
		size = dsp56156_op_jscc_1(cpustate, op, &cycle_count);
	}
	/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
	else if (((op & 0xfffc) == 0x0130) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_jsr(cpustate, op, op2, &cycle_count);
	}
	/* JSR : 0000 1010 AAAA AAAA : A-114 */
	else if ((op & 0xff00) == 0x0a00)
	{
		size = dsp56156_op_jsr_1(cpustate, op, &cycle_count);
	}
	/* JSR : 0000 0001 0010 00RR : A-114 */
	else if ((op & 0xfffc) == 0x0120)
	{
		size = dsp56156_op_jsr_2(cpustate, op, &cycle_count);
	}
	/* LEA : 0000 0001 11TT MMRR : A-116 */
	else if ((op & 0xffc0) == 0x01c0)
	{
		size = dsp56156_op_lea(cpustate, op, &cycle_count);
	}
	/* LEA : 0000 0001 10NN MMRR : A-116 */
	else if ((op & 0xffc0) == 0x0180)
	{
		size = dsp56156_op_lea_1(cpustate, op, &cycle_count);
	}
	/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
	else if ((op & 0xfff0) == 0x15e0)
	{
		size = dsp56156_op_macsuuu(cpustate, op, &cycle_count);
	}
	/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0x00ff) == 0x0011))
	{
		size = dsp56156_op_move_2(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
	else if ((op & 0xf810) == 0x3800)
	{
		size = dsp56156_op_movec(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
	else if ((op & 0xf814) == 0x3810)
	{
		size = dsp56156_op_movec_1(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
	else if ((op & 0xf816) == 0x3816)
	{
		size = dsp56156_op_movec_2(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
	else if (((op & 0xf816) == 0x3814) && ((op2 & 0x0000) == 0x0000))
	{
		size = dsp56156_op_movec_3(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
	else if ((op & 0xfc00) == 0x2800)
	{
		size = dsp56156_op_movec_4(cpustate, op, &cycle_count);
	}
	/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xf810) == 0x3800))
	{
		size = dsp56156_op_movec_5(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
	else if ((op & 0xfc00) == 0x2000)
	{
		size = dsp56156_op_movei(cpustate, op, &cycle_count);
	}
	/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
	else if ((op & 0xfe20) == 0x0200)
	{
		size = dsp56156_op_movem(cpustate, op, &cycle_count);
	}
	/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
	else if ((op & 0xfe30) == 0x0230)
	{
		size = dsp56156_op_movem_1(cpustate, op, &cycle_count);
	}
	/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
	else if (((op & 0xff00) == 0x0500) && ((op2 & 0xfe20) == 0x0200))
	{
		size = dsp56156_op_movem_2(cpustate, op, op2, &cycle_count);
	}
	/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
	else if ((op & 0xfe20) == 0x1820)
	{
		size = dsp56156_op_movep(cpustate, op, &cycle_count);
	}
	/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
	else if ((op & 0xfe00) == 0x0c00)
	{
		size = dsp56156_op_movep_1(cpustate, op, &cycle_count);
	}
	/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
	else if ((op & 0xfe20) == 0x1800)
	{
		size = dsp56156_op_moves(cpustate, op, &cycle_count);
	}
	/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
	else if ((op & 0xfff0) == 0x15c0)
	{
		size = dsp56156_op_mpysuuu(cpustate, op, &cycle_count);
	}
	/* NEGC : 0001 0101 0110 F000 : A-168 */
	else if ((op & 0xfff7) == 0x1560)
	{
		size = dsp56156_op_negc(cpustate, op, &cycle_count);
	}
	/* NOP : 0000 0000 0000 0000 : A-170 */
	else if ((op & 0xffff) == 0x0000)
	{
		size = dsp56156_op_nop(cpustate, op, &cycle_count);
	}
	/* NORM : 0001 0101 0010 F0RR : A-172 */
	else if ((op & 0xfff4) == 0x1520)
	{
		size = dsp56156_op_norm(cpustate, op, &cycle_count);
	}
	/* ORI : 0001 1EE1 iiii iiii : A-178 */
	else if ((op & 0xf900) == 0x1900)
	{
		size = dsp56156_op_ori(cpustate, op, &cycle_count);
	}
	/* REP : 0000 0000 111- --RR : A-180 */
	else if ((op & 0xffe0) == 0x00e0)
	{
		size = dsp56156_op_rep(cpustate, op, &cycle_count);
	}
	/* REP : 0000 1111 iiii iiii : A-180 */
	else if ((op & 0xff00) == 0x0f00)
	{
		size = dsp56156_op_rep_1(cpustate, op, &cycle_count);
	}
	/* REP : 0000 0100 001D DDDD : A-180 */
	else if ((op & 0xffe0) == 0x0420)
	{
		size = dsp56156_op_rep_2(cpustate, op, &cycle_count);
	}
	/* REPcc : 0000 0001 0101 cccc : A-184 */
	else if ((op & 0xfff0) == 0x0150)
	{
		size = dsp56156_op_repcc(cpustate, op, &cycle_count);
	}
	/* RESET : 0000 0000 0000 1000 : A-186 */
	else if ((op & 0xffff) == 0x0008)
	{
		size = dsp56156_op_reset(cpustate, op, &cycle_count);
	}
	/* RTI : 0000 0000 0000 0111 : A-194 */
	else if ((op & 0xffff) == 0x0007)
	{
		size = dsp56156_op_rti(cpustate, op, &cycle_count);
	}
	/* RTS : 0000 0000 0000 0110 : A-196 */
	else if ((op & 0xffff) == 0x0006)
	{
		size = dsp56156_op_rts(cpustate, op, &cycle_count);
	}
	/* STOP : 0000 0000 0000 1010 : A-200 */
	else if ((op & 0xffff) == 0x000a)
	{
		size = dsp56156_op_stop(cpustate, op, &cycle_count);
	}
	/* SWAP : 0001 0101 0111 F001 : A-206 */
	else if ((op & 0xfff7) == 0x1571)
	{
		size = dsp56156_op_swap(cpustate, op, &cycle_count);
	}
	/* SWI : 0000 0000 0000 0101 : A-208 */
	else if ((op & 0xffff) == 0x0005)
	{
		size = dsp56156_op_swi(cpustate, op, &cycle_count);
	}
	/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
	else if ((op & 0xfc02) == 0x1000)
	{
		size = dsp56156_op_tcc(cpustate, op, &cycle_count);
	}
	/* TFR(2) : 0001 0101 0000 F00J : A-214 */
	else if ((op & 0xfff6) == 0x1500)
	{
		size = dsp56156_op_tfr2(cpustate, op, &cycle_count);
	}
	/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
	else if ((op & 0xfc00) == 0x2400)
	{
		size = dsp56156_op_tfr3(cpustate, op, &cycle_count);
	}
	/* TST(2) : 0001 0101 0001 -1DD : A-220 */
	else if ((op & 0xfff4) == 0x1514)
	{
		size = dsp56156_op_tst2(cpustate, op, &cycle_count);
	}
	/* WAIT : 0000 0000 0000 1011 : A-222 */
	else if ((op & 0xffff) == 0x000b)
	{
		size = dsp56156_op_wait(cpustate, op, &cycle_count);
	}
	/* ZERO : 0001 0101 0101 F000 : A-224 */
	else if ((op & 0xfff7) == 0x1550)
	{
		size = dsp56156_op_zero(cpustate, op, &cycle_count);
	}


	/* Not recognized?  Nudge debugger onto the next word */
	if (size == 0x1337)
	{
		LOGMASKED(LOG_UNIMPLEMENTED, "DSP56156: Unimplemented opcode at 0x%04x : %04x\n", PC, op);
		size = 1;                      /* Just to get the debugger past the bad opcode */
	}

	/* Must have been a good opcode */
	PC += (uint16_t)size;

	dsp56156_process_loop(cpustate);
	dsp56156_process_rep(cpustate, size);

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
static size_t dsp56156_op_addsub_2(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	uint64_t useVal = 0;
	uint8_t op_type = OP_OTHER;
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_uuuuF_table(cpustate, BITS(op_byte,0x0017), BITS(op_byte,0x0008), op_type, &S, &D);

	/* If you gave an invalid operation type, presume it's a nop and move on with the parallel move, priming the accumulator as appropriate */
	if (op_type == OP_OTHER)
	{
		d_register->addr = BIT(op_byte, 3) ? &B : &A;
		d_register->data_type = DT_LONG_WORD;
		cycles += 2;
		return 1;
	}

	/* It's a real operation.  Get on with it. */
	switch(S.data_type)
	{
		case DT_WORD:        useVal = (uint64_t)*((uint16_t*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: useVal = (uint64_t)*((uint32_t*)S.addr);       break;
		case DT_LONG_WORD:   useVal = (uint64_t)*((uint64_t*)S.addr);       break;
	}

	/* Sign-extend word for proper add/sub op */
	if ((S.data_type != DT_LONG_WORD) && (useVal & 0x00000000'80000000ULL))
		useVal |= 0x000000ff'00000000ULL;

	/* Make sure they're both real 40-bit values */
	useVal &= 0x000000ff'ffffffffULL;
	*((uint64_t*)D.addr) &= 0x000000ff'ffffffffULL;

	/* Operate*/
	if (op_type == OP_ADD)
		*((uint64_t*)D.addr) += useVal;
	else if (op_type == OP_SUB)
		*((uint64_t*)D.addr) -= useVal;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U, V, C */
	if (*((uint64_t*)D.addr) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (*((uint64_t*)D.addr) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
static size_t dsp56156_op_mac_1(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S1, &S2, &D);

	/* Cast both values as being signed */
	const int32_t s1 = *((int16_t*)S1);
	const int32_t s2 = *((int16_t*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	int64_t result = (s1 * s2) << 1;

	/* Sign extend D into a temp variable */
	int64_t opD = util::sext(*((uint64_t*)D), 40);

	/* Accumulate */
	opD += result;

	/* And out the bits that don't live in the register */
	opD &= 0x000000ff'ffffffffULL;

	(*((uint64_t*)D)) = (uint64_t)opD;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
static size_t dsp56156_op_macr_1(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_macr_1\n");
	return 0;
}

/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
static size_t dsp56156_op_move_1(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_move_1\n");
	return 0;
}

/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
static size_t dsp56156_op_mpy_1(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S1, &S2, &D);

	/* Cast both values as being signed */
	const int32_t s1 = *((int16_t*)S1);
	const int32_t s2 = *((int16_t*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	int64_t result = (s1 * s2) << 1;

	/* And out the bits that don't live in the register */
	(*((uint64_t*)D)) = result & 0x000000ff'ffffffffULL;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
static size_t dsp56156_op_mpyr_1(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_mpyr_1\n");
	return 0;
}

/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
static size_t dsp56156_op_tfr_2(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_tfr_2\n");
	return 0;
}

/* MPY : 0001 0110 RRDD FQQQ : A-160 */
static size_t dsp56156_op_mpy_2(dsp56156_core* cpustate, const uint16_t op_byte, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_mpy_2\n");
	return 0;
}

/* MAC : 0001 0111 RRDD FQQQ : A-122 */
static size_t dsp56156_op_mac_2(dsp56156_core* cpustate, const uint16_t op_byte, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_mac_2\n");
	return 0;
}

/* CLR : .... .... 0000 F001 : A-60 */
static size_t dsp56156_op_clr(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_LONG_WORD};
	typed_pointer clear = {nullptr, DT_LONG_WORD};
	uint64_t clear_val = 0x00000000'00000000ULL;

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	clear.addr = &clear_val;
	clear.data_type = DT_LONG_WORD;
	dsp56156_set_destination_value(cpustate, clear, D);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * 0 - */
	/* TODO - S, L */
	DSP56156_E_CLEAR();
	DSP56156_U_SET();
	DSP56156_N_CLEAR();
	DSP56156_Z_SET();
	DSP56156_V_CLEAR();

	cycles += 2;    /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* ADD : .... .... 0000 FJJJ : A-22 */
static size_t dsp56156_op_add(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	uint64_t addVal = 0;

	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};
	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((uint64_t*)D.addr);

	switch(S.data_type)
	{
		case DT_WORD:        addVal = (uint64_t)*((uint16_t*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: addVal = (uint64_t)*((uint32_t*)S.addr);       break;
		case DT_LONG_WORD:   addVal = (uint64_t)*((uint64_t*)S.addr);       break;
	}

	/* Sign-extend word for proper add/sub op */
	if ((S.data_type != DT_LONG_WORD) && (addVal & 0x00000000'80000000ULL))
		addVal |= 0x000000ff'00000000ULL;

	/* Make sure they're both real 40-bit values */
	addVal &= 0x000000ff'ffffffffULL;
	*((uint64_t*)D.addr) &= 0x000000ff'ffffffffULL;

	/* Operate*/
	const uint64_t result = *((uint64_t*)D.addr) + addVal;
	const uint8_t ovf_index = (BIT(*((uint64_t*)D.addr), 39) << 2) | (BIT(addVal, 39) << 1) | BIT(result, 39);
	*((uint64_t*)D.addr) = result;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U */
	if (*((uint64_t*)D.addr) & 0x00000080'00000000ULL)        DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (*((uint64_t*)D.addr) == 0)                            DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if (dsp56156_add_ovf_table[ovf_index])                    DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((*((uint64_t*)D.addr) & 0xffffff00'00000000ULL) != 0) DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* MOVE : .... .... 0001 0001 : A-128 */
static size_t dsp56156_op_move(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	// Equivalent to a nop with a parallel move
	// These can't be used later.  Hopefully compilers would pick this up.
	*p_accum = 0;
	d_register->addr = nullptr;
	d_register->data_type = DT_BYTE;

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;    /* TODO: + mv oscillator cycles */
	return 1;
}

/* TFR : .... .... 0001 FJJJ : A-212 */
static size_t dsp56156_op_tfr(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((uint64_t*)D.addr);

	dsp56156_set_destination_value(cpustate, S, D);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* RND : .... .... 0010 F000 : A-188 */
static size_t dsp56156_op_rnd(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	uint64_t *d_ptr = (uint64_t*)D.addr;
	if (R_bit(cpustate))
	{
		if ((*d_ptr & 0x00000000'0000ffffULL) >= 0x8000)
			*d_ptr += 0x00000000'00010000ULL;
	}
	else
	{
		uint64_t lower = *d_ptr & 0x00000000'0000ffffULL;
		if (lower > 0x8000)
			*d_ptr += 0x00000000'00010000ULL;
		else if (lower == 0x8000 && BIT(*d_ptr, 16))
			*d_ptr += 0x00000000'00010000ULL;
	}

	*((uint64_t*)D.addr) = *((uint64_t*)D.addr) & 0x000000ff'ffff0000ULL;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, U, V */
	if ((*((uint64_t*)D.addr)) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr)) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* TST : .... .... 0010 F001 : A-218 */
static size_t dsp56156_op_tst(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_LONG_WORD};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* 0 * * * * * 0 0 */
	/* TODO: S, L, E, U */
	if ((*((uint64_t*)D.addr)) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr)) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();
	DSP56156_C_CLEAR();

	cycles += 2;    /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* INC : .... .... 0010 F010 : A-104 */
static size_t dsp56156_op_inc(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((uint64_t*)D.addr);

	/* Make sure the destination is a real 40-bit value */
	*((uint64_t*)D.addr) &= 0x000000ff'ffffffffULL;

	/* Increment */
	*((uint64_t*)D.addr) = *((uint64_t*)D.addr) + 1;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U */
	if ( *((uint64_t*)D.addr) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if ((*((uint64_t*)D.addr) & 0xffffff00'00000000ULL) != 0) DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((*((uint64_t*)D.addr) & 0xffffff00'00000000ULL) != 0) DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;    /* TODO: +mv oscillator cycles */
	return 1;
}

/* INC24 : .... .... 0010 F011 : A-106 */
static size_t dsp56156_op_inc24(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	uint32_t workBits24;

	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((uint64_t*)D.addr);

	/* TODO: I wonder if workBits24 should be signed? */
	workBits24 = ((*((uint64_t*)D.addr)) & 0x000000ff'ffff0000ULL) >> 16;
	workBits24++;
	//workBits24 &= 0x00ffffff;     /* Solves -x issues - TODO: huh? */

	/* Set the D bits with the dec result */
	*((uint64_t*)D.addr) &= 0x00000000'0000ffffULL;
	*((uint64_t*)D.addr) |= (((uint64_t)(workBits24)) << 16);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * ? * * */
	/* TODO: S, L, E, U */
	if ( *((uint64_t*)D.addr) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x000000ff'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if ((workBits24 & 0xff000000) != 0)                       DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((workBits24 & 0xff000000) != 0)                       DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* OR : .... .... 0010 F1JJ : A-176 */
static size_t dsp56156_op_or(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S, &D);

	/* Save some data for the parallel move */
	*p_accum = *((uint64_t*)D.addr);

	/* OR a word of S with A1|B1 */
	((PAIR64*)D.addr)->w.h = *((uint16_t*)S.addr) | ((PAIR64*)D.addr)->w.h;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S, L */
	if ( *((uint64_t*)D.addr) & 0x00000000'80000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x00000000'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* ASR : .... .... 0011 F000 : A-32 */
static size_t dsp56156_op_asr(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	*((uint64_t*)D.addr) = (*((uint64_t*)D.addr)) >> 1;

	/* Make sure the MSB is maintained */
	if (*p_accum & 0x00000080'00000000ULL)
		*((uint64_t*)D.addr) |= 0x00000080'00000000ULL;
	else
		*((uint64_t*)D.addr) &= (~u64(0x00000080'00000000ULL));

	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * 0 ? */
	/* TODO: S, L, E, U */
	if (*((uint64_t*)D.addr) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (*((uint64_t*)D.addr) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();
	if (*p_accum & 0x00000000'00000001ULL)             DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* ASL : .... .... 0011 F001 : A-28 */
static size_t dsp56156_op_asl(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	uint64_t dVal = *(uint64_t*)D.addr;
	const uint64_t oldBit39 = BIT(dVal, 39);
	dVal = (dVal << 1) & 0x000000ff'ffffffffULL;
	*((uint64_t*)D.addr) = dVal;
	const uint64_t newBit39 = BIT(dVal, 39);

	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * ? ? */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significant
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	/* C - Set if bit 39 of source operand is set. Cleared otherwise. */
	if (oldBit39 != newBit39) DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if (newBit39)             DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* LSR : .... .... 0011 F010 : A-120 */
static size_t dsp56156_op_lsr(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	((PAIR64*)D.addr)->w.h = (((PAIR64*)D.addr)->w.h) >> 1;

	/* Make sure bit 31 gets a 0 */
	((PAIR64*)D.addr)->w.h &= (~0x8000);

	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* TODO: S, L */
	DSP56156_N_CLEAR();
	if (((PAIR64*)D.addr)->w.h == 0)       DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();
	if (*p_accum & 0x00000000'00010000ULL) DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* LSL : .... .... 0011 F011 : A-118 */
static size_t dsp56156_op_lsl(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 31 of the source operand is set. Cleared otherwise. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_lsl\n");
	return 0;
}

/* EOR : .... .... 0011 F1JJ : A-94 */
static size_t dsp56156_op_eor(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S, &D);

	/* Save some data for the parallel move */
	*p_accum = *((uint64_t*)D.addr);

	/* XOR a word of S with A1|B1 */
	((PAIR64*)D.addr)->w.h = *((uint16_t*)S.addr) ^ ((PAIR64*)D.addr)->w.h;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S, L */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	if ( *((uint64_t*)D.addr) & 0x00000000'80000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x00000000'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* SUBL : .... .... 0100 F001 : A-204 */
static size_t dsp56156_op_subl(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * ? * */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significant
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_subl\n");
	return 0;
}

/* SUB : .... .... 0100 FJJJ : A-202 */
static size_t dsp56156_op_sub(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	uint64_t useVal = 0;
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((uint64_t*)D.addr);

	/* Get on with it. */
	switch(S.data_type)
	{
		case DT_WORD:        useVal = (uint64_t)*((uint16_t*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: useVal = (uint64_t)*((uint32_t*)S.addr);       break;
		case DT_LONG_WORD:   useVal = (uint64_t)*((uint64_t*)S.addr);       break;
	}

	/* Sign-extend word for proper sub op */
	if ((S.data_type != DT_LONG_WORD) && (useVal & 0x00000000'80000000ULL))
		useVal |= 0x000000ff'00000000ULL;

	/* Make sure they're both real 40-bit values */
	useVal &= 0x000000ff'ffffffffULL;
	*((uint64_t*)D.addr) &= 0x000000ff'ffffffffULL;

	/* Operate*/
	uint64_t result = *((uint64_t*)D.addr) - useVal;
	const uint8_t ovf_index = (BIT(*((uint64_t*)D.addr), 39) << 2) | (BIT(useVal, 39) << 1) | BIT(result, 39);
	*((uint64_t*)D.addr) = result;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO S, L, E, U */
	if ( *((uint64_t*)D.addr) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ( *((uint64_t*)D.addr) == 0)                           DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if (dsp56156_sub_ovf_table[ovf_index])                    DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((*((uint64_t*)D.addr) & 0xffffff00'00000000ULL) != 0) DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* CLR24 : .... .... 0101 F001 : A-62 */
static size_t dsp56156_op_clr24(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * ? 0 - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_clr24\n");
	return 0;
}

/* SBC : .... .... 0101 F01J : A-198 */
static size_t dsp56156_op_sbc(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_sbc\n");
	return 0;
}

/* CMP : .... .... 0101 FJJJ : A-64 */
static size_t dsp56156_op_cmp(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	uint64_t cmpVal = 0;
	uint64_t dVal = 0;

	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((uint64_t*)D.addr);
	dVal = *((int64_t*)D.addr);

	switch(S.data_type)
	{
		case DT_WORD:        cmpVal = (int64_t)*((uint16_t*)S.addr) << 16; break;
		case DT_DOUBLE_WORD: cmpVal = (int64_t)*((uint32_t*)S.addr);       break;
		case DT_LONG_WORD:   cmpVal = (int64_t)*((uint64_t*)S.addr);       break;
	}

	/* Sign-extend word for proper add/sub op */
	if ((S.data_type != DT_LONG_WORD) && (cmpVal & 0x00000000'80000000ULL))
		cmpVal |= 0x000000ff'00000000ULL;

	/* Make sure they're both real 40-bit values */
	cmpVal &= 0x000000ff'ffffffffULL;
	dVal &= 0x000000ff'ffffffffULL;

	/* Operate */
	uint64_t result = dVal - cmpVal;

	d_register->data_type = DT_LONG_WORD;
	d_register->addr = D.addr;

	uint8_t ovf_index = (BIT(dVal, 39) << 2) | (BIT(cmpVal, 39) << 1) | BIT(result, 39);

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U */
	if (result & 0x00000080'00000000ULL)        DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (result == 0)                            DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if (dsp56156_sub_ovf_table[ovf_index])      DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((result & 0xffffff00'00000000ULL) != 0) DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* NEG : .... .... 0110 F000 : A-166 */
static size_t dsp56156_op_neg(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_LONG_WORD};
	typed_pointer neg = {nullptr, DT_LONG_WORD};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	const uint64_t sVal = *p_accum = *((uint64_t*)D.addr);
	const uint64_t dVal = 0 - sVal;
	uint64_t dMasked = dVal & 0x000000ff'ffffffffULL;

	neg.addr = &dMasked;
	neg.data_type = DT_LONG_WORD;
	dsp56156_set_destination_value(cpustate, neg, D);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	const uint8_t ovf_index = (BIT(sVal, 39) << 1) | BIT(dVal, 39);

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO - S, L */
	if (dsp56156_value_is_unnormalized(cpustate, dVal)) DSP56156_U_SET(); else DSP56156_U_CLEAR();
	if (dsp56156_value_is_extended(cpustate, dVal))     DSP56156_E_SET(); else DSP56156_E_CLEAR();
	if (dVal & 0x00000080'00000000ULL)                  DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (dVal == 0)                                      DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if (dsp56156_sub_ovf_table[ovf_index])              DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((dVal & 0xffffff00'00000000ULL) != 0)           DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;    /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* NOT : .... .... 0110 F001 : A-174 */
static size_t dsp56156_op_not(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	/* Invert bits [16:31] of D */
	((PAIR64*)D.addr)->w.h = ~(((PAIR64*)D.addr)->w.h);

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S?, L */
	if ( *((uint64_t*)D.addr) & 0x00000000'80000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x00000000'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* DEC : .... .... 0110 F010 : A-72 */
static size_t dsp56156_op_dec(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * * */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_dec\n");
	return 0;
}

/* DEC24 : .... .... 0110 F011 : A-74 */
static size_t dsp56156_op_dec24(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	uint32_t workBits24;

	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	/* Save some data for the parallel move */
	*p_accum = *((uint64_t*)D.addr);

	/* TODO: I wonder if workBits24 should be signed? */
	workBits24 = ((*((uint64_t*)D.addr)) & 0x000000ff'ffff0000ULL) >> 16;
	workBits24--;
	const bool carry = BIT(workBits24, 24);
	workBits24 &= 0x00ffffff;       /* Solves -x issues */

	/* Set the D bits with the dec result */
	*((uint64_t*)D.addr) &= 0x00000000'0000ffffULL;
	*((uint64_t*)D.addr) |= uint64_t(workBits24) << 16;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * ? * * */
	/* TODO: S, L, E, U, V */
	if ( *((uint64_t*)D.addr) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x000000ff'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if (carry)                                                DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* AND : .... .... 0110 F1JJ : A-24 */
static size_t dsp56156_op_and(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJF_table(cpustate, BITS(op_byte,0x0003), BITS(op_byte,0x0008), &S, &D);

	/* Save some data for the parallel move */
	*p_accum = *((uint64_t*)D.addr);

	/* AND a word of S with A1|B1 */
	((PAIR64*)D.addr)->w.h = *((uint16_t*)S.addr) & ((PAIR64*)D.addr)->w.h;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * - - ? ? 0 - */
	/* TODO: S, L */
	if ( *((uint64_t*)D.addr) & 0x00000000'80000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x00000000'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* ABS : .... .... 0111 F001 : A-18 */
static size_t dsp56156_op_abs(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_LONG_WORD};

	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	/* Sign extend D into a temp variable */
	int64_t opD = util::sext(*p_accum, 40);

	/* Take the absolute value and clean up */
	opD = (opD < 0) ? -opD : opD;
	opD &= 0x000000ff'ffffffffULL;

	/* Reassign */
	*((uint64_t*)D.addr) = opD;

	/* Special overflow case */
	if ((*p_accum) == 0x00000080'00000000ULL)
		*((uint64_t*)D.addr) = 0x0000007f'ffffffffULL;

	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, U */
	if ( *((uint64_t*)D.addr) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if ((*p_accum) == 0x00000080'00000000ULL)                 DSP56156_V_SET(); else DSP56156_V_CLEAR();

	cycles += 2;            /* TODO: + mv oscillator clock cycles */
	return 1;
}

/* ROR : .... .... 0111 F010 : A-192 */
static size_t dsp56156_op_ror(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - ? ? 0 ? */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 16-31 of the result are zero. Cleared otherwise. */
	/* C - Set if bit 16 of the source operand is set. Cleared otherwise. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_ror\n");
	return 0;
}

/* ROL : .... .... 0111 F011 : A-190 */
static size_t dsp56156_op_rol(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op_byte,0x0008), &D);

	*p_accum = *((uint64_t*)D.addr);

	uint64_t dVal = *(uint64_t*)D.addr;
	uint16_t middle = (uint16_t)(dVal >> 16);
	bool new_carry = BIT(middle, 15);
	middle = (middle << 1) | C();
	dVal &= ~0x00000000'ffff0000ULL;
	dVal |= middle << 16;

	/* For the parallel move */
	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	*((uint64_t*)D.addr) = dVal;

	/* S L E U N Z V C */
	/* * * - - * * 0 * */
	/* TODO: S, L, E, U */
	/* N - Set if bit 31 of the result is set. Cleared otherwise. */
	/* Z - Set if bits 31-16 of the result are zero. Cleared otherwise. */
	/* V - Always cleared. */
	/* C - Set if bit 31 of the source operand was set. Cleared otherwise. */
	if (middle & 0x8000)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (middle == 0)           DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();
	if (new_carry)             DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: + mv oscillator cycles */
	return 1;
}

/* CMPM : .... .... 0111 FJJJ : A-66 */
static size_t dsp56156_op_cmpm(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JJJF_table(cpustate, BITS(op_byte,0x0007),BITS(op_byte,0x0008), &S, &D);

	*p_accum = *((uint64_t*)D.addr);

	/* Sign extend and get absolute value of the source */
	uint64_t absS;
	if (S.addr == &A || S.addr == &B)
		absS = std::abs(util::sext(*((uint64_t*)S.addr), 40));
	else
		absS = std::abs((int64_t)(int32_t)((*((uint16_t*)S.addr)) << 16));

	/* Sign extend and get absolute value of the destination */
	uint64_t absD;
	if (D.addr == &A || D.addr == &B)
		absD = std::abs(util::sext(*((uint64_t*)D.addr), 40));
	else
		absD = std::abs((int64_t)(int32_t)(*((uint16_t*)D.addr) << 16));

	/* Make sure they're both real 40-bit values */
	absS &= 0x000000ff'ffffffffULL;
	absD &= 0x000000ff'ffffffffULL;

	/* Compare */
	const uint64_t absResult = absD - absS;

	d_register->addr = D.addr;
	d_register->data_type = D.data_type;

	uint8_t ovf_index = (BIT(absD, 39) << 2) | (BIT(absS, 39) << 1) | BIT(absResult, 39);

	/* S L E U N Z V C */
	/* * * * * * * * * */
	/* TODO: S, L, E, U */
	if (absResult & 0x00000080'00000000ULL)        DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (absResult == 0)                            DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if (dsp56156_sub_ovf_table[ovf_index])         DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if ((absResult & 0xffffff00'00000000ULL) != 0) DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MPY : .... .... 1k00 FQQQ : A-160    -- CONFIRMED TYPO IN DOCS (HHHH vs HHHW) */
static size_t dsp56156_op_mpy(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQQF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S1, &S2, &D);

	*p_accum = *((uint64_t*)D);

	const uint16_t k = BITS(op_byte,0x0040);

	/* Cast both values as being signed */
	const int32_t s1 = *((int16_t*)S1);
	const int32_t s2 = *((int16_t*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	int64_t result = (s1 * s2) << 1;

	/* Negate the product if necessary */
	if (k)
		result *= -1;

	(*((uint64_t*)D)) = result & 0x000000ff'ffffffffULL;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MPYR : .... .... 1k01 FQQQ : A-162 */
static size_t dsp56156_op_mpyr(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_mpyr\n");
	return 0;
}

/* MAC : .... .... 1k10 FQQQ : A-122 */
static size_t dsp56156_op_mac(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQQF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S1, &S2, &D);

	*p_accum = *((uint64_t*)D);

	const uint16_t k = BITS(op_byte,0x0040);

	/* Cast both values as being signed */
	const int32_t s1 = *((int16_t*)S1);
	const int32_t s2 = *((int16_t*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	int64_t result = (s1 * s2) << 1;

	/* Sign extend D into a temp variable */
	int64_t opD = util::sext(*((uint64_t*)D), 40);

	/* Negate if necessary */
	if (k)
		result *= -1;

	/* Accumulate */
	opD += result;

	/* And out the bits that don't live in the register */
	opD &= 0x000000ff'ffffffffULL;

	(*((uint64_t*)D)) = (uint64_t)opD;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}

/* MACR : .... .... 1k11 FQQQ : A-124   -- DRAMA - rr vs xx (805) */
static size_t dsp56156_op_macr(dsp56156_core* cpustate, const uint16_t op_byte, typed_pointer* d_register, uint64_t* p_accum, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQQF_table(cpustate, BITS(op_byte,0x0007), BITS(op_byte,0x0008), &S1, &S2, &D);

	*p_accum = *((uint64_t*)D);

	const uint16_t k = BITS(op_byte,0x0040);

	/* Cast both values as being signed */
	const int32_t s1 = *((int16_t*)S1);
	const int32_t s2 = *((int16_t*)S2);

	/* Fixed-point 2's complement multiplication requires a shift */
	int64_t result = (s1 * s2) << 1;

	/* Sign extend D into a temp variable */
	int64_t opD = util::sext(*((uint64_t*)D), 40);

	/* Negate if necessary */
	if (k)
		result *= -1;

	/* Accumulate */
	opD += result;

	/* Round the result */
	if (R_bit(cpustate))
	{
		if ((opD & 0x00000000'0000ffffULL) >= 0x8000)
			opD += 0x00000000'00010000ULL;
	}
	else
	{
		const uint64_t lower = opD & 0x00000000'0000ffffULL;
		if (lower > 0x8000)
			opD += 0x00000000'00010000ULL;
		else if (lower == 0x8000 && BIT(opD, 16))
			opD += 0x00000000'00010000ULL;
	}
	opD &= 0x000000ffffff0000ULL;

	/* Store the result */
	(*((uint64_t*)D)) = (uint64_t)opD;

	/* For the parallel move */
	d_register->addr = D;
	d_register->data_type = DT_LONG_WORD;

	/* S L E U N Z V C */
	/* * * * * * * * - */
	/* TODO: S, L, E, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;        /* TODO: +mv oscillator cycles */
	return 1;
}


/******************************/
/* Remaining non-parallel ops */
/******************************/

/* ADC : 0001 0101 0000 F01J : A-20 */
static size_t dsp56156_op_adc(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * * */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_adc\n");
	return 0;
}

/* ANDI : 0001 1EE0 iiii iiii : A-26 */
static size_t dsp56156_op_andi(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	uint16_t immediate = BITS(op,0x00ff);

	/* There is not currently a good way to refer to CCR or MR.  Explicitly decode here. */
	switch(BITS(op,0x0600))
	{
		case 0x01:  /* MR */
			SR &= ((immediate << 8) | 0x00ff);
			break;

		case 0x02:  /* OMR */
			OMR &= (uint8_t)(immediate);
			break;

		case 0x03:  /* CCR */
			SR &= (immediate | 0xff00);
			break;

		default:
			fatalerror("DSP56156 - BAD EE value in andi operation\n");
	}

	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Cleared if the corresponding bit in the immediate data is cleared and if the operand
	   is the CCR. Not affected otherwise. */
	cycles += 2;
	return 1;
}

/* ASL4 : 0001 0101 0011 F001 : A-30 */
static size_t dsp56156_op_asl4(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	uint64_t p_accum = 0;
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op,0x0008), &D);

	p_accum = *((uint64_t*)D.addr);

	*((uint64_t*)D.addr) = (*((uint64_t*)D.addr)) << 4;
	*((uint64_t*)D.addr) = (*((uint64_t*)D.addr)) & 0x000000ff'ffffffffULL;

	/* S L E U N Z V C */
	/* - ? * * * * ? ? */
	/* TODO: L, E, U  */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if bit 35 through 39 are
	       not the same. */
	/* C - Set if bit 36 of source operand is set. Cleared otherwise. */
	if (*((uint64_t*)D.addr) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (*((uint64_t*)D.addr) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	if ((*((uint64_t*)D.addr) & 0x000000ff'00000000ULL) != (p_accum & 0x000000ff'00000000ULL)) DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if (p_accum & 0x00000010'00000000ULL)              DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;
	return 1;
}

/* ASR4 : 0001 0101 0011 F000 : A-34 */
static size_t dsp56156_op_asr4(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	uint64_t p_accum = 0;
	typed_pointer D = {nullptr, DT_BYTE};
	decode_F_table(cpustate, BITS(op,0x0008), &D);

	p_accum = *((uint64_t*)D.addr);

	*((uint64_t*)D.addr) = (*((uint64_t*)D.addr)) >> 4;
	*((uint64_t*)D.addr) = (*((uint64_t*)D.addr)) & 0x000000ff'ffffffffULL;

	/* The top 4 bits become the old bit 39 */
	if (p_accum & 0x00000080'00000000ULL)
		*((uint64_t*)D.addr) |= 0x000000f0'00000000ULL;
	else
		*((uint64_t*)D.addr) &= ~0x000000f0'00000000ULL;

	/* S L E U N Z V C */
	/* - * * * * * 0 ? */
	/* TODO: E, U  */
	/* C - Set if bit 3 of source operand is set. Cleared otherwise. */
	if (*((uint64_t*)D.addr) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (*((uint64_t*)D.addr) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();
	if (p_accum & 0x00000000'00000008ULL)              DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;
	return 1;
}

/* ASR16 : 0001 0101 0111 F000 : A-36 */
static size_t dsp56156_op_asr16(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	uint64_t backupVal;
	typed_pointer D = {nullptr, DT_BYTE};

	decode_F_table(cpustate, BITS(op,0x0008), &D);

	backupVal = *((uint64_t*)D.addr);

	*((uint64_t*)D.addr) = *((uint64_t*)D.addr) >> 16;

	if (backupVal & 0x00000080'00000000ULL)
		*((uint64_t*)D.addr) |= 0x000000ffff000000ULL;
	else
		*((uint64_t*)D.addr) &= 0x00000000'00ffffffULL;

	/* S L E U N Z V C */
	/* - * * * * * 0 ? */
	/* TODO: E, U */
	if (*((uint64_t*)D.addr) & 0x00000080'00000000ULL) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if (*((uint64_t*)D.addr) == 0)                     DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();
	if (backupVal & 0x00000000'00008000ULL)            DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;
	return 1;
}

/* BFCHG  : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
static size_t dsp56156_op_bfop(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	uint16_t workAddr = 0x0000;
	uint16_t workingWord = 0x0000;
	uint16_t previousValue = 0x0000;
	typed_pointer tempTP = { nullptr, DT_BYTE };

	uint16_t iVal = op2 & 0x00ff;
	decode_BBB_bitmask(cpustate, BITS(op2,0xe000), &iVal);

	workAddr = assemble_address_from_Pppppp_table(cpustate, BITS(op,0x0020), BITS(op,0x001f));
	previousValue = cpustate->data.read_word(workAddr);
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
	dsp56156_set_data_memory_value(cpustate, tempTP, workAddr);

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* TODO: L */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			if ((iVal & previousValue) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x04:  /* BFCLR */
			if ((iVal & previousValue) == 0x0000) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x18:  /* BFSET */
			if ((iVal & previousValue) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x10:  /* BFTSTH */
			if ((iVal & previousValue) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x00:  /* BFTSTL */
			if ((iVal & previousValue) == 0x0000) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
	}

	cycles += 4;    /* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* BFCHG  : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
static size_t dsp56156_op_bfop_1(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	uint16_t workAddr = 0x0000;
	uint16_t workingWord = 0x0000;
	uint16_t previousValue = 0x0000;
	typed_pointer R = { nullptr, DT_BYTE };
	typed_pointer tempTP = { nullptr, DT_BYTE };

	uint16_t iVal = op2 & 0x00ff;
	decode_BBB_bitmask(cpustate, BITS(op2,0xe000), &iVal);

	decode_RR_table(cpustate, BITS(op,0x0003), &R);

	workAddr = *((uint16_t*)R.addr);
	previousValue = cpustate->data.read_word(workAddr);
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
	dsp56156_set_data_memory_value(cpustate, tempTP, workAddr);

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* TODO: L */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			if ((iVal & previousValue) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x04:  /* BFCLR */
			if ((iVal & previousValue) == 0x0000) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x18:  /* BFSET */
			if ((iVal & previousValue) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x10:  /* BFTSTH */
			if ((iVal & previousValue) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x00:  /* BFTSTL */
			if ((iVal & previousValue) == 0x0000) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
	}

	cycles += 4;    /* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* BFCHG  : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
/* BFCLR  : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
/* BFSET  : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
static size_t dsp56156_op_bfop_2(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	uint16_t workingWord = 0;
	uint16_t previousWord = 0;
	uint64_t previousAccum = 0;

	uint16_t iVal = op2 & 0x00ff;
	typed_pointer S = { nullptr, DT_BYTE };

	decode_BBB_bitmask(cpustate, BITS(op2,0xe000), &iVal);
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &S);

	/* A & B are special */
	if (S.data_type == DT_LONG_WORD)
	{
		previousAccum = dsp56156_limit_long_to_long(cpustate, ((PAIR64*)S.addr)->q);
		previousWord = (uint16_t)(previousAccum >> 16);
	}
	else
	{
		previousWord = *((uint16_t*)S.addr);
	}

	workingWord = previousWord;

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
	{
		((PAIR64*)S.addr)->q = (previousAccum & 0x000000ff'00000000ULL) | ((uint64_t)workingWord << 16);
	}
	else
	{
		*(uint16_t*)S.addr = workingWord;
	}

	/* S L E U N Z V C */
	/* - * - - - - - ? */
	/* TODO: L */
	switch(BITS(op2, 0x1f00))
	{
		case 0x12:  /* BFCHG */
			if ((iVal & previousWord) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x04:  /* BFCLR */
			if ((iVal & previousWord) == 0x0000) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x18:  /* BFSET */
			if ((iVal & previousWord) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x10:  /* BFTSTH */
			if ((iVal & previousWord) == iVal) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
		case 0x00:  /* BFTSTL */
			if ((iVal & previousWord) == 0x0000) DSP56156_C_SET(); else DSP56156_C_CLEAR(); break;
	}

	cycles += 4;    /* TODO: + mvb oscillator clock cycles */
	return 2;
}

/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
static size_t dsp56156_op_bcc(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	int shouldBranch = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldBranch)
	{
		int16_t offset = (int16_t)op2;

		PC += offset + 2;

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
static size_t dsp56156_op_bcc_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	int shouldBranch = decode_cccc_table(cpustate, BITS(op,0x03c0));

	if (shouldBranch)
	{
		int16_t offset = util::sext(op, 6);

		PC += offset + 1;

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
static size_t dsp56156_op_bcc_2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_bcc_2\n");
	return 0;
}

/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
static size_t dsp56156_op_bra(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_bra\n");
	return 0;
}

/* BRA : 0000 1011 aaaa aaaa : A-50 */
static size_t dsp56156_op_bra_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* 8 bit immediate, relative offset */
	int8_t branchOffset = (int8_t)BITS(op,0x00ff);

	/* "The PC Contains the address of the next instruction" */
	PC += 1;

	/* Jump */
	PC += branchOffset;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4; /* TODO: + jx oscillator clock cycles */
	return 0;
}

/* BRA : 0000 0001 0010 11RR : A-50 */
static size_t dsp56156_op_bra_2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_bra_2\n");
	return 0;
}

/* BRKcc : 0000 0001 0001 cccc : A-52 */
static size_t dsp56156_op_brkcc(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	int shouldBreak = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldBreak)
	{
		/* TODO: I think this PC = LA thing is off-by-1, but it's working this way because its consistently so */
		PC = LA;

		SR &= 0x3fff;
		SR |= SSL & 0xc000;
		SP--;

		LA = SSH;
		LC = SSL;
		SP--;

		cycles += 2;
		return 0;
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
static size_t dsp56156_op_bscc(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	int shouldBranch = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldBranch)
	{
		/* The PC Contains the address of the next instruction */
		PC += 2;

		/* Push */
		SP++;
		SSH = (PC < 0x80) ? PC : PC; /* Long interrupt gets the previous PC, not the current one */
		SSL = SR;

		/* Change */
		PC = PC + (int16_t)op2;

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
static size_t dsp56156_op_bscc_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_bscc_1\n");
	return 0;
}

/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
static size_t dsp56156_op_bsr(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* The PC Contains the address of the next instruction */
	PC += 2;

	/* Push */
	SP++;
	SSH = (PC < 0x80) ? PC : PC; /* Long interrupt gets the previous PC, not the current one */
	SSL = SR;

	/* Change */
	PC = PC + (int16_t)op2;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;    /* TODO: + jx oscillator clock cycles */
	return 0;
}

/* BSR : 0000 0001 0010 10RR : A-56 */
static size_t dsp56156_op_bsr_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_bsr_1\n");
	return 0;
}

/* CHKAAU : 0000 0000 0000 0100 : A-58 */
static size_t dsp56156_op_chkaau(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - ? ? ? - */
	/* V - Set if the result of the last address ALU update performed a modulo wrap. Cleared if
	       result of the last address ALU did not perform a modulo wrap.*/
	/* Z - Set if the result of the last address ALU update is 0. Cleared if the result of the last
	       address ALU is positive. */
	/* N - Set if the result of the last address ALU update is negative. Cleared if the result of the
	       last address ALU is positive. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_chkaau\n");
	return 0;
}

/* DEBUG : 0000 0000 0000 0001 : A-68 */
static size_t dsp56156_op_debug(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_debug\n");
	return 0;
}

/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
static size_t dsp56156_op_debugcc(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_debugcc\n");
	return 0;
}

/* DIV : 0001 0101 0--0 F1DD : A-76 */
/* WARNING : DOCS SAY THERE IS A PARALLEL MOVE HERE !!! */
static size_t dsp56156_op_div(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_DDF_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S, &D);

	uint64_t dVal = *(uint64_t*)D.addr;
	uint64_t sVal = *(uint16_t*)S.addr;

	const bool quotient_negative = BIT(dVal, 39) != BIT(sVal, 15);

	const uint64_t oldBit39 = BIT(dVal, 39);
	dVal <<= 1;
	const uint64_t newBit39 = BIT(dVal, 39);
	dVal |= C();
	dVal &= 0x000000ffffffffffULL;

	if (quotient_negative)
		dVal += (uint32_t(sVal) << 16) | (BIT(sVal, 15) ? 0xff'00000000ULL : 0ULL);
	else
		dVal -= (uint32_t(sVal) << 16) | (BIT(sVal, 15) ? 0xff'00000000ULL : 0ULL);

	*(uint64_t*)D.addr = dVal & 0x000000ff'ffffffffULL;
	/* S L E U N Z V C */
	/* - * - - - - ? ? */
	/* L - Set if overflow bit V is set. */
	/* V - Set if the most significant bit of the destination operand is changed as a result of the left
	       shift. Cleared otherwise. */
	/* C - Set if bit 39 of the result is cleared. Cleared otherwise. */
	if (newBit39 != oldBit39) DSP56156_V_SET(); else DSP56156_V_CLEAR();
	if (!BIT(dVal, 39))       DSP56156_C_SET(); else DSP56156_C_CLEAR();

	cycles += 2;
	return 1;
}

/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
static size_t dsp56156_op_dmac(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	int64_t result = 0;

	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQF_special_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);

	const uint8_t ss = BITS(op,0x0024);

	/* Fixed-point 2's complement multiplication requires a shift */
	if (ss == 0x00 || ss == 0x01)
	{
		/* Signed * Signed */
		const int64_t s1 = int64_t(*((int16_t*)S1));
		const int32_t s2 = int32_t(*((int16_t*)S2));
		result = (s1 * s2) << 1;
	}
	else if (ss == 0x2)
	{
		/* Signed * Unsigned */
		const int64_t s1 = int64_t(*((int16_t*)S1));
		const int32_t s2 = uint32_t(*((uint16_t*)S2));
		result = (s1 * s2) << 1;
	}
	else if (ss == 0x3)
	{
		/* Unsigned * Unsigned */
		const uint64_t s1 = uint64_t(*((uint16_t*)S1));
		const uint32_t s2 = uint32_t(*((uint16_t*)S2));
		result = (s1 * s2) << 1;
	}

	/* Shift right, then accumulate */

	*((uint64_t*)D) = (uint64_t)((*(int64_t*)D << 24) >> 40);
	*((uint64_t*)D) += result;

	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;
	return 1;
}

/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56156_op_do(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_do\n");
	return 0;
}

/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56156_op_do_1(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	uint8_t retSize = 0;
	const uint8_t iValue = BITS(op,0x00ff);

	/* Don't execute if the loop counter == 0 */
	if (iValue != 0x00)
	{
		/* First instruction cycle */
		SP++;                       /* TODO: Should i really inc here first? */
		SSH = LA;
		SSL = LC;
		LC = (uint16_t)iValue;


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
		PC = PC + 2 + op2;

		cycles += 10;   /* TODO: + mv oscillator cycles */
		retSize = 0;
	}

	return retSize;
}

/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
static size_t dsp56156_op_do_2(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	uint8_t retSize = 0;
	uint16_t lValue = 0x0000;
	typed_pointer S = {nullptr, DT_BYTE};
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &S);

	/* TODO: Does not properly shift-limit sources A&B - Fix per the docs. */
	/* TODO: There are other cases besides A&B this code won't work. */
	if      (S.addr == &A) lValue = *((uint16_t*)(&A1));
	else if (S.addr == &B) lValue = *((uint16_t*)(&B1));
	else                   lValue = *((uint16_t*)S.addr);

	/* TODO: Fix for special cased SP S */
	if (S.addr == &SP)
		LOGMASKED(LOG_UNIMPLEMENTED, "DSP56156: do with SP as the source not properly implemented yet.\n");

	/* TODO: Fix for special cased SSSL S */
	if (S.addr == &SSL)
		LOGMASKED(LOG_UNIMPLEMENTED, "DSP56156: do with SP as the source not properly implemented yet.\n");

	/* Don't execute if the loop counter == 0 */
	if (lValue != 0x00)
	{
		/* First instruction cycle */
		SP++;                       /* TODO: Should i really inc here first? */
		SSH = LA;
		SSL = LC;
		LC = (uint16_t)lValue;


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
		PC = PC + 2 + op2;

		cycles += 10;   /* TODO: + mv oscillator cycles */
		retSize = 0;
	}

	return retSize;
}

/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
static size_t dsp56156_op_doforever(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
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
static size_t dsp56156_op_enddo(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	SR &= 0x3fff;
	SR |= SSL & 0xc000;
	SP--;

	LA = SSH;
	LC = SSL;
	SP--;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 8;
	return 1;
}

/* EXT : 0001 0101 0101 F010 : A-96 */
static size_t dsp56156_op_ext(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_ext\n");
	return 0;
}

/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
static size_t dsp56156_op_illegal(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_illegal\n");
	return 0;
}

/* IMAC : 0001 0101 1010 FQQQ : A-100 */
static size_t dsp56156_op_imac(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQQF_table(cpustate, BITS(op,0x0007), BITS(op,0x0008), &S1, &S2, &D);

	/* Cast both values as being signed */
	const int32_t s1 = *((int16_t*)S1);
	const int32_t s2 = *((int16_t*)S2);

	/* Integral multiply doesn't require the shift */
	int64_t result = (s1 * s2);

	/* Shift result 16 bits to the left before adding to destination */
	result = (result << 16) & 0xffff0000;

	/* Sign extend D into a temp variable */
	int64_t opD = util::sext(*((uint64_t*)D), 40);

	/* Accumulate */
	opD += result;

	/* And out the bits that don't live in the register */
	opD &= 0x000000ff'ffffffffULL;

	(*((uint64_t*)D)) = (uint64_t)opD;

	/* S L E U N Z V C */
	/* - * ? ? * ? ? - */
	/* TODO: L */
	/* U,E - Will not be set correctly by this instruction*/
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffff0000ULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	DSP56156_V_CLEAR();

	cycles += 2;
	return 1;
}

/* IMPY : 0001 0101 1000 FQQQ : A-102 */
static size_t dsp56156_op_impy(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * ? ? * ? ? - */
	/* Z - Set if the 24 most significant bits of the destination result are all zeroes. */
	/* U,E - Will not be set correctly by this instruction*/
	/* V - Set to zero regardless of the overflow */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_impy\n");
	return 0;
}

/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
static size_t dsp56156_op_jcc(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_jcc\n");
	return 0;
}

/* Jcc : 0000 0110 RR10 cccc : A-108 */
static size_t dsp56156_op_jcc_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_jcc_1\n");
	return 0;
}

/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
static size_t dsp56156_op_jmp(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	PC = op2;

	/* S L E U N Z V C */
	/* - - - - - - - - */

	cycles += 4;    /* TODO: + jx */
	return 0;
}

/* JMP : 0000 0001 0010 01RR : A-110 */
static size_t dsp56156_op_jmp_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer R = { nullptr, DT_BYTE };
	decode_RR_table(cpustate, BITS(op,0x0003), &R);

	PC = *((uint16_t*)R.addr);

	/* S L E U N Z V C */
	/* - - - - - - - - */

	cycles += 4;    /* TODO: + jx */
	return 0;
}

/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
static size_t dsp56156_op_jscc(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	const int shouldJump = decode_cccc_table(cpustate, BITS(op,0x000f));

	if (shouldJump)
	{
		/* TODO: It says "signed" absolute offset.  Weird. */
		const uint16_t branchOffset = op2;

		/* TODO: Verify, since it's not in the docs, but it must be true */
		PC += 2;

		/* Push */
		SP++;
		SSH = (PC < 0x80) ? PC : PC; /* Long interrupt gets the previous PC, not the current one */
		SSL = SR;

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
static size_t dsp56156_op_jscc_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_jscc_1\n");
	return 0;
}

/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
static size_t dsp56156_op_jsr(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* TODO: It says "signed" absolute offset.  Weird. */
	const uint16_t branchOffset = op2;

	PC += 2;

	/* Push */
	SP++;
	SSH = (PC < 0x80) ? IPC : PC; /* Long interrupt gets the previous PC, not the current one */
	SSL = SR;
	PC = branchOffset;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;        /* TODO: + jx oscillator cycles */
	return 0;
}

/* JSR : 0000 1010 AAAA AAAA : A-114 */
static size_t dsp56156_op_jsr_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_jsr_1\n");
	return 0;
}

/* JSR : 0000 0001 0010 00RR : A-114 */
static size_t dsp56156_op_jsr_2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_jsr_2\n");
	return 0;
}

/* LEA : 0000 0001 11TT MMRR : A-116 */
static size_t dsp56156_op_lea(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_TT_table(cpustate, BITS(op,0x0030), &D);

	/* TODO: change the execute_mm_functions to return values.  Maybe */
	/* Because this calculation isn't applied, do everything locally */
	/* RR table */
	uint16_t *rX = nullptr;
	uint16_t *nX = nullptr;
	switch(BITS(op,0x0003))
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: rX = &R3;  nX = &N3;  break;
	}

	/* MM table */
	uint16_t ea = 0;
	switch(BITS(op,0x000c))
	{
		case 0x0: ea = *rX;       break;
		case 0x1: ea = *rX + 1;   break;
		case 0x2: ea = *rX - 1;   break;
		case 0x3: ea = *rX + *nX; break;
	}

	*((uint16_t*)D.addr) = ea;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 1;
}

/* LEA : 0000 0001 10NN MMRR : A-116 */
static size_t dsp56156_op_lea_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_lea_1\n");
	return 0;
}

/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
static size_t dsp56156_op_macsuuu(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQF_special_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);

	const uint8_t s = BITS(op,0x0004);

	/* Fixed-point 2's complement multiplication requires a shift */
	int64_t result = 0;
	if (s)
	{
		/* Unsigned * Unsigned */
		const int64_t s1 = uint64_t(*((uint16_t*)S1));
		const int32_t s2 = uint32_t(*((uint16_t*)S2));
		result = (s1 * s2) << 1;
	}
	else
	{
		/* Signed * Unsigned */
		const int64_t s1 = int64_t(*((uint16_t*)S1));
		const int32_t s2 = uint32_t(*((uint16_t*)S2));
		result = (s1 * s2) << 1;
	}

	(*((uint64_t*)D)) += result;

	/* And out the bits that don't live in the register */
	(*((uint64_t*)D)) &= 0x000000ff'ffffffffULL;

	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;
	return 1;
}

/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
static size_t dsp56156_op_move_2(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_move_2\n");
	return 0;
}

/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
static size_t dsp56156_op_movec(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	uint8_t W;
	typed_pointer R = { nullptr, DT_BYTE };
	typed_pointer SD = { nullptr, DT_BYTE };

	W = BITS(op,0x0400);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);
	decode_RR_table(cpustate, BITS(op,0x0003), &R);

	if (W)
	{
		/* Write D */
		uint16_t value = cpustate->data.read_word(*((uint16_t*)R.addr));
		typed_pointer temp_src = { &value, DT_WORD };
		dsp56156_set_destination_value(cpustate, temp_src, SD);
	}
	else
	{
		/* Read S */
		uint16_t dataMemOffset = *((uint16_t*)R.addr);
		dsp56156_set_data_memory_value(cpustate, SD, dataMemOffset);
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
static size_t dsp56156_op_movec_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer SD = {nullptr, DT_BYTE};

	const uint8_t W = BITS(op,0x0400);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);
	const uint16_t memOffset = execute_q_table(cpustate, BITS(op,0x0003), BITS(op,0x0008));

	if (W)
	{
		/* Write D */
		uint16_t tempData = cpustate->data.read_word(memOffset);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };

		/* A & B are special */
		if (SD.data_type == DT_LONG_WORD)
		{
			SD.data_type = DT_WORD;
			SD.addr = &((PAIR64*)SD.addr)->w.h;
		}

		dsp56156_set_destination_value(cpustate, temp_src, SD);
	}
	else
	{
		/* Read S */
		uint16_t tempData = *((uint16_t*)SD.addr);

		/* A & B are special */
		if (SD.data_type == DT_LONG_WORD)
		{
			tempData = (uint16_t)(*((uint64_t*)SD.addr) >> 16);
		}

		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_data_memory_value(cpustate, temp_src, memOffset);
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
static size_t dsp56156_op_movec_2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer SD = {nullptr, DT_BYTE};
	typed_pointer XMemOffset = {nullptr, DT_BYTE};

	const uint8_t W = BITS(op,0x0400);
	decode_Z_table(cpustate, BITS(op,0x0008), &XMemOffset);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);

	const uint16_t memOffset = *((uint16_t*)XMemOffset.addr);

	if (W)
	{
		/* Write D */
		uint16_t tempData = cpustate->data.read_word(memOffset);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_destination_value(cpustate, temp_src, SD);
	}
	else
	{
		/* Read S */
		uint16_t tempData = *((uint16_t*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_data_memory_value(cpustate, temp_src, memOffset);
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
static size_t dsp56156_op_movec_3(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	typed_pointer SD = { nullptr, DT_BYTE };

	const uint8_t W = BITS(op,0x0400);
	const uint8_t t = BITS(op,0x0008);
	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &SD);

	// Useful Polygonet addresses:
	// 0x255: cpustate->ALU.a.w.h contains model to draw, go to 0x628 to skip. Data RAM 0x2F6 contains address into which model spans are drawn
	// 0x61F: R1 contains span-store pointer which is being stored
	// 0x554: 0x2F6 contains span-store pointer which is being loaded

	if (W)
	{
		/* Write D */
		if (t)
		{
			/* 16-bit long data */
			typed_pointer temp_src = { (void*)&op2, DT_WORD };
			dsp56156_set_destination_value(cpustate, temp_src, SD);
		}
		else
		{
			/* 16-bit long address */
			uint16_t tempD = cpustate->data.read_word(op2);
			typed_pointer tempTP = {&tempD, DT_WORD};
			dsp56156_set_destination_value(cpustate, tempTP, SD);
		}
	}
	else
	{
		/* Read S */
		if (t)
		{
			/* 16-bit long data */
			LOGMASKED(LOG_UNIMPLEMENTED, "DSP56156: Potentially nonexistent movec_3 at %04x", PC);
		}
		else
		{
			/* 16-bit long address */
			dsp56156_set_data_memory_value(cpustate, SD, op2);
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
static size_t dsp56156_op_movec_4(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_DDDDD_table(cpustate, BITS(op,0x03e0), &S);
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &D);

	const bool both_accum = (S.data_type == DT_LONG_WORD && D.data_type == DT_LONG_WORD);

	if (S.data_type == DT_LONG_WORD && D.data_type == DT_WORD)
	{
		S.data_type = DT_WORD;
		if (S.addr == &A)
			S.addr = &A1;
		else if (S.addr == &B)
			S.addr = &B1;
		dsp56156_set_destination_value(cpustate, S, D);
	}
	else if (both_accum)
	{
		typed_pointer tempTP;
		uint64_t temp_val = dsp56156_limit_long_to_long(cpustate, *(uint64_t*)S.addr);
		tempTP.addr = &temp_val;
		tempTP.data_type = DT_LONG_WORD;
		dsp56156_set_destination_value(cpustate, tempTP, D);
	}
	else
	{
		dsp56156_set_destination_value(cpustate, S, D);
	}

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
static size_t dsp56156_op_movec_5(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	typed_pointer SD = { nullptr, DT_BYTE };

	const int8_t xx = int8_t(op & 0x00ff);
	const uint8_t W = BITS(op2,0x0400);
	decode_DDDDD_table(cpustate, BITS(op2,0x03e0), &SD);

	const uint16_t memOffset = R2 + (int16_t)xx;

	if (W)
	{
		/* Write D */
		uint16_t tempData = cpustate->data.read_word(memOffset);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_destination_value(cpustate, temp_src, SD);
	}
	else
	{
		/* Read S */
		uint16_t tempData = *((uint16_t*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_data_memory_value(cpustate, temp_src, memOffset);
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
static size_t dsp56156_op_movei(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	typed_pointer immTP = {nullptr, DT_BYTE};

	/* Typecasting to int16_t sign-extends the BBBBBBBB operand */
	uint16_t immediateSignExtended = int16_t(op & 0x00ff);
	immTP.addr = &immediateSignExtended;
	immTP.data_type = DT_WORD;

	decode_DD_table(cpustate, BITS(op,0x0300), &D);

	dsp56156_set_destination_value(cpustate, immTP, D);

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
static size_t dsp56156_op_movem(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer R = { nullptr, DT_BYTE };
	typed_pointer SD = { nullptr, DT_BYTE };

	const uint8_t W = BITS(op,0x0100);
	decode_RR_table(cpustate, BITS(op,0x00c0), &R);
	decode_HHH_table(cpustate, BITS(op,0x0007), &SD);

	if (W)
	{
		/* Read from Program Memory */
		typed_pointer data;
		uint16_t ldata = cpustate->program.read_word(*((uint16_t*)R.addr));

		data.addr = &ldata;
		data.data_type = DT_WORD;
		dsp56156_set_destination_value(cpustate, data, SD);
	}
	else
	{
		/* Write to Program Memory */
		dsp56156_set_program_memory_value(cpustate, SD, *((uint16_t*)R.addr));
	}

	execute_MM_table(cpustate, BITS(op,0x00c0), BITS(op,0x0018));

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */
	cycles += 2;    /* TODO: + mvm oscillator clock cycles */
	return 1;
}

/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
static size_t dsp56156_op_movem_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_movem_1\n");
	return 0;
}

/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
static size_t dsp56156_op_movem_2(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_movem_2\n");
	return 0;
}

/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
static size_t dsp56156_op_movep(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer SD = {nullptr, DT_BYTE};

	decode_HH_table(cpustate, BITS(op,0x00c0), &SD);
	/* TODO: Special cases for A & B */

	const uint16_t pp = assemble_address_from_IO_short_address(cpustate, op & 0x001f);

	const uint16_t W = BITS(op,0x0100);

	if (W)
	{
		uint16_t data = cpustate->data.read_word(pp);

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		dsp56156_set_destination_value(cpustate, tempTP, SD);
	}
	else
	{
		dsp56156_set_data_memory_value(cpustate, SD, pp);
	}

	/* S L E U N Z V C */
	/* * * - - - - - - */
	/* TODO: S, L */

	cycles += 4;        /* TODO: + mvp oscillator cycles */
	return 1;
}

/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
static size_t dsp56156_op_movep_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* X:<Rx> and X:<pp> */

	typed_pointer SD = {nullptr, DT_BYTE};
	decode_RR_table(cpustate, BITS(op,0x00c0), &SD);

	const uint16_t pp = assemble_address_from_IO_short_address(cpustate, op & 0x001f);

	const uint16_t W = BITS(op,0x0100);

	/* A little different than most W if's - opposite read and write */
	if (W)
	{
		uint16_t data = cpustate->data.read_word(*((uint16_t*)SD.addr));

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		dsp56156_set_data_memory_value(cpustate, tempTP, pp);
	}
	else
	{
		/* TODO */
		fatalerror("dsp56156 : move(p) NOTHING HERE (yet)\n");
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
static size_t dsp56156_op_moves(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_moves\n");
	return 0;
}

/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
static size_t dsp56156_op_mpysuuu(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	int64_t result = 0;

	void* D = nullptr;
	void* S1 = nullptr;
	void* S2 = nullptr;

	decode_QQF_special_table(cpustate, BITS(op,0x0003), BITS(op,0x0008), &S1, &S2, &D);

	const uint8_t s = BITS(op,0x0004);

	/* Fixed-point 2's complement multiplication requires a shift */
	if (s)
	{
		/* Unsigned * Unsigned */
		const int64_t s1 = uint64_t(*((uint16_t*)S1));
		const int32_t s2 = uint32_t(*((uint16_t*)S2));
		result = (s1 * s2) << 1;
	}
	else
	{
		/* Signed * Unsigned */
		const int64_t s1 = int64_t(*((int16_t*)S1));
		const int32_t s2 = uint32_t(*((uint16_t*)S2));
		result = (s1 * s2) << 1;
	}

	(*((uint64_t*)D)) = result;

	/* And out the bits that don't live in the register */
	(*((uint64_t*)D)) &= 0x000000ff'ffffffffULL;

	/* S L E U N Z V C */
	/* - * * * * * * - */
	/* TODO: L, E, U, V */
	if ( *((uint64_t*)D) & 0x00000080'00000000ULL)       DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint64_t*)D) & 0x000000ff'ffffffffULL) == 0) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();

	cycles += 2;
	return 1;
}

/* NEGC : 0001 0101 0110 F000 : A-168 */
static size_t dsp56156_op_negc(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * * */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_negc\n");
	return 0;
}

/* NOP : 0000 0000 0000 0000 : A-170 */
static size_t dsp56156_op_nop(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	return 1;
}

/* NORM : 0001 0101 0010 F0RR : A-172 */
static size_t dsp56156_op_norm(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * ? - */
	/* V - Set if an arithmetic overflow occurs in the 40 bit result. Also set if the most significantst
	       bit of the destination operand is changed as a result of the left shift. Cleared otherwise. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_norm\n");
	return 0;
}

/* ORI : 0001 1EE1 iiii iiii : A-178 */
static size_t dsp56156_op_ori(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - ? ? ? ? ? ? ? */
	/* All ? bits - Set if the corresponding bit in the immediate data is set and if the operand is the
	   CCR. Not affected otherwise. */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_ori\n");
	return 0;
}

/* REP : 0000 0000 111- --RR : A-180 */
static size_t dsp56156_op_rep(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_rep\n");
	return 0;
}

/* REP : 0000 1111 iiii iiii : A-180 */
static size_t dsp56156_op_rep_1(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* TODO: This is non-interruptible, probably have to turn off interrupts here */
	const uint16_t iVal = op & 0x00ff;

	if (iVal != 0)
	{
		TEMP = LC;
		LC = iVal;

		cpustate->repFlag = 1;
		cpustate->repAddr = PC + 2;

		cycles += 4;        /* TODO: + mv oscillator clock cycles */
	}
	else
	{
		const uint16_t op1 = ROPCODE(PC + 1);
		const uint16_t op2 = ROPCODE(PC + 2);
		cycles += 6;        /* TODO: + mv oscillator clock cycles */
		return 1 + dsp56156_get_op_size(op1, op2);
	}


	/* S L E U N Z V C */
	/* - * - - - - - - */
	/* TODO: L */
	return 1;
}

/* REP : 0000 0100 001D DDDD : A-180 */
static size_t dsp56156_op_rep_2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* TODO: This is non-interruptible, probably have to turn off interrupts here */
	typed_pointer D = {nullptr, DT_BYTE};
	decode_DDDDD_table(cpustate, BITS(op,0x001f), &D);

	/* TODO: handle special A&B source cases */
	if (D.addr == &A || D.addr == &B)
		LOGMASKED(LOG_UNIMPLEMENTED, "DSP56156 ERROR: rep_2 with A or B instruction not implemented yet at %04x\n", PC);

	const uint16_t repValue = *((uint16_t*)D.addr);

	if (repValue != 0)
	{
		TEMP = LC;
		LC = repValue;

		cpustate->repFlag = 1;
		cpustate->repAddr = PC + 2;

		cycles += 4;        /* TODO: + mv oscillator clock cycles */
	}
	else
	{
		const uint16_t op1 = ROPCODE(PC + 1);
		const uint16_t op2 = ROPCODE(PC + 2);
		cycles += 6;        /* TODO: + mv oscillator clock cycles */
		return 1 + dsp56156_get_op_size(op1, op2);
	}

	/* S L E U N Z V C */
	/* - * - - - - - - */
	/* TODO: L */
	return 1;
}

/* REPcc : 0000 0001 0101 cccc : A-184 */
static size_t dsp56156_op_repcc(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_repcc\n");
	return 0;
}

/* RESET : 0000 0000 0000 1000 : A-186 */
static size_t dsp56156_op_reset(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_reset\n");
	return 0;
}

/* RTI : 0000 0000 0000 0111 : A-194 */
static size_t dsp56156_op_rti(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* WARNING : THERE SHOULD BE A MORE GENERAL HANDLING OF STACK ERRORS. */
	if (SP == 0)
	{
		dsp56156_add_pending_interrupt(cpustate, "Stack Error");
		return 0;
	}

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
static size_t dsp56156_op_rts(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* Pop */
	PC = SSH;

	/* SR = SSL; The status register is not affected. */

	SP--;

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 4;    /* TODO: + rx oscillator clock cycles */
	return 0;
}

/* STOP : 0000 0000 0000 1010 : A-200 */
static size_t dsp56156_op_stop(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_stop\n");
	return 0;
}

/* SWAP : 0001 0101 0111 F001 : A-206 */
static size_t dsp56156_op_swap(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_swap\n");
	return 0;
}

/* SWI : 0000 0000 0000 0101 : A-208 */
static size_t dsp56156_op_swi(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_swi\n");
	return 0;
}

/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
static size_t dsp56156_op_tcc(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	const int shouldTransfer = decode_cccc_table(cpustate, BITS(op,0x03c0));

	if (shouldTransfer)
	{
		typed_pointer S = {nullptr, DT_BYTE};
		typed_pointer D = {nullptr, DT_BYTE};
		typed_pointer S2 = {&R0, DT_WORD};
		typed_pointer D2 = {nullptr, DT_BYTE};

		decode_h0hF_table(cpustate, BITS(op,0x0007),BITS(op,0x0008), &S, &D);
		dsp56156_set_destination_value(cpustate, S, D);

		/* TODO: What's up with that A,A* thing in the docs?  Can you only ignore the R0->RX transfer if you do an A,A? */
		decode_RR_table(cpustate, BITS(op,0x0030), &D2); /* TT is the same as RR */
		dsp56156_set_destination_value(cpustate, S2, D2);
	}

	/* S L E U N Z V C */
	/* - - - - - - - - */
	cycles += 2;
	return 1;
}

/* TFR(2) : 0001 0101 0000 F00J : A-214 */
static size_t dsp56156_op_tfr2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_JF_table(cpustate, BITS(op,0x0001), BITS(op,0x0008), &S, &D);

	dsp56156_set_destination_value(cpustate, S, D);

	/* S L E U N Z V C */
	/* - * - - - - - - */
	/* TODO: L */
	cycles += 2;
	return 1;
}

/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
static size_t dsp56156_op_tfr3(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* * * - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_tfr3\n");
	return 0;
}

/* TST(2) : 0001 0101 0001 -1DD : A-220 */
static size_t dsp56156_op_tst2(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	typed_pointer D = {nullptr, DT_BYTE};
	decode_DD_table(cpustate, BITS(op,0x0003), &D);

	/* S L E U N Z V C */
	/* - * * * * * 0 0 */
	/* (L,E,U should be set to 0) */
	DSP56156_L_CLEAR();
	DSP56156_E_CLEAR();
	/* U_CLEAR(); */ /* TODO: Conflicting opinions?  "Set if unnormalized."  Documentation is weird (A&B?) */
	if ((*((uint16_t*)D.addr)) &  0x8000) DSP56156_N_SET(); else DSP56156_N_CLEAR();
	if ((*((uint16_t*)D.addr)) == 0x0000) DSP56156_Z_SET(); else DSP56156_Z_CLEAR();
	/* DSP56156_V_CLEAR(); */ /* Unaffected */
	DSP56156_C_CLEAR();

	cycles += 2;
	return 1;
}

/* WAIT : 0000 0000 0000 1011 : A-222 */
static size_t dsp56156_op_wait(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - - - - - - - - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_wait\n");
	return 0;
}

/* ZERO : 0001 0101 0101 F000 : A-224 */
static size_t dsp56156_op_zero(dsp56156_core* cpustate, const uint16_t op, uint8_t* cycles)
{
	/* S L E U N Z V C */
	/* - * * * * * * - */
	LOGMASKED(LOG_UNIMPLEMENTED, "dsp56156_op_zero\n");
	return 0;
}



/***************************************************************************
    Table decoding
***************************************************************************/
static uint16_t decode_BBB_bitmask(dsp56156_core* cpustate, uint16_t BBB, uint16_t *iVal)
{
	uint16_t retVal = 0x0000;

	switch(BBB)
	{
		case 0x4: retVal = 0xff00;  *iVal <<= 8;  break;
		case 0x2: retVal = 0x0ff0;  *iVal <<= 4;  break;
		case 0x1: retVal = 0x00ff;  *iVal <<= 0;  break;
	}

	return retVal;
}

static int decode_cccc_table(dsp56156_core* cpustate, uint16_t cccc)
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
		case 0xc: if((Z() || ((!U()) && (!E()))))       retVal = 1;  break;  /* nr */
		case 0x3: if( N() == 0)                         retVal = 1;  break;  /* pl */
		case 0x4: if(!(Z() || ((!U()) && (!E()))))      retVal = 1;  break;  /* nn */
	}

	return retVal;
}

static void decode_DDDDD_table(dsp56156_core* cpustate, uint16_t DDDDD, typed_pointer* ret)
{
	switch(DDDDD)
	{
		case 0x00: ret->addr = &X0;  ret->data_type = DT_WORD;       break;
		case 0x01: ret->addr = &Y0;  ret->data_type = DT_WORD;       break;
		case 0x02: ret->addr = &X1;  ret->data_type = DT_WORD;       break;
		case 0x03: ret->addr = &Y1;  ret->data_type = DT_WORD;       break;
		case 0x04: ret->addr = &A;   ret->data_type = DT_LONG_WORD;  break;
		case 0x05: ret->addr = &B;   ret->data_type = DT_LONG_WORD;  break;
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

static void decode_DD_table(dsp56156_core* cpustate, uint16_t DD, typed_pointer* ret)
{
	switch(DD)
	{
		case 0x00: ret->addr = &X0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &Y0;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &X1;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &Y1;  ret->data_type = DT_WORD;  break;
	}
}

static void decode_DDF_table(dsp56156_core* cpustate, uint16_t DD, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	uint16_t switchVal = (DD << 1) | F;

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

static void decode_F_table(dsp56156_core* cpustate, uint16_t F, typed_pointer* ret)
{
	switch(F)
	{
		case 0x0: ret->addr = &A;  ret->data_type = DT_LONG_WORD;  break;
		case 0x1: ret->addr = &B;  ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_h0hF_table(dsp56156_core* cpustate, uint16_t h0h, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	uint16_t switchVal = (h0h << 1) | F;

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

static void decode_HH_table(dsp56156_core* cpustate, uint16_t HH, typed_pointer* ret)
{
	switch(HH)
	{
		case 0x0: ret->addr = &X0;  ret->data_type = DT_WORD;       break;
		case 0x1: ret->addr = &Y0;  ret->data_type = DT_WORD;       break;
		case 0x2: ret->addr = &A;   ret->data_type = DT_LONG_WORD;  break;
		case 0x3: ret->addr = &B;   ret->data_type = DT_LONG_WORD;  break;
	}
}

static void decode_HHH_table(dsp56156_core* cpustate, uint16_t HHH, typed_pointer* ret)
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

static void decode_IIII_table(dsp56156_core* cpustate, uint16_t IIII, typed_pointer* src_ret, typed_pointer* dst_ret, void *working)
{
	void *opposite = nullptr;

	if (working == &A) opposite = &B;
	else               opposite = &A;

	switch(IIII)
	{
		case 0x0: src_ret->addr = &X0;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x1: src_ret->addr = &Y0;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x2: src_ret->addr = &X1;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x3: src_ret->addr = &Y1;      src_ret->data_type = DT_WORD;       dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		case 0x4: src_ret->addr = &A1;      src_ret->data_type = DT_WORD;       dst_ret->addr = &X0;       dst_ret->data_type = DT_WORD;       break;
		case 0x5: src_ret->addr = &B1;      src_ret->data_type = DT_WORD;       dst_ret->addr = &Y0;       dst_ret->data_type = DT_WORD;       break;
		case 0x6: src_ret->addr = &A0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &X0;       dst_ret->data_type = DT_WORD;       break;
		case 0x7: src_ret->addr = &B0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &Y0;       dst_ret->data_type = DT_WORD;       break;
		case 0x8:
		case 0x9: src_ret->addr = working;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;  break;
		//case 0x8:
		//case 0x9: src_ret->addr = &((PAIR64*)working)->w.h;  src_ret->data_type = DT_WORD;        dst_ret->addr = opposite;  dst_ret->data_type = DT_LONG_WORD;       break;
		case 0xc: src_ret->addr = &A1;      src_ret->data_type = DT_WORD;       dst_ret->addr = &X1;       dst_ret->data_type = DT_WORD;       break;
		case 0xd: src_ret->addr = &B1;      src_ret->data_type = DT_WORD;       dst_ret->addr = &Y1;       dst_ret->data_type = DT_WORD;       break;
		case 0xe: src_ret->addr = &A0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &X1;       dst_ret->data_type = DT_WORD;       break;
		case 0xf: src_ret->addr = &B0;      src_ret->data_type = DT_WORD;       dst_ret->addr = &Y1;       dst_ret->data_type = DT_WORD;       break;
	}
}

static void decode_JJJF_table(dsp56156_core* cpustate, uint16_t JJJ, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	uint16_t switchVal = (JJJ << 1) | F;

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

static void decode_JJF_table(dsp56156_core* cpustate, uint16_t JJ, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	uint16_t switchVal = (JJ << 1) | F;

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

static void decode_JF_table(dsp56156_core* cpustate, uint16_t J, uint16_t F, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	uint16_t switchVal = (J << 1) | F;

	switch (switchVal)
	{
		case 0x0: src_ret->addr = &A;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &X;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
		case 0x1: src_ret->addr = &B;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &X;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
		case 0x2: src_ret->addr = &A;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &Y;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
		case 0x3: src_ret->addr = &B;  src_ret->data_type = DT_LONG_WORD;  dst_ret->addr = &Y;  dst_ret->data_type = DT_DOUBLE_WORD;  break;
	}
}

static void decode_KKK_table(dsp56156_core* cpustate, uint16_t KKK, typed_pointer* dst_ret1, typed_pointer* dst_ret2, void* working)
{
	void *opposite = nullptr;

	if (working == &A) opposite = &B;
	else               opposite = &A;

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

static void decode_QQF_table(dsp56156_core* cpustate, uint16_t QQ, uint16_t F, void **S1, void **S2, void **D)
{
	uint16_t switchVal = (QQ << 1) | F;

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

static void decode_QQF_special_table(dsp56156_core* cpustate, uint16_t QQ, uint16_t F, void **S1, void **S2, void **D)
{
	uint16_t switchVal = (QQ << 1) | F;

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

static void decode_QQQF_table(dsp56156_core* cpustate, uint16_t QQQ, uint16_t F, void **S1, void **S2, void **D)
{
	uint16_t switchVal = (QQQ << 1) | F;

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

static void decode_RR_table(dsp56156_core* cpustate, uint16_t RR, typed_pointer* ret)
{
	switch(RR)
	{
		case 0x00: ret->addr = &R0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &R1;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &R2;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &R3;  ret->data_type = DT_WORD;  break;
	}
}

static void decode_TT_table(dsp56156_core* cpustate, uint16_t TT, typed_pointer* ret)
{
	switch(TT)
	{
		case 0x00: ret->addr = &R0;  ret->data_type = DT_WORD;  break;
		case 0x01: ret->addr = &R1;  ret->data_type = DT_WORD;  break;
		case 0x02: ret->addr = &R2;  ret->data_type = DT_WORD;  break;
		case 0x03: ret->addr = &R3;  ret->data_type = DT_WORD;  break;
	}
}


static void decode_uuuuF_table(dsp56156_core* cpustate, uint16_t uuuu, uint16_t F, uint8_t add_sub_other, typed_pointer* src_ret, typed_pointer* dst_ret)
{
	uint16_t switchVal = (uuuu << 1) | F;

	/* Unknown uuuuFs have been seen in the wild */
	add_sub_other = OP_OTHER;
	src_ret->addr = nullptr; src_ret->data_type = DT_BYTE;
	dst_ret->addr = nullptr; dst_ret->data_type = DT_BYTE;

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

static void decode_Z_table(dsp56156_core* cpustate, uint16_t Z, typed_pointer* ret)
{
	switch(Z)
	{
		/* Fixed as per the Family Manual addendum */
		case 0x01: ret->addr = &A1;  ret->data_type = DT_WORD;  break;
		case 0x00: ret->addr = &B1;  ret->data_type = DT_WORD;  break;
	}
}

static void execute_m_table(dsp56156_core* cpustate, int x, uint16_t m)
{
	uint16_t *rX = nullptr;
	uint16_t *nX = nullptr;

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

static void execute_mm_table(dsp56156_core* cpustate, uint16_t rnum, uint16_t mm)
{
	uint16_t *rX = nullptr;
	uint16_t *nX = nullptr;

	switch(rnum)
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: fatalerror("Dsp56156: Error. execute_mm_table specified R3 as its first source!\n");  break;
	}

	switch(mm)
	{
		case 0x0: (*rX)++;                  R3++;           break;
		case 0x1: (*rX)++;                  R3 = R3 + N3;   break;
		case 0x2: (*rX) = (*rX) + (*nX);    R3++;           break;
		case 0x3: (*rX) = (*rX) + (*nX);    R3 = R3 + N3;   break;
	}
}

static void execute_MM_table(dsp56156_core* cpustate, uint16_t rnum, uint16_t MM)
{
	uint16_t *rX = nullptr;
	uint16_t *nX = nullptr;

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
		case 0x1: (*rX)++;              break;
		case 0x2: (*rX)--;              break;
		case 0x3: (*rX) = (*rX)+(*nX);  break;
	}
}

/* Returns R value */
static uint16_t execute_q_table(dsp56156_core* cpustate, int RR, uint16_t q)
{
	uint16_t *rX = nullptr;
	uint16_t *nX = nullptr;

	switch(RR)
	{
		case 0x0: rX = &R0;  nX = &N0;  break;
		case 0x1: rX = &R1;  nX = &N1;  break;
		case 0x2: rX = &R2;  nX = &N2;  break;
		case 0x3: rX = &R3;  nX = &N3;  break;
	}

	switch(q)
	{
		case 0x0: /* No permanent changes */; return (*rX)+(*nX);
		case 0x1: (*rX)--;                    return (*rX);    /* This one is special - it's a *PRE-decrement*! */
	}

	/* Should not get here */
	fatalerror("dsp56156: execute_q_table did something impossible!\n");
	return 0;
}

static void execute_z_table(dsp56156_core* cpustate, int RR, uint16_t z)
{
	uint16_t *rX = nullptr;
	uint16_t *nX = nullptr;

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

static uint16_t assemble_address_from_Pppppp_table(dsp56156_core* cpustate, uint16_t P, uint16_t ppppp)
{
	uint16_t destAddr = 0x00;

	switch (P)
	{
		case 0x0: destAddr = ppppp;  break;     /* TODO:  Does this really only address up to 0x32? */
		case 0x1: destAddr = assemble_address_from_IO_short_address(cpustate, ppppp);  break;
	}

	return destAddr;
}

static uint16_t assemble_address_from_IO_short_address(dsp56156_core* cpustate, uint16_t pp)
{
	uint16_t fullAddy = 0xffe0;
	fullAddy |= pp;
	return fullAddy;
}

static void dsp56156_process_loop(dsp56156_core* cpustate)
{
	/* TODO: This might not work for dos nested in doForevers */
	if (LF_bit(cpustate) && FV_bit(cpustate))
	{
		/* Do Forever*/
		if (PC == LA)
		{
			LC--;

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

static void dsp56156_process_rep(dsp56156_core* cpustate, size_t repSize)
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
    Flag Utilities
***************************************************************************/

static bool dsp56156_value_is_unnormalized(dsp56156_core* cpustate, const uint64_t value)
{
	uint16_t S = (cpustate->PCU.sr >> 10) & 3;
	if (S == 0 || S == 3)
		return BIT(value, 31) == BIT(value, 30);
	else if (S == 1)
		return BIT(value, 32) == BIT(value, 31);
	else
		return BIT(value, 30) == BIT(value, 29);
}

static bool dsp56156_value_is_extended(dsp56156_core* cpustate, const uint64_t value)
{
	uint16_t S = (cpustate->PCU.sr >> 10) & 3;
	uint64_t extMask = 0x000000ffc0000000ULL;
	if (S == 0 || S == 3)
		extMask = 0x000000ff80000000ULL;
	else if (S == 1)
		extMask = 0x000000ff00000000ULL;
	uint64_t extVal = value & extMask;
	return extVal == extMask || extVal == 0;
}


/***************************************************************************
    Parallel Memory Ops
***************************************************************************/
/* Register to Register Data Move : 0100 IIII .... .... : A-132 */
static void execute_register_to_register_data_move(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register, uint64_t* prev_accum_value)
{
	typed_pointer S = {nullptr, DT_BYTE};
	typed_pointer D = {nullptr, DT_BYTE};

	decode_IIII_table(cpustate, BITS(op,0x0f00), &S, &D, d_register->addr);

	const bool opdest_is_a = d_register->addr == &A0 || d_register->addr == &A1;
	const bool opdest_is_b = d_register->addr == &B0 || d_register->addr == &B1;
	const bool s_is_a = S.addr == &A0 || S.addr == &A1;
	const bool s_is_b = S.addr == &B0 || S.addr == &B1;

	/* If the source is the same as the ALU destination, use the previous accumulator value */
	if ((opdest_is_a && s_is_a) || (opdest_is_b && s_is_b))
	{
		typed_pointer tempTP;
		uint64_t temp_long = 0ULL;
		uint16_t temp_word = 0U;
		if (S.addr == &A1 || S.addr == &B1)
		{
			temp_word = (uint16_t)(*prev_accum_value >> 16);
			tempTP.addr = &temp_word;
			tempTP.data_type = DT_WORD;
		}
		else if (S.data_type == DT_LONG_WORD)
		{
			temp_long = dsp56156_limit_long_to_long(cpustate, *prev_accum_value);
			tempTP.addr = &temp_long;
			tempTP.data_type = DT_LONG_WORD;
		}
		else
		{
			temp_word = (uint16_t)*prev_accum_value;
			tempTP.addr = &temp_word;
			tempTP.data_type = DT_WORD;
		}
		dsp56156_set_destination_value(cpustate, tempTP, D);
	}
	else
	{
		dsp56156_set_destination_value(cpustate, S, D);
	}
}

/* Address Register Update : 0011 0zRR .... .... : A-135 */
static void execute_address_register_update(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register, uint64_t* prev_accum_value)
{
	execute_z_table(cpustate, BITS(op,0x0300), BITS(op,0x0400));
}

/* X Memory Data Move : 1mRR HHHW .... .... : A-137 */
static void execute_x_memory_data_move(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register, uint64_t* prev_accum_value)
{
	typed_pointer R = {nullptr, DT_BYTE};
	typed_pointer SD = {nullptr, DT_BYTE};

	const uint16_t W = BITS(op,0x0100);
	decode_HHH_table(cpustate, BITS(op,0x0e00), &SD);
	decode_RR_table(cpustate, BITS(op,0x3000),&R);

	if (W)
	{
		/* From X:<ea> to SD */
		uint16_t data = cpustate->data.read_word(*((uint16_t*)R.addr));

		typed_pointer tempTP;
		tempTP.addr = &data;
		tempTP.data_type = DT_WORD;

		dsp56156_set_destination_value(cpustate, tempTP, SD);
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

			dsp56156_set_data_memory_value(cpustate, tempTP, *((uint16_t*)R.addr));
		}
		else
		{
			dsp56156_set_data_memory_value(cpustate, SD, *((uint16_t*)R.addr));
		}
	}

	execute_m_table(cpustate, BITS(op,0x3000), BITS(op,0x4000));
}

/* X Memory Data Move : 0101 HHHW .... .... : A-137 */
/* NOTE: previous accumulator value is not needed since ^F1 is always the opposite accumulator */
static void execute_x_memory_data_move2(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register)
{
	typed_pointer SD = {nullptr, DT_BYTE};

	const uint16_t W = BITS(op,0x0100);
	decode_HHH_table(cpustate, BITS(op,0x0e000), &SD);

	uint16_t *const mem_offset = (d_register->addr == &A) ? &B1 : &A1;

	if (W)
	{
		/* Write D */
		uint16_t value = cpustate->data.read_word(*mem_offset);
		typed_pointer tempV = {&value, DT_WORD};
		dsp56156_set_destination_value(cpustate, tempV, SD);
	}
	else
	{
		/* Read S */
		dsp56156_set_data_memory_value(cpustate, SD, *mem_offset);
	}
}

/* X Memory Data Move With Short Displacement : 0000 0101 BBBB BBBB ---- HHHW .... .... : A-139 */
static void execute_x_memory_data_move_with_short_displacement(dsp56156_core* cpustate, const uint16_t op, const uint16_t op2)
{
	typed_pointer SD = { nullptr, DT_BYTE };

	const int8_t xx = int8_t(op & 0x00ff);
	const uint8_t W = BITS(op2,0x0100);
	decode_HHH_table(cpustate, BITS(op2,0x0e00), &SD);

	const uint16_t memOffset = R2 + (int16_t)xx;

	if (W)
	{
		/* Write D */
		uint16_t tempData = cpustate->data.read_word(memOffset);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_destination_value(cpustate, temp_src, SD);
	}
	else
	{
		/* Read S */
		uint16_t tempData = *((uint16_t*)SD.addr);
		typed_pointer temp_src = { (void*)&tempData, DT_WORD };
		dsp56156_set_data_memory_value(cpustate, temp_src, memOffset);
	}
}

/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142*/
static void execute_dual_x_memory_data_read(dsp56156_core* cpustate, const uint16_t op, typed_pointer* d_register)
{
	typed_pointer tempV;
	typed_pointer R = {nullptr, DT_BYTE};
	typed_pointer D1 = {nullptr, DT_BYTE};
	typed_pointer D2 = {nullptr, DT_BYTE};

	decode_RR_table(cpustate, BITS(op,0x0060), &R);
	decode_KKK_table(cpustate, BITS(op,0x0700), &D1, &D2, d_register->addr);

	/* Can't do an R3 for S1 */
	if (R.addr == &R3)
		fatalerror("Dsp56156: Error. Dual x memory data read specified R3 as its first source!\n");

	/* The note on A-142 is very interesting.
	   You can effectively access external memory in the last 64 bytes of X data memory! */
	if (*((uint16_t*)R.addr) >= 0xffc0 || R3 >= 0xffc0)
	{
		cpustate->device->machine().debug_break();
		LOGMASKED(LOG_UNIMPLEMENTED, "Dsp56156: Unimplemented access to external X Data Memory >= 0xffc0 in Dual X Memory Data Read at %04x.\n", cpustate->PCU.pc);
	}

	/* First memmove */
	uint16_t srcVal1 = cpustate->data.read_word(*((uint16_t*)R.addr));
	tempV.addr = &srcVal1;
	tempV.data_type = DT_WORD;
	dsp56156_set_destination_value(cpustate, tempV, D1);

	/* Second memmove */
	uint16_t srcVal2 = cpustate->data.read_word(R3);
	tempV.addr = &srcVal2;
	tempV.data_type = DT_WORD;
	dsp56156_set_destination_value(cpustate, tempV, D2);

	/* Touch up the R regs after all the moves */
	execute_mm_table(cpustate, BITS(op,0x0060), BITS(op,0x1800));
}

/***************************************************************************
    Helper Functions
***************************************************************************/
static uint16_t dsp56156_op_mask(uint16_t cur, uint16_t mask)
{
	uint16_t retVal = (cur & mask);
	uint16_t temp = 0x0000;
	int offsetCount = 0;

	/* Shift everything right, eliminating 'whitespace' */
	for (int i = 0; i < 16; i++)
	{
		if (BIT(mask, i))        /* If mask bit is non-zero */
		{
			temp |= (BIT(retVal, i) << offsetCount);
			offsetCount++;
		}
	}

	return temp;
}

static uint64_t dsp56156_limit_long_to_long(dsp56156_core* /*cpustate*/, uint64_t val)
{
	int32_t signed_upper = (int32_t)(val >> 8) >> 8;
	if (signed_upper < -32768)
		return 0x000000ff'80000000ULL;
	else if (signed_upper > 32767)
		return 0x00000000'7fff0000ULL;
	return val & 0x000000ff'ffff0000ULL;
}

static void dsp56156_set_destination_value(dsp56156_core* cpustate, typed_pointer source, typed_pointer dest)
{
	uint64_t destinationValue = 0;

	switch(dest.data_type)
	{
		/* Copying to an 8-bit value */
		case DT_BYTE:
			switch(source.data_type)
			{
				/* From a ? */
				case DT_BYTE:        *((uint8_t*)dest.addr) = (*((uint8_t*) source.addr)) & 0xff; break;
				case DT_WORD:        *((uint8_t*)dest.addr) = (*((uint16_t*)source.addr)) & 0x00ff; break;
				case DT_DOUBLE_WORD: *((uint8_t*)dest.addr) = (*((uint32_t*)source.addr)) & 0x000000ff; break;
				case DT_LONG_WORD:   *((uint8_t*)dest.addr) = (*((uint64_t*)source.addr)) & 0x00000000000000ffULL; break;
			}
			break;

		/* Copying to a 16-bit value */
		case DT_WORD:
			switch(source.data_type)
			{
				case DT_BYTE:        *((uint16_t*)dest.addr) = (*((uint8_t*) source.addr)) & 0xff; break;
				case DT_WORD:        *((uint16_t*)dest.addr) = (*((uint16_t*)source.addr)) & 0xffff; break;
				case DT_DOUBLE_WORD: *((uint16_t*)dest.addr) = (*((uint32_t*)source.addr)) & 0x0000ffff; break;
				case DT_LONG_WORD:   *((uint16_t*)dest.addr) = (*((uint64_t*)source.addr)) & 0x000000000000ffffULL;
					/* TODO: Shift limiter action! A-147 */
					if (dest.addr == &A || dest.addr == &B)
					{
						LOGMASKED(LOG_CHECKS, "%04x: Double-check dsp56156_set_destination_value, writing A/B to a word.\n", cpustate->PCU.pc);
					}
					break;
			}
			break;

		/* Copying to a 32-bit value */
		case DT_DOUBLE_WORD:
			switch(source.data_type)
			{
				case DT_BYTE:        *((uint32_t*)dest.addr) = (*((uint8_t*) source.addr)) & 0xff; break;
				case DT_WORD:        *((uint32_t*)dest.addr) = (*((uint16_t*)source.addr)) & 0xffff; break;
				case DT_DOUBLE_WORD: *((uint32_t*)dest.addr) = (*((uint32_t*)source.addr)) & 0xffffffff; break;
				case DT_LONG_WORD:   *((uint32_t*)dest.addr) = (*((uint64_t*)source.addr)) & 0x00000000'ffffffffULL; break;
			}
			break;

		/* Copying to a 64-bit value */
		case DT_LONG_WORD:
			switch(source.data_type)
			{
				case DT_BYTE:        *((uint64_t*)dest.addr) = (*((uint8_t*)source.addr)) & 0xff; break;

				case DT_WORD:        destinationValue = (*((uint16_t*)source.addr)) << 16;
										if (destinationValue & 0x00000000'80000000ULL)
											destinationValue |= 0x000000ff'00000000ULL;
										*((uint64_t*)dest.addr) = (uint64_t)destinationValue; break;    /* Forget not, yon shift register */

				case DT_DOUBLE_WORD: *((uint64_t*)dest.addr) = (*((uint32_t*)source.addr)) & 0xffffffff; break;
				case DT_LONG_WORD:   *((uint64_t*)dest.addr) = (*((uint64_t*)source.addr)) & 0x000000ff'ffffffffULL; break;
			}
			break;
	}
}

/* TODO: Wait-state timings! */
static void dsp56156_set_data_memory_value(dsp56156_core* cpustate, typed_pointer source, uint32_t destinationAddr)
{
	switch(source.data_type)
	{
		case DT_BYTE:        cpustate->data.write_word(destinationAddr, (uint16_t)( (*((uint8_t*) source.addr) & 0xff)));       break;
		case DT_WORD:        cpustate->data.write_word(destinationAddr, (uint16_t)( (*((uint16_t*)source.addr) & 0xffff)));     break;
		case DT_DOUBLE_WORD: cpustate->data.write_word(destinationAddr, (uint16_t)( (*((uint32_t*)source.addr) & 0x0000ffff))); break;

		/* !!! Is this universal ??? */
		/* !!! Forget not, yon shift-limiter !!! */
		case DT_LONG_WORD:   cpustate->data.write_word(destinationAddr, (uint16_t)(((*((uint64_t*)source.addr)) & 0x00000000'ffff0000ULL) >> 16)); break;
	}
}

/* TODO: Wait-state timings! */
static void dsp56156_set_program_memory_value(dsp56156_core* cpustate, typed_pointer source, uint32_t destinationAddr)
{
	switch(source.data_type)
	{
		case DT_BYTE:        cpustate->program.write_word(destinationAddr, (uint16_t)( (*((uint8_t*) source.addr) & 0xff)));        break;
		case DT_WORD:        cpustate->program.write_word(destinationAddr, (uint16_t)( (*((uint16_t*)source.addr) & 0xffff)));      break;
		case DT_DOUBLE_WORD: cpustate->program.write_word(destinationAddr, (uint16_t)( (*((uint32_t*)source.addr) & 0x0000ffff)));  break;

		/* !!! Is this universal ??? */
		/* !!! Forget not, yon shift-limiter !!! */
		case DT_LONG_WORD:   cpustate->program.write_word(destinationAddr, (uint16_t)(((*((uint64_t*)source.addr)) & 0x00000000'ffff0000ULL) >> 16)); break;
	}
}

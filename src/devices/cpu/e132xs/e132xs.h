// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_E132XS_E132XS_H
#define MAME_CPU_E132XS_E132XS_H

#pragma once


/*
    A note about clock multipliers and dividers:

    E1-16[T] and E1-32[T] accept a straight clock

    E1-16X[T|N] and E1-32X[T|N] accept a clock and multiply it
        internally by 4; in the emulator, you MUST specify 4 * XTAL
        to achieve the correct speed

    E1-16XS[R] and E1-32XS[R] accept a clock and multiply it
        internally by 8; in the emulator, you MUST specify 8 * XTAL
        to achieve the correct speed
*/



/* Functions */

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

extern unsigned dasm_hyperstone(std::ostream &stream, unsigned pc, const uint8_t *oprom, unsigned h_flag, int private_fp);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> hyperstone_device

// Used by core CPU interface
class hyperstone_device : public cpu_device
{
protected:
	enum
	{
		E132XS_PC = 1,
		E132XS_SR,
		E132XS_FER,
		E132XS_G3,
		E132XS_G4,
		E132XS_G5,
		E132XS_G6,
		E132XS_G7,
		E132XS_G8,
		E132XS_G9,
		E132XS_G10,
		E132XS_G11,
		E132XS_G12,
		E132XS_G13,
		E132XS_G14,
		E132XS_G15,
		E132XS_G16,
		E132XS_G17,
		E132XS_SP,
		E132XS_UB,
		E132XS_BCR,
		E132XS_TPR,
		E132XS_TCR,
		E132XS_TR,
		E132XS_WCR,
		E132XS_ISR,
		E132XS_FCR,
		E132XS_MCR,
		E132XS_G28,
		E132XS_G29,
		E132XS_G30,
		E132XS_G31,
		E132XS_CL0, E132XS_CL1, E132XS_CL2, E132XS_CL3,
		E132XS_CL4, E132XS_CL5, E132XS_CL6, E132XS_CL7,
		E132XS_CL8, E132XS_CL9, E132XS_CL10,E132XS_CL11,
		E132XS_CL12,E132XS_CL13,E132XS_CL14,E132XS_CL15,
		E132XS_L0,  E132XS_L1,  E132XS_L2,  E132XS_L3,
		E132XS_L4,  E132XS_L5,  E132XS_L6,  E132XS_L7,
		E132XS_L8,  E132XS_L9,  E132XS_L10, E132XS_L11,
		E132XS_L12, E132XS_L13, E132XS_L14, E132XS_L15,
		E132XS_L16, E132XS_L17, E132XS_L18, E132XS_L19,
		E132XS_L20, E132XS_L21, E132XS_L22, E132XS_L23,
		E132XS_L24, E132XS_L25, E132XS_L26, E132XS_L27,
		E132XS_L28, E132XS_L29, E132XS_L30, E132XS_L31,
		E132XS_L32, E132XS_L33, E132XS_L34, E132XS_L35,
		E132XS_L36, E132XS_L37, E132XS_L38, E132XS_L39,
		E132XS_L40, E132XS_L41, E132XS_L42, E132XS_L43,
		E132XS_L44, E132XS_L45, E132XS_L46, E132XS_L47,
		E132XS_L48, E132XS_L49, E132XS_L50, E132XS_L51,
		E132XS_L52, E132XS_L53, E132XS_L54, E132XS_L55,
		E132XS_L56, E132XS_L57, E132XS_L58, E132XS_L59,
		E132XS_L60, E132XS_L61, E132XS_L62, E132XS_L63
	};

	enum reg_bank
	{
		LOCAL = 0,
		GLOBAL = 1
	};

	enum imm_size
	{
		SIMM = 0,
		LIMM = 1
	};

	enum shift_type
	{
		N_LO = 0,
		N_HI = 1
	};

	enum sign_mode
	{
		IS_UNSIGNED = 0,
		IS_SIGNED = 1
	};

	// construction/destruction
	hyperstone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
						const device_type type, uint32_t prg_data_width, uint32_t io_data_width, address_map_constructor internal_map);

	void init(int scale_mask);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override;
	virtual uint32_t disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_io_config;
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;

	// CPU registers
	uint32_t  m_global_regs[32];
	uint32_t  m_local_regs[64];

	/* internal stuff */
	uint16_t  m_op;           // opcode
	uint32_t  m_trap_entry;   // entry point to get trap address

	uint8_t   m_clock_scale_mask;
	uint8_t   m_clck_scale;
	uint8_t   m_clock_cycles_1;
	uint8_t   m_clock_cycles_2;
	uint8_t   m_clock_cycles_3;
	uint8_t   m_clock_cycles_4;
	uint8_t   m_clock_cycles_6;

	uint64_t  m_tr_base_cycles;
	uint32_t  m_tr_base_value;
	uint32_t  m_tr_clocks_per_tick;
	uint8_t   m_timer_int_pending;
	emu_timer *m_timer;

	uint32_t m_delay_pc;
	bool m_delay_slot;

	uint32_t m_opcodexor;

	int32_t m_instruction_length;
	int32_t m_intblock;

	// other internal state
	int     m_icount;

	uint8_t m_fl_lut[16];

private:
	// internal functions
	void check_interrupts();

	void set_global_register(uint8_t code, uint32_t val);
	void set_local_register(uint8_t code, uint32_t val);

	uint32_t get_global_register(uint8_t code);

	uint32_t get_trap_addr(uint8_t trapno);
	uint32_t get_emu_code_addr(uint8_t num);
	void hyperstone_set_trap_entry(int which);
	uint32_t compute_tr();
	void update_timer_prescale();
	void adjust_timer_interrupt();

	TIMER_CALLBACK_MEMBER(timer_callback);

	void execute_br();
	void execute_trap(uint32_t addr);
	void execute_int(uint32_t addr);
	void execute_exception(uint32_t addr);
	void execute_software();

	void ignore_immediate_s();
	uint32_t decode_immediate_s();
	uint32_t decode_const();

	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_chk();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_movd();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL, sign_mode SIGNED> void hyperstone_divsu();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_xm();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_mask();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_sum();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_sums();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_cmp();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_mov();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_add();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_adds();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_cmpb();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_subc();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_sub();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_subs();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_addc();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_neg();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_negs();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_and();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_andn();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_or();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_xor();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_not();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_cmpi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_movi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_addi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_addsi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_cmpbi();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_andni();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_ori();
	template <reg_bank DST_GLOBAL, imm_size IMM_LONG> void hyperstone_xori();
	template <shift_type HI_N> void hyperstone_shrdi();
	void hyperstone_shrd();
	void hyperstone_shr();
	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_shri();
	template <shift_type HI_N> void hyperstone_sardi();
	void hyperstone_sard();
	void hyperstone_sar();
	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_sari();
	template <shift_type HI_N> void hyperstone_shldi();
	void hyperstone_shld();
	void hyperstone_shl();
	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_shli();
	void hyperstone_testlz();
	void hyperstone_rol();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_ldxx1();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_ldxx2();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_stxx1();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_stxx2();

	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL, sign_mode SIGNED> void hyperstone_mulsu();
	template <reg_bank DST_GLOBAL, reg_bank SRC_GLOBAL> void hyperstone_mul();

	template <shift_type HI_N, reg_bank DST_GLOBAL> void hyperstone_set();

	template <reg_bank SRC_GLOBAL> void hyperstone_ldwr();
	template <reg_bank SRC_GLOBAL> void hyperstone_lddr();
	template <reg_bank SRC_GLOBAL> void hypesrtone_ldwp();
	template <reg_bank SRC_GLOBAL> void hyperstone_lddp();

	template <reg_bank SRC_GLOBAL> void hyperstone_stwr();
	template <reg_bank SRC_GLOBAL> void hyperstone_stdr();
	template <reg_bank SRC_GLOBAL> void hyperstone_stwp();
	template <reg_bank SRC_GLOBAL> void hyperstone_stdp();

	void hyperstone_dbv();
	void hyperstone_dbnv();
	void hyperstone_dbe();
	void hyperstone_dbne();
	void hyperstone_dbc();
	void hyperstone_dbnc();
	void hyperstone_dbse();
	void hyperstone_dbht();
	void hyperstone_dbn();
	void hyperstone_dbnn();
	void hyperstone_dble();
	void hyperstone_dbgt();
	void hyperstone_dbr();

	void hyperstone_frame();
	void hyperstone_call_global();
	void hyperstone_call_local();

	void hyperstone_bv();
	void hyperstone_bnv();
	void hyperstone_be();
	void hyperstone_bne();
	void hyperstone_bc();
	void hyperstone_bnc();
	void hyperstone_bse();
	void hyperstone_bht();
	void hyperstone_bn();
	void hyperstone_bnn();
	void hyperstone_ble();
	void hyperstone_bgt();

	void hyperstone_trap();
	void hyperstone_extend();

	void hyperstone_reserved();
	void hyperstone_do();

	int32_t decode_pcrel();
	void ignore_pcrel();

#if 0
	void execute_run_drc();
	void flush_drc_cache();
	void code_flush_cache();
	void code_compile_block(offs_t pc);
	inline void ccfunc_unimplemented();
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(int size, int iswrite, const char *name, code_handle *&handleptr);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, bool allow_exception);
	void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	bool generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
#endif

#if 0
	void generate_op00(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op01(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op02(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op03(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op04(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op05(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op06(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op07(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op08(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op09(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op0a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op0b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op0c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op0d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op0e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op0f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op10(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op11(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op12(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op13(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op14(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op15(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op16(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op17(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op18(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op19(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op1a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op1b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op1c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op1d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op1e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op1f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op20(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op21(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op22(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op23(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op24(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op25(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op26(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op27(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op28(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op29(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op2a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op2b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op2c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op2d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op2e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op2f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op30(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op31(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op32(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op33(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op34(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op35(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op36(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op37(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op38(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op39(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op3a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op3b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op3c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op3d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op3e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op3f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op40(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op41(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op42(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op43(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op44(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op45(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op46(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op47(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op48(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op49(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op4a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op4b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op4c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op4d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op4e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op4f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op50(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op51(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op52(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op53(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op54(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op55(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op56(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op57(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op58(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op59(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op5a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op5b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op5c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op5d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op5e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op5f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op60(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op61(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op62(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op63(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op64(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op65(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op66(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op67(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op68(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op69(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op6a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op6b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op6c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op6d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op6e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op6f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op70(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op71(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op72(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op73(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op74(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op75(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op76(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op77(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op78(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op79(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op7a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op7b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op7c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op7d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op7e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op7f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op80(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op81(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op82(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op83(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op84(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op85(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op86(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op87(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op88(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op89(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op8a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op8b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op8c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op8d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op8e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op8f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op90(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op91(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op92(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op93(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op94(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op95(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op96(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op97(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op98(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op99(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op9a(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op9b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op9c(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op9d(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_op9e(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_op9f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opa0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opa1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opa2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opa3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opa4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opa5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opa6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opa7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opa8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opa9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opaa(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opab(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opac(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opad(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opae(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opaf(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opb0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opb1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opb2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opb3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opb4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opb5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opb6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opb7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opb8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opb9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opba(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opbb(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opbc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opbd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opbe(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opbf(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opc0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opc1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opc2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opc3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opc4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opc5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opc6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opc7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opc8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opc9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opca(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opcb(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opcc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opcd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opce(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opcf(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opd0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opd1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opd2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opd3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opd4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opd5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opd6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opd7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opd8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opd9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opda(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opdb(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opdc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opdd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opde(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opdf(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_ope0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_ope1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_ope2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_ope3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_ope4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_ope5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_ope6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_ope7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_ope8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_ope9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opea(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opeb(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opec(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_oped(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opee(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opef(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opf0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opf1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opf2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opf3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opf4(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opf5(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opf6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opf7(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opf8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opf9(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opfa(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opfb(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opfc(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opfd(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opfe(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc); void generate_opff(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(E116T,      e116t_device)
DECLARE_DEVICE_TYPE(E116XT,     e116xt_device)
DECLARE_DEVICE_TYPE(E116XS,     e116xs_device)
DECLARE_DEVICE_TYPE(E116XSR,    e116xsr_device)
DECLARE_DEVICE_TYPE(E132N,      e132n_device)
DECLARE_DEVICE_TYPE(E132T,      e132t_device)
DECLARE_DEVICE_TYPE(E132XN,     e132xn_device)
DECLARE_DEVICE_TYPE(E132XT,     e132xt_device)
DECLARE_DEVICE_TYPE(E132XS,     e132xs_device)
DECLARE_DEVICE_TYPE(E132XSR,    e132xsr_device)
DECLARE_DEVICE_TYPE(GMS30C2116, gms30c2116_device)
DECLARE_DEVICE_TYPE(GMS30C2132, gms30c2132_device)
DECLARE_DEVICE_TYPE(GMS30C2216, gms30c2216_device)
DECLARE_DEVICE_TYPE(GMS30C2232, gms30c2232_device)


// ======================> e116t_device

class e116t_device : public hyperstone_device
{
public:
	// construction/destruction
	e116t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e116xt_device

class e116xt_device : public hyperstone_device
{
public:
	// construction/destruction
	e116xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e116xs_device

class e116xs_device : public hyperstone_device
{
public:
	// construction/destruction
	e116xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e116xsr_device

class e116xsr_device : public hyperstone_device
{
public:
	// construction/destruction
	e116xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e132n_device

class e132n_device : public hyperstone_device
{
public:
	// construction/destruction
	e132n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e132t_device

class e132t_device : public hyperstone_device
{
public:
	// construction/destruction
	e132t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e132xn_device

class e132xn_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e132xt_device

class e132xt_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e132xs_device

class e132xs_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> e132xsr_device

class e132xsr_device : public hyperstone_device
{
public:
	// construction/destruction
	e132xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> gms30c2116_device

class gms30c2116_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> gms30c2132_device

class gms30c2132_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> gms30c2216_device

class gms30c2216_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2216_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};


// ======================> gms30c2232_device

class gms30c2232_device : public hyperstone_device
{
public:
	// construction/destruction
	gms30c2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
};

#endif // MAME_CPU_E132XS_E132XS_H

// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
#ifndef MAME_CPU_E132XS_E132XS_H
#define MAME_CPU_E132XS_E132XS_H

#pragma once

#include "32xsdasm.h"

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



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class e132xs_frontend;

// ======================> hyperstone_device

// Used by core CPU interface
class hyperstone_device : public cpu_device, public hyperstone_disassembler::config
{
	friend class e132xs_frontend;

public:
	inline void ccfunc_unimplemented();
	inline void ccfunc_print();
	inline void ccfunc_total_cycles();
	void update_timer_prescale();
	void compute_tr();
	void adjust_timer_interrupt();

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

	enum branch_condition
	{
		COND_V = 0,
		COND_Z = 1,
		COND_C = 2,
		COND_CZ = 3,
		COND_N = 4,
		COND_NZ = 5
	};

	enum condition_set
	{
		IS_CLEAR = 0,
		IS_SET = 1
	};

	enum
	{
		EXCEPTION_IO2                  = 48,
		EXCEPTION_IO1                  = 49,
		EXCEPTION_INT4                 = 50,
		EXCEPTION_INT3                 = 51,
		EXCEPTION_INT2                 = 52,
		EXCEPTION_INT1                 = 53,
		EXCEPTION_IO3                  = 54,
		EXCEPTION_TIMER                = 55,
		EXCEPTION_RESERVED1            = 56,
		EXCEPTION_TRACE                = 57,
		EXCEPTION_PARITY_ERROR         = 58,
		EXCEPTION_EXTENDED_OVERFLOW    = 59,
		EXCEPTION_RANGE_ERROR          = 60,
		EXCEPTION_PRIVILEGE_ERROR      = EXCEPTION_RANGE_ERROR,
		EXCEPTION_FRAME_ERROR          = EXCEPTION_RANGE_ERROR,
		EXCEPTION_RESERVED2            = 61,
		EXCEPTION_RESET                = 62,  // reserved if not mapped @ MEM3
		EXCEPTION_ERROR_ENTRY          = 63,  // for instruction code of all ones
		EXCEPTION_COUNT
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
	virtual util::disasm_interface *create_disassembler() override;
	virtual u8 get_fp() const override;
	virtual bool get_h() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_io_config;
	address_space *m_program;
	direct_read_data<0> *m_direct;
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
	uint32_t  m_tr_result;
	uint32_t  m_tr_clocks_per_tick;
	uint8_t   m_timer_int_pending;
	emu_timer *m_timer;

	uint64_t m_numcycles;

	uint32_t m_prev_pc;
	uint32_t m_delay_pc;
	uint32_t m_delay_slot;

	uint32_t m_opcodexor;

	int32_t m_instruction_length;
	int32_t m_intblock;

	// other internal state
	int     m_icount;

	uint8_t m_fl_lut[16];
	static const uint32_t s_trap_entries[8];
	static const int32_t s_immediate_values[16];

	uint32_t get_trap_addr(uint8_t trapno);

private:
	// internal functions
	void check_interrupts();

	void set_global_register(uint8_t code, uint32_t val);
	void set_local_register(uint8_t code, uint32_t val);

	uint32_t get_global_register(uint8_t code);

	uint32_t get_emu_code_addr(uint8_t num);

	TIMER_CALLBACK_MEMBER(timer_callback);

	uint32_t decode_const();
	uint32_t decode_immediate_s();
	void ignore_immediate_s();
	int32_t decode_pcrel();
	void ignore_pcrel();

	void hyperstone_br();
	void execute_trap(uint32_t addr);
	void execute_int(uint32_t addr);
	void execute_exception(uint32_t addr);
	void execute_software();

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
	template <reg_bank SRC_GLOBAL> void hyperstone_ldwp();
	template <reg_bank SRC_GLOBAL> void hyperstone_lddp();

	template <reg_bank SRC_GLOBAL> void hyperstone_stwr();
	template <reg_bank SRC_GLOBAL> void hyperstone_stdr();
	template <reg_bank SRC_GLOBAL> void hyperstone_stwp();
	template <reg_bank SRC_GLOBAL> void hyperstone_stdp();

	template <branch_condition CONDITION, condition_set COND_SET> void hyperstone_b();
	template <branch_condition CONDITION, condition_set COND_SET> void hyperstone_db();
	void hyperstone_dbr();

	void hyperstone_frame();
	template <hyperstone_device::reg_bank SRC_GLOBAL> void hyperstone_call();

	void hyperstone_trap();
	void hyperstone_extend();

	void hyperstone_reserved();
	void hyperstone_do();
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

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
	virtual std::vector<std::pair<int, const address_space_config *>> memory_space_config() const override;

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

	/* Delay information */
	struct delay_info
	{
		int32_t   delay_cmd;
		uint32_t  delay_pc;
	};

	// CPU registers
	uint32_t  m_global_regs[32];
	uint32_t  m_local_regs[64];

	/* internal stuff */
	uint32_t  m_ppc;          // previous pc
	uint16_t  m_op;           // opcode
	uint32_t  m_trap_entry;   // entry point to get trap address

	uint8_t   m_clock_scale_mask;
	uint8_t   m_clck_scale;
	uint8_t   m_clock_cycles_1;
	uint8_t   m_clock_cycles_2;
	uint8_t   m_clock_cycles_4;
	uint8_t   m_clock_cycles_6;

	uint64_t  m_tr_base_cycles;
	uint32_t  m_tr_base_value;
	uint32_t  m_tr_clocks_per_tick;
	uint8_t   m_timer_int_pending;
	emu_timer *m_timer;

	delay_info m_delay;

	uint32_t m_opcodexor;

	int32_t m_instruction_length;
	int32_t m_intblock;

	// other internal state
	int     m_icount;

	typedef void (hyperstone_device::*ophandler)();

	ophandler m_opcode[256];

	static const ophandler s_opcodetable[256];

private:
	struct regs_decode
	{
		uint8_t   src, dst;       // destination and source register code
		uint32_t  src_value;      // current source register value
		uint32_t  next_src_value; // current next source register value
		uint32_t  dst_value;      // current destination register value
		uint32_t  next_dst_value; // current next destination register value
		uint8_t   sub_type;       // sub type opcode (for DD and X_CODE bits)
		union
		{
			uint32_t u;
			int32_t  s;
		} extra;                // extra value such as immediate value, const, pcrel, ...
		uint8_t   src_is_local;
		uint8_t   dst_is_local;
		uint8_t   same_src_dst;
		uint8_t   same_src_dstf;
		uint8_t   same_srcf_dst;
	};

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

	void execute_br(struct regs_decode *decode);
	void execute_dbr(struct regs_decode *decode);
	void execute_trap(uint32_t addr);
	void execute_int(uint32_t addr);
	void execute_exception(uint32_t addr);
	void execute_software(struct regs_decode *decode);

	void hyperstone_chk(struct regs_decode *decode);
	void hyperstone_movd(struct regs_decode *decode);
	void hyperstone_divu(struct regs_decode *decode);
	void hyperstone_divs(struct regs_decode *decode);
	void hyperstone_xm(struct regs_decode *decode);
	void hyperstone_mask(struct regs_decode *decode);
	void hyperstone_sum(struct regs_decode *decode);
	void hyperstone_sums(struct regs_decode *decode);
	void hyperstone_cmp(struct regs_decode *decode);
	void hyperstone_mov(struct regs_decode *decode);
	void hyperstone_add(struct regs_decode *decode);
	void hyperstone_adds(struct regs_decode *decode);
	void hyperstone_cmpb(struct regs_decode *decode);
	void hyperstone_andn(struct regs_decode *decode);
	void hyperstone_or(struct regs_decode *decode);
	void hyperstone_xor(struct regs_decode *decode);
	void hyperstone_subc(struct regs_decode *decode);
	void hyperstone_not(struct regs_decode *decode);
	void hyperstone_sub(struct regs_decode *decode);
	void hyperstone_subs(struct regs_decode *decode);
	void hyperstone_addc(struct regs_decode *decode);
	void hyperstone_and(struct regs_decode *decode);
	void hyperstone_neg(struct regs_decode *decode);
	void hyperstone_negs(struct regs_decode *decode);
	void hyperstone_cmpi(struct regs_decode *decode);
	void hyperstone_movi(struct regs_decode *decode);
	void hyperstone_addi(struct regs_decode *decode);
	void hyperstone_addsi(struct regs_decode *decode);
	void hyperstone_cmpbi(struct regs_decode *decode);
	void hyperstone_andni(struct regs_decode *decode);
	void hyperstone_ori(struct regs_decode *decode);
	void hyperstone_xori(struct regs_decode *decode);
	void hyperstone_shrdi(struct regs_decode *decode);
	void hyperstone_shrd(struct regs_decode *decode);
	void hyperstone_shr(struct regs_decode *decode);
	void hyperstone_shri(struct regs_decode *decode);
	void hyperstone_sardi(struct regs_decode *decode);
	void hyperstone_sard(struct regs_decode *decode);
	void hyperstone_sar(struct regs_decode *decode);
	void hyperstone_sari(struct regs_decode *decode);
	void hyperstone_shldi(struct regs_decode *decode);
	void hyperstone_shld(struct regs_decode *decode);
	void hyperstone_shl(struct regs_decode *decode);
	void hyperstone_shli(struct regs_decode *decode);
	void hyperstone_testlz(struct regs_decode *decode);
	void hyperstone_rol(struct regs_decode *decode);
	void hyperstone_ldxx1(struct regs_decode *decode);
	void hyperstone_ldxx2(struct regs_decode *decode);
	void hyperstone_stxx1(struct regs_decode *decode);
	void hyperstone_stxx2(struct regs_decode *decode);
	void hyperstone_mulu(struct regs_decode *decode);
	void hyperstone_muls(struct regs_decode *decode);
	void hyperstone_mul(struct regs_decode *decode);
	void hyperstone_set(struct regs_decode *decode);

	void hyperstone_fadd(struct regs_decode *decode);
	void hyperstone_faddd(struct regs_decode *decode);
	void hyperstone_fsub(struct regs_decode *decode);
	void hyperstone_fsubd(struct regs_decode *decode);
	void hyperstone_fmul(struct regs_decode *decode);
	void hyperstone_fmuld(struct regs_decode *decode);
	void hyperstone_fdiv(struct regs_decode *decode);
	void hyperstone_fdivd(struct regs_decode *decode);

	void hyperstone_fcmp(struct regs_decode *decode);
	void hyperstone_fcmpd(struct regs_decode *decode);
	void hyperstone_fcmpu(struct regs_decode *decode);
	void hyperstone_fcmpud(struct regs_decode *decode);

	void hyperstone_fcvt(struct regs_decode *decode);
	void hyperstone_fcvtd(struct regs_decode *decode);

	void hyperstone_extend(struct regs_decode *decode);

	void hyperstone_ldwr(struct regs_decode *decode);
	void hyperstone_lddr(struct regs_decode *decode);
	void hyperstone_ldwp(struct regs_decode *decode);
	void hyperstone_lddp(struct regs_decode *decode);

	void hyperstone_stwr(struct regs_decode *decode);
	void hyperstone_stdr(struct regs_decode *decode);
	void hyperstone_stwp(struct regs_decode *decode);
	void hyperstone_stdp(struct regs_decode *decode);

	void hyperstone_dbv(struct regs_decode *decode);
	void hyperstone_dbnv(struct regs_decode *decode);
	void hyperstone_dbe(struct regs_decode *decode);
	void hyperstone_dbne(struct regs_decode *decode);
	void hyperstone_dbc(struct regs_decode *decode);
	void hyperstone_dbnc(struct regs_decode *decode);
	void hyperstone_dbse(struct regs_decode *decode);
	void hyperstone_dbht(struct regs_decode *decode);
	void hyperstone_dbn(struct regs_decode *decode);
	void hyperstone_dbnn(struct regs_decode *decode);
	void hyperstone_dble(struct regs_decode *decode);
	void hyperstone_dbgt(struct regs_decode *decode);
	void hyperstone_dbr(struct regs_decode *decode);

	void hyperstone_frame(struct regs_decode *decode);
	void hyperstone_call(struct regs_decode *decode);

	void hyperstone_bv(struct regs_decode *decode);
	void hyperstone_bnv(struct regs_decode *decode);
	void hyperstone_be(struct regs_decode *decode);
	void hyperstone_bne(struct regs_decode *decode);
	void hyperstone_bc(struct regs_decode *decode);
	void hyperstone_bnc(struct regs_decode *decode);
	void hyperstone_bse(struct regs_decode *decode);
	void hyperstone_bht(struct regs_decode *decode);
	void hyperstone_bn(struct regs_decode *decode);
	void hyperstone_bnn(struct regs_decode *decode);
	void hyperstone_ble(struct regs_decode *decode);
	void hyperstone_bgt(struct regs_decode *decode);
	void hyperstone_br(struct regs_decode *decode);

	void hyperstone_trap(struct regs_decode *decode);
	void hyperstone_do(struct regs_decode *decode);

	void reserved(struct regs_decode *decode);

	void op00();    void op01();    void op02();    void op03();    void op04();    void op05();    void op06();    void op07();
	void op08();    void op09();    void op0a();    void op0b();    void op0c();    void op0d();    void op0e();    void op0f();
	void op10();    void op11();    void op12();    void op13();    void op14();    void op15();    void op16();    void op17();
	void op18();    void op19();    void op1a();    void op1b();    void op1c();    void op1d();    void op1e();    void op1f();
	void op20();    void op21();    void op22();    void op23();    void op24();    void op25();    void op26();    void op27();
	void op28();    void op29();    void op2a();    void op2b();    void op2c();    void op2d();    void op2e();    void op2f();
	void op30();    void op31();    void op32();    void op33();    void op34();    void op35();    void op36();    void op37();
	void op38();    void op39();    void op3a();    void op3b();    void op3c();    void op3d();    void op3e();    void op3f();
	void op40();    void op41();    void op42();    void op43();    void op44();    void op45();    void op46();    void op47();
	void op48();    void op49();    void op4a();    void op4b();    void op4c();    void op4d();    void op4e();    void op4f();
	void op50();    void op51();    void op52();    void op53();    void op54();    void op55();    void op56();    void op57();
	void op58();    void op59();    void op5a();    void op5b();    void op5c();    void op5d();    void op5e();    void op5f();
	void op60();    void op61();    void op62();    void op63();    void op64();    void op65();    void op66();    void op67();
	void op68();    void op69();    void op6a();    void op6b();    void op6c();    void op6d();    void op6e();    void op6f();
	void op70();    void op71();    void op72();    void op73();    void op74();    void op75();    void op76();    void op77();
	void op78();    void op79();    void op7a();    void op7b();    void op7c();    void op7d();    void op7e();    void op7f();
	void op80();    void op81();    void op82();    void op83();    void op84();    void op85();    void op86();    void op87();
	void op88();    void op89();    void op8a();    void op8b();    void op8c();    void op8d();    void op8e();    void op8f();
	void op90();    void op91();    void op92();    void op93();    void op94();    void op95();    void op96();    void op97();
	void op98();    void op99();    void op9a();    void op9b();    void op9c();    void op9d();    void op9e();    void op9f();
	void opa0();    void opa1();    void opa2();    void opa3();    void opa4();    void opa5();    void opa6();    void opa7();
	void opa8();    void opa9();    void opaa();    void opab();    void opac();    void opad();    void opae();    void opaf();
	void opb0();    void opb1();    void opb2();    void opb3();    void opb4();    void opb5();    void opb6();    void opb7();
	void opb8();    void opb9();    void opba();    void opbb();    void opbc();    void opbd();    void opbe();    void opbf();
	void opc0();    void opc1();    void opc2();    void opc3();    void opc4();    void opc5();    void opc6();    void opc7();
	void opc8();    void opc9();    void opca();    void opcb();    void opcc();    void opcd();    void opce();    void opcf();
	void opd0();    void opd1();    void opd2();    void opd3();    void opd4();    void opd5();    void opd6();    void opd7();
	void opd8();    void opd9();    void opda();    void opdb();    void opdc();    void opdd();    void opde();    void opdf();
	void ope0();    void ope1();    void ope2();    void ope3();    void ope4();    void ope5();    void ope6();    void ope7();
	void ope8();    void ope9();    void opea();    void opeb();    void opec();    void oped();    void opee();    void opef();
	void opf0();    void opf1();    void opf2();    void opf3();    void opf4();    void opf5();    void opf6();    void opf7();
	void opf8();    void opf9();    void opfa();    void opfb();    void opfc();    void opfd();    void opfe();    void opff();
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

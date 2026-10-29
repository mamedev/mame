// license:BSD-3-Clause
// copyright-holders:AJR, David Haywood

#ifndef MAME_CPU_XTENSA_XTENSA_H
#define MAME_CPU_XTENSA_XTENSA_H

#pragma once


class xtensa_device : public cpu_device
{
public:
	xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		AS_EXTREGS = AS_OPCODES + 1,
	};

	enum {
		XTENSA_PC,
		XTENSA_PS,
		XTENSA_WINDOW,
		XTENSA_WINDOWSTART,
		XTENSA_SAR,
		XTENSA_INTENABLE,
		XTENSA_INTERRUPT,
		XTENSA_CCOUNT,
		XTENSA_CCOMPARE0,
		XTENSA_EXCCAUSE,
		XTENSA_EPC1, XTENSA_EPC2, XTENSA_EPC3, XTENSA_EPC4, XTENSA_EPC5, XTENSA_EPC6, XTENSA_EPC7,
		XTENSA_LOOPBEGIN,
		XTENSA_LOOPEND,
		XTENSA_LOOPCOUNT,
		XTENSA_A0, XTENSA_A1, XTENSA_A2, XTENSA_A3,
		XTENSA_A4, XTENSA_A5, XTENSA_A6, XTENSA_A7,
		XTENSA_A8, XTENSA_A9, XTENSA_A10, XTENSA_A11,
		XTENSA_A12, XTENSA_A13, XTENSA_A14, XTENSA_A15,
	};

	// interrupt types (the INTTYPE configuration parameter)
	enum irq_type : u8
	{
		IRQ_LEVEL = 0,      // level-sensitive pin, cleared at the source
		IRQ_EDGE,           // edge-triggered pin, cleared by WSR.INTCLEAR
		IRQ_SOFTWARE,       // set by WSR.INTSET, cleared by WSR.INTCLEAR
		IRQ_TIMER,          // set when CCOUNT = CCOMPAREn, cleared by WSR.CCOMPAREn
		IRQ_WRITE_ERROR     // bus error on write, cleared by WSR.INTCLEAR
	};

	// configuration, fixed when a core is synthesised
	void set_startupvector(u32 vector) { m_startupvector = vector; }
	void set_kernel_exception_vector(u32 vector) { m_kernel_exception_vector = vector; }
	void set_user_exception_vector(u32 vector) { m_user_exception_vector = vector; }
	void set_double_exception_vector(u32 vector) { m_double_exception_vector = vector; }
	void set_interrupt_vector(int level, u32 vector) { m_interrupt_vector[level & 7] = vector; } // levels 2..7
	void set_window_vector_base(u32 base) { m_window_vector_base = base; m_window_vectors_set = true; } // six vectors 0x40 apart
	void set_num_aregs(int n) { m_num_physical_regs = n; } // 32 or 64
	void set_irq_level(int irq, int level) { m_irq_level[irq & 31] = level; } // 1..7
	void set_irq_type(int irq, irq_type type) { m_irq_type[irq & 31] = type; }
	void set_timer_interrupt(int timer, int irq) { m_timerint[timer % 3] = irq; } // CCOMPAREn -> interrupt number
	void set_excm_level(int level) { m_excmlevel = level; } // highest level masked by PS.EXCM
	void set_xea1(bool xea1) { m_xea1 = xea1; } // Exception Architecture 1: no PS.EXCM

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// PS register fields
	static constexpr u32 PS_INTLEVEL_MASK = 0x0000000f;
	static constexpr u32 PS_EXCM          = 0x00000010;
	static constexpr u32 PS_UM            = 0x00000020;
	static constexpr u32 PS_RING_MASK     = 0x000000c0;
	static constexpr u32 PS_OWB_MASK      = 0x00000f00;
	static constexpr u32 PS_CALLINC_MASK  = 0x00030000;
	static constexpr u32 PS_WOE           = 0x00040000;
	static constexpr u32 PS_WRITE_MASK    = 0x00070fff;
	static constexpr u32 PS_WRITE_MASK_XEA1 = 0x00070f2f; // no EXCM or RING fields

	// exception causes
	enum : u8
	{
		EXCCAUSE_ILLEGAL             = 0,
		EXCCAUSE_SYSCALL             = 1,
		EXCCAUSE_IFETCH_ERROR        = 2,
		EXCCAUSE_LOADSTORE_ERROR     = 3,
		EXCCAUSE_LEVEL1_INTERRUPT    = 4,
		EXCCAUSE_ALLOCA              = 5,
		EXCCAUSE_DIVIDE_BY_ZERO      = 6,
		EXCCAUSE_PRIVILEGED          = 8,
		EXCCAUSE_LOADSTORE_ALIGNMENT = 9
	};

	// address space
	address_space_config m_space_config;
	address_space_config m_extregs_config;

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_space;

	// special registers
	u32 extreg_windowbase_r();
	void extreg_windowbase_w(u32 data);
	u32 extreg_windowstart_r();
	void extreg_windowstart_w(u32 data);
	u32 extreg_lbeg_r();
	void extreg_lbeg_w(u32 data);
	u32 extreg_lend_r();
	void extreg_lend_w(u32 data);
	u32 extreg_lcount_r();
	void extreg_lcount_w(u32 data);
	u32 extreg_sar_r();
	void extreg_sar_w(u32 data);
	u32 extreg_scompare1_r();
	void extreg_scompare1_w(u32 data);
	u32 extreg_ps_r();
	void extreg_ps_w(u32 data);
	u32 extreg_cacheattr_r();
	void extreg_cacheattr_w(u32 data);
	u32 extreg_epc_r(offs_t offset);
	void extreg_epc_w(offs_t offset, u32 data);
	u32 extreg_depc_r();
	void extreg_depc_w(u32 data);
	u32 extreg_eps_r(offs_t offset);
	void extreg_eps_w(offs_t offset, u32 data);
	u32 extreg_excsave_r(offs_t offset);
	void extreg_excsave_w(offs_t offset, u32 data);
	u32 extreg_ibreaka0_r();
	void extreg_ibreaka0_w(u32 data);
	u32 extreg_dbreaka0_r();
	void extreg_dbreaka0_w(u32 data);
	u32 extreg_dbreakc0_r();
	void extreg_dbreakc0_w(u32 data);
	u32 extreg_icountlevel_r();
	void extreg_icountlevel_w(u32 data);
	u32 extreg_interrupt_r();
	void extreg_intset_w(u32 data);
	u32 extreg_intclear_r();
	void extreg_intclear_w(u32 data);
	u32 extreg_intenable_r();
	void extreg_intenable_w(u32 data);
	u32 extreg_exccause_r();
	void extreg_exccause_w(u32 data);
	u32 extreg_ccount_r();
	void extreg_ccount_w(u32 data);
	u32 extreg_ccompare_r(offs_t offset);
	void extreg_ccompare_w(offs_t offset, u32 data);

	void ext_regs(address_map &map) ATTR_COLD;

	u8 get_callinc() const { return (m_extreg_ps & PS_CALLINC_MASK) >> 16; }
	void set_callinc(u8 val) { m_extreg_ps = (m_extreg_ps & ~PS_CALLINC_MASK) | (u32(val & 3) << 16); }
	u32 cintlevel() const;

	void handle_bz(u32 inst);
	void handle_retw();
	bool cwoe() const;
	bool ws_bit(u32 window) const { return BIT(m_extreg_windowstart, window & m_windowbase_mask); }
	void set_ws_bit(u32 window, bool value);
	void set_windowbase(u32 wb);
	u8 window_class(u32 inst, u8 op0) const;
	u8 live_frame_above(u8 need) const;
	u32 overflow_vector(u32 window) const;
	bool window_check(u8 need);
	void raise_window_exception(u32 vector, u32 owb);
	void check_interrupts();
	void acknowledge_irqs(u32 mask);
	void take_interrupt(int level, u32 mask);
	void take_exception(u8 cause);
	void abort_instruction(u8 cause);
	void update_timers();
	void getop_and_execute();

	inline u32 get_reg(u8 reg);
	inline void set_reg(u8 reg, u32 value);

	inline u32 get_mem32(u32 addr);
	inline void set_mem32(u32 addr, u32 data);

	inline u8 get_mem8(u32 addr);
	inline void set_mem8(u32 addr, u8 data);

	inline u16 get_mem16(u32 addr);
	inline void set_mem16(u32 addr, u16 data);

	void handle_reserved(u32 inst);

	void switch_windowbase(s32 change);

	// internal state
	u32 m_a_real[64]; // NAREG physical registers
	u32 m_a[16];
	u32 m_pc;
	s32 m_icount;

	u32 m_extreg_windowbase;
	u32 m_extreg_windowstart;
	u32 m_extreg_sar;
	u32 m_extreg_scompare1;
	u32 m_extreg_lbeg;
	u32 m_extreg_lend;
	u32 m_extreg_lcount;
	u32 m_extreg_ps;
	u32 m_extreg_cacheattr;
	u32 m_extreg_epc[8];      // EPC1..EPC7
	u32 m_extreg_depc;
	u32 m_extreg_eps[8];      // EPS2..EPS7
	u32 m_extreg_excsave[8];  // EXCSAVE1..EXCSAVE7
	u32 m_extreg_ibreaka0;
	u32 m_extreg_dbreaka0;
	u32 m_extreg_dbreakc0;
	u32 m_extreg_icountlevel;
	u32 m_extreg_interrupt;
	u32 m_extreg_intenable;
	u32 m_extreg_exccause;
	u32 m_extreg_ccount;
	u32 m_extreg_ccompare[3];
	u32 m_nextpc;
	u32 m_lcount_at_fetch;
	bool m_waiting;           // suspended by WAITI
	bool m_aborted;           // current instruction raised an exception

	// config
	u32 m_num_physical_regs;
	u32 m_startupvector;
	u32 m_kernel_exception_vector;
	u32 m_user_exception_vector;
	u32 m_double_exception_vector;
	u32 m_interrupt_vector[8];
	u32 m_window_vector_base;
	bool m_window_vectors_set;
	u8 m_irq_level[32];
	u8 m_irq_type[32];
	int m_timerint[3];
	u8 m_excmlevel;
	bool m_xea1;

	// derived from config at start
	u32 m_level_mask[8];         // interrupts at each level
	u32 m_irq_soft_mask;         // software-type interrupts
	u32 m_irq_level_sensitive_mask; // level-type interrupts
	int m_nlevel;
	int m_ncompare;
	u32 m_ps_write_mask;
	u32 m_loop_disable_mask;   // PS bits that disable zero-overhead loops
	u32 m_windowbase_mask;     // NAREG/4 - 1
};

DECLARE_DEVICE_TYPE(XTENSA, xtensa_device)

#endif // MAME_CPU_XTENSA_XTENSA_H

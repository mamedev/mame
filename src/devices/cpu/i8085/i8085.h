// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, hap

#ifndef MAME_CPU_I8085_I8085_H
#define MAME_CPU_I8085_I8085_H


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define I8085_INTR_LINE     0
#define I8085_RST55_LINE    1
#define I8085_RST65_LINE    2
#define I8085_RST75_LINE    3
#define I8085_TRAP_LINE     INPUT_LINE_NMI


class i8085a_cpu_device : public cpu_device
{
public:
	// FIXME: public because drivers/altair.cpp, drivers/ipc.cpp and drivers/unior.cpp set initial PC through state interface
	// should fix boot vector loading in these drivers
	// machine/lviv.cpp and machine/poly88.cpp also access registers via state interface when handling snapshot files
	enum
	{
		I8085_PC, I8085_SP, I8085_AF, I8085_BC, I8085_DE, I8085_HL,
		I8085_A, I8085_B, I8085_C, I8085_D, I8085_E, I8085_F, I8085_H, I8085_L,
		I8085_STATUS, I8085_SOD, I8085_SID, I8085_INTE,
		I8085_HALT, I8085_IM
	};

	static constexpr u8 STATUS_INTA   = 0x01;
	static constexpr u8 STATUS_WO     = 0x02;
	static constexpr u8 STATUS_STACK  = 0x04;
	static constexpr u8 STATUS_HLTA   = 0x08;
	static constexpr u8 STATUS_OUT    = 0x10;
	static constexpr u8 STATUS_M1     = 0x20;
	static constexpr u8 STATUS_INP    = 0x40;
	static constexpr u8 STATUS_MEMR   = 0x80;

	// construction/destruction
	i8085a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// CLK rate callback (8085A only)
	void set_clk_out(clock_update_delegate callback) { m_clk_out_func = callback; }
	template <class FunctionClass> void set_clk_out(const char *devname, void (FunctionClass::*callback)(uint32_t), const char *name)
	{
		set_clk_out(clock_update_delegate(callback, name, devname, static_cast<FunctionClass *>(nullptr)));
	}
	template <class FunctionClass> void set_clk_out(void (FunctionClass::*callback)(uint32_t), const char *name)
	{
		set_clk_out(clock_update_delegate(callback, name, nullptr, static_cast<FunctionClass *>(nullptr)));
	}

	// STATUS changed callback
	auto out_status_func() { return m_out_status_func.bind(); }

	// INTE changed callback
	auto out_inte_func() { return m_out_inte_func.bind(); }

	// SID changed callback (8085A only)
	auto in_sid_func() { return m_in_sid_func.bind(); }

	// SOD changed callback (8085A only)
	auto out_sod_func() { return m_out_sod_func.bind(); }

protected:
	i8085a_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int cputype);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_clock_changed() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 4; }
	virtual u32 execute_max_cycles() const override { return 16; }
	virtual u32 execute_input_lines() const override { return 4; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const override { return inputnum == I8085_RST75_LINE; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const override { return (cycles * 2); }

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_import(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	enum
	{
		CPUTYPE_8080 = 0,
		CPUTYPE_8080A,
		CPUTYPE_8085A
	};

private:
	address_space_config m_program_config;
	address_space_config m_io_config;
	address_space_config m_opcode_config;

	devcb_write8 m_out_status_func;
	devcb_write_line m_out_inte_func;
	devcb_read_line m_in_sid_func;
	devcb_write_line m_out_sod_func;
	clock_update_delegate m_clk_out_func;

	inline bool is_8080() { return m_cputype == CPUTYPE_8080 || m_cputype == CPUTYPE_8080A; }
	inline bool is_8085() { return m_cputype == CPUTYPE_8085A; }
	int m_cputype;

	PAIR m_PC,m_SP,m_AF,m_BC,m_DE,m_HL,m_WZ;
	u8 m_halt;
	u8 m_im;             /* interrupt mask (8085A only) */
	u8 m_status;         /* status word */

	u8 m_after_ei;       /* post-EI processing; starts at 2, check for ints at 0 */
	u8 m_nmi_state;      /* raw NMI line state */
	u8 m_irq_state[4];   /* raw IRQ line states */
	u8 m_trap_pending;   /* TRAP interrupt latched? */
	u8 m_trap_im_copy;   /* copy of IM register when TRAP was taken */
	u8 m_sod_state;      /* state of the SOD line */

	bool m_ietemp;       /* import/export temp space */

	address_space *m_program;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_cache;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_opcode_cache;
	address_space *m_io;
	int m_icount;

	/* cycles lookup */
	static const u8 lut_cycles_8080[256];
	static const u8 lut_cycles_8085[256];
	u8 lut_cycles[256];
	/* flags lookup */
	u8 lut_zs[256];
	u8 lut_zsp[256];

	void set_sod(int state);
	void set_inte(int state);
	void set_status(u8 status);
	u8 get_rim_value();
	void break_halt_for_interrupt();
	u8 read_op();
	u8 read_arg();
	PAIR read_arg16();
	u8 read_mem(u32 a);
	void write_mem(u32 a, u8 v);
	void op_push(PAIR p);
	PAIR op_pop();
	void check_for_interrupts();
	void execute_one(int opcode);
	void init_tables();

	void op_ora(u8 v);
	void op_xra(u8 v);
	void op_ana(u8 v);
	u8 op_inr(u8 v);
	u8 op_dcr(u8 v);
	void op_add(u8 v);
	void op_adc(u8 v);
	void op_sub(u8 v);
	void op_sbb(u8 v);
	void op_cmp(u8 v);
	void op_dad(u16 v);
	void op_jmp(int cond);
	void op_call(int cond);
	void op_ret(int cond);
	void op_rst(u8 v);
};


class i8080_cpu_device : public i8085a_cpu_device
{
public:
	// construction/destruction
	i8080_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u32 execute_input_lines() const override { return 1; }
	virtual u64 execute_clocks_to_cycles(u64 clocks) const override { return clocks; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const override { return cycles; }
};


class i8080a_cpu_device : public i8085a_cpu_device
{
public:
	// construction/destruction
	i8080a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual u32 execute_input_lines() const override { return 1; }
	virtual u64 execute_clocks_to_cycles(u64 clocks) const override { return clocks; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const override { return cycles; }
};


DECLARE_DEVICE_TYPE(I8080,  i8080_cpu_device)
DECLARE_DEVICE_TYPE(I8080A, i8080a_cpu_device)
DECLARE_DEVICE_TYPE(I8085A, i8085a_cpu_device)

#endif // MAME_CPU_I8085_I8085_H

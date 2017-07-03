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


// STATUS changed callback
#define MCFG_I8085A_STATUS(_devcb) \
	devcb = &i8085a_cpu_device::set_out_status_func(*device, DEVCB_##_devcb);

// INTE changed callback
#define MCFG_I8085A_INTE(_devcb) \
	devcb = &i8085a_cpu_device::set_out_inte_func(*device, DEVCB_##_devcb);

// SID changed callback (8085A only)
#define MCFG_I8085A_SID(_devcb) \
	devcb = &i8085a_cpu_device::set_in_sid_func(*device, DEVCB_##_devcb);

// SOD changed callback (8085A only)
#define MCFG_I8085A_SOD(_devcb) \
	devcb = &i8085a_cpu_device::set_out_sod_func(*device, DEVCB_##_devcb);

// CLK rate callback (8085A only)
#define MCFG_I8085A_CLK_OUT_DEVICE(_tag) \
	i8085a_cpu_device::static_set_clk_out(*device, clock_update_delegate(FUNC(device_t::set_unscaled_clock), _tag, (device_t *)nullptr));
#define MCFG_I8085A_CLK_OUT_CUSTOM(_class, _func) \
	i8085a_cpu_device::static_set_clk_out(*device, clock_update_delegate(&_class::_func, #_class "::" _func, owner));


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

	static constexpr uint8_t STATUS_INTA   = 0x01;
	static constexpr uint8_t STATUS_WO     = 0x02;
	static constexpr uint8_t STATUS_STACK  = 0x04;
	static constexpr uint8_t STATUS_HLTA   = 0x08;
	static constexpr uint8_t STATUS_OUT    = 0x10;
	static constexpr uint8_t STATUS_M1     = 0x20;
	static constexpr uint8_t STATUS_INP    = 0x40;
	static constexpr uint8_t STATUS_MEMR   = 0x80;

	// construction/destruction
	i8085a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> static devcb_base &set_out_status_func(device_t &device, Object &&cb) { return downcast<i8085a_cpu_device &>(device).m_out_status_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out_inte_func(device_t &device, Object &&cb) { return downcast<i8085a_cpu_device &>(device).m_out_inte_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_in_sid_func(device_t &device, Object &&cb) { return downcast<i8085a_cpu_device &>(device).m_in_sid_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out_sod_func(device_t &device, Object &&cb) { return downcast<i8085a_cpu_device &>(device).m_out_sod_func.set_callback(std::forward<Object>(cb)); }
	static void static_set_clk_out(device_t &device, clock_update_delegate &&clk_out) { downcast<i8085a_cpu_device &>(device).m_clk_out_func = std::move(clk_out); }

protected:
	i8085a_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int cputype);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_clock_changed() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 4; }
	virtual uint32_t execute_max_cycles() const override { return 16; }
	virtual uint32_t execute_input_lines() const override { return 4; }
	virtual uint32_t execute_default_irq_vector() const override { return 0xff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override { return (cycles * 2); }

	// device_memory_interface overrides
	virtual std::vector<std::pair<int, const address_space_config *>> memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_import(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	devcb_write8       m_out_status_func;
	devcb_write_line   m_out_inte_func;
	devcb_read_line    m_in_sid_func;
	devcb_write_line   m_out_sod_func;
	clock_update_delegate m_clk_out_func;

	int                 m_cputype;        /* 0 8080, 1 8085A */
	PAIR                m_PC,m_SP,m_AF,m_BC,m_DE,m_HL,m_WZ;
	uint8_t               m_HALT;
	uint8_t               m_IM;             /* interrupt mask (8085A only) */
	uint8_t               m_STATUS;         /* status word */

	uint8_t               m_after_ei;       /* post-EI processing; starts at 2, check for ints at 0 */
	uint8_t               m_nmi_state;      /* raw NMI line state */
	uint8_t               m_irq_state[4];   /* raw IRQ line states */
	uint8_t               m_trap_pending;   /* TRAP interrupt latched? */
	uint8_t               m_trap_im_copy;   /* copy of IM register when TRAP was taken */
	uint8_t               m_sod_state;      /* state of the SOD line */

	bool                m_ietemp;         /* import/export temp space */

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	int                 m_icount;

	/* cycles lookup */
	static const uint8_t lut_cycles_8080[256];
	static const uint8_t lut_cycles_8085[256];
	uint8_t lut_cycles[256];
	/* flags lookup */
	uint8_t ZS[256];
	uint8_t ZSP[256];

	void set_sod(int state);
	void set_inte(int state);
	void set_status(uint8_t status);
	uint8_t get_rim_value();
	void break_halt_for_interrupt();
	uint8_t ROP();
	uint8_t ARG();
	uint16_t ARG16();
	uint8_t RM(uint32_t a);
	void WM(uint32_t a, uint8_t v);
	void check_for_interrupts();
	void execute_one(int opcode);
	void init_tables();

};


class i8080_cpu_device : public i8085a_cpu_device
{
public:
	// construction/destruction
	i8080_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override { return clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override { return cycles; }
};


class i8080a_cpu_device : public i8085a_cpu_device
{
public:
	// construction/destruction
	i8080a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint32_t execute_input_lines() const override { return 1; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override { return clocks; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override { return cycles; }
};


DECLARE_DEVICE_TYPE(I8080,  i8080_cpu_device)
DECLARE_DEVICE_TYPE(I8080A, i8080a_cpu_device)
DECLARE_DEVICE_TYPE(I8085A, i8085a_cpu_device)

#endif // MAME_CPU_I8085_I8085_H

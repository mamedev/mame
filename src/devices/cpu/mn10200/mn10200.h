// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    MAME conversion by R. Belmont

*/

#ifndef MN10200_H
#define MN10200_H

// port setup
#define MCFG_MN10200_READ_PORT_CB(X, _devcb) \
	mn10200_device::set_read_port##X##_callback(*device, DEVCB_##_devcb);
#define MCFG_MN10200_WRITE_PORT_CB(X, _devcb) \
	mn10200_device::set_write_port##X##_callback(*device, DEVCB_##_devcb);

enum
{
	MN10200_PORT0 = 0,
	MN10200_PORT1,
	MN10200_PORT2,
	MN10200_PORT3,
	MN10200_PORT4
};

enum
{
	MN10200_IRQ0 = 0,
	MN10200_IRQ1,
	MN10200_IRQ2,
	MN10200_IRQ3,

	MN10200_MAX_EXT_IRQ
};


#define MN10200_NUM_PRESCALERS (2)
#define MN10200_NUM_TIMERS_8BIT (10)
#define MN10200_NUM_IRQ_GROUPS (31)


class mn10200_device : public cpu_device
{
public:
	// construction/destruction
	mn10200_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, address_map_constructor program, std::string shortname, std::string source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_LITTLE, 16, 24, 0, program), m_program(nullptr)
			, m_read_port0(*this), m_read_port1(*this), m_read_port2(*this), m_read_port3(*this), m_read_port4(*this)
		, m_write_port0(*this), m_write_port1(*this), m_write_port2(*this), m_write_port3(*this), m_write_port4(*this), m_cycles(0), m_pc(0), m_psw(0), m_mdr(0), m_nmicr(0), m_iagr(0),
		m_extmdl(0), m_extmdh(0), m_possible_irq(false), m_pplul(0), m_ppluh(0), m_p3md(0), m_p4(0)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_port0_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_read_port0.set_callback(object); }
	template<class _Object> static devcb_base &set_read_port1_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_read_port1.set_callback(object); }
	template<class _Object> static devcb_base &set_read_port2_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_read_port2.set_callback(object); }
	template<class _Object> static devcb_base &set_read_port3_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_read_port3.set_callback(object); }
	template<class _Object> static devcb_base &set_read_port4_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_read_port4.set_callback(object); }

	template<class _Object> static devcb_base &set_write_port0_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_write_port0.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port1_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_write_port1.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port2_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_write_port2.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port3_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_write_port3.set_callback(object); }
	template<class _Object> static devcb_base &set_write_port4_callback(device_t &device, _Object object) { return downcast<mn10200_device &>(device).m_write_port4.set_callback(object); }

	DECLARE_READ8_MEMBER(io_control_r);
	DECLARE_WRITE8_MEMBER(io_control_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 2 - 1) / 2; } // internal /2 divider
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 2); } // internal /2 divider
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 13+7; } // max opcode cycles + interrupt duration
	virtual UINT32 execute_input_lines() const override { return 4; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 7; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space *m_program;

	// i/o handlers
	devcb_read8 m_read_port0, m_read_port1, m_read_port2, m_read_port3, m_read_port4;
	devcb_write8 m_write_port0, m_write_port1, m_write_port2, m_write_port3, m_write_port4;

	int m_cycles;

	// The UINT32s are really UINT24
	UINT32 m_pc;
	UINT32 m_d[4];
	UINT32 m_a[4];
	UINT16 m_psw;
	UINT16 m_mdr;

	// interrupts
	void take_irq(int level, int group);
	void check_irq();
	void check_ext_irq();

	UINT8 m_icrl[MN10200_NUM_IRQ_GROUPS];
	UINT8 m_icrh[MN10200_NUM_IRQ_GROUPS];

	UINT8 m_nmicr;
	UINT8 m_iagr;
	UINT8 m_extmdl;
	UINT8 m_extmdh;
	bool m_possible_irq;

	// timers
	void refresh_timer(int tmr);
	void refresh_all_timers();
	int timer_tick_simple(int tmr);
	TIMER_CALLBACK_MEMBER( simple_timer_cb );

	attotime m_sysclock_base;
	emu_timer *m_timer_timers[MN10200_NUM_TIMERS_8BIT];

	struct
	{
		UINT8 mode;
		UINT8 base;
		UINT8 cur;
	} m_simple_timer[MN10200_NUM_TIMERS_8BIT];

	struct
	{
		UINT8 mode;
		UINT8 base;
		UINT8 cur;
	} m_prescaler[MN10200_NUM_PRESCALERS];

	// dma
	struct
	{
		UINT32 adr;
		UINT32 count;
		UINT16 iadr;
		UINT8 ctrll;
		UINT8 ctrlh;
		UINT8 irq;
	} m_dma[8];

	// serial
	struct
	{
		UINT8 ctrll;
		UINT8 ctrlh;
		UINT8 buf;
	} m_serial[2];

	// ports
	UINT8 m_pplul;
	UINT8 m_ppluh;
	UINT8 m_p3md;
	UINT8 m_p4;

	struct
	{
		UINT8 out;
		UINT8 dir;
	} m_port[4];

	// internal read/write
	inline UINT8 read_arg8(UINT32 address) { return m_program->read_byte(address); }
	inline UINT16 read_arg16(UINT32 address) { return m_program->read_byte(address) | m_program->read_byte(address + 1) << 8; }
	inline UINT32 read_arg24(UINT32 address) { return m_program->read_byte(address) | m_program->read_byte(address + 1) << 8 | m_program->read_byte(address + 2) << 16; }

	inline UINT8 read_mem8(UINT32 address) { return m_program->read_byte(address); }
	inline UINT16 read_mem16(UINT32 address) { return m_program->read_word(address & ~1); }
	inline UINT32 read_mem24(UINT32 address) { return m_program->read_word(address & ~1) | m_program->read_byte((address & ~1) + 2) << 16; }

	inline void write_mem8(UINT32 address, UINT8 data) { m_program->write_byte(address, data); }
	inline void write_mem16(UINT32 address, UINT16 data) { m_program->write_word(address & ~1, data); }
	inline void write_mem24(UINT32 address, UINT32 data) { m_program->write_word(address & ~1, data); m_program->write_byte((address & ~1) + 2, data >> 16); }

	inline void change_pc(UINT32 pc) { m_pc = pc & 0xffffff; }

	// opcode helpers
	void illegal(UINT8 prefix, UINT8 op);
	UINT32 do_add(UINT32 a, UINT32 b, UINT32 c = 0);
	UINT32 do_sub(UINT32 a, UINT32 b, UINT32 c = 0);
	void test_nz16(UINT16 v);
	void do_jsr(UINT32 to, UINT32 ret);
	void do_branch(int condition = 1);
};


class mn1020012a_device : public mn10200_device
{
public:
	mn1020012a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};



extern const device_type MN1020012A;


#endif // MN10200_H

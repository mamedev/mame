// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, hap
/*
    Panasonic MN10200 emulator

    Written by Olivier Galibert
    MAME conversion by R. Belmont

*/

#ifndef MAME_CPU_MN10200_MN10200_H
#define MAME_CPU_MN10200_MN10200_H

#pragma once

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


class mn10200_device : public cpu_device
{
public:
	// configuration helpers
	template <std::size_t Port> auto read_port() { return m_read_port[Port].bind(); }
	template <std::size_t Port> auto write_port() { return m_write_port[Port].bind(); }

	uint8_t io_control_r(offs_t offset);
	void io_control_w(offs_t offset, uint8_t data);

	void mn1020012a_internal_map(address_map &map) ATTR_COLD;
protected:
	static constexpr unsigned MN10200_NUM_PRESCALERS = 2;
	static constexpr unsigned MN10200_NUM_TIMERS_8BIT = 10;
	static constexpr unsigned MN10200_NUM_IRQ_GROUPS = 31;


	// construction/destruction
	mn10200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor program);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; } // internal /2 divider
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); } // internal /2 divider
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 13+7; } // max opcode cycles + interrupt duration
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space *m_program;

	// i/o handlers
	devcb_read8::array<5> m_read_port;
	devcb_write8::array<5> m_write_port;

	int m_cycles;

	// The UINT32s are really UINT24
	uint32_t m_pc;
	uint32_t m_d[4];
	uint32_t m_a[4];
	uint16_t m_psw;
	uint16_t m_mdr;

	// interrupts
	void take_irq(int level, int group);
	void check_irq();
	void check_ext_irq();

	uint8_t m_icrl[MN10200_NUM_IRQ_GROUPS];
	uint8_t m_icrh[MN10200_NUM_IRQ_GROUPS];

	uint8_t m_nmicr;
	uint8_t m_iagr;
	uint8_t m_extmdl;
	uint8_t m_extmdh;
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
		uint8_t mode;
		uint8_t base;
		uint8_t cur;
	} m_simple_timer[MN10200_NUM_TIMERS_8BIT];

	struct
	{
		uint8_t mode;
		uint8_t base;
		uint8_t cur;
	} m_prescaler[MN10200_NUM_PRESCALERS];

	// dma
	struct
	{
		uint32_t adr;
		uint32_t count;
		uint16_t iadr;
		uint8_t ctrll;
		uint8_t ctrlh;
		uint8_t irq;
	} m_dma[8];

	// serial
	struct
	{
		uint8_t ctrll;
		uint8_t ctrlh;
		uint8_t buf;
		uint8_t recv;
	} m_serial[2];

	// ports
	uint8_t m_pplul;
	uint8_t m_ppluh;
	uint8_t m_p3md;
	uint8_t m_p4;

	struct
	{
		uint8_t out;
		uint8_t dir;
	} m_port[4];

	// internal read/write
	inline uint8_t read_arg8(uint32_t address) { return m_program->read_byte(address); }
	inline uint16_t read_arg16(uint32_t address) { return m_program->read_byte(address) | m_program->read_byte(address + 1) << 8; }
	inline uint32_t read_arg24(uint32_t address) { return m_program->read_byte(address) | m_program->read_byte(address + 1) << 8 | m_program->read_byte(address + 2) << 16; }

	inline uint8_t read_mem8(uint32_t address) { return m_program->read_byte(address); }
	inline uint16_t read_mem16(uint32_t address) { return m_program->read_word(address & ~1); }
	inline uint32_t read_mem24(uint32_t address) { return m_program->read_word(address & ~1) | m_program->read_byte((address & ~1) + 2) << 16; }

	inline void write_mem8(uint32_t address, uint8_t data) { m_program->write_byte(address, data); }
	inline void write_mem16(uint32_t address, uint16_t data) { m_program->write_word(address & ~1, data); }
	inline void write_mem24(uint32_t address, uint32_t data) { m_program->write_word(address & ~1, data); m_program->write_byte((address & ~1) + 2, data >> 16); }

	inline void change_pc(uint32_t pc) { m_pc = pc & 0xffffff; }

	// opcode helpers
	void illegal(uint8_t prefix, uint8_t op);
	uint32_t do_add(uint32_t a, uint32_t b, uint32_t c = 0);
	uint32_t do_sub(uint32_t a, uint32_t b, uint32_t c = 0);
	void test_nz16(uint16_t v);
	void do_jsr(uint32_t to, uint32_t ret);
	void do_branch(int condition = 1);
};


class mn1020012a_device : public mn10200_device
{
public:
	mn1020012a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};



DECLARE_DEVICE_TYPE(MN1020012A, mn1020012a_device)


#endif // MAME_CPU_MN10200_MN10200_H

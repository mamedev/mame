// license:BSD-3-Clause
// copyright-holders:hap, Tim Lindner
/*

  Texas Instruments TMS7000

*/

#ifndef MAME_CPU_TMS7000_TMS7000_H
#define MAME_CPU_TMS7000_TMS7000_H

#pragma once

#include "debugger.h"

enum { TMS7000_PC=1, TMS7000_SP, TMS7000_ST, TMS7000_A, TMS7000_B };

enum
{
	/* note: INT2,4,5 are generated internally */
	TMS7000_INT1_LINE = 0,
	TMS7000_INT3_LINE
};


class tms7000_device : public cpu_device
{
public:
	// construction/destruction
	tms7000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read-only on 70x0
	auto in_porta() { return m_port_in_cb[0].bind(); }
	auto out_porta() { return m_port_out_cb[0].bind(); }

	// write-only
	auto out_portb() { return m_port_out_cb[1].bind(); }

	auto in_portc() { return m_port_in_cb[2].bind(); }
	auto out_portc() { return m_port_out_cb[2].bind(); }
	auto in_portd() { return m_port_in_cb[3].bind(); }
	auto out_portd() { return m_port_out_cb[3].bind(); }

	// TMS70C46 only
	auto in_porte() { return m_port_in_cb[4].bind(); }
	auto out_porte() { return m_port_out_cb[4].bind(); }

	// clock divider mask option
	void set_divide_by_2() { m_divider = 2; }
	void set_divide_by_4() { m_divider = 4; }

	uint8_t tms7000_unmapped_rf_r(offs_t offset) { if (!machine().side_effects_disabled()) logerror("'%s' (%04X): unmapped_rf_r @ $%04x\n", tag(), m_pc, offset + 0x80); return 0; }
	void tms7000_unmapped_rf_w(offs_t offset, uint8_t data) { logerror("'%s' (%04X): unmapped_rf_w @ $%04x = $%02x\n", tag(), m_pc, offset + 0x80, data); }

	uint8_t tms7000_pf_r(offs_t offset);
	void tms7000_pf_w(offs_t offset, uint8_t data);
	uint8_t tms7002_pf_r(offs_t offset) { return tms7000_pf_r(offset + 0x10); }
	void tms7002_pf_w(offs_t offset, uint8_t data) { tms7000_pf_w(offset + 0x10, data); }

	bool chip_is_cmos() const { return (m_info_flags & CHIP_IS_CMOS) ? true : false; }
	bool chip_is_family_70x0() const { return chip_get_family() == CHIP_FAMILY_70X0; }
	bool chip_is_family_70x2() const { return chip_get_family() == CHIP_FAMILY_70X2; }
	bool chip_is_family_70cx2() const { return chip_get_family() == CHIP_FAMILY_70CX2; }

	void tms7000_mem(address_map &map);
	void tms7001_mem(address_map &map);
	void tms7002_mem(address_map &map);
	void tms7020_mem(address_map &map);
	void tms7040_mem(address_map &map);
	void tms7041_mem(address_map &map);
	void tms7042_mem(address_map &map);
protected:
	// chip info flags
	static constexpr uint32_t CHIP_IS_CMOS        = 0x01;
	static constexpr uint32_t CHIP_FAMILY_70X0    = 0x00;
	static constexpr uint32_t CHIP_FAMILY_70X2    = 0x02;
	static constexpr uint32_t CHIP_FAMILY_70CX2   = 0x04;
	static constexpr uint32_t CHIP_FAMILY_MASK    = 0x06;

	tms7000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal, uint32_t info_flags);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + m_divider - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * m_divider); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 49; }
	virtual uint32_t execute_input_lines() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int extline, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	uint32_t chip_get_family() const { return m_info_flags & CHIP_FAMILY_MASK; }

	virtual void execute_one(uint8_t op);

	address_space_config m_program_config;

	devcb_read8::array<5> m_port_in_cb;
	devcb_write8::array<5> m_port_out_cb;

	const uint32_t m_info_flags;
	unsigned m_divider;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;
	int m_icount;

	bool m_irq_state[2];
	bool m_idle_state;
	bool m_idle_halt;
	uint16_t m_pc;
	uint8_t m_sp;
	uint8_t m_sr;
	uint8_t m_op;

	uint8_t m_io_control[3];

	emu_timer *m_timer_handle[2];
	uint8_t m_timer_data[2];
	uint8_t m_timer_control[2];
	int m_timer_decrementer[2];
	int m_timer_prescaler[2];
	uint16_t m_timer_capture_latch[2];

	uint8_t m_port_latch[4];
	uint8_t m_port_ddr[4];

	void flag_ext_interrupt(int extline);
	void check_interrupts();
	void do_interrupt(int irqline);

	TIMER_CALLBACK_MEMBER(simple_timer_cb);
	void timer_run(int tmr);
	void timer_reload(int tmr);
	void timer_tick_pre(int tmr);
	void timer_tick_low(int tmr);

	// internal read/write
	inline uint8_t read_r8(uint8_t address) { return m_program.read_byte(address); }
	inline void write_r8(uint8_t address, uint8_t data) { m_program.write_byte(address, data); }
	inline uint16_t read_r16(uint8_t address) { return m_program.read_byte((address - 1) & 0xff) << 8 | m_program.read_byte(address); }
	inline void write_r16(uint8_t address, uint16_t data) { m_program.write_byte((address - 1) & 0xff, data >> 8 & 0xff); m_program.write_byte(address, data & 0xff); }

	inline uint8_t read_p(uint8_t address) { return m_program.read_byte(0x100 + address); }
	inline void write_p(uint8_t address, uint8_t data) { m_program.write_byte(0x100 + address, data); }

	inline uint8_t read_mem8(uint16_t address) { return m_program.read_byte(address); }
	inline void write_mem8(uint16_t address, uint8_t data) { m_program.write_byte(address, data); }
	inline uint16_t read_mem16(uint16_t address) { return m_program.read_byte(address) << 8 | m_program.read_byte((address + 1) & 0xffff); }
	inline void write_mem16(uint16_t address, uint16_t data) { m_program.write_byte(address, data >> 8 & 0xff); m_program.write_byte((address + 1) & 0xffff, data & 0xff); }

	inline uint8_t imm8() { return m_cache.read_byte(m_pc++); }
	inline uint16_t imm16() { uint16_t ret = m_cache.read_byte(m_pc++) << 8; return ret | m_cache.read_byte(m_pc++); }

	inline uint8_t pull8() { return m_program.read_byte(m_sp--); }
	inline void push8(uint8_t data) { m_program.write_byte(++m_sp, data); }
	inline uint16_t pull16() { uint16_t ret = m_program.read_byte(m_sp--); return ret | m_program.read_byte(m_sp--) << 8; }
	inline void push16(uint16_t data) { m_program.write_byte(++m_sp, data >> 8 & 0xff); m_program.write_byte(++m_sp, data & 0xff); }

	// statusreg flags
	enum
	{
		SR_C = 0x80, // carry
		SR_N = 0x40, // negative
		SR_Z = 0x20, // zero
		SR_I = 0x10  // interrupt
	};

	// opcode handlers
	void br_dir();
	void br_inx();
	void br_ind();
	void call_dir();
	void call_inx();
	void call_ind();
	void cmpa_dir();
	void cmpa_inx();
	void cmpa_ind();
	void decd_a();
	void decd_b();
	void decd_r();
	void dint();
	void eint();
	void idle();
	void lda_dir();
	void lda_inx();
	void lda_ind();
	void ldsp();
	void movd_dir();
	void movd_inx();
	void movd_ind();
	void nop();
	void pop_a();
	void pop_b();
	void pop_r();
	void pop_st();
	void push_a();
	void push_b();
	void push_r();
	void push_st();
	void reti();
	void rets();
	void setc();
	void sta_dir();
	void sta_inx();
	void sta_ind();
	void stsp();
	void trap(uint8_t address);
	void illegal(uint8_t op);

	typedef int (tms7000_device::*op_func)(uint8_t, uint8_t);
	int op_clr(uint8_t param1, uint8_t param2);
	int op_dec(uint8_t param1, uint8_t param2);
	int op_inc(uint8_t param1, uint8_t param2);
	int op_inv(uint8_t param1, uint8_t param2);
	int op_rl(uint8_t param1, uint8_t param2);
	int op_rlc(uint8_t param1, uint8_t param2);
	int op_rr(uint8_t param1, uint8_t param2);
	int op_rrc(uint8_t param1, uint8_t param2);
	int op_swap(uint8_t param1, uint8_t param2);
	int op_xchb(uint8_t param1, uint8_t param2);

	int op_adc(uint8_t param1, uint8_t param2);
	int op_add(uint8_t param1, uint8_t param2);
	int op_and(uint8_t param1, uint8_t param2);
	int op_cmp(uint8_t param1, uint8_t param2);
	int op_dac(uint8_t param1, uint8_t param2);
	int op_dsb(uint8_t param1, uint8_t param2);
	int op_mpy(uint8_t param1, uint8_t param2);
	int op_mov(uint8_t param1, uint8_t param2);
	int op_or(uint8_t param1, uint8_t param2);
	int op_sbb(uint8_t param1, uint8_t param2);
	int op_sub(uint8_t param1, uint8_t param2);
	int op_xor(uint8_t param1, uint8_t param2);

	void shortbranch(bool check);
	void jmp(bool check);
	int op_djnz(uint8_t param1, uint8_t param2);
	int op_btjo(uint8_t param1, uint8_t param2);
	int op_btjz(uint8_t param1, uint8_t param2);

	void am_a(op_func op);
	void am_b(op_func op);
	void am_r(op_func op);
	void am_a2a(op_func op);
	void am_a2b(op_func op);
	void am_a2r(op_func op);
	void am_a2p(op_func op);
	void am_b2a(op_func op);
	void am_b2b(op_func op);
	void am_b2r(op_func op);
	void am_b2p(op_func op);
	void am_r2a(op_func op);
	void am_r2b(op_func op);
	void am_r2r(op_func op);
	void am_i2a(op_func op);
	void am_i2b(op_func op);
	void am_i2r(op_func op);
	void am_i2p(op_func op);
	void am_p2a(op_func op);
	void am_p2b(op_func op);
};


class tms7020_device : public tms7000_device
{
public:
	tms7020_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms7020_exl_device : public tms7000_device
{
public:
	tms7020_exl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void execute_one(uint8_t op) override;

private:
	void lvdp();
};


class tms7040_device : public tms7000_device
{
public:
	tms7040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms70c00_device : public tms7000_device
{
public:
	tms70c00_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms70c20_device : public tms7000_device
{
public:
	tms70c20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms70c40_device : public tms7000_device
{
public:
	tms70c40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms70c46_device : public tms7000_device
{
public:
	tms70c46_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t control_r();
	void control_w(uint8_t data);

	uint8_t dockbus_status_r();
	void dockbus_status_w(uint8_t data);
	uint8_t dockbus_data_r();
	void dockbus_data_w(uint8_t data);

	// access I/O port E if databus is disabled
	uint8_t e_bus_data_r() { return machine().side_effects_disabled() ? 0xff : ((m_control & 0x20) ? 0xff : m_port_in_cb[4]()); }
	void e_bus_data_w(uint8_t data) { if (~m_control & 0x20) m_port_out_cb[4](data); }

	void tms70c46_mem(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_control;
};


class tms7001_device : public tms7000_device
{
public:
	tms7001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms7041_device : public tms7000_device
{
public:
	tms7041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms7002_device : public tms7000_device
{
public:
	tms7002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tms7042_device : public tms7000_device
{
public:
	tms7042_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(TMS7000,     tms7000_device)
DECLARE_DEVICE_TYPE(TMS7020,     tms7020_device)
DECLARE_DEVICE_TYPE(TMS7020_EXL, tms7020_exl_device)
DECLARE_DEVICE_TYPE(TMS7040,     tms7040_device)
DECLARE_DEVICE_TYPE(TMS70C00,    tms70c00_device)
DECLARE_DEVICE_TYPE(TMS70C20,    tms70c20_device)
DECLARE_DEVICE_TYPE(TMS70C40,    tms70c40_device)
DECLARE_DEVICE_TYPE(TMS70C46,    tms70c46_device)
DECLARE_DEVICE_TYPE(TMS7001,     tms7001_device)
DECLARE_DEVICE_TYPE(TMS7041,     tms7041_device)
DECLARE_DEVICE_TYPE(TMS7002,     tms7002_device)
DECLARE_DEVICE_TYPE(TMS7042,     tms7042_device)

#endif // MAME_CPU_TMS7000_TMS7000_H

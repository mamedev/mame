// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_M6805_M68HC05E1_H
#define MAME_CPU_M6805_M68HC05E1_H

#pragma once

#include "emu.h"
#include "m6805.h"

class m68hc05ex_device : public m6805_base_device
{
	friend class m68hc05e1_device;

public:
	const address_space_config m_program_config;

	template <std::size_t Bit> auto read_p() { return m_read_p[Bit].bind(); }
	template <std::size_t Bit> auto write_p() { return m_write_p[Bit].bind(); }
	template <std::size_t Bit> void set_pullups(u8 mask) { m_pullups[Bit] = mask; }

protected:
	// construction/destruction
	m68hc05ex_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual void interrupt_vector() override;

	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	u8 ports_r(offs_t offset);
	void ports_w(offs_t offset, u8 data);
	u8 ddrs_r(offs_t offset);
	void ddrs_w(offs_t offset, u8 data);
	u8 read_port(u8 offset);
	void send_port(u8 offset, u8 data);
	u8 pll_r();
	void pll_w(u8 data);
	u8 timer_ctrl_r();
	void timer_ctrl_w(u8 data);
	u8 timer_counter_r();
	u8 onesec_r();
	void onesec_w(u8 data);

	devcb_read8::array<5>  m_read_p;
	devcb_write8::array<5> m_write_p;

	u8 m_ports[4], m_ddrs[4], m_pullups[4];
	u8 m_pll_ctrl;
	u8 m_timer_ctrl;
	u8 m_onesec;
	emu_timer *m_timer, *m_prog_timer;

	TIMER_CALLBACK_MEMBER(seconds_tick);
	TIMER_CALLBACK_MEMBER(timer_tick);
};

class m68hc05e1_device : public m68hc05ex_device
{
public:
	m68hc05e1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read_internal_ram(offs_t offset);
	void write_internal_ram(offs_t offset, u8 data);

protected:
	m68hc05e1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor internal_map);

	required_shared_ptr<u8> m_internal_ram;

private:
	void m68hc05e1_map(address_map &map) ATTR_COLD;
};

class m68hc05e5_device : public m68hc05e1_device
{
public:
	m68hc05e5_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read_internal_ram(offs_t offset);
	void write_internal_ram(offs_t offset, u8 data);

protected:
	required_shared_ptr<u8> m_internal_ram;

private:
	void m68hc05e5_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(M68HC05E1, m68hc05e1_device)
DECLARE_DEVICE_TYPE(M68HC05E5, m68hc05e5_device)

#endif // MAME_CPU_M6805_M58HC05E1_H

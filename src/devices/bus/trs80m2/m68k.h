// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 16/16B/6000 68000 CPU board emulation

**********************************************************************/

#ifndef MAME_BUS_TRS80M2_M68K_H
#define MAME_BUS_TRS80M2_M68K_H

#pragma once

#include "slot.h"

#include "cpu/m68000/m68000.h"
#include "machine/am9519.h"


class trs80m2_m68k_device_base : public device_t, public device_trs80m2_card_interface
{
protected:
	trs80m2_m68k_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const XTAL &cpu_clock, bool burst, bool useirq);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void tcl_w(uint8_t data);
	void ual_w(uint8_t data);
	uint8_t window_r(offs_t offset);
	void window_w(offs_t offset, uint8_t data);

	void m68000_mem(address_map &map) ATTR_COLD;
	void m68000_user_mem(address_map &map) ATTR_COLD;
	void m68000_cpu_space(address_map &map) ATTR_COLD;

	uint16_t timeout_r();
	void timeout_w(uint16_t data);
	uint16_t user_r(offs_t offset, uint16_t mem_mask);
	void user_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t uic_r(offs_t offset, uint16_t mem_mask);
	void uic_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t iack_r();
	void useirq_pulse();

	offs_t z80_window_address(offs_t offset) const;
	bool translate(offs_t &offset);
	void pulse_ireq(int line);
	void remap_ram();
	TIMER_CALLBACK_MEMBER(install_window);

	required_device<m68000_device> m_subcpu;
	required_device<am9519_device> m_uic;

	XTAL const m_cpu_clock;
	bool const m_burst;
	bool const m_useirq;

	emu_timer *m_window_timer;
	bool m_window_installed;

	address_space *m_m68k_space;
	memory_access<24, 1, 0, ENDIANNESS_BIG>::specific m_m68k_program;
	uint16_t m_ual;
	uint8_t m_tcl;
	bool m_z80_master;
	uint8_t m_limit[2];
	uint8_t m_offset[2];
};


class trs80m2_m68k_m16_device : public trs80m2_m68k_device_base
{
public:
	trs80m2_m68k_m16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


class trs80m2_m68k_m16b_device : public trs80m2_m68k_device_base
{
public:
	trs80m2_m68k_m16b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


class trs80m2_m68k_t6000_device : public trs80m2_m68k_device_base
{
public:
	trs80m2_m68k_t6000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(TRS80M2_M68K_M16, trs80m2_m68k_m16_device)
DECLARE_DEVICE_TYPE(TRS80M2_M68K_M16B, trs80m2_m68k_m16b_device)
DECLARE_DEVICE_TYPE(TRS80M2_M68K_T6000, trs80m2_m68k_t6000_device)

#endif // MAME_BUS_TRS80M2_M68K_H

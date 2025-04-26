// license:BSD-3-Clause
// copyright-holders:David Haywood,Alex Marshall
/***************************************************************************

 NMK004 emulation

***************************************************************************/

#ifndef MAME_NMK_NMK004_H
#define MAME_NMK_NMK004_H

#include "cpu/tlcs90/tlcs90.h"

#pragma once

class nmk004_device : public device_t
{
public:
	nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto reset_cb() { return m_reset_cb.bind(); }
	template <typename T, typename U> void set_rom_tag(T &&tag1, U &&tag2)
	{
		m_okirom[0].set_tag(std::forward<T>(tag1));
		m_okirom[1].set_tag(std::forward<U>(tag2));
	}
	template <typename T, typename U> void set_rombank_tag(T &&tag1, U &&tag2)
	{
		m_okibank[0].set_tag(std::forward<T>(tag1));
		m_okibank[1].set_tag(std::forward<U>(tag2));
	}
	auto ym_read_callback() { return m_ym_read_cb.bind(); }
	auto ym_write_callback() { return m_ym_write_cb.bind(); }
	template <unsigned Which> auto oki_read_callback() { return m_oki_read_cb[Which].bind(); }
	template <unsigned Which> auto oki_write_callback() { return m_oki_write_cb[Which].bind(); }

	// host interface
	void write(uint8_t data);
	uint8_t read();
	void nmi_w(int state) { m_cpu->set_input_line(INPUT_LINE_NMI, state); }

	void ym2203_irq_handler(int irq);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<tlcs90_device> m_cpu;
	required_region_ptr_array<uint8_t, 2> m_okirom;
	required_memory_bank_array<2> m_okibank;
	devcb_write_line m_reset_cb;
	devcb_read8 m_ym_read_cb;
	devcb_write8 m_ym_write_cb;
	devcb_read8 m_oki_read_cb[2];
	devcb_write8 m_oki_write_cb[2];

	uint8_t to_nmk004;
	uint8_t to_main;

	template <unsigned Which> void oki_bankswitch_w(uint8_t data);
	uint8_t ym_r(offs_t offset);
	void ym_w(offs_t offset, uint8_t data);
	template <unsigned Which> uint8_t oki_r();
	template <unsigned Which> void oki_w(uint8_t data);
	uint8_t tonmk004_r();
	void tomain_w(uint8_t data);

	void port4_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NMK004, nmk004_device)

#endif // MAME_NMK_NMK004_H

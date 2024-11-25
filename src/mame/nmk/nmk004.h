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

	auto reset_cb() { return m_reset_cb.bind(); }

	// host interface
	void write(uint8_t data);
	uint8_t read();
	void nmi_w(int state) { m_cpu->set_input_line(INPUT_LINE_NMI, state); }

	void port4_w(uint8_t data);
	void ym2203_irq_handler(int irq);

	void nmk004_sound_mem_map(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<tlcs90_device>  m_cpu;
	devcb_write_line                m_reset_cb;

	uint8_t to_nmk004;
	uint8_t to_main;

	void oki0_bankswitch_w(uint8_t data);
	void oki1_bankswitch_w(uint8_t data);
	uint8_t tonmk004_r();
	void tomain_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(NMK004, nmk004_device)

#endif // MAME_NMK_NMK004_H

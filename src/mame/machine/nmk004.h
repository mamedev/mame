// license:BSD-3-Clause
// copyright-holders:David Haywood,Alex Marshall
/***************************************************************************

 NMK004 emulation

***************************************************************************/

#ifndef MAME_MACHINE_NMK004_H
#define MAME_MACHINE_NMK004_H

#include "cpu/tlcs90/tlcs90.h"

#pragma once

class nmk004_device : public device_t
{
public:
	nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto reset_cb() { return m_reset_cb.bind(); }

	// host interface
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_cpu->set_input_line(INPUT_LINE_NMI, state); }


	DECLARE_WRITE8_MEMBER(nmk004_port4_w);
	DECLARE_WRITE8_MEMBER(nmk004_oki0_bankswitch_w);
	DECLARE_WRITE8_MEMBER(nmk004_oki1_bankswitch_w);
	DECLARE_READ8_MEMBER(nmk004_tonmk004_r);
	DECLARE_WRITE8_MEMBER(nmk004_tomain_w);
	void ym2203_irq_handler(int irq);

	void nmk004_sound_mem_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	required_device<tlcs90_device>  m_cpu;
	devcb_write_line                m_reset_cb;

	uint8_t to_nmk004;
	uint8_t to_main;
};

DECLARE_DEVICE_TYPE(NMK004, nmk004_device)

#endif // MAME_MACHINE_NMK004_H

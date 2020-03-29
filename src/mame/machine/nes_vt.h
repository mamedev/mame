// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_NES_VT_H
#define MAME_MACHINE_NES_VT_H

#pragma once

#include "cpu/m6502/n2a03.h"
#include "machine/m6502_vtscr.h"
#include "machine/m6502_swap_op_d5_d6.h"
#include "video/ppu2c0x_vt.h"
#include "screen.h"
#include "speaker.h"

class nes_vt_soc_device : public device_t, public device_memory_interface
{
public:
	nes_vt_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void program_map(address_map &map);
protected:
	virtual void device_start() override;
	virtual space_config_vector memory_space_config() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<ppu_vt03_device> m_ppu;
	required_device<nesapu_device> m_apu;

	void nes_vt_map(address_map& map);

	/* OneBus read callbacks for getting sprite and tile data during rendering */
	DECLARE_READ8_MEMBER(spr_r);
	DECLARE_READ8_MEMBER(chr_r);

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

private:
	const address_space_config m_program_space_config;

	DECLARE_WRITE_LINE_MEMBER(apu_irq);
	DECLARE_READ8_MEMBER(apu_read_mem);

};

DECLARE_DEVICE_TYPE(NES_VT_SOC, nes_vt_soc_device)

#endif // MAME_MACHINE_NES_VT_H

// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/
#ifndef MAME_INCLUDES_DRIBLING_H
#define MAME_INCLUDES_DRIBLING_H

#pragma once

#include "machine/i8255.h"
#include "machine/watchdog.h"
#include "emupal.h"

class dribling_state : public driver_device
{
public:
	dribling_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_watchdog(*this, "watchdog"),
		m_ppi8255(*this, "ppi8255%d", 0),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_mux(*this, "MUX%u", 0),
		m_proms(*this, "proms"),
		m_gfxroms(*this, "gfx")
	{ }

	void dribling(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device_array<i8255_device, 2>  m_ppi8255;
	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_ioport_array<3> m_mux;
	required_region_ptr<uint8_t> m_proms;
	required_region_ptr<uint8_t> m_gfxroms;

	// misc
	uint8_t    m_abca = 0U;
	uint8_t    m_dr = 0U;
	uint8_t    m_ds = 0U;
	uint8_t    m_sh = 0U;
	uint8_t    m_input_mux = 0U;
	uint8_t    m_di = 0U;

	uint8_t ioread(offs_t offset);
	void iowrite(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t dsr_r();
	uint8_t input_mux0_r();
	void misc_w(uint8_t data);
	void sound_w(uint8_t data);
	void pb_w(uint8_t data);
	void shr_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(irq_gen);
	void prg_map(address_map &map);
	void io_map(address_map &map);
};

#endif // MAME_INCLUDES_DRIBLING_H

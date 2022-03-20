// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Epos games

**************************************************************************/
#ifndef MAME_INCLUDES_EPOS_H
#define MAME_INCLUDES_EPOS_H

#pragma once

#include "emupal.h"

class epos_state : public driver_device
{
public:
	epos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_inputs(*this, { "INPUTS", "INPUTS2" }),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_leds(*this, "led%u", 0U)
	{ }

	void epos(machine_config &config);
	void dealer(machine_config &config);

	void init_dealer();

protected:
	virtual void machine_start() override { m_leds.resolve(); }
	virtual void machine_reset() override;

private:
	void dealer_decrypt_rom(offs_t offset, uint8_t data);
	void port_1_w(uint8_t data);
	uint8_t i8255_porta_r();
	void i8255_portc_w(uint8_t data);
	uint8_t ay_porta_mpx_r();
	void flip_screen_w(uint8_t data);
	void dealer_pal_w(offs_t offset, uint8_t data);
	DECLARE_MACHINE_START(epos);
	DECLARE_MACHINE_START(dealer);
	void epos_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	static void set_pal_color(palette_device &palette, uint8_t offset, uint8_t data); // TODO: convert to an RGB converter and set_format
	void dealer_io_map(address_map &map);
	void dealer_map(address_map &map);
	void epos_io_map(address_map &map);
	void epos_map(address_map &map);

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_ioport_array<2> m_inputs;

	/* video-related */
	uint8_t    m_palette_bank = 0U;

	/* misc */
	int      m_counter = 0;
	int      m_input_multiplex = 0;
	bool     m_ay_porta_multiplex = false;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	output_finder<2> m_leds;
};

#endif // MAME_INCLUDES_EPOS_H

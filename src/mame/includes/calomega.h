// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
#ifndef MAME_INCLUDES_CALOMEGA_H
#define MAME_INCLUDES_CALOMEGA_H

#pragma once

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "emupal.h"

class calomega_state : public driver_device
{
public:
	calomega_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pia(*this, "pia%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_acia6850_0(*this, "acia6850_0"),
		m_aciabaud(*this, "aciabaud"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_in0(*this, "IN0"),
		m_in0_0(*this, "IN0-0"),
		m_in0_1(*this, "IN0-1"),
		m_in0_2(*this, "IN0-2"),
		m_in0_3(*this, "IN0-3"),
		m_frq(*this, "FRQ"),
		m_sw2(*this, "SW2"),
		m_lamps(*this, "lamp%u", 1U)
	{ }

	void init_sys903();
	void init_comg080();
	void init_s903mod();
	void init_sys905();

	void sys905(machine_config &config);
	void s903mod(machine_config &config);
	void sys906(machine_config &config);
	void sys903(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override;

private:
	DECLARE_WRITE8_MEMBER(calomega_videoram_w);
	DECLARE_WRITE8_MEMBER(calomega_colorram_w);
	DECLARE_READ8_MEMBER(s903_mux_port_r);
	DECLARE_WRITE8_MEMBER(s903_mux_w);
	DECLARE_READ8_MEMBER(s905_mux_port_r);
	DECLARE_WRITE8_MEMBER(s905_mux_w);
	DECLARE_READ8_MEMBER(pia0_ain_r);
	DECLARE_READ8_MEMBER(pia0_bin_r);
	DECLARE_WRITE8_MEMBER(pia0_aout_w);
	DECLARE_WRITE8_MEMBER(pia0_bout_w);
	DECLARE_WRITE_LINE_MEMBER(pia0_ca2_w);
	DECLARE_READ8_MEMBER(pia1_ain_r);
	DECLARE_READ8_MEMBER(pia1_bin_r);
	DECLARE_WRITE8_MEMBER(pia1_aout_w);
	DECLARE_WRITE8_MEMBER(pia1_bout_w);
	DECLARE_WRITE8_MEMBER(lamps_903a_w);
	DECLARE_WRITE8_MEMBER(lamps_903b_w);
	DECLARE_WRITE8_MEMBER(lamps_905_w);
	DECLARE_WRITE_LINE_MEMBER(write_acia_tx);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(update_aciabaud_scale);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_calomega(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void calomega_palette(palette_device &palette) const;

	void s903mod_map(address_map &map);
	void sys903_map(address_map &map);
	void sys905_map(address_map &map);
	void sys906_map(address_map &map);

	optional_device_array<pia6821_device, 2> m_pia;

	required_device<cpu_device> m_maincpu;
	optional_device<acia6850_device> m_acia6850_0;
	optional_device<clock_device> m_aciabaud;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	optional_ioport m_in0;
	optional_ioport m_in0_0;
	optional_ioport m_in0_1;
	optional_ioport m_in0_2;
	optional_ioport m_in0_3;
	optional_ioport m_frq;
	optional_ioport m_sw2;
	output_finder<9> m_lamps;

	uint8_t m_tx_line;
	int m_s903_mux_data;
	int m_s905_mux_data;
	tilemap_t *m_bg_tilemap;
};

#endif // MAME_INCLUDES_CALOMEGA_H

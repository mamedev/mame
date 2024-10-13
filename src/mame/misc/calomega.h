// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

#ifndef MAME_MISC_CALOMEGA_H
#define MAME_MISC_CALOMEGA_H

#pragma once

#include "cpu/m6502/m6502.h"
#include "cpu/m6502/r65c02.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

class calomega_state : public driver_device
{
public:
	calomega_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pia(*this, "pia%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_kstec(*this, "kstec"),
		m_uart(*this, "uart"),
		m_key_row(*this, "KB_%u", 0),
		m_acia6850(*this, "acia6850_%u", 0U),
		m_aciabaud(*this, "aciabaud"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_hopper(*this, "hopper"),
		m_in0(*this, "IN0"),
		m_in0_0(*this, "IN0-0"),
		m_in0_1(*this, "IN0-1"),
		m_in0_2(*this, "IN0-2"),
		m_in0_3(*this, "IN0-3"),
		m_frq(*this, "FRQ"),
		m_sw2(*this, "SW2"),
		m_lamps(*this, "lamp%u", 1U),
		m_red(*this, "POT1_RED"),
		m_grn(*this, "POT2_GREEN"),
		m_blu(*this, "POT3_BLUE")

	{
	}

	void init_comg079();
	void init_comg080();
	void init_comg145();
	void init_comg176();
	void init_any();

	void sys903(machine_config &config);
	void s903mod(machine_config &config);
	void sys903kb(machine_config &config);
	void sys905(machine_config &config);
	void sys906(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	void calomega_videoram_w(offs_t offset, uint8_t data);
	void calomega_colorram_w(offs_t offset, uint8_t data);
	uint8_t s903_mux_port_r();
	void s903_mux_w(uint8_t data);
	uint8_t s905_mux_port_r();
	void s905_mux_w(uint8_t data);
	uint8_t pia0_bin_r();
	void pia0_aout_w(uint8_t data);
	void pia0_bout_w(uint8_t data);
	uint8_t pia1_ain_r();
	uint8_t pia1_bin_r();
	uint8_t dummy_pia_r();
	void pia1_aout_w(uint8_t data);
	void pia1_bout_w(uint8_t data);
	void lamps_903a_w(uint8_t data);
	void lamps_903b_w(uint8_t data);
	void lamps_905_w(uint8_t data);
	void dummy_pia_w(uint8_t data);
	uint8_t keyb_903_r();

	void pia1_cb2_w(int state);
	void vblank0_w(int state);
	void vblank1_w(int state);
	void vblank2_w(int state);
	void dummy_pia_line_w(int state);
	void write_acia_clock(int state);
	void w_903kb_acia_clock(int state);
	void update_aciabaud_scale(int state);

	TIMER_DEVICE_CALLBACK_MEMBER(timer_0);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_1);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_2);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update_calomega(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void calomega_palette(palette_device &palette) const;

	void sys903_map(address_map &map) ATTR_COLD;
	void s903mod_map(address_map &map) ATTR_COLD;
	void sys905_map(address_map &map) ATTR_COLD;
	void sys906_map(address_map &map) ATTR_COLD;
	void kstec_mem_map(address_map &map) ATTR_COLD;
	void kstec_io_map(address_map &map) ATTR_COLD;

	optional_device_array<pia6821_device, 2> m_pia;
	required_device<m6502_device> m_maincpu;
	optional_device<i8035_device> m_kstec;
	optional_device<i8251_device> m_uart;
	optional_ioport_array<16> m_key_row;
	optional_device_array<acia6850_device, 1> m_acia6850;  // keep array mode for future implementations
	optional_device<clock_device> m_aciabaud;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	required_device<ticket_dispenser_device> m_hopper;

	optional_ioport m_in0;
	optional_ioport m_in0_0;
	optional_ioport m_in0_1;
	optional_ioport m_in0_2;
	optional_ioport m_in0_3;

	optional_ioport m_frq;
	optional_ioport m_sw2;

	output_finder<9> m_lamps;
	required_ioport m_red;
	required_ioport m_grn;
	required_ioport m_blu;

	uint8_t m_timer = 0U;
	int m_s903_mux_data = 0;
	int m_s905_mux_data = 0;
	int m_pia_data = 0;
	bool m_lockout = false;
	bool m_diverter = false;
	int m_kbscan = 0;
	int m_rxrdy = 0;
	int r_pot = 0;
	int g_pot = 0;
	int b_pot = 0;

	tilemap_t *m_bg_tilemap = nullptr;
};

#endif // MAME_MISC_CALOMEGA_H

// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_INCLUDES_PASTELG_H
#define MAME_INCLUDES_PASTELG_H

#pragma once

#include "machine/nb1413m3.h"
#include "emupal.h"
#include "screen.h"

class pastelg_state : public driver_device
{
public:
	pastelg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_clut(*this, "clut"),
		m_blitter_rom(*this, "blitter"),
		m_voice_rom(*this, "voice"),
		m_p1_keys(*this, "PL1_KEY%u", 0U),
		m_p2_keys(*this, "PL2_KEY%u", 0U)
	{ }

	void threeds(machine_config &config);
	void pastelg(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_clut;
	required_region_ptr<uint8_t> m_blitter_rom;
	optional_region_ptr<uint8_t> m_voice_rom;
	optional_ioport_array<5> m_p1_keys;
	optional_ioport_array<5> m_p2_keys;

	uint8_t m_mux_data;
	uint8_t m_blitter_destx;
	uint8_t m_blitter_desty;
	uint8_t m_blitter_sizex;
	uint8_t m_blitter_sizey;
	uint16_t m_blitter_src_addr;
	uint8_t m_gfxbank;
	bool m_dispflag;
	bool m_flipscreen;
	bool m_blitter_direction_x;
	bool m_blitter_direction_y;
	uint8_t m_palbank;
	std::unique_ptr<uint8_t[]> m_videoram;
	bool m_flipscreen_old;
	emu_timer *m_blitter_timer;

	uint8_t irq_ack_r();
	void blitter_w(offs_t offset, uint8_t data);
	void blitter_timer_callback(void *ptr, s32 param);

	uint8_t pastelg_sndrom_r();
	void pastelg_romsel_w(address_space &space, uint8_t data);

	uint8_t threeds_inputport1_r();
	uint8_t threeds_inputport2_r();
	void threeds_inputportsel_w(uint8_t data);
	void threeds_romsel_w(uint8_t data);
	void threeds_output_w(uint8_t data);
	uint8_t threeds_rom_readback_r();

	virtual void machine_start() override;
	virtual void video_start() override;

	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vramflip();
	void gfxdraw();

	uint16_t pastelg_blitter_src_addr_r();

	void prg_map(address_map &map);
	void pastelg_io_map(address_map &map);
	void threeds_io_map(address_map &map);
};

#endif // MAME_INCLUDES_PASTELG_H

// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_INCLUDES_PASTELG_H
#define MAME_INCLUDES_PASTELG_H

#pragma once

#include "machine/nb1413m3.h"
#include "emupal.h"
#include "screen.h"

class pastelg_common_state : public driver_device
{
public:
	pastelg_common_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_blitter_rom(*this, "blitter"),
		m_clut(*this, "clut")
	{ }

protected:
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_region_ptr<uint8_t> m_blitter_rom;

	uint8_t m_gfxbank;
	uint8_t m_palbank;
	uint16_t m_blitter_src_addr;

	uint8_t irq_ack_r();
	void blitter_w(offs_t offset, uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map);

private:
	required_shared_ptr<uint8_t> m_clut;

	uint8_t m_blitter_destx;
	uint8_t m_blitter_desty;
	uint8_t m_blitter_sizex;
	uint8_t m_blitter_sizey;
	bool m_dispflag;
	bool m_flipscreen;
	bool m_blitter_direction_x;
	bool m_blitter_direction_y;
	std::unique_ptr<uint8_t[]> m_videoram;
	bool m_flipscreen_old;
	emu_timer *m_blitter_timer;

	void blitter_timer_callback(s32 param);

	void vramflip();
	void gfxdraw();
};

class pastelg_state : public pastelg_common_state
{
public:
	pastelg_state(const machine_config &mconfig, device_type type, const char *tag) :
		pastelg_common_state(mconfig, type, tag),
		m_voice_rom(*this, "voice")
	{ }

	void pastelg(machine_config &config);

private:
	required_region_ptr<uint8_t> m_voice_rom;

	uint8_t sndrom_r();
	void romsel_w(uint8_t data);
	uint16_t blitter_src_addr_r();

	void io_map(address_map &map);
};

class threeds_state : public pastelg_common_state
{
public:
	threeds_state(const machine_config &mconfig, device_type type, const char *tag) :
		pastelg_common_state(mconfig, type, tag),
		m_p1_keys(*this, "PL1_KEY%u", 0U),
		m_p2_keys(*this, "PL2_KEY%u", 0U)
	{ }

	void threeds(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_ioport_array<5> m_p1_keys;
	required_ioport_array<5> m_p2_keys;

	uint8_t m_mux_data;

	uint8_t inputport1_r();
	uint8_t inputport2_r();
	void inputportsel_w(uint8_t data);
	void romsel_w(uint8_t data);
	void output_w(uint8_t data);
	uint8_t rom_readback_r();

	void io_map(address_map &map);
};

#endif // MAME_INCLUDES_PASTELG_H

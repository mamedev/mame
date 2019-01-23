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
	enum
	{
		TIMER_BLITTER
	};

	pastelg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_nb1413m3(*this, "nb1413m3"),
		m_screen(*this, "screen"),
		m_clut(*this, "clut")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<nb1413m3_device> m_nb1413m3;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_clut;

	uint8_t m_mux_data;
	int m_blitter_destx;
	int m_blitter_desty;
	int m_blitter_sizex;
	int m_blitter_sizey;
	int m_blitter_src_addr;
	int m_gfxrom;
	int m_dispflag;
	int m_flipscreen;
	int m_blitter_direction_x;
	int m_blitter_direction_y;
	int m_palbank;
	std::unique_ptr<uint8_t[]> m_videoram;
	int m_flipscreen_old;
	emu_timer *m_blitter_timer;

	DECLARE_READ8_MEMBER(pastelg_sndrom_r);
	DECLARE_READ8_MEMBER(pastelg_irq_ack_r);
	DECLARE_READ8_MEMBER(threeds_inputport1_r);
	DECLARE_READ8_MEMBER(threeds_inputport2_r);
	DECLARE_WRITE8_MEMBER(threeds_inputportsel_w);
	DECLARE_WRITE8_MEMBER(pastelg_blitter_w);
	DECLARE_WRITE8_MEMBER(threeds_romsel_w);
	DECLARE_WRITE8_MEMBER(threeds_output_w);
	DECLARE_READ8_MEMBER(threeds_rom_readback_r);
	DECLARE_WRITE8_MEMBER(pastelg_romsel_w);

	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_busyflag_r);
	DECLARE_CUSTOM_INPUT_MEMBER(nb1413m3_hackbusyflag_r);

	virtual void machine_start() override;
	virtual void video_start() override;

	void pastelg_palette(palette_device &palette) const;
	uint32_t screen_update_pastelg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int pastelg_blitter_src_addr_r();
	void pastelg_vramflip();
	void pastelg_gfxdraw();

	void threeds(machine_config &config);
	void pastelg(machine_config &config);
	void pastelg_io_map(address_map &map);
	void pastelg_map(address_map &map);
	void threeds_io_map(address_map &map);
protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_PASTELG_H

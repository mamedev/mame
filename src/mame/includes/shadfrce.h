// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_SHADFRCE_H
#define MAME_INCLUDES_SHADFRCE_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"

class shadfrce_state : public driver_device
{
public:
	shadfrce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_dsw1(*this, "DSW1"),
		m_io_dsw2(*this, "DSW2"),
		m_io_other(*this, "OTHER"),
		m_io_extra(*this, "EXTRA"),
		m_io_misc(*this, "MISC"),
		m_io_system(*this, "SYSTEM"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bg0videoram(*this, "bg0videoram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_spvideoram(*this, "spvideoram")
	{ }

	void shadfrce(machine_config &config);

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_ioport m_io_p1;
	required_ioport m_io_p2;
	required_ioport m_io_dsw1;
	required_ioport m_io_dsw2;
	required_ioport m_io_other;
	required_ioport m_io_extra;
	required_ioport m_io_misc;
	required_ioport m_io_system;

	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bg0videoram;
	required_shared_ptr<uint16_t> m_bg1videoram;
	required_shared_ptr<uint16_t> m_spvideoram;

	std::unique_ptr<uint16_t[]> m_spvideoram_old;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_bg0tilemap;
	tilemap_t *m_bg1tilemap;
	int m_video_enable;
	int m_irqs_enable;
	int m_raster_scanline;
	int m_raster_irq_enable;
	int m_vblank;
	int m_prev_value;

	DECLARE_WRITE16_MEMBER(flip_screen);
	DECLARE_READ16_MEMBER(input_ports_r);
	DECLARE_WRITE8_MEMBER(screen_brt_w);
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE16_MEMBER(irq_w);
	DECLARE_WRITE16_MEMBER(scanline_w);
	DECLARE_WRITE16_MEMBER(fgvideoram_w);
	DECLARE_WRITE16_MEMBER(bg0videoram_w);
	DECLARE_WRITE16_MEMBER(bg1videoram_w);
	DECLARE_WRITE16_MEMBER(bg0scrollx_w);
	DECLARE_WRITE16_MEMBER(bg0scrolly_w);
	DECLARE_WRITE16_MEMBER(bg1scrollx_w);
	DECLARE_WRITE16_MEMBER(bg1scrolly_w);
	DECLARE_WRITE8_MEMBER(oki_bankswitch_w);

	TILE_GET_INFO_MEMBER(get_fgtile_info);
	TILE_GET_INFO_MEMBER(get_bg0tile_info);
	TILE_GET_INFO_MEMBER(get_bg1tile_info);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void shadfrce_map(address_map &map);
	void shadfrce_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_SHADFRCE_H

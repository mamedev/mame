// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Pandora's Palace

*************************************************************************/
#ifndef MAME_INCLUDES_PANDORAS_H
#define MAME_INCLUDES_PANDORAS_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "emupal.h"
#include "tilemap.h"

class pandoras_state : public driver_device
{
public:
	pandoras_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t     *m_layer0;
	int         m_flipscreen;

	int m_irq_enable_a;
	int m_irq_enable_b;
	int m_firq_old_data_a;
	int m_firq_old_data_b;
	int m_i8039_status;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE_LINE_MEMBER(cpua_irq_enable_w);
	DECLARE_WRITE_LINE_MEMBER(cpub_irq_enable_w);
	DECLARE_WRITE8_MEMBER(pandoras_cpua_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(pandoras_cpub_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(pandoras_i8039_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(i8039_irqen_and_status_w);
	DECLARE_WRITE8_MEMBER(pandoras_z80_irqtrigger_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(pandoras_vram_w);
	DECLARE_WRITE8_MEMBER(pandoras_cram_w);
	DECLARE_WRITE8_MEMBER(pandoras_scrolly_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_READ8_MEMBER(pandoras_portA_r);
	DECLARE_READ8_MEMBER(pandoras_portB_r);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void pandoras_palette(palette_device &palette) const;
	uint32_t screen_update_pandoras(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t* sr );
	void pandoras(machine_config &config);
	void pandoras_i8039_io_map(address_map &map);
	void pandoras_i8039_map(address_map &map);
	void pandoras_master_map(address_map &map);
	void pandoras_slave_map(address_map &map);
	void pandoras_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_PANDORAS_H

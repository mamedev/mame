// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Couriersud
/*************************************************************************

    Gyruss

*************************************************************************/
#ifndef MAME_INCLUDES_GYRUSS_H
#define MAME_INCLUDES_GYRUSS_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class gyruss_state : public driver_device
{
public:
	gyruss_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_audiocpu_2(*this, "audio2"),
		m_discrete(*this, "discrete"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void gyruss(machine_config &config);

	void init_gyruss();

private:
	/* devices/memory pointers */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8039_device> m_audiocpu_2;
	required_device<discrete_sound_device> m_discrete;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	tilemap_t *m_tilemap = nullptr;
	uint8_t m_master_nmi_mask = 0U;
	uint8_t m_slave_irq_mask = 0U;
	bool m_flipscreen = false;

	void gyruss_irq_clear_w(uint8_t data);
	void gyruss_sh_irqtrigger_w(uint8_t data);
	void gyruss_i8039_irq_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(master_nmi_mask_w);
	void slave_irq_mask_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	void gyruss_spriteram_w(offs_t offset, uint8_t data);
	uint8_t gyruss_scanline_r();
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	uint8_t gyruss_portA_r();
	void gyruss_dac_w(uint8_t data);
	void gyruss_filter0_w(uint8_t data);
	void gyruss_filter1_w(uint8_t data);
	TILE_GET_INFO_MEMBER(gyruss_get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	void gyruss_palette(palette_device &palette) const;
	uint32_t screen_update_gyruss(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void filter_w(int chip, int data );
	void audio_cpu1_io_map(address_map &map);
	void audio_cpu1_map(address_map &map);
	void audio_cpu2_io_map(address_map &map);
	void audio_cpu2_map(address_map &map);
	void main_cpu1_map(address_map &map);
	void main_cpu2_map(address_map &map);
};

#endif // MAME_INCLUDES_GYRUSS_H

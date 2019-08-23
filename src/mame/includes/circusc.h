// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Chris Hardy, Couriersud
/*************************************************************************

    Circus Charlie

*************************************************************************/
#ifndef MAME_INCLUDES_CIRCUSC_H
#define MAME_INCLUDES_CIRCUSC_H

#pragma once

#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "tilemap.h"

class circusc_state : public driver_device
{
public:
	circusc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_spriteram(*this, "spriteram"),
		m_audiocpu(*this, "audiocpu"),
		m_sn_1(*this, "sn1"),
		m_sn_2(*this, "sn2"),
		m_dac(*this, "dac"),
		m_discrete(*this, "fltdisc"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	bool             m_spritebank;

	/* sound-related */
	uint8_t          m_sn_latch;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<sn76496_device> m_sn_1;
	required_device<sn76496_device> m_sn_2;
	required_device<dac_byte_interface> m_dac;
	required_device<discrete_device> m_discrete;

	bool             m_irq_mask;

	DECLARE_READ8_MEMBER(circusc_sh_timer_r);
	DECLARE_WRITE8_MEMBER(circusc_sh_irqtrigger_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_counter_2_w);
	DECLARE_WRITE8_MEMBER(circusc_sound_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(circusc_videoram_w);
	DECLARE_WRITE8_MEMBER(circusc_colorram_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);
	DECLARE_WRITE_LINE_MEMBER(spritebank_w);
	void init_circusc();
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void circusc_palette(palette_device &palette) const;
	uint32_t screen_update_circusc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void circusc(machine_config &config);
	void circusc_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CIRCUSC_H

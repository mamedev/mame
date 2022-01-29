// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

  Knuckle Joe

*************************************************************************/

#ifndef MAME_INCLUDES_KNCLJOE_H
#define MAME_INCLUDES_KNCLJOE_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class kncljoe_state : public driver_device
{
public:
	kncljoe_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrollregs(*this, "scrollregs"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ay8910(*this, "aysnd"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void kncljoe(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrollregs;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tile_bank;
	int         m_sprite_bank;
	int        m_flipscreen;

	/* misc */
	uint8_t      m_port1;
	uint8_t      m_port2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<m6803_cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ay8910_device> m_ay8910;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_cmd_w(uint8_t data);
	void sound_irq_ack_w(uint8_t data);
	void kncljoe_videoram_w(offs_t offset, uint8_t data);
	void kncljoe_control_w(uint8_t data);
	void kncljoe_scroll_w(offs_t offset, uint8_t data);
	void m6803_port1_w(uint8_t data);
	void m6803_port2_w(uint8_t data);
	uint8_t m6803_port1_r();
	uint8_t m6803_port2_r();
	void unused_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void kncljoe_palette(palette_device &palette) const;
	uint32_t screen_update_kncljoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_nmi);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_KNCLJOE_H

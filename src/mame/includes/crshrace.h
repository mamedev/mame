// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_CRSHRACE_H
#define MAME_INCLUDES_CRSHRACE_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "video/vsystem_spr.h"
#include "video/k053936.h"
#include "emupal.h"
#include "tilemap.h"

class crshrace_state : public driver_device
{
public:
	crshrace_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_z80bank(*this, "bank1"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_spr(*this, "vsystem_spr"),
		m_k053936(*this, "k053936"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_videoram1;
	required_shared_ptr<uint16_t> m_videoram2;

	required_memory_bank m_z80bank;

	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<vsystem_spr_device> m_spr;
	required_device<k053936_device> m_k053936;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	/* video-related */
	tilemap_t   *m_tilemap1;
	tilemap_t   *m_tilemap2;
	int       m_roz_bank;
	int       m_gfxctrl;
	int       m_flipscreen;
	uint32_t crshrace_tile_callback( uint32_t code );

	/* devices */
	DECLARE_WRITE8_MEMBER(crshrace_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(crshrace_videoram1_w);
	DECLARE_WRITE16_MEMBER(crshrace_videoram2_w);
	DECLARE_WRITE16_MEMBER(crshrace_roz_bank_w);
	DECLARE_WRITE16_MEMBER(crshrace_gfxctrl_w);
	void init_crshrace2();
	void init_crshrace();
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_crshrace(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void crshrace(machine_config &config);
	void crshrace_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_CRSHRACE_H

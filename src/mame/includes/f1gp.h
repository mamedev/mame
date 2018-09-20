// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_F1GP_H
#define MAME_INCLUDES_F1GP_H

#pragma once

#include "machine/6850acia.h"
#include "machine/gen_latch.h"
#include "video/vsystem_spr.h"
#include "video/vsystem_spr2.h"
#include "video/k053936.h"
#include "emupal.h"

class f1gp_state : public driver_device
{
public:
	f1gp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_sharedram(*this, "sharedram"),
		m_spr1vram(*this, "spr1vram"),
		m_spr2vram(*this, "spr2vram"),
		m_spr1cgram(*this, "spr1cgram"),
		m_spr2cgram(*this, "spr2cgram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_rozvideoram(*this, "rozvideoram"),
		m_sprcgram(*this, "sprcgram"),
		m_spritelist(*this, "spritelist"),
		m_spriteram(*this, "spriteram"),
		m_fgregs(*this, "fgregs"),
		m_rozregs(*this, "rozregs"),
		m_z80bank(*this, "bank1"),
		m_maincpu(*this, "maincpu"),
		m_spr_old(*this, "vsystem_spr_old"),
		m_spr_old2(*this, "vsystem_spr_ol2"),
		m_spr(*this, "vsystem_spr"),
		m_gfxdecode(*this, "gfxdecode"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_acia(*this, "acia")
	{ }

	void f1gp2(machine_config &config);
	void f1gpb(machine_config &config);
	void f1gp(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_sharedram;
	optional_shared_ptr<uint16_t> m_spr1vram;
	optional_shared_ptr<uint16_t> m_spr2vram;
	optional_shared_ptr<uint16_t> m_spr1cgram;
	optional_shared_ptr<uint16_t> m_spr2cgram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_rozvideoram;
	optional_shared_ptr<uint16_t> m_sprcgram;
	optional_shared_ptr<uint16_t> m_spritelist;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_fgregs;
	optional_shared_ptr<uint16_t> m_rozregs;

	optional_memory_bank m_z80bank;

	uint16_t *  m_zoomdata;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_roz_tilemap;
	int       m_roz_bank;
	int       m_flipscreen;
	int       m_gfxctrl;
	int       m_scroll[2];
	uint32_t f1gp2_tile_callback( uint32_t code );
	uint32_t f1gp_old_tile_callback( uint32_t code );
	uint32_t f1gp_ol2_tile_callback( uint32_t code );

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<vsystem_spr2_device> m_spr_old; // f1gp
	optional_device<vsystem_spr2_device> m_spr_old2; // f1gp
	optional_device<vsystem_spr_device> m_spr; // f1gp2
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<cpu_device> m_audiocpu;
	optional_device<k053936_device> m_k053936;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // not f1gpb
	required_device<acia6850_device> m_acia;

	DECLARE_WRITE8_MEMBER(f1gp_sh_bankswitch_w);
	DECLARE_READ8_MEMBER(command_pending_r);
	DECLARE_WRITE16_MEMBER(f1gpb_misc_w);
	DECLARE_READ16_MEMBER(f1gp_zoomdata_r);
	DECLARE_WRITE16_MEMBER(f1gp_zoomdata_w);
	DECLARE_READ16_MEMBER(f1gp_rozvideoram_r);
	DECLARE_WRITE16_MEMBER(f1gp_rozvideoram_w);
	DECLARE_WRITE16_MEMBER(f1gp_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(f1gp_fgscroll_w);
	DECLARE_WRITE16_MEMBER(f1gp_gfxctrl_w);
	DECLARE_WRITE16_MEMBER(f1gp2_gfxctrl_w);
	TILE_GET_INFO_MEMBER(f1gp_get_roz_tile_info);
	TILE_GET_INFO_MEMBER(f1gp2_get_roz_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_MACHINE_START(f1gp);
	DECLARE_MACHINE_RESET(f1gp);
	DECLARE_VIDEO_START(f1gp);
	DECLARE_MACHINE_START(f1gpb);
	DECLARE_VIDEO_START(f1gpb);
	DECLARE_VIDEO_START(f1gp2);
	uint32_t screen_update_f1gp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_f1gpb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_f1gp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1gpb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect );
	void f1gp2_cpu1_map(address_map &map);
	void f1gp_cpu1_map(address_map &map);
	void f1gp_cpu2_map(address_map &map);
	void f1gpb_cpu1_map(address_map &map);
	void f1gpb_cpu2_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_F1GP_H

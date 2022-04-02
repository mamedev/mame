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
#include "tilemap.h"

class f1gp_state : public driver_device
{
public:
	f1gp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_sharedram(*this, "sharedram"),
		m_sprvram(*this, "spr%uvram", 1U),
		m_sprcgram(*this, "spr%ucgram", 1U),
		m_fgvideoram(*this, "fgvideoram"),
		m_rozvideoram(*this, "rozvideoram"),
		m_spriteram(*this, "spriteram"),
		m_fgregs(*this, "fgregs"),
		m_rozregs(*this, "rozregs"),
		m_z80bank(*this, "z80bank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_acia(*this, "acia"),
		m_rozgfxram(*this, "rozgfxram"),
		m_spr_old(*this, "vsystem_spr_old%u", 1U)
	{ }

	void f1gpb(machine_config &config);
	void f1gp(machine_config &config);

protected:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_sharedram;
	optional_shared_ptr_array<uint16_t, 2> m_sprvram;
	optional_shared_ptr_array<uint16_t, 2> m_sprcgram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_rozvideoram;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_fgregs;
	optional_shared_ptr<uint16_t> m_rozregs;

	optional_memory_bank m_z80bank;

	/* video-related */
	tilemap_t   *m_fg_tilemap = nullptr;
	tilemap_t   *m_roz_tilemap = nullptr;
	int       m_flipscreen = 0;
	int       m_gfxctrl = 0;
	int       m_scroll[2]{};
	template<int Chip> uint32_t tile_callback( uint32_t code );

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<cpu_device> m_audiocpu;
	optional_device<k053936_device> m_k053936;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // not f1gpb
	required_device<acia6850_device> m_acia;

	void sh_bankswitch_w(uint8_t data);
	uint8_t command_pending_r();
	void rozvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fgscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void gfxctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void f1gp_cpu2_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);

private:
	/* memory pointers */
	optional_shared_ptr<uint16_t> m_rozgfxram;

	/* devices */
	optional_device_array<vsystem_spr2_device, 2> m_spr_old; // f1gp

	void f1gpb_misc_w(uint16_t data);
	void rozgfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_roz_tile_info);

	virtual void video_start() override;

	uint32_t screen_update_f1gp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_f1gpb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1gpb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect );
	void f1gp_cpu1_map(address_map &map);
	void f1gpb_cpu1_map(address_map &map);
	void f1gpb_cpu2_map(address_map &map);
};

class f1gp2_state : public f1gp_state
{
public:
	f1gp2_state(const machine_config &mconfig, device_type type, const char *tag) :
		f1gp_state(mconfig, type, tag),
		m_spr(*this, "vsystem_spr")
	{ }

	void f1gp2(machine_config &config);

private:
	/* video-related */
	int       m_roz_bank = 0;

	/* devices */
	optional_device<vsystem_spr_device> m_spr; // f1gp2

	void rozbank_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_roz_tile_info);

	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void f1gp2_cpu1_map(address_map &map);
};

#endif // MAME_INCLUDES_F1GP_H

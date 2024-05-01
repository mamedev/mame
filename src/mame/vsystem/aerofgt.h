// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VSYSTEM_AEROFGT_H
#define MAME_VSYSTEM_AEROFGT_H

#pragma once

#include "vsystem_spr.h"
#include "vsystem_spr2.h"

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/upd7759.h"

#include "emupal.h"
#include "tilemap.h"


class aerofgt_state : public driver_device
{
public:
	aerofgt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "vram.%u", 0)
		, m_rasterram(*this, "rasterram")
		, m_bitmapram(*this, "bitmapram")
		, m_sprlookupram(*this, "sprlookupram%u", 1)
		, m_spriteram(*this, "spriteram")
		, m_tx_tilemap_ram(*this, "tx_tilemap_ram")
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spr(*this, "vsystem_spr")
		, m_spr_old(*this, "vsystem_spr_old%u", 1)
	{ }

	void pspikesb(machine_config &config) ATTR_COLD;
	void pspikesc(machine_config &config) ATTR_COLD;
	void aerfboo2(machine_config &config) ATTR_COLD;

protected:
	/* memory pointers */
	optional_shared_ptr_array<uint16_t, 2> m_vram;
	optional_shared_ptr<uint16_t> m_rasterram;
	optional_shared_ptr<uint16_t> m_bitmapram;
	optional_shared_ptr_array<uint16_t, 2> m_sprlookupram;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_tx_tilemap_ram;

	/* devices referenced above */
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<vsystem_spr_device> m_spr; // only the aerofgt parent uses this chip
	optional_device_array<vsystem_spr2_device, 2> m_spr_old; // every other (non-bootleg) uses this or a pair of them..

	/* video-related */
	tilemap_t   *m_tilemap[2]{};
	uint8_t     m_gfxbank[8]{};
	uint16_t    m_bank[4]{};
	uint16_t    m_scrollx[2]{};
	uint16_t    m_scrolly[2]{};
	bool        m_flip_screen = false;
	int       m_charpalettebank = 0;
	int       m_spritepalettebank = 0;
	int       m_sprite_gfx = 0;

	uint32_t aerofgt_old_tile_callback(uint32_t code);

	/* handlers */
	template<int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pspikes_gfxbank_w(uint8_t data);
	void pspikesb_gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void karatblz_gfxbank_w(uint8_t data);
	void spinlbrk_gfxbank_w(uint8_t data);
	void kickball_gfxbank_w(uint8_t data);
	void turbofrc_gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void aerofgt_gfxbank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void scrollx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void scrolly_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pspikes_palette_bank_w(uint8_t data);
	void turbofrc_flip_screen_w(uint8_t data);
	void pspikesb_oki_banking_w(uint16_t data);
	void aerfboo2_okim6295_banking_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_pspikes_tile_info);
	template<int Layer> TILE_GET_INFO_MEMBER(karatblz_tile_info);
	template<int Layer> TILE_GET_INFO_MEMBER(spinlbrk_tile_info);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_VIDEO_START(pspikes) ATTR_COLD;
	DECLARE_VIDEO_START(karatblz) ATTR_COLD;
	DECLARE_VIDEO_START(turbofrc) ATTR_COLD;
	uint32_t screen_update_pspikes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pspikesb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aerfboot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aerfboo2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void aerofgt_register_state_globals(  );
	void setbank( int layer, int num, int bank );
	void aerfboo2_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, int chip_disabled_pri );
	void pspikesb_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void aerfboot_draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void pspikesb_map(address_map &map) ATTR_COLD;
	void pspikesc_map(address_map &map) ATTR_COLD;
	void aerfboo2_map(address_map &map) ATTR_COLD;
};


class aerofgt_sound_cpu_state : public aerofgt_state
{
public:
	aerofgt_sound_cpu_state(const machine_config &mconfig, device_type type, const char *tag)
		: aerofgt_state(mconfig, type, tag)
		, m_audiocpu(*this, "audiocpu")
		, m_upd7759(*this, "upd")
		, m_soundlatch(*this, "soundlatch")
		, m_okibank(*this, "okibank")
		, m_sprlookuprom(*this, "sprlookuprom")
	{
	}

	void init_banked_oki() ATTR_COLD;
	void init_kickball() ATTR_COLD;

	void spikes91(machine_config &config) ATTR_COLD;
	void kickball(machine_config &config) ATTR_COLD;
	void karatblzbl(machine_config &config) ATTR_COLD;
	void aerfboot(machine_config &config) ATTR_COLD;
	void wbbc97(machine_config &config) ATTR_COLD;

protected:
	required_device<cpu_device> m_audiocpu;
	optional_device<upd7759_device> m_upd7759; // karatblzbl
	required_device<generic_latch_8_device> m_soundlatch;

	optional_memory_bank m_okibank;

	optional_region_ptr<uint16_t> m_sprlookuprom;

	virtual void machine_start() override ATTR_COLD;

	DECLARE_VIDEO_START(wbbc97);

	uint8_t soundlatch_pending_r();
	void soundlatch_pending_w(int state);
	void spinlbrk_flip_screen_w(uint8_t data);

	uint32_t aerofgt_ol2_tile_callback(uint32_t code);

	uint32_t screen_update_karatblz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	uint16_t m_spikes91_lookup = 0;
	uint16_t m_wbbc97_bitmap_enable = 0;

	void spikes91_lookup_w(uint16_t data);
	void wbbc97_bitmap_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void aerfboot_okim6295_banking_w(uint8_t data);

	void karatblzbl_soundlatch_w(uint8_t data);
	void karatblzbl_d7759_write_port_0_w(uint8_t data);
	void karatblzbl_d7759_reset_w(uint8_t data);

	uint32_t screen_update_spikes91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_wbbc97(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void spikes91_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void wbbc97_draw_bitmap(bitmap_rgb32 &bitmap);

	void spikes91_map(address_map &map);
	void kickball_map(address_map &map);
	void karatblzbl_map(address_map &map);
	void aerfboot_map(address_map &map);

	void aerfboot_sound_map(address_map &map);
	void wbbc97_map(address_map &map);
	void wbbc97_sound_map(address_map &map);
	void karatblzbl_sound_map(address_map &map);
	void karatblzbl_sound_portmap(address_map &map);
	void kickball_sound_map(address_map &map);
	void kickball_sound_portmap(address_map &map);
	void spikes91_sound_map(address_map &map);

	void oki_map(address_map &map);
};


class aerofgt_banked_sound_state : public aerofgt_sound_cpu_state
{
public:
	aerofgt_banked_sound_state(const machine_config &mconfig, device_type type, const char *tag)
		: aerofgt_sound_cpu_state(mconfig, type, tag)
		, m_soundbank(*this, "soundbank")
	{
	}

	void pspikes(machine_config &config) ATTR_COLD;
	void karatblz(machine_config &config) ATTR_COLD;
	void spinlbrk(machine_config &config) ATTR_COLD;
	void turbofrc(machine_config &config) ATTR_COLD;
	void aerofgtb(machine_config &config) ATTR_COLD;
	void aerofgt(machine_config &config) ATTR_COLD;

protected:
	required_memory_bank m_soundbank;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	DECLARE_VIDEO_START(spinlbrk);

private:
	uint32_t aerofgt_tile_callback( uint32_t code );
	uint32_t spinbrk_tile_callback(uint32_t code);

	void sh_bankswitch_w(uint8_t data);

	uint32_t screen_update_spinlbrk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_turbofrc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_aerofgt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pspikes_map(address_map &map);
	void karatblz_map(address_map &map);
	void spinlbrk_map(address_map &map) ATTR_COLD;
	void turbofrc_map(address_map &map) ATTR_COLD;
	void aerofgtb_map(address_map &map);
	void aerofgt_map(address_map &map);

	void sound_map(address_map &map);
	void spinlbrk_sound_portmap(address_map &map) ATTR_COLD;
	void turbofrc_sound_portmap(address_map &map) ATTR_COLD;
	void aerofgt_sound_portmap(address_map &map) ATTR_COLD;
};

#endif // MAME_VSYSTEM_AEROFGT_H

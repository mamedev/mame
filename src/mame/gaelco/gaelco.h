// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco game hardware from 1991-1996

***************************************************************************/
#ifndef MAME_GAELCO_GAELCO_H
#define MAME_GAELCO_GAELCO_H

#pragma once

#include "gaelcrpt.h"

#include "machine/gen_latch.h"
#include "machine/74259.h"

#include "emupal.h"
#include "tilemap.h"


class gaelco_state : public driver_device
{
public:
	gaelco_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_okibank(*this, "okibank"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_sprite_palette_force_high(0x38),
		m_spritexoffset(0),
		m_spriteyoffset(0)
	{ }

	void maniacsq(machine_config &config) ATTR_COLD;

protected:
	static constexpr double FRAMERATE_922804 = 57.42;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<ls259_device> m_outlatch;
	optional_memory_bank m_okibank;

	// memory pointers
	required_shared_ptr<u16> m_videoram;
	required_shared_ptr<u16> m_vregs;
	required_shared_ptr<u16> m_spriteram;

	// video-related
	tilemap_t      *m_tilemap[2]{};

	// per-game configuration
	u8 m_sprite_palette_force_high = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void create_tilemaps(bool split);

	template <unsigned Which> void coin_lockout_w(int state);
	template <unsigned Which> void coin_counter_w(int state);
	void oki_bankswitch_w(u8 data);
	void vram_w(offs_t offset, u16 data, u16 mem_mask);
	void irqack_w(u16 data);

	template <int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	u32 screen_update_maniacsq(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void maniacsq_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;

	int m_spritexoffset;
	int m_spriteyoffset;
};

class xorwflat_state : public gaelco_state
{
public:
	xorwflat_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelco_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{
		m_spritexoffset = -80;
		m_spriteyoffset = 9;
	}

	void xorwflat(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void xorwflat_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_audiocpu;

	void sound_map(address_map &map);
	void sound_io_map(address_map &map);
};

class bigkarnk_state : public gaelco_state
{
public:
	bigkarnk_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelco_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void bigkarnk(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;

	u32 screen_update_bigkarnk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bigkarnk_map(address_map &map) ATTR_COLD;
	void bigkarnk_snd_map(address_map &map) ATTR_COLD;
};

class squash_state : public gaelco_state
{
public:
	squash_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelco_state(mconfig, type, tag),
		m_vramcrypt(*this, "vramcrypt"),
		m_screenram(*this, "screenram")
	{ }

	void squash(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<gaelco_vram_encryption_device> m_vramcrypt;

	// memory pointers
	required_shared_ptr<u16> m_screenram;

	void vram_encrypted_w(offs_t offset, u16 data, u16 mem_mask);
	void encrypted_w(offs_t offset, u16 data, u16 mem_mask);

	u32 screen_update_squash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void squash_map(address_map &map) ATTR_COLD;
};


class thoop_state : public squash_state
{
public:
	thoop_state(const machine_config &mconfig, device_type type, const char *tag) :
		squash_state(mconfig, type, tag)
	{ }

	void thoop(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void thoop_map(address_map &map) ATTR_COLD;
};


#endif // MAME_GAELCO_GAELCO_H

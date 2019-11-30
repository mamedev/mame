// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/*************************************************************************

    Zero Zone

*************************************************************************/
#ifndef MAME_INCLUDES_ZEROZONE_H
#define MAME_INCLUDES_ZEROZONE_H

#pragma once

#include "machine/gen_latch.h"
#include "cpu/z80/z80.h"
#include "tilemap.h"

class zerozone_state : public driver_device
{
public:
	zerozone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_soundlatch(*this, "soundlatch")
		, m_vram(*this, "videoram")
		, m_gfxdecode(*this, "gfxdecode")
	{ }

	void zerozone(machine_config &config);

private:
	// in drivers/zerozone.c
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_WRITE_LINE_MEMBER(vblank_w);

	// in video/zerozone.c
	DECLARE_WRITE16_MEMBER(tilemap_w);
	DECLARE_WRITE16_MEMBER(tilebank_w);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;

	// shared pointers
	required_shared_ptr<uint16_t> m_vram;
	// currently this driver uses generic palette handling

	required_device<gfxdecode_device> m_gfxdecode;
	// state
	// video-related
	uint16_t         m_tilebank;
	tilemap_t     *m_zz_tilemap;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void sound_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	TILE_GET_INFO_MEMBER(get_zerozone_tile_info);
};

#endif // MAME_INCLUDES_ZEROZONE_H

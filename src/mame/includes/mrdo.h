// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Mr. Do

*************************************************************************/
#ifndef MAME_INCLUDES_MRDO_H
#define MAME_INCLUDES_MRDO_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class mrdo_state : public driver_device
{
public:
	mrdo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void mrdo(machine_config &config);
	void mrdobl(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	virtual void protection_w(uint8_t data);
	virtual uint8_t protection_r();

	uint8_t m_pal_u001;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void main_map(address_map &map);

	// video-related
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_flipscreen;

	void bgvideoram_w(offs_t offset, uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void scrollx_w(uint8_t data);
	void scrolly_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update_mrdo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};

class mrdot_state : public mrdo_state
{
public:
	mrdot_state(const machine_config &mconfig, device_type type, const char *tag) :
		mrdo_state(mconfig, type, tag)
	{ }

protected:
	virtual void protection_w(uint8_t data) override;
	virtual uint8_t protection_r() override;
};

class mrlo_state : public mrdo_state
{
public:
	mrlo_state(const machine_config &mconfig, device_type type, const char *tag) :
		mrdo_state(mconfig, type, tag)
	{ }

protected:
	// no PAL protection
	virtual void protection_w(uint8_t data) override { }
	virtual uint8_t protection_r() override { return 0; }
};

#endif // MAME_INCLUDES_MRDO_H

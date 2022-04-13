// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Mosaic

*************************************************************************/
#ifndef MAME_INCLUDES_MOSAIC_H
#define MAME_INCLUDES_MOSAIC_H

#pragma once

#include "tilemap.h"

class mosaic_state : public driver_device
{
public:
	mosaic_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram")
	{ }

	void mosaic(machine_config &config);
	void gfire2(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_bgvideoram;

	/* video-related */
	tilemap_t      *m_bg_tilemap = nullptr;
	tilemap_t      *m_fg_tilemap = nullptr;

	/* misc */
	int            m_prot_val = 0;

	void protection_w(uint8_t data);
	uint8_t protection_r();
	void gfire2_protection_w(uint8_t data);
	uint8_t gfire2_protection_r();
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gfire2_io_map(address_map &map);
	void gfire2_map(address_map &map);
	void mosaic_io_map(address_map &map);
	void mosaic_map(address_map &map);
};

#endif // MAME_INCLUDES_MOSAIC_H

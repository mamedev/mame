// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/***************************************************************************

    Zaccaria Galaxia HW

****************************************************************************/
#ifndef MAME_INCLUDES_GALAXIA_H
#define MAME_INCLUDES_GALAXIA_H

#pragma once

#include "includes/cvs.h"
#include "tilemap.h"

class galaxia_state : public cvs_state
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag)
	{ }

	void astrowar(machine_config &config);
	void galaxia(machine_config &config);

	void init_common();

private:
	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_temp_bitmap;
	DECLARE_WRITE8_MEMBER(galaxia_video_w);
	DECLARE_WRITE8_MEMBER(galaxia_scroll_w);
	DECLARE_WRITE8_MEMBER(galaxia_ctrlport_w);
	DECLARE_WRITE8_MEMBER(galaxia_dataport_w);
	DECLARE_READ8_MEMBER(galaxia_collision_r);
	DECLARE_READ8_MEMBER(galaxia_collision_clear);
	TILE_GET_INFO_MEMBER(get_galaxia_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_astrowar_bg_tile_info);
	DECLARE_VIDEO_START(galaxia);
	void galaxia_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(astrowar);
	void astrowar_palette(palette_device &palette) const;
	uint32_t screen_update_galaxia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_astrowar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void astrowar_mem_map(address_map &map);
	void galaxia_data_map(address_map &map);
	void galaxia_io_map(address_map &map);
	void galaxia_mem_map(address_map &map);
};

#endif // MAME_INCLUDES_GALAXIA_H

// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/***************************************************************************

    Zaccaria Galaxia HW

****************************************************************************/
#ifndef MAME_INCLUDES_GALAXIA_H
#define MAME_INCLUDES_GALAXIA_H

#pragma once

#include "cvs.h"
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

protected:
	virtual void video_start() override;

private:
	tilemap_t *m_bg_tilemap = nullptr;
	bitmap_ind16 m_temp_bitmap;
	void galaxia_video_w(offs_t offset, uint8_t data);
	void galaxia_scroll_w(uint8_t data);
	void galaxia_ctrlport_w(uint8_t data);
	void galaxia_dataport_w(uint8_t data);
	uint8_t galaxia_collision_r();
	uint8_t galaxia_collision_clear();
	TILE_GET_INFO_MEMBER(get_galaxia_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_astrowar_bg_tile_info);
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

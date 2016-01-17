// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/***************************************************************************

    Zaccaria Galaxia HW

****************************************************************************/

#include "includes/cvs.h"

class galaxia_state : public cvs_state
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, std::string tag)
		: cvs_state(mconfig, type, tag) { }


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
	DECLARE_PALETTE_INIT(galaxia);
	DECLARE_VIDEO_START(astrowar);
	DECLARE_PALETTE_INIT(astrowar);
	UINT32 screen_update_galaxia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_astrowar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(galaxia_interrupt);
	void init_common();
};

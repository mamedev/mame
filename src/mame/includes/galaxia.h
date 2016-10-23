// license:BSD-3-Clause
// copyright-holders:David Haywood, hap
/***************************************************************************

    Zaccaria Galaxia HW

****************************************************************************/

#include "includes/cvs.h"

class galaxia_state : public cvs_state
{
public:
	galaxia_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }


	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_temp_bitmap;
	void galaxia_video_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxia_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxia_ctrlport_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void galaxia_dataport_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t galaxia_collision_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t galaxia_collision_clear(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	TILE_GET_INFO_MEMBER(get_galaxia_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_astrowar_bg_tile_info);
	void video_start_galaxia();
	DECLARE_PALETTE_INIT(galaxia);
	void video_start_astrowar();
	DECLARE_PALETTE_INIT(astrowar);
	uint32_t screen_update_galaxia(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_astrowar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(galaxia_interrupt);
	void init_common();
};

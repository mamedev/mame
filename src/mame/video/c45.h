// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Aaron Giles, Alex W. Jackson
#ifndef MAME_VIDEO_C45_H
#define MAME_VIDEO_C45_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> namco_c45_road_device

class namco_c45_road_device : public device_t, public device_gfx_interface, public device_memory_interface
{
public:
	// construction/destruction
	namco_c45_road_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	// read/write handlers
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// C45 Land (Road) Emulation
	void set_transparent_color(pen_t pen) { m_transparent_color = pen; }
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual space_config_vector memory_space_config() const override;

private:
	// constants
	static constexpr int ROAD_COLS = 64;
	static constexpr int ROAD_ROWS = 512;
	static constexpr int ROAD_TILE_SIZE = 16;
	static constexpr int ROAD_TILEMAP_WIDTH = ROAD_TILE_SIZE * ROAD_COLS;
	static constexpr int ROAD_TILEMAP_HEIGHT = ROAD_TILE_SIZE * ROAD_ROWS;
	static constexpr int WORDS_PER_ROAD_TILE = 0x40/2;
	static const gfx_layout tilelayout;

	// internal helpers
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_WRITE16_MEMBER( tilemap_w );
	DECLARE_WRITE16_MEMBER( tileram_w );
	TILE_GET_INFO_MEMBER( get_road_info );

	// internal state
	address_space_config        m_space_config;
	required_shared_ptr<uint16_t> m_tmapram;
	required_shared_ptr<uint16_t> m_tileram;
	required_shared_ptr<uint16_t> m_lineram;
	optional_region_ptr<uint8_t>  m_clut;
	tilemap_t *                 m_tilemap;
	pen_t                       m_transparent_color;
};


// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C45_ROAD, namco_c45_road_device)

#endif // MAME_VIDEO_C45_H

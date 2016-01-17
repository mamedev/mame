// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Aaron Giles, Alex W. Jackson
#pragma once

#ifndef __C45_H__
#define __C45_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C45_ROAD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C45_ROAD, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> namco_c45_road_device

class namco_c45_road_device : public device_t, public device_gfx_interface, public device_memory_interface
{
	// constants
	static const int ROAD_COLS = 64;
	static const int ROAD_ROWS = 512;
	static const int ROAD_TILE_SIZE = 16;
	static const int ROAD_TILEMAP_WIDTH = ROAD_TILE_SIZE * ROAD_COLS;
	static const int ROAD_TILEMAP_HEIGHT = ROAD_TILE_SIZE * ROAD_ROWS;
	static const int WORDS_PER_ROAD_TILE = 0x40/2;
	static const gfx_layout tilelayout;

public:
	// construction/destruction
	namco_c45_road_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_ADDRESS_MAP(map, 16);

	// read/write handlers
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// C45 Land (Road) Emulation
	void set_transparent_color(pen_t pen) { m_transparent_color = pen; }
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	// internal helpers
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_WRITE16_MEMBER( tilemap_w );
	DECLARE_WRITE16_MEMBER( tileram_w );
	TILE_GET_INFO_MEMBER( get_road_info );

	// internal state
	address_space_config        m_space_config;
	required_shared_ptr<UINT16> m_tmapram;
	required_shared_ptr<UINT16> m_tileram;
	required_shared_ptr<UINT16> m_lineram;
	UINT8 *                     m_clut;
	tilemap_t *                 m_tilemap;
	pen_t                       m_transparent_color;
};


// device type definition
extern const device_type NAMCO_C45_ROAD;

#endif

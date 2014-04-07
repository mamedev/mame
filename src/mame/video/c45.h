#pragma once

#ifndef __C45_H__
#define __C45_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C45_ROAD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C45_ROAD, 0)

#define MCFG_NAMCO_C45_ROAD_PALETTE(_palette_tag) \
	namco_c45_road_device::static_set_palette_tag(*device, "^" _palette_tag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> namco_c45_road_device

class namco_c45_road_device : public device_t
{
	// constants
	static const int ROAD_COLS = 64;
	static const int ROAD_ROWS = 512;
	static const int ROAD_TILE_SIZE = 16;
	static const int ROAD_TILEMAP_WIDTH = ROAD_TILE_SIZE * ROAD_COLS;
	static const int ROAD_TILEMAP_HEIGHT = ROAD_TILE_SIZE * ROAD_ROWS;
	static const int ROAD_TILE_COUNT_MAX = 0xfa00 / 0x40; // 0x3e8
	static const int WORDS_PER_ROAD_TILE = 0x40/2;

public:
	// construction/destruction
	namco_c45_road_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_palette_tag(device_t &device, const char *tag);

	// read/write handlers
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// C45 Land (Road) Emulation
	void set_transparent_color(pen_t pen) { m_transparent_color = pen; }
	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();

	// internal helpers
	TILE_GET_INFO_MEMBER( get_road_info );

	// internal state
	pen_t           m_transparent_color;
	tilemap_t *     m_tilemap;
	UINT16          m_ram[0x20000/2]; // at 0x880000 in Final Lap; at 0xa00000 in Lucky&Wild

	static const gfx_layout s_tile_layout;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


// device type definition
extern const device_type NAMCO_C45_ROAD;

#endif

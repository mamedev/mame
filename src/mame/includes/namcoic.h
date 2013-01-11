/* namcoic.h

Custom Chips:                       Final Lap   Assault     LuckyWld    System21    NA1/2       NB1/2
    C45     Land Generator            *                       *
    C65     I/O Controller            *           *
    C68                                                       *           *
    C70                                                                               *
    C95                               *           *
    C102    ROZ:Memory Access Control             *
    C106    OBJ:X-Axis Zoom Control   *           *
    C107    Land Line Buffer          *
    C116    Screen Waveform Generator *           *           *                                   *
    C121    Yamaha YM2151 Sound Gen   *           *           *
    C123    GFX:Tile Mem Decoder      *           *           *                                   *
    C134    OBJ:Address Generator     *           *
    C135    OBJ:Line matching         *           *
    C137    Clock Generator IC        *           *           *           *                       *
    C138                                                                  *
    C139    Serial I/F Controller     *           *           *           *
    C140    24 Channel PCM            *           *           *
    C145    GFX:Tile Memory Access    *           *           *                                   *
    C146    OBJ:Line Buf Steering     *           *
    C148    CPU Bus Manager           *           *           *           *
    C149    Mouse/Trackball Decoder   *           *           *           *
    C156    Pixel Stream Combo        *           *           *                                   *
    C160    Control                                                                               *
    C165                                                                  *
    C169    ROZ(B)                                            *                                   *
    C187                                                      *           *                       *
    C210                                                                              *
    C215                                                                              *
    C218                                                                              *
    C219                                                                              *
    C329    CPU?                                                                                  *
    C347    GfxObj                                                                                *
    C352    PCM                                                                                   *
    C355    Motion Obj(B)                                     *           *                       *
    C373    LAND-related                                      *
    C382                                                                                          *
    C383                                                                                          *
    C384    GFX(3)                                                                                *
    C385                                                                                          *
    C390    Key Custom                                                                            *


General Support
---------------
C65  - This is the I/O Microcontroller, handles all input/output devices. 63705 uC, CPU4 in Namco System2.
C137 - Takes System clock and generates all sub-system clocks, doesnt need emulation, not accessed via CPU
C139 - Serial Interface Controller
C148 - Does some Memory Decode, Interrupt Handling, 3 bit PIO port, Bus Controller
C149 - Does decoding of mouse/trackball input streams for the I/O Controller. (Offset Square wave)


Tile Fields Static/Scrolled
---------------------------
Combination of these two devices and associated RAM & TileGFX produces a pixel stream that is fed
into the Pixel stream decoder.

C145 - Tile Screen Memory Access controller
C123 - Tile Memory decoder Part 1, converts X,Y,Tile into character ROM address index


Pixel Stream Decode
-------------------
These two devices take the pixel streams from the tilefield generator and the associated graphics board
and combine them to form an RGB data stream that is fed to the monitor.

C156 - Pixel stream combiner
Takes tile field & graphics board streams and generates the priorisied pixel, then does the lookup to
go from palettised to 24bit RGB pixel.

C116 - Screen Waveform Generator
Takes RGB24 pixel stream from C156 and generates the waveform signals for the monitor, also generates
the line interrupt and controls screen blanking,shift, etc.

Object Control
--------------
C106 - Generates memory output clocks to generate X-Axis Zoom for Line Buffer Writes
C134 - Object Memory Address Generator. Sequences the sprite memory contents to the hardware.
C135 - Checks is object is displayed on Current output line.
C146 - Steers the Decode Object Pixel data to the correct line buffer A or B

ROZ
---
C102 - Controls CPU access to ROZ Memory Area.
*/

/***********************************************************************************/

#pragma once

#ifndef __NAMCOIC_H__
#define __NAMCOIC_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_NAMCO_C45_ROAD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NAMCO_C45_ROAD, 0) \



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

	// read/write handlers
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );

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
	gfx_element *   m_gfx;
	tilemap_t *     m_tilemap;
	UINT16          m_ram[0x20000/2]; // at 0x880000 in Final Lap; at 0xa00000 in Lucky&Wild

	static const gfx_layout s_tile_layout;
};


// device type definition
extern const device_type NAMCO_C45_ROAD;



/*----------- defined in drivers/namcoic.c -----------*/

void namco_tilemap_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
void namco_tilemap_invalidate( void );
DECLARE_WRITE16_HANDLER( namco_tilemapvideoram16_w );
DECLARE_READ16_HANDLER( namco_tilemapvideoram16_r );
DECLARE_WRITE16_HANDLER( namco_tilemapcontrol16_w );
DECLARE_READ16_HANDLER( namco_tilemapcontrol16_r );

DECLARE_READ32_HANDLER( namco_tilemapvideoram32_r );
DECLARE_WRITE32_HANDLER( namco_tilemapvideoram32_w );
DECLARE_READ32_HANDLER( namco_tilemapcontrol32_r );
DECLARE_WRITE32_HANDLER( namco_tilemapcontrol32_w );

DECLARE_READ32_HANDLER( namco_tilemapvideoram32_le_r );
DECLARE_WRITE32_HANDLER( namco_tilemapvideoram32_le_w );
DECLARE_READ32_HANDLER( namco_tilemapcontrol32_le_r );
DECLARE_WRITE32_HANDLER( namco_tilemapcontrol32_le_w );

/***********************************************************************************/

#endif

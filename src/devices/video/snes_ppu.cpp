// license:GPL-2.0+
// copyright-holders:byuu, Anthony Kruize, Fabio Priuli
/***************************************************************************

  snes.cpp

  Video file to handle emulation of the Nintendo Super NES.

  Anthony Kruize
  Based on the original code by Lee Hammerton (aka Savoury Snax)

  Some notes on the snes video hardware:

  Object Attribute Memory(OAM) is made up of 128 blocks of 32 bits, followed
  by 128 blocks of 2 bits. The format for each block is:
  -First Block----------------------------------------------------------------
  | x pos  | y pos  |char no.| v flip | h flip |priority|palette |char no msb|
  +--------+--------+--------+--------+--------+--------+--------+-----------+
  | 8 bits | 8 bits | 8 bits | 1 bit  | 1 bit  | 2 bits | 3 bits | 1 bit     |
  -Second Block---------------------------------------------------------------
  | size  | x pos msb |
  +-------+-----------+
  | 1 bit | 1 bit     |
  ---------------------

  Video RAM contains information for character data and screen maps.
  Screen maps are made up of 32 x 32 blocks of 16 bits each.
  The format for each block is:
  ----------------------------------------------
  | v flip | x flip |priority|palette |char no.|
  +--------+--------+--------+--------+--------+
  | 1 bit  | 1 bit  | 1 bit  | 3 bits |10 bits |
  ----------------------------------------------
  Mode 7 is stored differently. Character data and screen map are interleaved.
  There are two formats:
  -Normal-----------------  -EXTBG-----------------------------
  | char data | char no. |  | priority | char data | char no. |
  +-----------+----------+  +----------+-----------+----------+
  | 8 bits    | 8 bits   |  | 1 bit    | 7 bits    | 8 bits   |
  ------------------------  -----------------------------------

  The screen layers are drawn with the following priorities (updated info courtesy of byuu):

  |           |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |
  -------------------------------------------------------------------------------------------------------------
  | Mode 0    |  BG4B |  BG3B |  OAM0 |  BG4A |  BG3A |  OAM1 |  BG2B |  BG1B |  OAM2 |  BG2A |  BG1A |  OAM3 |
  -------------------------------------------------------------------------------------------------------------
  | Mode 1 (*)|  BG3B |  OAM0 |  OAM1 |  BG2B |  BG1B |  OAM2 |  BG2A |  BG1A |  OAM3 |  BG3A |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 1 (!)|  BG3B |  OAM0 |  BG3A |  OAM1 |  BG2B |  BG1B |  OAM2 |  BG2A |  BG1A |  OAM3 |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 2    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 3    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 4    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 5    |  BG2B |  OAM0 |  BG1B |  OAM1 |  BG2A |  OAM2 |  BG1A |  OAM3 |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 6    |  OAM0 |  BG1B |  OAM1 |  OAM2 |  BG1A |  OAM3 |       |       |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 7 (+)|  OAM0 |  BG1n |  OAM1 |  OAM2 |  OAM3 |       |       |       |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------
  | Mode 7 (-)|  BG2B |  OAM0 |  BG1n |  OAM1 |  BG2A |  OAM2 |  OAM3 |       |       |       |       |       |
  -------------------------------------------------------------------------------------------------------------

  Where:
   - Mode 1 (*) is Mode 1 with bg3_pty = 1
   - Mode 1 (!) is Mode 1 with bg3_pty = 0
   - Mode 7 (+) is base Mode 7
   - Mode 7 (-) is Mode 7 EXTBG

***************************************************************************/

#include "emu.h"
#include "snes_ppu.h"

#define SNES_MAINSCREEN    0
#define SNES_SUBSCREEN     1
#define SNES_CLIP_NEVER    0
#define SNES_CLIP_IN       1
#define SNES_CLIP_OUT      2
#define SNES_CLIP_ALWAYS   3

#define SNES_VRAM_SIZE        0x10000   /* 64kb of video ram */
#define SNES_CGRAM_SIZE       0x200     /* 256 16-bit colours */
#define SNES_OAM_SIZE         0x440     /* 1088 bytes of Object Attribute Memory */


/* Definitions for PPU Memory-Mapped registers */
#define INIDISP        0x2100
#define OBSEL          0x2101
#define OAMADDL        0x2102
#define OAMADDH        0x2103
#define OAMDATA        0x2104
#define BGMODE         0x2105   /* abcdefff = abcd: bg4-1 tile size | e: BG3 high priority | f: mode */
#define MOSAIC         0x2106   /* xxxxabcd = x: pixel size | abcd: affects bg 1-4 */
#define BG1SC          0x2107
#define BG2SC          0x2108
#define BG3SC          0x2109
#define BG4SC          0x210A
#define BG12NBA        0x210B
#define BG34NBA        0x210C
#define BG1HOFS        0x210D
#define BG1VOFS        0x210E
#define BG2HOFS        0x210F
#define BG2VOFS        0x2110
#define BG3HOFS        0x2111
#define BG3VOFS        0x2112
#define BG4HOFS        0x2113
#define BG4VOFS        0x2114
#define VMAIN          0x2115   /* i---ffrr = i: Increment timing | f: Full graphic | r: increment rate */
#define VMADDL         0x2116   /* aaaaaaaa = a: LSB of vram address */
#define VMADDH         0x2117   /* aaaaaaaa = a: MSB of vram address */
#define VMDATAL        0x2118   /* dddddddd = d: data to be written */
#define VMDATAH        0x2119   /* dddddddd = d: data to be written */
#define M7SEL          0x211A   /* ab----yx = a: screen over | y: vertical flip | x: horizontal flip */
#define M7A            0x211B   /* aaaaaaaa = a: COSINE rotate angle / X expansion */
#define M7B            0x211C   /* aaaaaaaa = a: SINE rotate angle / X expansion */
#define M7C            0x211D   /* aaaaaaaa = a: SINE rotate angle / Y expansion */
#define M7D            0x211E   /* aaaaaaaa = a: COSINE rotate angle / Y expansion */
#define M7X            0x211F
#define M7Y            0x2120
#define CGADD          0x2121
#define CGDATA         0x2122
#define W12SEL         0x2123
#define W34SEL         0x2124
#define WOBJSEL        0x2125
#define WH0            0x2126   /* pppppppp = p: Left position of window 1 */
#define WH1            0x2127   /* pppppppp = p: Right position of window 1 */
#define WH2            0x2128   /* pppppppp = p: Left position of window 2 */
#define WH3            0x2129   /* pppppppp = p: Right position of window 2 */
#define WBGLOG         0x212A   /* aabbccdd = a: BG4 params | b: BG3 params | c: BG2 params | d: BG1 params */
#define WOBJLOG        0x212B   /* ----ccoo = c: Colour window params | o: Object window params */
#define TM             0x212C
#define TS             0x212D
#define TMW            0x212E
#define TSW            0x212F
#define CGWSEL         0x2130
#define CGADSUB        0x2131
#define COLDATA        0x2132
#define SETINI         0x2133
#define MPYL           0x2134
#define MPYM           0x2135
#define MPYH           0x2136
#define SLHV           0x2137
#define ROAMDATA       0x2138
#define RVMDATAL       0x2139
#define RVMDATAH       0x213A
#define RCGDATA        0x213B
#define OPHCT          0x213C
#define OPVCT          0x213D
#define STAT77         0x213E
#define STAT78         0x213F


#if SNES_LAYER_DEBUG
/*                                    red   green  blue    purple  yellow cyan    grey    white */
//static const uint16_t dbg_mode_colours[8] = { 0x1f, 0x3e0, 0x7c00, 0x7c1f, 0x3ff, 0x7fe0, 0x4210, 0x7fff };
#endif /* SNES_LAYER_DEBUG */


enum
{
	SNES_COLOR_DEPTH_2BPP = 0,
	SNES_COLOR_DEPTH_4BPP,
	SNES_COLOR_DEPTH_8BPP,
	SNES_COLOR_DEPTH_NONE,
	SNES_COLOR_DEPTH_MODE7
};


#define PPU_REG(a) m_regs[a - 0x2100]



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SNES_PPU, snes_ppu_device, "snes_ppu", "SNES PPU")


//**************************************************************************
//  live device
//**************************************************************************

//-------------------------------------------------
//  snes_ppu_device - constructor
//-------------------------------------------------

snes_ppu_device::snes_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNES_PPU, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_palette_interface(mconfig, *this)
	, m_openbus_cb(*this, 0)
	, m_options(*this, ":OPTIONS")
	, m_debug1(*this, ":DEBUG1")
	, m_debug2(*this, ":DEBUG2")
	, m_debug3(*this, ":DEBUG3")
	, m_debug4(*this, ":DEBUG4")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snes_ppu_device::device_start()
{
	m_vram = std::make_unique<uint8_t[]>(SNES_VRAM_SIZE);
	m_cgram = std::make_unique<uint16_t[]>(SNES_CGRAM_SIZE/2);

	m_light_table = std::make_unique<std::unique_ptr<uint16_t[]>[]>(16);
	for (uint8_t l = 0; l < 16; l++)
	{
		m_light_table[l] = std::make_unique<uint16_t[]>(32768);
		for (uint8_t r = 0; r < 32; r++)
		{
			for (uint8_t g = 0; g < 32; g++)
			{
				for (uint8_t b = 0; b < 32; b++)
				{
					double luma = (double)l / 15.0;
					uint8_t ar = (uint8_t)(luma * r + 0.5);
					uint8_t ag = (uint8_t)(luma * g + 0.5);
					uint8_t ab = (uint8_t)(luma * b + 0.5);
					m_light_table[l][r << 0 | g << 5 | b << 10] = ar << 0 | ag << 5 | ab << 10;
				}
			}
		}
	}
	// initialize direct colours
	for (int rgb = 0; rgb < 32*32*32; rgb++)
	{
		set_indirect_color(rgb, pal555(rgb, 0, 5, 10));
	}
	for (uint8_t group = 0; group < 8; group++)
	{
		for (int c = 0; c < 256; c++)
		{
			//palette = -------- BBGGGRRR
			//group   = -------- -----bgr
			//output  = 0BBb00GG Gg0RRRr0
			const u16 direct = (c << 7 & 0x6000) + (group << 10 & 0x1000)
							 + (c << 4 & 0x0380) + (group <<  5 & 0x0040)
							 + (c << 2 & 0x001c) + (group <<  1 & 0x0002);
			set_pen_indirect(DIRECT_COLOUR + c + (group * 256), direct);
		}
	}

	save_item(STRUCT_MEMBER(m_scanlines, enable));
	save_item(STRUCT_MEMBER(m_scanlines, clip));
	save_item(STRUCT_MEMBER(m_scanlines, buffer));
	save_item(STRUCT_MEMBER(m_scanlines, priority));
	save_item(STRUCT_MEMBER(m_scanlines, layer));
	save_item(STRUCT_MEMBER(m_scanlines, blend_exception));

	save_item(STRUCT_MEMBER(m_layer, window1_enabled));
	save_item(STRUCT_MEMBER(m_layer, window1_invert));
	save_item(STRUCT_MEMBER(m_layer, window2_enabled));
	save_item(STRUCT_MEMBER(m_layer, window2_invert));
	save_item(STRUCT_MEMBER(m_layer, wlog_mask));
	save_item(STRUCT_MEMBER(m_layer, color_math));
	save_item(STRUCT_MEMBER(m_layer, charmap));
	save_item(STRUCT_MEMBER(m_layer, tilemap));
	save_item(STRUCT_MEMBER(m_layer, tilemap_size));
	save_item(STRUCT_MEMBER(m_layer, tile_size));
	save_item(STRUCT_MEMBER(m_layer, tile_mode));
	save_item(STRUCT_MEMBER(m_layer, mosaic_enabled));
	save_item(STRUCT_MEMBER(m_layer, main_window_enabled));
	save_item(STRUCT_MEMBER(m_layer, sub_window_enabled));
	save_item(STRUCT_MEMBER(m_layer, main_bg_enabled));
	save_item(STRUCT_MEMBER(m_layer, sub_bg_enabled));
	save_item(STRUCT_MEMBER(m_layer, hoffs));
	save_item(STRUCT_MEMBER(m_layer, voffs));
	save_item(STRUCT_MEMBER(m_layer, priority));
	save_item(STRUCT_MEMBER(m_layer, mosaic_counter));
	save_item(STRUCT_MEMBER(m_layer, mosaic_offset));

	save_item(NAME(m_clipmasks));

	save_item(NAME(m_oam.address));
	save_item(NAME(m_oam.base_address));
	save_item(NAME(m_oam.priority_rotation));
	save_item(NAME(m_oam.tile_data_address));
	save_item(NAME(m_oam.name_select));
	save_item(NAME(m_oam.base_size));
	save_item(NAME(m_oam.first));
	save_item(NAME(m_oam.write_latch));
	save_item(NAME(m_oam.data_latch));
	save_item(NAME(m_oam.interlace));
	save_item(NAME(m_oam.priority));

	save_item(NAME(m_beam.latch_horz));
	save_item(NAME(m_beam.latch_vert));
	save_item(NAME(m_beam.current_vert));
	save_item(NAME(m_beam.last_visible_line));
	save_item(NAME(m_beam.interlace_count));

	save_item(NAME(m_mode7.repeat));
	save_item(NAME(m_mode7.hflip));
	save_item(NAME(m_mode7.vflip));
	save_item(NAME(m_mode7.matrix_a));
	save_item(NAME(m_mode7.matrix_b));
	save_item(NAME(m_mode7.matrix_c));
	save_item(NAME(m_mode7.matrix_d));
	save_item(NAME(m_mode7.origin_x));
	save_item(NAME(m_mode7.origin_y));
	save_item(NAME(m_mode7.hor_offset));
	save_item(NAME(m_mode7.ver_offset));
	save_item(NAME(m_mode7.extbg));

	save_item(STRUCT_MEMBER(m_objects, x));
	save_item(STRUCT_MEMBER(m_objects, y));
	save_item(STRUCT_MEMBER(m_objects, character));
	save_item(STRUCT_MEMBER(m_objects, name_select));
	save_item(STRUCT_MEMBER(m_objects, vflip));
	save_item(STRUCT_MEMBER(m_objects, hflip));
	save_item(STRUCT_MEMBER(m_objects, pri));
	save_item(STRUCT_MEMBER(m_objects, pal));
	save_item(STRUCT_MEMBER(m_objects, size));

	save_item(NAME(m_mosaic_size));
	save_item(NAME(m_clip_to_black));
	save_item(NAME(m_prevent_color_math));
	save_item(NAME(m_sub_add_mode));
	save_item(NAME(m_bg_priority));
	save_item(NAME(m_direct_color));
	save_item(NAME(m_ppu_last_scroll));
	save_item(NAME(m_mode7_last_scroll));

	save_item(NAME(m_ppu1_open_bus));
	save_item(NAME(m_ppu2_open_bus));
	save_item(NAME(m_ppu1_version));
	save_item(NAME(m_ppu2_version));
	save_item(NAME(m_window1_left));
	save_item(NAME(m_window1_right));
	save_item(NAME(m_window2_left));
	save_item(NAME(m_window2_right));

	save_item(NAME(m_mode));
	save_item(NAME(m_interlace));
	save_item(NAME(m_screen_brightness));
	save_item(NAME(m_screen_disabled));
	save_item(NAME(m_pseudo_hires));
	save_item(NAME(m_color_modes));
	save_item(NAME(m_stat77));
	save_item(NAME(m_stat78));

	save_item(NAME(m_htmult));
	save_item(NAME(m_cgram_address));
	save_item(NAME(m_read_ophct));
	save_item(NAME(m_read_opvct));
	save_item(NAME(m_vram_fgr_high));
	save_item(NAME(m_vram_fgr_increment));
	save_item(NAME(m_vram_fgr_count));
	save_item(NAME(m_vram_fgr_mask));
	save_item(NAME(m_vram_fgr_shift));
	save_item(NAME(m_vram_read_buffer));
	save_item(NAME(m_vmadd));

	save_item(NAME(m_regs));

	save_pointer(NAME(m_vram), SNES_VRAM_SIZE);
	save_pointer(NAME(m_cgram), SNES_CGRAM_SIZE/2);
}

void snes_ppu_device::device_reset()
{
#if SNES_LAYER_DEBUG
	memset(&m_debug_options, 0, sizeof(m_debug_options));
#endif

	/* Inititialize registers/variables */
	m_beam.latch_vert = 0;
	m_beam.latch_horz = 0;
	m_beam.current_vert = 0;

	/* Set STAT78 to NTSC or PAL */
	m_stat78 = (screen().frame_period().as_hz() >= 59.0) ? SNES_NTSC : SNES_PAL;
	m_beam.last_visible_line = m_stat78 & SNES_PAL ? 240 : 225;
	m_mode = 0;
	m_ppu1_version = 1;  // 5C77 chip version number, read by STAT77, only '1' is known
	m_ppu2_version = 3;  // 5C78 chip version number, read by STAT78, only '2' & '3' encountered so far.

	m_cgram_address = 0;
	m_read_ophct = 0;
	m_read_opvct = 0;

	m_vmadd = 0;

	PPU_REG(VMAIN) = 0x80;
	// what about other regs?

	/* Inititialize mosaic table */
	for (int j = 0; j < 16; j++)
	{
		for (int i = 0; i < 4096; i++)
			m_mosaic_table[j][i] = (i / (j + 1)) * (j + 1);
	}

	/* Init VRAM */
	memset(m_vram.get(), 0, SNES_VRAM_SIZE);

	/* Init Palette RAM */
	memset((uint8_t *)m_cgram.get(), 0, SNES_CGRAM_SIZE);
	for (int i = 0; i < 256; i++)
		set_pen_indirect(i, m_cgram[i]);

	set_pen_indirect(FIXED_COLOUR, 0);

	// other initializations to 0
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_objects, 0, sizeof(object) * 128);
	memset(&m_oam, 0, sizeof(m_oam));
	memset(&m_mode7, 0, sizeof(m_mode7));

	for (auto & elem : m_scanlines)
	{
		elem.enable = 0;
		elem.clip = 0;
		memset(elem.buffer, 0, SNES_SCR_WIDTH * sizeof(uint16_t));
		memset(elem.priority, 0, SNES_SCR_WIDTH);
		memset(elem.layer, 0, SNES_SCR_WIDTH);
		memset(elem.blend_exception, 0, SNES_SCR_WIDTH);
	}

	for (int i = 0; i < 6; i++)
	{
		m_layer[i].window1_enabled = 0;
		m_layer[i].window1_invert = 0;
		m_layer[i].window2_enabled = 0;
		m_layer[i].window2_invert = 0;
		m_layer[i].wlog_mask = 0;
		m_layer[i].color_math = 0;
		m_layer[i].charmap = 0;
		m_layer[i].tilemap = 0;
		m_layer[i].tilemap_size = 0;
		m_layer[i].tile_size = 0;
		m_layer[i].mosaic_enabled = 0;
		m_layer[i].main_window_enabled = 0;
		m_layer[i].sub_window_enabled = 0;
		m_layer[i].main_bg_enabled = 0;
		m_layer[i].sub_bg_enabled = 0;
		m_layer[i].hoffs = 0;
		m_layer[i].voffs = 0;

		memset(m_clipmasks[i], 0, SNES_SCR_WIDTH);
	}
}

/*************************************************************************************************
 * SNES tiles
 *
 * The way vram is accessed to draw tiles is basically the same for both BG and OAM tiles. Main
 * differences are bit planes (variable for BG and fixed for OAM) and a few details of the scanline
 * output (since OAM has neither mosaic, nor hires, nor direct colors).
 * Hence, we use a common function to take data from VRAM and then we call specific routines for
 * OAM vs BG vs Hi-Res BG tiles.
 *************************************************************************************************/

/*****************************************
 * render_window()
 *
 * An example of how windows work:
 * Win1: ...#####......
 * Win2: ......#####...
 *             IN                 OUT
 * OR:   ...########...     ###........###
 * AND:  ......##......     ######..######
 * XOR:  ...###..###...     ###...##...###
 * XNOR: ###...##...###     ...###..###...
 *****************************************/

void snes_ppu_device::render_window(uint16_t layer_idx, uint8_t enable, uint8_t *output)
{
	layer_t &self = m_layer[layer_idx];
	if (!enable || (!self.window1_enabled && !self.window2_enabled))
	{
		memset(output, 0, 256);
		return;
	}

	if (self.window1_enabled && !self.window2_enabled)
	{
		const uint8_t set = 1 ^ self.window1_invert;
		const uint8_t clear = 1 - set;
		for (uint16_t x = 0; x < 256; x++)
		{
			output[x] = (x >= m_window1_left && x <= m_window1_right) ? set : clear;
		}
		return;
	}

	if (self.window2_enabled && !self.window1_enabled)
	{
		const uint8_t set = 1 ^ self.window2_invert;
		const uint8_t clear = 1 - set;
		for (uint16_t x = 0; x < 256; x++)
		{
			output[x] = (x >= m_window2_left && x <= m_window2_right) ? set : clear;
		}
		return;
	}

	for (uint16_t x = 0; x < 256; x++)
	{
		uint8_t one_mask = ((x >= m_window1_left && x <= m_window1_right) ? 1 : 0) ^ self.window1_invert;
		uint8_t two_mask = ((x >= m_window2_left && x <= m_window2_right) ? 1 : 0) ^ self.window2_invert;
		switch (self.wlog_mask)
		{
			case 0: output[x] = (one_mask | two_mask); break;
			case 1: output[x] = (one_mask & two_mask); break;
			case 2: output[x] = (one_mask ^ two_mask); break;
			case 3: output[x] = 1 - (one_mask ^ two_mask); break;
		}
	}
}

/*************************************************************************************************
 * SNES BG layers
 *
 * BG drawing theory of each scanline is quite easy: depending on the graphics Mode (0-7), there
 * are up to 4 background layers. Pixels for each BG layer can have two different priorities.
 * Depending on the line and on the BGHOFS and BGVOFS PPU registers, we first determine the tile
 * address in m_vram (by determining x,y coord and tile size and by calling get_tmap_addr).
 * Then, we load the correspondent data and we determine the tile properties: which priority to
 * use, which palette etc. Finally, for each pixel of the tile appearing on screen, we check if
 * the tile priority is higher than the BG/OAM already stored in that pixel for that line. If so
 * we store the pixel in the buffer, otherwise we discard it.
 *
 * Of course, depending on the graphics Mode, it might be easier or harder to determine the proper
 * tile address in vram (Mode 7 uses different registers, Mode 2, 4 and 6 uses OPT effect, etc.),
 * but in general it works as described.
 *************************************************************************************************/

inline uint32_t snes_ppu_device::get_tile( uint8_t layer_idx, uint32_t hoffset, uint32_t voffset )
{
	layer_t &self = m_layer[layer_idx];
	bool hires = m_mode == 5 || m_mode == 6;
	uint32_t tile_height = 3 + self.tile_size;
	uint32_t tile_width = !hires ? tile_height : 4;
	uint32_t screenx = self.tilemap_size & 1 ? 32 << 5 : 0;
	uint32_t screeny = self.tilemap_size & 2 ? 32 << (5 + (self.tilemap_size & 1)) : 0;
	uint32_t tilex = hoffset >> tile_width;
	uint32_t tiley = voffset >> tile_height;
	uint32_t offset = (tiley & 0x1f) << 5 | (tilex & 0x1f);
	if (tilex & 0x20) offset += screenx;
	if (tiley & 0x20) offset += screeny;
	uint32_t addr = ((self.tilemap + offset) & 0x7fff) << 1;
	return m_vram[addr] | (m_vram[addr + 1] << 8);
}

/*********************************************
 * plot_above()
 *
 * Update a main-screen pixel based on
 * priority.
 *********************************************/

inline void snes_ppu_device::plot_above( uint16_t x, uint8_t source, uint8_t priority, uint16_t color, int blend_exception )
{
	if (priority > m_scanlines[SNES_MAINSCREEN].priority[x])
	{
		m_scanlines[SNES_MAINSCREEN].priority[x] = priority;
		m_scanlines[SNES_MAINSCREEN].buffer[x] = color;
		m_scanlines[SNES_MAINSCREEN].layer[x] = source;
		m_scanlines[SNES_MAINSCREEN].blend_exception[x] = blend_exception;
	}
}

/*********************************************
 * plot_below()
 *
 * Update a sub-screen pixel based on
 * priority.
 *********************************************/

inline void snes_ppu_device::plot_below( uint16_t x, uint8_t source, uint8_t priority, uint16_t color, int blend_exception )
{
	if (priority > m_scanlines[SNES_SUBSCREEN].priority[x])
	{
		m_scanlines[SNES_SUBSCREEN].priority[x] = priority;
		m_scanlines[SNES_SUBSCREEN].buffer[x] = color;
		m_scanlines[SNES_SUBSCREEN].layer[x] = source;
		m_scanlines[SNES_SUBSCREEN].blend_exception[x] = blend_exception;
	}
}

/*********************************************
 * update_line()
 *
 * Update an entire line of tiles.
 *********************************************/

void snes_ppu_device::update_line( uint16_t curline, uint8_t layer_idx, uint8_t direct_colors )
{
	layer_t &layer = m_layer[layer_idx];

	if (layer.tile_mode == SNES_COLOR_DEPTH_NONE) return;

#if SNES_LAYER_DEBUG
	if (m_debug_options.bg_disabled[layer_idx])
		return;
#endif /* SNES_LAYER_DEBUG */

	m_scanlines[SNES_MAINSCREEN].enable = layer.main_bg_enabled;
	m_scanlines[SNES_SUBSCREEN].enable = layer.sub_bg_enabled;

	if (!m_scanlines[SNES_MAINSCREEN].enable && !m_scanlines[SNES_SUBSCREEN].enable)
	{
		return;
	}

	uint8_t window_above[256];
	uint8_t window_below[256];
	render_window(layer_idx, layer.main_window_enabled, window_above);
	render_window(layer_idx, layer.sub_window_enabled, window_below);

	bool hires = m_mode == 5 || m_mode == 6;
	bool opt_mode = m_mode == 2 || m_mode == 4 || m_mode == 6;
	bool direct_color_mode = direct_colors && layer_idx == SNES_BG1 && (m_mode == 3 || m_mode == 4);
	uint32_t color_shift = 3 + layer.tile_mode;
	int width = 256 << (hires ? 1 : 0);

	uint32_t tile_height = 3 + layer.tile_size;
	uint32_t tile_width = !hires ? tile_height : 4;
	uint32_t tile_mask = 0x0fff >> layer.tile_mode;
	uint32_t tiledata_index = layer.charmap >> (3 + layer.tile_mode);

	uint32_t palette_base = m_mode == 0 ? layer_idx << 5 : 0;
	uint32_t palette_shift = 2 << layer.tile_mode;

	uint32_t hscroll = layer.hoffs;
	uint32_t vscroll = layer.voffs;
	uint32_t hmask = (width << layer.tile_size << (layer.tilemap_size & 1)) - 1;
	uint32_t vmask = (width << layer.tile_size << ((layer.tilemap_size & 2) >> 1)) - 1;

	uint32_t y = layer.mosaic_enabled ? layer.mosaic_offset : curline;

	if (hires)
	{
		hscroll <<= 1;
		if (m_interlace == 2) y = (y & ~1) | (m_stat78 >> 7);
	}

	uint32_t mosaic_counter = 1;
	uint32_t mosaic_palette = 0;
	uint32_t mosaic_priority = 0;
	uint32_t mosaic_color = 0;

	int x = 0 - (hscroll & 7);
	while (x < width)
	{
		uint32_t hoffset = x + hscroll;
		uint32_t voffset = y + vscroll;
		if (opt_mode)
		{
			uint32_t valid_bit = 0x2000 << layer_idx;
			uint32_t offset_x = x + (hscroll & 7);
			if (offset_x >= 8) // first column is exempt
			{
				uint32_t hlookup = get_tile(SNES_BG3, (offset_x - 8) + (m_layer[SNES_BG3].hoffs & ~7), m_layer[SNES_BG3].voffs);
				if (m_mode == 4)
				{
					if (hlookup & valid_bit)
					{
						if (!(hlookup & 0x8000))
						{
							hoffset = offset_x + (hlookup & ~7);
						}
						else
						{
							voffset = y + hlookup;
						}
					}
				}
				else
				{
					uint32_t vlookup = get_tile(SNES_BG3, (offset_x - 8) + (m_layer[SNES_BG3].hoffs & ~7), m_layer[SNES_BG3].voffs + 8);
					if (hlookup & valid_bit)
					{
						hoffset = offset_x + (hlookup & ~7);
					}
					if (vlookup & valid_bit)
					{
						voffset = y + vlookup;
					}
				}
			}
		}

		hoffset &= hmask;
		voffset &= vmask;

		uint32_t tile_number = get_tile(layer_idx, hoffset, voffset);
		uint32_t mirrory = tile_number & 0x8000 ? 7 : 0;
		uint32_t mirrorx = tile_number & 0x4000 ? 7 : 0;
		uint8_t tile_priority = layer.priority[BIT(tile_number, 13)];
		uint32_t palette_number = tile_number >> 10 & 7;
		uint32_t palette_index = (palette_base + (palette_number << palette_shift)) & 0xff;

		if (tile_width  == 4 && (bool(hoffset & 8) ^ bool(mirrorx))) tile_number +=  1;
		if (tile_height == 4 && (bool(voffset & 8) ^ bool(mirrory))) tile_number += 16;
		tile_number = ((tile_number & 0x03ff) + tiledata_index) & tile_mask;

		uint16_t address = (((tile_number << color_shift) + ((voffset & 7) ^ mirrory)) & 0x7fff) << 1;

		uint64_t data;
		data  = (uint64_t)m_vram[address +   0] <<  0;
		data |= (uint64_t)m_vram[address +   1] <<  8;
		data |= (uint64_t)m_vram[address +  16] << 16;
		data |= (uint64_t)m_vram[address +  17] << 24;
		data |= (uint64_t)m_vram[address +  32] << 32;
		data |= (uint64_t)m_vram[address +  33] << 40;
		data |= (uint64_t)m_vram[address +  48] << 48;
		data |= (uint64_t)m_vram[address +  49] << 56;

		for (uint32_t tilex = 0; tilex < 8; tilex++, x++)
		{
			if (x & width) continue;
			if (!layer.mosaic_enabled || --mosaic_counter == 0)
			{
				uint32_t color, shift = mirrorx ? tilex : 7 - tilex;
				{
					color  = data >> (shift +  0) & 0x01;
					color |= data >> (shift +  7) & 0x02;
				}
				if (layer.tile_mode >= SNES_COLOR_DEPTH_4BPP)
				{
					color |= data >> (shift + 14) & 0x04;
					color |= data >> (shift + 21) & 0x08;
				}
				if (layer.tile_mode >= SNES_COLOR_DEPTH_8BPP)
				{
					color |= data >> (shift + 28) & 0x10;
					color |= data >> (shift + 35) & 0x20;
					color |= data >> (shift + 42) & 0x40;
					color |= data >> (shift + 49) & 0x80;
				}

				mosaic_counter = 1 + m_mosaic_size;
				mosaic_palette = color;
				mosaic_priority = tile_priority;
				if (direct_color_mode)
				{
					mosaic_color = direct_color(mosaic_palette, palette_number);
				}
				else
				{
					mosaic_color = pen_indirect((palette_index + mosaic_palette) & 0xff);
				}
			}
			if (!mosaic_palette) continue;

			if (!hires)
			{
				if (layer.main_bg_enabled && window_above[x] == 0) plot_above(x, layer_idx, mosaic_priority, mosaic_color);
				if (layer.sub_bg_enabled  && window_below[x] == 0) plot_below(x, layer_idx, mosaic_priority, mosaic_color);
			}
			else
			{
				uint32_t _x = x >> 1;
				if (x & 1)
				{
					if (layer.main_bg_enabled && window_above[_x] == 0) plot_above(_x, layer_idx, mosaic_priority, mosaic_color);
				}
				else
				{
					if (layer.sub_bg_enabled  && window_below[_x] == 0) plot_below(_x, layer_idx, mosaic_priority, mosaic_color);
				}
			}
		}
	}
}


/*********************************************
 * update_line_mode7()
 *
 * Update an entire line of mode7 tiles.
 *********************************************/

#define MODE7_CLIP(x) (((x) & 0x2000) ? ((x) | ~0x03ff) : ((x) & 0x03ff))

void snes_ppu_device::update_line_mode7( uint16_t curline, uint8_t layer_idx )
{
	layer_t &self = m_layer[layer_idx];
	int _y = self.mosaic_enabled ? self.mosaic_offset : curline;
	int y = !m_mode7.vflip ? _y : 255 - _y;

	int a = m_mode7.matrix_a;
	int b = m_mode7.matrix_b;
	int c = m_mode7.matrix_c;
	int d = m_mode7.matrix_d;
	int hcenter = util::sext(m_mode7.origin_x, 13);
	int vcenter = util::sext(m_mode7.origin_y, 13);
	int hoffset = util::sext(m_mode7.hor_offset, 13);
	int voffset = util::sext(m_mode7.ver_offset, 13);

	uint32_t mosaic_counter = 1;
	uint32_t mosaic_palette = 0;
	uint8_t mosaic_priority = 0;
	uint16_t mosaic_color = 0;

	int origin_x = (a * MODE7_CLIP(hoffset - hcenter) & ~63) + (b * MODE7_CLIP(voffset - vcenter) & ~63) + (b * y & ~63) + (hcenter << 8);
	int origin_y = (c * MODE7_CLIP(hoffset - hcenter) & ~63) + (d * MODE7_CLIP(voffset - vcenter) & ~63) + (d * y & ~63) + (vcenter << 8);

	uint8_t window_above[256];
	uint8_t window_below[256];
	render_window(layer_idx, self.main_window_enabled, window_above);
	render_window(layer_idx, self.sub_window_enabled,  window_below);

	for (int _x = 0; _x < 256; _x++)
	{
		int x = !m_mode7.hflip ? _x : 255 - _x;
		int pixel_x = (origin_x + (a * x)) >> 8;
		int pixel_y = (origin_y + (c * x)) >> 8;
		int tile_x = (pixel_x >> 3) & 0x7f;
		int tile_y = (pixel_y >> 3) & 0x7f;
		bool out_of_bounds = (pixel_x | pixel_y) & ~0x03ff;
		uint16_t tile_address = (tile_y * 0x80 + tile_x) << 1;
		uint16_t palette_address = (((pixel_y & 7) << 3) + (pixel_x & 7)) & 0x7fff;
		uint8_t tile = m_mode7.repeat == 3 && out_of_bounds ? 0 : m_vram[tile_address];
		uint8_t palette = m_mode7.repeat == 2 && out_of_bounds ? 0 : m_vram[((tile << 6 | palette_address) << 1) | 1];

		uint8_t priority = 0;
		if (layer_idx == SNES_BG1)
		{
			priority = self.priority[0];
		}
		else if (layer_idx == SNES_BG2)
		{
			priority = self.priority[palette >> 7];
			palette &= 0x7f;
		}

		if (!self.mosaic_enabled || --mosaic_counter == 0)
		{
			mosaic_counter = 1 + m_mosaic_size;
			mosaic_palette = palette;
			mosaic_priority = priority;
			if (m_direct_color && layer_idx == SNES_BG1)
			{
				mosaic_color = direct_color(palette, 0);
			}
			else
			{
				mosaic_color = pen_indirect(palette);
			}
		}
		if (!mosaic_palette) continue;

		if (self.main_bg_enabled && window_above[_x] == 0) plot_above(_x, layer_idx, mosaic_priority, mosaic_color);
		if (self.sub_bg_enabled  && window_below[_x] == 0) plot_below(_x, layer_idx, mosaic_priority, mosaic_color);
	}
}

/*********************************************
 * update_objects()
 *
 * Update an entire line of sprites.
 *********************************************/

void snes_ppu_device::update_objects( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.bg_disabled[SNES_OAM])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!m_layer[SNES_OAM].main_bg_enabled && !m_layer[SNES_OAM].sub_bg_enabled)
		return;

	uint8_t window_above[256];
	uint8_t window_below[256];
	render_window(SNES_OAM, m_layer[SNES_OAM].main_window_enabled, window_above);
	render_window(SNES_OAM, m_layer[SNES_OAM].sub_window_enabled, window_below);

	uint32_t item_count = 0;
	uint32_t tile_count = 0;
	object_item items[32];
	object_tile tiles[34];
	memset(items, 0, sizeof(object_item) * 32);
	memset(tiles, 0, sizeof(object_tile) * 34);

	for (uint32_t n = 0; n < 128; n++)
	{
		object_item item = { true, (uint8_t)((m_oam.first + n) & 0x7f), 0, 0 };
		const object &obj = m_objects[item.index];

		if (obj.size == 0)
		{
			static const uint8_t s_widths[8]  = { 8, 8, 8, 16, 16, 32, 16, 16 };
			static const uint8_t s_heights[8] = { 8, 8, 8, 16, 16, 32, 32, 32 };
			item.width  = s_widths [m_oam.base_size];
			item.height = s_heights[m_oam.base_size];
			if (m_oam.interlace && m_oam.base_size >= 6) item.height = 16; // hardware quirk
		}
		else
		{
			static const uint8_t s_widths[8]  = { 16, 32, 64, 32, 64, 64, 32, 32 };
			static const uint8_t s_heights[8] = { 16, 32, 64, 32, 64, 64, 64, 32 };
			item.width  = s_widths [m_oam.base_size];
			item.height = s_heights[m_oam.base_size];
		}

		if (obj.x > SNES_SCR_WIDTH && (obj.x + item.width - 1) < (SNES_SCR_WIDTH * 2)) continue;
		uint32_t height = item.height >> m_oam.interlace;
		if ((curline >= obj.y && curline < (obj.y + height)) || ((obj.y + height) >= 256 && curline < ((obj.y + height) & 255)))
		{
			if (item_count++ >= 32) break;
			items[item_count - 1] = item;
		}
	}

	for (int n = 31; n >= 0; n--)
	{
		object_item &item = items[n];
		if (!item.valid) continue;

		const object &obj = m_objects[item.index];
		uint32_t tile_width = item.width >> 3;
		uint16_t x = obj.x;
		uint16_t y = (curline - obj.y) & 0xff;
		if (m_oam.interlace) y <<= 1;

		if (obj.vflip)
		{
			if (item.width == item.height)
				y = item.height - 1 - y;
			else if (y < item.width)
				y = item.width - 1 - y;
			else
				y = item.width + (item.width - 1) - (y - item.width);
		}

		if (m_oam.interlace)
		{
			y = !obj.vflip ? (y + BIT(m_stat78, 7)) : (y - BIT(m_stat78, 7));
		}

		x &= 0x1ff;
		y &= 0x0ff;

		uint16_t tile_data_address = m_oam.tile_data_address;
		if (obj.name_select) tile_data_address += (m_oam.name_select + 1) << 12;
		uint16_t character_x =  (obj.character & 15);
		uint16_t character_y = (((obj.character >> 4) + (y >> 3)) & 15) << 4;

		for (uint32_t tile_x = 0; tile_x < tile_width; tile_x++)
		{
			uint32_t object_x = (x + (tile_x << 3)) & 0x1ff;
			if (x != SNES_SCR_WIDTH && object_x >= SNES_SCR_WIDTH && (object_x + 7) < (SNES_SCR_WIDTH * 2)) continue;

			object_tile tile = { true, 0, 0, 0, 0, 0, 0 };
			tile.x = object_x;
			tile.y = y;
			tile.pri = obj.pri;
			tile.pal = 128 + (obj.pal << 4);
			tile.hflip = obj.hflip;

			uint32_t mirror_x = !obj.hflip ? tile_x : (tile_width - 1 - tile_x);
			uint32_t address = tile_data_address + ((character_y + ((character_x + mirror_x) & 15)) << 4);
			address = ((address & 0x7ff0) + (y & 7)) << 1;
			tile.data  = m_vram[address +  0] <<  0;
			tile.data |= m_vram[address +  1] <<  8;
			tile.data |= m_vram[address + 16] << 16;
			tile.data |= m_vram[address + 17] << 24;

			if (tile_count++ >= 34) break;
			tiles[tile_count - 1] = tile;
		}
	}

	/* set Range Over flag if necessary */
	if (item_count > 32)
		m_stat77 |= 0x40;

	/* set Time Over flag if necessary */
	if (tile_count > 34)
		m_stat77 |= 0x80;

	uint8_t palbuf[256] = {};
	uint8_t pribuf[256] = {};

	for (uint32_t n = 0; n < 34; n++)
	{
		object_tile &tile = tiles[n];
		if (!tile.valid) continue;

		uint32_t tile_x = tile.x;
		for (uint32_t x = 0; x < 8; x++)
		{
			tile_x &= 0x1ff;
			if (tile_x < SNES_SCR_WIDTH)
			{
				uint32_t color, shift = tile.hflip ? x : (7 - x);
				color  = (tile.data >> (shift +  0)) & 0x01;
				color |= (tile.data >> (shift +  7)) & 0x02;
				color |= (tile.data >> (shift + 14)) & 0x04;
				color |= (tile.data >> (shift + 21)) & 0x08;
				if (color)
				{
					palbuf[tile_x] = tile.pal + color;
					pribuf[tile_x] = m_oam.priority[tile.pri];
				}
			}
			tile_x++;
		}
	}

	for (uint32_t x = 0; x < SNES_SCR_WIDTH; x++)
	{
		if (!pribuf[x]) continue;
		int blend = (palbuf[x] < 192) ? 1 : 0;
		if (m_layer[SNES_OAM].main_bg_enabled && window_above[x] == 0) plot_above(x, SNES_OAM, pribuf[x], pen_indirect(palbuf[x]), blend);
		if (m_layer[SNES_OAM].sub_bg_enabled  && window_below[x] == 0) plot_below(x, SNES_OAM, pribuf[x], pen_indirect(palbuf[x]), blend);
	}
}

void snes_ppu_device::oam_address_reset( void )
{
	m_oam.address = m_oam.base_address;
	oam_set_first_object();
}

void snes_ppu_device::oam_set_first_object( void )
{
	m_oam.first = (!m_oam.priority_rotation ? 0 : ((m_oam.address >> 2) & 0x7f));
}


/*********************************************
 * snes_update_mode_X()
 *
 * Update Mode X line.
 *********************************************/

void snes_ppu_device::update_mode_0( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[0])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_line(curline, SNES_BG2, 0);
	update_line(curline, SNES_BG3, 0);
	update_line(curline, SNES_BG4, 0);
}

void snes_ppu_device::update_mode_1( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[1])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_line(curline, SNES_BG2, 0);
	update_line(curline, SNES_BG3, 0);
}

void snes_ppu_device::update_mode_2( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[2])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_line(curline, SNES_BG2, 0);
}

void snes_ppu_device::update_mode_3( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[3])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, m_direct_color);
	update_line(curline, SNES_BG2, 0);
}

void snes_ppu_device::update_mode_4( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[4])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, m_direct_color);
	update_line(curline, SNES_BG2, 0);
}

void snes_ppu_device::update_mode_5( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[5])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_line(curline, SNES_BG2, 0);
}

void snes_ppu_device::update_mode_6( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[6])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
}

void snes_ppu_device::update_mode_7( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[7])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!m_mode7.extbg)
	{
		update_line_mode7(curline, SNES_BG1);
	}
	else
	{
		update_line_mode7(curline, SNES_BG1);
		update_line_mode7(curline, SNES_BG2);
	}
}

/*********************************************
 * snes_draw_screens()
 *
 * Draw the whole screen (Mode 0 -> 7).
 *********************************************/

void snes_ppu_device::draw_screens( uint16_t curline )
{
	switch (m_mode)
	{
		case 0: update_mode_0(curline); break;     /* Mode 0 */
		case 1: update_mode_1(curline); break;     /* Mode 1 */
		case 2: update_mode_2(curline); break;     /* Mode 2 - Supports offset per tile */
		case 3: update_mode_3(curline); break;     /* Mode 3 - Supports direct colour */
		case 4: update_mode_4(curline); break;     /* Mode 4 - Supports offset per tile and direct colour */
		case 5: update_mode_5(curline); break;     /* Mode 5 - Supports hires */
		case 6: update_mode_6(curline); break;     /* Mode 6 - Supports offset per tile and hires */
		case 7: update_mode_7(curline); break;     /* Mode 7 - Supports direct colour */
	}
}

/*********************************************
 * update_color_windowmasks()
 *
 * An example of how windows work:
 * Win1: ...#####......
 * Win2: ......#####...
 *             IN                 OUT
 * OR:   ...########...     ###........###
 * AND:  ......##......     ######..######
 * XOR:  ...###..###...     ###...##...###
 * XNOR: ###...##...###     ...###..###...
 *********************************************/

void snes_ppu_device::update_color_windowmasks( uint8_t mask, uint8_t *output )
{
	layer_t &self = m_layer[SNES_COLOR];
	uint8_t set = 0, clear = 0;
	switch (mask)
	{
		case 0: memset(output, 1, 256); return; // always
		case 1: set = 1; clear = 0; break;
		case 2: set = 0; clear = 1; break;
		case 3: memset(output, 0, 256); return; // never
	}

	if (!self.window1_enabled && !self.window2_enabled)
	{
		memset(output, clear, 256);
		return;
	}

	if (self.window1_enabled && !self.window2_enabled)
	{
		if (self.window1_invert)
		{
			set ^= 1;
			clear ^= 1;
		}
		for (uint16_t x = 0; x < 256; x++)
		{
			output[x] = (x >= m_window1_left && x <= m_window1_right) ? set : clear;
		}
		return;
	}

	if (self.window2_enabled && !self.window1_enabled)
	{
		if (self.window2_invert)
		{
			set ^= 1;
			clear ^= 1;
		}
		for (uint16_t x = 0; x < 256; x++)
		{
			output[x] = (x >= m_window2_left && x <= m_window2_right) ? set : clear;
		}
		return;
	}

	for (uint16_t x = 0; x < 256; x++)
	{
		uint8_t one_mask = ((x >= m_window1_left && x <= m_window1_right) ? 1 : 0) ^ self.window1_invert;
		uint8_t two_mask = ((x >= m_window2_left && x <= m_window2_right) ? 1 : 0) ^ self.window2_invert;
		switch (self.wlog_mask)
		{
			case 0: output[x] = (one_mask | two_mask) == 1 ? set : clear; break;
			case 1: output[x] = (one_mask & two_mask) == 1 ? set : clear; break;
			case 2: output[x] = (one_mask ^ two_mask) == 1 ? set : clear; break;
			case 3: output[x] = (one_mask ^ two_mask) == 0 ? set : clear; break;
		}
	}
}

/*********************************************
 * refresh_scanline()
 *
 * Redraw the current line.
 *********************************************/
/*********************************************
 * Notice that in hires and pseudo hires modes,
 * i.e. when 512 different pixels are present
 * in a scanline, a crt TV monitor would end
 * up blending adjacent pixels. To mimic this,
 * we add a small (optional) hack which enters
 * only in the very last stage of the scanline
 * drawing and which simulates the TV by
 * replacing the exact pixel color with an
 * average of the current and next pixel colors.
 * Credits (and thanks) to Blargg and Byuu for
 * the optimized averaging algorithm.
 *********************************************/

void snes_ppu_device::refresh_scanline( bitmap_rgb32 &bitmap, uint16_t curline )
{
	bool blurring = m_options.read_safe(0) & 0x01;

	auto profile = g_profiler.start(PROFILER_VIDEO);

	cache_background();

	if (m_screen_disabled) /* screen is forced blank */
		for (int x = 0; x < SNES_SCR_WIDTH * 2; x++)
			bitmap.pix(0, x) = rgb_t::black();
	else
	{
		uint8_t window_above[256];
		uint8_t window_below[256];

		/* Clear priority */
		memset(m_scanlines[SNES_MAINSCREEN].priority, 0, SNES_SCR_WIDTH);
		memset(m_scanlines[SNES_SUBSCREEN].priority, 0, SNES_SCR_WIDTH);

		/* Clear layers */
		memset(m_scanlines[SNES_MAINSCREEN].layer, SNES_COLOR, SNES_SCR_WIDTH);
		memset(m_scanlines[SNES_SUBSCREEN].layer, SNES_COLOR, SNES_SCR_WIDTH);

		/* Clear blend_exception (only used for OAM) */
		memset(m_scanlines[SNES_MAINSCREEN].blend_exception, 0, SNES_SCR_WIDTH);
		memset(m_scanlines[SNES_SUBSCREEN].blend_exception, 0, SNES_SCR_WIDTH);

		struct SNES_SCANLINE *above = &m_scanlines[SNES_MAINSCREEN];
		struct SNES_SCANLINE *below = &m_scanlines[SNES_SUBSCREEN];
#if SNES_LAYER_DEBUG
		if (dbg_video(curline))
			return;

		/* Toggle drawing of SNES_SUBSCREEN or SNES_MAINSCREEN */
		if (m_debug_options.draw_subscreen)
		{
			above = &m_scanlines[SNES_SUBSCREEN];
			below = &m_scanlines[SNES_MAINSCREEN];
		}
#endif

		const bool hires = m_mode == 5 || m_mode == 6 || m_pseudo_hires;
		uint16_t above_color = pen_indirect(0);
		uint16_t below_color = hires ? pen_indirect(0) : pen_indirect(FIXED_COLOUR);
		for (int x = 0; x < SNES_SCR_WIDTH; x++)
		{
			above->buffer[x] = above_color;
			above->priority[x] = 0;
			above->layer[x] = SNES_COLOR;

			below->buffer[x] = below_color;
			below->priority[x] = 0;
			below->layer[x] = SNES_COLOR;
		}

		update_color_windowmasks(m_clip_to_black, window_above);
		update_color_windowmasks(m_prevent_color_math, window_below);

		/* Draw backgrounds */
		draw_screens(curline);

		/* Draw OAM */
		update_objects(curline);

		/* Draw the scanline to screen */
		uint16_t prev = 0;

		uint16_t *luma = &m_light_table[m_screen_brightness][0];
		for (int x = 0; x < SNES_SCR_WIDTH; x++)
		{
			/* in hires, the first pixel (of 512) is subscreen pixel, then the first mainscreen pixel follows, and so on... */
			if (!hires)
			{
				const uint16_t c = luma[pixel(x, above, below, window_above, window_below)];

				bitmap.pix(0, x * 2 + 0) = indirect_color(c & 0x7fff);
				bitmap.pix(0, x * 2 + 1) = indirect_color(c & 0x7fff);
			}
			else if (!blurring)
			{
				const uint16_t c0 = luma[pixel(x, below, above, window_above, window_below)];
				const uint16_t c1 = luma[pixel(x, above, below, window_above, window_below)];

				bitmap.pix(0, x * 2 + 0) = indirect_color(c0 & 0x7fff);
				bitmap.pix(0, x * 2 + 1) = indirect_color(c1 & 0x7fff);
			}
			else
			{
				uint16_t curr = luma[pixel(x, below, above, window_above, window_below)];

				uint16_t c0 = (prev + curr - ((prev ^ curr) & 0x0421)) >> 1;
				bitmap.pix(0, x * 2 + 0) = indirect_color(c0 & 0x7fff);

				prev = curr;
				curr = luma[pixel(x, above, below, window_above, window_below)];

				uint16_t c1 = (prev + curr - ((prev ^ curr) & 0x0421)) >> 1;
				bitmap.pix(0, x * 2 + 1) = indirect_color(c1 & 0x7fff);

				prev = curr;
			}
		}
	}
}

uint16_t snes_ppu_device::pixel(uint16_t x, SNES_SCANLINE *above, SNES_SCANLINE *below, uint8_t *window_above, uint8_t *window_below)
{
	if (!window_above[x]) above->buffer[x] = 0;
	if (!window_below[x]) return above->buffer[x];
	if (!m_layer[above->layer[x]].color_math || (above->layer[x] == SNES_OAM && above->blend_exception[x])) return above->buffer[x];
	if (!m_sub_add_mode) return blend(above->buffer[x], pen_indirect(FIXED_COLOUR), BIT(m_color_modes, 0) != 0 && window_above[x] != 0);
	return blend(above->buffer[x], below->buffer[x], BIT(m_color_modes, 0) != 0 && window_above[x] != 0 && below->layer[x] != SNES_COLOR);
}

inline uint16_t snes_ppu_device::blend( uint16_t x, uint16_t y, bool halve )
{
	if (!BIT(m_color_modes, 1)) // add
	{
		if (!halve)
		{
			uint16_t sum = x + y;
			uint16_t carry = (sum - ((x ^ y) & 0x0421)) & 0x8420;
			return (sum - carry) | (carry - (carry >> 5));
		}
		else
		{
			return (x + y - ((x ^ y) & 0x0421)) >> 1;
		}
	}
	else // sub
	{
		uint16_t diff = x - y + 0x8420;
		uint16_t borrow = (diff - ((x ^ y) & 0x8420)) & 0x8420;
		if (!halve)
		{
			return (diff - borrow) & (borrow - (borrow >> 5));
		}
		else
		{
			return (((diff - borrow) & (borrow - (borrow >> 5))) & 0x7bde) >> 1;
		}
	}
}

/* CPU <-> PPU comms */

// full graphic variables
static const uint16_t vram_fgr_inctab[4] = { 1, 32, 128, 128 };
static const uint16_t vram_fgr_inccnts[4] = { 0, 32, 64, 128 };
static const uint16_t vram_fgr_shiftab[4] = { 0, 5, 6, 7 };

// utility function - latches the H/V counters.  Used by IRQ, writes to WRIO, etc.
void snes_ppu_device::set_latch_hv(int16_t x, int16_t y)
{
	m_beam.latch_vert = y;
	m_beam.latch_horz = x / m_htmult;
	m_stat78 |= 0x40;   // indicate we latched

//  printf("latched @ H %d V %d\n", m_beam.latch_horz, m_beam.latch_vert);
}

void snes_ppu_device::dynamic_res_change()
{
	rectangle visarea = screen().visible_area();
	attoseconds_t refresh;

	visarea.min_x = visarea.min_y = 0;
	visarea.max_y = m_beam.last_visible_line * m_interlace - 1;
	visarea.max_x = (SNES_SCR_WIDTH * 2) - 1;

	// fixme: should compensate for SNES_DBG_VIDEO
	if (m_mode == 5 || m_mode == 6 || m_pseudo_hires)
		m_htmult = 2;
	else
		m_htmult = 1;

	/* FIXME: does the timing changes when the gfx mode is equal to 5 or 6? */
	if ((m_stat78 & 0x10) == SNES_NTSC)
	{
		refresh = HZ_TO_ATTOSECONDS(DOTCLK_NTSC) * SNES_HTOTAL * SNES_VTOTAL_NTSC;
		screen().configure(SNES_HTOTAL * m_htmult, SNES_VTOTAL_NTSC * m_interlace, visarea, refresh);
	}
	else
	{
		refresh = HZ_TO_ATTOSECONDS(DOTCLK_PAL) * SNES_HTOTAL * SNES_VTOTAL_PAL;
		screen().configure(SNES_HTOTAL * m_htmult, SNES_VTOTAL_PAL * m_interlace, visarea, refresh);
	}
}

/*************************************************

 SNES VRAM accesses:

 VRAM accesses during active display are invalid.
 Unlike OAM and CGRAM, they will not be written
 anywhere at all. Thanks to byuu's researches,
 the ranges where writes are invalid have been
 validated on hardware, as has the edge case where
 the S-CPU open bus can be written if the write
 occurs during the very last clock cycle of
 vblank.
 Our implementation could be not 100% accurate
 when interlace is active.
*************************************************/

inline uint32_t snes_ppu_device::get_vram_address()
{
	uint32_t addr = m_vmadd;

	if (m_vram_fgr_count)
	{
		uint32_t rem = addr & m_vram_fgr_mask;
		uint32_t faddr = (addr & ~m_vram_fgr_mask) + (rem >> m_vram_fgr_shift) + ((rem & (m_vram_fgr_count - 1)) << 3);
		return faddr << 1;
	}

	return addr << 1;
}

uint8_t snes_ppu_device::vram_read(offs_t offset)
{
	uint8_t res;
	offset &= 0xffff; // only 64KB are present on SNES

	if (m_screen_disabled)
		res = m_vram[offset];
	else
	{
		uint16_t v = screen().vpos();
		uint16_t h = screen().hpos();
		uint16_t ls = (((m_stat78 & 0x10) == SNES_NTSC ? 525 : 625) >> 1) - 1;

		if (m_interlace == 2)
			ls++;

		if (v == ls && h == 1362)
			res = 0;
		else if (v < m_beam.last_visible_line - 1)
			res = 0;
		else if (v == m_beam.last_visible_line - 1)
		{
			if (h == 1362)
				res = m_vram[offset];
			else
			{
				//printf("%d %d VRAM read, CHECK!\n",h,v);
				res = 0;
			}
		}
		else
			res = m_vram[offset];
	}
	return res;
}

void snes_ppu_device::vram_write(offs_t offset, uint8_t data)
{
	offset &= 0xffff; // only 64KB are present on SNES, Robocop 3 relies on this

	if (m_screen_disabled)
		m_vram[offset] = data;
	else
	{
		uint16_t v = screen().vpos();
		uint16_t h = screen().hpos();
		if (v == 0)
		{
			if (h <= 4)
				m_vram[offset] = data;
			else if (h == 6)
				m_vram[offset] = m_openbus_cb(0);
			else
			{
				//printf("%d %d VRAM write, CHECK!\n",h,v);
				//no write
			}
		}
		else if (v < m_beam.last_visible_line)
		{
			//printf("%d %d VRAM write, CHECK!\n",h,v);
			//no write
		}
		else if (v == m_beam.last_visible_line)
		{
			if (h <= 4)
			{
				//printf("%d %d VRAM write, CHECK!\n",h,v);
				//no write
			}
			else
				m_vram[offset] = data;
		}
		else
			m_vram[offset] = data;
	}
}

/*************************************************

 SNES OAM accesses:

 OAM accesses during active display are allowed.
 The actual address varies during rendering, as the
 PPU reads in data itself for processing.
 Unfortunately, no one has been able (yet) to
 determine how this works. The only known game to
 actually access OAM during active display is
 Uniracers and it expects accesses to map to
 offset 0x0218. Hence, following byuu's choice
 we rerouted OAM accesses during active display
 to 0x0218.
 This is a hack, but it is more accurate than
 writing to the 'expected' address set by
 $2102,$2103.
*************************************************/

uint8_t snes_ppu_device::read_oam( uint16_t address )
{
	if (!m_screen_disabled && screen().vpos() < m_beam.last_visible_line) address = m_oam.write_latch;
	return read_object(address);
}

uint8_t snes_ppu_device::read_object( uint16_t address )
{
	if (!(address & 0x0200))
	{
		uint8_t n = (address >> 2) & 0x7f;
		object &obj = m_objects[n];
		switch (address & 3)
		{
		case 0:  return (uint8_t)obj.x;
		case 1:  return obj.y;
		case 2:  return obj.character;
		default: return (obj.vflip << 7) | (obj.hflip << 6) | (obj.pri << 4) | (obj.pal << 1) | obj.name_select;
		}
	}
	else
	{
		uint8_t n = (address & 0x1f) << 2;
		return (BIT(m_objects[n + 0].x, 8) << 0) |
			   (BIT(m_objects[n + 1].x, 8) << 2) |
			   (BIT(m_objects[n + 2].x, 8) << 4) |
			   (BIT(m_objects[n + 3].x, 8) << 6) |
			   (m_objects[n + 0].size << 1) |
			   (m_objects[n + 1].size << 3) |
			   (m_objects[n + 2].size << 5) |
			   (m_objects[n + 3].size << 7);
	}
}

void snes_ppu_device::write_oam( uint16_t address, uint8_t data )
{
	if (!m_screen_disabled && screen().vpos() < m_beam.last_visible_line) address = 0x0218;
	return write_object(address, data);
}

void snes_ppu_device::write_object( uint16_t address, uint8_t data )
{
	if (!(address & 0x0200))
	{
		const uint8_t n = (address >> 2) & 0x7f;
		object &obj = m_objects[n];
		switch (address & 3)
		{
		case 0:  obj.x = (obj.x & 0x100) | data; return;
		case 1:  obj.y = data + 1; return; // +1: rendering happens one scanline late
		case 2:  obj.character = data; return;
		default:
			obj.name_select = BIT(data, 0);
			obj.pal = (data >> 1) & 7;
			obj.pri = (data >> 4) & 3;
			obj.hflip = BIT(data, 6);
			obj.vflip = BIT(data, 7);
			return;
		}
	}
	else
	{
		const uint8_t n = (address & 0x1f) << 2;
		m_objects[n + 0].x = (m_objects[n + 0].x & 0xff) | ((data << 8) & 0x100);
		m_objects[n + 1].x = (m_objects[n + 1].x & 0xff) | ((data << 6) & 0x100);
		m_objects[n + 2].x = (m_objects[n + 2].x & 0xff) | ((data << 4) & 0x100);
		m_objects[n + 3].x = (m_objects[n + 3].x & 0xff) | ((data << 2) & 0x100);
		m_objects[n + 0].size = BIT(data, 1);
		m_objects[n + 1].size = BIT(data, 3);
		m_objects[n + 2].size = BIT(data, 5);
		m_objects[n + 3].size = BIT(data, 7);
	}
}

/*************************************************

 SNES CGRAM accesses:

 CGRAM writes during hblank are valid. During
 active display, the actual address the data
 is written to varies, as the PPU itself changes
 the address. Like OAM, it is not known the exact
 algorithm used, but no commercial software seems
 to attempt this. While byuu, in his emu, maps
 those accesses to 0x01ff, because it is more
 accurate to invalidate the 'expected' address
 than not, MAME has issues if we don't write to
 the expected address (see e.g. Tokimeki Memorial).
 This is because writes should work during hblank
 (so that the game can produce color fading), but
 ends up not working with the conditions below.
 Hence, for the moment, we only document the
 solution adopted by BSNES without enabling it.
*************************************************/

uint8_t snes_ppu_device::cgram_read(offs_t offset)
{
	uint8_t res;
	offset &= 0x1ff;

#if 0
	if (!m_screen_disabled)
	{
		uint16_t v = screen().vpos();
		uint16_t h = screen().hpos();

		if (v < m_beam.last_visible_line && h >= 128 && h < 1096)
			offset = 0x1ff;
	}
#endif

	res = ((uint8_t *)m_cgram.get())[offset];

	// CGRAM palette data format is 15-bits (0,bbbbb,ggggg,rrrrr).
	// Highest bit is simply ignored.
	if (offset & 0x01)
		res &= 0x7f;

	return res;
}

void snes_ppu_device::cgram_write(offs_t offset, uint8_t data)
{
	offset &= 0x1ff;

#if 0
	// FIXME: this currently breaks some games (e.g. Tokimeki Memorial),
	// even if it's expected to be more accurate than allowing for
	// writes to the cgram address
	if (!m_screen_disabled)
	{
		uint16_t v = screen().vpos();
		uint16_t h = screen().hpos();

		if (v < m_beam.last_visible_line && h >= 128 && h < 1096)
			offset = 0x1ff;
	}
#endif

	// CGRAM palette data format is 15-bits (0,bbbbb,ggggg,rrrrr).
	// Highest bit is simply ignored.
	if (offset & 0x01)
		data &= 0x7f;

	((uint8_t *)m_cgram.get())[offset] = data;
	set_pen_indirect(offset >> 1, m_cgram[offset >> 1] & 0x7fff);
}

uint16_t snes_ppu_device::direct_color(uint16_t palette, uint16_t group)
{
  //palette = -------- BBGGGRRR
  //group   = -------- -----bgr
  //output  = 0BBb00GG Gg0RRRr0
  return pen_indirect(DIRECT_COLOUR + palette + (group * 256));
}

void snes_ppu_device::set_current_vert(uint16_t value)
{
	m_beam.current_vert = value;
}

void snes_ppu_device::cache_background()
{
	for (int i = SNES_BG1; i <= SNES_BG4; i++)
	{
		if (m_beam.current_vert == 1)
		{
			m_layer[i].mosaic_counter = m_mosaic_size + 1;
			m_layer[i].mosaic_offset = 1;
		}
		else if (--m_layer[i].mosaic_counter == 0)
		{
			m_layer[i].mosaic_counter = m_mosaic_size + 1;
			m_layer[i].mosaic_offset += m_mosaic_size + 1;
		}
	}
}

void snes_ppu_device::update_video_mode()
{
	switch (m_mode)
	{
	case 0:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG1].priority[0] = 8; m_layer[SNES_BG1].priority[1] = 11;
		m_layer[SNES_BG2].priority[0] = 7; m_layer[SNES_BG2].priority[1] = 10;
		m_layer[SNES_BG3].priority[0] = 2; m_layer[SNES_BG3].priority[1] =  5;
		m_layer[SNES_BG4].priority[0] = 1; m_layer[SNES_BG4].priority[1] =  4;
		static const uint8_t s_oam_priority[4] = { 3, 6, 9, 12 };
		memcpy(m_oam.priority, s_oam_priority, 4);
		break;
	}
	case 1:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		if (m_bg_priority)
		{
			m_layer[SNES_BG1].priority[0] = 5; m_layer[SNES_BG1].priority[1] =  8;
			m_layer[SNES_BG2].priority[0] = 4; m_layer[SNES_BG2].priority[1] =  7;
			m_layer[SNES_BG3].priority[0] = 1; m_layer[SNES_BG3].priority[1] = 10;
			static const uint8_t s_oam_priority[4] = { 2, 3, 6, 9 };
			memcpy(m_oam.priority, s_oam_priority, 4);
		}
		else
		{
			m_layer[SNES_BG1].priority[0] = 6; m_layer[SNES_BG1].priority[1] =  9;
			m_layer[SNES_BG2].priority[0] = 5; m_layer[SNES_BG2].priority[1] =  8;
			m_layer[SNES_BG3].priority[0] = 1; m_layer[SNES_BG3].priority[1] =  3;
			static const uint8_t s_oam_priority[4] = { 2, 4, 7, 10 };
			memcpy(m_oam.priority, s_oam_priority, 4);
		}
		break;
	}
	case 2:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		static const uint8_t s_oam_priority[4] = { 2, 4, 6, 8 };
		memcpy(m_oam.priority, s_oam_priority, 4);
		break;
	}
	case 3:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_8BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		static const uint8_t s_oam_priority[4] = { 2, 4, 6, 8 };
		memcpy(m_oam.priority, s_oam_priority, 4);
		break;
	}
	case 4:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_8BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		static const uint8_t s_oam_priority[4] = { 2, 4, 6, 8 };
		memcpy(m_oam.priority, s_oam_priority, 4);
		break;
	}
	case 5:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		static const uint8_t s_oam_priority[4] = { 2, 4, 6, 8 };
		memcpy(m_oam.priority, s_oam_priority, 4);
		break;
	}
	case 6:
	{
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 2; m_layer[SNES_BG1].priority[1] =  5;
		static const uint8_t s_oam_priority[4] = { 1, 3, 4, 6 };
		memcpy(m_oam.priority, s_oam_priority, 4);
		break;
	}
	case 7:
	{
		if (!m_mode7.extbg)
		{
			m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_MODE7;
			m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG1].priority[0] = 2; m_layer[SNES_BG1].priority[1] =  2;
			static const uint8_t s_oam_priority[4] = { 1, 3, 4, 5 };
			memcpy(m_oam.priority, s_oam_priority, 4);
		}
		else
		{
			m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_MODE7;
			m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_MODE7;
			m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  3;
			m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
			static const uint8_t s_oam_priority[4] = { 2, 4, 6, 7 };
			memcpy(m_oam.priority, s_oam_priority, 4);
		}
		break;
	}
	}
}

uint8_t snes_ppu_device::read(uint32_t offset, uint8_t wrio_bit7)
{
	uint8_t value;

	switch (offset)
	{
		case OAMDATA:   /* 21xy for x=0,1,2 and y=4,5,6,8,9,a returns PPU1 open bus*/
		case BGMODE:
		case MOSAIC:
		case BG2SC:
		case BG3SC:
		case BG4SC:
		case BG4VOFS:
		case VMAIN:
		case VMADDL:
		case VMDATAL:
		case VMDATAH:
		case M7SEL:
		case W34SEL:
		case WOBJSEL:
		case WH0:
		case WH2:
		case WH3:
		case WBGLOG:
			return m_ppu1_open_bus;

		case MPYL:      /* Multiplication result (low) */
			{
				/* Perform 16bit * 8bit multiply */
				uint32_t c = (int16_t)m_mode7.matrix_a * (int8_t)(m_mode7.matrix_b >> 8);
				m_ppu1_open_bus = c & 0xff;
				return m_ppu1_open_bus;
			}
		case MPYM:      /* Multiplication result (mid) */
			{
				/* Perform 16bit * 8bit multiply */
				uint32_t c = (int16_t)m_mode7.matrix_a * (int8_t)(m_mode7.matrix_b >> 8);
				m_ppu1_open_bus = (c >> 8) & 0xff;
				return m_ppu1_open_bus;
			}
		case MPYH:      /* Multiplication result (high) */
			{
				/* Perform 16bit * 8bit multiply */
				uint32_t c = (int16_t)m_mode7.matrix_a * (int8_t)(m_mode7.matrix_b >> 8);
				m_ppu1_open_bus = (c >> 16) & 0xff;
				return m_ppu1_open_bus;
			}
		case SLHV:      /* Software latch for H/V counter */
			set_latch_hv(screen().hpos(), screen().vpos());
			return m_openbus_cb(0);       /* Return value is meaningless */

		case ROAMDATA:  /* Read data from OAM (DR) */
		{
			const uint8_t data = read_oam(m_oam.address);
			m_oam.address = (m_oam.address + 1) & 0x3ff;
			oam_set_first_object();
			m_ppu1_open_bus = data;
			return m_ppu1_open_bus;
		}
		case RVMDATAL:  /* Read data from VRAM (low) */
			{
				uint32_t addr = get_vram_address();
				m_ppu1_open_bus = m_vram_read_buffer & 0xff;

				if (!m_vram_fgr_high)
				{
					m_vram_read_buffer = vram_read(addr);
					m_vram_read_buffer |= (vram_read(addr + 1) << 8);

					m_vmadd = (m_vmadd + m_vram_fgr_increment) & 0xffff;
				}

				return m_ppu1_open_bus;
			}
		case RVMDATAH:  /* Read data from VRAM (high) */
			{
				uint32_t addr = get_vram_address();
				m_ppu1_open_bus = (m_vram_read_buffer >> 8) & 0xff;

				if (m_vram_fgr_high)
				{
					m_vram_read_buffer = vram_read(addr);
					m_vram_read_buffer |= (vram_read(addr + 1) << 8);

					m_vmadd = (m_vmadd + m_vram_fgr_increment) & 0xffff;
				}

				return m_ppu1_open_bus;
			}
		case RCGDATA:   /* Read data from CGRAM */
			if (!(m_cgram_address & 0x01))
				m_ppu2_open_bus = cgram_read(m_cgram_address);
			else
			{
				m_ppu2_open_bus &= 0x80;
				m_ppu2_open_bus |= cgram_read(m_cgram_address) & 0x7f;
			}

			m_cgram_address = (m_cgram_address + 1) & (SNES_CGRAM_SIZE - 1);
			return m_ppu2_open_bus;
		case OPHCT:     /* Horizontal counter data by ext/soft latch */
			if (m_read_ophct)
			{
				m_ppu2_open_bus &= 0xfe;
				m_ppu2_open_bus |= (m_beam.latch_horz >> 8) & 0x01;
			}
			else
			{
				m_ppu2_open_bus = m_beam.latch_horz & 0xff;
			}
			m_read_ophct ^= 1;
			return m_ppu2_open_bus;
		case OPVCT:     /* Vertical counter data by ext/soft latch */
			if (m_read_opvct)
			{
				m_ppu2_open_bus &= 0xfe;
				m_ppu2_open_bus |= (m_beam.latch_vert >> 8) & 0x01;
			}
			else
			{
				m_ppu2_open_bus = m_beam.latch_vert & 0xff;
			}
			m_read_opvct ^= 1;
			return m_ppu2_open_bus;
		case STAT77:    /* PPU status flag and version number */
			value = m_stat77 & 0xc0; // 0x80 & 0x40 are Time Over / Range Over Sprite flags, set by the video code
			// 0x20 - Master/slave mode select. Little is known about this bit. We always seem to read back 0 here.
			value |= (m_ppu1_open_bus & 0x10);
			value |= (m_ppu1_version & 0x0f);
			m_stat77 = value;  // not sure if this is needed...
			m_ppu1_open_bus = value;
			return m_ppu1_open_bus;
		case STAT78:    /* PPU status flag and version number */
			m_read_ophct = 0;
			m_read_opvct = 0;
			if (wrio_bit7)
				m_stat78 &= ~0x40; //clear ext latch if bit 7 of WRIO is set
			m_stat78 = (m_stat78 & ~0x2f) | (m_ppu2_open_bus & 0x20) | (m_ppu2_version & 0x0f);
			m_ppu2_open_bus = m_stat78;
			return m_ppu2_open_bus;
	}

	/* note: remaining registers (Namely TM in Super Kick Boxing) returns MDR open bus, not PPU Open Bus! */
	return m_openbus_cb(0);
}


void snes_ppu_device::write(uint32_t offset, uint8_t data)
{
	switch (offset)
	{
		case INIDISP:   /* Initial settings for screen */
			if ((m_screen_disabled & 0x80) && (!(data & 0x80))) //a 1->0 force blank transition causes a reset OAM address
			{
				oam_address_reset();
			}
			m_screen_disabled = data & 0x80;
			m_screen_brightness = data & 0x0f;
			break;
		case OBSEL:     /* Object size and data area designation */
			m_oam.tile_data_address = (data << 13) & 0x6000;
			m_oam.name_select = (data >> 3) & 3;
			m_oam.base_size = (data >> 5) & 7;
			break;
		case OAMADDL:   /* Address for accessing OAM (low) */
			m_oam.base_address = (m_oam.base_address & 0x0200) | (data << 1);
			oam_address_reset();
			break;
		case OAMADDH:   /* Address for accessing OAM (high) */
			m_oam.base_address = ((data & 1) << 9) | (m_oam.base_address & 0x01fe);
			m_oam.priority_rotation = (data >> 7) & 1;
			oam_address_reset();
			break;
		case OAMDATA:   /* Data for OAM write (DW) */
		{
			uint8_t latch_bit = m_oam.address & 1;
			uint16_t address = m_oam.address;
			m_oam.address = (m_oam.address + 1) & 0x03ff;
			if (latch_bit == 0)
			{
				m_oam.data_latch = data;
			}
			if (address & 0x0200)
			{
				write_oam(address, data);
			}
			else if (latch_bit == 1)
			{
				write_oam((address & ~1) + 0, m_oam.data_latch);
				write_oam((address & ~1) + 1, data);
			}
			oam_set_first_object();
			return;
		}
		case BGMODE:    /* BG mode and character size settings */
			m_mode = data & 0x07;
			dynamic_res_change();
			m_bg_priority = BIT(data, 3);
			m_layer[SNES_BG1].tile_size = BIT(data, 4);
			m_layer[SNES_BG2].tile_size = BIT(data, 5);
			m_layer[SNES_BG3].tile_size = BIT(data, 6);
			m_layer[SNES_BG4].tile_size = BIT(data, 7);
			update_video_mode();
			break;
		case MOSAIC:    /* Size and screen designation for mosaic */
			m_mosaic_size = (data & 0xf0) >> 4;
			m_layer[SNES_BG1].mosaic_enabled = BIT(data, 0);
			m_layer[SNES_BG2].mosaic_enabled = BIT(data, 1);
			m_layer[SNES_BG3].mosaic_enabled = BIT(data, 2);
			m_layer[SNES_BG4].mosaic_enabled = BIT(data, 3);
			break;
		case BG1SC:     /* Address for storing SC data BG1 SC size designation */
		case BG2SC:     /* Address for storing SC data BG2 SC size designation  */
		case BG3SC:     /* Address for storing SC data BG3 SC size designation  */
		case BG4SC:     /* Address for storing SC data BG4 SC size designation  */
			m_layer[offset - BG1SC].tilemap = data << 8 & 0x7c00;
			m_layer[offset - BG1SC].tilemap_size = data & 0x3;
			break;
		case BG12NBA:   /* Address for BG 1 and 2 character data */
			m_layer[SNES_BG1].charmap = data << 12 & 0x7000;
			m_layer[SNES_BG2].charmap = data <<  8 & 0x7000;
			break;
		case BG34NBA:   /* Address for BG 3 and 4 character data */
			m_layer[SNES_BG3].charmap = data << 12 & 0x7000;
			m_layer[SNES_BG4].charmap = data <<  8 & 0x7000;
			break;

		// Anomie says "H Current = (Byte<<8) | (Prev&~7) | ((Current>>8)&7); V Current = (Current<<8) | Prev;" and Prev is shared by all scrolls but in Mode 7!
		case BG1HOFS:   /* BG1 - horizontal scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			m_layer[SNES_BG1].hoffs = (data << 8) | (m_ppu_last_scroll & ~7) | ((m_layer[SNES_BG1].hoffs >> 8) & 7);
			m_ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			m_mode7.hor_offset = (data << 8) | (m_mode7_last_scroll & ~7) | ((m_mode7.hor_offset >> 8) & 7);
			m_mode7_last_scroll = data;
			return;
		case BG1VOFS:   /* BG1 - vertical scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			m_layer[SNES_BG1].voffs = (data << 8) | m_ppu_last_scroll;
			m_ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			m_mode7.ver_offset = (data << 8) | m_mode7_last_scroll;
			m_mode7_last_scroll = data;
			return;
		case BG2HOFS:   /* BG2 - horizontal scroll (DW) */
			m_layer[SNES_BG2].hoffs = (data << 8) | (m_ppu_last_scroll & ~7) | ((m_layer[SNES_BG2].hoffs >> 8) & 7);
			m_ppu_last_scroll = data;
			return;
		case BG2VOFS:   /* BG2 - vertical scroll (DW) */
			m_layer[SNES_BG2].voffs = (data << 8) | (m_ppu_last_scroll);
			m_ppu_last_scroll = data;
			return;
		case BG3HOFS:   /* BG3 - horizontal scroll (DW) */
			m_layer[SNES_BG3].hoffs = (data << 8) | (m_ppu_last_scroll & ~7) | ((m_layer[SNES_BG3].hoffs >> 8) & 7);
			m_ppu_last_scroll = data;
			return;
		case BG3VOFS:   /* BG3 - vertical scroll (DW) */
			m_layer[SNES_BG3].voffs = (data << 8) | (m_ppu_last_scroll);
			m_ppu_last_scroll = data;
			return;
		case BG4HOFS:   /* BG4 - horizontal scroll (DW) */
			m_layer[SNES_BG4].hoffs = (data << 8) | (m_ppu_last_scroll & ~7) | ((m_layer[SNES_BG4].hoffs >> 8) & 7);
			m_ppu_last_scroll = data;
			return;
		case BG4VOFS:   /* BG4 - vertical scroll (DW) */
			m_layer[SNES_BG4].voffs = (data << 8) | (m_ppu_last_scroll);
			m_ppu_last_scroll = data;
			return;
		case VMAIN:     /* VRAM address increment value designation */
			m_vram_fgr_high = (data & 0x80);
			m_vram_fgr_increment = vram_fgr_inctab[data & 3];

			if (data & 0xc)
			{
				int md = (data & 0xc) >> 2;

				m_vram_fgr_count = vram_fgr_inccnts[md];         // 0x20, 0x40, 0x80
				m_vram_fgr_mask = (m_vram_fgr_count * 8) - 1; // 0xff, 0x1ff, 0x2ff
				m_vram_fgr_shift = vram_fgr_shiftab[md];         // 5, 6, 7
			}
			else
			{
				m_vram_fgr_count = 0;
			}
//          printf("VMAIN: high %x inc %x count %x mask %x shift %x\n", m_vram_fgr_high, m_vram_fgr_increment, m_vram_fgr_count, m_vram_fgr_mask, m_vram_fgr_shift);
			break;
		case VMADDL:    /* Address for VRAM read/write (low) */
			{
				uint32_t addr;
				m_vmadd = (m_vmadd & 0xff00) | (data << 0);
				addr = get_vram_address();
				m_vram_read_buffer = vram_read(addr);
				m_vram_read_buffer |= (vram_read(addr + 1) << 8);
			}
			break;
		case VMADDH:    /* Address for VRAM read/write (high) */
			{
				uint32_t addr;
				m_vmadd = (m_vmadd & 0x00ff) | (data << 8);
				addr = get_vram_address();
				m_vram_read_buffer = vram_read(addr);
				m_vram_read_buffer |= (vram_read(addr + 1) << 8);
			}
			break;
		case VMDATAL:   /* 2118: Data for VRAM write (low) */
			{
				uint32_t addr = get_vram_address();
				vram_write(addr, data);

				if (!m_vram_fgr_high)
					m_vmadd = (m_vmadd + m_vram_fgr_increment) & 0xffff;
			}
			return;
		case VMDATAH:   /* 2119: Data for VRAM write (high) */
			{
				uint32_t addr = get_vram_address();
				vram_write(addr + 1, data);

				if (m_vram_fgr_high)
					m_vmadd = (m_vmadd + m_vram_fgr_increment) & 0xffff;
			}
			return;
		case M7SEL:     /* Mode 7 initial settings */
			m_mode7.repeat = (data >> 6) & 3;
			m_mode7.vflip  = BIT(data, 1);
			m_mode7.hflip  = BIT(data, 0);
			break;
		/* As per Anomie's doc: Reg = (Current<<8) | Prev; and there is only one Prev, shared by these matrix regs and Mode 7 scroll regs */
		case M7A:       /* Mode 7 COS angle/x expansion (DW) */
			m_mode7.matrix_a = m_mode7_last_scroll + (data << 8);
			m_mode7_last_scroll = data;
			break;
		case M7B:       /* Mode 7 SIN angle/ x expansion (DW) */
			m_mode7.matrix_b = m_mode7_last_scroll + (data << 8);
			m_mode7_last_scroll = data;
			break;
		case M7C:       /* Mode 7 SIN angle/y expansion (DW) */
			m_mode7.matrix_c = m_mode7_last_scroll + (data << 8);
			m_mode7_last_scroll = data;
			break;
		case M7D:       /* Mode 7 COS angle/y expansion (DW) */
			m_mode7.matrix_d = m_mode7_last_scroll + (data << 8);
			m_mode7_last_scroll = data;
			break;
		case M7X:       /* Mode 7 x center position (DW) */
			m_mode7.origin_x = m_mode7_last_scroll + (data << 8);
			m_mode7_last_scroll = data;
			break;
		case M7Y:       /* Mode 7 y center position (DW) */
			m_mode7.origin_y = m_mode7_last_scroll + (data << 8);
			m_mode7_last_scroll = data;
			break;
		case CGADD:     /* Initial address for colour RAM writing */
			/* CGRAM is 16-bit, but when reading/writing we treat it as 8-bit, so we need to double the address */
			m_cgram_address = data << 1;
			break;
		case CGDATA:    /* Data for colour RAM */
			cgram_write(m_cgram_address, data);
			m_cgram_address = (m_cgram_address + 1) & (SNES_CGRAM_SIZE - 1);
			break;
		case W12SEL:    /* Window mask settings for BG1-2 */
			if (data != PPU_REG(W12SEL))
			{
				m_layer[SNES_BG1].window1_invert  = BIT(data, 0);
				m_layer[SNES_BG1].window1_enabled = BIT(data, 1);
				m_layer[SNES_BG1].window2_invert  = BIT(data, 2);
				m_layer[SNES_BG1].window2_enabled = BIT(data, 3);
				m_layer[SNES_BG2].window1_invert  = BIT(data, 4);
				m_layer[SNES_BG2].window1_enabled = BIT(data, 5);
				m_layer[SNES_BG2].window2_invert  = BIT(data, 6);
				m_layer[SNES_BG2].window2_enabled = BIT(data, 7);
			}
			break;
		case W34SEL:    /* Window mask settings for BG3-4 */
			if (data != PPU_REG(W34SEL))
			{
				m_layer[SNES_BG3].window1_invert  = BIT(data, 0);
				m_layer[SNES_BG3].window1_enabled = BIT(data, 1);
				m_layer[SNES_BG3].window2_invert  = BIT(data, 2);
				m_layer[SNES_BG3].window2_enabled = BIT(data, 3);
				m_layer[SNES_BG4].window1_invert  = BIT(data, 4);
				m_layer[SNES_BG4].window1_enabled = BIT(data, 5);
				m_layer[SNES_BG4].window2_invert  = BIT(data, 6);
				m_layer[SNES_BG4].window2_enabled = BIT(data, 7);
			}
			break;
		case WOBJSEL:   /* Window mask settings for objects */
			if (data != PPU_REG(WOBJSEL))
			{
				m_layer[SNES_OAM].window1_invert  = BIT(data, 0);
				m_layer[SNES_OAM].window1_enabled = BIT(data, 1);
				m_layer[SNES_OAM].window2_invert  = BIT(data, 2);
				m_layer[SNES_OAM].window2_enabled = BIT(data, 3);
				m_layer[SNES_COLOR].window1_invert  = BIT(data, 4);
				m_layer[SNES_COLOR].window1_enabled = BIT(data, 5);
				m_layer[SNES_COLOR].window2_invert  = BIT(data, 6);
				m_layer[SNES_COLOR].window2_enabled = BIT(data, 7);
			}
			break;
		case WH0:       /* Window 1 left position */
			if (data != PPU_REG(WH0))
			{
				m_window1_left = data;
			}
			break;
		case WH1:       /* Window 1 right position */
			if (data != PPU_REG(WH1))
			{
				m_window1_right = data;
			}
			break;
		case WH2:       /* Window 2 left position */
			if (data != PPU_REG(WH2))
			{
				m_window2_left = data;
			}
			break;
		case WH3:       /* Window 2 right position */
			if (data != PPU_REG(WH3))
			{
				m_window2_right = data;
			}
			break;
		case WBGLOG:    /* Window mask logic for BG's */
			if (data != PPU_REG(WBGLOG))
			{
				m_layer[SNES_BG1].wlog_mask = data & 0x03;
				m_layer[SNES_BG2].wlog_mask = (data & 0x0c) >> 2;
				m_layer[SNES_BG3].wlog_mask = (data & 0x30) >> 4;
				m_layer[SNES_BG4].wlog_mask = (data & 0xc0) >> 6;
			}
			break;
		case WOBJLOG:   /* Window mask logic for objects */
			if (data != PPU_REG(WOBJLOG))
			{
				m_layer[SNES_OAM].wlog_mask = data & 0x03;
				m_layer[SNES_COLOR].wlog_mask = (data & 0x0c) >> 2;
			}
			break;
		case TM:        /* Main screen designation */
			m_layer[SNES_BG1].main_bg_enabled = BIT(data, 0);
			m_layer[SNES_BG2].main_bg_enabled = BIT(data, 1);
			m_layer[SNES_BG3].main_bg_enabled = BIT(data, 2);
			m_layer[SNES_BG4].main_bg_enabled = BIT(data, 3);
			m_layer[SNES_OAM].main_bg_enabled = BIT(data, 4);
			break;
		case TS:        /* Subscreen designation */
			m_layer[SNES_BG1].sub_bg_enabled = BIT(data, 0);
			m_layer[SNES_BG2].sub_bg_enabled = BIT(data, 1);
			m_layer[SNES_BG3].sub_bg_enabled = BIT(data, 2);
			m_layer[SNES_BG4].sub_bg_enabled = BIT(data, 3);
			m_layer[SNES_OAM].sub_bg_enabled = BIT(data, 4);
			break;
		case TMW:       /* Window mask for main screen designation */
			m_layer[SNES_BG1].main_window_enabled = BIT(data, 0);
			m_layer[SNES_BG2].main_window_enabled = BIT(data, 1);
			m_layer[SNES_BG3].main_window_enabled = BIT(data, 2);
			m_layer[SNES_BG4].main_window_enabled = BIT(data, 3);
			m_layer[SNES_OAM].main_window_enabled = BIT(data, 4);
			break;
		case TSW:       /* Window mask for subscreen designation */
			m_layer[SNES_BG1].sub_window_enabled = BIT(data, 0);
			m_layer[SNES_BG2].sub_window_enabled = BIT(data, 1);
			m_layer[SNES_BG3].sub_window_enabled = BIT(data, 2);
			m_layer[SNES_BG4].sub_window_enabled = BIT(data, 3);
			m_layer[SNES_OAM].sub_window_enabled = BIT(data, 4);
			break;
		case CGWSEL:    /* Initial settings for Fixed colour addition or screen addition */
			m_clip_to_black = (data >> 6) & 0x03;
			m_prevent_color_math = (data >> 4) & 0x03;
			m_sub_add_mode = BIT(data, 1);
			m_direct_color = BIT(data, 0);
#ifdef SNES_DBG_REG_W
			if ((data & 0x2) != (PPU_REG(CGWSEL) & 0x2))
				osd_printf_debug("Add/Sub Layer: %s\n", ((data & 0x2) >> 1) ? "Subscreen" : "Fixed colour");
#endif
			break;
		case CGADSUB:   /* Addition/Subtraction designation for each screen */
			m_color_modes = (data >> 6) & 0x03;
			m_layer[SNES_BG1].color_math = BIT(data, 0);
			m_layer[SNES_BG2].color_math = BIT(data, 1);
			m_layer[SNES_BG3].color_math = BIT(data, 2);
			m_layer[SNES_BG4].color_math = BIT(data, 3);
			m_layer[SNES_OAM].color_math = BIT(data, 4);
			m_layer[SNES_COLOR].color_math = BIT(data, 5);
			break;
		case COLDATA:   /* Fixed colour data for fixed colour addition/subtraction */
			{
				/* Store it in the extra space we made in the palette entry. */
				uint8_t r, g, b;

				/* Get existing value. */
				r = pen_indirect(FIXED_COLOUR) & 0x1f;
				g = (pen_indirect(FIXED_COLOUR) & 0x3e0) >> 5;
				b = (pen_indirect(FIXED_COLOUR) & 0x7c00) >> 10;
				/* Set new value */
				if (data & 0x20)
					r = data & 0x1f;
				if (data & 0x40)
					g = data & 0x1f;
				if (data & 0x80)
					b = data & 0x1f;
				set_pen_indirect(FIXED_COLOUR, r | (g << 5) | (b << 10));
			} break;
		case SETINI:    /* Screen mode/video select */
			m_interlace = (data & 0x01) ? 2 : 1;
			m_oam.interlace = BIT(data, 1);
			// TODO: this should actually be always 240, then fill black remaining lines if bit is 0.
			//m_beam.last_visible_line = (m_stat78 & SNES_PAL) ? 240 : (data & 0x04) ? 240 : 225;
			m_beam.last_visible_line = (data & 0x04) ? 240 : 225;
			m_pseudo_hires = BIT(data, 3);
			m_mode7.extbg = BIT(data, 6);
			dynamic_res_change();
#ifdef SNES_DBG_REG_W
			if ((data & 0x8) != (PPU_REG(SETINI) & 0x8))
				osd_printf_debug("Pseudo 512 mode: %s\n", (data & 0x8) ? "on" : "off");
#endif
			update_video_mode();
			break;
		}

	PPU_REG(offset) = data;
}

/***** Debug Functions *****/

#if SNES_LAYER_DEBUG

#define DEBUG_TOGGLE(bit, debug_settings, MSG1, MSG2) \
	if (BIT(toggles, bit) && !debug_settings)       \
	{                                               \
		debug_settings = 1;                       \
		popmessage MSG1;                          \
	}                                               \
	else if (!BIT(toggles, bit) && debug_settings)  \
	{                                               \
		debug_settings = 0;                       \
		popmessage MSG2;                          \
	}

uint8_t snes_ppu_device::dbg_video( uint16_t curline )
{
	int i;
	uint8_t toggles = m_debug1.read_safe(0);
	m_debug_options.select_pri[SNES_BG1] = (toggles & 0x03);
	m_debug_options.select_pri[SNES_BG2] = (toggles & 0x0c) >> 2;
	m_debug_options.select_pri[SNES_BG3] = (toggles & 0x30) >> 4;
	m_debug_options.select_pri[SNES_BG4] = (toggles & 0xc0) >> 6;

	toggles = m_debug2.read_safe(0);
	for (i = 0; i < 4; i++)
		DEBUG_TOGGLE(i, m_debug_options.bg_disabled[i], ("Debug: Disabled BG%d.\n", i + 1), ("Debug: Enabled BG%d.\n", i + 1))
	DEBUG_TOGGLE(4, m_debug_options.bg_disabled[SNES_OAM], ("Debug: Disabled OAM.\n"), ("Debug: Enabled OAM.\n"))
	DEBUG_TOGGLE(5, m_debug_options.draw_subscreen, ("Debug: Switched screens.\n"), ("Debug: Switched screens.\n"))
	DEBUG_TOGGLE(6, m_debug_options.colormath_disabled, ("Debug: Disabled Color Math.\n"), ("Debug: Enabled Color Math.\n"))
	DEBUG_TOGGLE(7, m_debug_options.windows_disabled, ("Debug: Disabled Window Masks.\n"), ("Debug: Enabled Window Masks.\n"))

	toggles = m_debug4.read_safe(0);
	for (i = 0; i < 8; i++)
		DEBUG_TOGGLE(i, m_debug_options.mode_disabled[i], ("Debug: Disabled Mode %d drawing.\n", i), ("Debug: Enabled Mode %d drawing.\n", i))

	toggles = m_debug3.read_safe(0);
	DEBUG_TOGGLE(2, m_debug_options.mosaic_disabled, ("Debug: Disabled Mosaic.\n"), ("Debug: Enabled Mosaic.\n"))
	m_debug_options.select_pri[SNES_OAM] = (toggles & 0x70) >> 4;

#ifdef MAME_DEBUG
	/* Once per frame, log video properties */
	if (curline == 1)
	{
		static const char WINLOGIC[4] = { '|', '&', '^', '!' };

		logerror("%s", m_debug_options.windows_disabled?" ":"W");
		logerror("%s1 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
				m_debug_options.bg_disabled[0]?" ":"*",
				(PPU_REG(TM) & 0x1)?"M":" ",
				(PPU_REG(TS) & 0x1)?"S":" ",
				(PPU_REG(CGADSUB) & 0x1)?"B":" ",
				(PPU_REG(TMW) & 0x1)?"m":" ",
				(PPU_REG(TSW) & 0x1)?"s":" ",
				WINLOGIC[(PPU_REG(WBGLOG) & 0x3)],
				(PPU_REG(W12SEL) & 0x2)?((PPU_REG(W12SEL) & 0x1)?"o":"i"):" ",
				(PPU_REG(W12SEL) & 0x8)?((PPU_REG(W12SEL) & 0x4)?"o":"i"):" ",
				m_layer[SNES_BG1].tile_size + 1,
				(PPU_REG(MOSAIC) & 0x1)?"m":" ",
				PPU_REG(BG1SC) & 0x3,
				(PPU_REG(BG1SC) & 0xfc) << 9,
				m_layer[SNES_BG1].charmap << 13);
		logerror("%s2 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
				m_debug_options.bg_disabled[1]?" ":"*",
				(PPU_REG(TM) & 0x2)?"M":" ",
				(PPU_REG(TS) & 0x2)?"S":" ",
				(PPU_REG(CGADSUB) & 0x2)?"B":" ",
				(PPU_REG(TMW) & 0x2)?"m":" ",
				(PPU_REG(TSW) & 0x2)?"s":" ",
				WINLOGIC[(PPU_REG(WBGLOG) & 0xc) >> 2],
				(PPU_REG(W12SEL) & 0x20)?((PPU_REG(W12SEL) & 0x10)?"o":"i"):" ",
				(PPU_REG(W12SEL) & 0x80)?((PPU_REG(W12SEL) & 0x40)?"o":"i"):" ",
				m_layer[SNES_BG2].tile_size + 1,
				(PPU_REG(MOSAIC) & 0x2)?"m":" ",
				PPU_REG(BG2SC) & 0x3,
				(PPU_REG(BG2SC) & 0xfc) << 9,
				m_layer[SNES_BG2].charmap << 13);
		logerror("%s3 %s%s%s%s%s%c%s%s%d%s%s%d %4X %4X",
				m_debug_options.bg_disabled[2]?" ":"*",
				(PPU_REG(TM) & 0x4)?"M":" ",
				(PPU_REG(TS) & 0x4)?"S":" ",
				(PPU_REG(CGADSUB) & 0x4)?"B":" ",
				(PPU_REG(TMW) & 0x4)?"m":" ",
				(PPU_REG(TSW) & 0x4)?"s":" ",
				WINLOGIC[(PPU_REG(WBGLOG) & 0x30)>>4],
				(PPU_REG(W34SEL) & 0x2)?((PPU_REG(W34SEL) & 0x1)?"o":"i"):" ",
				(PPU_REG(W34SEL) & 0x8)?((PPU_REG(W34SEL) & 0x4)?"o":"i"):" ",
				m_layer[SNES_BG3].tile_size + 1,
				(PPU_REG(MOSAIC) & 0x4)?"m":" ",
				(PPU_REG(BGMODE) & 0x8)?"P":" ",
				PPU_REG(BG3SC) & 0x3,
				(PPU_REG(BG3SC) & 0xfc) << 9,
				m_layer[SNES_BG3].charmap << 13);
		logerror("%s4 %s%s%s%s%s%c%s%s%d%s %d %4X %4X",
				m_debug_options.bg_disabled[3]?" ":"*",
				(PPU_REG(TM) & 0x8)?"M":" ",
				(PPU_REG(TS) & 0x8)?"S":" ",
				(PPU_REG(CGADSUB) & 0x8)?"B":" ",
				(PPU_REG(TMW) & 0x8)?"m":" ",
				(PPU_REG(TSW) & 0x8)?"s":" ",
				WINLOGIC[(PPU_REG(WBGLOG) & 0xc0)>>6],
				(PPU_REG(W34SEL) & 0x20)?((PPU_REG(W34SEL) & 0x10)?"o":"i"):" ",
				(PPU_REG(W34SEL) & 0x80)?((PPU_REG(W34SEL) & 0x40)?"o":"i"):" ",
				m_layer[SNES_BG4].tile_size + 1,
				(PPU_REG(MOSAIC) & 0x8)?"m":" ",
				PPU_REG(BG4SC) & 0x3,
				(PPU_REG(BG4SC) & 0xfc) << 9,
				m_layer[SNES_BG4].charmap << 13 );
		logerror("%sO %s%s%s%s%s%c%s%s       %4X",
				m_debug_options.bg_disabled[4]?" ":"*",
				(PPU_REG(TM) & 0x10)?"M":" ",
				(PPU_REG(TS) & 0x10)?"S":" ",
				(PPU_REG(CGADSUB) & 0x10)?"B":" ",
				(PPU_REG(TMW) & 0x10)?"m":" ",
				(PPU_REG(TSW) & 0x10)?"s":" ",
				WINLOGIC[(PPU_REG(WOBJLOG) & 0x3)],
				(PPU_REG(WOBJSEL) & 0x2)?((PPU_REG(WOBJSEL) & 0x1)?"o":"i"):" ",
				(PPU_REG(WOBJSEL) & 0x8)?((PPU_REG(WOBJSEL) & 0x4)?"o":"i"):" ",
				m_layer[SNES_OAM].charmap << 13 );
		logerror("%sB   %s  %c%s%s",
				m_debug_options.colormath_disabled?" ":"*",
				(PPU_REG(CGADSUB) & 0x20)?"B":" ",
				WINLOGIC[(PPU_REG(WOBJLOG) & 0xc)>>2],
				(PPU_REG(WOBJSEL) & 0x20)?((PPU_REG(WOBJSEL) & 0x10)?"o":"i"):" ",
				(PPU_REG(WOBJSEL) & 0x80)?((PPU_REG(WOBJSEL) & 0x40)?"o":"i"):" " );
		logerror("Flags: %s%s%s %s %2d", (PPU_REG(CGWSEL) & 0x2)?"S":"F", (PPU_REG(CGADSUB) & 0x80)?"-":"+", (PPU_REG(CGADSUB) & 0x40)?" 50%":"100%",(PPU_REG(CGWSEL) & 0x1)?"D":"P", (PPU_REG(MOSAIC) & 0xf0) >> 4 );
		logerror("SetINI: %s %s %s %s %s %s", (PPU_REG(SETINI) & 0x1)?" I":"NI", (PPU_REG(SETINI) & 0x2)?"P":"R", (PPU_REG(SETINI) & 0x4)?"240":"225",(PPU_REG(SETINI) & 0x8)?"512":"256",(PPU_REG(SETINI) & 0x40)?"E":"N",(PPU_REG(SETINI) & 0x80)?"ES":"NS" );
		logerror("Mode7: A %5d B %5d", m_mode7.matrix_a, m_mode7.matrix_b );
		logerror(" %s%s%s   C %5d D %5d", (PPU_REG(M7SEL) & 0xc0)?((PPU_REG(M7SEL) & 0x40)?"0":"C"):"R", (PPU_REG(M7SEL) & 0x1)?"H":" ", (PPU_REG(M7SEL) & 0x2)?"V":" ", m_mode7.matrix_c, m_mode7.matrix_d );
		logerror("       X %5d Y %5d", m_mode7.origin_x, m_mode7.origin_y );
	}
#endif

	return 0;
}
#endif /* SNES_LAYER_DEBUG */

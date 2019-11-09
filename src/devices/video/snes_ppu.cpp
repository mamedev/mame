// license:GPL-2.0+
// copyright-holders:byuu, Anthony Kruize, Fabio Priuli
/***************************************************************************

  snes.c

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
#include "video/snes_ppu.h"

#define SNES_MAINSCREEN    0
#define SNES_SUBSCREEN     1
#define SNES_CLIP_NEVER    0
#define SNES_CLIP_IN       1
#define SNES_CLIP_OUT      2
#define SNES_CLIP_ALWAYS   3

#define SNES_VRAM_SIZE        0x10000   /* 64kb of video ram */
#define SNES_CGRAM_SIZE       0x202     /* 256 16-bit colours + 1 tacked on 16-bit colour for fixed colour */
#define SNES_OAM_SIZE         0x440     /* 1088 bytes of Object Attribute Memory */
#define FIXED_COLOUR          256       /* Position in cgram for fixed colour */


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
	, m_openbus_cb(*this)
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
	m_openbus_cb.resolve_safe(0);

	m_vram = std::make_unique<uint8_t[]>(SNES_VRAM_SIZE);
	m_cgram = std::make_unique<uint16_t[]>(SNES_CGRAM_SIZE/2);
	m_oam_ram = std::make_unique<uint16_t[]>(SNES_OAM_SIZE/2);

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
					m_light_table[l][r << 10 | g << 5 | b << 0] = ar << 10 | ag << 5 | ab << 0;
				}
			}
		}
	}

	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_scanlines[i].enable), i);
		save_item(NAME(m_scanlines[i].clip), i);
		save_item(NAME(m_scanlines[i].buffer), i);
		save_item(NAME(m_scanlines[i].priority), i);
		save_item(NAME(m_scanlines[i].layer), i);
		save_item(NAME(m_scanlines[i].blend_exception), i);
	}

	for (int i = 0; i < 6; i++)
	{
		save_item(NAME(m_layer[i].window1_enabled), i);
		save_item(NAME(m_layer[i].window1_invert), i);
		save_item(NAME(m_layer[i].window2_enabled), i);
		save_item(NAME(m_layer[i].window2_invert), i);
		save_item(NAME(m_layer[i].wlog_mask), i);
		save_item(NAME(m_layer[i].color_math), i);
		save_item(NAME(m_layer[i].charmap), i);
		save_item(NAME(m_layer[i].tilemap), i);
		save_item(NAME(m_layer[i].tilemap_size), i);
		save_item(NAME(m_layer[i].tile_size), i);
		save_item(NAME(m_layer[i].tile_mode), i);
		save_item(NAME(m_layer[i].mosaic_enabled), i);
		save_item(NAME(m_layer[i].main_window_enabled), i);
		save_item(NAME(m_layer[i].sub_window_enabled), i);
		save_item(NAME(m_layer[i].main_bg_enabled), i);
		save_item(NAME(m_layer[i].sub_bg_enabled), i);
		save_item(NAME(m_layer[i].hoffs), i);
		save_item(NAME(m_layer[i].voffs), i);
		save_item(NAME(m_layer[i].priority[0]), i);
		save_item(NAME(m_layer[i].priority[1]), i);
		save_item(NAME(m_layer[i].mosaic_counter), i);
		save_item(NAME(m_layer[i].mosaic_offset), i);

		save_item(NAME(m_clipmasks[i]), i);
	}

	save_item(NAME(m_oam.address_low));
	save_item(NAME(m_oam.address_high));
	save_item(NAME(m_oam.saved_address_low));
	save_item(NAME(m_oam.saved_address_high));
	save_item(NAME(m_oam.address));
	save_item(NAME(m_oam.priority_rotation));
	save_item(NAME(m_oam.next_charmap));
	save_item(NAME(m_oam.next_size));
	save_item(NAME(m_oam.size));
	save_item(NAME(m_oam.next_name_select));
	save_item(NAME(m_oam.name_select));
	save_item(NAME(m_oam.first_sprite));
	save_item(NAME(m_oam.flip));
	save_item(NAME(m_oam.write_latch));

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

	for (int i = 0; i < ARRAY_LENGTH(m_oam_spritelist); i++)
	{
		save_item(NAME(m_oam_spritelist[i].tile), i);
		save_item(NAME(m_oam_spritelist[i].x), i);
		save_item(NAME(m_oam_spritelist[i].y), i);
		save_item(NAME(m_oam_spritelist[i].size), i);
		save_item(NAME(m_oam_spritelist[i].vflip), i);
		save_item(NAME(m_oam_spritelist[i].hflip), i);
		save_item(NAME(m_oam_spritelist[i].priority_bits), i);
		save_item(NAME(m_oam_spritelist[i].pal), i);
		save_item(NAME(m_oam_spritelist[i].height), i);
		save_item(NAME(m_oam_spritelist[i].width), i);
	}

	for (int i = 0; i < ARRAY_LENGTH(m_oam_tilelist); i++)
	{
		save_item(NAME(m_oam_tilelist[i].x), i);
		save_item(NAME(m_oam_tilelist[i].priority), i);
		save_item(NAME(m_oam_tilelist[i].pal), i);
		save_item(NAME(m_oam_tilelist[i].tileaddr), i);
		save_item(NAME(m_oam_tilelist[i].hflip), i);
	}

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

	save_item(NAME(m_update_windows));
	save_item(NAME(m_update_offsets));
	save_item(NAME(m_update_oam_list));
	save_item(NAME(m_mode));
	save_item(NAME(m_interlace));
	save_item(NAME(m_obj_interlace));
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
	save_pointer(NAME(m_oam_ram), SNES_OAM_SIZE/2);
}

void snes_ppu_device::device_reset()
{
#if SNES_LAYER_DEBUG
	memset(&m_debug_options, 0, sizeof(m_debug_options));
#endif

	/* Inititialize registers/variables */
	m_update_windows = 1;
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

	/* Init oam RAM */
	memset((uint8_t *)m_oam_ram.get(), 0xff, SNES_OAM_SIZE);

	// other initializations to 0
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_oam_itemlist, 0, sizeof(m_oam_itemlist));
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

	for (auto & elem : m_oam_spritelist)
	{
		elem.tile = 0;
		elem.x = 0;
		elem.y = 0;
		elem.size = 0;
		elem.vflip = 0;
		elem.hflip = 0;
		elem.priority_bits = 0;
		elem.pal = 0;
		elem.height = 0;
		elem.width = 0;
	}

	for (auto & elem : m_oam_tilelist)
	{
		elem.x = 0;
		elem.priority = 0;
		elem.pal = 0;
		elem.tileaddr = 0;
		elem.hflip = 0;
	}
}

/*****************************************
 * get_bgcolor()
 *
 * Get the proper color (direct or from cgram)
 *****************************************/

inline uint16_t snes_ppu_device::get_bgcolor( uint8_t direct_colors, uint16_t palette, uint8_t color )
{
	uint16_t c;

	if (direct_colors)
	{
		/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
		c = ((color & 0x07) << 2) | ((color & 0x38) << 4) | ((color & 0xc0) << 7);
		c |= ((palette & 0x04) >> 1) | ((palette & 0x08) << 3) | ((palette & 0x10) << 8);
	}
	else
		c = m_cgram[(palette + color) % FIXED_COLOUR];

	return c;
}

/*****************************************
 * set_scanline_pixel()
 *
 * Store pixel color, priority, layer and
 * color math exception (for OAM) in the
 * proper scanline
 *****************************************/

inline void snes_ppu_device::set_scanline_pixel( int screen, int16_t x, uint16_t color, uint8_t priority, uint8_t layer, int blend )
{
	m_scanlines[screen].buffer[x] = color;
	m_scanlines[screen].priority[x] = priority;
	m_scanlines[screen].layer[x] = layer;
	m_scanlines[screen].blend_exception[x] = blend;
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

/*****************************************
 * draw_oamtile()
 *
 * Draw 8 pixels from the expected OAM
 * tile into priority/palette buffers
 * by reading the color planes from vram
 * and by calling draw_oamtile
 *****************************************/

inline void snes_ppu_device::draw_oamtile( uint32_t tileaddr, int16_t tile_x, uint8_t priority, uint8_t flip, uint16_t pal, uint8_t *palbuf, uint8_t *pribuf )
{
	uint8_t plane[4];

	for (int16_t ii = 0; ii < 2; ii++)
	{
		plane[2 * ii + 0] = m_vram[((tileaddr << 1) + 16 * ii + 0) % SNES_VRAM_SIZE];
		plane[2 * ii + 1] = m_vram[((tileaddr << 1) + 16 * ii + 1) % SNES_VRAM_SIZE];
	}

	for (int16_t x = 0; x < 8; x++)
	{
		tile_x &= 0x01ff;
		if (tile_x < 256)
		{
			uint8_t colour = 0;

			if (flip)
			{
				for (int16_t jj = 0; jj < 4; jj++)
					colour |= BIT(plane[jj], x) ? (1 << jj) : 0;
			}
			else
			{
				for (int16_t jj = 0; jj < 4; jj++)
					colour |= BIT(plane[jj], 7 - x) ? (1 << jj) : 0;
			}

			if (colour)
			{
				palbuf[tile_x] = pal + colour;
				pribuf[tile_x] = priority;
			}
		}
		tile_x++;
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
					mosaic_color = direct_color(palette_number, mosaic_palette);
				}
				else
				{
					mosaic_color = m_cgram[(palette_index + mosaic_palette) % FIXED_COLOUR];
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
	int hcenter = (m_mode7.origin_x << 19) >> 19;
	int vcenter = (m_mode7.origin_y << 19) >> 19;
	int hoffset = (m_mode7.hor_offset << 19) >> 19;
	int voffset = (m_mode7.ver_offset << 19) >> 19;

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
				mosaic_color = direct_color(0, palette);
			}
			else
			{
				mosaic_color = m_cgram[palette];
			}
		}
		if (!mosaic_palette) continue;

		if (self.main_bg_enabled && window_above[_x] == 0) plot_above(_x, layer_idx, mosaic_priority, mosaic_color);
		if (self.sub_bg_enabled  && window_below[_x] == 0) plot_below(_x, layer_idx, mosaic_priority, mosaic_color);
	}
}

/*************************************************************************************************
 * SNES Sprites
 *
 * 1. First of all: sprites are drawn one line in advance. We emulate this by caching the
 * starting vram address, the sprite size and the "name select" at each line, and by using
 * them the next line to output the proper sprites - see update_obsel.
 *
 * 2. Each line can select its sprites among 128 available ones in oam_ram, hence we start
 * by creating a list of the available objects (each one with its x,y coordinate, its size,
 * its tile address, etc.) - see oam_list_build.
 *
 * 3. Next, we start finding out which sprites will appear in the line: starting from
 * FirstSprite, we count 32 OBJs which intersect our line and we store their indexes in the
 * oam_itemlist array (if more than 32 sprites intersect our line, we set the Range Over
 * flag); then, selecting among these sprites, we count 34 8x8 tiles which are visible
 * in our line (i.e. whose x coord is between -size and 256) and we store the corresponding
 * coordinates/priorities/palettes/etc. in the oam_tilelist array (if more than 34 tiles would
 * appear on screen, we set the Time Over flag).
 * Notice that when we populate oam_tilelist, we proceed from oam_itemlist[31] (or from the last
 * item which intersects the scanline), towards oam_itemlist[0], i.e. the higher tiles (say
 * oam_tilelist[34], or the last tile which appear on screen) will contain FirstSprite object,
 * or the sprites with closer index to FirstSprite which get displayed. This will play an
 * important role for sprite priority - see update_objects_rto.
 *
 * 4. All the above happens at the beginning of each VIDEO_UPDATE. When we finally draw the
 * scanline, we pass through the oam_tilelist and we store the displayed pixels in our scanline
 * buffer. Notice that, for each pixel of a SNES sprite, only the priority of the topmost sprite
 * is tested against the priority of the BG pixel (because FirstSprite is on top of FirstSprite+1,
 * which is on top of FirstSprite+2, etc., and therefore other sprites are already covered by the
 * topmost one). To emulate this, we draw each tile over the previous ones no matter what
 * priorities are (differently from what we did with BGs): in the end, we will have in each pixel z
 * its topmost sprite and scanline.priority[z] will be the topmost sprite priority as expected.
 * Of course, sprite drawing must happen before BG drawing, so that afterwords BG pixels properly
 * test their priority with the one of the correct sprite - see update_objects.
 *************************************************************************************************/


/*********************************************
 * update_obsel()
 *
 * Update sprite settings for next line.
 *********************************************/

void snes_ppu_device::update_obsel( void )
{
	m_layer[SNES_OAM].charmap = m_oam.next_charmap;
	m_oam.name_select = m_oam.next_name_select;

	if (m_oam.size != m_oam.next_size)
	{
		m_oam.size = m_oam.next_size;
		m_update_oam_list = 1;
	}
}

/*********************************************
 * oam_list_build()
 *
 * Build a list of the available obj in OAM ram.
 *********************************************/

void snes_ppu_device::oam_list_build( void )
{
	uint8_t *oamram = (uint8_t *)m_oam_ram.get();
	int16_t oam = 0x1ff;
	uint16_t oam_extra = oam + 0x20;
	uint16_t extra = 0;
	int ii;

	m_update_oam_list = 0;       // eventually, we can optimize the code by only calling this function when there is a change in size

	for (ii = 127; ii >= 0; ii--)
	{
		if (((ii + 1) % 4) == 0)
			extra = oamram[oam_extra--];

		m_oam_spritelist[ii].vflip = (oamram[oam] & 0x80) >> 7;
		m_oam_spritelist[ii].hflip = (oamram[oam] & 0x40) >> 6;
		m_oam_spritelist[ii].priority_bits = (oamram[oam] & 0x30) >> 4;
		m_oam_spritelist[ii].pal = 128 + ((oamram[oam] & 0x0e) << 3);
		m_oam_spritelist[ii].tile = (oamram[oam--] & 0x1) << 8;
		m_oam_spritelist[ii].tile |= oamram[oam--];
		m_oam_spritelist[ii].y = oamram[oam--] + 1;
		m_oam_spritelist[ii].x = oamram[oam--];
		m_oam_spritelist[ii].size = (extra & 0x80) >> 7;
		extra <<= 1;
		m_oam_spritelist[ii].x |= ((extra & 0x80) << 1);
		extra <<= 1;

		m_oam_spritelist[ii].y *= m_obj_interlace;
		m_oam_spritelist[ii].y &= 0x1ff;

		m_oam_spritelist[ii].x &= 0x1ff;

		/* Determine object size */
		switch (m_oam.size)
		{
		case 0:         /* 8x8 or 16x16 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 2 : 1;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 2 : 1;
			break;
		case 1:         /* 8x8 or 32x32 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 4 : 1;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 4 : 1;
			break;
		case 2:         /* 8x8 or 64x64 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 8 : 1;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 8 : 1;
			break;
		case 3:         /* 16x16 or 32x32 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 4 : 2;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 4 : 2;
			break;
		case 4:         /* 16x16 or 64x64 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 8 : 2;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 8 : 2;
			break;
		case 5:         /* 32x32 or 64x64 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 8 : 4;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 8 : 4;
			break;
		case 6:         /* undocumented: 16x32 or 32x64 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 4 : 2;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 8 : 4;
			if (m_obj_interlace && !m_oam_spritelist[ii].size)
				m_oam_spritelist[ii].height = 2;
			break;
		case 7:         /* undocumented: 16x32 or 32x32 */
			m_oam_spritelist[ii].width  = m_oam_spritelist[ii].size ? 4 : 2;
			m_oam_spritelist[ii].height = m_oam_spritelist[ii].size ? 4 : 4;
			if (m_obj_interlace && !m_oam_spritelist[ii].size)
				m_oam_spritelist[ii].height = 2;
			break;
		default:
			/* we should never enter here... */
			logerror("Object size unsupported: %d\n", m_oam.size);
			break;
		}
	}
}

/*********************************************
 * is_sprite_on_scanline()
 *
 * Check if a given sprite intersects current
 * scanline
 *********************************************/

int snes_ppu_device::is_sprite_on_scanline( uint16_t curline, uint8_t sprite )
{
	//if sprite is entirely offscreen and doesn't wrap around to the left side of the screen,
	//then it is not counted. this *should* be 256, and not 255, even though dot 256 is offscreen.
	int spr_height = (m_oam_spritelist[sprite].height << 3);

	if (m_oam_spritelist[sprite].x > 256 && (m_oam_spritelist[sprite].x + (m_oam_spritelist[sprite].width << 3) - 1) < 512)
		return 0;

	if (curline >= m_oam_spritelist[sprite].y && curline < (m_oam_spritelist[sprite].y + spr_height))
		return 1;

	if ((m_oam_spritelist[sprite].y + spr_height) >= 256 && curline < ((m_oam_spritelist[sprite].y + spr_height) & 255))
		return 1;

	return 0;
}

/*********************************************
 * update_objects_rto()
 *
 * Determine which OBJs will be drawn on this
 * scanline.
 *********************************************/

void snes_ppu_device::update_objects_rto( uint16_t curline )
{
	int ii, jj, active_sprite;
	uint8_t range_over, time_over;
	int8_t xs, ys;
	uint8_t line;
	uint8_t height, width, vflip, hflip, priority, pal;
	uint16_t tile;
	int16_t x, y;

	oam_list_build();

	/* initialize counters */
	range_over = 0;
	time_over = 0;

	/* setup the proper line */
	curline /= m_interlace;
	curline *= m_obj_interlace;

	/* reset the list of first 32 objects which intersect current scanline */
	memset(m_oam_itemlist, 0xff, 32);

	/* populate the list of 32 objects */
	for (ii = 0; ii < 128; ii++)
	{
		active_sprite = (ii + m_oam.first_sprite) & 0x7f;

		if (!is_sprite_on_scanline(curline, active_sprite))
			continue;

		if (range_over++ >= 32)
			break;

		m_oam_itemlist[range_over - 1] = active_sprite;
	}

	/* reset the list of first 34 tiles to be drawn */
	for (ii = 0; ii < 34; ii++)
		m_oam_tilelist[ii].tileaddr = 0xffff;

	/* populate the list of 34 tiles */
	for (ii = 31; ii >= 0; ii--)
	{
		if (m_oam_itemlist[ii] == 0xff)
			continue;

		active_sprite = m_oam_itemlist[ii];

		tile = m_oam_spritelist[active_sprite].tile;
		x = m_oam_spritelist[active_sprite].x;
		y = m_oam_spritelist[active_sprite].y;
		height = m_oam_spritelist[active_sprite].height;
		width = m_oam_spritelist[active_sprite].width;
		vflip = m_oam_spritelist[active_sprite].vflip;
		hflip = m_oam_spritelist[active_sprite].hflip;
		priority = m_oam_spritelist[active_sprite].priority_bits;
		pal = m_oam_spritelist[active_sprite].pal;

		/* Adjust y, if past maximum position (for sprites which overlap between top & bottom) */
		if (y >= (0x100 - 16) * m_interlace)
			y -= (0x100) * m_interlace;

		if (curline >= y && curline < (y + (height << 3)))
		{
			/* Only objects using tiles over 255 use name select */
			uint32_t name_sel = (tile < 256) ? 0 : ((1 + m_oam.name_select) << 12);

			ys = (curline - y) >> 3;
			line = (curline - y) % 8;
			if (vflip)
			{
				ys = height - ys - 1;
				line = 7 - line;
			}

			uint16_t character_x = tile & 15;
			uint16_t character_y = (((tile >> 4) + ys) & 15) << 4;

			for (jj = 0; jj < width; jj++)
			{
				int16_t xx = (x + (jj << 3)) & 0x1ff;

				if (x != 256 && xx >= 256 && (xx + 7) < 512)
					continue;

				if (time_over++ >= 34)
					break;

				xs = (hflip) ? (width - 1 - jj) : jj;
				m_oam_tilelist[time_over - 1].tileaddr = name_sel + ((character_y + ((character_x + xs) & 15)) << 4);
				m_oam_tilelist[time_over - 1].tileaddr &= 0x7ff0;
				m_oam_tilelist[time_over - 1].tileaddr += line;
				m_oam_tilelist[time_over - 1].hflip = hflip;
				m_oam_tilelist[time_over - 1].x = xx;
				m_oam_tilelist[time_over - 1].pal = pal;
				m_oam_tilelist[time_over - 1].priority = priority;
			}
		}
	}

	/* set Range Over flag if necessary */
	if (range_over > 32)
		m_stat77 |= 0x40;

	/* set Time Over flag if necessary */
	if (time_over > 34)
		m_stat77 |= 0x80;
}

/*********************************************
 * update_objects()
 *
 * Update an entire line of sprites.
 *********************************************/

void snes_ppu_device::update_objects( uint8_t priority_oam0, uint8_t priority_oam1, uint8_t priority_oam2, uint8_t priority_oam3 )
{
	uint8_t pri, priority[4];
	uint32_t charaddr;
	int ii;

#if SNES_LAYER_DEBUG
	if (m_debug_options.bg_disabled[SNES_OAM])
		return;
#endif /* SNES_LAYER_DEBUG */

	m_scanlines[SNES_MAINSCREEN].enable = m_layer[SNES_OAM].main_bg_enabled;
	m_scanlines[SNES_SUBSCREEN].enable = m_layer[SNES_OAM].sub_bg_enabled;

	if (!m_scanlines[SNES_MAINSCREEN].enable && !m_scanlines[SNES_SUBSCREEN].enable)
		return;

	uint8_t window_above[256];
	uint8_t window_below[256];
	render_window(SNES_OAM, m_layer[SNES_OAM].main_window_enabled, window_above);
	render_window(SNES_OAM, m_layer[SNES_OAM].sub_window_enabled, window_below);

	charaddr = m_layer[SNES_OAM].charmap;

	priority[0] = priority_oam0;
	priority[1] = priority_oam1;
	priority[2] = priority_oam2;
	priority[3] = priority_oam3;

	uint8_t palbuf[256] = {};
	uint8_t pribuf[256] = {};

	/* finally draw the tiles from the tilelist */
	for (ii = 0; ii < 34; ii++)
	{
		int tile = ii;

#if SNES_LAYER_DEBUG
		if (m_debug_options.sprite_reversed)
			tile = 33 - ii;
#endif /* SNES_LAYER_DEBUG */

		if (m_oam_tilelist[tile].tileaddr == 0xffff)
			continue;

		pri = priority[m_oam_tilelist[tile].priority];

#if SNES_LAYER_DEBUG
		if (m_debug_options.select_pri[SNES_OAM])
		{
			int oam_draw = m_debug_options.select_pri[SNES_OAM] - 1;
			if (oam_draw != m_oam_tilelist[tile].priority)
				continue;
		}
#endif /* SNES_LAYER_DEBUG */

		/* OAM tiles have fixed planes (4), no direct color and no hires, but otherwise work the same as BG ones */
		draw_oamtile(charaddr + m_oam_tilelist[tile].tileaddr, m_oam_tilelist[tile].x, pri, m_oam_tilelist[tile].hflip, m_oam_tilelist[tile].pal, palbuf, pribuf);
	}

	for (uint16_t x = 0; x < SNES_SCR_WIDTH; x++)
	{
		if (pribuf[x] == 0) continue;
		uint16_t c = m_cgram[palbuf[x]];
		int blend = (palbuf[x] < 192) ? 1 : 0;
		if (m_layer[SNES_OAM].main_bg_enabled && window_above[x] == 0) plot_above(x, SNES_OAM, pribuf[x], c, blend);
		if (m_layer[SNES_OAM].sub_bg_enabled  && window_below[x] == 0) plot_below(x, SNES_OAM, pribuf[x], c, blend);
	}
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
	update_objects(3, 6, 9, 12);
}

void snes_ppu_device::update_mode_1( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[1])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!m_bg_priority)
	{
		update_line(curline, SNES_BG1, 0);
		update_line(curline, SNES_BG2, 0);
		update_line(curline, SNES_BG3, 0);
		update_objects(2, 4, 7, 10);
	}
	else
	{
		update_line(curline, SNES_BG1, 0);
		update_line(curline, SNES_BG2, 0);
		update_line(curline, SNES_BG3, 0);
		update_objects(2, 3, 6, 9);
	}
}

void snes_ppu_device::update_mode_2( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[2])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_line(curline, SNES_BG2, 0);
	update_objects(2, 4, 6, 8);
}

void snes_ppu_device::update_mode_3( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[3])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, m_direct_color);
	update_line(curline, SNES_BG2, 0);
	update_objects(2, 4, 6, 8);
}

void snes_ppu_device::update_mode_4( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[4])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, m_direct_color);
	update_line(curline, SNES_BG2, 0);
	update_objects(2, 4, 6, 8);
}

void snes_ppu_device::update_mode_5( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[5])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_line(curline, SNES_BG2, 0);
	update_objects(2, 4, 6, 8);
}

void snes_ppu_device::update_mode_6( uint16_t curline )
{
#if SNES_LAYER_DEBUG
	if (m_debug_options.mode_disabled[6])
		return;
#endif /* SNES_LAYER_DEBUG */

	update_line(curline, SNES_BG1, 0);
	update_objects(1, 3, 4, 6);
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
		update_objects(1, 3, 4, 5);
	}
	else
	{
		update_line_mode7(curline, SNES_BG1);
		update_line_mode7(curline, SNES_BG2);
		update_objects(2, 4, 6, 7);
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

	g_profiler.start(PROFILER_VIDEO);

	cache_background();

	if (m_screen_disabled) /* screen is forced blank */
		for (int x = 0; x < SNES_SCR_WIDTH * 2; x++)
			bitmap.pix32(0, x) = rgb_t::black();
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
		{
			g_profiler.stop();
			return;
		}

		/* Toggle drawing of SNES_SUBSCREEN or SNES_MAINSCREEN */
		if (m_debug_options.draw_subscreen)
		{
			above = &m_scanlines[SNES_SUBSCREEN];
			below = &m_scanlines[SNES_MAINSCREEN];
		}
#endif

		const bool hires = m_mode == 5 || m_mode == 6 || m_pseudo_hires;
		uint16_t above_color = m_cgram[0];
		uint16_t below_color = hires ? m_cgram[0] : m_cgram[FIXED_COLOUR];
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

		/* Draw backgrounds and OAM */
		draw_screens(curline);

		/* Prepare OAM for this scanline */
		update_objects_rto(curline);

		update_obsel();

		/* Draw the scanline to screen */
		uint16_t prev = 0;

		uint16_t *luma = &m_light_table[m_screen_brightness][0];
		for (int x = 0; x < SNES_SCR_WIDTH; x++)
		{
			/* in hires, the first pixel (of 512) is subscreen pixel, then the first mainscreen pixel follows, and so on... */
			if (!hires)
			{
				const uint16_t c = luma[pixel(x, above, below, window_above, window_below)];
				const int r = (c & 0x1f);
				const int g = (c & 0x3e0) >> 5;
				const int b = (c & 0x7c00) >> 10;

				bitmap.pix32(0, x * 2 + 0) = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
				bitmap.pix32(0, x * 2 + 1) = rgb_t(pal5bit(r), pal5bit(g), pal5bit(b));
			}
			else if (!blurring)
			{
				const uint16_t c0 = luma[pixel(x, below, above, window_above, window_below)];
				const uint16_t c1 = luma[pixel(x, above, below, window_above, window_below)];
				const int r0 = (c0 & 0x1f);
				const int r1 = (c1 & 0x1f);
				const int g0 = (c0 & 0x3e0) >> 5;
				const int g1 = (c1 & 0x3e0) >> 5;
				const int b0 = (c0 & 0x7c00) >> 10;
				const int b1 = (c1 & 0x7c00) >> 10;

				bitmap.pix32(0, x * 2 + 0) = rgb_t(pal5bit(r0), pal5bit(g0), pal5bit(b0));
				bitmap.pix32(0, x * 2 + 1) = rgb_t(pal5bit(r1), pal5bit(g1), pal5bit(b1));
			}
			else
			{
				uint16_t curr = luma[pixel(x, below, above, window_above, window_below)];

				uint16_t c0 = (prev + curr - ((prev ^ curr) & 0x0421)) >> 1;
				const int r0 = (c0 & 0x1f);
				const int g0 = (c0 & 0x3e0) >> 5;
				const int b0 = (c0 & 0x7c00) >> 10;
				bitmap.pix32(0, x * 2 + 0) = rgb_t(pal5bit(r0), pal5bit(g0), pal5bit(b0));

				prev = curr;
				curr = luma[pixel(x, above, below, window_above, window_below)];

				uint16_t c1 = (prev + curr - ((prev ^ curr) & 0x0421)) >> 1;
				const int r1 = (c1 & 0x1f);
				const int g1 = (c1 & 0x3e0) >> 5;
				const int b1 = (c1 & 0x7c00) >> 10;
				bitmap.pix32(0, x * 2 + 1) = rgb_t(pal5bit(r1), pal5bit(g1), pal5bit(b1));

				prev = curr;
			}
		}
	}

	g_profiler.stop();
}

uint16_t snes_ppu_device::pixel(uint16_t x, SNES_SCANLINE *above, SNES_SCANLINE *below, uint8_t *window_above, uint8_t *window_below)
{
	if (!window_above[x]) above->buffer[x] = 0;
	if (!window_below[x]) return above->buffer[x];
	if (!m_layer[above->layer[x]].color_math || (above->layer[x] == SNES_OAM && above->blend_exception[x])) return above->buffer[x];
	if (!m_sub_add_mode) return blend(above->buffer[x], m_cgram[FIXED_COLOUR], BIT(m_color_modes, 0) != 0 && window_above[x] != 0);
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

READ8_MEMBER( snes_ppu_device::vram_read )
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

WRITE8_MEMBER( snes_ppu_device::vram_write )
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
				m_vram[offset] = m_openbus_cb(space, 0);
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
 to 0x0218 (0x010c in our snes_oam).
 This is a hack, but it is more accurate than
 writing to the 'expected' address set by
 $2102,$2103.

 Notice that, since PPU_REG(OAMDATA) is never
 read/written directly, we use it as an index
 to choose the high/low byte of the snes_oam word.
*************************************************/

READ8_MEMBER( snes_ppu_device::oam_read )
{
	offset &= 0x1ff;

	if (offset & 0x100)
		offset &= 0x10f;

	if (!m_screen_disabled)
	{
		uint16_t v = screen().vpos();

		if (v < m_beam.last_visible_line)
			offset = 0x010c;
	}

	return (m_oam_ram[offset] >> (PPU_REG(OAMDATA) << 3)) & 0xff;
}

WRITE8_MEMBER( snes_ppu_device::oam_write )
{
	offset &= 0x1ff;

	if (offset & 0x100)
		offset &= 0x10f;

	if (!m_screen_disabled)
	{
		uint16_t v = screen().vpos();

		if (v < m_beam.last_visible_line)
			offset = 0x010c;
	}

	if (!(PPU_REG(OAMDATA)))
		m_oam_ram[offset] = (m_oam_ram[offset] & 0xff00) | (data << 0);
	else
		m_oam_ram[offset] = (m_oam_ram[offset] & 0x00ff) | (data << 8);
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
 than not, MESS has issues if we don't write to
 the expected address (see e.g. Tokimeki Memorial).
 This is because writes should work during hblank
 (so that the game can produce color fading), but
 ends up not working with the conditions below.
 Hence, for the moment, we only document the
 solution adopted by BSNES without enabling it.
*************************************************/

READ8_MEMBER( snes_ppu_device::cgram_read )
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

WRITE8_MEMBER( snes_ppu_device::cgram_write )
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
}

uint16_t snes_ppu_device::direct_color(uint16_t palette, uint16_t group)
{
  //palette = -------- BBGGGRRR
  //group   = -------- -----bgr
  //output  = 0BBb00GG Gg0RRRr0
  return (palette << 7 & 0x6000) + (group << 10 & 0x1000)
	   + (palette << 4 & 0x0380) + (group <<  5 & 0x0040)
	   + (palette << 2 & 0x001c) + (group <<  1 & 0x0002);
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
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG1].priority[0] = 8; m_layer[SNES_BG1].priority[1] = 11;
		m_layer[SNES_BG2].priority[0] = 7; m_layer[SNES_BG2].priority[1] = 10;
		m_layer[SNES_BG3].priority[0] = 2; m_layer[SNES_BG3].priority[1] =  5;
		m_layer[SNES_BG4].priority[0] = 1; m_layer[SNES_BG4].priority[1] =  4;
		break;
	case 1:
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		if (m_bg_priority)
		{
			m_layer[SNES_BG1].priority[0] = 5; m_layer[SNES_BG1].priority[1] =  8;
			m_layer[SNES_BG2].priority[0] = 4; m_layer[SNES_BG2].priority[1] =  7;
			m_layer[SNES_BG3].priority[0] = 1; m_layer[SNES_BG3].priority[1] = 10;
		}
		else
		{
			m_layer[SNES_BG1].priority[0] = 6; m_layer[SNES_BG1].priority[1] =  9;
			m_layer[SNES_BG2].priority[0] = 5; m_layer[SNES_BG2].priority[1] =  8;
			m_layer[SNES_BG3].priority[0] = 1; m_layer[SNES_BG3].priority[1] =  3;
		}
		break;
	case 2:
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		break;
	case 3:
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_8BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		break;
	case 4:
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_8BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		break;
	case 5:
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_2BPP;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  7;
		m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		break;
	case 6:
		m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_4BPP;
		m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
		m_layer[SNES_BG1].priority[0] = 2; m_layer[SNES_BG1].priority[1] =  5;
		break;
	case 7:
		if (!m_mode7.extbg)
		{
			m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_MODE7;
			m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG1].priority[0] = 2; m_layer[SNES_BG1].priority[1] =  2;
		}
		else
		{
			m_layer[SNES_BG1].tile_mode = SNES_COLOR_DEPTH_MODE7;
			m_layer[SNES_BG2].tile_mode = SNES_COLOR_DEPTH_MODE7;
			m_layer[SNES_BG3].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG4].tile_mode = SNES_COLOR_DEPTH_NONE;
			m_layer[SNES_BG1].priority[0] = 3; m_layer[SNES_BG1].priority[1] =  3;
			m_layer[SNES_BG2].priority[0] = 1; m_layer[SNES_BG2].priority[1] =  5;
		}
		break;
	}
}

uint8_t snes_ppu_device::read(address_space &space, uint32_t offset, uint8_t wrio_bit7)
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
			return m_openbus_cb(space, 0);       /* Return value is meaningless */

		case ROAMDATA:  /* Read data from OAM (DR) */
			m_ppu1_open_bus = oam_read(space, m_oam.address);
			PPU_REG(OAMDATA) = (PPU_REG(OAMDATA) + 1) % 2;
			if (!PPU_REG(OAMDATA))
			{
				m_oam.address++;
				m_oam.address &= 0x1ff;
				m_oam.first_sprite = m_oam.priority_rotation ? (m_oam.address >> 1) & 127 : 0;
			}
			return m_ppu1_open_bus;
		case RVMDATAL:  /* Read data from VRAM (low) */
			{
				uint32_t addr = get_vram_address();
				m_ppu1_open_bus = m_vram_read_buffer & 0xff;

				if (!m_vram_fgr_high)
				{
					m_vram_read_buffer = vram_read(space, addr);
					m_vram_read_buffer |= (vram_read(space, addr + 1) << 8);

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
					m_vram_read_buffer = vram_read(space, addr);
					m_vram_read_buffer |= (vram_read(space, addr + 1) << 8);

					m_vmadd = (m_vmadd + m_vram_fgr_increment) & 0xffff;
				}

				return m_ppu1_open_bus;
			}
		case RCGDATA:   /* Read data from CGRAM */
			if (!(m_cgram_address & 0x01))
				m_ppu2_open_bus = cgram_read(space, m_cgram_address);
			else
			{
				m_ppu2_open_bus &= 0x80;
				m_ppu2_open_bus |= cgram_read(space, m_cgram_address) & 0x7f;
			}

			m_cgram_address = (m_cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
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
	return m_openbus_cb(space, 0);
}


void snes_ppu_device::write(address_space &space, uint32_t offset, uint8_t data)
{
	switch (offset)
	{
		case INIDISP:   /* Initial settings for screen */
			if ((m_screen_disabled & 0x80) && (!(data & 0x80))) //a 1->0 force blank transition causes a reset OAM address
			{
				space.write_byte(OAMADDL, m_oam.saved_address_low);
				space.write_byte(OAMADDH, m_oam.saved_address_high);
				m_oam.first_sprite = m_oam.priority_rotation ? (m_oam.address >> 1) & 127 : 0;
			}
			m_screen_disabled = data & 0x80;
			m_screen_brightness = data & 0x0f;
			break;
		case OBSEL:     /* Object size and data area designation */
			m_oam.next_charmap = (data << 13) & 0x6000;
			m_oam.next_name_select = (data >> 3) & 3;
			m_oam.next_size = (data >> 5) & 7;
			break;
		case OAMADDL:   /* Address for accessing OAM (low) */
			m_oam.saved_address_low = data;
			m_oam.address = (m_oam.address & 0xff00) + data;
			m_oam.first_sprite = m_oam.priority_rotation ? (m_oam.address >> 1) & 127 : 0;
			PPU_REG(OAMDATA) = 0;
			break;
		case OAMADDH:   /* Address for accessing OAM (high) */
			m_oam.saved_address_high = data;
			m_oam.address = (m_oam.address & 0x00ff) | ((data & 0x01) << 8);
			m_oam.priority_rotation = BIT(data, 7);
			m_oam.first_sprite = m_oam.priority_rotation ? (m_oam.address >> 1) & 127 : 0;
			PPU_REG(OAMDATA) = 0;
			break;
		case OAMDATA:   /* Data for OAM write (DW) */
			if (m_oam.address >= 0x100)
				oam_write(space, m_oam.address, data);
			else
			{
				if (!PPU_REG(OAMDATA))
					m_oam.write_latch = data;
				else
				{
					// in this case, we not only write data to the upper byte of the word,
					// but also m_oam.write_latch to the lower byte (recall that
					// PPU_REG(OAMDATA) is used to select high/low byte)
					oam_write(space, m_oam.address, data);
					PPU_REG(OAMDATA) = 0;
					oam_write(space, m_oam.address, m_oam.write_latch);
					PPU_REG(OAMDATA) = 1;
				}
			}
			PPU_REG(OAMDATA) = (PPU_REG(OAMDATA) + 1) % 2;
			if (!PPU_REG(OAMDATA))
			{
				m_oam.address++;
				m_oam.address &= 0x1ff;
				m_oam.first_sprite = m_oam.priority_rotation ? (m_oam.address >> 1) & 127 : 0;
			}
			return;
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
				m_vram_read_buffer = vram_read(space, addr);
				m_vram_read_buffer |= (vram_read(space, addr + 1) << 8);
			}
			break;
		case VMADDH:    /* Address for VRAM read/write (high) */
			{
				uint32_t addr;
				m_vmadd = (m_vmadd & 0x00ff) | (data << 8);
				addr = get_vram_address();
				m_vram_read_buffer = vram_read(space, addr);
				m_vram_read_buffer |= (vram_read(space, addr + 1) << 8);
			}
			break;
		case VMDATAL:   /* 2118: Data for VRAM write (low) */
			{
				uint32_t addr = get_vram_address();
				vram_write(space, addr, data);

				if (!m_vram_fgr_high)
					m_vmadd = (m_vmadd + m_vram_fgr_increment) & 0xffff;
			}
			return;
		case VMDATAH:   /* 2119: Data for VRAM write (high) */
			{
				uint32_t addr = get_vram_address();
				vram_write(space, addr + 1, data);

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
			cgram_write(space, m_cgram_address, data);
			m_cgram_address = (m_cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
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
				/* Store it in the extra space we made in the CGRAM. It doesn't really go there, but it's as good a place as any. */
				uint8_t r, g, b;

				/* Get existing value. */
				r = m_cgram[FIXED_COLOUR] & 0x1f;
				g = (m_cgram[FIXED_COLOUR] & 0x3e0) >> 5;
				b = (m_cgram[FIXED_COLOUR] & 0x7c00) >> 10;
				/* Set new value */
				if (data & 0x20)
					r = data & 0x1f;
				if (data & 0x40)
					g = data & 0x1f;
				if (data & 0x80)
					b = data & 0x1f;
				m_cgram[FIXED_COLOUR] = (r | (g << 5) | (b << 10));
			} break;
		case SETINI:    /* Screen mode/video select */
			m_interlace = (data & 0x01) ? 2 : 1;
			m_obj_interlace = (data & 0x02) ? 2 : 1;
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
	m_debug_options.sprite_reversed = BIT(toggles, 7);
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

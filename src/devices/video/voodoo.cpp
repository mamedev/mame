// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.c

    3dfx Voodoo Graphics SST-1/2 emulator.

****************************************************************************

//fix me -- blitz2k dies when starting a game with heavy fog (in DRC)

****************************************************************************

    3dfx Voodoo Graphics SST-1/2 emulator

    emulator by Aaron Giles

    --------------------------

    Specs:

    Voodoo 1 (SST1):
        2,4MB frame buffer RAM
        1,2,4MB texture RAM
        50MHz clock frequency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        64 entry PCI FIFO
        memory FIFO up to 65536 entries

    Voodoo 2:
        2,4MB frame buffer RAM
        2,4,8,16MB texture RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 16 pixels/clock
        128 entry PCI FIFO
        memory FIFO up to 65536 entries

    Voodoo Banshee (h3):
        Integrated VGA support
        2,4,8MB frame buffer RAM
        90MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    Voodoo 3 ("Avenger"/h4):
        Integrated VGA support
        4,8,16MB frame buffer RAM
        143MHz clock frquency
        clears @ 2 pixels/clock (RGB and depth simultaneously)
        renders @ 1 pixel/clock
        ultrafast clears @ 32 pixels/clock

    --------------------------

    still to be implemented:
        * trilinear textures

    things to verify:
        * floating Z buffer


iterated RGBA = 12.12 [24 bits]
iterated Z    = 20.12 [32 bits]
iterated W    = 18.32 [48 bits]

>mamepm blitz
Stall PCI for HWM: 1
PCI FIFO Empty Entries LWM: D
LFB -> FIFO: 1
Texture -> FIFO: 1
Memory FIFO: 1
Memory FIFO HWM: 2000
Memory FIFO Write Burst HWM: 36
Memory FIFO LWM for PCI: 5
Memory FIFO row start: 120
Memory FIFO row rollover: 3FF
Video dither subtract: 0
DRAM banking: 1
Triple buffer: 0
Video buffer offset: 60
DRAM banking: 1

>mamepm wg3dh
Stall PCI for HWM: 1
PCI FIFO Empty Entries LWM: D
LFB -> FIFO: 1
Texture -> FIFO: 1
Memory FIFO: 1
Memory FIFO HWM: 2000
Memory FIFO Write Burst HWM: 36
Memory FIFO LWM for PCI: 5
Memory FIFO row start: C0
Memory FIFO row rollover: 3FF
Video dither subtract: 0
DRAM banking: 1
Triple buffer: 0
Video buffer offset: 40
DRAM banking: 1


As a point of reference, the 3D engine uses the following algorithm to calculate the linear memory address as a
function of the video buffer offset (fbiInit2 bits(19:11)), the number of 32x32 tiles in the X dimension (fbiInit1
bits(7:4) and bit(24)), X, and Y:

    tilesInX[4:0] = {fbiInit1[24], fbiInit1[7:4], fbiInit6[30]}
    rowBase = fbiInit2[19:11]
    rowStart = ((Y>>5) * tilesInX) >> 1

    if (!(tilesInX & 1))
    {
        rowOffset = (X>>6);
        row[9:0] = rowStart + rowOffset (for color buffer 0)
        row[9:0] = rowBase + rowStart + rowOffset (for color buffer 1)
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for depth/alpha buffer when double color buffering[fbiInit5[10:9]=0])
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for color buffer 2 when triple color buffering[fbiInit5[10:9]=1 or 2])
        row[9:0] = (rowBase<<1) + rowBase + rowStart + rowOffset (for depth/alpha buffer when triple color buffering[fbiInit5[10:9]=2])
        column[8:0] = ((Y % 32) <<4) + ((X % 32)>>1)
        ramSelect[1] = ((X&0x20) ? 1 : 0) (for color buffers)
        ramSelect[1] = ((X&0x20) ? 0 : 1) (for depth/alpha buffers)
    }
    else
    {
        rowOffset = (!(Y&0x20)) ? (X>>6) : ((X>31) ? (((X-32)>>6)+1) : 0)
        row[9:0] = rowStart + rowOffset (for color buffer 0)
        row[9:0] = rowBase + rowStart + rowOffset (for color buffer 1)
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for depth/alpha buffer when double color buffering[fbiInit5[10:9]=0])
        row[9:0] = (rowBase<<1) + rowStart + rowOffset (for color buffer 2 when triple color buffering[fbiInit5[10:9]=1 or 2])
        row[9:0] = (rowBase<<1) + rowBase + rowStart + rowOffset (for depth/alpha buffer when triple color buffering[fbiInit5[10:9]=2])
        column[8:0] = ((Y % 32) <<4) + ((X % 32)>>1)
        ramSelect[1] = (((X&0x20)^(Y&0x20)) ? 1 : 0) (for color buffers)
        ramSelect[1] = (((X&0x20)^(Y&0x20)) ? 0 : 1) (for depth/alpha buffers)
    }
    ramSelect[0] = X % 2
    pixelMemoryAddress[21:0] = (row[9:0]<<12) + (column[8:0]<<3) + (ramSelect[1:0]<<1)
    bankSelect = pixelMemoryAddress[21]

**************************************************************************/


#include "emu.h"
#include "voodoo.h"

#include "screen.h"

using namespace voodoo;


//**************************************************************************
//  DEBUGGING
//**************************************************************************

// debug
static constexpr bool DEBUG_DEPTH = false;		// ENTER key to view depthbuf
static constexpr bool DEBUG_BACKBUF = false;	// L key to view backbuf
static constexpr bool DEBUG_STATS = false;		// \ key to view stats

// logging
static constexpr bool LOG_VBLANK_SWAP = false;
static constexpr bool LOG_FIFO = false;
static constexpr bool LOG_FIFO_VERBOSE = false;
static constexpr bool LOG_REGISTERS = false;
static constexpr bool LOG_WAITS = false;
static constexpr bool LOG_LFB = false;
static constexpr bool LOG_TEXTURE_RAM = false;
static constexpr bool LOG_CMDFIFO = false;
static constexpr bool LOG_CMDFIFO_VERBOSE = false;
static constexpr bool LOG_BANSHEE_2D = false;

// Need to turn off cycle eating when debugging MIPS drc
// otherwise timer interrupts won't match nodrc debug mode.
static constexpr bool EAT_CYCLES = true;



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// enumeration describing reasons we might be stalled
enum
{
	NOT_STALLED = 0,
	STALLED_UNTIL_FIFO_LWM,
	STALLED_UNTIL_FIFO_EMPTY
};

// number of clocks to set up a triangle (just a guess)
static constexpr u32 TRIANGLE_SETUP_CLOCKS = 100;



//**************************************************************************
//  GLOBAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  float_to_int32 - convert a floating-point
//  value in raw IEEE format into an integer with
//  the given number of fractional bits
//-------------------------------------------------

static inline s32 float_to_int32(u32 data, int fixedbits)
{
	// compute the effective exponent
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;

	// extract the mantissa and return the implied leading 1 bit
	s32 result = (data & 0x7fffff) | 0x800000;

	// shift by the exponent, handling minimum/maximum
	if (exponent < 0)
	{
		if (exponent > -32)
			result >>= -exponent;
		else
			result = 0;
	}
	else
	{
		if (exponent < 32)
			result <<= exponent;
		else
			result = 0x7fffffff;
	}

	// negate based on the sign
	return (data & 0x80000000) ? -result : result;
}


//-------------------------------------------------
//  float_to_int64 - convert a floating-point
//  value in raw IEEE format into an integer with
//  the given number of fractional bits
//-------------------------------------------------

static inline s64 float_to_int64(u32 data, int fixedbits)
{
	// compute the effective exponent
	int exponent = ((data >> 23) & 0xff) - 127 - 23 + fixedbits;

	// extract the mantissa and return the implied leading 1 bit
	s64 result = (data & 0x7fffff) | 0x800000;

	// shift by the exponent, handling minimum/maximum
	if (exponent < 0)
	{
		if (exponent > -64)
			result >>= -exponent;
		else
			result = 0;
	}
	else
	{
		if (exponent < 64)
			result <<= exponent;
		else
			result = 0x7fffffffffffffffull;
	}

	// negate based on the sign
	return (data & 0x80000000) ? -result : result;
}



//**************************************************************************
//  MEMORY FIFO
//**************************************************************************

//-------------------------------------------------
//  memory_fifo - constructor
//-------------------------------------------------

voodoo::memory_fifo::memory_fifo() :
	m_base(nullptr),
	m_size(0),
	m_in(0),
	m_out(0)
{
}


//-------------------------------------------------
//  add - append an item to the fifo
//-------------------------------------------------

inline void voodoo::memory_fifo::add(u32 data)
{
	// compute the value of 'in' after we add this item
	s32 next_in = m_in + 1;
	if (next_in >= m_size)
		next_in = 0;

	// as long as it's not equal to the output pointer, we can do it
	if (next_in != m_out)
	{
		m_base[m_in] = data;
		m_in = next_in;
	}
}


//-------------------------------------------------
//  remove - remove the next item from the fifo
//-------------------------------------------------

inline u32 voodoo::memory_fifo::remove()
{
	// return invalid data if empty
	if (m_out == m_in)
		return 0xffffffff;

	// determine next output
	s32 next_out = m_out + 1;
	if (next_out >= m_size)
		next_out = 0;

	// fetch current and advance
	u32 data = m_base[m_out];
	m_out = next_out;
	return data;
}



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

int voodoo_device::update_common(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t const *pens)
{
	// reset the video changed flag
	bool changed = m_fbi.m_video_changed;
	m_fbi.m_video_changed = false;

	// select the buffer to draw
	int drawbuf = m_fbi.m_frontbuf;
	if (DEBUG_BACKBUF && machine().input().code_pressed(KEYCODE_L))
		drawbuf = m_fbi.m_backbuf;

	// copy from the current front buffer
	if (LOG_VBLANK_SWAP) logerror("--- update_common @ %d from %08X\n", m_screen->vpos(), m_fbi.m_rgboffs[m_fbi.m_frontbuf]);
	for (s32 y = cliprect.min_y; y <= cliprect.max_y; y++)
		m_fbi.copy_scanline(&bitmap.pix(y), drawbuf, y, cliprect.min_x, cliprect.max_x + 1, pens);

	// update stats display
	if (DEBUG_STATS)
	{
		int statskey = (machine().input().code_pressed(KEYCODE_BACKSLASH));
		if (statskey && statskey != m_stats.lastkey)
			m_stats.display = !m_stats.display;
		m_stats.lastkey = statskey;

		/* display stats */
		if (m_stats.display)
			popmessage(m_stats.buffer, 0, 0);
	}

	// update render override
	if (DEBUG_DEPTH)
	{
		m_stats.render_override = machine().input().code_pressed(KEYCODE_ENTER);
		if (m_stats.render_override)
		{
			for (s32 y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				u16 const *const src = m_fbi.aux_buffer() + (y - m_fbi.m_yoffs) * m_fbi.m_rowpixels - m_fbi.m_xoffs;
				u32 *const dst = &bitmap.pix(y);
				for (s32 x = cliprect.min_x; x <= cliprect.max_x; x++)
					dst[x] = ((src[x] << 8) & 0xff0000) | ((src[x] >> 0) & 0xff00) | ((src[x] >> 8) & 0xff);
			}
		}
	}
	return changed;
}

int voodoo_device::update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if we are blank, just fill with black
	if (m_reg.fbi_init1().software_blank())
	{
		bitmap.fill(0, cliprect);
		int changed = m_fbi.m_video_changed;
		m_fbi.m_video_changed = false;
		return changed;
	}

	// if the CLUT is dirty, recompute the pens array
	if (m_fbi.m_clut_dirty)
	{
		rgb_t const *clutbase = &m_fbi.m_clut[0];

		// kludge: some of the Midway games write 0 to the last entry when they obviously mean FF
		if ((m_fbi.m_clut[32] & 0xffffff) == 0 && (m_fbi.m_clut[31] & 0xffffff) != 0)
			m_fbi.m_clut[32] = 0x20ffffff;

		// compute the R/G/B pens first
		u8 rtable[32], gtable[64], btable[32];
		for (u32 rawcolor = 0; rawcolor < 32; rawcolor++)
		{
			// treat X as a 5-bit value, scale up to 8 bits, and linear interpolate for red/blue
			u32 color = pal5bit(rawcolor);
			rtable[rawcolor] = (clutbase[color >> 3].r() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].r() * (color & 7)) >> 3;
			btable[rawcolor] = (clutbase[color >> 3].b() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].b() * (color & 7)) >> 3;

			// treat X as a 6-bit value with LSB=0, scale up to 8 bits, and linear interpolate
			color = pal6bit(rawcolor * 2 + 0);
			gtable[rawcolor * 2 + 0] = (clutbase[color >> 3].g() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].g() * (color & 7)) >> 3;

			// treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate
			color = pal6bit(rawcolor * 2 + 1);
			gtable[rawcolor * 2 + 1] = (clutbase[color >> 3].g() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].g() * (color & 7)) >> 3;
		}

		// now compute the actual pens array
		for (u32 pen = 0; pen < 65536; pen++)
			m_fbi.m_pen[pen] = rgb_t(rtable[BIT(pen, 11, 5)], gtable[BIT(pen, 5, 6)], btable[BIT(pen, 0, 5)]);

		// no longer dirty
		m_fbi.m_clut_dirty = false;
		m_fbi.m_video_changed = true;
	}
	return update_common(bitmap, cliprect, &m_fbi.m_pen[0]);
}


int voodoo_banshee_device::update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if bypassing the clut, don't worry about the rest
	if (BIT(m_banshee.io[io_vidProcCfg], 11))
		return update_common(bitmap, cliprect, m_shared->rgb565);

	// if the CLUT is dirty, recompute the pens array
	if (m_fbi.m_clut_dirty)
	{
		rgb_t const *clutbase = &m_fbi.m_clut[256 * BIT(m_banshee.io[io_vidProcCfg], 13)];

		// compute R/G/B pens first
		u8 rtable[32], gtable[64], btable[32];
		for (u32 rawcolor = 0; rawcolor < 32; rawcolor++)
		{
			// treat X as a 5-bit value, scale up to 8 bits
			u32 color = pal5bit(rawcolor);
			rtable[rawcolor] = clutbase[color].r();
			btable[rawcolor] = clutbase[color].b();

			// treat X as a 6-bit value with LSB=0, scale up to 8 bits, and linear interpolate
			color = pal6bit(rawcolor * 2 + 0);
			gtable[rawcolor * 2 + 0] = clutbase[color].g();

			// treat X as a 6-bit value with LSB=1, scale up to 8 bits, and linear interpolate
			color = pal6bit(rawcolor * 2 + 1);
			gtable[rawcolor * 2 + 1] = clutbase[color].g();
		}

		// now compute the actual pens array
		for (u32 pen = 0; pen < 65536; pen++)
			m_fbi.m_pen[pen] = rgb_t(rtable[BIT(pen, 11, 5)], gtable[BIT(pen, 5, 6)], btable[BIT(pen, 0, 5)]);

		// no longer dirty
		m_fbi.m_clut_dirty = false;
		m_fbi.m_video_changed = true;
	}
	return update_common(bitmap, cliprect, &m_fbi.m_pen[0]);
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

void voodoo_device::set_init_enable(u32 newval)
{
	m_pci.init_enable = voodoo::reg_init_en(newval);
	if (LOG_REGISTERS)
		logerror("VOODOO.REG:initEnable write = %08X\n", newval);
}



/*************************************
 *
 *  Common initialization
 *
 *************************************/

void voodoo_device::fbi_state::init(voodoo_model model, std::vector<u8> &memory)
{
	// allocate frame buffer RAM and set pointers
	m_ram = &memory[0];
	m_mask = memory.size() - 1;
	m_rgboffs[0] = m_rgboffs[1] = m_rgboffs[2] = 0;
	m_auxoffs = ~0;

	m_frontbuf = 0;
	m_backbuf = 1;
	m_swaps_pending = 0;
	m_video_changed = true;

	m_yorigin = 0;
	m_lfb_base = 0;
	m_lfb_stride = (model <= MODEL_VOODOO_2) ? 10 : 11;

	m_width = 512;
	m_height = 384;
	m_xoffs = 0;
	m_yoffs = 0;
	m_vsyncstart = 0;
	m_vsyncstop = 0;
	m_rowpixels = 0;

	m_vblank = 0;
	m_vblank_count = 0;
	m_vblank_swap_pending = 0;
	m_vblank_swap = 0;
	m_vblank_dont_swap = 0;

	m_lfb_stats.reset();

	m_sverts = 0;

	// initialize the memory FIFO
	m_fifo.configure(nullptr, 0);
	m_cmdfifo[0].init(memory);
	m_cmdfifo[1].init(memory);

	if (model <= MODEL_VOODOO_2)
	{
		for (int pen = 0; pen < 32; pen++)
			m_clut[pen] = rgb_t(pen, pal5bit(pen), pal5bit(pen), pal5bit(pen));
		m_clut[32] = rgb_t(32,0xff,0xff,0xff);
	}
	else
	{
		for (int pen = 0; pen < 512; pen++)
			m_clut[pen] = rgb_t(pen, pen, pen);
	}
	m_clut_dirty = true;
}


shared_tables::shared_tables()
{
	// build static 8-bit texel tables
	for (int val = 0; val < 256; val++)
	{
		// 8-bit RGB (3-3-2)
		rgb332[val] = rgbexpand<3,3,2>(val, 5, 2, 0).set_a(0xff);

		// 8-bit alpha
		alpha8[val] = rgb_t(val, val, val, val);

		// 8-bit intensity
		int8[val] = rgb_t(0xff, val, val, val);

		// 8-bit alpha, intensity
		ai44[val] = argbexpand<4,4,4,4>(val, 4, 0, 0, 0);
	}

	// build static 16-bit texel tables
	for (int val = 0; val < 65536; val++)
	{
		// table 10 = 16-bit RGB (5-6-5)
		rgb565[val] = rgbexpand<5,6,5>(val, 11, 5, 0).set_a(0xff);

		// table 11 = 16 ARGB (1-5-5-5)
		argb1555[val] = argbexpand<1,5,5,5>(val, 15, 10, 5, 0);

		// table 12 = 16-bit ARGB (4-4-4-4)
		argb4444[val] = argbexpand<4,4,4,4>(val, 12, 8, 4, 0);
	}
}


void voodoo_device::tmu_state::init(int index, shared_tables &share, std::vector<u8> &memory)
{
	// configure texture RAM
	m_index = index;
	m_ram = &memory[0];
	m_mask = memory.size() - 1;
	m_regdirty = true;
	m_palette_dirty[0] = m_palette_dirty[1] = m_palette_dirty[2] = m_palette_dirty[3] = true;

	// create pointers to the static lookups
	m_texel[0] = share.rgb332;
	m_texel[1] = nullptr;
	m_texel[2] = share.alpha8;
	m_texel[3] = share.int8;
	m_texel[4] = share.ai44;
	m_texel[5] = nullptr;
	m_texel[6] = nullptr;
	m_texel[7] = nullptr;
	m_texel[8] = share.rgb332;
	m_texel[9] = nullptr;
	m_texel[10] = share.rgb565;
	m_texel[11] = share.argb1555;
	m_texel[12] = share.argb4444;
	m_texel[13] = share.int8;
	m_texel[14] = nullptr;
	m_texel[15] = nullptr;
}


ALLOW_SAVE_TYPE(voodoo::reg_init_en);
ALLOW_SAVE_TYPE(voodoo::voodoo_regs::register_data);

void voodoo_device::register_save()
{
	/* register states: core */
	save_item(NAME(m_reg.m_regs));

	/* register states: pci */
//	save_item(NAME(m_pci.fifo.m_in));
//	save_item(NAME(m_pci.fifo.m_out));
	save_item(NAME(m_pci.init_enable));
	save_item(NAME(m_pci.stall_state));
	save_item(NAME(m_pci.op_pending));
	save_item(NAME(m_pci.op_end_time));
	save_item(NAME(m_pci.fifo_mem));

	/* register states: dac */
	save_item(NAME(m_dac.m_reg));
	save_item(NAME(m_dac.read_result));

	/* register states: fbi */
	save_pointer(NAME(m_fbi.m_ram), m_fbi.m_mask + 1);
	save_item(NAME(m_fbi.m_rgboffs));
	save_item(NAME(m_fbi.m_auxoffs));
	save_item(NAME(m_fbi.m_frontbuf));
	save_item(NAME(m_fbi.m_backbuf));
	save_item(NAME(m_fbi.m_swaps_pending));
	save_item(NAME(m_fbi.m_video_changed));
	save_item(NAME(m_fbi.m_yorigin));
	save_item(NAME(m_fbi.m_lfb_base));
	save_item(NAME(m_fbi.m_lfb_stride));
	save_item(NAME(m_fbi.m_width));
	save_item(NAME(m_fbi.m_height));
	save_item(NAME(m_fbi.m_xoffs));
	save_item(NAME(m_fbi.m_yoffs));
	save_item(NAME(m_fbi.m_vsyncstart));
	save_item(NAME(m_fbi.m_vsyncstop));
	save_item(NAME(m_fbi.m_rowpixels));
	save_item(NAME(m_fbi.m_vblank));
	save_item(NAME(m_fbi.m_vblank_count));
	save_item(NAME(m_fbi.m_vblank_swap_pending));
	save_item(NAME(m_fbi.m_vblank_swap));
	save_item(NAME(m_fbi.m_vblank_dont_swap));
	save_item(NAME(m_fbi.m_sign));
	save_item(NAME(m_fbi.m_ax));
	save_item(NAME(m_fbi.m_ay));
	save_item(NAME(m_fbi.m_bx));
	save_item(NAME(m_fbi.m_by));
	save_item(NAME(m_fbi.m_cx));
	save_item(NAME(m_fbi.m_cy));
	save_item(NAME(m_fbi.m_startr));
	save_item(NAME(m_fbi.m_startg));
	save_item(NAME(m_fbi.m_startb));
	save_item(NAME(m_fbi.m_starta));
	save_item(NAME(m_fbi.m_startz));
	save_item(NAME(m_fbi.m_startw));
	save_item(NAME(m_fbi.m_drdx));
	save_item(NAME(m_fbi.m_dgdx));
	save_item(NAME(m_fbi.m_dbdx));
	save_item(NAME(m_fbi.m_dadx));
	save_item(NAME(m_fbi.m_dzdx));
	save_item(NAME(m_fbi.m_dwdx));
	save_item(NAME(m_fbi.m_drdy));
	save_item(NAME(m_fbi.m_dgdy));
	save_item(NAME(m_fbi.m_dbdy));
	save_item(NAME(m_fbi.m_dady));
	save_item(NAME(m_fbi.m_dzdy));
	save_item(NAME(m_fbi.m_dwdy));
	save_item(NAME(m_fbi.m_lfb_stats.pixels_in));
	save_item(NAME(m_fbi.m_lfb_stats.pixels_out));
	save_item(NAME(m_fbi.m_lfb_stats.chroma_fail));
	save_item(NAME(m_fbi.m_lfb_stats.zfunc_fail));
	save_item(NAME(m_fbi.m_lfb_stats.afunc_fail));
	save_item(NAME(m_fbi.m_lfb_stats.clip_fail));
	save_item(NAME(m_fbi.m_lfb_stats.stipple_count));
	save_item(NAME(m_fbi.m_sverts));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, x));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, y));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, a));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, r));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, g));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, b));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, z));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, wb));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, w0));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, s0));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, t0));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, w1));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, s1));
	save_item(STRUCT_MEMBER(m_fbi.m_svert, t1));
//	save_item(NAME(m_fbi.m_fifo.m_size));
//	save_item(NAME(m_fbi.m_fifo.m_in));
//	save_item(NAME(m_fbi.m_fifo.m_out));
/*
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, enable));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, count_holes));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, base));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, end));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, rdptr));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, amin));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, amax));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, depth));
	save_item(STRUCT_MEMBER(m_fbi.m_cmdfifo, holes));
*/
	save_item(NAME(m_fbi.m_clut));

	// renderer states
//	save_item(NAME(m_renderer->m_fogblend));
//	save_item(NAME(m_renderer->m_fogdelta));

	/* register states: tmu */
	for (int index = 0; index < std::size(m_tmu); index++)
	{
		tmu_state *tmu = &m_tmu[index];
		if (tmu->m_ram == nullptr)
			continue;
		save_item(NAME(tmu->m_reg.m_regs), index);
		if (tmu->m_ram != m_fbi.m_ram)
			save_pointer(NAME(tmu->m_ram), tmu->m_mask + 1, index);
		save_item(NAME(tmu->m_starts), index);
		save_item(NAME(tmu->m_startt), index);
		save_item(NAME(tmu->m_startw), index);
		save_item(NAME(tmu->m_dsdx), index);
		save_item(NAME(tmu->m_dtdx), index);
		save_item(NAME(tmu->m_dwdx), index);
		save_item(NAME(tmu->m_dsdy), index);
		save_item(NAME(tmu->m_dtdy), index);
		save_item(NAME(tmu->m_dwdy), index);
		save_item(NAME(tmu->m_palette), index);
	}
}



/*************************************
 *
 *  Statistics management
 *
 *************************************/

void voodoo_device::accumulate_statistics(const thread_stats_block &block)
{
	/* apply internal voodoo statistics */
	m_reg.add(voodoo_regs::reg_fbiPixelsIn, block.pixels_in);
	m_reg.add(voodoo_regs::reg_fbiPixelsOut, block.pixels_out);
	m_reg.add(voodoo_regs::reg_fbiChromaFail, block.chroma_fail);
	m_reg.add(voodoo_regs::reg_fbiZfuncFail, block.zfunc_fail);
	m_reg.add(voodoo_regs::reg_fbiAfuncFail, block.afunc_fail);

	/* apply emulation statistics */
	if (DEBUG_STATS)
	{
		m_stats.total_pixels_in += block.pixels_in;
		m_stats.total_pixels_out += block.pixels_out;
		m_stats.total_chroma_fail += block.chroma_fail;
		m_stats.total_zfunc_fail += block.zfunc_fail;
		m_stats.total_afunc_fail += block.afunc_fail;
		m_stats.total_clipped += block.clip_fail;
		m_stats.total_stippled += block.stipple_count;
	}
}


void voodoo_device::update_statistics(bool accumulate)
{
	/* accumulate/reset statistics from all units */
	for (auto &stats : m_renderer->thread_stats())
	{
		if (accumulate)
			accumulate_statistics(stats);
		stats.reset();
	}

	/* accumulate/reset statistics from the LFB */
	if (accumulate)
		accumulate_statistics(m_fbi.m_lfb_stats);
	m_fbi.m_lfb_stats.reset();
}



/*************************************
 *
 *  VBLANK management
 *
 *************************************/

void voodoo_device::swap_buffers()
{
	if (LOG_VBLANK_SWAP) logerror("--- swap_buffers @ %d\n", m_screen->vpos());

	/* force a partial update */
	m_screen->update_partial(m_screen->vpos());
	m_fbi.m_video_changed = true;

	/* keep a history of swap intervals */
	m_reg.update_swap_history(std::min<u8>(m_fbi.m_vblank_count, 15));

	/* rotate the buffers */
	if (m_reg.rev1_or_2())
	{
		if (m_reg.rev1() || !m_fbi.m_vblank_dont_swap)
		{
			if (m_fbi.m_rgboffs[2] == ~0)
			{
				m_fbi.m_frontbuf = 1 - m_fbi.m_frontbuf;
				m_fbi.m_backbuf = 1 - m_fbi.m_frontbuf;
			}
			else
			{
				m_fbi.m_frontbuf = (m_fbi.m_frontbuf + 1) % 3;
				m_fbi.m_backbuf = (m_fbi.m_frontbuf + 1) % 3;
			}
		}
	}
	else
		m_fbi.m_rgboffs[0] = m_reg.read(voodoo_regs::reg_leftOverlayBuf) & m_fbi.m_mask & ~0x0f;

	/* decrement the pending count and reset our state */
	if (m_fbi.m_swaps_pending)
		m_fbi.m_swaps_pending--;
	m_fbi.m_vblank_count = 0;
	m_fbi.m_vblank_swap_pending = false;

	/* reset the last_op_time to now and start processing the next command */
	if (m_pci.op_pending)
	{
		if (LOG_VBLANK_SWAP) logerror("---- swap_buffers flush begin\n");
		m_pci.op_end_time = machine().time();
		flush_fifos(m_pci.op_end_time);
		if (LOG_VBLANK_SWAP) logerror("---- swap_buffers flush end\n");
	}

	/* we may be able to unstall now */
	if (m_pci.stall_state != NOT_STALLED)
		check_stalled_cpu(machine().time());

	/* periodically log rasterizer info */
	m_stats.swaps++;
	if (m_stats.swaps % 1000 == 0)
		m_renderer->dump_rasterizer_stats();

	/* update the statistics (debug) */
	if (DEBUG_STATS && m_stats.display)
	{
		const rectangle &visible_area = m_screen->visible_area();
		int screen_area = visible_area.width() * visible_area.height();
		char *statsptr = m_stats.buffer;
		int pixelcount;
		int i;

		update_statistics(true);
		pixelcount = m_stats.total_pixels_out;

		statsptr += sprintf(statsptr, "Swap:%6d\n", m_stats.swaps);
		statsptr += sprintf(statsptr, "Hist:%08X\n", m_reg.swap_history());
		statsptr += sprintf(statsptr, "Stal:%6d\n", m_stats.stalls);
		statsptr += sprintf(statsptr, "Rend:%6d%%\n", pixelcount * 100 / screen_area);
		statsptr += sprintf(statsptr, "Poly:%6d\n", m_stats.total_triangles);
		statsptr += sprintf(statsptr, "PxIn:%6d\n", m_stats.total_pixels_in);
		statsptr += sprintf(statsptr, "POut:%6d\n", m_stats.total_pixels_out);
		statsptr += sprintf(statsptr, "Clip:%6d\n", m_stats.total_clipped);
		statsptr += sprintf(statsptr, "Stip:%6d\n", m_stats.total_stippled);
		statsptr += sprintf(statsptr, "Chro:%6d\n", m_stats.total_chroma_fail);
		statsptr += sprintf(statsptr, "ZFun:%6d\n", m_stats.total_zfunc_fail);
		statsptr += sprintf(statsptr, "AFun:%6d\n", m_stats.total_afunc_fail);
		statsptr += sprintf(statsptr, "RegW:%6d\n", m_stats.reg_writes);
		statsptr += sprintf(statsptr, "RegR:%6d\n", m_stats.reg_reads);
		statsptr += sprintf(statsptr, "LFBW:%6d\n", m_stats.lfb_writes);
		statsptr += sprintf(statsptr, "LFBR:%6d\n", m_stats.lfb_reads);
		statsptr += sprintf(statsptr, "TexW:%6d\n", m_stats.tex_writes);
		statsptr += sprintf(statsptr, "TexM:");
		for (i = 0; i < 16; i++)
			if (m_stats.texture_mode[i])
				*statsptr++ = "0123456789ABCDEF"[i];
		*statsptr = 0;
	}

	/* update statistics */
	if (DEBUG_STATS)
	{
		m_stats.stalls = 0;
		m_stats.total_triangles = 0;
		m_stats.total_pixels_in = 0;
		m_stats.total_pixels_out = 0;
		m_stats.total_chroma_fail = 0;
		m_stats.total_zfunc_fail = 0;
		m_stats.total_afunc_fail = 0;
		m_stats.total_clipped = 0;
		m_stats.total_stippled = 0;
		m_stats.reg_writes = 0;
		m_stats.reg_reads = 0;
		m_stats.lfb_writes = 0;
		m_stats.lfb_reads = 0;
		m_stats.tex_writes = 0;
		memset(m_stats.texture_mode, 0, sizeof(m_stats.texture_mode));
	}
}


void voodoo_device::adjust_vblank_timer()
{
	attotime vblank_period = m_screen->time_until_pos(m_fbi.m_vsyncstart);
	if (LOG_VBLANK_SWAP) logerror("adjust_vblank_timer: period: %s\n", vblank_period.as_string());
	/* if zero, adjust to next frame, otherwise we may get stuck in an infinite loop */
	if (vblank_period == attotime::zero)
		vblank_period = m_screen->frame_period();
	m_vsync_start_timer->adjust(vblank_period);
}


TIMER_CALLBACK_MEMBER( voodoo_device::vblank_off_callback )
{
	if (LOG_VBLANK_SWAP) logerror("--- vblank end\n");

	/* set internal state and call the client */
	m_fbi.m_vblank = false;

	// PCI Vblank IRQ enable is VOODOO2 and up
	if (m_reg.rev2_or_3())
	{
		if (m_reg.intr_ctrl().vsync_falling_enable())
		{
			m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_FALLING_GENERATED);
			if (!m_pciint.isnull())
				m_pciint(true);
		}
	}

	// External vblank handler
	if (!m_vblank.isnull())
		m_vblank(false);

	/* go to the end of the next frame */
	adjust_vblank_timer();
}


TIMER_CALLBACK_MEMBER( voodoo_device::vblank_callback )
{
	if (LOG_VBLANK_SWAP) logerror("--- vblank start\n");

	/* flush the pipes */
	if (m_pci.op_pending)
	{
		if (LOG_VBLANK_SWAP) logerror("---- vblank flush begin\n");
		flush_fifos(machine().time());
		if (LOG_VBLANK_SWAP) logerror("---- vblank flush end\n");
	}

	/* increment the count */
	m_fbi.m_vblank_count++;
	if (m_fbi.m_vblank_count > 250)
		m_fbi.m_vblank_count = 250;
	if (LOG_VBLANK_SWAP) logerror("---- vblank count = %u swap = %u pending = %u", m_fbi.m_vblank_count, m_fbi.m_vblank_swap, m_fbi.m_vblank_swap_pending);
	if (m_fbi.m_vblank_swap_pending)
		if (LOG_VBLANK_SWAP) logerror(" (target=%d)", m_fbi.m_vblank_swap);
	if (LOG_VBLANK_SWAP) logerror("\n");

	/* if we're past the swap count, do the swap */
	if (m_fbi.m_vblank_swap_pending && m_fbi.m_vblank_count >= m_fbi.m_vblank_swap)
		swap_buffers();

	/* set a timer for the next off state */
	m_vsync_stop_timer->adjust(m_screen->time_until_pos(m_fbi.m_vsyncstop));

	/* set internal state and call the client */
	m_fbi.m_vblank = true;

	// PCI Vblank IRQ enable is VOODOO2 and up
	if (m_reg.rev2_or_3())
	{
		if (m_reg.intr_ctrl().vsync_rising_enable())
		{
			m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_RISING_GENERATED);
			if (!m_pciint.isnull())
				m_pciint(true);
		}
	}

	// External vblank handler
	if (!m_vblank.isnull())
		m_vblank(true);
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

void voodoo_device::reset_counters()
{
	update_statistics(false);
	m_reg.write(voodoo_regs::reg_fbiPixelsIn, 0);
	m_reg.write(voodoo_regs::reg_fbiChromaFail, 0);
	m_reg.write(voodoo_regs::reg_fbiZfuncFail, 0);
	m_reg.write(voodoo_regs::reg_fbiAfuncFail, 0);
	m_reg.write(voodoo_regs::reg_fbiPixelsOut, 0);
}


void voodoo_device::soft_reset()
{
	reset_counters();
	m_reg.write(voodoo_regs::reg_fbiTrianglesOut, 0);
	m_fbi.m_fifo.reset();
	m_pci.fifo.reset();
}



/*************************************
 *
 *  Recompute video memory layout
 *
 *************************************/

void voodoo_device::fbi_state::recompute_video_memory(voodoo_regs &regs)
{
	// remember the front buffer configuration to check for changes
	u16 *starting_front = front_buffer();
	u32 starting_rowpix = m_rowpixels;

	// memory config is determined differently between V1 and V2
	u32 memory_config = regs.fbi_init2().enable_triple_buf();
	if (regs.rev2() && memory_config == 0)
		memory_config = regs.fbi_init5().buffer_allocation();

	// tiles are 64x16 (V1) or 32x32 (V2); x_tiles specifies how many half-tiles
	u32 xtiles = regs.fbi_init1().x_video_tiles();
	if (regs.rev2())
	{
		xtiles = (xtiles << 1) |
				(regs.fbi_init1().x_video_tiles_bit5() << 5) |
				(regs.fbi_init6().x_video_tiles_bit0());
	}
	m_rowpixels = xtiles * (regs.rev1() ? 64 : 32);

	// first RGB buffer always starts at 0
	m_rgboffs[0] = 0;

	// second RGB buffer starts immediately afterwards
	u32 const buffer_pages = regs.fbi_init2().video_buffer_offset();
	m_rgboffs[1] = buffer_pages * 0x1000;

	// remaining buffers are based on the config
	switch (memory_config)
	{
		case 3: // reserved
//			logerror("VOODOO.ERROR:Unexpected memory configuration in recompute_video_memory!\n");
			[[fallthrough]];
		case 0: // 2 color buffers, 1 aux buffer
			m_rgboffs[2] = ~0;
			m_auxoffs = 2 * buffer_pages * 0x1000;
			break;

		case 1: // 3 color buffers, 0 aux buffers
			m_rgboffs[2] = 2 * buffer_pages * 0x1000;
			m_auxoffs = ~0;
			break;

		case 2: // 3 color buffers, 1 aux buffers
			m_rgboffs[2] = 2 * buffer_pages * 0x1000;
			m_auxoffs = 3 * buffer_pages * 0x1000;
			break;
	}

	// clamp the RGB buffers to video memory
	for (int buf = 0; buf < 3; buf++)
		if (m_rgboffs[buf] != ~0 && m_rgboffs[buf] > m_mask)
			m_rgboffs[buf] = m_mask;

	// clamp the aux buffer to video memory
	if (m_auxoffs != ~0 && m_auxoffs > m_mask)
		m_auxoffs = m_mask;

/*  osd_printf_debug("rgb[0] = %08X   rgb[1] = %08X   rgb[2] = %08X   aux = %08X\n",
            m_fbi.m_rgboffs[0], m_fbi.m_rgboffs[1], m_fbi.m_rgboffs[2], m_fbi.m_auxoffs);*/

	// reset our front/back buffers if they are out of range
	if (m_rgboffs[2] == ~0)
	{
		if (m_frontbuf == 2)
			m_frontbuf = 0;
		if (m_backbuf == 2)
			m_backbuf = 0;
	}

	// mark video changed if the front buffer configuration is different
	if (front_buffer() != starting_front || m_rowpixels != starting_rowpix)
		m_video_changed = true;
}


void voodoo_device::fbi_state::recompute_fifo_layout(voodoo_regs &regs)
{
	// compute the memory FIFO location and size
	u32 fifo_last_page = regs.fbi_init4().memory_fifo_stop_row();
	if (fifo_last_page > m_mask / 0x1000)
		fifo_last_page = m_mask / 0x1000;

	// is it valid and enabled?
	u32 const fifo_start_page = regs.fbi_init4().memory_fifo_start_row();
	if (fifo_start_page <= fifo_last_page && regs.fbi_init0().enable_memory_fifo())
	{
		u32 size = std::min<u32>((fifo_last_page + 1 - fifo_start_page) * 0x1000 / 4, 65536*2);
		m_fifo.configure((u32 *)(m_ram + fifo_start_page * 0x1000), size);
	}

	/* if not, disable the FIFO */
	else
		m_fifo.configure(nullptr, 0);
}



/*************************************
 *
 *  NCC table management
 *
 *************************************/

void voodoo_device::tmu_state::ncc_w(offs_t regnum, u32 data)
{
	u32 regindex = regnum - voodoo_regs::reg_nccTable;

	// I/Q entries in NCC 0 reference the palette if the high bit is set
	if (BIT(data, 31) && regindex >= 4 && regindex < 12)
	{
		// extract the palette index
		int const index = (BIT(data, 24, 7) << 1) | BIT(regindex, 0);

		// compute RGB and ARGB values
		rgb_t rgb = 0xff000000 | data;
		rgb_t argb = argbexpand<6,6,6,6>(data, 18, 12, 6, 0);

		// set and mark dirty
		if (m_palette[0][index] != rgb)
		{
			m_palette[0][index] = rgb;
			m_palette_dirty[0] = true;
		}
		if (m_palette[1][index] != argb)
		{
			m_palette[1][index] = argb;
			m_palette_dirty[1] = true;
		}
	}

	// if no delta, don't mark dirty
	if (m_reg.read(regnum) == data)
		return;

	// write the updated data and mark dirty
	m_reg.write(regnum, data);
	m_palette_dirty[2 + regindex / 12] = true;
}



/*************************************
 *
 *  Faux DAC implementation
 *
 *************************************/

void voodoo_device::dac_state::data_w(u8 regnum, u8 data)
{
	m_reg[regnum] = data;
}


void voodoo_device::dac_state::data_r(u8 regnum)
{
	u8 result = 0xff;

	/* switch off the DAC register requested */
	switch (regnum)
	{
		case 5:
			/* this is just to make startup happy */
			switch (m_reg[7])
			{
				case 0x01:  result = 0x55; break;
				case 0x07:  result = 0x71; break;
				case 0x0b:  result = 0x79; break;
			}
			break;

		default:
			result = m_reg[regnum];
			break;
	}

	/* remember the read result; it is fetched elsewhere */
	read_result = result;
}




inline rasterizer_texture &voodoo_device::tmu_state::prepare_texture()
{
	auto &renderer = m_device.renderer();

	// if the texture parameters are dirty, update them
	if (m_regdirty)
	{
		// determine the lookup
		auto const texmode = m_reg.texture_mode();
		u32 const texformat = texmode.format();
		rgb_t const *lookup = m_texel[texformat];

		// if null lookup, then we need something dynamic
		if (lookup == nullptr)
		{
			// could be either straight palette or NCC table
			int palindex;
			if ((texformat & 7) == 1)
			{
				// NCC case: palindex = 2 or 3 based on table select
				palindex = 2 + texmode.ncc_table_select();
				if (m_palette_dirty[palindex])
				{
					u32 const *regs = m_reg.subset(voodoo_regs::reg_nccTable + 12 * (palindex & 1));
					renderer.alloc_palette(m_index * 4 + palindex).compute_ncc(regs);
				}
			}
			else
			{
				// palette case: palindex = 0 or 1 based on RGB vs RGBA
				palindex = (texformat == 6) ? 1 : 0;
				if (m_palette_dirty[palindex])
					renderer.alloc_palette(m_index * 4 + palindex).copy(&m_palette[palindex & 1][0]);
			}

			// clear the dirty flag and fetch the texels
			m_palette_dirty[palindex] = false;
			lookup = renderer.last_palette(m_index * 4 + palindex).texels();
		}

		// recompute the rasterization parameters
		renderer.alloc_texture(m_index).recompute(m_reg, m_ram, m_mask, lookup);
		m_regdirty = false;
	}
	return renderer.last_texture(m_index);
}


inline s32 voodoo_device::tmu_state::compute_lodbase()
{
	// compute (ds^2 + dt^2) in both X and Y as 28.36 numbers
	s64 texdx = s64(m_dsdx >> 14) * s64(m_dsdx >> 14) + s64(m_dtdx >> 14) * s64(m_dtdx >> 14);
	s64 texdy = s64(m_dsdy >> 14) * s64(m_dsdy >> 14) + s64(m_dtdy >> 14) * s64(m_dtdy >> 14);

	// pick whichever is larger and shift off some high bits -> 28.20
	if (texdx < texdy)
		texdx = texdy;
	texdx >>= 16;

	// use our fast reciprocal/log on this value; it expects input as a
	// 16.32 number, and returns the log of the reciprocal, so we have to
	// adjust the result: negative to get the log of the original value
	// plus 12 to account for the extra exponent, and divided by 2 to
	// get the log of the square root of texdx
	return (fast_log2(double(texdx), 0) + (12 << 8)) / 2;
}



//-------------------------------------------------
//  command_fifo - constructor
//-------------------------------------------------

command_fifo::command_fifo(voodoo_device &device) :
	m_device(device),
	m_ram(nullptr),
	m_mask(0),
	m_enable(false),
	m_count_holes(false),
	m_ram_base(0),
	m_ram_end(0),
	m_read_index(0),
	m_address_min(0),
	m_address_max(0),
	m_depth(0),
	m_holes(0)
{
}


//-------------------------------------------------
//  execute_if_ready - if we have enough data to
//  execute a command, do so; otherwise, return -1
//-------------------------------------------------

s32 command_fifo::execute_if_ready()
{
	// all CMDFIFO commands need at least one word
	if (m_depth == 0)
		return -1;

	// see if we have enough for the current command
	u32 const needed_depth = words_needed(peek_next());
	if (m_depth < needed_depth)
		return -1;

	// read the next command and handle it based on the low 3 bits
	u32 command = read_next();
	return (this->*s_packet_handler[BIT(command, 0, 3)])(command);
}


//-------------------------------------------------
//  write - handle a write to the FIFO
//-------------------------------------------------

void command_fifo::write(offs_t addr, u32 data)
{
	if (LOG_CMDFIFO_VERBOSE)
		m_device.logerror("CMDFIFO_w(%04X) = %08X\n", addr, data);

	// write the data if it's within range
	if (addr < m_ram_end)
		m_ram[(addr / 4) & m_mask] = data;

	// count holes?
	if (m_count_holes)
	{
		// in-order, no holes
		if (m_holes == 0 && addr == m_address_min + 4)
		{
			m_address_min = m_address_max = addr;
			m_depth++;
		}

		// out-of-order, below the minimum
		else if (addr < m_address_min)
		{
			if (m_holes != 0)
				m_device.logerror("Unexpected CMDFIFO: AMin=%08X AMax=%08X Holes=%d WroteTo:%08X\n", m_address_min, m_address_max, m_holes, addr);
			m_holes += (addr - m_ram_base) / 4;
			m_address_min = m_ram_base;
			m_address_max = addr;
			m_depth++;
		}

		// out-of-order, but within the min-max range
		else if (addr < m_address_max)
		{
			m_holes--;
			if (m_holes == 0)
			{
				m_depth += (m_address_max - m_address_min) / 4;
				m_address_min = m_address_max;
			}
		}

		// out-of-order, bumping max
		else
		{
			m_holes += (addr - m_address_max) / 4 - 1;
			m_address_max = addr;
		}
	}

	// execute if we can
	auto &pci = m_device.m_pci;
	if (!pci.op_pending)
	{
		s32 cycles = execute_if_ready();
		if (cycles > 0)
		{
			attotime curtime = m_device.machine().time();
			pci.op_pending = true;
			pci.op_end_time = curtime + m_device.clocks_to_attotime(cycles);

			if (LOG_FIFO_VERBOSE)
				m_device.logerror("VOODOO.FIFO:direct write start at %s end at %s\n", curtime.as_string(18), pci.op_end_time.as_string(18));
		}
	}
}


//-------------------------------------------------
//  words_needed - return the total number of
//  words needed for the given command and all its
//  parameters
//-------------------------------------------------

u32 command_fifo::words_needed(u32 command)
{
	// low 3 bits specify the packet type
	switch (BIT(command, 0, 3))
	{
		case 0:
			// Packet type 0: 1 or 2 words
			//
			//   Word  Bits
			//     0  31:29 = reserved
			//     0  28:6  = Address [24:2]
			//     0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
			//     0   2:0  = Packet type (0)
			return (BIT(command, 3, 3) == 4) ? 2 : 1;

		case 1:
			// Packet type 1: 1 + N words
			//
			//   Word  Bits
			//     0  31:16 = Number of words
			//     0    15  = Increment?
			//     0  14:3  = Register base
			//     0   2:0  = Packet type (1)
			return 1 + BIT(command, 16, 16);

		case 2:
			// Packet type 2: 1 + N words
			//
			//   Word  Bits
			//     0  31:3  = 2D Register mask
			//     0   2:0  = Packet type (2)
			return 1 + population_count_32(BIT(command, 3, 29));

		case 3:
		{
			// Packet type 3: 1 + N words
			//
			//   Word  Bits
			//     0  31:29 = Number of dummy entries following the data
			//     0   28   = Packed color data?
			//     0   25   = Disable ping pong sign correction (0=normal, 1=disable)
			//     0   24   = Culling sign (0=positive, 1=negative)
			//     0   23   = Enable culling (0=disable, 1=enable)
			//     0   22   = Strip mode (0=strip, 1=fan)
			//     0   17   = Setup S1 and T1
			//     0   16   = Setup W1
			//     0   15   = Setup S0 and T0
			//     0   14   = Setup W0
			//     0   13   = Setup Wb
			//     0   12   = Setup Z
			//     0   11   = Setup Alpha
			//     0   10   = Setup RGB
			//     0   9:6  = Number of vertices
			//     0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
			//     0   2:0  = Packet type (3)

			// determine words per vertex
			u32 count = 2;	// X/Y
			if (BIT(command, 28))
				count += (BIT(command, 10, 2) != 0) ? 1 : 0;       // ARGB in one word
			else
				count += 3 * BIT(command, 10) + BIT(command, 11);  // RGB + A
			count += BIT(command, 12);     // Z
			count += BIT(command, 13);     // Wb
			count += BIT(command, 14);     // W0
			count += 2 * BIT(command, 15); // S0/T0
			count += BIT(command, 16);     // W1
			count += 2 * BIT(command, 17); // S1/T1

			// multiply by the number of verticies
			count *= BIT(command, 6, 4);
			return 1 + count + BIT(command, 29, 3);
		}

		case 4:
			// Packet type 4: 1 + N words
			//
			//   Word  Bits
			//     0  31:29 = Number of dummy entries following the data
			//     0  28:15 = General register mask
			//     0  14:3  = Register base
			//     0   2:0  = Packet type (4)
			return 1 + population_count_32(BIT(command, 15, 14)) + BIT(command, 29, 3);

		case 5:
			// Packet type 5: 2 + N words
			//
			//	Word  Bits
			//    0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
			//    0  29:26 = Byte disable W2
			//    0  25:22 = Byte disable WN
			//    0  21:3  = Num words
			//    0   2:0  = Packet type (5)
			return 2 + BIT(command, 3, 19);

		default:
			osd_printf_debug("UNKNOWN PACKET TYPE %d\n", command & 7);
			return 1;
	}
}


//-------------------------------------------------
//  packet_type_0 - handle FIFO packet type 0
//-------------------------------------------------

u32 command_fifo::packet_type_0(u32 command)
{
	// Packet type 0: 1 or 2 words
	//
	//   Word  Bits
	//     0  31:29 = reserved
	//     0  28:6  = Address [24:2]
	//     0   5:3  = Function (0 = NOP, 1 = JSR, 2 = RET, 3 = JMP LOCAL, 4 = JMP AGP)
	//     0   2:0  = Packet type (0)
	//     1  31:11 = reserved (JMP AGP only)
	//     1  10:0  = Address [35:25]
	u32 target = BIT(command, 6, 23) << 2;

	// switch off of the specific command; many are unimplemented until we
	// see them in real life
	switch (BIT(command, 3, 3))
	{
		case 0:     // NOP
			if (LOG_CMDFIFO)
				m_device.logerror("  NOP\n");
			break;

		case 1:     // JSR
			if (LOG_CMDFIFO)
				m_device.logerror("  JSR $%06X\n", target);
			fatalerror("JSR in CMDFIFO!\n");
			break;

		case 2:     // RET
			if (LOG_CMDFIFO)
				m_device.logerror("  RET $%06X\n", target);
			fatalerror("RET in CMDFIFO!\n");
			break;

		case 3:     // JMP LOCAL FRAME BUFFER
			if (LOG_CMDFIFO)
				m_device.logerror("  JMP LOCAL FRAMEBUF $%06X\n", target);
			m_read_index = target / 4;
			break;

		case 4:     // JMP AGP
			if (LOG_CMDFIFO)
				m_device.logerror("  JMP AGP $%06X\n", target);
			fatalerror("JMP AGP in CMDFIFO!\n");
			break;

		default:
			fatalerror("  INVALID JUMP COMMAND\n");
			break;
	}
	return 0;
}


//-------------------------------------------------
//  packet_type_1 - handle FIFO packet type 1
//-------------------------------------------------

u32 command_fifo::packet_type_1(u32 command)
{
	// Packet type 1: 1 + N words
	//
	//   Word  Bits
	//     0  31:16 = Number of words
	//     0    15  = Increment?
	//     0  14:3  = Register base
	//     0   2:0  = Packet type (1)
	//     1  31:0  = Data word
	u32 count = BIT(command, 16, 16);
	u32 inc = BIT(command, 15);
	u32 target = BIT(command, 3, 12);

	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 1: count=%d inc=%d reg=%04X\n", count, inc, target);

	// loop over all registers and write them one at a time
	u32 cycles = 0;
	if (m_device.model() >= MODEL_VOODOO_BANSHEE && BIT(target, 11))
	{
		for (u32 regbit = 0; regbit < count; regbit++, target += inc)
			cycles += m_device.banshee_2d_w(target & 0xff, read_next());
	}
	else
	{
		for (u32 regbit = 0; regbit < count; regbit++, target += inc)
			cycles += m_device.register_w(target, read_next());
	}
	return cycles;
}


//-------------------------------------------------
//  packet_type_2 - handle FIFO packet type 2
//-------------------------------------------------

u32 command_fifo::packet_type_2(u32 command)
{
	// Packet type 2: 1 + N words
	//
	//   Word  Bits
	//     0  31:3  = 2D Register mask
	//     0   2:0  = Packet type (2)
	//     1  31:0  = Data word
	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 2: mask=%X\n", BIT(command, 3, 29));

	// loop over all registers and write them one at a time
	u32 cycles = 0;
	if (m_device.model() >= MODEL_VOODOO_BANSHEE)
	{
		for (u32 regbit = 3; regbit <= 31; regbit++)
			if (BIT(command, regbit))
				cycles += m_device.banshee_2d_w(banshee2D_clip0Min + (regbit - 3), read_next());
	}
	else
	{
		for (u32 regbit = 3; regbit <= 31; regbit++)
			if (BIT(command, regbit))
				cycles += m_device.register_w(voodoo_regs::reg_bltSrcBaseAddr + (regbit - 3), read_next());
	}
	return cycles;
}


//-------------------------------------------------
//  packet_type_3 - handle FIFO packet type 3
//-------------------------------------------------

u32 command_fifo::packet_type_3(u32 command)
{
	// Packet type 3: 1 + N words
	//
	//   Word  Bits
	//     0  31:29 = Number of dummy entries following the data
	//     0   28   = Packed color data?
	//     0   25   = Disable ping pong sign correction (0=normal, 1=disable)
	//     0   24   = Culling sign (0=positive, 1=negative)
	//     0   23   = Enable culling (0=disable, 1=enable)
	//     0   22   = Strip mode (0=strip, 1=fan)
	//     0   17   = Setup S1 and T1
	//     0   16   = Setup W1
	//     0   15   = Setup S0 and T0
	//     0   14   = Setup W0
	//     0   13   = Setup Wb
	//     0   12   = Setup Z
	//     0   11   = Setup Alpha
	//     0   10   = Setup RGB
	//     0   9:6  = Number of vertices
	//     0   5:3  = Command (0=Independent tris, 1=Start new strip, 2=Continue strip)
	//     0   2:0  = Packet type (3)
	//     1  31:0  = Data word
	u32 count = BIT(command, 6, 4);
	u32 code = BIT(command, 3, 3);

	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 3: count=%d code=%d mask=%03X smode=%02X pc=%d\n", count, code, BIT(command, 10, 12), BIT(command, 22, 6), BIT(command, 28));

	// copy relevant bits into the setup mode register
	m_device.m_reg.write(voodoo_regs::reg_sSetupMode, BIT(command, 10, 8) | (BIT(command, 22, 4) << 16));

	// loop over triangles
	voodoo_device::fbi_state::setup_vertex svert = { 0 };
	u32 cycles = 0;
	for (u32 trinum = 0; trinum < count; trinum++)
	{
		// always extract X/Y
		svert.x = read_next_float();
		svert.y = read_next_float();

		// load ARGB values
		if (BIT(command, 28))
		{
			// packed form
			if (BIT(command, 10, 2) != 0)
			{
				rgb_t argb = read_next();
				if (BIT(command, 10))
				{
					svert.r = argb.r();
					svert.g = argb.g();
					svert.b = argb.b();
				}
				if (BIT(command, 11))
					svert.a = argb.a();
			}
		}
		else
		{
			// unpacked form
			if (BIT(command, 10))
			{
				svert.r = read_next_float();
				svert.g = read_next_float();
				svert.b = read_next_float();
			}
			if (BIT(command, 11))
				svert.a = read_next_float();
		}

		// load Z and Wb values
		if (BIT(command, 12))
			svert.z = read_next_float();
		if (BIT(command, 13))
			svert.wb = svert.w0 = svert.w1 = read_next_float();

		// load W0, S0, T0 values
		if (BIT(command, 14))
			svert.w0 = svert.w1 = read_next_float();
		if (BIT(command, 15))
		{
			svert.s0 = svert.s1 = read_next_float();
			svert.t0 = svert.t1 = read_next_float();
		}

		// load W1, S1, T1 values
		if (BIT(command, 16))
			svert.w1 = read_next_float();
		if (BIT(command, 17))
		{
			svert.s1 = read_next_float();
			svert.t1 = read_next_float();
		}

		// if we're starting a new strip, or if this is the first of a set of verts
		// for a series of individual triangles, initialize all the verts
		auto &fbi = m_device.m_fbi;
		if ((code == 1 && trinum == 0) || (code == 0 && trinum % 3 == 0))
		{
			fbi.m_sverts = 1;
			fbi.m_svert[0] = fbi.m_svert[1] = fbi.m_svert[2] = svert;
		}

		// otherwise, add this to the list
		else
		{
			// for strip mode, shuffle vertex 1 down to 0
			if (!BIT(command, 22))
				fbi.m_svert[0] = fbi.m_svert[1];

			// copy 2 down to 1 and add our new one regardless
			fbi.m_svert[1] = fbi.m_svert[2];
			fbi.m_svert[2] = svert;

			// if we have enough, draw
			if (++fbi.m_sverts >= 3)
				cycles += m_device.setup_and_draw_triangle();
		}
	}

	// account for the extra dummy words
	consume(BIT(command, 29, 3));
	return cycles;
}


//-------------------------------------------------
//  packet_type_4 - handle FIFO packet type 4
//-------------------------------------------------

u32 command_fifo::packet_type_4(u32 command)
{
	// Packet type 4: 1 + N words
	//
	//   Word  Bits
	//     0  31:29 = Number of dummy entries following the data
	//     0  28:15 = General register mask
	//     0  14:3  = Register base
	//     0   2:0  = Packet type (4)
	//     1  31:0  = Data word
	u32 target = BIT(command, 3, 12);

	if (LOG_CMDFIFO)
		m_device.logerror("  PACKET TYPE 4: mask=%X reg=%04X pad=%d\n", BIT(command, 15, 14), target, BIT(command, 29, 3));

	// loop over all registers and write them one at a time
	u32 cycles = 0;
	if (m_device.model() >= MODEL_VOODOO_BANSHEE && BIT(target, 11))
	{
		for (u32 regbit = 15; regbit <= 28; regbit++, target++)
			if (BIT(command, regbit))
				cycles += m_device.banshee_2d_w(target & 0xff, read_next());
	}
	else
	{
		for (u32 regbit = 15; regbit <= 28; regbit++, target++)
			if (BIT(command, regbit))
				cycles += m_device.register_w(target, read_next());
	}

	// account for the extra dummy words
	consume(BIT(command, 29, 3));
	return cycles;
}


//-------------------------------------------------
//  packet_type_5 - handle FIFO packet type 5
//-------------------------------------------------

u32 command_fifo::packet_type_5(u32 command)
{
	// Packet type 5: 2 + N words
	//
	//	Word  Bits
	//    0  31:30 = Space (0,1=reserved, 2=LFB, 3=texture)
	//    0  29:26 = Byte disable W2
	//    0  25:22 = Byte disable WN
	//    0  21:3  = Num words
	//    0   2:0  = Packet type (5)
	//    1  31:30 = Reserved
	//    1  29:0  = Base address [24:0]
	//    2  31:0  = Data word
	u32 count = BIT(command, 3, 19);
	u32 target = read_next() / 4;

	// handle LFB writes
	u32 cycles = 0;
	switch (BIT(command, 30, 2))
	{
		// Linear FB
		case 0:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: FB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				m_ram[target++ & m_mask] = little_endianize_int32(read_next());
			cycles = count;
			break;

		// 3D LFB
		case 2:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: 3D LFB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				cycles += m_device.lfb_w(target++, read_next(), 0xffffffff);
			break;

		// Planar YUV
		case 1:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: Planar YUV count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			fatalerror("Unimplemented YUV update in Voodoo");
			break;

		// Texture port
		case 3:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: textureRAM count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				cycles += m_device.texture_w(target++, read_next());
			break;
	}
	return cycles;
}


//-------------------------------------------------
//  packet_type_unknown - error out on unhandled
//  packets
//-------------------------------------------------

u32 command_fifo::packet_type_unknown(u32 command)
{
	fatalerror("Unhandled cmdFifo packet type %d\n", BIT(command, 0, 3));
}


command_fifo::packet_handler command_fifo::s_packet_handler[8] =
{
	&command_fifo::packet_type_0,
	&command_fifo::packet_type_1,
	&command_fifo::packet_type_2,
	&command_fifo::packet_type_3,
	&command_fifo::packet_type_4,
	&command_fifo::packet_type_5,
	&command_fifo::packet_type_unknown,
	&command_fifo::packet_type_unknown
};


/*************************************
 *
 *  Stall the active cpu until we are
 *  ready
 *
 *************************************/

TIMER_CALLBACK_MEMBER( voodoo_device::stall_cpu_callback )
{
	check_stalled_cpu(machine().time());
}


void voodoo_device::check_stalled_cpu(attotime current_time)
{
	int resume = false;

	/* flush anything we can */
	if (m_pci.op_pending)
		flush_fifos(current_time);

	/* if we're just stalled until the LWM is passed, see if we're ok now */
	if (m_pci.stall_state == STALLED_UNTIL_FIFO_LWM)
	{
		/* if there's room in the memory FIFO now, we can proceed */
		if (m_reg.fbi_init0().enable_memory_fifo())
		{
			if (m_fbi.m_fifo.items() < 2 * 32 * m_reg.fbi_init0().memory_fifo_hwm())
				resume = true;
		}
		else if (m_pci.fifo.space() > 2 * m_reg.fbi_init0().pci_fifo_lwm())
			resume = true;
	}

	/* if we're stalled until the FIFOs are empty, check now */
	else if (m_pci.stall_state == STALLED_UNTIL_FIFO_EMPTY)
	{
		if (m_reg.fbi_init0().enable_memory_fifo())
		{
			if (m_fbi.m_fifo.empty() && m_pci.fifo.empty())
				resume = true;
		}
		else if (m_pci.fifo.empty())
			resume = true;
	}

	/* resume if necessary */
	if (resume || !m_pci.op_pending)
	{
		if (LOG_FIFO) logerror("VOODOO.FIFO:Stall condition cleared; resuming\n");
		m_pci.stall_state = NOT_STALLED;

		/* either call the callback, or trigger the trigger */
		if (!m_stall.isnull())
			m_stall(false);
		else
			machine().scheduler().trigger(m_trigger);
	}

	/* if not, set a timer for the next one */
	else
	{
		m_pci.continue_timer->adjust(m_pci.op_end_time - current_time);
	}
}


void voodoo_device::stall_cpu(int state, attotime current_time)
{
	/* sanity check */
	if (!m_pci.op_pending) fatalerror("FIFOs not empty, no op pending!\n");

	/* set the state and update statistics */
	m_pci.stall_state = state;
	if (DEBUG_STATS)
		m_stats.stalls++;

	/* either call the callback, or spin the CPU */
	if (!m_stall.isnull())
		m_stall(true);
	else
		m_cpu->spin_until_trigger(m_trigger);

	/* set a timer to clear the stall */
	m_pci.continue_timer->adjust(m_pci.op_end_time - current_time);
}



/*************************************
 *
 *  Voodoo register writes
 *
 *************************************/

s32 voodoo_device::register_w(offs_t offset, u32 data)
{
	u32 origdata = data;
	s32 cycles = 0;
	s64 data64;

	// statistics
	if (DEBUG_STATS)
		m_stats.reg_writes++;

	// determine which chips we are addressing
	u32 chips = BIT(offset, 8, 4);
	if (chips == 0)
		chips = 0xf;
	chips &= m_chipmask;

	// the first 64 registers can be aliased differently
	u32 regnum = BIT(offset, 19) ? m_reg.alias(offset) : BIT(offset, 0, 8);

	// first make sure this register is writable */
	if (!m_reg.is_writable(regnum))
	{
		logerror("VOODOO.ERROR:Invalid attempt to write %s\n", m_reg.name(regnum));
		return 0;
	}

	// switch off the register
	switch (regnum)
	{
		case voodoo_regs::reg_intrCtrl:
			m_reg.write(regnum, data);

			// Setting bit 31 clears the PCI interrupts
			if (BIT(data, 31) && !m_pciint.isnull())
				m_pciint(false);
			break;

		// Vertex data is 12.4 formatted fixed point
		case voodoo_regs::reg_fvertexAx:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexAx:
			if (BIT(chips, 0)) m_fbi.m_ax = s16(data);
			break;

		case voodoo_regs::reg_fvertexAy:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexAy:
			if (BIT(chips, 0)) m_fbi.m_ay = s16(data);
			break;

		case voodoo_regs::reg_fvertexBx:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexBx:
			if (BIT(chips, 0)) m_fbi.m_bx = s16(data);
			break;

		case voodoo_regs::reg_fvertexBy:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexBy:
			if (BIT(chips, 0)) m_fbi.m_by = s16(data);
			break;

		case voodoo_regs::reg_fvertexCx:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexCx:
			if (BIT(chips, 0)) m_fbi.m_cx = s16(data);
			break;

		case voodoo_regs::reg_fvertexCy:
			data = float_to_int32(data, 4);
			[[fallthrough]];
		case voodoo_regs::reg_vertexCy:
			if (BIT(chips, 0)) m_fbi.m_cy = s16(data);
			break;

		// RGB data is 12.12 formatted fixed point
		case voodoo_regs::reg_fstartR:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startR:
			if (BIT(chips, 0)) m_fbi.m_startr = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fstartG:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startG:
			if (BIT(chips, 0)) m_fbi.m_startg = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fstartB:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startB:
			if (BIT(chips, 0)) m_fbi.m_startb = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fstartA:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startA:
			if (BIT(chips, 0)) m_fbi.m_starta = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdRdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dRdX:
			if (BIT(chips, 0)) m_fbi.m_drdx = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdGdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dGdX:
			if (BIT(chips, 0)) m_fbi.m_dgdx = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdBdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dBdX:
			if (BIT(chips, 0)) m_fbi.m_dbdx = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdAdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dAdX:
			if (BIT(chips, 0)) m_fbi.m_dadx = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdRdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dRdY:
			if (BIT(chips, 0)) m_fbi.m_drdy = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdGdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dGdY:
			if (BIT(chips, 0)) m_fbi.m_dgdy = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdBdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dBdY:
			if (BIT(chips, 0)) m_fbi.m_dbdy = s32(data << 8) >> 8;
			break;

		case voodoo_regs::reg_fdAdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dAdY:
			if (BIT(chips, 0)) m_fbi.m_dady = s32(data << 8) >> 8;
			break;

		// Z data is 20.12 formatted fixed point
		case voodoo_regs::reg_fstartZ:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_startZ:
			if (BIT(chips, 0)) m_fbi.m_startz = s32(data);
			break;

		case voodoo_regs::reg_fdZdX:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dZdX:
			if (BIT(chips, 0)) m_fbi.m_dzdx = s32(data);
			break;

		case voodoo_regs::reg_fdZdY:
			data = float_to_int32(data, 12);
			[[fallthrough]];
		case voodoo_regs::reg_dZdY:
			if (BIT(chips, 0)) m_fbi.m_dzdy = s32(data);
			break;

		// S,T data is 14.18 formatted fixed point, converted to 16.32 internally
		case voodoo_regs::reg_fstartS:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 1)) m_tmu[0].m_starts = data64;
			if (BIT(chips, 2)) m_tmu[1].m_starts = data64;
			break;
		case voodoo_regs::reg_startS:
			if (BIT(chips, 1)) m_tmu[0].m_starts = s64(s32(data)) << 14;
			if (BIT(chips, 2)) m_tmu[1].m_starts = s64(s32(data)) << 14;
			break;

		case voodoo_regs::reg_fstartT:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 1)) m_tmu[0].m_startt = data64;
			if (BIT(chips, 2)) m_tmu[1].m_startt = data64;
			break;
		case voodoo_regs::reg_startT:
			if (BIT(chips, 1)) m_tmu[0].m_startt = s64(s32(data)) << 14;
			if (BIT(chips, 2)) m_tmu[1].m_startt = s64(s32(data)) << 14;
			break;

		case voodoo_regs::reg_fdSdX:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 1)) m_tmu[0].m_dsdx = data64;
			if (BIT(chips, 2)) m_tmu[1].m_dsdx = data64;
			break;
		case voodoo_regs::reg_dSdX:
			if (BIT(chips, 1)) m_tmu[0].m_dsdx = s64(s32(data)) << 14;
			if (BIT(chips, 2)) m_tmu[1].m_dsdx = s64(s32(data)) << 14;
			break;

		case voodoo_regs::reg_fdTdX:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 1)) m_tmu[0].m_dtdx = data64;
			if (BIT(chips, 2)) m_tmu[1].m_dtdx = data64;
			break;
		case voodoo_regs::reg_dTdX:
			if (BIT(chips, 1)) m_tmu[0].m_dtdx = s64(s32(data)) << 14;
			if (BIT(chips, 2)) m_tmu[1].m_dtdx = s64(s32(data)) << 14;
			break;

		case voodoo_regs::reg_fdSdY:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 1)) m_tmu[0].m_dsdy = data64;
			if (BIT(chips, 2)) m_tmu[1].m_dsdy = data64;
			break;
		case voodoo_regs::reg_dSdY:
			if (BIT(chips, 1)) m_tmu[0].m_dsdy = s64(s32(data)) << 14;
			if (BIT(chips, 2)) m_tmu[1].m_dsdy = s64(s32(data)) << 14;
			break;

		case voodoo_regs::reg_fdTdY:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 1)) m_tmu[0].m_dtdy = data64;
			if (BIT(chips, 2)) m_tmu[1].m_dtdy = data64;
			break;
		case voodoo_regs::reg_dTdY:
			if (BIT(chips, 1)) m_tmu[0].m_dtdy = s64(s32(data)) << 14;
			if (BIT(chips, 2)) m_tmu[1].m_dtdy = s64(s32(data)) << 14;
			break;

		// W data is 2.30 formatted fixed point, converted to 16.32 internally
		case voodoo_regs::reg_fstartW:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 0)) m_fbi.m_startw = data64;
			if (BIT(chips, 1)) m_tmu[0].m_startw = data64;
			if (BIT(chips, 2)) m_tmu[1].m_startw = data64;
			break;
		case voodoo_regs::reg_startW:
			if (BIT(chips, 0)) m_fbi.m_startw = s64(s32(data)) << 2;
			if (BIT(chips, 1)) m_tmu[0].m_startw = s64(s32(data)) << 2;
			if (BIT(chips, 2)) m_tmu[1].m_startw = s64(s32(data)) << 2;
			break;

		case voodoo_regs::reg_fdWdX:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 0)) m_fbi.m_dwdx = data64;
			if (BIT(chips, 1)) m_tmu[0].m_dwdx = data64;
			if (BIT(chips, 2)) m_tmu[1].m_dwdx = data64;
			break;
		case voodoo_regs::reg_dWdX:
			if (BIT(chips, 0)) m_fbi.m_dwdx = s64(s32(data)) << 2;
			if (BIT(chips, 1)) m_tmu[0].m_dwdx = s64(s32(data)) << 2;
			if (BIT(chips, 2)) m_tmu[1].m_dwdx = s64(s32(data)) << 2;
			break;

		case voodoo_regs::reg_fdWdY:
			data64 = float_to_int64(data, 32);
			if (BIT(chips, 0)) m_fbi.m_dwdy = data64;
			if (BIT(chips, 1)) m_tmu[0].m_dwdy = data64;
			if (BIT(chips, 2)) m_tmu[1].m_dwdy = data64;
			break;
		case voodoo_regs::reg_dWdY:
			if (BIT(chips, 0)) m_fbi.m_dwdy = s64(s32(data)) << 2;
			if (BIT(chips, 1)) m_tmu[0].m_dwdy = s64(s32(data)) << 2;
			if (BIT(chips, 2)) m_tmu[1].m_dwdy = s64(s32(data)) << 2;
			break;

		// setup bits
		case voodoo_regs::reg_sARGB:
			if (BIT(chips, 0))
			{
				rgb_t rgbdata(data);
				m_reg.write_float(voodoo_regs::reg_sAlpha, rgbdata.a());
				m_reg.write_float(voodoo_regs::reg_sRed, rgbdata.r());
				m_reg.write_float(voodoo_regs::reg_sGreen, rgbdata.g());
				m_reg.write_float(voodoo_regs::reg_sBlue, rgbdata.b());
			}
			break;

		// mask off invalid bits for different cards
		case voodoo_regs::reg_fbzColorPath:
			if (m_reg.rev1())
				data &= 0x0fffffff;
			if (BIT(chips, 0)) m_reg.write(regnum, data);
			break;

		case voodoo_regs::reg_fbzMode:
			if (m_reg.rev1())
				data &= 0x001fffff;
			if (BIT(chips, 0)) m_reg.write(regnum, data);
			break;

		case voodoo_regs::reg_fogMode:
			if (m_reg.rev1())
				data &= 0x0000003f;
			if (BIT(chips, 0)) m_reg.write(regnum, data);
			break;

		// triangle drawing
		case voodoo_regs::reg_triangleCMD:
			m_fbi.m_sign = data;
			cycles = triangle();
			break;

		case voodoo_regs::reg_ftriangleCMD:
			m_fbi.m_sign = data;
			cycles = triangle();
			break;

		case voodoo_regs::reg_sBeginTriCMD:
			cycles = begin_triangle();
			break;

		case voodoo_regs::reg_sDrawTriCMD:
			cycles = draw_triangle();
			break;

		// other commands
		case voodoo_regs::reg_nopCMD:
			m_renderer->wait(m_reg.name(regnum));
			if (BIT(data, 0))
				reset_counters();
			if (BIT(data, 1))
				m_reg.write(voodoo_regs::reg_fbiTrianglesOut, 0);
			break;

		case voodoo_regs::reg_fastfillCMD:
			cycles = fastfill();
			break;

		case voodoo_regs::reg_swapbufferCMD:
			m_renderer->wait(m_reg.name(regnum));
			cycles = swapbuffer(data);
			break;

		case voodoo_regs::reg_userIntrCMD:
			m_renderer->wait(m_reg.name(regnum));

			// Bit 5 of intrCtrl enables user interrupts
			if (m_reg.intr_ctrl().user_interrupt_enable())
			{
				// Bits 19:12 are set to cmd 9:2, bit 11 is user interrupt flag
				m_reg.clear_set(voodoo_regs::reg_intrCtrl,
					reg_intr_ctrl::EXTERNAL_PIN_ACTIVE | reg_intr_ctrl::USER_INTERRUPT_TAG_MASK,
					((data << 10) & reg_intr_ctrl::USER_INTERRUPT_TAG_MASK) | reg_intr_ctrl::USER_INTERRUPT_GENERATED);

				// Signal pci interrupt handler
				if (!m_pciint.isnull())
					m_pciint(true);
			}
			break;

		// gamma table access -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_clutData:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0))
			{
				m_renderer->wait(m_reg.name(regnum));
				if (m_reg.fbi_init1().video_timing_reset() == 0)
				{
					int index = BIT(data, 24, 8);
					if (index <= 32)
					{
						m_fbi.m_clut[index] = data;
						m_fbi.m_clut_dirty = true;
					}
				}
				else
					logerror("clutData ignored because video timing reset = 1\n");
			}
			break;

		// external DAC access -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_dacData:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0))
			{
				if (!BIT(data, 11))
					m_dac.data_w(BIT(data, 8, 3), BIT(data, 0, 8));
				else
					m_dac.data_r(BIT(data, 8, 3));
			}
			break;

		// vertical sync rate -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_hSync:
		case voodoo_regs::reg_vSync:
		case voodoo_regs::reg_backPorch:
		case voodoo_regs::reg_videoDimensions:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0))
			{
				m_renderer->wait(m_reg.name(regnum));
				m_reg.write(regnum, data);
				m_fbi.recompute_screen_params(m_reg, *m_screen);
				adjust_vblank_timer();
			}
			break;

		// fbiInit0 can only be written if initEnable says we can -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_fbiInit0:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
			{
				m_renderer->wait(m_reg.name(regnum));
				m_reg.write(regnum, data);
				if (m_reg.fbi_init0().graphics_reset())
					soft_reset();
				if (m_reg.fbi_init0().fifo_reset())
					m_pci.fifo.reset();
				m_fbi.recompute_fifo_layout(m_reg);
			}
			break;

		// fbiInit1/2 can only be written if initEnable says we can -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_fbiInit1:
		case voodoo_regs::reg_fbiInit2:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
			{
				m_renderer->wait(m_reg.name(regnum));
				m_reg.write(regnum, data);
				m_fbi.recompute_video_memory(m_reg);
				m_renderer->set_rowpixels(m_fbi.m_rowpixels);
			}
			break;

		// fbiInit3 can only be written if initEnable says we can -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_fbiInit3:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
			{
				m_reg.write(regnum, data);
				m_renderer->set_yorigin(m_fbi.m_yorigin = m_reg.fbi_init3().yorigin_subtract());
			}
			break;

		// fbiInit4 can only be written if initEnable says we can -- Voodoo/Voodoo2 only
		case voodoo_regs::reg_fbiInit4:
			if (m_reg.rev3())
				break;
			if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
			{
				m_reg.write(regnum, data);
				m_fbi.recompute_fifo_layout(m_reg);
			}
			break;

		// fbiInit5 can only be written if initEnable says we can -- Voodoo2 only
		case voodoo_regs::reg_fbiInit5:
			if (!m_reg.rev2())
				break;
			if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
				m_reg.write(regnum, data);
			break;

		// fbiInit6 can only be written if initEnable says we can -- Voodoo2 only
		case voodoo_regs::reg_fbiInit6:
			if (!m_reg.rev2())
				break;
			if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
			{
				m_renderer->wait(m_reg.name(regnum));
				m_reg.write(regnum, data);
				m_fbi.recompute_video_memory(m_reg);
				m_renderer->set_rowpixels(m_fbi.m_rowpixels);
			}
			break;

		// fbiInit7 can only be written if initEnable says we can -- Voodoo2 only
		// swapPending -- Banshee only
		case voodoo_regs::reg_fbiInit7:
			if (m_reg.rev1())
				break;
			if (m_reg.rev2())
			{
				if (BIT(chips, 0) && m_pci.init_enable.enable_hw_init())
				{
					m_reg.write(regnum, data);
					m_fbi.m_cmdfifo[0].set_enable(m_reg.fbi_init7().cmdfifo_enable());
					m_fbi.m_cmdfifo[0].set_count_holes(!m_reg.fbi_init7().disable_cmdfifo_holes());
				}
			}
			else
				m_fbi.m_swaps_pending++;
			break;

		// cmdFifo* -- Voodoo2 only
		case voodoo_regs::reg_cmdFifoBaseAddr:
			if (!m_reg.rev2())
				break;
			if (BIT(chips, 0))
			{
				m_renderer->wait(m_reg.name(regnum));
				m_reg.write(regnum, data);
				m_fbi.m_cmdfifo[0].set_base(BIT(data, 0, 10) << 12);
				m_fbi.m_cmdfifo[0].set_end((BIT(data, 16, 10) + 1) << 12);
			}
			break;

		case voodoo_regs::reg_cmdFifoBump:
			if (!m_reg.rev2())
				break;
			if (BIT(chips, 0))
				fatalerror("cmdFifoBump\n");
			break;

		case voodoo_regs::reg_cmdFifoRdPtr:
			if (!m_reg.rev2())
				break;
			if (BIT(chips, 0))
				m_fbi.m_cmdfifo[0].set_read_pointer(data);
			break;

		// this register is repurposed as reg_colBufferAddr on Banshee
		case voodoo_regs::reg_cmdFifoAMin:
			if (m_reg.rev1())
				break;
			if (BIT(chips, 0))
			{
				if (m_reg.rev2())
					m_fbi.m_cmdfifo[0].set_address_min(data);
				else
					m_fbi.m_rgboffs[1] = data & m_fbi.m_mask & ~0x0f;
			}
			break;

		// this register is repurposed as reg_colBufferStride on Banshee
		case voodoo_regs::reg_cmdFifoAMax:
			if (m_reg.rev1())
				break;
			if (BIT(chips, 0))
			{
				if (m_reg.rev2())
					m_fbi.m_cmdfifo[0].set_address_max(data);
				else
				{
					u32 newpix = BIT(data, 15) ? (BIT(data, 0, 7) << 6) : (BIT(data, 0, 14) >> 1);
					if (newpix != m_fbi.m_rowpixels)
					{
						m_renderer->set_rowpixels(m_fbi.m_rowpixels = newpix);
						m_fbi.m_video_changed = true;
					}
				}
			}
			break;

		// this register is repurposed as reg_auxBufferAddr on Banshee
		case voodoo_regs::reg_cmdFifoDepth:
			if (m_reg.rev1())
				break;
			if (BIT(chips, 0))
			{
				if (m_reg.rev2())
					m_fbi.m_cmdfifo[0].set_depth(data);
				else
					m_fbi.m_auxoffs = data & m_fbi.m_mask & ~0x0f;
			}
			break;

		// this register is repurposed as reg_auxBufferStride on Banshee
		case voodoo_regs::reg_cmdFifoHoles:
			if (m_reg.rev1())
				break;
			if (BIT(chips, 0))
			{
				if (m_reg.rev2())
					m_fbi.m_cmdfifo[0].set_holes(data);
				else
				{
					int rowpixels = BIT(data, 15) ? (BIT(data, 0, 7) << 6) : (BIT(data, 0, 14) >> 1);
					if (rowpixels != m_fbi.m_rowpixels)
						fatalerror("aux buffer stride differs from color buffer stride\n");
				}
			}
			break;

		// nccTable entries are processed and expanded immediately
		case voodoo_regs::reg_nccTable + 0:
		case voodoo_regs::reg_nccTable + 1:
		case voodoo_regs::reg_nccTable + 2:
		case voodoo_regs::reg_nccTable + 3:
		case voodoo_regs::reg_nccTable + 4:
		case voodoo_regs::reg_nccTable + 5:
		case voodoo_regs::reg_nccTable + 6:
		case voodoo_regs::reg_nccTable + 7:
		case voodoo_regs::reg_nccTable + 8:
		case voodoo_regs::reg_nccTable + 9:
		case voodoo_regs::reg_nccTable + 10:
		case voodoo_regs::reg_nccTable + 11:
		case voodoo_regs::reg_nccTable + 12:
		case voodoo_regs::reg_nccTable + 13:
		case voodoo_regs::reg_nccTable + 14:
		case voodoo_regs::reg_nccTable + 15:
		case voodoo_regs::reg_nccTable + 16:
		case voodoo_regs::reg_nccTable + 17:
		case voodoo_regs::reg_nccTable + 18:
		case voodoo_regs::reg_nccTable + 19:
		case voodoo_regs::reg_nccTable + 20:
		case voodoo_regs::reg_nccTable + 21:
		case voodoo_regs::reg_nccTable + 22:
		case voodoo_regs::reg_nccTable + 23:
			if (BIT(chips, 1)) m_tmu[0].ncc_w(regnum, data);
			if (BIT(chips, 2)) m_tmu[1].ncc_w(regnum, data);
			break;

		// fogTable entries are processed and expanded immediately
		case voodoo_regs::reg_fogTable + 0:
		case voodoo_regs::reg_fogTable + 1:
		case voodoo_regs::reg_fogTable + 2:
		case voodoo_regs::reg_fogTable + 3:
		case voodoo_regs::reg_fogTable + 4:
		case voodoo_regs::reg_fogTable + 5:
		case voodoo_regs::reg_fogTable + 6:
		case voodoo_regs::reg_fogTable + 7:
		case voodoo_regs::reg_fogTable + 8:
		case voodoo_regs::reg_fogTable + 9:
		case voodoo_regs::reg_fogTable + 10:
		case voodoo_regs::reg_fogTable + 11:
		case voodoo_regs::reg_fogTable + 12:
		case voodoo_regs::reg_fogTable + 13:
		case voodoo_regs::reg_fogTable + 14:
		case voodoo_regs::reg_fogTable + 15:
		case voodoo_regs::reg_fogTable + 16:
		case voodoo_regs::reg_fogTable + 17:
		case voodoo_regs::reg_fogTable + 18:
		case voodoo_regs::reg_fogTable + 19:
		case voodoo_regs::reg_fogTable + 20:
		case voodoo_regs::reg_fogTable + 21:
		case voodoo_regs::reg_fogTable + 22:
		case voodoo_regs::reg_fogTable + 23:
		case voodoo_regs::reg_fogTable + 24:
		case voodoo_regs::reg_fogTable + 25:
		case voodoo_regs::reg_fogTable + 26:
		case voodoo_regs::reg_fogTable + 27:
		case voodoo_regs::reg_fogTable + 28:
		case voodoo_regs::reg_fogTable + 29:
		case voodoo_regs::reg_fogTable + 30:
		case voodoo_regs::reg_fogTable + 31:
			if (BIT(chips, 0))
				m_renderer->write_fog(2 * (regnum - voodoo_regs::reg_fogTable), data);
			break;

		// texture modifications cause us to recompute everything
		case voodoo_regs::reg_textureMode:
		case voodoo_regs::reg_tLOD:
		case voodoo_regs::reg_tDetail:
		case voodoo_regs::reg_texBaseAddr:
		case voodoo_regs::reg_texBaseAddr_1:
		case voodoo_regs::reg_texBaseAddr_2:
		case voodoo_regs::reg_texBaseAddr_3_8:
			if (BIT(chips, 1))
			{
				if (m_tmu[0].m_reg.read(regnum) != data)
				{
					m_tmu[0].m_regdirty = true;
					m_tmu[0].m_reg.write(regnum, data);
				}
			}
			if (BIT(chips, 2))
			{
				if (m_tmu[1].m_reg.read(regnum) != data)
				{
					m_tmu[1].m_regdirty = true;
					m_tmu[1].m_reg.write(regnum, data);
				}
			}
			break;

		// these registers are referenced in the renderer; we must wait for pending work before changing
		case voodoo_regs::reg_chromaRange:
			m_renderer->wait(m_reg.name(regnum));
			[[fallthrough]];
		// by default, just feed the data to the chips
		default:
			if (BIT(chips, 0)) m_reg.write(regnum, data);
			if (BIT(chips, 1)) m_tmu[0].m_reg.write(regnum, data);
			if (BIT(chips, 2)) m_tmu[1].m_reg.write(regnum, data);
			break;
	}

	if (LOG_REGISTERS)
	{
		if (regnum < voodoo_regs::reg_fvertexAx || regnum > voodoo_regs::reg_fdWdY)
			logerror("VOODOO.REG:%s(%d) write = %08X\n", (regnum < 0x384/4) ? m_reg.name(regnum) : "oob", chips, origdata);
		else
			logerror("VOODOO.REG:%s(%d) write = %f\n", (regnum < 0x384/4) ? m_reg.name(regnum) : "oob", chips, double(u2f(origdata)));
	}
	return cycles;
}


void voodoo_device::fbi_state::recompute_screen_params(voodoo_regs &regs, screen_device &screen)
{
	auto const hsync = regs.hsync();
	auto const vsync = regs.vsync();
	auto const back_porch = regs.back_porch();
	auto const video_dimensions = regs.video_dimensions();
	if (hsync.raw() != 0 && vsync.raw() != 0 && video_dimensions.raw() != 0 && back_porch.raw() != 0)
	{
		u32 htotal = hsync.hsync_off(regs.rev1()) + 1 + hsync.hsync_on(regs.rev1()) + 1;
		u32 vtotal = vsync.vsync_off(regs.rev1()) + vsync.vsync_on(regs.rev1());
		u32 hvis = video_dimensions.xwidth(regs.rev1());
		u32 vvis = video_dimensions.yheight(regs.rev1());
		u32 hbp = back_porch.horizontal(regs.rev1()) + 2;
		u32 vbp = back_porch.vertical(regs.rev1());

		/* create a new visarea */
		rectangle visarea(hbp, hbp + std::max(hvis - 1, 0), vbp, vbp + std::max(vvis - 1, 0));

		/* keep within bounds */
		visarea.max_x = std::min<s32>(visarea.max_x, htotal - 1);
		visarea.max_y = std::min<s32>(visarea.max_y, vtotal - 1);

		/* compute the new period for standard res, medium res, and VGA res */
		attoseconds_t stdperiod = HZ_TO_ATTOSECONDS(15750) * vtotal;
		attoseconds_t medperiod = HZ_TO_ATTOSECONDS(25000) * vtotal;
		attoseconds_t vgaperiod = HZ_TO_ATTOSECONDS(31500) * vtotal;

		/* compute a diff against the current refresh period */
		attoseconds_t refresh = screen.frame_period().attoseconds();
		attoseconds_t stddiff = std::abs(stdperiod - refresh);
		attoseconds_t meddiff = std::abs(medperiod - refresh);
		attoseconds_t vgadiff = std::abs(vgaperiod - refresh);

		osd_printf_debug("hSync=%08X  vSync=%08X  backPorch=%08X  videoDimensions=%08X\n",
			hsync.raw(), vsync.raw(), back_porch.raw(), video_dimensions.raw());
		osd_printf_debug("Horiz: %d-%d (%d total)  Vert: %d-%d (%d total) -- ", visarea.min_x, visarea.max_x, htotal, visarea.min_y, visarea.max_y, vtotal);

		/* configure the screen based on which one matches the closest */
		if (stddiff < meddiff && stddiff < vgadiff)
		{
			screen.configure(htotal, vtotal, visarea, stdperiod);
			osd_printf_debug("Standard resolution, %f Hz\n", ATTOSECONDS_TO_HZ(stdperiod));
		}
		else if (meddiff < vgadiff)
		{
			screen.configure(htotal, vtotal, visarea, medperiod);
			osd_printf_debug("Medium resolution, %f Hz\n", ATTOSECONDS_TO_HZ(medperiod));
		}
		else
		{
			screen.configure(htotal, vtotal, visarea, vgaperiod);
			osd_printf_debug("VGA resolution, %f Hz\n", ATTOSECONDS_TO_HZ(vgaperiod));
		}

		/* configure the new framebuffer info */
		m_width = hvis;
		m_height = vvis;
		m_xoffs = hbp;
		m_yoffs = vbp;
		m_vsyncstart = vsync.vsync_off(regs.rev1());
		m_vsyncstop = vsync.vsync_on(regs.rev1());
		osd_printf_debug("yoffs: %d vsyncstart: %d vsyncstop: %d\n", vbp, m_vsyncstart, m_vsyncstop);
	}
}

/*************************************
 *
 *  Voodoo LFB writes
 *
 *************************************/
s32 voodoo_device::lfb_direct_w(offs_t offset, u32 data, u32 mem_mask)
{
	/* statistics */
	if (DEBUG_STATS)
		m_stats.lfb_writes++;

	/* byte swizzling */
	auto const lfbmode = m_reg.lfb_mode();
	if (lfbmode.byte_swizzle_writes())
	{
		data = swapendian_int32(data);
		mem_mask = swapendian_int32(mem_mask);
	}

	/* word swapping */
	if (lfbmode.word_swap_writes())
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// TODO: This direct write is not verified.
	// For direct lfb access just write the data
	/* compute X,Y */
	offset <<= 1;
	int const x = offset & ((1 << m_fbi.m_lfb_stride) - 1);
	int const y = (offset >> m_fbi.m_lfb_stride);
	u16 *const dest = (u16 *)(m_fbi.m_ram + m_fbi.m_lfb_base*4);
	u32 const destmax = m_fbi.ram_end() - dest;
	u32 const bufoffs = y * m_fbi.m_rowpixels + x;
	if (bufoffs >= destmax)
	{
		logerror("lfb_direct_w: Buffer offset out of bounds x=%i y=%i offset=%08X bufoffs=%08X data=%08X\n", x, y, offset, bufoffs, data);
		return 0;
	}
	if (ACCESSING_BITS_0_15)
		dest[bufoffs + 0] = data&0xffff;
	if (ACCESSING_BITS_16_31)
		dest[bufoffs + 1] = data>>16;
	// Need to notify that frame buffer has changed
	m_fbi.m_video_changed = true;
	if (LOG_LFB) logerror("VOODOO.LFB:write direct (%d,%d) = %08X & %08X\n", x, y, data, mem_mask);
	return 0;
}

s32 voodoo_device::lfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.lfb_writes++;

	// byte swizzling
	auto const lfbmode = m_reg.lfb_mode();
	if (lfbmode.byte_swizzle_writes())
	{
		data = swapendian_int32(data);
		mem_mask = swapendian_int32(mem_mask);
	}

	// word swapping
	if (lfbmode.word_swap_writes())
	{
		data = (data << 16) | (data >> 16);
		mem_mask = (mem_mask << 16) | (mem_mask >> 16);
	}

	// extract default depth value from low bits of zaColor
	u16 sz[2];
	sz[0] = sz[1] = m_reg.za_color() & 0xffff;

	// if not otherwise specified, alpha defaults to the upper bits of zaColor
	u32 src_alpha = m_reg.za_color() >> 24;

	// extract color information from the data
	rgb_t src_color[2];
	u32 mask = 0;
	switch (16 * lfbmode.rgba_lanes() + lfbmode.write_format())
	{
		case 16*0 + 0:      // ARGB, format 0: 16-bit RGB 5-6-5
		case 16*2 + 0:      // RGBA, format 0: 16-bit RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data, 11,  5,  0).set_a(src_alpha);
			src_color[1] = rgbexpand<5,6,5>(data, 27, 21, 16).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*1 + 0:      // ABGR, format 0: 16-bit RGB 5-6-5
		case 16*3 + 0:      // BGRA, format 0: 16-bit RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data,  0,  5, 11).set_a(src_alpha);
			src_color[1] = rgbexpand<5,6,5>(data, 16, 21, 27).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*0 + 1:      // ARGB, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 10,  5,  0).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 26, 21, 16).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*1 + 1:      // ABGR, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  0,  5, 10).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 16, 21, 26).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*2 + 1:      // RGBA, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 11,  6,  1).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 27, 22, 17).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*3 + 1:      // BGRA, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  1,  6, 11).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 17, 22, 27).set_a(src_alpha);
			mask = LFB_RGB_PRESENT | (LFB_RGB_PRESENT << 4);
			offset <<= 1;
			break;

		case 16*0 + 2:      // ARGB, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15, 10,  5,  0);
			src_color[1] = argbexpand<1,5,5,5>(data, 31, 26, 21, 16);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*1 + 2:      // ABGR, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15,  0,  5, 10);
			src_color[1] = argbexpand<1,5,5,5>(data, 31, 16, 21, 26);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*2 + 2:      // RGBA, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0, 11,  6,  1);
			src_color[1] = argbexpand<1,5,5,5>(data, 16, 27, 22, 17);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*3 + 2:      // BGRA, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0,  1,  6, 11);
			src_color[1] = argbexpand<1,5,5,5>(data, 16, 17, 22, 27);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | ((LFB_RGB_PRESENT | LFB_ALPHA_PRESENT) << 4);
			offset <<= 1;
			break;

		case 16*0 + 4:      // ARGB, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data, 16,  8,  0);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*1 + 4:      // ABGR, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data,  0,  8, 16);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*2 + 4:      // RGBA, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data, 24, 16,  8);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*3 + 4:      // BGRA, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data,  8, 16, 24);
			mask = LFB_RGB_PRESENT;
			break;

		case 16*0 + 5:      // ARGB, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data, 24, 16,  8,  0);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*1 + 5:      // ABGR, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data, 24,  0,  8, 16);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*2 + 5:      // RGBA, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data,  0, 24, 16,  8);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*3 + 5:      // BGRA, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data,  0,  8, 16, 24);
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT;
			break;

		case 16*0 + 12:     // ARGB, format 12: 32-bit depth+RGB 5-6-5
		case 16*2 + 12:     // RGBA, format 12: 32-bit depth+RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data, 11,  5,  0).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*1 + 12:     // ABGR, format 12: 32-bit depth+RGB 5-6-5
		case 16*3 + 12:     // BGRA, format 12: 32-bit depth+RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data,  0,  5, 11).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 13:     // ARGB, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 10,  5,  0).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*1 + 13:     // ABGR, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  0,  5, 10).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*2 + 13:     // RGBA, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 11,  6,  1).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*3 + 13:     // BGRA, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  1,  6, 11).set_a(src_alpha);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 14:     // ARGB, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15, 10,  5,  0);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*1 + 14:     // ABGR, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15,  0,  5, 10);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*2 + 14:     // RGBA, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0, 11,  6,  1);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*3 + 14:     // BGRA, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0,  1,  6, 11);
			sz[0] = data >> 16;
			mask = LFB_RGB_PRESENT | LFB_ALPHA_PRESENT | LFB_DEPTH_PRESENT_MSW;
			break;

		case 16*0 + 15:     // ARGB, format 15: 16-bit depth
		case 16*1 + 15:     // ARGB, format 15: 16-bit depth
		case 16*2 + 15:     // ARGB, format 15: 16-bit depth
		case 16*3 + 15:     // ARGB, format 15: 16-bit depth
			sz[0] = data & 0xffff;
			sz[1] = data >> 16;
			mask = LFB_DEPTH_PRESENT | (LFB_DEPTH_PRESENT << 4);
			offset <<= 1;
			break;

		default:            // reserved
			logerror("lfb_w: Unknown format\n");
			return 0;
	}

	// compute X,Y
	s32 x = offset & ((1 << m_fbi.m_lfb_stride) - 1);
	s32 y = (offset >> m_fbi.m_lfb_stride) & 0x3ff;

	// adjust the mask based on which half of the data is written
	if (!ACCESSING_BITS_0_15)
		mask &= ~(0x0f - LFB_DEPTH_PRESENT_MSW);
	if (!ACCESSING_BITS_16_31)
		mask &= ~(0xf0 + LFB_DEPTH_PRESENT_MSW);

	// select the target buffer
	int destbuf = m_reg.rev3() ? 1 : lfbmode.write_buffer_select();
	u16 *dest;
	switch (destbuf)
	{
		case 0:         // front buffer
			dest = m_fbi.front_buffer();
			m_fbi.m_video_changed = true;
			break;

		case 1:         // back buffer
			dest = m_fbi.back_buffer();
			break;

		default:        // reserved
			return 0;
	}
	u32 destmax = m_fbi.ram_end() - dest;
	u16 *depth = m_fbi.aux_buffer();
	u32 depthmax = m_fbi.ram_end() - depth;

	// simple case: no pipeline
	auto const fbzmode = m_reg.fbz_mode();
	if (!lfbmode.enable_pixel_pipeline())
	{
		if (LOG_LFB) logerror("VOODOO.LFB:write raw mode %X (%d,%d) = %08X & %08X\n", lfbmode.write_format(), x, y, data, mem_mask);

		// determine the screen Y
		s32 scry = y;
		if (lfbmode.y_origin())
			scry = m_fbi.m_yorigin - y;

		// advance pointers to the proper row
		u32 bufoffs = scry * m_fbi.m_rowpixels + x;

		// wait for any outstanding work to finish
		m_renderer->wait("LFB Write");

		// loop over up to two pixels
		voodoo::dither_helper dither(scry, fbzmode);
		for (int pix = 0; mask != 0; pix++)
		{
			// make sure we care about this pixel
			if ((mask & 0x0f) != 0)
			{
				// write to the RGB buffer
				rgb_t pixel = src_color[pix];
				if ((mask & LFB_RGB_PRESENT) && bufoffs < destmax)
					dest[bufoffs] = dither.pixel(x, pixel.r(), pixel.g(), pixel.b());

				// make sure we have an aux buffer to write to
				if (depth != nullptr && bufoffs < depthmax)
				{
					// write to the alpha buffer
					if ((mask & LFB_ALPHA_PRESENT) && fbzmode.enable_alpha_planes())
						depth[bufoffs] = pixel.a();

					// write to the depth buffer
					if ((mask & (LFB_DEPTH_PRESENT | LFB_DEPTH_PRESENT_MSW)) && !fbzmode.enable_alpha_planes())
						depth[bufoffs] = sz[pix];
				}

				// track pixel writes to the frame buffer regardless of mask
				m_reg.add(voodoo_regs::reg_fbiPixelsOut, 1);
			}

			// advance our pointers
			bufoffs++;
			x++;
			mask >>= 4;
		}
	}

	// tricky case: run the full pixel pipeline on the pixel
	else
	{
		if (LOG_LFB) logerror("VOODOO.LFB:write pipelined mode %X (%d,%d) = %08X & %08X\n", lfbmode.write_format(), x, y, data, mem_mask);

		// determine the screen Y
		s32 scry = y;
		if (fbzmode.y_origin())
			scry = m_fbi.m_yorigin - y;

		// advance pointers to the proper row
		dest += scry * m_fbi.m_rowpixels;
		if (depth != nullptr)
			depth += scry * m_fbi.m_rowpixels;

		// make a dummy poly_extra_data structure with some cached values
		poly_data poly;
		poly.raster.compute(m_reg, nullptr, nullptr);
		poly.destbase = dest;
		poly.depthbase = depth;
		poly.clipleft = m_reg.clip_left();
		poly.clipright = m_reg.clip_right();
		poly.cliptop = m_reg.clip_top();
		poly.clipbottom = m_reg.clip_bottom();
		poly.color0 = m_reg.color0().argb();
		poly.color1 = m_reg.color1().argb();
		poly.chromakey = m_reg.chroma_key().argb();
		poly.fogcolor = m_reg.fog_color().argb();
		poly.zacolor = m_reg.za_color();
		poly.stipple = m_reg.stipple();
		poly.alpharef = m_reg.alpha_mode().alpharef();
		if (poly.raster.fbzmode().enable_stipple() && !poly.raster.fbzmode().stipple_pattern())
			printf("Warning: rotated stipple pattern\n");

		// loop over up to two pixels
		thread_stats_block &threadstats = m_fbi.m_lfb_stats;
		rgbaint_t iterargb(0);
		for (int pix = 0; mask != 0; pix++)
		{
			// make sure we care about this pixel
			if ((mask & 0x0f) != 0)
				m_renderer->pixel_pipeline(threadstats, poly, lfbmode, x, y, src_color[pix], sz[pix]);

			// advance our pointers
			x++;
			mask >>= 4;
		}
	}
	return 0;
}



/*************************************
 *
 *  Voodoo texture RAM writes
 *
 *************************************/

s32 voodoo_device::texture_w(offs_t offset, u32 data)
{
	/* statistics */
	if (DEBUG_STATS)
		m_stats.tex_writes++;

	/* point to the right TMU */
	int tmunum = (offset >> 19) & 0x03;
	if (!(m_chipmask & (2 << tmunum)))
		return 0;

	/* wait for any outstanding work to finish */
	m_renderer->wait("Texture write");

	m_tmu[tmunum].texture_w(offset, data, m_tmu[0].m_reg.texture_mode().seq_8_downld());
	return 0;
}

void voodoo_device::tmu_state::texture_w(offs_t offset, u32 data, bool seq_8_downld)
{
	auto const texlod = m_reg.texture_lod();

	// texture direct not handled (but never seen so far)
	if (texlod.tdirect_write())
		fatalerror("Texture direct write!\n");

	// swizzle the data
	if (texlod.tdata_swizzle())
		data = swapendian_int32(data);
	if (texlod.tdata_swap())
		data = (data >> 16) | (data << 16);

	// update texture info if dirty
	auto const texmode = m_reg.texture_mode();
	auto &texture = prepare_texture();

	// determine destination pointer
	u32 scale = (texmode.format() < 8) ? 1 : 2;
	u8 *dest;
	if (m_reg.rev1_or_2())
	{
		u32 lod = (offset >> 15) & 0x0f;
		u32 ts = (offset << ((seq_8_downld && scale == 1) ? 2 : 1)) & 0xff;
		u32 tt = (offset >> 7) & 0xff;

		// validate parameters
		if (lod > 8)
			return;

		dest = texture.write_ptr(lod, ts, tt, scale);
	}
	else
		dest = texture.write_ptr(0, offset * 4, 0, 1);

	// write the four bytes in little-endian order
	if (scale == 1)
	{
		dest[BYTE4_XOR_LE(0)] = (data >> 0) & 0xff;
		dest[BYTE4_XOR_LE(1)] = (data >> 8) & 0xff;
		dest[BYTE4_XOR_LE(2)] = (data >> 16) & 0xff;
		dest[BYTE4_XOR_LE(3)] = (data >> 24) & 0xff;
	}
	else
	{
		u16 *dest16 = reinterpret_cast<u16 *>(dest);
		dest16[BYTE_XOR_LE(0)] = (data >> 0) & 0xffff;
		dest16[BYTE_XOR_LE(1)] = (data >> 16) & 0xffff;
	}

}



/*************************************
 *
 *  Flush data from the FIFOs
 *
 *************************************/

void voodoo_device::flush_fifos(attotime current_time)
{
	static u8 in_flush;

	/* check for recursive calls */
	if (in_flush)
		return;
	in_flush = true;

	if (!m_pci.op_pending)
		fatalerror("flush_fifos called with no pending operation\n");

	if (LOG_FIFO_VERBOSE)
	{
		logerror("VOODOO.FIFO:flush_fifos start -- pending=%d.%018d cur=%d.%018d\n",
				m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds(),
				current_time.seconds(), current_time.attoseconds());
	}

	/* loop while we still have cycles to burn */
	while (m_pci.op_end_time <= current_time)
	{
		s32 cycles;

		/* loop over 0-cycle stuff; this constitutes the bulk of our writes */
		do
		{
			/* we might be in CMDFIFO mode */
			if (m_fbi.m_cmdfifo[0].enabled())
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = m_fbi.m_cmdfifo[0].execute_if_ready();
				if (cycles == -1)
				{
					m_pci.op_pending = false;
					in_flush = false;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:flush_fifos end -- CMDFIFO empty\n");
					return;
				}
			}
			else if (m_fbi.m_cmdfifo[1].enabled())
			{
				/* if we don't have anything to execute, we're done for now */
				cycles = m_fbi.m_cmdfifo[1].execute_if_ready();
				if (cycles == -1)
				{
					m_pci.op_pending = false;
					in_flush = false;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:flush_fifos end -- CMDFIFO empty\n");
					return;
				}
			}

			/* else we are in standard PCI/memory FIFO mode */
			else
			{
				voodoo::memory_fifo *fifo;

				/* choose which FIFO to read from */
				if (!m_fbi.m_fifo.empty())
					fifo = &m_fbi.m_fifo;
				else if (!m_pci.fifo.empty())
					fifo = &m_pci.fifo;
				else
				{
					m_pci.op_pending = false;
					in_flush = false;
					if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:flush_fifos end -- FIFOs empty\n");
					return;
				}

				/* extract address and data */
				u32 address = fifo->remove();
				u32 data = fifo->remove();

				/* target the appropriate location */
				if ((address & (0xc00000/4)) == 0)
					cycles = register_w(address, data);
				else if (address & (0x800000/4))
					cycles = texture_w(address, data);
				else
				{
					u32 mem_mask = 0xffffffff;

					/* compute mem_mask */
					if (address & 0x80000000)
						mem_mask &= 0x0000ffff;
					if (address & 0x40000000)
						mem_mask &= 0xffff0000;
					address &= 0xffffff;

					cycles = lfb_w(address, data, mem_mask);
				}
			}
		}
		while (cycles == 0);

		/* account for those cycles */
		m_pci.op_end_time += clocks_to_attotime(cycles);

		if (LOG_FIFO_VERBOSE)
		{
			logerror("VOODOO.FIFO:update -- pending=%d.%018d cur=%d.%018d\n",
					m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds(),
					current_time.seconds(), current_time.attoseconds());
		}
	}

	if (LOG_FIFO_VERBOSE)
	{
		logerror("VOODOO.FIFO:flush_fifos end -- pending command complete at %d.%018d\n",
				m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds());
	}

	in_flush = false;
}



/*************************************
 *
 *  Handle a write to the Voodoo
 *  memory space
 *
 *************************************/

void voodoo_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	g_profiler.start(PROFILER_USER1);

	// should not be getting accesses while stalled (but we do)
	if (m_pci.stall_state != NOT_STALLED)
		logerror("voodoo_device::write while stalled!\n");

	// if we have something pending, flush the FIFOs up to the current time
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	/* special handling for registers */
	if ((offset & 0xc00000/4) == 0)
	{
		/* some special stuff for Voodoo 2 */
		if (m_reg.rev2_or_3())
		{
			/* we might be in CMDFIFO mode */
			if (m_reg.fbi_init7().cmdfifo_enable())
			{
				/* if bit 21 is set, we're writing to the FIFO */
				if (offset & 0x200000/4)
				{
					/* check for byte swizzling (bit 18) */
					if (offset & 0x40000/4)
						data = swapendian_int32(data);
					m_fbi.m_cmdfifo[0].write_direct(BIT(offset, 0, 16), data);
					g_profiler.stop();
					return;
				}

				/* we're a register access; but only certain ones are allowed */
				if (!m_reg.is_writethru(offset & 0xff))
				{
					/* track swap buffers regardless */
					if ((offset & 0xff) == voodoo_regs::reg_swapbufferCMD)
						m_fbi.m_swaps_pending++;

					logerror("Ignoring write to %s in CMDFIFO mode\n", m_reg.name(offset));
					g_profiler.stop();
					return;
				}
			}

			/* if not, we might be byte swizzled (bit 20) */
			else if (offset & 0x100000/4)
				data = swapendian_int32(data);
		}

		/* ignore if writes aren't allowed */
		if (!m_reg.is_writable(offset & 0xff))
		{
			g_profiler.stop();
			return;
		}

		// if this is non-FIFO command, execute immediately
		if (!m_reg.is_fifo(offset & 0xff))
		{
			register_w(offset, data);
			g_profiler.stop();
			return;
		}

		/* track swap buffers */
		if ((offset & 0xff) == voodoo_regs::reg_swapbufferCMD)
			m_fbi.m_swaps_pending++;
	}

	/* if we don't have anything pending, or if FIFOs are disabled, just execute */
	if (!m_pci.op_pending || !m_pci.init_enable.enable_pci_fifo())
	{
		int cycles;

		/* target the appropriate location */
		if ((offset & (0xc00000/4)) == 0)
			cycles = register_w(offset, data);
		else if (offset & (0x800000/4))
			cycles = texture_w(offset, data);
		else
			cycles = lfb_w(offset, data, mem_mask);

		/* if we ended up with cycles, mark the operation pending */
		if (cycles)
		{
			m_pci.op_pending = true;
			m_pci.op_end_time = machine().time() + clocks_to_attotime(cycles);

			if (LOG_FIFO_VERBOSE)
			{
				logerror("VOODOO.FIFO:direct write start at %d.%018d end at %d.%018d\n",
						machine().time().seconds(), machine().time().attoseconds(),
						m_pci.op_end_time.seconds(), m_pci.op_end_time.attoseconds());
			}
		}
		g_profiler.stop();
		return;
	}

	/* modify the offset based on the mem_mask */
	if (mem_mask != 0xffffffff)
	{
		if (!ACCESSING_BITS_16_31)
			offset |= 0x80000000;
		if (!ACCESSING_BITS_0_15)
			offset |= 0x40000000;
	}

	/* if there's room in the PCI FIFO, add there */
	if (LOG_FIFO_VERBOSE) logerror("VOODOO.%d.FIFO:adding to PCI FIFO @ %08X=%08X\n", this, offset, data);
	if (!m_pci.fifo.full())
	{
		m_pci.fifo.add(offset);
		m_pci.fifo.add(data);
	}
	else
		fatalerror("PCI FIFO full\n");

	/* handle flushing to the memory FIFO */
	if (m_reg.fbi_init0().enable_memory_fifo() && m_pci.fifo.space() <= 2 * m_reg.fbi_init4().memory_fifo_lwm())
	{
		u8 valid[4];

		/* determine which types of data can go to the memory FIFO */
		valid[0] = true;
		valid[1] = m_reg.fbi_init0().lfb_to_memory_fifo();
		valid[2] = valid[3] = m_reg.fbi_init0().texmem_to_memory_fifo();

		/* flush everything we can */
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:moving PCI FIFO to memory FIFO\n");
		while (!m_pci.fifo.empty() && valid[(m_pci.fifo.peek() >> 22) & 3])
		{
			m_fbi.m_fifo.add(m_pci.fifo.remove());
			m_fbi.m_fifo.add(m_pci.fifo.remove());
		}

		/* if we're above the HWM as a result, stall */
		if (m_reg.fbi_init0().stall_pcie_for_hwm() && m_fbi.m_fifo.items() >= 2 * 32 * m_reg.fbi_init0().memory_fifo_hwm())
		{
			if (LOG_FIFO) logerror("VOODOO.FIFO:hit memory FIFO HWM -- stalling\n");
			stall_cpu(STALLED_UNTIL_FIFO_LWM, machine().time());
		}
	}

	/* if we're at the LWM for the PCI FIFO, stall */
	if (m_reg.fbi_init0().stall_pcie_for_hwm() && m_pci.fifo.space() <= 2 * m_reg.fbi_init0().pci_fifo_lwm())
	{
		if (LOG_FIFO) logerror("VOODOO.FIFO:hit PCI FIFO free LWM -- stalling\n");
		stall_cpu(STALLED_UNTIL_FIFO_LWM, machine().time());
	}
	g_profiler.stop();
}



/*************************************
 *
 *  Handle a register read
 *
 *************************************/

u32 voodoo_device::register_r(offs_t offset)
{
	int regnum = offset & 0xff;
	u32 result;

	/* statistics */
	if (DEBUG_STATS)
		m_stats.reg_reads++;

	/* first make sure this register is readable */
	if (!m_reg.is_readable(offset))
	{
		logerror("VOODOO.ERROR:Invalid attempt to read %s\n", regnum < 225 ? m_reg.name(regnum) : "unknown register");
		return 0xffffffff;
	}

	/* default result is the FBI register value */
	result = m_reg.read(regnum);

	/* some registers are dynamic; compute them */
	switch (regnum)
	{
		case voodoo_regs::reg_vdstatus:

			/* start with a blank slate */
			result = 0;

			/* bits 5:0 are the PCI FIFO free space */
			if (m_pci.fifo.empty())
				result |= 0x3f << 0;
			else
			{
				int temp = m_pci.fifo.space()/2;
				if (temp > 0x3f)
					temp = 0x3f;
				result |= temp << 0;
			}

			/* bit 6 is the vertical retrace */
			result |= m_fbi.m_vblank << 6;

			/* bit 7 is FBI graphics engine busy */
			if (m_pci.op_pending)
				result |= 1 << 7;

			/* bit 8 is TREX busy */
			if (m_pci.op_pending)
				result |= 1 << 8;

			/* bit 9 is overall busy */
			if (m_pci.op_pending)
				result |= 1 << 9;

			/* Banshee is different starting here */
			if (m_reg.rev1_or_2())
			{
				/* bits 11:10 specifies which buffer is visible */
				result |= m_fbi.m_frontbuf << 10;

				/* bits 27:12 indicate memory FIFO freespace */
				if (m_reg.fbi_init0().enable_memory_fifo() == 0 || m_fbi.m_fifo.empty())
					result |= 0xffff << 12;
				else
				{
					int temp = m_fbi.m_fifo.space()/2;
					if (temp > 0xffff)
						temp = 0xffff;
					result |= temp << 12;
				}
			}
			else
			{
				/* bit 10 is 2D busy */

				/* bit 11 is cmd FIFO 0 busy */
				if (m_fbi.m_cmdfifo[0].enabled() && m_fbi.m_cmdfifo[0].depth() > 0)
					result |= 1 << 11;

				/* bit 12 is cmd FIFO 1 busy */
				if (m_fbi.m_cmdfifo[1].enabled() && m_fbi.m_cmdfifo[1].depth() > 0)
					result |= 1 << 12;
			}

			/* bits 30:28 are the number of pending swaps */
			if (m_fbi.m_swaps_pending > 7)
				result |= 7 << 28;
			else
				result |= m_fbi.m_swaps_pending << 28;

			/* bit 31 is not used */

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(1000);
			break;

		/* bit 2 of the initEnable register maps this to dacRead */
		case voodoo_regs::reg_fbiInit2:
			if (m_pci.init_enable.remap_init_to_dac())
				result = m_dac.read_result;
			break;

		/* return the current visible scanline */
		case voodoo_regs::reg_vRetrace:
			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(10);
			// Return 0 if vblank is active
			if (m_fbi.m_vblank) {
				result = 0;
			}
			else {
				// Want screen position from vblank off
				result = m_screen->vpos();
			}
			break;

		/* return visible horizontal and vertical positions. Read by the Vegas startup sequence */
		case voodoo_regs::reg_hvRetrace:
			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(10);
			//result = 0x200 << 16;   /* should be between 0x7b and 0x267 */
			//result |= 0x80;         /* should be between 0x17 and 0x103 */
			// Return 0 if vblank is active
			if (m_fbi.m_vblank) {
				result = 0;
			}
			else {
				// Want screen position from vblank off
				result = m_screen->vpos();
			}
			// Hpos
			result |= m_screen->hpos() << 16;
			break;

		/* cmdFifo -- Voodoo2 only */
		case voodoo_regs::reg_cmdFifoRdPtr:
			result = m_fbi.m_cmdfifo[0].read_pointer();

			/* eat some cycles since people like polling here */
			if (EAT_CYCLES) m_cpu->eat_cycles(1000);
			break;

		case voodoo_regs::reg_cmdFifoAMin:
			result = m_fbi.m_cmdfifo[0].address_min();
			break;

		case voodoo_regs::reg_cmdFifoAMax:
			result = m_fbi.m_cmdfifo[0].address_max();
			break;

		case voodoo_regs::reg_cmdFifoDepth:
			result = m_fbi.m_cmdfifo[0].depth();
			break;

		case voodoo_regs::reg_cmdFifoHoles:
			result = m_fbi.m_cmdfifo[0].holes();
			break;

		/* all counters are 24-bit only */
		case voodoo_regs::reg_fbiPixelsIn:
		case voodoo_regs::reg_fbiChromaFail:
		case voodoo_regs::reg_fbiZfuncFail:
		case voodoo_regs::reg_fbiAfuncFail:
		case voodoo_regs::reg_fbiPixelsOut:
			update_statistics(true);
			[[fallthrough]];
		case voodoo_regs::reg_fbiTrianglesOut:
			result = m_reg.read(regnum) & 0xffffff;
			break;
	}

	if (LOG_REGISTERS)
	{
		int logit = true;

		/* don't log multiple identical status reads from the same address */
		if (regnum == voodoo_regs::reg_vdstatus)
		{
			offs_t pc = m_cpu->pc();
			if (pc == m_last_status_pc && result == m_last_status_value)
				logit = false;
			m_last_status_pc = pc;
			m_last_status_value = result;
		}
		if (regnum == voodoo_regs::reg_cmdFifoRdPtr)
			logit = false;

		if (logit)
			logerror("VOODOO.REG:%s read = %08X\n", m_reg.name(regnum), result);
	}

	return result;
}



/*************************************
 *
 *  Handle an LFB read
 *
 *************************************/

u32 voodoo_device::lfb_r(offs_t offset, bool lfb_3d)
{
	u16 *buffer;
	u32 bufmax;
	u32 bufoffs;
	u32 data;
	int x, y, scry, destbuf;

	/* statistics */
	if (DEBUG_STATS)
		m_stats.lfb_reads++;

	/* compute X,Y */
	offset <<= 1;
	x = offset & ((1 << m_fbi.m_lfb_stride) - 1);
	y = (offset >> m_fbi.m_lfb_stride);

	/* select the target buffer */
	auto const lfbmode = m_reg.lfb_mode();
	if (lfb_3d)
	{
		y &= 0x3ff;
		destbuf = m_reg.rev3() ? 1 : lfbmode.read_buffer_select();
		switch (destbuf)
		{
			case 0:         /* front buffer */
				buffer = m_fbi.front_buffer();
				break;

			case 1:         /* back buffer */
				buffer = m_fbi.back_buffer();
				break;

			case 2:         /* aux buffer */
				if (m_fbi.m_auxoffs == ~0)
					return 0xffffffff;
				buffer = m_fbi.aux_buffer();
				break;

			default:        /* reserved */
				return 0xffffffff;
		}

		/* determine the screen Y */
		scry = y;
		if (lfbmode.y_origin())
			scry = m_fbi.m_yorigin - y;
	}
	else
	{
		// Direct lfb access
		buffer = (u16 *)(m_fbi.m_ram + m_fbi.m_lfb_base*4);
		scry = y;
	}
	bufmax = m_fbi.ram_end() - buffer;

	/* advance pointers to the proper row */
	bufoffs = scry * m_fbi.m_rowpixels + x;
	if (bufoffs >= bufmax)
	{
		logerror("LFB_R: Buffer offset out of bounds x=%i y=%i lfb_3d=%i offset=%08X bufoffs=%08X\n", x, y, lfb_3d, offset, (u32) bufoffs);
		return 0xffffffff;
	}

	/* wait for any outstanding work to finish */
	m_renderer->wait("LFB read");

	/* compute the data */
	data = buffer[bufoffs + 0] | (buffer[bufoffs + 1] << 16);

	/* word swapping */
	if (lfbmode.word_swap_reads())
		data = (data << 16) | (data >> 16);

	/* byte swizzling */
	if (lfbmode.byte_swizzle_reads())
		data = swapendian_int32(data);

	if (LOG_LFB) logerror("VOODOO.LFB:read (%d,%d) = %08X\n", x, y, data);
	return data;
}



/*************************************
 *
 *  Handle a read from the Voodoo
 *  memory space
 *
 *************************************/

u32 voodoo_device::read(offs_t offset)
{
	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	/* target the appropriate location */
	if (!(offset & (0xc00000/4)))
		return register_r(offset);
	else if (!(offset & (0x800000/4)))
		return lfb_r(offset, true);

	return 0xffffffff;
}



/*************************************
 *
 *  Handle a read from the Banshee
 *  I/O space
 *
 *************************************/

u32 voodoo_banshee_device::banshee_agp_r(offs_t offset)
{
	u32 result;

	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdRdPtrL0:
			result = m_fbi.m_cmdfifo[0].read_pointer();
			break;

		case cmdAMin0:
			result = m_fbi.m_cmdfifo[0].address_min();
			break;

		case cmdAMax0:
			result = m_fbi.m_cmdfifo[0].address_max();
			break;

		case cmdFifoDepth0:
			result = m_fbi.m_cmdfifo[0].depth();
			break;

		case cmdHoleCnt0:
			result = m_fbi.m_cmdfifo[0].holes();
			break;

		case cmdRdPtrL1:
			result = m_fbi.m_cmdfifo[1].read_pointer();
			break;

		case cmdAMin1:
			result = m_fbi.m_cmdfifo[1].address_min();
			break;

		case cmdAMax1:
			result = m_fbi.m_cmdfifo[1].address_max();
			break;

		case cmdFifoDepth1:
			result = m_fbi.m_cmdfifo[1].depth();
			break;

		case cmdHoleCnt1:
			result = m_fbi.m_cmdfifo[1].holes();
			break;

		default:
			result = m_banshee.agp[offset];
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_r(AGP:%s)\n", machine().describe_context(), voodoo_regs::s_banshee_agp_reg_name[offset]);
	return result;
}


u32 voodoo_banshee_device::banshee_r(offs_t offset, u32 mem_mask)
{
	u32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < 0x80000/4)
		result = banshee_io_r(offset, mem_mask);
	else if (offset < 0x100000/4)
		result = banshee_agp_r(offset);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_r(2D:%X)\n", machine().describe_context(), (offset*4) & 0xfffff);
	else if (offset < 0x600000/4)
		result = register_r(offset & 0x1fffff/4);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_r(TEX0:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_r(TEX1:%X)\n", machine().describe_context(), (offset*4) & 0x1fffff);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_r(YUV:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x2000000/4)
	{
		result = lfb_r(offset & 0xffffff/4, true);
	} else {
			logerror("%s:banshee_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	return result;
}


u32 voodoo_banshee_device::banshee_fb_r(offs_t offset)
{
	u32 result = 0xffffffff;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < m_fbi.m_lfb_base)
	{
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X)\n", machine().describe_context(), offset*4);
		if (offset*4 <= m_fbi.m_mask)
			result = ((u32 *)m_fbi.m_ram)[offset];
		else
			logerror("%s:banshee_fb_r(%X) Access out of bounds\n", machine().describe_context(), offset*4);
	}
	else {
		if (LOG_LFB)
			logerror("%s:banshee_fb_r(%X) to lfb_r: %08X lfb_base=%08X\n", machine().describe_context(), offset*4, offset - m_fbi.m_lfb_base, m_fbi.m_lfb_base);
		result = lfb_r(offset - m_fbi.m_lfb_base, false);
	}
	return result;
}


u8 voodoo_banshee_device::banshee_vga_r(offs_t offset)
{
	u8 result = 0xff;

	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
			if (m_banshee.vga[0x3c1 & 0x1f] < std::size(m_banshee.att))
				result = m_banshee.att[m_banshee.vga[0x3c1 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_att_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3c1 & 0x1f]);
			break;

		/* Input status 0 */
		case 0x3c2:
			/*
			    bit 7 = Interrupt Status. When its value is ?1?, denotes that an interrupt is pending.
			    bit 6:5 = Feature Connector. These 2 bits are readable bits from the feature connector.
			    bit 4 = Sense. This bit reflects the state of the DAC monitor sense logic.
			    bit 3:0 = Reserved. Read back as 0.
			*/
			result = 0x00;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Sequencer access */
		case 0x3c5:
			if (m_banshee.vga[0x3c4 & 0x1f] < std::size(m_banshee.seq))
				result = m_banshee.seq[m_banshee.vga[0x3c4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3c4 & 0x1f]);
			break;

		/* Feature control */
		case 0x3ca:
			result = m_banshee.vga[0x3da & 0x1f];
			m_banshee.attff = 0;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Miscellaneous output */
		case 0x3cc:
			result = m_banshee.vga[0x3c2 & 0x1f];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (m_banshee.vga[0x3ce & 0x1f] < std::size(m_banshee.gc))
				result = m_banshee.gc[m_banshee.vga[0x3ce & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3ce & 0x1f]);
			break;

		/* CRTC access */
		case 0x3d5:
			if (m_banshee.vga[0x3d4 & 0x1f] < std::size(m_banshee.crtc))
				result = m_banshee.crtc[m_banshee.vga[0x3d4 & 0x1f]];
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_r(%X)\n", machine().describe_context(), m_banshee.vga[0x3d4 & 0x1f]);
			break;

		/* Input status 1 */
		case 0x3da:
			/*
			    bit 7:6 = Reserved. These bits read back 0.
			    bit 5:4 = Display Status. These 2 bits reflect 2 of the 8 pixel data outputs from the Attribute
			                controller, as determined by the Attribute controller index 0x12 bits 4 and 5.
			    bit 3 = Vertical sync Status. A ?1? indicates vertical retrace is in progress.
			    bit 2:1 = Reserved. These bits read back 0x2.
			    bit 0 = Display Disable. When this bit is 1, either horizontal or vertical display end has occurred,
			                otherwise video data is being displayed.
			*/
			result = 0x04;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;

		default:
			result = m_banshee.vga[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_r(%X)\n", machine().describe_context(), 0x3c0+offset);
			break;
	}
	return result;
}


u32 voodoo_banshee_device::banshee_io_r(offs_t offset, u32 mem_mask)
{
	u32 result;

	offset &= 0xff/4;

	/* switch off the offset */
	switch (offset)
	{
		case io_status:
			result = register_r(0);
			break;

		case io_dacData:
			result = m_fbi.m_clut[m_banshee.io[io_dacAddr] & 0x1ff] = m_banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_r(%X)\n", machine().describe_context(), m_banshee.io[io_dacAddr] & 0x1ff);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			result = 0;
			if (ACCESSING_BITS_0_7)
				result |= banshee_vga_r(offset*4+0) << 0;
			if (ACCESSING_BITS_8_15)
				result |= banshee_vga_r(offset*4+1) << 8;
			if (ACCESSING_BITS_16_23)
				result |= banshee_vga_r(offset*4+2) << 16;
			if (ACCESSING_BITS_24_31)
				result |= banshee_vga_r(offset*4+3) << 24;
			break;

		default:
			result = m_banshee.io[offset];
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_r(%s)\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset]);
			break;
	}

	return result;
}


u32 voodoo_banshee_device::banshee_rom_r(offs_t offset)
{
	logerror("%s:banshee_rom_r(%X)\n", machine().describe_context(), offset*4);
	return 0xffffffff;
}

void voodoo_banshee_device::banshee_blit_2d(u32 data)
{
	switch (m_banshee.blt_cmd)
	{
		case 0:         // NOP - wait for idle
		{
			break;
		}

		case 1:         // Screen-to-screen blit
		{
			// TODO
			if (LOG_BANSHEE_2D)
				logerror("   blit_2d:screen_to_screen: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
			break;
		}

		case 2:         // Screen-to-screen stretch blit
		{
			fatalerror("   blit_2d:screen_to_screen_stretch: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 3:         // Host-to-screen blit
		{
			u32 addr = m_banshee.blt_dst_base;

			addr += (m_banshee.blt_dst_y * m_banshee.blt_dst_stride) + (m_banshee.blt_dst_x * m_banshee.blt_dst_bpp);

			if (LOG_BANSHEE_2D)
				logerror("   blit_2d:host_to_screen: %08x -> %08x, %d, %d\n", data, addr, m_banshee.blt_dst_x, m_banshee.blt_dst_y);

			switch (m_banshee.blt_dst_bpp)
			{
				case 1:
					m_fbi.m_ram[addr+0] = data & 0xff;
					m_fbi.m_ram[addr+1] = (data >> 8) & 0xff;
					m_fbi.m_ram[addr+2] = (data >> 16) & 0xff;
					m_fbi.m_ram[addr+3] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 4;
					break;
				case 2:
					m_fbi.m_ram[addr+1] = data & 0xff;
					m_fbi.m_ram[addr+0] = (data >> 8) & 0xff;
					m_fbi.m_ram[addr+3] = (data >> 16) & 0xff;
					m_fbi.m_ram[addr+2] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 2;
					break;
				case 3:
					m_banshee.blt_dst_x += 1;
					break;
				case 4:
					m_fbi.m_ram[addr+3] = data & 0xff;
					m_fbi.m_ram[addr+2] = (data >> 8) & 0xff;
					m_fbi.m_ram[addr+1] = (data >> 16) & 0xff;
					m_fbi.m_ram[addr+0] = (data >> 24) & 0xff;
					m_banshee.blt_dst_x += 1;
					break;
			}

			if (m_banshee.blt_dst_x >= m_banshee.blt_dst_width)
			{
				m_banshee.blt_dst_x = 0;
				m_banshee.blt_dst_y++;
			}
			break;
		}

		case 5:         // Rectangle fill
		{
			fatalerror("blit_2d:rectangle_fill: src X %d, src Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 6:         // Line
		{
			fatalerror("blit_2d:line: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 7:         // Polyline
		{
			fatalerror("blit_2d:polyline: end X %d, end Y %d\n", data & 0xfff, (data >> 16) & 0xfff);
		}

		case 8:         // Polygon fill
		{
			fatalerror("blit_2d:polygon_fill\n");
		}

		default:
		{
			fatalerror("blit_2d: unknown command %d\n", m_banshee.blt_cmd);
		}
	}
}

s32 voodoo_device::banshee_2d_w(offs_t offset, u32 data)
{
	// placeholder for Banshee 2D access
	return 0;
}

s32 voodoo_banshee_device::banshee_2d_w(offs_t offset, u32 data)
{
	switch (offset)
	{
		case banshee2D_command:
			if (LOG_BANSHEE_2D)
				logerror("   2D:command: cmd %d, ROP0 %02X\n", data & 0xf, data >> 24);

			m_banshee.blt_src_x        = m_banshee.blt_regs[banshee2D_srcXY] & 0xfff;
			m_banshee.blt_src_y        = (m_banshee.blt_regs[banshee2D_srcXY] >> 16) & 0xfff;
			m_banshee.blt_src_base     = m_banshee.blt_regs[banshee2D_srcBaseAddr] & 0xffffff;
			m_banshee.blt_src_stride   = m_banshee.blt_regs[banshee2D_srcFormat] & 0x3fff;
			m_banshee.blt_src_width    = m_banshee.blt_regs[banshee2D_srcSize] & 0xfff;
			m_banshee.blt_src_height   = (m_banshee.blt_regs[banshee2D_srcSize] >> 16) & 0xfff;

			switch ((m_banshee.blt_regs[banshee2D_srcFormat] >> 16) & 0xf)
			{
				case 1: m_banshee.blt_src_bpp = 1; break;
				case 3: m_banshee.blt_src_bpp = 2; break;
				case 4: m_banshee.blt_src_bpp = 3; break;
				case 5: m_banshee.blt_src_bpp = 4; break;
				case 8: m_banshee.blt_src_bpp = 2; break;
				case 9: m_banshee.blt_src_bpp = 2; break;
				default: m_banshee.blt_src_bpp = 1; break;
			}

			m_banshee.blt_dst_x        = m_banshee.blt_regs[banshee2D_dstXY] & 0xfff;
			m_banshee.blt_dst_y        = (m_banshee.blt_regs[banshee2D_dstXY] >> 16) & 0xfff;
			m_banshee.blt_dst_base     = m_banshee.blt_regs[banshee2D_dstBaseAddr] & 0xffffff;
			m_banshee.blt_dst_stride   = m_banshee.blt_regs[banshee2D_dstFormat] & 0x3fff;
			m_banshee.blt_dst_width    = m_banshee.blt_regs[banshee2D_dstSize] & 0xfff;
			m_banshee.blt_dst_height   = (m_banshee.blt_regs[banshee2D_dstSize] >> 16) & 0xfff;

			switch ((m_banshee.blt_regs[banshee2D_dstFormat] >> 16) & 0x7)
			{
				case 1: m_banshee.blt_dst_bpp = 1; break;
				case 3: m_banshee.blt_dst_bpp = 2; break;
				case 4: m_banshee.blt_dst_bpp = 3; break;
				case 5: m_banshee.blt_dst_bpp = 4; break;
				default: m_banshee.blt_dst_bpp = 1; break;
			}

			m_banshee.blt_cmd = data & 0xf;
			break;

		case banshee2D_colorBack:
			if (LOG_BANSHEE_2D)
				logerror("   2D:colorBack: %08X\n", data);
			m_banshee.blt_regs[banshee2D_colorBack] = data;
			break;

		case banshee2D_colorFore:
			if (LOG_BANSHEE_2D)
				logerror("   2D:colorFore: %08X\n", data);
			m_banshee.blt_regs[banshee2D_colorFore] = data;
			break;

		case banshee2D_srcBaseAddr:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
			m_banshee.blt_regs[banshee2D_srcBaseAddr] = data;
			break;

		case banshee2D_dstBaseAddr:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstBaseAddr: %08X, %s\n", data & 0xffffff, data & 0x80000000 ? "tiled" : "non-tiled");
			m_banshee.blt_regs[banshee2D_dstBaseAddr] = data;
			break;

		case banshee2D_srcSize:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_srcSize] = data;
			break;

		case banshee2D_dstSize:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstSize: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_dstSize] = data;
			break;

		case banshee2D_srcXY:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_srcXY] = data;
			break;

		case banshee2D_dstXY:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstXY: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_dstXY] = data;
			break;

		case banshee2D_srcFormat:
			if (LOG_BANSHEE_2D)
				logerror("   2D:srcFormat: str %d, fmt %d, packing %d\n", data & 0x3fff, (data >> 16) & 0xf, (data >> 22) & 0x3);
			m_banshee.blt_regs[banshee2D_srcFormat] = data;
			break;

		case banshee2D_dstFormat:
			if (LOG_BANSHEE_2D)
				logerror("   2D:dstFormat: str %d, fmt %d\n", data & 0x3fff, (data >> 16) & 0xf);
			m_banshee.blt_regs[banshee2D_dstFormat] = data;
			break;

		case banshee2D_clip0Min:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip0Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip0Min] = data;
			break;

		case banshee2D_clip0Max:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip0Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip0Max] = data;
			break;

		case banshee2D_clip1Min:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip1Min: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip1Min] = data;
			break;

		case banshee2D_clip1Max:
			if (LOG_BANSHEE_2D)
				logerror("   2D:clip1Max: %d, %d\n", data & 0xfff, (data >> 16) & 0xfff);
			m_banshee.blt_regs[banshee2D_clip1Max] = data;
			break;

		case banshee2D_rop:
			if (LOG_BANSHEE_2D)
				logerror("   2D:rop: %d, %d, %d\n",  data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff);
			m_banshee.blt_regs[banshee2D_rop] = data;
			break;

		default:
			if (offset >= 0x20 && offset < 0x40)
			{
				banshee_blit_2d(data);
			}
			else if (offset >= 0x40 && offset < 0x80)
			{
				// TODO: colorPattern
			}
			break;
	}


	return 1;
}




void voodoo_banshee_device::banshee_agp_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset &= 0x1ff/4;

	/* switch off the offset */
	switch (offset)
	{
		case cmdBaseAddr0:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[0].set_base(BIT(data, 0, 24) << 12);
			m_fbi.m_cmdfifo[0].set_size((BIT(m_banshee.agp[cmdBaseSize0], 0, 8) + 1) << 12);
			break;

		case cmdBaseSize0:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[0].set_size((BIT(m_banshee.agp[cmdBaseSize0], 0, 8) + 1) << 12);
			m_fbi.m_cmdfifo[0].set_enable(BIT(data, 8));
			m_fbi.m_cmdfifo[0].set_count_holes(!BIT(data, 10));
			break;

		case cmdBump0:
			fatalerror("cmdBump0\n");

		case cmdRdPtrL0:
			m_fbi.m_cmdfifo[0].set_read_pointer(data);
			break;

		case cmdAMin0:
			m_fbi.m_cmdfifo[0].set_address_min(data);
			break;

		case cmdAMax0:
			m_fbi.m_cmdfifo[0].set_address_max(data);
			break;

		case cmdFifoDepth0:
			m_fbi.m_cmdfifo[0].set_depth(data);
			break;

		case cmdHoleCnt0:
			m_fbi.m_cmdfifo[0].set_holes(data);
			break;

		case cmdBaseAddr1:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[1].set_base(BIT(data, 0, 24) << 12);
			m_fbi.m_cmdfifo[1].set_size((BIT(m_banshee.agp[cmdBaseSize1], 0, 8) + 1) << 12);
			break;

		case cmdBaseSize1:
			COMBINE_DATA(&m_banshee.agp[offset]);
			m_fbi.m_cmdfifo[1].set_size((BIT(m_banshee.agp[cmdBaseSize1], 0, 8) + 1) << 12);
			m_fbi.m_cmdfifo[1].set_enable(BIT(data, 8));
			m_fbi.m_cmdfifo[1].set_count_holes(!BIT(data, 10));
			break;

		case cmdBump1:
			fatalerror("cmdBump1\n");

		case cmdRdPtrL1:
			m_fbi.m_cmdfifo[1].set_read_pointer(data);
			break;

		case cmdAMin1:
			m_fbi.m_cmdfifo[1].set_address_min(data);
			break;

		case cmdAMax1:
			m_fbi.m_cmdfifo[1].set_address_max(data);
			break;

		case cmdFifoDepth1:
			m_fbi.m_cmdfifo[1].set_depth(data);
			break;

		case cmdHoleCnt1:
			m_fbi.m_cmdfifo[1].set_holes(data);
			break;

		default:
			COMBINE_DATA(&m_banshee.agp[offset]);
			break;
	}

	if (LOG_REGISTERS)
		logerror("%s:banshee_w(AGP:%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_agp_reg_name[offset], data, mem_mask);
}


void voodoo_banshee_device::banshee_w(offs_t offset, u32 data, u32 mem_mask)
{
	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < 0x80000/4)
		banshee_io_w(offset, data, mem_mask);
	else if (offset < 0x100000/4)
		banshee_agp_w(offset, data, mem_mask);
	else if (offset < 0x200000/4)
		logerror("%s:banshee_w(2D:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0xfffff, data, mem_mask);
	else if (offset < 0x600000/4)
		register_w(offset & 0x1fffff/4, data);
	else if (offset < 0x800000/4)
		logerror("%s:banshee_w(TEX0:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xa00000/4)
		logerror("%s:banshee_w(TEX1:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x1fffff, data, mem_mask);
	else if (offset < 0xc00000/4)
		logerror("%s:banshee_r(FLASH Bios ROM:%X)\n", machine().describe_context(), (offset*4) & 0x3fffff);
	else if (offset < 0x1000000/4)
		logerror("%s:banshee_w(YUV:%X) = %08X & %08X\n", machine().describe_context(), (offset*4) & 0x3fffff, data, mem_mask);
	else if (offset < 0x2000000/4)
	{
		lfb_w(offset & 0xffffff/4, data, mem_mask);
	} else {
		logerror("%s:banshee_w Address out of range %08X = %08X & %08X\n", machine().describe_context(), (offset*4), data, mem_mask);
	}
}


void voodoo_banshee_device::banshee_fb_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 addr = offset*4;

	/* if we have something pending, flush the FIFOs up to the current time */
	if (m_pci.op_pending)
		flush_fifos(machine().time());

	if (offset < m_fbi.m_lfb_base)
	{
		if (!m_fbi.m_cmdfifo[0].write_if_in_range(addr, data) && !m_fbi.m_cmdfifo[1].write_if_in_range(addr, data))
		{
			if (offset*4 <= m_fbi.m_mask)
				COMBINE_DATA(&((u32 *)m_fbi.m_ram)[offset]);
			else
				logerror("%s:banshee_fb_w Out of bounds (%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			if (LOG_LFB)
				logerror("%s:banshee_fb_w(%X) = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
		}
	}
	else
		lfb_direct_w(offset - m_fbi.m_lfb_base, data, mem_mask);
}


void voodoo_banshee_device::banshee_vga_w(offs_t offset, u8 data)
{
	offset &= 0x1f;

	/* switch off the offset */
	switch (offset + 0x3c0)
	{
		/* attribute access */
		case 0x3c0:
		case 0x3c1:
			if (m_banshee.attff == 0)
			{
				m_banshee.vga[0x3c1 & 0x1f] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			}
			else
			{
				if (m_banshee.vga[0x3c1 & 0x1f] < std::size(m_banshee.att))
					m_banshee.att[m_banshee.vga[0x3c1 & 0x1f]] = data;
				if (LOG_REGISTERS)
					logerror("%s:banshee_att_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3c1 & 0x1f], data);
			}
			m_banshee.attff ^= 1;
			break;

		/* Sequencer access */
		case 0x3c5:
			if (m_banshee.vga[0x3c4 & 0x1f] < std::size(m_banshee.seq))
				m_banshee.seq[m_banshee.vga[0x3c4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_seq_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3c4 & 0x1f], data);
			break;

		/* Graphics controller access */
		case 0x3cf:
			if (m_banshee.vga[0x3ce & 0x1f] < std::size(m_banshee.gc))
				m_banshee.gc[m_banshee.vga[0x3ce & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_gc_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3ce & 0x1f], data);
			break;

		/* CRTC access */
		case 0x3d5:
			if (m_banshee.vga[0x3d4 & 0x1f] < std::size(m_banshee.crtc))
				m_banshee.crtc[m_banshee.vga[0x3d4 & 0x1f]] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_crtc_w(%X) = %02X\n", machine().describe_context(), m_banshee.vga[0x3d4 & 0x1f], data);
			break;

		default:
			m_banshee.vga[offset] = data;
			if (LOG_REGISTERS)
				logerror("%s:banshee_vga_w(%X) = %02X\n", machine().describe_context(), 0x3c0+offset, data);
			break;
	}
}


void voodoo_banshee_device::banshee_io_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 old;

	offset &= 0xff/4;
	old = m_banshee.io[offset];

	/* switch off the offset */
	switch (offset)
	{
		case io_vidProcCfg:
			COMBINE_DATA(&m_banshee.io[offset]);
			if ((m_banshee.io[offset] ^ old) & 0x2000)
				m_fbi.m_clut_dirty = true;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_dacData:
			COMBINE_DATA(&m_banshee.io[offset]);
			if (m_banshee.io[offset] != m_fbi.m_clut[m_banshee.io[io_dacAddr] & 0x1ff])
			{
				m_fbi.m_clut[m_banshee.io[io_dacAddr] & 0x1ff] = m_banshee.io[offset];
				m_fbi.m_clut_dirty = true;
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_dac_w(%X) = %08X & %08X\n", machine().describe_context(), m_banshee.io[io_dacAddr] & 0x1ff, data, mem_mask);
			break;

		case io_miscInit0:
			m_renderer->wait(voodoo_regs::s_banshee_io_reg_name[offset]);
			COMBINE_DATA(&m_banshee.io[offset]);
			m_renderer->set_yorigin(m_fbi.m_yorigin = (data >> 18) & 0xfff);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vidScreenSize:
			if (data & 0xfff)
				m_fbi.m_width = data & 0xfff;
			if (data & 0xfff000)
				m_fbi.m_height = (data >> 12) & 0xfff;
			[[fallthrough]];
		case io_vidOverlayDudx:
		case io_vidOverlayDvdy:
		{
			COMBINE_DATA(&m_banshee.io[offset]);

			// Get horizontal total and vertical total from CRTC registers
			int htotal = (m_banshee.crtc[0] + 5) * 8;
			int vtotal = m_banshee.crtc[6];
			vtotal |= ((m_banshee.crtc[7] >> 0) & 0x1) << 8;
			vtotal |= ((m_banshee.crtc[7] >> 5) & 0x1) << 9;
			vtotal += 2;

			int vstart = m_banshee.crtc[0x10];
			vstart |= ((m_banshee.crtc[7] >> 2) & 0x1) << 8;
			vstart |= ((m_banshee.crtc[7] >> 7) & 0x1) << 9;

			int vstop = m_banshee.crtc[0x11] & 0xf;
			// Compare to see if vstop is before or after low 4 bits of vstart
			if (vstop < (vstart & 0xf))
				vstop |= (vstart + 0x10) & ~0xf;
			else
				vstop |= vstart & ~0xf;

			// Get pll k, m and n from pllCtrl0
			const u32 k = (m_banshee.io[io_pllCtrl0] >> 0) & 0x3;
			const u32 m = (m_banshee.io[io_pllCtrl0] >> 2) & 0x3f;
			const u32 n = (m_banshee.io[io_pllCtrl0] >> 8) & 0xff;
			const double video_clock = (XTAL(14'318'181) * (n + 2) / ((m + 2) << k)).dvalue();
			const double frame_period = vtotal * htotal / video_clock;
			//osd_printf_info("k: %d m: %d n: %d clock: %f period: %f rate: %.2f\n", k, m, n, video_clock, frame_period, 1.0 / frame_period);

			int width = m_fbi.m_width;
			int height = m_fbi.m_height;
			//m_fbi.m_xoffs = hbp;
			//m_fbi.m_yoffs = vbp;

			if (m_banshee.io[io_vidOverlayDudx] != 0)
				width = (m_fbi.m_width * m_banshee.io[io_vidOverlayDudx]) / 1048576;
			if (m_banshee.io[io_vidOverlayDvdy] != 0)
				height = (m_fbi.m_height * m_banshee.io[io_vidOverlayDvdy]) / 1048576;
			if (LOG_REGISTERS)
				logerror("configure screen: htotal: %d vtotal: %d vstart: %d vstop: %d width: %d height: %d refresh: %f\n",
					htotal, vtotal, vstart, vstop, width, height, 1.0 / frame_period);
			if (htotal > 0 && vtotal > 0) {
				rectangle visarea(0, width - 1, 0, height - 1);
				m_screen->configure(htotal, vtotal, visarea, DOUBLE_TO_ATTOSECONDS(frame_period));

				// Set the vsync start and stop
				m_fbi.m_vsyncstart = vstart;
				m_fbi.m_vsyncstop = vstop;
				adjust_vblank_timer();
			}
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;
		}

		case io_lfbMemoryConfig:
			m_fbi.m_lfb_base = (data & 0x1fff) << (12-2);
			m_fbi.m_lfb_stride = ((data >> 13) & 7) + 9;
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		case io_vgab0:  case io_vgab4:  case io_vgab8:  case io_vgabc:
		case io_vgac0:  case io_vgac4:  case io_vgac8:  case io_vgacc:
		case io_vgad0:  case io_vgad4:  case io_vgad8:  case io_vgadc:
			if (ACCESSING_BITS_0_7)
				banshee_vga_w(offset*4+0, data >> 0);
			if (ACCESSING_BITS_8_15)
				banshee_vga_w(offset*4+1, data >> 8);
			if (ACCESSING_BITS_16_23)
				banshee_vga_w(offset*4+2, data >> 16);
			if (ACCESSING_BITS_24_31)
				banshee_vga_w(offset*4+3, data >> 24);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;

		default:
			COMBINE_DATA(&m_banshee.io[offset]);
			if (LOG_REGISTERS)
				logerror("%s:banshee_io_w(%s) = %08X & %08X\n", machine().describe_context(), voodoo_regs::s_banshee_io_reg_name[offset], data, mem_mask);
			break;
	}
}



//-------------------------------------------------
//  fastfill - execute the 'fastfill' command
//-------------------------------------------------

s32 voodoo_device::fastfill()
{
	auto &poly = m_renderer->alloc_poly();

	// determine the draw buffer (Banshee and later are hard-coded to the back buffer)
	poly.destbase = (m_reg.rev3() || m_reg.fbz_mode().draw_buffer() == 1) ? m_fbi.back_buffer() : m_fbi.front_buffer();
	poly.depthbase = m_fbi.aux_buffer();
	poly.clipleft = m_reg.clip_left();
	poly.clipright = m_reg.clip_right();
	poly.cliptop = m_reg.clip_top();
	poly.clipbottom = m_reg.clip_bottom();
	poly.color1 = m_reg.color1().argb();
	poly.zacolor = m_reg.za_color();

	// 2 pixels per clock
	return m_renderer->enqueue_fastfill(poly) / 2;
}


//-------------------------------------------------
//  swapbuffer - execute the 'swapbuffer' command
//-------------------------------------------------

s32 voodoo_device::swapbuffer(u32 data)
{
	// set the don't swap value for Voodoo 2
	m_fbi.m_vblank_swap_pending = true;
	m_fbi.m_vblank_swap = BIT(data, 1, 8);
	m_fbi.m_vblank_dont_swap = BIT(data, 9);

	// if we're not syncing to the retrace, process the command immediately
	if (!BIT(data, 0))
	{
		swap_buffers();
		return 0;
	}

	// determine how many cycles to wait; we deliberately overshoot here because
	// the final count gets updated on the VBLANK
	return (m_fbi.m_vblank_swap + 1) * clock() / 10;
}


//-------------------------------------------------
//  triangle - execute the 'triangle' command
//-------------------------------------------------

s32 voodoo_device::triangle()
{
	g_profiler.start(PROFILER_USER2);

	// allocate polygon information now
	auto &poly = m_renderer->alloc_poly();

	// perform subpixel adjustments -- note that the documentation indicates this
	// is done in the internal registers, so do it there
	if (m_reg.fbz_colorpath().cca_subpixel_adjust())
	{
		s32 dx = 8 - (m_fbi.m_ax & 15);
		s32 dy = 8 - (m_fbi.m_ay & 15);

		// adjust iterated R,G,B,A and W/Z
		m_fbi.m_startr += (dy * m_fbi.m_drdy + dx * m_fbi.m_drdx) >> 4;
		m_fbi.m_startg += (dy * m_fbi.m_dgdy + dx * m_fbi.m_dgdx) >> 4;
		m_fbi.m_startb += (dy * m_fbi.m_dbdy + dx * m_fbi.m_dbdx) >> 4;
		m_fbi.m_starta += (dy * m_fbi.m_dady + dx * m_fbi.m_dadx) >> 4;
		m_fbi.m_startw += (dy * m_fbi.m_dwdy + dx * m_fbi.m_dwdx) >> 4;
		m_fbi.m_startz += mul_32x32_shift(dy, m_fbi.m_dzdy, 4) + mul_32x32_shift(dx, m_fbi.m_dzdx, 4);

		// adjust iterated W/S/T for TMU 0
		m_tmu[0].m_startw += (dy * m_tmu[0].m_dwdy + dx * m_tmu[0].m_dwdx) >> 4;
		m_tmu[0].m_starts += (dy * m_tmu[0].m_dsdy + dx * m_tmu[0].m_dsdx) >> 4;
		m_tmu[0].m_startt += (dy * m_tmu[0].m_dtdy + dx * m_tmu[0].m_dtdx) >> 4;

		// adjust iterated W/S/T for TMU 1
		if (BIT(m_chipmask, 2))
		{
			m_tmu[1].m_startw += (dy * m_tmu[1].m_dwdy + dx * m_tmu[1].m_dwdx) >> 4;
			m_tmu[1].m_starts += (dy * m_tmu[1].m_dsdy + dx * m_tmu[1].m_dsdx) >> 4;
			m_tmu[1].m_startt += (dy * m_tmu[1].m_dtdy + dx * m_tmu[1].m_dtdx) >> 4;
		}
	}

	// determine the draw buffer (Banshee and later are hard-coded to the back buffer)
	poly.destbase = (m_reg.rev3() || m_reg.fbz_mode().draw_buffer() == 1) ? m_fbi.back_buffer() : m_fbi.front_buffer();
	poly.depthbase = m_fbi.aux_buffer();
	poly.clipleft = m_reg.clip_left();
	poly.clipright = m_reg.clip_right();
	poly.cliptop = m_reg.clip_top();
	poly.clipbottom = m_reg.clip_bottom();

	// fill in triangle parameters
	poly.ax = m_fbi.m_ax;
	poly.ay = m_fbi.m_ay;
	poly.startr = m_fbi.m_startr;
	poly.startg = m_fbi.m_startg;
	poly.startb = m_fbi.m_startb;
	poly.starta = m_fbi.m_starta;
	poly.startz = m_fbi.m_startz;
	poly.startw = m_fbi.m_startw;
	poly.drdx = m_fbi.m_drdx;
	poly.dgdx = m_fbi.m_dgdx;
	poly.dbdx = m_fbi.m_dbdx;
	poly.dadx = m_fbi.m_dadx;
	poly.dzdx = m_fbi.m_dzdx;
	poly.dwdx = m_fbi.m_dwdx;
	poly.drdy = m_fbi.m_drdy;
	poly.dgdy = m_fbi.m_dgdy;
	poly.dbdy = m_fbi.m_dbdy;
	poly.dady = m_fbi.m_dady;
	poly.dzdy = m_fbi.m_dzdy;
	poly.dwdy = m_fbi.m_dwdy;

	// fill in texture 0 parameters
	poly.tex0 = nullptr;
	if (poly.raster.texmode0().raw() != 0xffffffff)
	{
		poly.starts0 = m_tmu[0].m_starts;
		poly.startt0 = m_tmu[0].m_startt;
		poly.startw0 = m_tmu[0].m_startw;
		poly.ds0dx = m_tmu[0].m_dsdx;
		poly.dt0dx = m_tmu[0].m_dtdx;
		poly.dw0dx = m_tmu[0].m_dwdx;
		poly.ds0dy = m_tmu[0].m_dsdy;
		poly.dt0dy = m_tmu[0].m_dtdy;
		poly.dw0dy = m_tmu[0].m_dwdy;
		poly.lodbase0 = m_tmu[0].compute_lodbase();
		poly.tex0 = &m_tmu[0].prepare_texture();
		if (DEBUG_STATS)
			m_stats.texture_mode[m_tmu[0].m_reg.texture_mode().format()]++;
	}

	// fill in texture 1 parameters
	poly.tex1 = nullptr;
	if (poly.raster.texmode1().raw() != 0xffffffff)
	{
		poly.starts1 = m_tmu[1].m_starts;
		poly.startt1 = m_tmu[1].m_startt;
		poly.startw1 = m_tmu[1].m_startw;
		poly.ds1dx = m_tmu[1].m_dsdx;
		poly.dt1dx = m_tmu[1].m_dtdx;
		poly.dw1dx = m_tmu[1].m_dwdx;
		poly.ds1dy = m_tmu[1].m_dsdy;
		poly.dt1dy = m_tmu[1].m_dtdy;
		poly.dw1dy = m_tmu[1].m_dwdy;
		poly.lodbase1 = m_tmu[1].compute_lodbase();
		poly.tex1 = &m_tmu[1].prepare_texture();
		if (DEBUG_STATS)
			m_stats.texture_mode[m_tmu[1].m_reg.texture_mode().format()]++;
	}

	// fill in color parameters
	poly.color0 = m_reg.color0().argb();
	poly.color1 = m_reg.color1().argb();
	poly.chromakey = m_reg.chroma_key().argb();
	poly.fogcolor = m_reg.fog_color().argb();
	poly.zacolor = m_reg.za_color();
	poly.stipple = m_reg.stipple();
	poly.alpharef = m_reg.alpha_mode().alpharef();

	// fill in the vertex data
	voodoo_renderer::vertex_t vert[3];
	vert[0].x = float(m_fbi.m_ax) * (1.0f / 16.0f);
	vert[0].y = float(m_fbi.m_ay) * (1.0f / 16.0f);
	vert[1].x = float(m_fbi.m_bx) * (1.0f / 16.0f);
	vert[1].y = float(m_fbi.m_by) * (1.0f / 16.0f);
	vert[2].x = float(m_fbi.m_cx) * (1.0f / 16.0f);
	vert[2].y = float(m_fbi.m_cy) * (1.0f / 16.0f);

	// enqueue a triangle
	s32 pixels = m_renderer->enqueue_triangle(poly, vert);

	// update stats
	m_reg.add(voodoo_regs::reg_fbiTrianglesOut, 1);
	if (DEBUG_STATS)
		m_stats.total_triangles++;

	g_profiler.stop();

	if (LOG_REGISTERS)
		logerror("cycles = %d\n", TRIANGLE_SETUP_CLOCKS + pixels);

	// 1 pixel per clock, plus some setup time
	return TRIANGLE_SETUP_CLOCKS + pixels;
}


//-------------------------------------------------
//  begin_triangle - execute the 'beginTri'
//  command
//-------------------------------------------------

s32 voodoo_device::begin_triangle()
{
	// extract all the data from registers
	auto &sv = m_fbi.m_svert[2];
	sv.x  = m_reg.read_float(voodoo_regs::reg_sVx);
	sv.y  = m_reg.read_float(voodoo_regs::reg_sVy);
	sv.wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv.w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv.s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv.t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv.w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv.s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv.t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv.a  = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv.r  = m_reg.read_float(voodoo_regs::reg_sRed);
	sv.g  = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv.b  = m_reg.read_float(voodoo_regs::reg_sBlue);

	// spread it across all three verts and reset the count
	m_fbi.m_svert[0] = m_fbi.m_svert[1] = m_fbi.m_svert[2];
	m_fbi.m_sverts = 1;
	return 0;
}


//-------------------------------------------------
//  draw_triangle - execute the 'DrawTri'
//  command
//-------------------------------------------------

s32 voodoo_device::draw_triangle()
{
	// for strip mode, shuffle vertex 1 down to 0
	if (!m_reg.setup_mode().fan_mode())
		m_fbi.m_svert[0] = m_fbi.m_svert[1];

	// copy 2 down to 1 regardless
	m_fbi.m_svert[1] = m_fbi.m_svert[2];

	// extract all the data from registers
	auto &sv = m_fbi.m_svert[2];
	sv.x  = m_reg.read_float(voodoo_regs::reg_sVx);
	sv.y  = m_reg.read_float(voodoo_regs::reg_sVy);
	sv.wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv.w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv.s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv.t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv.w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv.s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv.t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv.a  = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv.r  = m_reg.read_float(voodoo_regs::reg_sRed);
	sv.g  = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv.b  = m_reg.read_float(voodoo_regs::reg_sBlue);

	// if we have enough verts, go ahead and draw
	int cycles = 0;
	if (++m_fbi.m_sverts >= 3)
		cycles = setup_and_draw_triangle();
	return cycles;
}



/***************************************************************************
    TRIANGLE HELPERS
***************************************************************************/

/*-------------------------------------------------
    setup_and_draw_triangle - process the setup
    parameters and render the triangle
-------------------------------------------------*/

s32 voodoo_device::setup_and_draw_triangle()
{
	/* compute the divisor */
	// Just need sign for now
	float divisor = ((m_fbi.m_svert[0].x - m_fbi.m_svert[1].x) * (m_fbi.m_svert[0].y - m_fbi.m_svert[2].y) -
					 (m_fbi.m_svert[0].x - m_fbi.m_svert[2].x) * (m_fbi.m_svert[0].y - m_fbi.m_svert[1].y));

	/* backface culling */
	auto const setup_mode = m_reg.setup_mode();
	if (setup_mode.enable_culling() & 0x20000)
	{
		int culling_sign = setup_mode.culling_sign();
		int divisor_sign = (divisor < 0);

		/* if doing strips and ping pong is enabled, apply the ping pong */
		if (!setup_mode.fan_mode() && !setup_mode.disable_ping_pong_correction())
			culling_sign ^= (m_fbi.m_sverts - 3) & 1;

		/* if our sign matches the culling sign, we're done for */
		if (divisor_sign == culling_sign)
			return TRIANGLE_SETUP_CLOCKS;
	}

	// Finish the divisor
	divisor = 1.0f / divisor;

	/* grab the X/Ys at least */
	m_fbi.m_ax = s16(m_fbi.m_svert[0].x * 16.0f);
	m_fbi.m_ay = s16(m_fbi.m_svert[0].y * 16.0f);
	m_fbi.m_bx = s16(m_fbi.m_svert[1].x * 16.0f);
	m_fbi.m_by = s16(m_fbi.m_svert[1].y * 16.0f);
	m_fbi.m_cx = s16(m_fbi.m_svert[2].x * 16.0f);
	m_fbi.m_cy = s16(m_fbi.m_svert[2].y * 16.0f);

	/* compute the dx/dy values */
	float dx1 = m_fbi.m_svert[0].y - m_fbi.m_svert[2].y;
	float dx2 = m_fbi.m_svert[0].y - m_fbi.m_svert[1].y;
	float dy1 = m_fbi.m_svert[0].x - m_fbi.m_svert[1].x;
	float dy2 = m_fbi.m_svert[0].x - m_fbi.m_svert[2].x;

	/* set up R,G,B */
	float tdiv = divisor * 4096.0f;
	if (setup_mode.setup_rgb())
	{
		m_fbi.m_startr = (s32)(m_fbi.m_svert[0].r * 4096.0f);
		m_fbi.m_drdx = (s32)(((m_fbi.m_svert[0].r - m_fbi.m_svert[1].r) * dx1 - (m_fbi.m_svert[0].r - m_fbi.m_svert[2].r) * dx2) * tdiv);
		m_fbi.m_drdy = (s32)(((m_fbi.m_svert[0].r - m_fbi.m_svert[2].r) * dy1 - (m_fbi.m_svert[0].r - m_fbi.m_svert[1].r) * dy2) * tdiv);
		m_fbi.m_startg = (s32)(m_fbi.m_svert[0].g * 4096.0f);
		m_fbi.m_dgdx = (s32)(((m_fbi.m_svert[0].g - m_fbi.m_svert[1].g) * dx1 - (m_fbi.m_svert[0].g - m_fbi.m_svert[2].g) * dx2) * tdiv);
		m_fbi.m_dgdy = (s32)(((m_fbi.m_svert[0].g - m_fbi.m_svert[2].g) * dy1 - (m_fbi.m_svert[0].g - m_fbi.m_svert[1].g) * dy2) * tdiv);
		m_fbi.m_startb = (s32)(m_fbi.m_svert[0].b * 4096.0f);
		m_fbi.m_dbdx = (s32)(((m_fbi.m_svert[0].b - m_fbi.m_svert[1].b) * dx1 - (m_fbi.m_svert[0].b - m_fbi.m_svert[2].b) * dx2) * tdiv);
		m_fbi.m_dbdy = (s32)(((m_fbi.m_svert[0].b - m_fbi.m_svert[2].b) * dy1 - (m_fbi.m_svert[0].b - m_fbi.m_svert[1].b) * dy2) * tdiv);
	}

	/* set up alpha */
	if (setup_mode.setup_alpha())
	{
		m_fbi.m_starta = (s32)(m_fbi.m_svert[0].a * 4096.0f);
		m_fbi.m_dadx = (s32)(((m_fbi.m_svert[0].a - m_fbi.m_svert[1].a) * dx1 - (m_fbi.m_svert[0].a - m_fbi.m_svert[2].a) * dx2) * tdiv);
		m_fbi.m_dady = (s32)(((m_fbi.m_svert[0].a - m_fbi.m_svert[2].a) * dy1 - (m_fbi.m_svert[0].a - m_fbi.m_svert[1].a) * dy2) * tdiv);
	}

	/* set up Z */
	if (setup_mode.setup_z())
	{
		m_fbi.m_startz = (s32)(m_fbi.m_svert[0].z * 4096.0f);
		m_fbi.m_dzdx = (s32)(((m_fbi.m_svert[0].z - m_fbi.m_svert[1].z) * dx1 - (m_fbi.m_svert[0].z - m_fbi.m_svert[2].z) * dx2) * tdiv);
		m_fbi.m_dzdy = (s32)(((m_fbi.m_svert[0].z - m_fbi.m_svert[2].z) * dy1 - (m_fbi.m_svert[0].z - m_fbi.m_svert[1].z) * dy2) * tdiv);
	}

	/* set up Wb */
	tdiv = divisor * 65536.0f * 65536.0f;
	if (setup_mode.setup_wb())
	{
		m_fbi.m_startw = m_tmu[0].m_startw = m_tmu[1].m_startw = (s64)(m_fbi.m_svert[0].wb * 65536.0f * 65536.0f);
		m_fbi.m_dwdx = m_tmu[0].m_dwdx = m_tmu[1].m_dwdx = ((m_fbi.m_svert[0].wb - m_fbi.m_svert[1].wb) * dx1 - (m_fbi.m_svert[0].wb - m_fbi.m_svert[2].wb) * dx2) * tdiv;
		m_fbi.m_dwdy = m_tmu[0].m_dwdy = m_tmu[1].m_dwdy = ((m_fbi.m_svert[0].wb - m_fbi.m_svert[2].wb) * dy1 - (m_fbi.m_svert[0].wb - m_fbi.m_svert[1].wb) * dy2) * tdiv;
	}

	/* set up W0 */
	if (setup_mode.setup_w0())
	{
		m_tmu[0].m_startw = m_tmu[1].m_startw = (s64)(m_fbi.m_svert[0].w0 * 65536.0f * 65536.0f);
		m_tmu[0].m_dwdx = m_tmu[1].m_dwdx = ((m_fbi.m_svert[0].w0 - m_fbi.m_svert[1].w0) * dx1 - (m_fbi.m_svert[0].w0 - m_fbi.m_svert[2].w0) * dx2) * tdiv;
		m_tmu[0].m_dwdy = m_tmu[1].m_dwdy = ((m_fbi.m_svert[0].w0 - m_fbi.m_svert[2].w0) * dy1 - (m_fbi.m_svert[0].w0 - m_fbi.m_svert[1].w0) * dy2) * tdiv;
	}

	/* set up S0,T0 */
	if (setup_mode.setup_st0())
	{
		m_tmu[0].m_starts = m_tmu[1].m_starts = (s64)(m_fbi.m_svert[0].s0 * 65536.0f * 65536.0f);
		m_tmu[0].m_dsdx = m_tmu[1].m_dsdx = ((m_fbi.m_svert[0].s0 - m_fbi.m_svert[1].s0) * dx1 - (m_fbi.m_svert[0].s0 - m_fbi.m_svert[2].s0) * dx2) * tdiv;
		m_tmu[0].m_dsdy = m_tmu[1].m_dsdy = ((m_fbi.m_svert[0].s0 - m_fbi.m_svert[2].s0) * dy1 - (m_fbi.m_svert[0].s0 - m_fbi.m_svert[1].s0) * dy2) * tdiv;
		m_tmu[0].m_startt = m_tmu[1].m_startt = (s64)(m_fbi.m_svert[0].t0 * 65536.0f * 65536.0f);
		m_tmu[0].m_dtdx = m_tmu[1].m_dtdx = ((m_fbi.m_svert[0].t0 - m_fbi.m_svert[1].t0) * dx1 - (m_fbi.m_svert[0].t0 - m_fbi.m_svert[2].t0) * dx2) * tdiv;
		m_tmu[0].m_dtdy = m_tmu[1].m_dtdy = ((m_fbi.m_svert[0].t0 - m_fbi.m_svert[2].t0) * dy1 - (m_fbi.m_svert[0].t0 - m_fbi.m_svert[1].t0) * dy2) * tdiv;
	}

	/* set up W1 */
	if (setup_mode.setup_w1())
	{
		m_tmu[1].m_startw = (s64)(m_fbi.m_svert[0].w1 * 65536.0f * 65536.0f);
		m_tmu[1].m_dwdx = ((m_fbi.m_svert[0].w1 - m_fbi.m_svert[1].w1) * dx1 - (m_fbi.m_svert[0].w1 - m_fbi.m_svert[2].w1) * dx2) * tdiv;
		m_tmu[1].m_dwdy = ((m_fbi.m_svert[0].w1 - m_fbi.m_svert[2].w1) * dy1 - (m_fbi.m_svert[0].w1 - m_fbi.m_svert[1].w1) * dy2) * tdiv;
	}

	/* set up S1,T1 */
	if (setup_mode.setup_st1())
	{
		m_tmu[1].m_starts = (s64)(m_fbi.m_svert[0].s1 * 65536.0f * 65536.0f);
		m_tmu[1].m_dsdx = ((m_fbi.m_svert[0].s1 - m_fbi.m_svert[1].s1) * dx1 - (m_fbi.m_svert[0].s1 - m_fbi.m_svert[2].s1) * dx2) * tdiv;
		m_tmu[1].m_dsdy = ((m_fbi.m_svert[0].s1 - m_fbi.m_svert[2].s1) * dy1 - (m_fbi.m_svert[0].s1 - m_fbi.m_svert[1].s1) * dy2) * tdiv;
		m_tmu[1].m_startt = (s64)(m_fbi.m_svert[0].t1 * 65536.0f * 65536.0f);
		m_tmu[1].m_dtdx = ((m_fbi.m_svert[0].t1 - m_fbi.m_svert[1].t1) * dx1 - (m_fbi.m_svert[0].t1 - m_fbi.m_svert[2].t1) * dx2) * tdiv;
		m_tmu[1].m_dtdy = ((m_fbi.m_svert[0].t1 - m_fbi.m_svert[2].t1) * dy1 - (m_fbi.m_svert[0].t1 - m_fbi.m_svert[1].t1) * dy2) * tdiv;
	}

	/* draw the triangle */
	return triangle();
}



//-------------------------------------------------
//  voodoo_device - constructor
//-------------------------------------------------

voodoo_device::voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model) :
	device_t(mconfig, type, tag, owner, clock),
	m_model(model),
	m_chipmask(0),
	m_fbmem_in_mb(0),
	m_tmumem0_in_mb(0),
	m_tmumem1_in_mb(0),
	m_reg(model),
	m_trigger(0),
	m_last_status_pc(0),
	m_last_status_value(0),
	m_vblank(*this),
	m_stall(*this),
	m_pciint(*this),
	m_fbi(*this),
	m_tmu{ *this, *this },
	m_screen(*this, finder_base::DUMMY_TAG),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_shared(new shared_tables)
{
}


//-------------------------------------------------
//  ~voodoo_device - destructor
//-------------------------------------------------

voodoo_device::~voodoo_device()
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void voodoo_device::device_start()
{
	// validate configuration
	if (m_fbmem_in_mb == 0)
		fatalerror("Invalid Voodoo memory configuration");
	if (m_reg.rev1_or_2() && m_tmumem0_in_mb == 0)
		fatalerror("Invalid Voodoo memory configuration");

	// determine our index within the system, then set our trigger
	u32 index = 0;
	for (device_t &scan : device_enumerator(machine().root_device()))
		if (scan.type() == this->type())
		{
			if (&scan == this)
				break;
			index++;
		}
	m_trigger = 51324 + index;

	// resolve callbacks
	m_vblank.resolve();
	m_stall.resolve();
	m_pciint.resolve();

	// allocate timers
	m_vsync_stop_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::vblank_off_callback), this), this);
	m_vsync_start_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::vblank_callback),this), this);

	// initialize helpers
	voodoo::dither_helper::init_static();

	// allocate memory
	m_fbmem.resize(m_fbmem_in_mb << 20);
	m_chipmask = 0x03;
	if (m_reg.rev1_or_2())
	{
		// Voodoo 1/2 have separate framebuffer and texture RAM
		m_tmumem[0].resize(m_tmumem0_in_mb << 20);
		m_tmumem[1].resize(m_tmumem1_in_mb << 20);
		m_chipmask |= (m_tmumem1_in_mb != 0) ? 0x04 : 0x00;
	}
	else
	{
		// Voodoo Banshee/3 have shared RAM; Voodoo 3 has two TMUs
		m_chipmask |= (m_model == MODEL_VOODOO_3) ? 0x04 : 0x00;
	}

	// initialize the frame buffer
	m_fbi.init(m_model, m_fbmem);

	// initialize the TMUs
	m_tmu[0].init(0, *m_shared.get(), m_tmumem[0].empty() ? m_fbmem : m_tmumem[0]);
	if (BIT(m_chipmask, 2))
		m_tmu[1].init(1, *m_shared.get(), m_tmumem[1].empty() ? m_fbmem : m_tmumem[1]);

	// determine the TMU configuration flags
	u16 tmu_config = 0x11;   // revision 1
	if (m_reg.rev2())
		tmu_config |= 0x800;
	if (BIT(m_chipmask, 2))
		tmu_config |= 0xc0;  // two TMUs

	// create the renderer
	m_renderer = std::make_unique<voodoo_renderer>(machine(), tmu_config, m_shared->rgb565, m_reg, &m_tmu[0].m_reg, BIT(m_chipmask, 2) ? &m_tmu[1].m_reg : nullptr);

	// set up the PCI FIFO
	m_pci.fifo.configure(m_pci.fifo_mem, 64*2);
	m_pci.stall_state = NOT_STALLED;
	m_pci.continue_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_device::stall_cpu_callback), this));

	// initialize registers
	m_pci.init_enable = 0;
	m_reg.write(voodoo_regs::reg_fbiInit0, (1 << 4) | (0x10 << 6));
	m_reg.write(voodoo_regs::reg_fbiInit1, (1 << 1) | (1 << 8) | (1 << 12) | (2 << 20));
	m_reg.write(voodoo_regs::reg_fbiInit2, (1 << 6) | (0x100 << 23));
	m_reg.write(voodoo_regs::reg_fbiInit3, (2 << 13) | (0xf << 17));
	m_reg.write(voodoo_regs::reg_fbiInit4, (1 << 0));

	// do a soft reset to reset everything else
	soft_reset();

	// register for save states
	register_save();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void voodoo_device::device_reset()
{
	soft_reset();
}


//-------------------------------------------------
//  device_post_load - update after loading save
//  state
//-------------------------------------------------

void voodoo_device::device_post_load()
{
	// dirty everything so it gets recomputed
	m_fbi.m_clut_dirty = true;
	for (tmu_state &tm : m_tmu)
	{
		tm.m_regdirty = true;
		tm.m_palette_dirty[0] = tm.m_palette_dirty[1] = tm.m_palette_dirty[2] = tm.m_palette_dirty[3] = true;
	}

	// recompute video memory to get the FBI FIFO base recomputed
	if (m_reg.rev1_or_2())
	{
		m_fbi.recompute_video_memory(m_reg);
		m_renderer->set_rowpixels(m_fbi.m_rowpixels);
	}
}




DEFINE_DEVICE_TYPE(VOODOO_1, voodoo_1_device, "voodoo_1", "3dfx Voodoo Graphics")

voodoo_1_device::voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_device(mconfig, VOODOO_1, tag, owner, clock, MODEL_VOODOO_1)
{
}


DEFINE_DEVICE_TYPE(VOODOO_2, voodoo_2_device, "voodoo_2", "3dfx Voodoo 2")

voodoo_2_device::voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_device(mconfig, VOODOO_2, tag, owner, clock, MODEL_VOODOO_2)
{
}


DEFINE_DEVICE_TYPE(VOODOO_BANSHEE, voodoo_banshee_device, "voodoo_banshee", "3dfx Voodoo Banshee")

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_BANSHEE, tag, owner, clock, MODEL_VOODOO_BANSHEE)
{
}

voodoo_banshee_device::voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model)
	: voodoo_device(mconfig, type, tag, owner, clock, model)
{
}


void voodoo_banshee_device::device_start()
{
	// initialize banshee registers
	memset(m_banshee.io, 0, sizeof(m_banshee.io));
	m_banshee.io[io_pciInit0] = 0x01800040;
	m_banshee.io[io_sipMonitor] = 0x40000000;
	m_banshee.io[io_lfbMemoryConfig] = 0x000a2200;
	m_banshee.io[io_dramInit0] = 0x00579d29;
	if (m_fbmem_in_mb == 16)
		m_banshee.io[io_dramInit0] |= 0x0c000000;      // Midway Vegas (denver) expects 2 banks of 16MBit SGRAMs
	else
		m_banshee.io[io_dramInit0] |= 0x08000000;      // Konami Viper expects 16MBit SGRAMs
	m_banshee.io[io_dramInit1] = 0x00f02200;
	m_banshee.io[io_tmuGbeInit] = 0x00000bfb;

	// call our parent
	voodoo_device::device_start();

	/* register states: banshee */
	save_item(NAME(m_banshee.io));
	save_item(NAME(m_banshee.agp));
	save_item(NAME(m_banshee.vga));
	save_item(NAME(m_banshee.crtc));
	save_item(NAME(m_banshee.seq));
	save_item(NAME(m_banshee.gc));
	save_item(NAME(m_banshee.att));
	save_item(NAME(m_banshee.attff));
}


DEFINE_DEVICE_TYPE(VOODOO_3, voodoo_3_device, "voodoo_3", "3dfx Voodoo 3")

voodoo_3_device::voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: voodoo_banshee_device(mconfig, VOODOO_3, tag, owner, clock, MODEL_VOODOO_3)
{
}

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

/*
Revision sensitivity:

rasterizer_texture::recompute()
 - addr shift/mask = 3/0xfffff (1/2) vs 0/0xfffff0 (3)
 - bilinear mask - 0xf0 (1) vs 0xff (2/3)

voodoo_renderer::voodoo_renderer()
 - fogdelta_mask = 0xff (1) vs 0xfc (2/3)

todo:
 - size the CLUT properly

*/

#include "emu.h"
#include "screen.h"
#include "voodoo.h"

#include "voodoo_regs.ipp"

using namespace voodoo;


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
//  SHARED TABLES
//**************************************************************************

//-------------------------------------------------
//  shared_tables - constructor
//-------------------------------------------------

shared_tables::shared_tables()
{
	// configure the array of texel formats
	texel[0] = rgb332;
	texel[1] = nullptr;
	texel[2] = alpha8;
	texel[3] = int8;
	texel[4] = ai44;
	texel[5] = nullptr;
	texel[6] = nullptr;
	texel[7] = nullptr;
	texel[8] = rgb332;
	texel[9] = nullptr;
	texel[10] = rgb565;
	texel[11] = argb1555;
	texel[12] = argb4444;
	texel[13] = int8;
	texel[14] = nullptr;
	texel[15] = nullptr;

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


//**************************************************************************
//  TMU STATE
//**************************************************************************

//-------------------------------------------------
//  tmu_state - constructor
//-------------------------------------------------

tmu_state::tmu_state(voodoo_model model) :
	m_reg(model)
{
}


//-------------------------------------------------
//  init - configure local state
//-------------------------------------------------

void tmu_state::init(int index, shared_tables const &share, u8 *ram, u32 size)
{
	// configure texture RAM
	m_index = index;
	m_ram = ram;
	m_mask = size - 1;
	m_regdirty = true;
	m_palette_dirty[0] = m_palette_dirty[1] = m_palette_dirty[2] = m_palette_dirty[3] = true;
	m_texel_lookup = &share.texel[0];
}


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void tmu_state::register_save(save_proxy &save)
{
	// register state
	save.save_class(NAME(m_reg));
	save.save_item(NAME(m_palette));
}


//-------------------------------------------------
//  post_load - mark everything dirty following a
//  state load
//-------------------------------------------------

void tmu_state::post_load()
{
	m_regdirty = true;
	m_palette_dirty[0] = m_palette_dirty[1] = m_palette_dirty[2] = m_palette_dirty[3] = true;
}


//-------------------------------------------------
//  ncc_w - handle a write to the NCC/palette
//  registers
//-------------------------------------------------

void tmu_state::ncc_w(offs_t regnum, u32 data)
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
		return;
	}

	// if no delta, don't mark dirty
	if (m_reg.read(regnum) == data)
		return;

	// write the updated data and mark dirty
	m_reg.write(regnum, data);
	m_palette_dirty[2 + regindex / 12] = true;
}


//-------------------------------------------------
//  prepare_texture - handle updating the texture
//  state if the texture configuration is dirty
//-------------------------------------------------

inline rasterizer_texture &tmu_state::prepare_texture(voodoo_renderer &renderer)
{
	// if the texture parameters are dirty, update them
	if (m_regdirty)
	{
		// determine the lookup
		auto const texmode = m_reg.texture_mode();
		u32 const texformat = texmode.format();
		rgb_t const *lookup = m_texel_lookup[texformat];

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



//**************************************************************************
//  MEMORY FIFO
//**************************************************************************

//-------------------------------------------------
//  memory_fifo - constructor
//-------------------------------------------------

memory_fifo::memory_fifo() :
	m_base(nullptr),
	m_size(0),
	m_in(0),
	m_out(0)
{
}


//-------------------------------------------------
//  configure - set the base/size and reset
//-------------------------------------------------

void memory_fifo::configure(u32 *base, u32 size)
{
	m_base = base;
	m_size = size;
	reset();
}


//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void memory_fifo::register_save(save_proxy &save)
{
	save.save_item(NAME(m_size));
	save.save_item(NAME(m_in));
	save.save_item(NAME(m_out));
}


//-------------------------------------------------
//  add - append an item to the fifo
//-------------------------------------------------

inline void memory_fifo::add(u32 data)
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

inline u32 memory_fifo::remove()
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


//**************************************************************************
//  DEBUG STATS
//**************************************************************************

//-------------------------------------------------
//  debug_stats - constructor
//-------------------------------------------------

debug_stats::debug_stats() :
	m_lastkey(false),
	m_display(false)
{
	reset();
}


//-------------------------------------------------
//  add_emulation_stats - add in statistics from
//  the emulation stats
//-------------------------------------------------

void debug_stats::add_emulation_stats(thread_stats_block const &block)
{
	m_pixels_in += block.pixels_in;
	m_pixels_out += block.pixels_out;
	m_chroma_fail += block.chroma_fail;
	m_zfunc_fail += block.zfunc_fail;
	m_afunc_fail += block.afunc_fail;
	m_clipped += block.clip_fail;
	m_stippled += block.stipple_count;
}


//-------------------------------------------------
//  reset - reset per-swap statistics
//-------------------------------------------------

void debug_stats::reset()
{
	m_stalls = 0;
	m_triangles = 0;
	m_pixels_in = 0;
	m_pixels_out = 0;
	m_chroma_fail = 0;
	m_zfunc_fail = 0;
	m_afunc_fail = 0;
	m_clipped = 0;
	m_stippled = 0;
	m_reg_writes = 0;
	m_reg_reads = 0;
	m_lfb_writes = 0;
	m_lfb_reads = 0;
	m_tex_writes = 0;
	std::fill_n(&m_texture_mode[0], std::size(m_texture_mode), 0);
}


//-------------------------------------------------
//  update_string - compute the string to display
//  all the statistics
//-------------------------------------------------

void debug_stats::update_string(rectangle const &visarea, u32 swap_history)
{
	// create a string of texture modes used
	char texmodes[17] = { 0 };
	char *texptr = &texmodes[0];
	for (int mode = 0; mode < 16; mode++)
		if (m_texture_mode[mode])
			*texptr++ = "0123456789ABCDEF"[mode];
	*texptr = 0;

	// build the string
	m_string = string_format("Swap:%6d\n"
							 "Hist:%08X\n"
							 "Stal:%6d\n"
							 "Rend:%6d%%\n"
							 "Poly:%6d\n"
							 "PxIn:%6d\n"
							 "POut:%6d\n"
							 "Clip:%6d\n"
							 "Stip:%6d\n"
							 "Chro:%6d\n"
							 "ZFun:%6d\n"
							 "AFun:%6d\n"
							 "RegW:%6d\n"
							 "RegR:%6d\n"
							 "LFBW:%6d\n"
							 "LFBR:%6d\n"
							 "TexW:%6d\n"
							 "TexM:%s",
							 m_swaps, swap_history, m_stalls, m_pixels_out * 100 / (visarea.width() * visarea.height()),
							 m_triangles, m_pixels_in, m_pixels_out, m_clipped, m_stippled,
							 m_chroma_fail, m_zfunc_fail, m_afunc_fail,
							 m_reg_writes, m_reg_reads, m_lfb_writes, m_lfb_reads, m_tex_writes, texmodes);
}


//-------------------------------------------------
//  update_display_state - based on the current key
//  state, update and return whether stats should
//  be shown
//-------------------------------------------------

bool debug_stats::update_display_state(bool key_pressed)
{
	if (key_pressed && key_pressed != m_lastkey)
		m_display = !m_display;
	m_lastkey = key_pressed;
	return m_display;
}



//**************************************************************************
//  GENERIC VOODOO DEVICE
//**************************************************************************

//-------------------------------------------------
//  generic_voodoo_device - constructor
//-------------------------------------------------

generic_voodoo_device::generic_voodoo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model) :
	device_t(mconfig, type, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_model(model),
	m_fbmem_in_mb(0),
	m_tmumem0_in_mb(0),
	m_tmumem1_in_mb(0),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_vblank_cb(*this),
	m_stall_cb(*this),
	m_pciint_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void generic_voodoo_device::device_start()
{
	// resolve callbacks
	m_vblank_cb.resolve();
	m_stall_cb.resolve();
	m_pciint_cb.resolve();
}


//**************************************************************************
//  VOODOO 1 DEVICE
//**************************************************************************

//-------------------------------------------------
//  voodoo_1_device - constructor
//-------------------------------------------------

voodoo_1_device::voodoo_1_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model) :
	generic_voodoo_device(mconfig, type, tag, owner, clock, model),
	m_chipmask(1),
	m_init_enable(0),
	m_stall_state(NOT_STALLED),
	m_stall_trigger(0),
	m_operation_end(attotime::zero),
	m_flush_flag(false),
	m_fbram(nullptr),
	m_fbmask(0),
	m_rgboffs{ u32(~0), u32(~0), u32(~0) },
	m_auxoffs(~0),
	m_frontbuf(0),
	m_backbuf(1),
	m_video_changed(true),
	m_lfb_base(0),
	m_lfb_stride(0),
	m_width(512),
	m_height(384),
	m_xoffs(0),
	m_yoffs(0),
	m_vsyncstart(0),
	m_vsyncstop(0),
	m_swaps_pending(0),
	m_vblank(0),
	m_vblank_count(0),
	m_vblank_swap_pending(0),
	m_vblank_swap(0),
	m_vblank_dont_swap(0),
	m_reg(model),
	m_tmu{ model, model },
	m_vsync_start_timer(nullptr),
	m_vsync_stop_timer(nullptr),
	m_stall_resume_timer(nullptr),
	m_last_status_pc(0),
	m_last_status_value(0),
	m_clut_dirty(true)
{
	for (int index = 0; index < std::size(m_regtable); index++)
		m_regtable[index].unpack(s_register_table[index], *this);
}


//-------------------------------------------------
//  ~voodoo_1_device - destructor
//-------------------------------------------------

voodoo_1_device::~voodoo_1_device()
{
}


//-------------------------------------------------
//  core_map - device map for core memory access
//-------------------------------------------------

void voodoo_1_device::core_map(address_map &map)
{
	// Voodoo-1 memory map:
	//
	//   00ab----`--ccccrr`rrrrrr-- Register access
	//                                a = alternate register map if fbi_init3().tri_register_remap()
	//                                b = byte swizzle data if fbi_init0().swizzle_reg_writes()
	//                                c = chip mask select
	//                                r = register index ($00-$FF)
	//   01-yyyyy`yyyyyxxx`xxxxxxx- Linear frame buffer access (16-bit)
	//   01yyyyyy`yyyyxxxx`xxxxxx-- Linear frame buffer access (32-bit)
	//   1-ccllll`tttttttt`sssssss- Texture memory access, where:
	//                                c = chip mask select
	//                                l = LOD
	//                                t = Y index
	//                                s = X index
	//
	map(0x000000, 0x3fffff).rw(FUNC(voodoo_1_device::map_register_r), FUNC(voodoo_1_device::map_register_w));
	map(0x400000, 0x7fffff).rw(FUNC(voodoo_1_device::map_lfb_r), FUNC(voodoo_1_device::map_lfb_w));
	map(0x800000, 0xffffff).w(FUNC(voodoo_1_device::map_texture_w));
}


//-------------------------------------------------
//  read - generic read handler until everyone is
//  using the memory map
//-------------------------------------------------

u32 voodoo_1_device::read(offs_t offset, u32 mem_mask)
{
	switch (offset >> (22-2))
	{
		case 0x000000 >> 22:
			return map_register_r(offset);

		case 0x400000 >> 22:
			return map_lfb_r(offset - 0x400000/4);

		default:
			return 0xffffffff;
	}
}


//-------------------------------------------------
//  write - generic write handler until everyone is
//  using the memory map
//-------------------------------------------------

void voodoo_1_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset >> (22-2))
	{
		case 0x000000 >> 22:
			map_register_w(offset, data, mem_mask);
			break;

		case 0x400000 >> 22:
			map_lfb_w(offset - 0x400000/4, data, mem_mask);
			break;

		case 0x800000 >> 22:
		case 0xc00000 >> 22:
			map_texture_w(offset - 0x800000/4, data, mem_mask);
			break;
	}
}


//-------------------------------------------------
//  set_init_enable - set the externally-controlled
//  init_en register
//-------------------------------------------------

void voodoo_1_device::set_init_enable(u32 newval)
{
	m_init_enable = reg_init_en(newval);
	if (LOG_REGISTERS)
		logerror("VOODOO.REG:initEnable write = %08X\n", newval);
}


//-------------------------------------------------
//  update - update the screen bitmap
//-------------------------------------------------

int voodoo_1_device::update(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// if we are blank, just fill with black
	if (m_reg.fbi_init1().software_blank())
	{
		bitmap.fill(0, cliprect);
		int changed = m_video_changed;
		m_video_changed = false;
		return changed;
	}

	// if the CLUT is dirty, recompute the pens array
	if (m_clut_dirty)
	{
		rgb_t const *clutbase = &m_clut[0];

		// kludge: some of the Midway games write 0 to the last entry when they obviously mean FF
		if ((m_clut[32] & 0xffffff) == 0 && (m_clut[31] & 0xffffff) != 0)
			m_clut[32] = 0x20ffffff;

		// compute the R/B pens first
		u8 rtable[32], gtable[64], btable[32];
		for (u32 rawcolor = 0; rawcolor < 32; rawcolor++)
		{
			// treat rawcolor as a 5-bit value, scale up to 8 bits, and linear interpolate for red/blue
			u32 color = pal5bit(rawcolor);
			rtable[rawcolor] = (clutbase[color >> 3].r() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].r() * (color & 7)) >> 3;
			btable[rawcolor] = (clutbase[color >> 3].b() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].b() * (color & 7)) >> 3;
		}

		// then the G pens
		for (u32 rawcolor = 0; rawcolor < 64; rawcolor++)
		{
			// treat rawcolor as a 6-bit value, scale up to 8 bits, and linear interpolate
			u32 color = pal6bit(rawcolor);
			gtable[rawcolor] = (clutbase[color >> 3].g() * (8 - (color & 7)) + clutbase[(color >> 3) + 1].g() * (color & 7)) >> 3;
		}

		// now assemble the values into their final form
		for (u32 pen = 0; pen < 65536; pen++)
			m_pen[pen] = rgb_t(rtable[BIT(pen, 11, 5)], gtable[BIT(pen, 5, 6)], btable[BIT(pen, 0, 5)]);

		// no longer dirty
		m_clut_dirty = false;
		m_video_changed = true;
	}
	return update_common(bitmap, cliprect, &m_pen[0]);
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void voodoo_1_device::device_start()
{
	// resolve configuration-related items
	generic_voodoo_device::device_start();

	// validate configuration
	if (m_fbmem_in_mb == 0)
		fatalerror("Invalid Voodoo memory configuration");
	if (!BIT(m_chipmask, 1) && m_tmumem0_in_mb == 0)
		fatalerror("Invalid Voodoo memory configuration");

	// create shared tables
	m_shared = std::make_unique<shared_tables>();
	voodoo::dither_helper::init_static();

	// determine our index within the system, then set our trigger
	u32 index = 0;
	for (device_t &scan : device_enumerator(machine().root_device()))
		if (scan.type() == this->type())
		{
			if (&scan == this)
				break;
			index++;
		}
	m_stall_trigger = 51324 + index;

	// allocate timers for VBLANK
	m_vsync_stop_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_1_device::vblank_stop), this), this);
	m_vsync_start_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_1_device::vblank_start),this), this);

	// add TMUs to the chipmask if memory is specified (later chips leave
	// the tmumem values at 0 and set the chipmask directly to indicate
	// that RAM is shared)
	if (m_tmumem0_in_mb != 0)
	{
		m_chipmask |= 2;
		if (m_tmumem1_in_mb != 0)
			m_chipmask |= 4;
	}

	// allocate memory
	u32 total_allocation = m_fbmem_in_mb + m_tmumem0_in_mb + m_tmumem1_in_mb;
	m_memory = std::make_unique<u8[]>(total_allocation * 1024 * 1024 + 4096);

	// configure frame buffer memory, aligning the base to a 4k boundary
	m_fbram = (u8 *)(((uintptr_t(m_memory.get()) + 4095) >> 12) << 12);
	m_fbmask = m_fbmem_in_mb * 1024 * 1024 - 1;

	// configure texture memory
	u8 *tmumem[2] = { nullptr, nullptr };
	u8 tmusize[2] = { m_tmumem0_in_mb, m_tmumem1_in_mb };
	if (tmusize[0] != 0)
	{
		// separate framebuffer and texture RAM (Voodoo 1/2)
		tmumem[0] = m_fbram + m_fbmem_in_mb * 1024 * 1024;
		tmumem[1] = tmumem[0] + tmusize[0] * 1024 * 1024;
	}
	else
	{
		// shared framebuffer and texture RAM (Voodoo Banshee/3)
		tmumem[0] = tmumem[1] = m_fbram;
		tmusize[0] = tmusize[1] = m_fbmem_in_mb;
	}

	// initialize the frame buffer
	m_rgboffs[0] = m_rgboffs[1] = m_rgboffs[2] = 0;
	m_auxoffs = ~0;

	m_frontbuf = 0;
	m_backbuf = 1;
	m_swaps_pending = 0;
	m_video_changed = true;

	m_lfb_base = 0;
	m_lfb_stride = 10;

	m_width = 512;
	m_height = 384;
	m_xoffs = 0;
	m_yoffs = 0;
	m_vsyncstart = 0;
	m_vsyncstop = 0;

	m_vblank = 0;
	m_vblank_count = 0;
	m_vblank_swap_pending = 0;
	m_vblank_swap = 0;
	m_vblank_dont_swap = 0;

	m_lfb_stats.reset();

	// initialize the memory FIFO
	m_fbmem_fifo.configure(nullptr, 0);

	// initialize the CLUT
	for (int pen = 0; pen < 32; pen++)
		m_clut[pen] = rgb_t(pen, pal5bit(pen), pal5bit(pen), pal5bit(pen));
	m_clut[32] = rgb_t(32,0xff,0xff,0xff);
	m_clut_dirty = true;

	// initialize the TMUs
	u16 tmu_config = 0x11;
	m_tmu[0].init(0, *m_shared.get(), tmumem[0], tmusize[0] * 1024 * 1024);
	if (BIT(m_chipmask, 2))
	{
		m_tmu[1].init(1, *m_shared.get(), tmumem[1], tmusize[1] * 1024 * 1024);
		tmu_config |= 0xc0;
	}

	// create the renderer
	m_renderer = std::make_unique<voodoo_renderer>(machine(), tmu_config, m_shared->rgb565, m_reg, &m_tmu[0].regs(), BIT(m_chipmask, 2) ? &m_tmu[1].regs() : nullptr);

	// set up the PCI FIFO
	m_pci_fifo.configure(m_pci_fifo_mem, 64*2);
	m_stall_state = NOT_STALLED;
	m_stall_resume_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(voodoo_1_device::stall_resume_callback), this));

	// initialize registers
	m_init_enable = 0;
	m_reg.write(voodoo_regs::reg_fbiInit0, (1 << 4) | (0x10 << 6));
	m_reg.write(voodoo_regs::reg_fbiInit1, (1 << 1) | (1 << 8) | (1 << 12) | (2 << 20));
	m_reg.write(voodoo_regs::reg_fbiInit2, (1 << 6) | (0x100 << 23));
	m_reg.write(voodoo_regs::reg_fbiInit3, (2 << 13) | (0xf << 17));
	m_reg.write(voodoo_regs::reg_fbiInit4, (1 << 0));

	// do a soft reset to reset everything else
	soft_reset();

	// register for save states
	save_proxy save(*this);
	register_save(save, total_allocation);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void voodoo_1_device::device_reset()
{
	soft_reset();
}


//-------------------------------------------------
//  device_post_load - update after loading save
//  state
//-------------------------------------------------

void voodoo_1_device::device_post_load()
{
	// dirty everything so it gets recomputed
	m_clut_dirty = true;
	for (tmu_state &tm : m_tmu)
		tm.post_load();

	// recompute FBI memory FIFO to get the base pointer set
	if (m_fbmem_fifo.configured())
		recompute_fbmem_fifo();
}


//-------------------------------------------------
//  soft_reset - handle reset when initiated by
//  a register write
//-------------------------------------------------

void voodoo_1_device::soft_reset()
{
	reset_counters();
	m_reg.write(voodoo_regs::reg_fbiTrianglesOut, 0);
	m_pci_fifo.reset();
	m_fbmem_fifo.reset();
}


//-------------------------------------------------
//  register_save - register items for save states
//-------------------------------------------------

ALLOW_SAVE_TYPE(reg_init_en);
ALLOW_SAVE_TYPE(voodoo_regs::register_data);
ALLOW_SAVE_TYPE(voodoo_1_device::stall_state);

void voodoo_1_device::register_save(save_proxy &save, u32 total_allocation)
{
	// PCI state/FIFOs
	save.save_item(NAME(m_init_enable));
	save.save_item(NAME(m_stall_state));
	save.save_item(NAME(m_operation_end));
	save.save_class(NAME(m_pci_fifo));
	save.save_class(NAME(m_fbmem_fifo));

	// allocated memory
	save.save_pointer(NAME(m_fbram), 1024 * 1024 * total_allocation);
	save.save_class(NAME(*m_renderer.get()));

	// video buffer configuration
	save.save_item(NAME(m_rgboffs));
	save.save_item(NAME(m_auxoffs));
	save.save_item(NAME(m_frontbuf));
	save.save_item(NAME(m_backbuf));

	// linear frame buffer access configuration
	save.save_item(NAME(m_lfb_base));
	save.save_item(NAME(m_lfb_stride));

	// video configuration
	save.save_item(NAME(m_width));
	save.save_item(NAME(m_height));
	save.save_item(NAME(m_xoffs));
	save.save_item(NAME(m_yoffs));
	save.save_item(NAME(m_vsyncstart));
	save.save_item(NAME(m_vsyncstop));

	// VBLANK/swapping state
	save.save_item(NAME(m_swaps_pending));
	save.save_item(NAME(m_vblank));
	save.save_item(NAME(m_vblank_count));
	save.save_item(NAME(m_vblank_swap_pending));
	save.save_item(NAME(m_vblank_swap));
	save.save_item(NAME(m_vblank_dont_swap));

	// register state
	save.save_class(NAME(m_reg));
	save.save_class(NAME(m_tmu[0]));
	save.save_class(NAME(m_tmu[1]));
	save.save_item(NAME(m_dac_reg));
	save.save_item(NAME(m_dac_read_result));

	// memory for PCI FIFO
	save.save_item(NAME(m_pci_fifo_mem));

	// pens and CLUT
	save.save_item(NAME(m_clut));
}


//-------------------------------------------------
//  draw_buffer_indirect - given a 2-bit index,
//  return the front/back buffer for drawing
//-------------------------------------------------

u16 *voodoo_1_device::draw_buffer_indirect(int index)
{
	switch (index)
	{
		case 0:	m_video_changed = true; return front_buffer();
		case 1:	return back_buffer();
		default: return nullptr;
	}
}


//-------------------------------------------------
//  lfb_buffer_indirect - given a 2-bit index,
//  return the front/back/depth buffer for LFB
//  access
//-------------------------------------------------

u16 *voodoo_1_device::lfb_buffer_indirect(int index)
{
	switch (index)
	{
		case 0:	m_video_changed = true; return front_buffer();
		case 1:	return back_buffer();
		case 2: return aux_buffer();
		default: return nullptr;
	}
}


//-------------------------------------------------
//  prepare_for_read - handle housekeeping before
//  processing a direct PCI read
//-------------------------------------------------

void voodoo_1_device::prepare_for_read()
{
	// if we have something pending, flush the FIFOs up to the current time
	if (operation_pending())
		flush_fifos(machine().time());
}


//-------------------------------------------------
//  prepare_for_write - handle housekeeping before
//  processing a direct PCI write
//-------------------------------------------------

bool voodoo_1_device::prepare_for_write()
{
	// should not be getting accesses while stalled (but we do)
	if (m_stall_state != NOT_STALLED)
		logerror("voodoo_1_device::write while stalled!\n");

	// if we have something pending, flush the FIFOs up to the current time
	bool pending = operation_pending();
	if (pending)
	{
		flush_fifos(machine().time());
		pending = operation_pending();
	}
	return pending;
}


//-------------------------------------------------
//  recompute_fbmem_fifo - recompute and configure
//  the framebuffer RAM-based FIFO based on the
//  fbiInit registers
//-------------------------------------------------

void voodoo_1_device::recompute_fbmem_fifo()
{
	// compute the memory FIFO location and size
	u32 fifo_last_page = m_reg.fbi_init4().memory_fifo_stop_row();
	if (fifo_last_page > m_fbmask / 0x1000)
		fifo_last_page = m_fbmask / 0x1000;

	// is it valid and enabled?
	u32 const fifo_start_page = m_reg.fbi_init4().memory_fifo_start_row();
	if (fifo_start_page <= fifo_last_page && m_reg.fbi_init0().enable_memory_fifo())
	{
		u32 size = std::min<u32>((fifo_last_page + 1 - fifo_start_page) * 0x1000 / 4, 65536*2);
		m_fbmem_fifo.configure((u32 *)(m_fbram + fifo_start_page * 0x1000), size);
	}

	// if not, disable the FIFO
	else
		m_fbmem_fifo.configure(nullptr, 0);
}


//-------------------------------------------------
//  add_to_fifo - add a write to the PCI FIFO,
//  spilling to the memory FIFO as configured
//-------------------------------------------------

void voodoo_1_device::add_to_fifo(u32 offset, u32 data, u32 mem_mask)
{
	// add flags to the offset based on the mem_mask
	if (!ACCESSING_BITS_16_31)
		offset |= memory_fifo::NO_16_31;
	if (!ACCESSING_BITS_0_15)
		offset |= memory_fifo::NO_0_15;

	// if there's room in the PCI FIFO, add there
	if (LOG_FIFO_VERBOSE)
		logerror("VOODOO.%d.FIFO:adding to PCI FIFO @ %08X=%08X\n", this, offset, data);
	if (m_pci_fifo.full())
		fatalerror("PCI FIFO full\n");

	// add as offset/data pair
	m_pci_fifo.add(offset);
	m_pci_fifo.add(data);

	// handle flushing to the memory FIFO
	if (m_reg.fbi_init0().enable_memory_fifo() && m_pci_fifo.space() <= 2 * m_reg.fbi_init4().memory_fifo_lwm())
	{
		u8 valid[4];

		// determine which types of data can go to the memory FIFO
		valid[0] = true;
		valid[1] = m_reg.fbi_init0().lfb_to_memory_fifo();
		valid[2] = valid[3] = m_reg.fbi_init0().texmem_to_memory_fifo();

		// flush everything we can
		if (LOG_FIFO_VERBOSE) logerror("VOODOO.FIFO:moving PCI FIFO to memory FIFO\n");
		while (!m_pci_fifo.empty() && valid[(m_pci_fifo.peek() >> 22) & 3])
		{
			m_fbmem_fifo.add(m_pci_fifo.remove());
			m_fbmem_fifo.add(m_pci_fifo.remove());
		}

		// if we're above the HWM as a result, stall
		if (m_reg.fbi_init0().stall_pcie_for_hwm() && m_fbmem_fifo.items() >= 2 * 32 * m_reg.fbi_init0().memory_fifo_hwm())
		{
			if (LOG_FIFO) logerror("VOODOO.FIFO:hit memory FIFO HWM -- stalling\n");
			stall_cpu(STALLED_UNTIL_FIFO_LWM);
		}
	}

	// if we're at the LWM for the PCI FIFO, stall
	if (m_reg.fbi_init0().stall_pcie_for_hwm() && m_pci_fifo.space() <= 2 * m_reg.fbi_init0().pci_fifo_lwm())
	{
		if (LOG_FIFO) logerror("VOODOO.FIFO:hit PCI FIFO free LWM -- stalling\n");
		stall_cpu(STALLED_UNTIL_FIFO_LWM);
	}
}


//-------------------------------------------------
//  flush_fifos - flush data out of FIFOs up to
//  the current time
//-------------------------------------------------

void voodoo_1_device::flush_fifos(attotime current_time)
{
	// check for recursive calls
	if (m_flush_flag)
		return;
	m_flush_flag = true;

	// should only be called if something is pending
	if (!operation_pending())
		fatalerror("flush_fifos called with no pending operation\n");

	if (LOG_FIFO_VERBOSE)
		logerror("VOODOO.FIFO:flush_fifos start -- pending=%s cur=%s\n", m_operation_end.as_string(18), current_time.as_string(18));

	// loop while we still have cycles to burn
	while (m_operation_end <= current_time)
	{
		// execute from the FIFOs until we get something that's non-zero
		u32 cycles = execute_fifos();

		// if nothing remains, we're done; clear the flags
		if (cycles == 0)
		{
			clear_pending_operation();
			if (LOG_FIFO_VERBOSE)
				logerror("VOODOO.FIFO:flush_fifos end -- FIFOs empty\n");
			m_flush_flag = false;
			return;
		}

		// account for those cycles
		m_operation_end += clocks_to_attotime(cycles);

		if (LOG_FIFO_VERBOSE)
			logerror("VOODOO.FIFO:update -- pending=%s cur=%s\n", m_operation_end.as_string(18), current_time.as_string(18));
	}

	if (LOG_FIFO_VERBOSE)
		logerror("VOODOO.FIFO:flush_fifos end -- pending command complete at %s\n", m_operation_end.as_string(18));

	m_flush_flag = false;
}


//-------------------------------------------------
//  execute_fifos - execute commands from the FIFOs
//  until a non-zero cycle count operation is run
//-------------------------------------------------

u32 voodoo_1_device::execute_fifos()
{
	// loop until FIFOs are empty or until we get a non-zero cycle count
	while (1)
	{
		// prioritize framebuffer FIFO over PCI FIFO
		voodoo::memory_fifo &memfifo = !m_fbmem_fifo.empty() ? m_fbmem_fifo : m_pci_fifo;

		// if empty, return 0
		if (memfifo.empty())
			return 0;

		// extract address and data
		u32 offset = memfifo.remove();
		u32 data = memfifo.remove();

		// target the appropriate location
		switch (offset & memory_fifo::TYPE_MASK)
		{
			case memory_fifo::TYPE_REGISTER:
			{
				// just use the chipmask raw since it was adjusted prior to being added to the FIFO
				u32 regnum = BIT(offset, 0, 8);
				u32 chipmask = BIT(offset, 8, 4);

				// if we got a non-zero number of cycles back, return
				u32 cycles = m_regtable[regnum].write(*this, chipmask, regnum, data);
				if (cycles > 0)
					return cycles;
				break;
			}

			case memory_fifo::TYPE_TEXTURE:
				texture_w(offset & ~memory_fifo::FLAGS_MASK, data);
				break;

			case memory_fifo::TYPE_LFB:
			{
				u32 mem_mask = 0xffffffff;
				if (offset & memory_fifo::NO_16_31)
					mem_mask &= 0x0000ffff;
				if (offset & memory_fifo::NO_0_15)
					mem_mask &= 0xffff0000;
				lfb_w(offset & ~memory_fifo::FLAGS_MASK, data, mem_mask);
				break;
			}
		}
	}
}


//-------------------------------------------------
//  map_register_r - handle a mapped read from
//  regular register space
//-------------------------------------------------

u32 voodoo_1_device::map_register_r(offs_t offset)
{
	prepare_for_read();

	// extract chipmask and register
	u32 chipmask = chipmask_from_offset(offset);
	u32 regnum = BIT(offset, 0, 8);
	return m_regtable[regnum].read(*this, chipmask, regnum);
}


//-------------------------------------------------
//  map_lfb_r - handle a mapped read from LFB space
//-------------------------------------------------

u32 voodoo_1_device::map_lfb_r(offs_t offset)
{
	prepare_for_read();
	return lfb_r(offset);
}


//-------------------------------------------------
//  map_register_w - handle a mapped write to
//  regular register space
//-------------------------------------------------

void voodoo_1_device::map_register_w(offs_t offset, u32 data, u32 mem_mask)
{
	bool pending = prepare_for_write();

	// extract chipmask and register
	u32 chipmask = chipmask_from_offset(offset);
	u32 regnum = BIT(offset, 0, 8);

	// handle register swizzling -- manual says bit 21; voodoo2 manual says bit 20
	// guessing it does not overlap with the alternate register mapping bit
	if (BIT(offset, 20-2) && m_reg.fbi_init0().swizzle_reg_writes())
		data = swapendian_int32(data);

	// handle aliasing
	if (BIT(offset, 21-2) && m_reg.fbi_init3().tri_register_remap())
		regnum = voodoo_regs::alias(regnum);

	// look up the register
	auto const &regentry = m_regtable[regnum];

	// track swap buffer commands seen
	if (regnum == voodoo_regs::reg_swapbufferCMD)
		m_swaps_pending++;

	// if this is non-FIFO command, execute immediately
	if (!regentry.is_fifo())
		return void(regentry.write(*this, chipmask, regnum, data));

	// if we're busy add to the FIFO
	if (pending && m_init_enable.enable_pci_fifo())
		return add_to_fifo(memory_fifo::TYPE_REGISTER | (chipmask << 8) | regnum, data, mem_mask);

	// if we get a non-zero number of cycles back, mark things pending
	u32 cycles = regentry.write(*this, chipmask, regnum, data);
	if (cycles > 0)
	{
		m_operation_end = machine().time() + clocks_to_attotime(cycles);
		if (LOG_FIFO_VERBOSE)
			logerror("VOODOO.FIFO:direct write start at %s end at %s\n", machine().time().as_string(18), m_operation_end.as_string(18));
	}
}


//-------------------------------------------------
//  map_lfb_w - handle a mapped write to LFB space
//-------------------------------------------------

void voodoo_1_device::map_lfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if we're busy add to the FIFO, else just execute immediately
	if (prepare_for_write() && m_init_enable.enable_pci_fifo())
		add_to_fifo(memory_fifo::TYPE_LFB | offset, data, mem_mask);
	else
		lfb_w(offset, data, mem_mask);
}


//-------------------------------------------------
//  map_texture_w - handle a mapped write to
//  texture space
//-------------------------------------------------

void voodoo_1_device::map_texture_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if we're busy add to the FIFO, else just execute immediately
	if (prepare_for_write() && m_init_enable.enable_pci_fifo())
		add_to_fifo(memory_fifo::TYPE_TEXTURE | offset, data, mem_mask);
	else
		texture_w(offset, data);
}


//-------------------------------------------------
//  lfb_r - handle a read from the linear frame
//  buffer
//-------------------------------------------------

u32 voodoo_1_device::lfb_r(offs_t offset)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.m_lfb_reads++;

	// linear frame buffer reads are inherently 16-bit; convert offset to an pixel index
	offset <<= 1;

	// convert offset into X/Y coordinates
	s32 x = offset & ((1 << m_lfb_stride) - 1);
	s32 y = offset >> m_lfb_stride;
	s32 scry = y;

	// effective Y is determined by the Y origin bit
	scry &= 0x3ff;
	auto const lfbmode = m_reg.lfb_mode();
	if (lfbmode.y_origin())
		scry = m_renderer->yorigin() - scry;

	// select the target buffer
	u16 *buffer = lfb_buffer_indirect(lfbmode.read_buffer_select());
	if (buffer == nullptr)
		return 0xffffffff;

	// advance pointers to the proper row
	buffer += scry * m_renderer->rowpixels() + x;
	if (buffer + 1 >= ram_end())
	{
		logerror("LFB_R: Buffer offset out of bounds x=%i y=%i offset=%08X bufoffs=%08X\n", x, y, offset, u32(buffer - lfb_buffer_indirect(lfbmode.read_buffer_select())));
		return 0xffffffff;
	}

	// wait for any outstanding work to finish before reading
	m_renderer->wait("LFB read");

	// read and assemble two pixels
	u32 data = buffer[0] | (buffer[1] << 16);

	// word swapping
	if (lfbmode.word_swap_reads())
		data = (data << 16) | (data >> 16);

	// byte swizzling
	if (lfbmode.byte_swizzle_reads())
		data = swapendian_int32(data);

	if (LOG_LFB)
		logerror("VOODOO.LFB:read (%d,%d) = %08X\n", x, y, data);
	return data;
}


//-------------------------------------------------
//  lfb_w - handle a write to the linear frame
//  buffer
//-------------------------------------------------

void voodoo_1_device::lfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.m_lfb_writes++;

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

	// convert the incoming data
	rgb_t src_color[2];
	u16 src_depth[2];
	u32 mask = expand_lfb_data(lfbmode, data, src_color, src_depth);

	// if there are two pixels, then the offset is *2
	if ((mask & LFB_PIXEL1_MASK) != 0)
		offset <<= 1;

	// compute X,Y
	s32 x = offset & ((1 << m_lfb_stride) - 1);
	s32 y = (offset >> m_lfb_stride) & 0x3ff;

	// adjust the mask based on which half of the data is written
	if (!ACCESSING_BITS_0_15)
		mask &= ~(LFB_PIXEL0_MASK - LFB_DEPTH_PRESENT_MSW_0);
	if (!ACCESSING_BITS_16_31)
		mask &= ~(LFB_PIXEL1_MASK + LFB_DEPTH_PRESENT_MSW_0);

	// select the target buffers
	u16 *dest = draw_buffer_indirect(lfbmode.write_buffer_select());
	if (dest == nullptr)
		return;
	u16 *depth = aux_buffer();
	u16 *end = ram_end();

	// simple case: no pipeline
	auto const fbzmode = m_reg.fbz_mode();
	if (!lfbmode.enable_pixel_pipeline())
	{
		if (LOG_LFB)
			logerror("VOODOO.LFB:write raw mode %X (%d,%d) = %08X & %08X\n", lfbmode.write_format(), x, y, data, mem_mask);

		// determine the screen Y
		s32 scry = y;
		if (lfbmode.y_origin())
			scry = m_renderer->yorigin() - y;

		// advance pointers to the proper row
		dest += scry * m_renderer->rowpixels() + x;
		if (depth != nullptr)
			depth += scry * m_renderer->rowpixels() + x;

		// wait for any outstanding work to finish
		m_renderer->wait("LFB Write");

		// loop over up to two pixels
		voodoo::dither_helper dither(scry, fbzmode);
		for (int pix = 0; mask != 0; pix++)
		{
			// make sure we care about this pixel
			if ((mask & LFB_PIXEL0_MASK) != 0)
			{
				// write to the RGB buffer
				rgb_t pixel = src_color[pix];
				if ((mask & LFB_RGB_PRESENT_0) != 0 && dest + pix < end)
					dest[pix] = dither.pixel(x, pixel.r(), pixel.g(), pixel.b());

				// make sure we have an aux buffer to write to
				if (depth != nullptr && depth + pix < end)
				{
					if (fbzmode.enable_alpha_planes())
					{
						// write to the alpha buffer
						if ((mask & LFB_ALPHA_PRESENT_0) != 0)
							depth[pix] = pixel.a();
					}
					else
					{
						// write to the depth buffer
						if ((mask & (LFB_DEPTH_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0)) != 0)
							depth[pix] = src_depth[pix];
					}
				}

				// track pixel writes to the frame buffer regardless of mask
				m_reg.add(voodoo_regs::reg_fbiPixelsOut, 1);
			}

			// advance our pointers
			x++;
			mask >>= 4;
		}
	}

	// tricky case: run the full pixel pipeline on the pixel
	else
	{
		if (LOG_LFB)
			logerror("VOODOO.LFB:write pipelined mode %X (%d,%d) = %08X & %08X\n", lfbmode.write_format(), x, y, data, mem_mask);

		// determine the screen Y
		s32 scry = y;
		if (fbzmode.y_origin())
			scry = m_renderer->yorigin() - y;

		// advance pointers to the proper row
		dest += scry * m_renderer->rowpixels();
		if (depth != nullptr)
			depth += scry * m_renderer->rowpixels();

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
		thread_stats_block &threadstats = m_lfb_stats;
		rgbaint_t iterargb(0);
		for (int pix = 0; mask != 0; pix++)
		{
			// make sure we care about this pixel
			if ((mask & LFB_PIXEL0_MASK) != 0)
				m_renderer->pixel_pipeline(threadstats, poly, lfbmode, x, y, src_color[pix], src_depth[pix]);

			// advance our pointers
			x++;
			mask >>= 4;
		}
	}
}


//-------------------------------------------------
//  expand_lfb_data - expand a 32-bit raw data
//  value into 1 or 2 expanded RGBA and depth
//  values
//-------------------------------------------------

u32 voodoo_1_device::expand_lfb_data(reg_lfb_mode const lfbmode, u32 data, rgb_t src_color[2], u16 src_depth[2])
{
	// extract default depth value from low bits of zaColor
	src_depth[0] = src_depth[1] = m_reg.za_color() & 0xffff;

	// if not otherwise specified, alpha defaults to the upper bits of zaColor
	u32 src_alpha = m_reg.za_color() >> 24;

	// extract color information from the data
	switch (16 * lfbmode.rgba_lanes() + lfbmode.write_format())
	{
		case 16*0 + 0:      // ARGB, format 0: 16-bit RGB 5-6-5
		case 16*2 + 0:      // RGBA, format 0: 16-bit RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data, 11,  5,  0).set_a(src_alpha);
			src_color[1] = rgbexpand<5,6,5>(data, 27, 21, 16).set_a(src_alpha);
			return LFB_RGB_PRESENT_0 | LFB_RGB_PRESENT_1;

		case 16*1 + 0:      // ABGR, format 0: 16-bit RGB 5-6-5
		case 16*3 + 0:      // BGRA, format 0: 16-bit RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data,  0,  5, 11).set_a(src_alpha);
			src_color[1] = rgbexpand<5,6,5>(data, 16, 21, 27).set_a(src_alpha);
			return LFB_RGB_PRESENT_0 | LFB_RGB_PRESENT_1;

		case 16*0 + 1:      // ARGB, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 10,  5,  0).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 26, 21, 16).set_a(src_alpha);
			return LFB_RGB_PRESENT_0 | LFB_RGB_PRESENT_1;

		case 16*1 + 1:      // ABGR, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  0,  5, 10).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 16, 21, 26).set_a(src_alpha);
			return LFB_RGB_PRESENT_0 | LFB_RGB_PRESENT_1;

		case 16*2 + 1:      // RGBA, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 11,  6,  1).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 27, 22, 17).set_a(src_alpha);
			return LFB_RGB_PRESENT_0 | LFB_RGB_PRESENT_1;

		case 16*3 + 1:      // BGRA, format 1: 16-bit RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  1,  6, 11).set_a(src_alpha);
			src_color[1] = rgbexpand<5,5,5>(data, 17, 22, 27).set_a(src_alpha);
			return LFB_RGB_PRESENT_0 | LFB_RGB_PRESENT_1;

		case 16*0 + 2:      // ARGB, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15, 10,  5,  0);
			src_color[1] = argbexpand<1,5,5,5>(data, 31, 26, 21, 16);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_RGB_PRESENT_1 | LFB_ALPHA_PRESENT_1;

		case 16*1 + 2:      // ABGR, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15,  0,  5, 10);
			src_color[1] = argbexpand<1,5,5,5>(data, 31, 16, 21, 26);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_RGB_PRESENT_1 | LFB_ALPHA_PRESENT_1;

		case 16*2 + 2:      // RGBA, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0, 11,  6,  1);
			src_color[1] = argbexpand<1,5,5,5>(data, 16, 27, 22, 17);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_RGB_PRESENT_1 | LFB_ALPHA_PRESENT_1;

		case 16*3 + 2:      // BGRA, format 2: 16-bit ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0,  1,  6, 11);
			src_color[1] = argbexpand<1,5,5,5>(data, 16, 17, 22, 27);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_RGB_PRESENT_1 | LFB_ALPHA_PRESENT_1;

		case 16*0 + 4:      // ARGB, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data, 16,  8,  0).set_a(src_alpha);
			return LFB_RGB_PRESENT_0;

		case 16*1 + 4:      // ABGR, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data,  0,  8, 16).set_a(src_alpha);
			return LFB_RGB_PRESENT_0;

		case 16*2 + 4:      // RGBA, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data, 24, 16,  8).set_a(src_alpha);
			return LFB_RGB_PRESENT_0;

		case 16*3 + 4:      // BGRA, format 4: 32-bit RGB x-8-8-8
			src_color[0] = rgbexpand<8,8,8>(data,  8, 16, 24).set_a(src_alpha);
			return LFB_RGB_PRESENT_0;

		case 16*0 + 5:      // ARGB, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data, 24, 16,  8,  0);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0;

		case 16*1 + 5:      // ABGR, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data, 24,  0,  8, 16);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0;

		case 16*2 + 5:      // RGBA, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data,  0, 24, 16,  8);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0;

		case 16*3 + 5:      // BGRA, format 5: 32-bit ARGB 8-8-8-8
			src_color[0] = argbexpand<8,8,8,8>(data,  0,  8, 16, 24);
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0;

		case 16*0 + 12:     // ARGB, format 12: 32-bit depth+RGB 5-6-5
		case 16*2 + 12:     // RGBA, format 12: 32-bit depth+RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data, 11,  5,  0).set_a(src_alpha);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*1 + 12:     // ABGR, format 12: 32-bit depth+RGB 5-6-5
		case 16*3 + 12:     // BGRA, format 12: 32-bit depth+RGB 5-6-5
			src_color[0] = rgbexpand<5,6,5>(data,  0,  5, 11).set_a(src_alpha);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*0 + 13:     // ARGB, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 10,  5,  0).set_a(src_alpha);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*1 + 13:     // ABGR, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  0,  5, 10).set_a(src_alpha);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*2 + 13:     // RGBA, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data, 11,  6,  1).set_a(src_alpha);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*3 + 13:     // BGRA, format 13: 32-bit depth+RGB x-5-5-5
			src_color[0] = rgbexpand<5,5,5>(data,  1,  6, 11).set_a(src_alpha);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*0 + 14:     // ARGB, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15, 10,  5,  0);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*1 + 14:     // ABGR, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data, 15,  0,  5, 10);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*2 + 14:     // RGBA, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0, 11,  6,  1);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*3 + 14:     // BGRA, format 14: 32-bit depth+ARGB 1-5-5-5
			src_color[0] = argbexpand<1,5,5,5>(data,  0,  1,  6, 11);
			src_depth[0] = data >> 16;
			return LFB_RGB_PRESENT_0 | LFB_ALPHA_PRESENT_0 | LFB_DEPTH_PRESENT_MSW_0;

		case 16*0 + 15:     // ARGB, format 15: 16-bit depth
		case 16*1 + 15:     // ARGB, format 15: 16-bit depth
		case 16*2 + 15:     // ARGB, format 15: 16-bit depth
		case 16*3 + 15:     // ARGB, format 15: 16-bit depth
			src_depth[0] = data & 0xffff;
			src_depth[1] = data >> 16;
			return LFB_DEPTH_PRESENT_0 | LFB_DEPTH_PRESENT_1;

		default:            // reserved
			logerror("lfb_w: Unknown format\n");
			return 0;
	}
}


//-------------------------------------------------
//  texture_w - handle writes to texture RAM
//-------------------------------------------------

void voodoo_1_device::texture_w(offs_t offset, u32 data)
{
	// statistics
	if (DEBUG_STATS)
		m_stats.m_tex_writes++;

	// point to the right TMU
	int tmunum = BIT(offset, 19, 2);
	if (!BIT(m_chipmask, 1 + tmunum))
		return;

	// wait for any outstanding work to finish
	m_renderer->wait("Texture write");

	// the seq_8_downld flag seems to always come from TMU #0
	bool seq_8_downld = m_tmu[0].regs().texture_mode().seq_8_downld();

	// pull out modes from the TMU and update state
	auto &regs = m_tmu[tmunum].regs();
	auto const texlod = regs.texture_lod();
	auto const texmode = regs.texture_mode();
	auto &texture = m_tmu[tmunum].prepare_texture(*m_renderer.get());

	// texture direct not handled (but never seen so far)
	if (texlod.tdirect_write())
		fatalerror("Texture direct write!\n");

	// swizzle the data
	if (texlod.tdata_swizzle())
		data = swapendian_int32(data);
	if (texlod.tdata_swap())
		data = (data >> 16) | (data << 16);

	// determine destination pointer
	u32 bytes_per_texel = (texmode.format() < 8) ? 1 : 2;
	u32 lod = BIT(offset, 15, 4);
	u32 tt = BIT(offset, 7, 8);
	u32 ts = (offset << ((seq_8_downld && bytes_per_texel == 1) ? 2 : 1)) & 0xff;

	// validate parameters
	if (lod > 8)
		return;
	u8 *dest = texture.write_ptr(lod, ts, tt, bytes_per_texel);

	// write the four bytes in little-endian order
	if (bytes_per_texel == 1)
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






int voodoo_1_device::update_common(bitmap_rgb32 &bitmap, const rectangle &cliprect, rgb_t const *pens)
{
	// reset the video changed flag
	bool changed = m_video_changed;
	m_video_changed = false;

	// select the buffer to draw
	int drawbuf = m_frontbuf;
	if (DEBUG_BACKBUF && machine().input().code_pressed(KEYCODE_L))
		drawbuf = m_backbuf;

	// copy from the current front buffer
	if (LOG_VBLANK_SWAP) logerror("--- update_common @ %d from %08X\n", screen().vpos(), m_rgboffs[m_frontbuf]);
	u32 rowpixels = m_renderer->rowpixels();
	for (s32 y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		if (y < m_yoffs)
			continue;
		u16 const *const src = draw_buffer(drawbuf) + (y - m_yoffs) * rowpixels - m_xoffs;
		u32 *dst = &bitmap.pix(y);
		for (s32 x = cliprect.min_x; x <= cliprect.max_x; x++)
			dst[x] = pens[src[x]];
	}

	// update stats display
	if (DEBUG_STATS && m_stats.update_display_state(machine().input().code_pressed(KEYCODE_BACKSLASH)))
		popmessage(m_stats.string(), 0, 0);

	// overwrite with the depth buffer if debugging and the ENTER key is pressed
	if (DEBUG_DEPTH && machine().input().code_pressed(KEYCODE_ENTER))
		for (s32 y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			u16 const *const src = aux_buffer() + (y - m_yoffs) * rowpixels - m_xoffs;
			u32 *const dst = &bitmap.pix(y);
			for (s32 x = cliprect.min_x; x <= cliprect.max_x; x++)
				dst[x] = ((src[x] << 8) & 0xff0000) | ((src[x] >> 0) & 0xff00) | ((src[x] >> 8) & 0xff);
		}

	return changed;
}






/*************************************
 *
 *  Statistics management
 *
 *************************************/

void voodoo_1_device::accumulate_statistics(thread_stats_block const &block)
{
	// update live voodoo statistics
	m_reg.add(voodoo_regs::reg_fbiPixelsIn, block.pixels_in);
	m_reg.add(voodoo_regs::reg_fbiPixelsOut, block.pixels_out);
	m_reg.add(voodoo_regs::reg_fbiChromaFail, block.chroma_fail);
	m_reg.add(voodoo_regs::reg_fbiZfuncFail, block.zfunc_fail);
	m_reg.add(voodoo_regs::reg_fbiAfuncFail, block.afunc_fail);

	// update emulation statistics
	if (DEBUG_STATS)
		m_stats.add_emulation_stats(block);
}


void voodoo_1_device::update_statistics(bool accumulate)
{
	// accumulate/reset statistics from all units
	for (auto &stats : m_renderer->thread_stats())
	{
		if (accumulate)
			accumulate_statistics(stats);
		stats.reset();
	}

	// accumulate/reset statistics from the LFB
	if (accumulate)
		accumulate_statistics(m_lfb_stats);
	m_lfb_stats.reset();
}



/*************************************
 *
 *  VBLANK management
 *
 *************************************/

void voodoo_1_device::rotate_buffers()
{
	if (!m_vblank_dont_swap)
	{
		u32 buffers = (m_rgboffs[2] == ~0) ? 2 : 3;
		m_frontbuf = (m_frontbuf + 1) % buffers;
		m_backbuf = (m_frontbuf + 1) % buffers;
	}
}

void voodoo_1_device::swap_buffers()
{
	if (LOG_VBLANK_SWAP)
		logerror("--- swap_buffers @ %d\n", screen().vpos());

	// force a partial update
	screen().update_partial(screen().vpos());
	m_video_changed = true;

	// keep a history of swap intervals
	m_reg.update_swap_history(std::min<u8>(m_vblank_count, 15));

	// rotate the buffers; implementation differs between models
	rotate_buffers();

	// decrement the pending count and reset our state
	if (m_swaps_pending != 0)
		m_swaps_pending--;
	m_vblank_count = 0;
	m_vblank_swap_pending = false;

	// reset the last_op_time to now and start processing the next command
	if (operation_pending())
	{
		if (LOG_VBLANK_SWAP)
			logerror("---- swap_buffers flush begin\n");
		flush_fifos(m_operation_end = machine().time());
		if (LOG_VBLANK_SWAP)
			logerror("---- swap_buffers flush end\n");
	}

	// we may be able to unstall now
	if (m_stall_state != NOT_STALLED)
		check_stalled_cpu(machine().time());

	// periodically log rasterizer info
	m_stats.m_swaps++;
	if (m_stats.m_swaps % 1000 == 0)
		m_renderer->dump_rasterizer_stats();

	// update the statistics (debug)
	if (DEBUG_STATS)
	{
		if (m_stats.displayed())
		{
			update_statistics(true);
			m_stats.update_string(screen().visible_area(), m_reg.swap_history());
		}
		m_stats.reset();
	}
}


void voodoo_1_device::adjust_vblank_start_timer()
{
	attotime vblank_period = screen().time_until_pos(m_vsyncstart);
	if (LOG_VBLANK_SWAP) logerror("adjust_vblank_start_timer: period: %s\n", vblank_period.as_string());
	/* if zero, adjust to next frame, otherwise we may get stuck in an infinite loop */
	if (vblank_period == attotime::zero)
		vblank_period = screen().frame_period();
	m_vsync_start_timer->adjust(vblank_period);
}


void voodoo_1_device::vblank_start(void *ptr, s32 param)
{
	if (LOG_VBLANK_SWAP)
		logerror("--- vblank start\n");

	// flush the pipes
	if (operation_pending())
	{
		if (LOG_VBLANK_SWAP)
			logerror("---- vblank flush begin\n");
		flush_fifos(machine().time());
		if (LOG_VBLANK_SWAP)
			logerror("---- vblank flush end\n");
	}

	// increment the count
	m_vblank_count = std::min(m_vblank_count + 1, 250);

	// logging
	if (LOG_VBLANK_SWAP)
		logerror("---- vblank count = %u swap = %u pending = %u", m_vblank_count, m_vblank_swap, m_vblank_swap_pending);
	if (LOG_VBLANK_SWAP && m_vblank_swap_pending)
		logerror(" (target=%d)", m_vblank_swap);
	if (LOG_VBLANK_SWAP)
		logerror("\n");

	// if we're past the swap count, do the swap
	if (m_vblank_swap_pending && m_vblank_count >= m_vblank_swap)
		swap_buffers();

	// set a timer for the next off state
	m_vsync_stop_timer->adjust(screen().time_until_pos(m_vsyncstop));

	// set internal state and call the client
	m_vblank = true;

	// notify external VBLANK handler on all models
	if (!m_vblank_cb.isnull())
		m_vblank_cb(true);
}


void voodoo_1_device::vblank_stop(void *ptr, s32 param)
{
	if (LOG_VBLANK_SWAP)
		logerror("--- vblank end\n");

	// set internal state and call the client
	m_vblank = false;

	// notify external VBLANK handler on all models
	if (!m_vblank_cb.isnull())
		m_vblank_cb(false);

	// go to the end of the next frame
	adjust_vblank_start_timer();
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

void voodoo_1_device::reset_counters()
{
	update_statistics(false);
	m_reg.write(voodoo_regs::reg_fbiPixelsIn, 0);
	m_reg.write(voodoo_regs::reg_fbiChromaFail, 0);
	m_reg.write(voodoo_regs::reg_fbiZfuncFail, 0);
	m_reg.write(voodoo_regs::reg_fbiAfuncFail, 0);
	m_reg.write(voodoo_regs::reg_fbiPixelsOut, 0);
}



/*************************************
 *
 *  Recompute video memory layout
 *
 *************************************/

void voodoo_1_device::recompute_video_memory()
{
	// configuration is either double-buffered (0) or triple-buffered (1)
	u32 config = m_reg.fbi_init2().enable_triple_buf();

	// 4-bit tile count; tiles are 64x16
	u32 xtiles = m_reg.fbi_init1().x_video_tiles();
	recompute_video_memory_common(config, xtiles * 64);
}

void voodoo_1_device::recompute_video_memory_common(u32 config, u32 rowpixels)
{
	// remember the front buffer configuration to check for changes
	u16 *starting_front = front_buffer();
	u32 starting_rowpix = m_renderer->rowpixels();

	// first RGB buffer always starts at 0
	m_rgboffs[0] = 0;

	// second RGB buffer starts immediately afterwards
	u32 const buffer_pages = m_reg.fbi_init2().video_buffer_offset();
	m_rgboffs[1] = buffer_pages * 0x1000;

	// remaining buffers are based on the config
	switch (config)
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
		if (m_rgboffs[buf] != ~0 && m_rgboffs[buf] > m_fbmask)
			m_rgboffs[buf] = m_fbmask;

	// clamp the aux buffer to video memory
	if (m_auxoffs != ~0 && m_auxoffs > m_fbmask)
		m_auxoffs = m_fbmask;

	// reset our front/back buffers if they are out of range
	if (m_rgboffs[2] == ~0)
	{
		if (m_frontbuf == 2)
			m_frontbuf = 0;
		if (m_backbuf == 2)
			m_backbuf = 0;
	}

	// mark video changed if the front buffer configuration is different
	if (front_buffer() != starting_front || rowpixels != starting_rowpix)
		m_video_changed = true;
	m_renderer->set_rowpixels(rowpixels);
}



/*************************************
 *
 *  Stall the active cpu until we are
 *  ready
 *
 *************************************/

TIMER_CALLBACK_MEMBER( voodoo_1_device::stall_resume_callback )
{
	check_stalled_cpu(machine().time());
}


void voodoo_1_device::check_stalled_cpu(attotime current_time)
{
	int resume = false;

	/* flush anything we can */
	if (operation_pending())
		flush_fifos(current_time);

	/* if we're just stalled until the LWM is passed, see if we're ok now */
	if (m_stall_state == STALLED_UNTIL_FIFO_LWM)
	{
		/* if there's room in the memory FIFO now, we can proceed */
		if (m_reg.fbi_init0().enable_memory_fifo())
		{
			if (m_fbmem_fifo.items() < 2 * 32 * m_reg.fbi_init0().memory_fifo_hwm())
				resume = true;
		}
		else if (m_pci_fifo.space() > 2 * m_reg.fbi_init0().pci_fifo_lwm())
			resume = true;
	}

	/* if we're stalled until the FIFOs are empty, check now */
	else if (m_stall_state == STALLED_UNTIL_FIFO_EMPTY)
	{
		if (m_reg.fbi_init0().enable_memory_fifo())
		{
			if (m_fbmem_fifo.empty() && m_pci_fifo.empty())
				resume = true;
		}
		else if (m_pci_fifo.empty())
			resume = true;
	}

	/* resume if necessary */
	if (resume || !operation_pending())
	{
		if (LOG_FIFO) logerror("VOODOO.FIFO:Stall condition cleared; resuming\n");
		m_stall_state = NOT_STALLED;

		/* either call the callback, or trigger the trigger */
		if (!m_stall_cb.isnull())
			m_stall_cb(false);
		else
			machine().scheduler().trigger(m_stall_trigger);
	}

	/* if not, set a timer for the next one */
	else
		m_stall_resume_timer->adjust(m_operation_end - current_time);
}


void voodoo_1_device::stall_cpu(stall_state state)
{
	// sanity check
	if (!operation_pending())
		fatalerror("FIFOs not empty, no op pending!\n");

	// set the state and update statistics
	m_stall_state = state;
	if (DEBUG_STATS)
		m_stats.m_stalls++;

	// either call the callback, or spin the CPU
	if (!m_stall_cb.isnull())
		m_stall_cb(true);
	else
		m_cpu->spin_until_trigger(m_stall_trigger);

	// set a timer to clear the stall
	m_stall_resume_timer->adjust(m_operation_end - machine().time());
}



//-------------------------------------------------
//  reg_invalid_r - generic invalid register read
//-------------------------------------------------

u32 voodoo_1_device::reg_invalid_r(u32 chipmask, u32 regnum)
{
	return 0xffffffff;
}


//-------------------------------------------------
//  reg_passive_r - generic passive register read
//-------------------------------------------------

u32 voodoo_1_device::reg_passive_r(u32 chipmask, u32 regnum)
{
	return m_reg.read(regnum);
}


//-------------------------------------------------
//  reg_status_r - status register read
//-------------------------------------------------

u32 voodoo_1_device::reg_status_r(u32 chipmask, u32 regnum)
{
	u32 result = 0;

	// bits 5:0 are the PCI FIFO free space
	result |= std::min(m_pci_fifo.space() / 2, 0x3f) << 0;

	// bit 6 is the vertical retrace
	result |= m_vblank << 6;

	// bit 7 is FBI graphics engine busy
	// bit 8 is TREX busy
	// bit 9 is overall busy
	if (operation_pending())
		result |= (1 << 7) | (1 << 8) | (1 << 9);

	// bits 10-11 is displayed buffer
	result |= m_frontbuf << 10;

	// bits 12-27 is memory FIFO free space
	if (m_reg.fbi_init0().enable_memory_fifo() == 0)
		result |= 0xffff << 12;
	else
		result |= std::min(m_fbmem_fifo.space() / 2, 0xffff) << 12;

	// bits 30:28 are the number of pending swaps
	result |= std::min<s32>(m_swaps_pending, 7) << 28;

	// eat some cycles since people like polling here
	if (EAT_CYCLES)
		m_cpu->eat_cycles(1000);

	// bit 31 is PCI interrupt pending (not implemented)
	return result;
}


//-------------------------------------------------
//  reg_fbiinit2_r - fbiInit2 register read
//-------------------------------------------------

u32 voodoo_1_device::reg_fbiinit2_r(u32 chipmask, u32 regnum)
{
	// bit 2 of the initEnable register maps this to dacRead
	return m_init_enable.remap_init_to_dac() ? m_dac_read_result : m_reg.read(regnum);
}


//-------------------------------------------------
//  reg_vretrace_r - vRetrace register read
//-------------------------------------------------

u32 voodoo_1_device::reg_vretrace_r(u32 chipmask, u32 regnum)
{
	// eat some cycles since people like polling here
	if (EAT_CYCLES)
		m_cpu->eat_cycles(10);

	// return 0 if vblank is active
	return m_vblank ? 0 : screen().vpos();
}


//-------------------------------------------------
//  reg_stats_r - statistics register reads
//-------------------------------------------------

u32 voodoo_1_device::reg_stats_r(u32 chipmask, u32 regnum)
{
	update_statistics(true);
	return m_reg.read(regnum);
}


//-------------------------------------------------
//  reg_invalid_w - generic invalid register write
//-------------------------------------------------

u32 voodoo_1_device::reg_invalid_w(u32 chipmask, u32 regnum, u32 data)
{
	logerror("%s: Unexpected write to register %02X[%X] = %08X\n", machine().describe_context(), regnum, chipmask, data);
	return 0;
}


//-------------------------------------------------
//  reg_status_w - status register write (Voodoo 1)
//-------------------------------------------------

u32 voodoo_1_device::reg_unimplemented_w(u32 chipmask, u32 regnum, u32 data)
{
	logerror("%s: Unimplemented write to register %02X(%X) = %08X\n", machine().describe_context(), regnum, chipmask, data);
	return 0;
}

//-------------------------------------------------
//  reg_passive_w - generic passive register write
//-------------------------------------------------

u32 voodoo_1_device::reg_passive_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0)) m_reg.write(regnum, data);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write(regnum, data);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write(regnum, data);
	return 0;
}


//-------------------------------------------------
//  reg_fpassive_4_w -- passive write with floating
//  point to x.4 fixed point conversion
//-------------------------------------------------

u32 voodoo_1_device::reg_fpassive_4_w(u32 chipmask, u32 regnum, u32 data)
{
	return reg_passive_w(chipmask, regnum - 0x80/4, float_to_int32(data, 4));
}


//-------------------------------------------------
//  reg_fpassive_12_w -- passive write with
//  floating point to x.12 fixed point conversion
//-------------------------------------------------

u32 voodoo_1_device::reg_fpassive_12_w(u32 chipmask, u32 regnum, u32 data)
{
	return reg_passive_w(chipmask, regnum - 0x80/4, float_to_int32(data, 12));
}


//-------------------------------------------------
//  reg_starts_w -- write to startS (14.18)
//  reg_starts_w -- write to startT (14.18)
//  reg_dsdx_w -- write to dSdX (14.18)
//  reg_dtdx_w -- write to dTdX (14.18)
//  reg_dsdy_w -- write to dSdY (14.18)
//  reg_dtdy_w -- write to dTdY (14.18)
//-------------------------------------------------

u32 voodoo_1_device::reg_starts_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 14;
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_start_s(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_start_s(data64);
	return 0;
}
u32 voodoo_1_device::reg_startt_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 14;
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_start_t(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_start_t(data64);
	return 0;
}
u32 voodoo_1_device::reg_dsdx_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 14;
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_ds_dx(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_ds_dx(data64);
	return 0;
}
u32 voodoo_1_device::reg_dtdx_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 14;
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dt_dx(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dt_dx(data64);
	return 0;
}
u32 voodoo_1_device::reg_dsdy_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 14;
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_ds_dy(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_ds_dy(data64);
	return 0;
}
u32 voodoo_1_device::reg_dtdy_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 14;
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dt_dy(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dt_dy(data64);
	return 0;
}


//-------------------------------------------------
//  reg_fstarts_w -- write to fstartS
//  reg_fstartt_w -- write to fstartT
//  reg_fdsdx_w -- write to fdSdX
//  reg_fdtdx_w -- write to fdTdX
//  reg_fdsdy_w -- write to fdSdY
//  reg_fdtdy_w -- write to fdTdY
//-------------------------------------------------

u32 voodoo_1_device::reg_fstarts_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_start_s(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_start_s(data64);
	return 0;
}
u32 voodoo_1_device::reg_fstartt_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_start_t(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_start_t(data64);
	return 0;
}
u32 voodoo_1_device::reg_fdsdx_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_ds_dx(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_ds_dx(data64);
	return 0;
}
u32 voodoo_1_device::reg_fdtdx_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dt_dx(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dt_dx(data64);
	return 0;
}
u32 voodoo_1_device::reg_fdsdy_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_ds_dy(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_ds_dy(data64);
	return 0;
}
u32 voodoo_1_device::reg_fdtdy_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dt_dy(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dt_dy(data64);
	return 0;
}


//-------------------------------------------------
//  reg_startw_w -- write to startW (2.30 -> 16.32)
//  reg_dwdx_w -- write to dWdX (2.30 -> 16.32)
//  reg_dwdy_w -- write to dWdY (2.30 -> 16.32)
//-------------------------------------------------

u32 voodoo_1_device::reg_startw_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 2;
	if (BIT(chipmask, 0))          m_reg.write_start_w(data64);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_start_w(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_start_w(data64);
	return 0;
}
u32 voodoo_1_device::reg_dwdx_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 2;
	if (BIT(chipmask, 0))          m_reg.write_dw_dx(data64);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dw_dx(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dw_dx(data64);
	return 0;
}
u32 voodoo_1_device::reg_dwdy_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = s64(s32(data)) << 2;
	if (BIT(chipmask, 0))          m_reg.write_dw_dy(data64);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dw_dy(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dw_dy(data64);
	return 0;
}


//-------------------------------------------------
//  reg_fstartw_w -- write to fstartW
//  reg_fdwdx_w -- write to fdWdX
//  reg_fdwdy_w -- write to fdWdY
//-------------------------------------------------

u32 voodoo_1_device::reg_fstartw_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 0))          m_reg.write_start_w(data64);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_start_w(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_start_w(data64);
	return 0;
}
u32 voodoo_1_device::reg_fdwdx_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 0))          m_reg.write_dw_dx(data64);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dw_dx(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dw_dx(data64);
	return 0;
}
u32 voodoo_1_device::reg_fdwdy_w(u32 chipmask, u32 regnum, u32 data)
{
	s64 data64 = float_to_int64(data, 32);
	if (BIT(chipmask, 0))          m_reg.write_dw_dy(data64);
	if (BIT(chipmask, 1)) m_tmu[0].regs().write_dw_dy(data64);
	if (BIT(chipmask, 2)) m_tmu[1].regs().write_dw_dy(data64);
	return 0;
}


//-------------------------------------------------
//  reg_triangle_w -- write to triangleCMD/
//  ftriangleCMD
//-------------------------------------------------

u32 voodoo_1_device::reg_triangle_w(u32 chipmask, u32 regnum, u32 data)
{
	return triangle();
}


//-------------------------------------------------
//  reg_nop_w -- write to nopCMD
//-------------------------------------------------

u32 voodoo_1_device::reg_nop_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(data, 0))
		reset_counters();
	if (BIT(data, 1))
		m_reg.write(voodoo_regs::reg_fbiTrianglesOut, 0);
	return 0;
}


//-------------------------------------------------
//  reg_fastfill_w -- write to fastfillCMD
//-------------------------------------------------

u32 voodoo_1_device::reg_fastfill_w(u32 chipmask, u32 regnum, u32 data)
{
	auto &poly = m_renderer->alloc_poly();

	// determine the draw buffer (Banshee and later are hard-coded to the back buffer)
	poly.destbase = draw_buffer_indirect(m_reg.fbz_mode().draw_buffer());
	if (poly.destbase == nullptr)
		return 0;
	poly.depthbase = aux_buffer();
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
//  reg_swapbuffer_w -- write to swapbufferCMD
//-------------------------------------------------

u32 voodoo_1_device::reg_swapbuffer_w(u32 chipmask, u32 regnum, u32 data)
{
	m_renderer->wait("swapbufferCMD");

	// the don't swap value is Voodoo 2-only, masked off by the register engine
	m_vblank_swap_pending = true;
	m_vblank_swap = BIT(data, 1, 8);
	m_vblank_dont_swap = BIT(data, 9);

	// if we're not syncing to the retrace, process the command immediately
	if (!BIT(data, 0))
	{
		swap_buffers();
		return 0;
	}

	// determine how many cycles to wait; we deliberately overshoot here because
	// the final count gets updated on the VBLANK
	return (m_vblank_swap + 1) * clock() / 10;
}


//-------------------------------------------------
//  reg_fogtable_w -- write to fogTable
//-------------------------------------------------

u32 voodoo_1_device::reg_fogtable_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0)) m_renderer->write_fog(2 * (regnum - voodoo_regs::reg_fogTable), data);
	return 0;
}


//-------------------------------------------------
//  reg_fbiinit_w -- write to an fbiinit register
//-------------------------------------------------

u32 voodoo_1_device::reg_fbiinit_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0) && m_init_enable.enable_hw_init())
	{
		m_renderer->wait("fbi_init");
		m_reg.write(regnum, data);

		// handle resets written to fbiInit0
		if (regnum == voodoo_regs::reg_fbiInit0 && m_reg.fbi_init0().graphics_reset())
			soft_reset();
		if (regnum == voodoo_regs::reg_fbiInit0 && m_reg.fbi_init0().fifo_reset())
			m_pci_fifo.reset();

		// compute FIFO layout when fbiInit0 or fbiInit4 change
		if (regnum == voodoo_regs::reg_fbiInit0 || regnum == voodoo_regs::reg_fbiInit4)
			recompute_fbmem_fifo();

		// recompute video memory when fbiInit1 or fbiInit2 change
		if (regnum == voodoo_regs::reg_fbiInit1 || regnum == voodoo_regs::reg_fbiInit2)
			recompute_video_memory();

		// update Y origina when fbiInit3 changes
		if (regnum == voodoo_regs::reg_fbiInit3)
			m_renderer->set_yorigin(m_reg.fbi_init3().yorigin_subtract());
	}
	return 0;
}


//-------------------------------------------------
//  reg_video_w -- write to a video configuration
//  register; synchronize then recompute everything
//-------------------------------------------------

u32 voodoo_1_device::reg_video_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("video_configuration");
		m_reg.write(regnum, data);

		auto const hsync = m_reg.hsync<true>();
		auto const vsync = m_reg.vsync<true>();
		auto const back_porch = m_reg.back_porch<true>();
		auto const video_dimensions = m_reg.video_dimensions<true>();
		if (hsync.raw() != 0 && vsync.raw() != 0 && video_dimensions.raw() != 0 && back_porch.raw() != 0)
		{
			recompute_video_timing(
					hsync.hsync_on(), hsync.hsync_off(),
					video_dimensions.xwidth(), back_porch.horizontal() + 2,
					vsync.vsync_on(), vsync.vsync_off(),
					video_dimensions.yheight(), back_porch.vertical());
		}
	}
	return 0;
}


//-------------------------------------------------
//  reg_clut_w -- write to clutData; mark dirty if
//  changed
//-------------------------------------------------

u32 voodoo_1_device::reg_clut_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("clut");
		if (m_reg.fbi_init1().video_timing_reset() == 0)
		{
			int index = BIT(data, 24, 8);
			if (index <= 32 && m_clut[index] != data)
			{
				m_clut[index] = data;
				m_clut_dirty = true;
			}
		}
		else
			logerror("clutData ignored because video timing reset = 1\n");
	}
	return 0;
}


//-------------------------------------------------
//  reg_dac_w -- write to dacData
//-------------------------------------------------

u32 voodoo_1_device::reg_dac_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		// upper 2 address bits are only on Voodoo2+ but are masked by the
		// register entry for Voodoo 1 so safe to just use them as presented
		u32 regnum = BIT(data, 8, 3) + 8 * BIT(data, 12, 2);
		if (!BIT(data, 11))
			m_dac_reg[regnum] = BIT(data, 0, 8);
		else
		{
			// this is just to make startup happy
			m_dac_read_result = m_dac_reg[regnum];
			switch (m_dac_reg[7])
			{
				case 0x01:  m_dac_read_result = 0x55; break;
				case 0x07:  m_dac_read_result = 0x71; break;
				case 0x0b:  m_dac_read_result = 0x79; break;
			}
		}
	}
	return 0;
}


//-------------------------------------------------
//  reg_texture_w -- passive write to a TMU; mark
//  dirty if changed
//-------------------------------------------------

u32 voodoo_1_device::reg_texture_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 1))
	{
		if (data != m_tmu[0].regs().read(regnum))
		{
			m_tmu[0].regs().write(regnum, data);
			m_tmu[0].mark_dirty();
		}
	}
	if (BIT(chipmask, 2))
	{
		if (data != m_tmu[1].regs().read(regnum))
		{
			m_tmu[1].regs().write(regnum, data);
			m_tmu[1].mark_dirty();
		}
	}
	return 0;
}


//-------------------------------------------------
//  reg_palette_w -- passive write to a palette or
//  NCC table; mark dirty if changed
//-------------------------------------------------

u32 voodoo_1_device::reg_palette_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 1)) m_tmu[0].ncc_w(regnum, data);
	if (BIT(chipmask, 2)) m_tmu[1].ncc_w(regnum, data);
	return 0;
}









void voodoo_1_device::recompute_video_timing(u32 hsyncon, u32 hsyncoff, u32 hvis, u32 hbp, u32 vsyncon, u32 vsyncoff, u32 vvis, u32 vbp)
{
	u32 htotal = hsyncoff + 1 + hsyncon + 1;
	u32 vtotal = vsyncoff + vsyncon;

	// create a new visarea from the backporch and visible values
	rectangle visarea(hbp, hbp + std::max(s32(hvis) - 1, 0), vbp, vbp + std::max(s32(vvis) - 1, 0));

	// keep within bounds
	visarea.max_x = std::min<s32>(visarea.max_x, htotal - 1);
	visarea.max_y = std::min<s32>(visarea.max_y, vtotal - 1);

	// compute the new period for standard res, medium res, and VGA res
	attoseconds_t stdperiod = HZ_TO_ATTOSECONDS(15750) * vtotal;
	attoseconds_t medperiod = HZ_TO_ATTOSECONDS(25000) * vtotal;
	attoseconds_t vgaperiod = HZ_TO_ATTOSECONDS(31500) * vtotal;

	// compute a diff against the current refresh period
	attoseconds_t refresh = screen().frame_period().attoseconds();
	attoseconds_t stddiff = std::abs(stdperiod - refresh);
	attoseconds_t meddiff = std::abs(medperiod - refresh);
	attoseconds_t vgadiff = std::abs(vgaperiod - refresh);

	osd_printf_debug("hSync=%d-%d, bp=%d, vis=%d  vSync=%d-%d, bp=%d, vis=%d\n", hsyncon, hsyncoff, hbp, hvis, vsyncon, vsyncoff, vbp, vvis);
	osd_printf_debug("Horiz: %d-%d (%d total)  Vert: %d-%d (%d total) -- ", visarea.min_x, visarea.max_x, htotal, visarea.min_y, visarea.max_y, vtotal);

	// configure the screen based on which one matches the closest
	if (stddiff < meddiff && stddiff < vgadiff)
	{
		screen().configure(htotal, vtotal, visarea, stdperiod);
		osd_printf_debug("Standard resolution, %f Hz\n", ATTOSECONDS_TO_HZ(stdperiod));
	}
	else if (meddiff < vgadiff)
	{
		screen().configure(htotal, vtotal, visarea, medperiod);
		osd_printf_debug("Medium resolution, %f Hz\n", ATTOSECONDS_TO_HZ(medperiod));
	}
	else
	{
		screen().configure(htotal, vtotal, visarea, vgaperiod);
		osd_printf_debug("VGA resolution, %f Hz\n", ATTOSECONDS_TO_HZ(vgaperiod));
	}

	// configure the new framebuffer info
	m_width = hvis;
	m_height = vvis;
	m_xoffs = hbp;
	m_yoffs = vbp;
	m_vsyncstart = vsyncoff;
	m_vsyncstop = vsyncon;
	osd_printf_debug("yoffs: %d vsyncstart: %d vsyncstop: %d\n", vbp, m_vsyncstart, m_vsyncstop);

	adjust_vblank_start_timer();
}



//-------------------------------------------------
//  triangle - execute the 'triangle' command
//-------------------------------------------------

s32 voodoo_1_device::triangle()
{
	g_profiler.start(PROFILER_USER2);

	// allocate polygon information now
	auto &poly = m_renderer->alloc_poly();

	// determine the draw buffer
	poly.destbase = draw_buffer_indirect(m_reg.fbz_mode().draw_buffer());
	if (poly.destbase == nullptr)
		return TRIANGLE_SETUP_CLOCKS;
	poly.depthbase = aux_buffer();
	poly.clipleft = m_reg.clip_left();
	poly.clipright = m_reg.clip_right();
	poly.cliptop = m_reg.clip_top();
	poly.clipbottom = m_reg.clip_bottom();

	// fill in triangle parameters
	poly.ax = m_reg.ax();
	poly.ay = m_reg.ay();
	poly.startr = m_reg.start_r();
	poly.startg = m_reg.start_g();
	poly.startb = m_reg.start_b();
	poly.starta = m_reg.start_a();
	poly.startz = m_reg.start_z();
	poly.startw = m_reg.start_w();
	poly.drdx = m_reg.dr_dx();
	poly.dgdx = m_reg.dg_dx();
	poly.dbdx = m_reg.db_dx();
	poly.dadx = m_reg.da_dx();
	poly.dzdx = m_reg.dz_dx();
	poly.dwdx = m_reg.dw_dx();
	poly.drdy = m_reg.dr_dy();
	poly.dgdy = m_reg.dg_dy();
	poly.dbdy = m_reg.db_dy();
	poly.dady = m_reg.da_dy();
	poly.dzdy = m_reg.dz_dy();
	poly.dwdy = m_reg.dw_dy();

	// perform subpixel adjustments -- note that the documentation indicates this
	// is done in the internal registers, so do it there
	if (m_reg.fbz_colorpath().cca_subpixel_adjust())
	{
		s32 dx = 8 - (poly.ax & 15);
		s32 dy = 8 - (poly.ay & 15);

		// adjust iterated R,G,B,A and W/Z
		m_reg.write(voodoo_regs::reg_startR, poly.startr += (dy * poly.drdy + dx * poly.drdx) >> 4);
		m_reg.write(voodoo_regs::reg_startG, poly.startg += (dy * poly.dgdy + dx * poly.dgdx) >> 4);
		m_reg.write(voodoo_regs::reg_startB, poly.startb += (dy * poly.dbdy + dx * poly.dbdx) >> 4);
		m_reg.write(voodoo_regs::reg_startA, poly.starta += (dy * poly.dady + dx * poly.dadx) >> 4);
		m_reg.write(voodoo_regs::reg_startZ, poly.startz += (dy * poly.dzdy + dx * poly.dzdx) >> 4);
		m_reg.write_start_w(poly.startw += (dy * poly.dwdy + dx * poly.dwdx) >> 4);

		// adjust iterated W/S/T for TMU 0
		auto &tmu0regs = m_tmu[0].regs();
		tmu0regs.write_start_w(tmu0regs.start_w() + ((dy * tmu0regs.dw_dy() + dx * tmu0regs.dw_dx()) >> 4));
		tmu0regs.write_start_s(tmu0regs.start_s() + ((dy * tmu0regs.ds_dy() + dx * tmu0regs.ds_dx()) >> 4));
		tmu0regs.write_start_t(tmu0regs.start_t() + ((dy * tmu0regs.dt_dy() + dx * tmu0regs.dt_dx()) >> 4));

		// adjust iterated W/S/T for TMU 1
		if (BIT(m_chipmask, 2))
		{
			auto &tmu1regs = m_tmu[1].regs();
			tmu1regs.write_start_w(tmu1regs.start_w() + ((dy * tmu1regs.dw_dy() + dx * tmu1regs.dw_dx()) >> 4));
			tmu1regs.write_start_s(tmu1regs.start_s() + ((dy * tmu1regs.ds_dy() + dx * tmu1regs.ds_dx()) >> 4));
			tmu1regs.write_start_t(tmu1regs.start_t() + ((dy * tmu1regs.dt_dy() + dx * tmu1regs.dt_dx()) >> 4));
		}
	}

	// fill in texture 0 parameters
	poly.tex0 = nullptr;
	if (poly.raster.texmode0().raw() != 0xffffffff)
	{
		auto &tmu0regs = m_tmu[0].regs();
		poly.starts0 = tmu0regs.start_s();
		poly.startt0 = tmu0regs.start_t();
		poly.startw0 = tmu0regs.start_w();
		poly.ds0dx = tmu0regs.ds_dx();
		poly.dt0dx = tmu0regs.dt_dx();
		poly.dw0dx = tmu0regs.dw_dx();
		poly.ds0dy = tmu0regs.ds_dy();
		poly.dt0dy = tmu0regs.dt_dy();
		poly.dw0dy = tmu0regs.dw_dy();
		poly.tex0 = &m_tmu[0].prepare_texture(*m_renderer.get());
		if (DEBUG_STATS)
			m_stats.m_texture_mode[tmu0regs.texture_mode().format()]++;
	}

	// fill in texture 1 parameters
	poly.tex1 = nullptr;
	if (poly.raster.texmode1().raw() != 0xffffffff)
	{
		auto &tmu1regs = m_tmu[1].regs();
		poly.starts1 = tmu1regs.start_s();
		poly.startt1 = tmu1regs.start_t();
		poly.startw1 = tmu1regs.start_w();
		poly.ds1dx = tmu1regs.ds_dx();
		poly.dt1dx = tmu1regs.dt_dx();
		poly.dw1dx = tmu1regs.dw_dx();
		poly.ds1dy = tmu1regs.ds_dy();
		poly.dt1dy = tmu1regs.dt_dy();
		poly.dw1dy = tmu1regs.dw_dy();
		poly.tex1 = &m_tmu[1].prepare_texture(*m_renderer.get());
		if (DEBUG_STATS)
			m_stats.m_texture_mode[tmu1regs.texture_mode().format()]++;
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
	vert[0].x = float(m_reg.ax()) * (1.0f / 16.0f);
	vert[0].y = float(m_reg.ay()) * (1.0f / 16.0f);
	vert[1].x = float(m_reg.bx()) * (1.0f / 16.0f);
	vert[1].y = float(m_reg.by()) * (1.0f / 16.0f);
	vert[2].x = float(m_reg.cx()) * (1.0f / 16.0f);
	vert[2].y = float(m_reg.cy()) * (1.0f / 16.0f);

	// enqueue a triangle
	s32 pixels = m_renderer->enqueue_triangle(poly, vert);

	// update stats
	m_reg.add(voodoo_regs::reg_fbiTrianglesOut, 1);
	if (DEBUG_STATS)
		m_stats.m_triangles++;

	g_profiler.stop();

	if (LOG_REGISTERS)
		logerror("cycles = %d\n", TRIANGLE_SETUP_CLOCKS + pixels);

	// 1 pixel per clock, plus some setup time
	return TRIANGLE_SETUP_CLOCKS + pixels;
}




DEFINE_DEVICE_TYPE(VOODOO_1, voodoo_1_device, "voodoo_1", "3dfx Voodoo Graphics")













//-------------------------------------------------
//  command_fifo - constructor
//-------------------------------------------------

command_fifo::command_fifo(voodoo_2_device &device) :
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


void voodoo_2_device::device_start()
{
	// start like a Voodoo-1
	voodoo_1_device::device_start();

	// TMU configuration has an extra bit
	m_renderer->set_tmu_config(m_renderer->tmu_config() | 0x800);

	// initialize Voodoo 2 additions
	m_sverts = 0;
	m_cmdfifo.init(m_fbram, m_fbmask + 1);
}

void voodoo_2_device::recompute_video_memory()
{
	// for backwards compatibility, the triple-buffered bit is still supported
	u32 config = m_reg.fbi_init2().enable_triple_buf();

	// but if left at 0, configuration comes from fbiInit5 instead
	if (config == 0)
		config = m_reg.fbi_init5().buffer_allocation();

	// 6-bit tile count is assembled from various bits; tiles are 32x32
	u32 xtiles = m_reg.fbi_init6().x_video_tiles_bit0() |
				 (m_reg.fbi_init1().x_video_tiles() << 1) |
				 (m_reg.fbi_init1().x_video_tiles_bit5() << 5);
	recompute_video_memory_common(config, xtiles * 32);
}

//-------------------------------------------------
//  register_save - register for save states
//-------------------------------------------------

void command_fifo::register_save(save_proxy &save)
{
	save.save_item(NAME(m_enable));
	save.save_item(NAME(m_count_holes));
	save.save_item(NAME(m_ram_base));
	save.save_item(NAME(m_ram_end));
	save.save_item(NAME(m_read_index));
	save.save_item(NAME(m_address_min));
	save.save_item(NAME(m_address_max));
	save.save_item(NAME(m_depth));
	save.save_item(NAME(m_holes));
}


//-------------------------------------------------
//  execute_if_ready - execute everything we have
//  the data for, until we encounter an operation
//  that consumes a non-zero number of cycles
//-------------------------------------------------

u32 command_fifo::execute_if_ready()
{
	while (1)
	{
		// all CMDFIFO commands need at least one word
		if (m_depth == 0)
			return 0;

		// see if we have enough for the current command
		u32 const needed_depth = words_needed(peek_next());
		if (m_depth < needed_depth)
			return 0;

		// read the next command and handle it based on the low 3 bits
		u32 command = read_next();
		u32 cycles = (this->*s_packet_handler[BIT(command, 0, 3)])(command);

		// if the number of cycles is non-zero, return
		if (cycles > 0)
			return cycles;
	}
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
	if (!m_device.operation_pending())
	{
		s32 cycles = execute_if_ready();
		if (cycles > 0)
		{
			attotime curtime = m_device.machine().time();
			m_device.m_operation_end = curtime + m_device.clocks_to_attotime(cycles);

			if (LOG_FIFO_VERBOSE)
				m_device.logerror("VOODOO.FIFO:direct write start at %s end at %s\n", curtime.as_string(18), m_device.m_operation_end.as_string(18));
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
		u32 chipmask = m_device.chipmask_from_offset(target);
		for (u32 regbit = 0; regbit < count; regbit++, target += inc)
			cycles += m_device.m_regtable[target & 0xff].write(m_device, chipmask, target & 0xff, read_next());
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
			{
				u32 regnum = voodoo_regs::reg_bltSrcBaseAddr + (regbit - 3);
				cycles += m_device.m_regtable[regnum].write(m_device, 0x1, regnum, read_next());
			}
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
	setup_vertex svert = { 0 };
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
		if ((code == 1 && trinum == 0) || (code == 0 && trinum % 3 == 0))
		{
			m_device.m_sverts = 1;
			m_device.m_svert[0] = m_device.m_svert[1] = m_device.m_svert[2] = svert;
		}

		// otherwise, add this to the list
		else
		{
			// for strip mode, shuffle vertex 1 down to 0
			if (!BIT(command, 22))
				m_device.m_svert[0] = m_device.m_svert[1];

			// copy 2 down to 1 and add our new one regardless
			m_device.m_svert[1] = m_device.m_svert[2];
			m_device.m_svert[2] = svert;

			// if we have enough, draw
			if (++m_device.m_sverts >= 3)
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
		u32 chipmask = m_device.chipmask_from_offset(target);
		for (u32 regbit = 15; regbit <= 28; regbit++, target++)
			if (BIT(command, regbit))
				cycles += m_device.m_regtable[target & 0xff].write(m_device, chipmask, target & 0xff, read_next());
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
	switch (BIT(command, 30, 2))
	{
		// Linear FB
		case 0:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: FB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				m_ram[target++ & m_mask] = little_endianize_int32(read_next());
			break;

		// 3D LFB
		case 2:
			if (LOG_CMDFIFO)
				m_device.logerror("  PACKET TYPE 5: 3D LFB count=%d dest=%08X bd2=%X bdN=%X\n", count, target, BIT(command, 26, 4), BIT(command, 22, 4));

			for (u32 word = 0; word < count; word++)
				m_device.lfb_w(target++, read_next(), 0xffffffff);
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
				m_device.texture_w(target++, read_next());
			break;
	}
	return 0;
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


voodoo_2_device::voodoo_2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, voodoo_model model) :
	voodoo_1_device(mconfig, type, tag, owner, clock, model),
	m_sverts(0),
	m_cmdfifo(*this)
#if USE_MEMORY_VIEWS
	m_regview(*this, "registers")
#endif
{
	for (int index = 0; index < std::size(m_regtable); index++)
		m_regtable[index].unpack(s_register_table[index], *this);
}


void voodoo_2_device::register_save(save_proxy &save, u32 total_allocation)
{
	voodoo_1_device::register_save(save, total_allocation);

	// Voodoo 2 stuff
	save.save_item(NAME(m_sverts));
	save.save_class(NAME(m_svert[0]));
	save.save_class(NAME(m_svert[1]));
	save.save_class(NAME(m_svert[2]));
	save.save_class(NAME(m_cmdfifo));
}

u32 voodoo_2_device::execute_fifos()
{
	// we might be in CMDFIFO mode
	if (m_cmdfifo.enabled())
		return m_cmdfifo.execute_if_ready();

	// otherwise, run the traditional memory FIFOs
	return voodoo_1_device::execute_fifos();
}


void voodoo_2_device::vblank_start(void *ptr, s32 param)
{
	voodoo_1_device::vblank_start(ptr, param);

	// signal PCI VBLANK rising IRQ on Voodoo-2 and later
	if (m_reg.intr_ctrl().vsync_rising_enable())
	{
		m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_RISING_GENERATED);
		if (!m_pciint_cb.isnull())
			m_pciint_cb(true);
	}
}


void voodoo_2_device::vblank_stop(void *ptr, s32 param)
{
	voodoo_1_device::vblank_stop(ptr, param);

	// signal PCI VBLANK falling IRQ on Voodoo-2 and later
	if (m_reg.intr_ctrl().vsync_falling_enable())
	{
		m_reg.clear_set(voodoo_regs::reg_intrCtrl, reg_intr_ctrl::EXTERNAL_PIN_ACTIVE, reg_intr_ctrl::VSYNC_FALLING_GENERATED);
		if (!m_pciint_cb.isnull())
			m_pciint_cb(true);
	}
}

//-------------------------------------------------
//  reg_hvretrace_r - hvRetrace register read
//-------------------------------------------------

u32 voodoo_2_device::reg_hvretrace_r(u32 chipmask, u32 regnum)
{
	// eat some cycles since people like polling here
	if (EAT_CYCLES)
		m_cpu->eat_cycles(10);

	// return 0 for vertical if vblank is active
	u32 result = m_vblank ? 0 : screen().vpos();
	return result |= screen().hpos() << 16;
}

u32 voodoo_2_device::reg_cmdfifoptr_r(u32 chipmask, u32 regnum)
{
	return m_cmdfifo.read_pointer();
}

u32 voodoo_2_device::reg_cmdfifodepth_r(u32 chipmask, u32 regnum)
{
	return m_cmdfifo.depth();
}

u32 voodoo_2_device::reg_cmdfifoholes_r(u32 chipmask, u32 regnum)
{
	return m_cmdfifo.holes();
}


// voodoo 2
u32 voodoo_2_device::reg_intrctrl_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_reg.write(regnum, data);

		// Setting bit 31 clears the PCI interrupts
		if (BIT(data, 31) && !m_pciint_cb.isnull())
			m_pciint_cb(false);
	}
	return 0;
}

//-------------------------------------------------
//  reg_video2_w -- write to a video configuration
//  register; synchronize then recompute everything
//-------------------------------------------------

u32 voodoo_2_device::reg_video2_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("video_configuration");
		m_reg.write(regnum, data);

		auto const hsync = m_reg.hsync<false>();
		auto const vsync = m_reg.vsync<false>();
		auto const back_porch = m_reg.back_porch<false>();
		auto const video_dimensions = m_reg.video_dimensions<false>();
		if (hsync.raw() != 0 && vsync.raw() != 0 && video_dimensions.raw() != 0 && back_porch.raw() != 0)
		{
			recompute_video_timing(
					hsync.hsync_on(), hsync.hsync_off(),
					video_dimensions.xwidth(), back_porch.horizontal() + 2,
					vsync.vsync_on(), vsync.vsync_off(),
					video_dimensions.yheight(), back_porch.vertical());
		}
	}
	return 0;
}

// voodoo 2
u32 voodoo_2_device::reg_sargb_w(u32 chipmask, u32 regnum, u32 data)
{
	rgb_t rgbdata(data);
	m_reg.write_float(voodoo_regs::reg_sAlpha, float(rgbdata.a()));
	m_reg.write_float(voodoo_regs::reg_sRed, float(rgbdata.r()));
	m_reg.write_float(voodoo_regs::reg_sGreen, float(rgbdata.g()));
	m_reg.write_float(voodoo_regs::reg_sBlue, float(rgbdata.b()));
	return 0;
}

// voodoo 2
u32 voodoo_2_device::reg_userintr_w(u32 chipmask, u32 regnum, u32 data)
{
	m_renderer->wait("userIntrCMD");

	// Bit 5 of intrCtrl enables user interrupts
	if (m_reg.intr_ctrl().user_interrupt_enable())
	{
		// Bits 19:12 are set to cmd 9:2, bit 11 is user interrupt flag
		m_reg.clear_set(voodoo_regs::reg_intrCtrl,
			reg_intr_ctrl::EXTERNAL_PIN_ACTIVE | reg_intr_ctrl::USER_INTERRUPT_TAG_MASK,
			((data << 10) & reg_intr_ctrl::USER_INTERRUPT_TAG_MASK) | reg_intr_ctrl::USER_INTERRUPT_GENERATED);

		// Signal pci interrupt handler
		if (!m_pciint_cb.isnull())
			m_pciint_cb(true);
	}
	return 0;
}

u32 voodoo_2_device::reg_cmdfifo_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("cmdFifo write");
		m_reg.write(regnum, data);
		m_cmdfifo.set_base(BIT(m_reg.read(voodoo_regs::reg_cmdFifoBaseAddr), 0, 10) << 12);
		m_cmdfifo.set_end((BIT(m_reg.read(voodoo_regs::reg_cmdFifoBaseAddr), 16, 10) + 1) << 12);
		m_cmdfifo.set_address_min(m_reg.read(voodoo_regs::reg_cmdFifoAMin));
		m_cmdfifo.set_address_max(m_reg.read(voodoo_regs::reg_cmdFifoAMax));
	}
	return 0;
}

u32 voodoo_2_device::reg_cmdfifoptr_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("cmdFifoReadPtr");
		m_reg.write(regnum, data);
		m_cmdfifo.set_read_pointer(data);
	}
	return 0;
}

u32 voodoo_2_device::reg_cmdfifodepth_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("cmdFifoDepth");
		m_reg.write(regnum, data);
		m_cmdfifo.set_depth(data);
	}
	return 0;
}

u32 voodoo_2_device::reg_cmdfifoholes_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0))
	{
		m_renderer->wait("cmdFifoHoles");
		m_reg.write(regnum, data);
		m_cmdfifo.set_holes(data);
	}
	return 0;
}

u32 voodoo_2_device::reg_fbiinit5_7_w(u32 chipmask, u32 regnum, u32 data)
{
	if (BIT(chipmask, 0) && m_init_enable.enable_hw_init())
	{
		m_renderer->wait("fbiInit5-7");
		m_reg.write(regnum, data);
		if (regnum == voodoo_regs::reg_fbiInit5 || regnum == voodoo_regs::reg_fbiInit6)
			recompute_video_memory();
		m_cmdfifo.set_enable(m_reg.fbi_init7().cmdfifo_enable());
		m_cmdfifo.set_count_holes(!m_reg.fbi_init7().disable_cmdfifo_holes());
		update_register_view();
	}
	return 0;
}

u32 voodoo_2_device::reg_draw_tri_w(u32 chipmask, u32 regnum, u32 data)
{
	return draw_triangle();
}

u32 voodoo_2_device::reg_begin_tri_w(u32 chipmask, u32 regnum, u32 data)
{
	return begin_triangle();
}


//-------------------------------------------------
//  begin_triangle - execute the 'beginTri'
//  command
//-------------------------------------------------

s32 voodoo_2_device::begin_triangle()
{
	// extract setup data
	auto &sv = m_svert[2];
	sv.x = m_reg.read_float(voodoo_regs::reg_sVx);
	sv.y = m_reg.read_float(voodoo_regs::reg_sVy);
	sv.wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv.w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv.s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv.t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv.w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv.s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv.t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv.a = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv.r = m_reg.read_float(voodoo_regs::reg_sRed);
	sv.g = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv.b = m_reg.read_float(voodoo_regs::reg_sBlue);

	// spread it across all three verts and reset the count
	m_svert[0] = m_svert[1] = sv;
	m_sverts = 1;
	return 0;
}


//-------------------------------------------------
//  draw_triangle - execute the 'DrawTri'
//  command
//-------------------------------------------------

s32 voodoo_2_device::draw_triangle()
{
	// for strip mode, shuffle vertex 1 down to 0
	if (!m_reg.setup_mode().fan_mode())
		m_svert[0] = m_svert[1];

	// copy 2 down to 1 regardless
	m_svert[1] = m_svert[2];

	// extract setup data
	auto &sv = m_svert[2];
	sv.x = m_reg.read_float(voodoo_regs::reg_sVx);
	sv.y = m_reg.read_float(voodoo_regs::reg_sVy);
	sv.wb = m_reg.read_float(voodoo_regs::reg_sWb);
	sv.w0 = m_reg.read_float(voodoo_regs::reg_sWtmu0);
	sv.s0 = m_reg.read_float(voodoo_regs::reg_sS_W0);
	sv.t0 = m_reg.read_float(voodoo_regs::reg_sT_W0);
	sv.w1 = m_reg.read_float(voodoo_regs::reg_sWtmu1);
	sv.s1 = m_reg.read_float(voodoo_regs::reg_sS_Wtmu1);
	sv.t1 = m_reg.read_float(voodoo_regs::reg_sT_Wtmu1);
	sv.a = m_reg.read_float(voodoo_regs::reg_sAlpha);
	sv.r = m_reg.read_float(voodoo_regs::reg_sRed);
	sv.g = m_reg.read_float(voodoo_regs::reg_sGreen);
	sv.b = m_reg.read_float(voodoo_regs::reg_sBlue);

	// if we have enough verts, go ahead and draw
	int cycles = 0;
	if (++m_sverts >= 3)
		cycles = setup_and_draw_triangle();
	return cycles;
}


//-------------------------------------------------
//  setup_and_draw_triangle - process the setup
//  parameters and render the triangle
//-------------------------------------------------

s32 voodoo_2_device::setup_and_draw_triangle()
{
	auto &sv0 = m_svert[0];
	auto &sv1 = m_svert[1];
	auto &sv2 = m_svert[2];

	// compute the divisor, but we only need to know the sign up front
	// for backface culling
	float divisor = (sv0.x - sv1.x) * (sv0.y - sv2.y) - (sv0.x - sv2.x) * (sv0.y - sv1.y);

	// backface culling
	auto const setup_mode = m_reg.setup_mode();
	if (setup_mode.enable_culling())
	{
		int culling_sign = setup_mode.culling_sign();
		int divisor_sign = (divisor < 0);

		// if doing strips and ping pong is enabled, apply the ping pong
		if (!setup_mode.fan_mode() && !setup_mode.disable_ping_pong_correction())
			culling_sign ^= (m_sverts - 3) & 1;

		// if our sign matches the culling sign, we're done for
		if (divisor_sign == culling_sign)
			return TRIANGLE_SETUP_CLOCKS;
	}

	// compute the reciprocal now that we know we need it
	divisor = 1.0f / divisor;

	// grab the X/Ys at least
	m_reg.write(voodoo_regs::reg_vertexAx, s16(sv0.x * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexAy, s16(sv0.y * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexBx, s16(sv1.x * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexBy, s16(sv1.y * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexCx, s16(sv2.x * 16.0f));
	m_reg.write(voodoo_regs::reg_vertexCy, s16(sv2.y * 16.0f));

	// compute the dx/dy values
	float dx1 = sv0.y - sv2.y;
	float dx2 = sv0.y - sv1.y;
	float dy1 = sv0.x - sv1.x;
	float dy2 = sv0.x - sv2.x;

	// set up R,G,B
	float const argbzscale = 4096.0f;
	float const argbzdiv = argbzscale * divisor;
	if (setup_mode.setup_rgb())
	{
		m_reg.write(voodoo_regs::reg_startR, s32(sv0.r * argbzscale));
		m_reg.write(voodoo_regs::reg_dRdX, s32(((sv0.r - sv1.r) * dx1 - (sv0.r - sv2.r) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dRdY, s32(((sv0.r - sv2.r) * dy1 - (sv0.r - sv1.r) * dy2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_startG, s32(sv0.g * argbzscale));
		m_reg.write(voodoo_regs::reg_dGdX, s32(((sv0.g - sv1.g) * dx1 - (sv0.g - sv2.g) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dGdY, s32(((sv0.g - sv2.g) * dy1 - (sv0.g - sv1.g) * dy2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_startB, s32(sv0.b * argbzscale));
		m_reg.write(voodoo_regs::reg_dBdX, s32(((sv0.b - sv1.b) * dx1 - (sv0.b - sv2.b) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dBdY, s32(((sv0.b - sv2.b) * dy1 - (sv0.b - sv1.b) * dy2) * argbzdiv));
	}

	// set up alpha
	if (setup_mode.setup_alpha())
	{
		m_reg.write(voodoo_regs::reg_startA, s32(sv0.a * argbzscale));
		m_reg.write(voodoo_regs::reg_dAdX, s32(((sv0.a - sv1.a) * dx1 - (sv0.a - sv2.a) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dAdY, s32(((sv0.a - sv2.a) * dy1 - (sv0.a - sv1.a) * dy2) * argbzdiv));
	}

	// set up Z
	if (setup_mode.setup_z())
	{
		m_reg.write(voodoo_regs::reg_startZ, s32(sv0.z * argbzscale));
		m_reg.write(voodoo_regs::reg_dZdX, s32(((sv0.z - sv1.z) * dx1 - (sv0.z - sv2.z) * dx2) * argbzdiv));
		m_reg.write(voodoo_regs::reg_dZdY, s32(((sv0.z - sv2.z) * dy1 - (sv0.z - sv1.z) * dy2) * argbzdiv));
	}

	// set up Wb
	float const wscale = 65536.0f * 65536.0f;
	float const wdiv = wscale * divisor;
	auto &tmu0reg = m_tmu[0].regs();
	auto &tmu1reg = m_tmu[1].regs();
	if (setup_mode.setup_wb())
	{
		s64 startw = s64(sv0.wb * wscale);
		s64 dwdx = s64(((sv0.wb - sv1.wb) * dx1 - (sv0.wb - sv2.wb) * dx2) * wdiv);
		s64 dwdy = s64(((sv0.wb - sv2.wb) * dy1 - (sv0.wb - sv1.wb) * dy2) * wdiv);
		m_reg.write_start_w(startw);
		m_reg.write_dw_dx(dwdx);
		m_reg.write_dw_dy(dwdy);
		tmu0reg.write_start_w(startw);
		tmu0reg.write_dw_dx(dwdx);
		tmu0reg.write_dw_dy(dwdy);
		tmu1reg.write_start_w(startw);
		tmu1reg.write_dw_dx(dwdx);
		tmu1reg.write_dw_dy(dwdy);
	}

	// set up W0
	if (setup_mode.setup_w0())
	{
		s64 startw = s64(sv0.w0 * wscale);
		s64 dwdx = s64(((sv0.w0 - sv1.w0) * dx1 - (sv0.w0 - sv2.w0) * dx2) * wdiv);
		s64 dwdy = s64(((sv0.w0 - sv2.w0) * dy1 - (sv0.w0 - sv1.w0) * dy2) * wdiv);
		tmu0reg.write_start_w(startw);
		tmu0reg.write_dw_dx(dwdx);
		tmu0reg.write_dw_dy(dwdy);
		tmu1reg.write_start_w(startw);
		tmu1reg.write_dw_dx(dwdx);
		tmu1reg.write_dw_dy(dwdy);
	}

	// set up S0,T0
	float const stscale = 65536.0f * 65536.0f;
	float const stdiv = stscale * divisor;
	if (setup_mode.setup_st0())
	{
		s64 starts = s64(sv0.s0 * stscale);
		s64 dsdx = s64(((sv0.s0 - sv1.s0) * dx1 - (sv0.s0 - sv2.s0) * dx2) * stdiv);
		s64 dsdy = s64(((sv0.s0 - sv2.s0) * dy1 - (sv0.s0 - sv1.s0) * dy2) * stdiv);
		s64 startt = s64(sv0.t0 * stscale);
		s64 dtdx = s64(((sv0.t0 - sv1.t0) * dx1 - (sv0.t0 - sv2.t0) * dx2) * stdiv);
		s64 dtdy = s64(((sv0.t0 - sv2.t0) * dy1 - (sv0.t0 - sv1.t0) * dy2) * stdiv);
		tmu0reg.write_start_s(starts);
		tmu0reg.write_start_t(startt);
		tmu0reg.write_ds_dx(dsdx);
		tmu0reg.write_dt_dx(dtdx);
		tmu0reg.write_ds_dy(dsdy);
		tmu0reg.write_dt_dy(dtdy);
		tmu1reg.write_start_s(starts);
		tmu1reg.write_start_t(startt);
		tmu1reg.write_ds_dx(dsdx);
		tmu1reg.write_dt_dx(dtdx);
		tmu1reg.write_ds_dy(dsdy);
		tmu1reg.write_dt_dy(dtdy);
	}

	// set up W1
	if (setup_mode.setup_w1())
	{
		s64 startw = s64(sv0.w1 * wscale);
		s64 dwdx = s64(((sv0.w1 - sv1.w1) * dx1 - (sv0.w1 - sv2.w1) * dx2) * wdiv);
		s64 dwdy = s64(((sv0.w1 - sv2.w1) * dy1 - (sv0.w1 - sv1.w1) * dy2) * wdiv);
		tmu1reg.write_start_w(startw);
		tmu1reg.write_dw_dx(dwdx);
		tmu1reg.write_dw_dy(dwdy);
	}

	// set up S1,T1
	if (setup_mode.setup_st1())
	{
		s64 starts = s64(sv0.s1 * stscale);
		s64 dsdx = s64(((sv0.s1 - sv1.s1) * dx1 - (sv0.s1 - sv2.s1) * dx2) * stdiv);
		s64 dsdy = s64(((sv0.s1 - sv2.s1) * dy1 - (sv0.s1 - sv1.s1) * dy2) * stdiv);
		s64 startt = s64(sv0.t1 * stscale);
		s64 dtdx = s64(((sv0.t1 - sv1.t1) * dx1 - (sv0.t1 - sv2.t1) * dx2) * stdiv);
		s64 dtdy = s64(((sv0.t1 - sv2.t1) * dy1 - (sv0.t1 - sv1.t1) * dy2) * stdiv);
		tmu1reg.write_start_s(starts);
		tmu1reg.write_start_t(startt);
		tmu1reg.write_ds_dx(dsdx);
		tmu1reg.write_dt_dx(dtdx);
		tmu1reg.write_ds_dy(dsdy);
		tmu1reg.write_dt_dy(dtdy);
	}

	// draw the triangle
	return triangle();
}


s32 voodoo_2_device::banshee_2d_w(offs_t offset, u32 data)
{
	// placeholder for Banshee 2D access
	return 0;
}


#if USE_MEMORY_VIEWS

//-------------------------------------------------
//  map_register_cmdfifo_w - handle a mapped write
//  to regular register space when cmdfifo is
//  enabled
//-------------------------------------------------

void voodoo_2_device::map_register_cmdfifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if the register is not writable in cmdfifo mode, ignore it
	u32 regnum = offset & 0xff;
	if (!m_reg.is_writethru(regnum))
	{
		// track swap buffers regardless
		if (regnum == voodoo_regs::reg_swapbufferCMD)
			m_swaps_pending++;
		logerror("Ignoring write to %s in CMDFIFO mode\n", m_reg.name(offset));
		return;
	}
	map_register_w(offset, data, mem_mask);
}


//-------------------------------------------------
//  map_cmdfifo_w - handle a mapped write to the
//  cmdfifo
//-------------------------------------------------

void voodoo_2_device::map_cmdfifo_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(offset, 18-2))
		data = swapendian_int32(data);
	m_cmdfifo[0].write_direct(BIT(offset, 0, 16), data);
}

#endif


//**************************************************************************
//  VOODOO 2 MEMORY MAP
//**************************************************************************

//-------------------------------------------------
//  core_map - device map for core memory access
//-------------------------------------------------

void voodoo_2_device::core_map(address_map &map)
{
	printf("voodoo_2_device::core_map\n");

	// Voodoo-2 memory map:
	//
	// cmdfifo = fbi_init7().cmdfifo_enable()
	//
	//   00ab----`--ccccrr`rrrrrr-- Register access (if cmdfifo == 0)
	//                                a = alternate register map if fbi_init3().tri_register_remap()
	//                                b = byte swizzle data if fbi_init0().swizzle_reg_writes()
	//                                c = chip mask select
	//                                r = register index ($00-$FF)
	//   000-----`------rr`rrrrrr-- Register access (if cmdfifo == 1)
	//                                r = register index ($00-$FF)
	//   001--boo`oooooooo`oooooo-- CMDFifo write (if cmdfifo == 1)
	//                                b = byte swizzle data
	//                                o = cmdfifo offset
	//   01-yyyyy`yyyyyxxx`xxxxxxx- Linear frame buffer access (16-bit)
	//   01yyyyyy`yyyyxxxx`xxxxxx-- Linear frame buffer access (32-bit)
	//   1-ccllll`tttttttt`sssssss- Texture memory access, where:
	//                                c = chip mask select
	//                                l = LOD
	//                                t = Y index
	//                                s = X index
	//
#if USE_MEMORY_VIEWS
	map(0x000000, 0x3fffff).view(m_regview);
	// cmdfifo == 0
	m_regview[0](0x000000, 0x3fffff).rw(FUNC(voodoo_2_device::map_register_r), FUNC(voodoo_2_device::map_register_w));
	// cmdfifo == 1
	m_regview[1](0x000000, 0x0003ff).mirror(0x1ffc00).rw(FUNC(voodoo_2_device::map_register_r), FUNC(voodoo_2_device::map_register_cmdfifo_w));
	m_regview[1](0x200000, 0x27ffff).mirror(0x180000).w(FUNC(voodoo_2_device::map_cmdfifo_w));
#else
	map(0x000000, 0x3fffff).rw(FUNC(voodoo_2_device::map_register_r), FUNC(voodoo_2_device::map_register_w));
#endif

	map(0x400000, 0x7fffff).rw(FUNC(voodoo_2_device::map_lfb_r), FUNC(voodoo_2_device::map_lfb_w));
	map(0x800000, 0xffffff).w(FUNC(voodoo_2_device::map_texture_w));
}

u32 voodoo_2_device::read(offs_t offset, u32 mem_mask)
{
	switch (offset >> (22-2))
	{
		case 0x000000 >> 22:
			return map_register_r(offset);

		case 0x400000 >> 22:
			return map_lfb_r(offset - 0x400000/4);

		default:
			return 0xffffffff;
	}
}

void voodoo_2_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	switch (offset >> (22-2))
	{
		case 0x000000 >> 22:
			map_register_w(offset, data, mem_mask);
			break;

		case 0x400000 >> 22:
			map_lfb_w(offset - 0x400000/4, data, mem_mask);
			break;

		case 0x800000 >> 22:
		case 0xc00000 >> 22:
			map_texture_w(offset - 0x800000/4, data, mem_mask);
			break;
	}
}

//-------------------------------------------------
//  update_register_view - switch the view in
//  response to changes in relevant bits
//-------------------------------------------------

#if USE_MEMORY_VIEWS
void voodoo_2_device::update_register_view()
{
	m_regview.select(m_reg.fbi_init7().cmdfifo_enable());
}
#endif


//-------------------------------------------------
//  map_register_r - handle a mapped read from
//  regular register space
//-------------------------------------------------

u32 voodoo_2_device::map_register_r(offs_t offset)
{
	prepare_for_read();

	// extract chipmask and register
	u32 chipmask = chipmask_from_offset(offset);
	u32 regnum = BIT(offset, 0, 8);

	// look up the register
	return m_regtable[regnum].read(*this, chipmask, regnum);
}


//-------------------------------------------------
//  map_lfb_r - handle a mapped read from LFB space
//-------------------------------------------------

u32 voodoo_2_device::map_lfb_r(offs_t offset)
{
	prepare_for_read();
	return lfb_r(offset);
}


//-------------------------------------------------
//  map_register_w - handle a mapped write to
//  regular register space
//-------------------------------------------------

void voodoo_2_device::map_register_w(offs_t offset, u32 data, u32 mem_mask)
{
	bool pending = prepare_for_write();

#if !USE_MEMORY_VIEWS
	// handle cmdfifo writes
	if (BIT(offset, 21-2) && m_reg.fbi_init7().cmdfifo_enable())
	{
		// check for byte swizzling (bit 18)
		if (BIT(offset, 18-2))
			data = swapendian_int32(data);
		m_cmdfifo.write_direct(BIT(offset, 0, 16), data);
		return;
	}
#endif

	// extract chipmask and register
	u32 chipmask = chipmask_from_offset(offset);
	u32 regnum = BIT(offset, 0, 8);

	// handle register swizzling
	if (BIT(offset, 20-2) && m_reg.fbi_init0().swizzle_reg_writes())
		data = swapendian_int32(data);

	// handle aliasing
	if (BIT(offset, 21-2) && m_reg.fbi_init3().tri_register_remap())
		offset = (offset & ~0xff) | m_reg.alias(offset);

	// look up the register
	auto const &regentry = m_regtable[regnum];

	// if this is non-FIFO command, execute immediately
	if (!regentry.is_fifo())
		return void(regentry.write(*this, chipmask, regnum, data));

	// track swap buffers
	if (regnum == voodoo_regs::reg_swapbufferCMD)
		m_swaps_pending++;

#if !USE_MEMORY_VIEWS
	// if cmdfifo is enabled, ignore everything else
	if (m_reg.fbi_init7().cmdfifo_enable())
	{
		logerror("Ignoring write to %s when CMDFIFO is enabled\n", regentry.name());
		return;
	}
#endif

	// if we're busy add to the fifo
	if (pending && m_init_enable.enable_pci_fifo())
		return add_to_fifo((chipmask << 8) | regnum | memory_fifo::TYPE_REGISTER, data, mem_mask);

	// if we get a non-zero number of cycles back, mark things pending
	int cycles = regentry.write(*this, chipmask, regnum, data);
	if (cycles > 0)
	{
		// if we ended up with cycles, mark the operation pending
		m_operation_end = machine().time() + clocks_to_attotime(cycles);
		if (LOG_FIFO_VERBOSE)
			logerror("VOODOO.FIFO:direct write start at %s end at %s\n", machine().time().as_string(18), m_operation_end.as_string(18));
	}
}


//-------------------------------------------------
//  map_lfb_w - handle a mapped write to LFB space
//-------------------------------------------------

void voodoo_2_device::map_lfb_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if we're busy add to the fifo, else just execute immediately
	if (prepare_for_write() && m_init_enable.enable_pci_fifo())
		add_to_fifo(offset | memory_fifo::TYPE_LFB, data, mem_mask);
	else
		lfb_w(offset, data, mem_mask);
}


//-------------------------------------------------
//  map_texture_w - handle a mapped write to
//  texture space
//-------------------------------------------------

void voodoo_2_device::map_texture_w(offs_t offset, u32 data, u32 mem_mask)
{
	// if we're busy add to the fifo, else just execute immediately
	if (prepare_for_write() && m_init_enable.enable_pci_fifo())
		add_to_fifo(offset | memory_fifo::TYPE_TEXTURE, data, mem_mask);
	else
		texture_w(offset, data);
}


DEFINE_DEVICE_TYPE(VOODOO_2, voodoo_2_device, "voodoo_2", "3dfx Voodoo 2")


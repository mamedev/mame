// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/***************************************************************************

    NEC / VideoLogic PowerVR 250 (PMX1, sold on cards as Neon 250)

    PowerVR second generation 3D with 2D, video and display in one chip
    for the PCI and AGP buses.  It is the PC relative of the CLX2 of the
    Dreamcast and shares its 3D core, but everything around that core is
    different: a processor in the chip (the primary core) runs code the
    host loads, and that code takes the host's commands, drives the tile
    accelerator, starts the renders and feeds the display.

    Host view:
    - PCI configuration space
    - immediate registers, 16KB.  Most are a window on the implicit
      registers, which are otherwise reached through the write access
      register at 0c0 and the read access registers at 0c8 and 0c4
    - frame buffer, a 64MB window on the 8, 16 or 32MB of SDRAM

    The 3D core's registers are indexed through implicit registers 104
    (index) and 105 (data).

    References: PowerVR 250 Programming Guide (VideoLogic, draft A, 1998),
    PMX1 Preliminary Data Sheet (draft H, 1998), PMX1c Tile Accelerator
    Specification, Highlander Host Software Kernel Manager API Definition.

    TODO:
    - tile accelerator: culling, antialiasing, and tiling without a copy
    - 3D core: modifier volumes, dither, color keys, YUV and bump map
      textures, supersampling, filtering in two passes, the scaler; how
      long a render takes
    - 2D core: only the copy, blend, fill, line, 1-bit image and copy of
      source B that the Aristocrat MK6 uses, worked out from what its code
      writes to the port registers; the registers that it writes and
      nothing here reads, the raster operation (perhaps port register 4c)
      and what port register ff gives back are not known
    - video in, VGA, ROM and GPIO port, IIC
    - bus mastering: only the chip's own memory is reached
    - DAC feed: scaler, colour space conversion, lookup table, slave
      stream, cursor, line compare interrupt, sync polarities
    - a render is drawn while the machine runs on, and waited for when the
      host, the primary core or the display touch the memory it uses; reads
      by the primary core of pixels that are still being drawn do not wait
    - the 3D core is that of the CLX2, which sega/powervr2.cpp emulates
      for the Dreamcast with a renderer of its own: the two should share
      what reads the parameters and textures

    The primary core sees the memory where the frame buffer offset puts
    it, in any of 64 places, so the RAM is installed there when the offset
    is written rather than held in as many views.

***************************************************************************/

#include "emu.h"
#include "powervr250.h"

#include "video/poly.h"

#include "screen.h"

#include "corefloat.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#define LOG_CONFIG      (1U << 1)
#define LOG_IMPLICIT    (1U << 2)
#define LOG_3D          (1U << 3)
#define LOG_TA          (1U << 4)
#define LOG_DAC         (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(POWERVR250, powervr250_device, "powervr250", "NEC PowerVR 250")


//**************************************************************************
//  3D core types
//**************************************************************************

// a pass over a tile, as its region header asks
struct powervr250_device::tile_pass
{
	u32 opaque = 0;             // its triangles in the renderer's list of triangle numbers
	u32 opaque_count = 0;
	u32 translucent = 0;
	u32 translucent_count = 0;
	s32 background = -1;        // the triangle that is the background, or -1 to keep the depths of the pass before
	s16 x = 0;
	s16 y = 0;
	bool presorted = false;     // translucent triangles are drawn in the order given
	bool flush = false;         // the pixels go to the frame buffer after the pass
};

class powervr250_device::renderer : public poly_manager<float, powervr250_device::tile_pass, 0>
{
public:
	renderer(powervr250_device &device);

	void render();
	void finish();
	template <typename... T> void logerror(T &&... args) const { m_device.logerror(std::forward<T>(args)...); }

private:
	static constexpr u32 MAX_REGIONS = 0x10000;    // headers read before a scene without a last one is given up
	static constexpr u32 MAX_POINTERS = 0x100000;  // object pointers read before a list without an end is given up
	static constexpr u32 MAX_KEPT_TEXELS = 16 * 1024 * 1024;

	// a value that varies linearly over the screen
	struct plane
	{
		float dx = 0.0f;
		float dy = 0.0f;
		float c = 0.0f;         // at the origin of the screen

		float at(float x, float y) const { return dx * x + dy * y + c; }
	};

	// what the triangles of a strip share: the instruction words, taken apart
	struct surface
	{
		u32 isp = 0;            // the ISP/TSP instruction word as it is, for what only the setup of a triangle needs
		bool textured = false;
		bool offset = false;
		bool use_alpha = false;
		bool ignore_texture_alpha = false;
		bool color_clamp = false;
		bool bilinear = false;
		bool source_select = false;
		bool dest_select = false;
		u8 source_blend = 1;
		u8 dest_blend = 0;
		u8 fog = 2;
		u8 shading = 0;
		u8 mode_u = 0;
		u8 mode_v = 0;
		u8 lod_adjust = 4;      // in quarters

		u32 texels = 0;         // where in memory the texture, or its code book or palette, is
		u32 indexes = 0;        // where what selects from those is
		u16 width = 8;
		u16 height = 8;
		u16 stride = 8;         // texels from a line to the next, when not twiddled
		u8 log_u = 3;
		u8 log_v = 3;
		u8 format = 0;
		bool vq = false;
		bool indexed = false;   // a byte for each texel selects from a palette
		bool scan_order = false;
		bool mipmapped = false;
		const u32 *decoded = nullptr; // all its levels as 32-bit pixels, the largest first, when that is kept
	};

	// a texture that is costly to read from memory, kept as 32-bit pixels for as long as its memory is not written
	struct texture
	{
		std::unique_ptr<u32 []> pixels;
		u32 texels = 0;         // of all its levels together
		u32 first[2] = { 0, 0 }; // the two stretches of memory it is made from
		u32 bytes[2] = { 0, 0 };
		u32 made = 0;           // the render it was made in
		u32 used = 0;           // the last render it was found good and used in
	};

	struct triangle
	{
		// the corners from the top down, and how far X moves with each line down an edge
		float x[3];
		float y[3];
		float slope_long;       // first to last
		float slope_upper;      // first to middle
		float slope_lower;      // middle to last

		plane depth;            // what the ISP compares, 1/W or Z
		plane invw;
		plane u;                // these are over W, as the colors are
		plane v;
		plane base[4];          // blue, green, red, alpha
		plane offset[4];
		float lod;              // texels to a pixel, times the square of W
		u32 surface;
		u8 compare;
		bool depth_write;
		bool whole;             // the background: all of a tile whatever its corners
	};

	struct strip
	{
		u32 surface;
		u32 vertices;           // address of the first
		u8 size;                // of a vertex in words
		s32 triangle[6];        // number of each once set up: -2 not yet, -1 not drawn
	};

	struct vertex
	{
		float x, y, depth, invw, u, v;
		u32 base, offset;
	};

	void read_list(u32 pointer);
	s32 find_strip(u32 pointer);
	surface make_surface(u32 isp, u32 tsp, u32 tcw);
	s32 setup_triangle(strip &from, u32 number, bool background);

	void render_row(s32 scanline, const extent_t &extent, const tile_pass &tile, int threadid);
	bool span(const triangle &tri, float y, int left, int right, int &first, int &end) const;
	void shade_tags(s32 scanline, int left, int count, const s32 *tag);
	void shade(const triangle &tri, s32 scanline, int first, int end);
	void write_row(s32 scanline, int left, int count);

	u32 sample(const surface &surf, float u, float v, u32 level) const;
	u32 texel(const surface &surf, u32 level, s32 x, s32 y) const;
	const u32 *decoded_texture(const surface &surf);
	u32 decode(u32 format, u32 offset, bool replicate) const;
	u32 fog_density(float invw) const;
	void use(u32 offset, u32 bytes);

	u8 byte(u32 offset) const { return m_device.m_vram[(offset & m_mask) >> 2] >> (8 * (offset & 3)); }
	u16 word(u32 offset) const { return m_device.m_vram[(offset & m_mask) >> 2] >> (8 * (offset & 2)); }
	u32 dword(u32 offset) const { return m_device.m_vram[(offset & m_mask) >> 2]; }

	powervr250_device &m_device;
	u32 m_mask;                 // of an offset in the chip's memory

	// the scene of the render being done
	std::vector<tile_pass> m_passes;
	std::vector<triangle> m_tris;       // each set up once, whatever the number of tiles it is in
	std::vector<surface> m_surfaces;
	std::vector<strip> m_strips;
	std::vector<u32> m_lists;   // triangle numbers, tile after tile
	std::unordered_map<u32, u32> m_strip_index;

	// a render is drawn while the machine runs on, until something needs the memory it uses
	bool m_drawing;
	std::vector<u32> m_pages_used;

	// textures kept from render to render
	std::map<std::pair<u64, u64>, texture> m_textures;
	u32 m_kept_texels;

	// the accumulation buffers and the depths, for all of the tiles at once
	std::vector<u32> m_accumulation[2];
	std::vector<float> m_depth;
	int m_width;
	int m_height;

	// the registers as they are when a render starts
	u32 m_param_base;
	u32 m_pointer_base;
	u32 m_object_base;
	bool m_zsort;
	bool m_intensity_volumes;
	float m_isp_sample;         // where in a pixel the ISP and the TSP take their values
	float m_tsp_sample;
	float m_texel_sample;
	float m_cull_value;
	float m_background_depth;
	u32 m_texture_stride;
	u32 m_fog_table_color;
	u32 m_fog_vertex_color;
	float m_fog_scale;          // what 1/W is multiplied by to find its place in the fog table
	u16 m_fog_table[128];
	u32 m_clamp_max;
	u32 m_clamp_min;
	u32 m_target;               // where in memory the first pixel goes, or ~0 when nowhere
	u32 m_target_stride;
	u8 m_pack_mode;
	u8 m_alpha_threshold;
	u8 m_kval;
	int m_clip_left;
	int m_clip_right;
	int m_clip_top;
	int m_clip_bottom;

	// what the scene asks for that is not emulated
	bool m_host_memory;
	bool m_modifier_volumes;
};


powervr250_device::powervr250_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, POWERVR250, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_core(*this, "core")
	, m_irq_cb(*this)
	, m_vram_size(32 * 1024 * 1024)
	, m_render_serial(0)
	, m_dac_timer(nullptr)
	, m_render_timer(nullptr)
	, m_war(0)
	, m_index_low(0)
	, m_index_low_held(false)
	, m_irq_level(false)
	, m_fb_mapped(0xff)
	, m_ta_setup(0)
	, m_ta_clip(0)
	, m_ta_copy_dest(0)
	, m_ta_copy_limit(0)
	, m_ta_copy_words(0)
	, m_ta_tiling(false)
	, m_ta_list_words(0)
	, m_ta_list_position(0)
	, m_ta_header_words(3)
	, m_ta_pointer(0)
	, m_ta_tile_x(0)
	, m_ta_tile_y(0)
	, m_ta_status(TA_EOL)
	, m_dac_pll(0)
	, m_dac_busy(false)
	, m_dac_line(0)
	, m_dac_htotal(0)
	, m_dac_feeding_count(0)
	, m_dac_after_blank(true)
	, m_dac_showing_count(0)
{
}

powervr250_device::~powervr250_device()
{
}

void powervr250_device::device_add_mconfig(machine_config &config)
{
	PMX1CORE(config, m_core, DERIVED_CLOCK(1, 1));
	m_core->implicit_read().set(FUNC(powervr250_device::implicit_r));
	m_core->implicit_write().set(FUNC(powervr250_device::implicit_w));
	m_core->ta_instruction().set(FUNC(powervr250_device::ta_instruction_w));
	m_core->ta_status().set(FUNC(powervr250_device::ta_status_r));
	m_core->dac_feed().set(FUNC(powervr250_device::dac_feed_w));
	m_core->dac_busy().set(FUNC(powervr250_device::dac_busy_r));
	m_core->port_read().set(FUNC(powervr250_device::port_r));
	m_core->port_write().set(FUNC(powervr250_device::port_w));
}

void powervr250_device::device_start()
{
	if ((m_vram_size != 8 * 1024 * 1024) && (m_vram_size != 16 * 1024 * 1024) && (m_vram_size != 32 * 1024 * 1024))
		throw emu_fatalerror("%s: memory of %u bytes, the chip takes 8, 16 or 32MB", tag(), m_vram_size);

	m_vram = make_unique_clear<u32 []>(m_vram_size / 4);
	m_page_written = make_unique_clear<u32 []>(m_vram_size >> PAGE_SHIFT);
	m_page_in_use = make_unique_clear<u8 []>(m_vram_size >> PAGE_SHIFT);
	m_renderer = std::make_unique<renderer>(*this);

	m_dac_timer = timer_alloc(FUNC(powervr250_device::dac_span_done), this);
	m_render_timer = timer_alloc(FUNC(powervr250_device::render_done), this);

	std::fill(std::begin(m_config), std::end(m_config), 0);
	std::fill(std::begin(m_core_pll), std::end(m_core_pll), 0);
	std::fill(std::begin(m_implicit), std::end(m_implicit), 0);
	std::fill(std::begin(m_rar), std::end(m_rar), 0);
	std::fill(std::begin(m_regs3d), std::end(m_regs3d), 0);
	std::fill(std::begin(m_ta_list_page), std::end(m_ta_list_page), 0);
	std::fill(std::begin(m_ta_vertex_page), std::end(m_ta_vertex_page), 0);
	std::fill(std::begin(m_ta_header), std::end(m_ta_header), 0);
	std::fill(std::begin(m_dac_reg), std::end(m_dac_reg), 0);

	save_pointer(NAME(m_vram), m_vram_size / 4);
	save_item(NAME(m_config));
	save_item(NAME(m_core_pll));
	save_item(NAME(m_implicit));
	save_item(NAME(m_war));
	save_item(NAME(m_rar));
	save_item(NAME(m_regs3d));
	save_item(NAME(m_index_low));
	save_item(NAME(m_index_low_held));
	save_item(NAME(m_irq_level));
	save_item(NAME(m_ta_list_page));
	save_item(NAME(m_ta_setup));
	save_item(NAME(m_ta_clip));
	save_item(NAME(m_ta_copy_dest));
	save_item(NAME(m_ta_copy_limit));
	save_item(NAME(m_ta_copy_words));
	save_item(NAME(m_ta_tiling));
	save_item(NAME(m_ta_list_words));
	save_item(NAME(m_ta_list_position));
	save_item(NAME(m_ta_vertex_page));
	save_item(NAME(m_ta_header));
	save_item(NAME(m_ta_header_words));
	save_item(NAME(m_ta_pointer));
	save_item(STRUCT_MEMBER(m_ta_triangle, left));
	save_item(STRUCT_MEMBER(m_ta_triangle, right));
	save_item(STRUCT_MEMBER(m_ta_triangle, top));
	save_item(STRUCT_MEMBER(m_ta_triangle, bottom));
	save_item(NAME(m_ta_strip.left));
	save_item(NAME(m_ta_strip.right));
	save_item(NAME(m_ta_strip.top));
	save_item(NAME(m_ta_strip.bottom));
	save_item(NAME(m_ta_tile_x));
	save_item(NAME(m_ta_tile_y));
	save_item(NAME(m_ta_status));
	save_item(NAME(m_dac_pll));
	save_item(NAME(m_dac_reg));
	save_item(NAME(m_dac_busy));
	save_item(NAME(m_dac_line));
	save_item(NAME(m_dac_htotal));
	save_item(STRUCT_MEMBER(m_dac_feeding, address));
	save_item(STRUCT_MEMBER(m_dac_feeding, line));
	save_item(STRUCT_MEMBER(m_dac_feeding, lines));
	save_item(STRUCT_MEMBER(m_dac_feeding, pixels));
	save_item(STRUCT_MEMBER(m_dac_feeding, format));
	save_item(STRUCT_MEMBER(m_dac_feeding, blank));
	save_item(STRUCT_MEMBER(m_dac_showing, address));
	save_item(STRUCT_MEMBER(m_dac_showing, line));
	save_item(STRUCT_MEMBER(m_dac_showing, lines));
	save_item(STRUCT_MEMBER(m_dac_showing, pixels));
	save_item(STRUCT_MEMBER(m_dac_showing, format));
	save_item(STRUCT_MEMBER(m_dac_showing, blank));
	save_item(NAME(m_dac_feeding_count));
	save_item(NAME(m_dac_after_blank));
	save_item(NAME(m_dac_showing_count));
	save_item(NAME(m_2d));

	map_frame_buffer();
}

void powervr250_device::device_reset()
{
	std::fill(std::begin(m_config), std::end(m_config), 0);
	m_config[0x00 / 4] = 0x0067'1033;
	m_config[0x08 / 4] = 0x0300'0002;

	// core PLL 0, 1 and 2
	m_core_pll[1] = 0x21;
	m_core_pll[2] = 0x00;
	m_core_pll[3] = 0x00;
	m_core->set_unscaled_clock(core_frequency());

	std::fill(std::begin(m_implicit), std::end(m_implicit), 0);
	m_implicit[REG_THREAD_ENABLE] = 0x0001;
	m_implicit[REG_RESET_VECTOR_LO] = 0xfff8;
	m_implicit[REG_RESET_VECTOR_HI] = 0x0403;
	m_implicit[REG_DAC_PLL0] = 0x0021;
	m_implicit[REG_INTERRUPT_MASK] = 0xffff;
	m_implicit[REG_SOFTWARE_RESET] = 0x7fff;
	m_implicit[REG_PCI_DEVICE_ID] = 0x0067;
	m_implicit[REG_PCI_VENDOR_ID] = 0x1033;
	m_implicit[REG_SDRAM_CONFIG0] = 0x8981;
	m_implicit[0x281] = 0x1040;
	m_implicit[0x282] = 0x3101;
	m_implicit[0x283] = 0x0020;
	m_implicit[0x284] = 0x001f;
	m_implicit[0x285] = 0x0003;
	m_implicit[0x289] = 0x00ff;
	m_implicit[0x28b] = 0x00ff;
	m_implicit[0x28d] = 0x00ff;
	m_implicit[0x292] = 0xffff;

	m_war = 0;
	m_rar[0] = m_rar[1] = 0;
	std::fill(std::begin(m_regs3d), std::end(m_regs3d), 0);
	m_index_low = 0;
	m_index_low_held = false;
	update_irq();
	map_frame_buffer();

	// what the registers tell the primary core
	m_core->set_reset_vector((u32(m_implicit[REG_RESET_VECTOR_HI]) << 16) | m_implicit[REG_RESET_VECTOR_LO]);
	m_core->set_fb_offset(m_implicit[REG_FB_OFFSET] & 0x3f);
	for (u32 thread = 0; thread < pmx1core_device::THREADS; thread++)
		m_core->set_thread_enable(thread, BIT(m_implicit[REG_THREAD_ENABLE + thread], 0));
	m_core->core_reset_w(BIT(m_implicit[REG_SOFTWARE_RESET], 2));

	std::fill(std::begin(m_ta_list_page), std::end(m_ta_list_page), 0);
	m_ta_setup = 0;
	m_ta_clip = 0;
	m_ta_copy_dest = 0;
	m_ta_copy_limit = 0;
	m_ta_copy_words = 0;
	m_ta_tiling = false;
	m_ta_list_words = 0;
	m_ta_list_position = 0;
	std::fill(std::begin(m_ta_vertex_page), std::end(m_ta_vertex_page), 0);
	std::fill(std::begin(m_ta_header), std::end(m_ta_header), 0);
	m_ta_header_words = 3;
	m_ta_pointer = 0;
	std::fill(std::begin(m_ta_triangle), std::end(m_ta_triangle), ta_box());
	m_ta_strip = ta_box();
	m_ta_tile_x = m_ta_strip.left;
	m_ta_tile_y = m_ta_strip.top;
	m_ta_status = TA_EOL;
	m_render_timer->adjust(attotime::never);

	std::fill(std::begin(m_dac_reg), std::end(m_dac_reg), 0);
	m_dac_pll = m_implicit[REG_DAC_PLL0];
	m_dac_busy = false;
	m_dac_line = 0;
	m_dac_htotal = 0;
	m_dac_feeding_count = 0;
	m_dac_after_blank = true;
	m_dac_showing_count = 0;
	std::fill(std::begin(m_2d), std::end(m_2d), 0);
	m_dac_timer->adjust(attotime::never);
}

void powervr250_device::device_post_load()
{
	m_renderer->finish();
	map_frame_buffer();

	// nothing the renderer has kept can be relied on
	std::fill_n(m_page_written.get(), m_vram_size >> PAGE_SHIFT, m_render_serial);
}


//**************************************************************************
//  clocks
//**************************************************************************

u32 powervr250_device::pll_frequency(u32 m, u32 n, u32 p) const
{
	// Fref * N / (M * P), with M - 1 and N - 1 in the register (Programming Guide, "Clocking")
	// FIXME: the guide has the output divider P for the codes 0, 1, 3 and 7 only, as 1, 2, 4 and 8.  Halving the frequency
	// for each bit that is set gives those; that the other four codes work the same way is a guess
	const u32 divider = (m + 1) << std::popcount(p);
	return u32(u64(clock()) * (n + 1) / divider);
}

u32 powervr250_device::core_frequency() const
{
	return pll_frequency(BIT(m_core_pll[1], 0, 5), (BIT(m_core_pll[2], 0, 4) << 3) | BIT(m_core_pll[1], 5, 3), BIT(m_core_pll[2], 4, 3));
}

u32 powervr250_device::pixel_frequency() const
{
	// DACPLL_M, DACPLL_N and DACPLL_P of the setting that DAC PLL 1 was last written with (Programming Guide, "Clocking")
	return pll_frequency(BIT(m_dac_pll, 0, 5), BIT(m_dac_pll, 5, 7), BIT(m_dac_pll, 12, 3));
}


//**************************************************************************
//  host bus
//**************************************************************************

u32 powervr250_device::config_r(offs_t offset)
{
	return m_config[offset & 0x3f];
}

void powervr250_device::config_w(offs_t offset, u32 data, u32 mem_mask)
{
	offset &= 0x3f;
	LOGMASKED(LOG_CONFIG, "%s: configuration %02x = %08x & %08x\n", machine().describe_context(), offset * 4, data, mem_mask);

	switch (offset)
	{
	case 0x00 / 4:
	case 0x08 / 4:
		break;

	case 0x70 / 4:
		{
			// the core PLL is set a byte at a time, clocked in by bit 14
			const u32 before = m_config[offset];
			COMBINE_DATA(&m_config[offset]);
			if (BIT(m_config[offset], 14) && !BIT(before, 14))
			{
				const u32 address = BIT(m_config[offset], 8, 6);
				if (address >= 1 && address <= 3)
					m_core_pll[address] = m_config[offset] & 0xff;
			}
		}
		break;

	default:
		COMBINE_DATA(&m_config[offset]);
		break;
	}
}

void powervr250_device::map(address_map &map)
{
	// the immediate registers but for the access registers are implicit registers too, 16 bits in a word of 32
	map(0x000, 0x00f).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_THREAD_ENABLE>), FUNC(powervr250_device::immediate_w<REG_THREAD_ENABLE>));
	map(0x020, 0x02f).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_THREAD_RECOVER>), FUNC(powervr250_device::immediate_w<REG_THREAD_RECOVER>));
	map(0x040, 0x043).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_RESET_VECTOR_LO>), FUNC(powervr250_device::immediate_w<REG_RESET_VECTOR_LO>));
	map(0x044, 0x047).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_RESET_VECTOR_HI>), FUNC(powervr250_device::immediate_w<REG_RESET_VECTOR_HI>));
	map(0x048, 0x04b).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_SOFT_INTERRUPT>), FUNC(powervr250_device::immediate_w<REG_SOFT_INTERRUPT>));
	map(0x04c, 0x04f).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_VIDEO_CLOCK_DIV>), FUNC(powervr250_device::immediate_w<REG_VIDEO_CLOCK_DIV>));
	map(0x050, 0x053).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_IIC_A>), FUNC(powervr250_device::immediate_w<REG_IIC_A>));
	map(0x054, 0x057).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_IIC_B>), FUNC(powervr250_device::immediate_w<REG_IIC_B>));
	map(0x060, 0x063).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_DAC_LINE_COMPARE>), FUNC(powervr250_device::immediate_w<REG_DAC_LINE_COMPARE>));
	map(0x064, 0x067).umask32(0x0000'ffff).r(FUNC(powervr250_device::immediate_r<REG_DAC_LINE_COUNT>));
	map(0x070, 0x073).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_INTERRUPT_STATUS>), FUNC(powervr250_device::immediate_w<REG_INTERRUPT_STATUS>));
	map(0x074, 0x077).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_INTERRUPT_MASK>), FUNC(powervr250_device::immediate_w<REG_INTERRUPT_MASK>));
	map(0x080, 0x083).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_DAC_PLL0>), FUNC(powervr250_device::immediate_w<REG_DAC_PLL0>));
	map(0x084, 0x087).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_DAC_PLL1>), FUNC(powervr250_device::immediate_w<REG_DAC_PLL1>));
	map(0x0a0, 0x0a3).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_VGA_PAGE>), FUNC(powervr250_device::immediate_w<REG_VGA_PAGE>));
	map(0x0a4, 0x0a7).umask32(0x0000'ffff).rw(FUNC(powervr250_device::immediate_r<REG_SOFTWARE_RESET>), FUNC(powervr250_device::immediate_w<REG_SOFTWARE_RESET>));
	map(0x0b0, 0x0bf).umask32(0x0000'ffff).w(FUNC(powervr250_device::immediate_w<REG_THREAD_KICKER>));

	// the write access register and the two read access registers
	map(0x0c0, 0x0c3).lr32(NAME([this] () -> u32 { return m_war; })).w(FUNC(powervr250_device::write_access_w));
	map(0x0c4, 0x0c7).lr32(NAME([this] () -> u32 { return m_rar[1]; }));
	map(0x0c8, 0x0cb).lr32(NAME([this] () -> u32 { return m_rar[0]; }));
}

template <u16 Reg>
u16 powervr250_device::immediate_r(offs_t offset)
{
	return implicit_r(Reg + offset);
}

template <u16 Reg>
void powervr250_device::immediate_w(offs_t offset, u16 data, u16 mem_mask)
{
	host_implicit_w(Reg + offset, data, mem_mask);
}

void powervr250_device::write_access_w(offs_t offset, u32 data, u32 mem_mask)
{
	// IA_RW, bits 31-30 of the write access register, is 0 to read and 1 to write the implicit register whose address is in
	// 25-16.  A write takes its data from 15-0; what is read turns up in the read access register that IA_ROUTE, 29-26, names
	COMBINE_DATA(&m_war);
	switch (BIT(m_war, 30, 2))
	{
	case 0:
		if (BIT(m_war, 26, 4) == 0x1)
			m_rar[0] = (m_war & 0xffff'0000) | implicit_r(BIT(m_war, 16, 10));
		else if (BIT(m_war, 26, 4) == 0x9)
			m_rar[1] = (m_war & 0xffff'0000) | implicit_r(BIT(m_war, 16, 10));
		else
			logerror("%s: write access register %08x, unknown route\n", machine().describe_context(), m_war);
		break;
	case 1:
		host_implicit_w(BIT(m_war, 16, 10), m_war & 0xffff, 0xffff);
		break;
	default:
		logerror("%s: write access register %08x, neither read nor write\n", machine().describe_context(), m_war);
		break;
	}
}

void powervr250_device::host_implicit_w(offs_t offset, u16 data, u16 mem_mask)
{
	// the primary core has a clock of its own, and reads and writes these registers too: a write by the host is made
	// when the core has got as far in time as the host has
	LOGMASKED(LOG_IMPLICIT, "%s: host writes implicit register %03x = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
	machine().scheduler().synchronize(
			timer_expired_delegate(FUNC(powervr250_device::host_implicit_sync), this),
			(ACCESSING_BITS_8_15 ? 0x0800'0000 : 0) | (ACCESSING_BITS_0_7 ? 0x0400'0000 : 0) | ((offset & 0x3ff) << 16) | data);
}

TIMER_CALLBACK_MEMBER(powervr250_device::host_implicit_sync)
{
	implicit_w(BIT(param, 16, 10), param & 0xffff, (BIT(param, 27) ? 0xff00 : 0) | (BIT(param, 26) ? 0x00ff : 0));
}

u32 powervr250_device::fb_r(offs_t offset)
{
	if (offset >= (m_vram_size / 4))
		return 0;
	if (m_page_in_use[offset >> (PAGE_SHIFT - 2)] && !machine().side_effects_disabled())
		m_renderer->finish();
	return m_vram[offset];
}

void powervr250_device::fb_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset < (m_vram_size / 4))
	{
		if (m_page_in_use[offset >> (PAGE_SHIFT - 2)])
			m_renderer->finish();
		COMBINE_DATA(&m_vram[offset]);
		m_page_written[offset >> (PAGE_SHIFT - 2)] = m_render_serial;
	}
}


//**************************************************************************
//  implicit registers
//**************************************************************************

void powervr250_device::update_irq()
{
	// a latched status bit reaches the host when neither its own mask bit nor the master one is set (Programming Guide, "Interrupts & Reset")
	const u16 mask = m_implicit[REG_INTERRUPT_MASK];
	const bool level = !(mask & IRQ_MASTER) && ((m_implicit[REG_INTERRUPT_STATUS] & ~mask & 0x7fff) != 0);
	if (level != m_irq_level)
	{
		m_irq_level = level;
		m_irq_cb(level ? 1 : 0);
	}
}

void powervr250_device::set_irq(u16 bits)
{
	// the master bit is not a latch, it is made from the others when the register is read
	m_implicit[REG_INTERRUPT_STATUS] |= bits & 0x7fff;
	update_irq();
}

void powervr250_device::map_frame_buffer()
{
	// the SDRAM answers the chip's own accesses at the addresses that have the frame buffer offset register in bits 31-26
	const u8 offset = m_implicit[REG_FB_OFFSET] & 0x3f;
	if (offset == m_fb_mapped)
		return;

	address_space &space = m_core->space(AS_PROGRAM);
	m_core_write_tap.remove();
	if (m_fb_mapped != 0xff)
		space.unmap_readwrite(u32(m_fb_mapped) << 26, (u32(m_fb_mapped) << 26) + m_vram_size - 1);
	space.install_ram(u32(offset) << 26, (u32(offset) << 26) + m_vram_size - 1, m_vram.get());
	m_core_write_tap = space.install_write_tap(u32(offset) << 26, (u32(offset) << 26) + m_vram_size - 1, "written",
			[this] (offs_t address, u32 &data, u32 mem_mask)
			{
				const u32 page = (address & (m_vram_size - 1)) >> PAGE_SHIFT;
				if (m_page_in_use[page])
					m_renderer->finish();
				m_page_written[page] = m_render_serial;
			});
	m_fb_mapped = offset;
}

void powervr250_device::settle(u32 offset, u32 bytes)
{
	// a render that is still being drawn and uses these pages has to be done before they change
	if (bytes)
	{
		const u32 pages = m_vram_size >> PAGE_SHIFT;
		const u32 last = std::min((offset + bytes - 1) >> PAGE_SHIFT, pages - 1);
		for (u32 page = std::min(offset >> PAGE_SHIFT, pages - 1); page <= last; page++)
		{
			if (m_page_in_use[page])
			{
				m_renderer->finish();
				return;
			}
		}
	}
}

void powervr250_device::written(u32 offset, u32 bytes)
{
	// what the renderer keeps of the textures in these pages has to be made again
	if (bytes)
	{
		const u32 pages = m_vram_size >> PAGE_SHIFT;
		const u32 last = std::min((offset + bytes - 1) >> PAGE_SHIFT, pages - 1);
		for (u32 page = std::min(offset >> PAGE_SHIFT, pages - 1); page <= last; page++)
			m_page_written[page] = m_render_serial;
	}
}

u32 *powervr250_device::memory(u32 address, u32 words) const
{
	// an address that the chip itself reads or writes at is in its SDRAM only if bits 31-26 equal the frame buffer offset
	const u32 offset = address & 0x03ff'fffc;
	if ((address >> 26) != (m_implicit[REG_FB_OFFSET] & 0x3f) || offset >= m_vram_size || words > ((m_vram_size - offset) >> 2))
		return nullptr;
	return &m_vram[offset >> 2];
}

u16 powervr250_device::implicit_r(offs_t offset)
{
	offset &= 0x3ff;
	switch (offset)
	{
	case REG_INTERRUPT_STATUS:
		return m_implicit[offset] | ((m_implicit[offset] & 0x7fff) ? IRQ_MASTER : 0);

	case REG_INDEX_DATA:
		{
			const u32 data = m_regs3d[BIT(m_implicit[REG_INDEX_ADDRESS], 2, 9)];
			return BIT(m_implicit[REG_INDEX_ADDRESS], 15) ? (data >> 16) : data;
		}

	case REG_DAC_LINE_COUNT:
		return m_dac_line & 0x0fff;

	default:
		return m_implicit[offset];
	}
}

void powervr250_device::implicit_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= 0x3ff;
	LOGMASKED(LOG_IMPLICIT, "%s: implicit register %03x = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);

	// a register that holds what is written keeps the half that a write leaves out
	const u16 before = m_implicit[offset];
	const u16 after = (before & ~mem_mask) | (data & mem_mask);
	data &= mem_mask;

	switch (offset)
	{
	case REG_THREAD_ENABLE: case REG_THREAD_ENABLE + 1: case REG_THREAD_ENABLE + 2: case REG_THREAD_ENABLE + 3:
		m_implicit[offset] = after & 1;
		m_core->set_thread_enable(offset - REG_THREAD_ENABLE, BIT(after, 0));
		break;

	case REG_RESET_VECTOR_LO:
	case REG_RESET_VECTOR_HI:
		m_implicit[offset] = after;
		m_core->set_reset_vector((u32(m_implicit[REG_RESET_VECTOR_HI]) << 16) | m_implicit[REG_RESET_VECTOR_LO]);
		break;

	case REG_SOFT_INTERRUPT:
		// bits 3-0 are the sources of bits 11-8 of the interrupt status, taken as the others are on their rising edge
		// (Programming Guide, "Primary Core Soft Interrupt")
		m_implicit[offset] = after;
		if (after & ~before & 0x000f)
			set_irq((after & ~before & 0x000f) << 8);
		break;

	case REG_THREAD_KICKER: case REG_THREAD_KICKER + 1: case REG_THREAD_KICKER + 2: case REG_THREAD_KICKER + 3:
		m_core->kick(offset - REG_THREAD_KICKER, data);
		break;

	case REG_INDEX_ADDRESS:
		// the first half of a value in two packets is lost when its second half has not come
		m_implicit[offset] = after;
		m_index_low_held = false;
		break;

	case REG_INDEX_DATA:
		m_implicit[offset] = after;
		index_data_w(after);
		break;

	case REG_DAC_PLL1:
		// a write here gives the PLL its setting: what DAC PLL 0 holds, with DACPLL_S1 and DACPLL_MB above it
		m_implicit[offset] = after;
		m_dac_pll = (BIT(after, 0, 2) << 16) | m_implicit[REG_DAC_PLL0];
		break;

	case REG_INTERRUPT_STATUS:
		// a bit is cleared by writing a 1 to it
		m_implicit[offset] &= ~data;
		update_irq();
		break;

	case REG_INTERRUPT_MASK:
		m_implicit[offset] = after;
		update_irq();
		break;

	case REG_SOFTWARE_RESET:
		// C1_RST, bit 2, holds the primary core in reset while it is 0.  What was clocked into the core PLL's registers has
		// no effect until a reset from this register (Programming Guide, "CORE PLL Control"): the core takes it as it is let go
		m_implicit[offset] = after;
		if (BIT(after, 2) && !BIT(before, 2))
			m_core->set_unscaled_clock(core_frequency());
		m_core->core_reset_w(BIT(after, 2));
		break;

	case REG_FB_OFFSET:
		m_implicit[offset] = after;
		m_core->set_fb_offset(after & 0x3f);
		map_frame_buffer();
		break;

	default:
		m_implicit[offset] = after;
		break;
	}
}

void powervr250_device::index_data_w(u16 data)
{
	// Index Address: 15 IDX_PKT, a value comes in two packets, the low half first; 14 IDX_INC, the address moves on to the
	// next register after each value; 10-0 IDX_ADDR, the address of a register, which are four apart (Programming Guide, "Indexed Registers")
	const u16 index = m_implicit[REG_INDEX_ADDRESS];
	const u16 address = index & 0x07fc;
	u32 &value = m_regs3d[address >> 2];
	if (!BIT(index, 15))
	{
		value = (value & 0xffff'0000) | data;
	}
	else if (!m_index_low_held)
	{
		m_index_low = data;
		m_index_low_held = true;
		return;
	}
	else
	{
		value = (u32(data) << 16) | m_index_low;
		m_index_low_held = false;
	}
	LOGMASKED(LOG_3D, "%s: 3D register %03x = %08x\n", machine().describe_context(), address, value);

	if (address == REG3D_START_RENDER)
	{
		// FIXME: how long the chip takes over a render is not known
		m_renderer->render();
		m_render_timer->adjust(attotime::from_msec(2));
	}

	if (BIT(index, 14))
		m_implicit[REG_INDEX_ADDRESS] = (index & 0xf800) | ((index + 4) & 0x07ff);
}

TIMER_CALLBACK_MEMBER(powervr250_device::render_done)
{
	set_irq(IRQ_PVR);
}


//**************************************************************************
//  tile accelerator
//**************************************************************************

/*
    The primary core hands the tile accelerator a list of strips and is
    told which tiles of 32 by 16 pixels each of them is in, from which it
    makes the object pointer lists of the tiles.  Its instructions, by
    bits 31-30 (PMX1c Tile Accelerator Specification, figure 2, and
    Programming Guide, "TA / Primary Core Interface"):
    0  with bit 29 clear 51-32 and 19-0 are the second and the first page
       of the control list, with it set the fourth and the third; with
       bit 63 set tiling starts instead, 11-0 the words of the list less
       one
    1  63-34 where the copies of the strips go, as a word address; 20-0
       the words there is room for, less one
    2  61-56 and 53-48 the first and the last tile across of the macro
       tile, 46-40 and 38-32 its first and last down; 29-24 and 22-16 the
       last tile across and down of the screen; 7-0 mode
    3  63-62 what to clip, 61-56, 53-48, 46-40 and 38-32 the rectangle of
       tiles to clip by, laid out as that of the macro tile

    The control list is made of object pointers as the 3D core has them,
    with the place of the strip in its page of memory in bits 9-0.  A
    pointer with bit 31 set is followed by the addresses of that page and
    of the one in which a strip that does not fit carries on, and by the
    three or five ISP/TSP words that the strips from this one on share.

    A strip that is in some tile is copied, those words first and then its
    vertices, to where instruction 1 said, one after another.  For each
    of its tiles a status word goes to the primary core: 62-32 the object
    pointer, with the triangles that are in the tile and with the place
    of the copy, 12-0 the number of the tile, counted across and then
    down the macro tile.  The last word has bit 31 for the end of the
    list, and with it bit 30 when the room for the copies ran out.
*/

void powervr250_device::ta_instruction_w(u64 data)
{
	LOGMASKED(LOG_TA, "TA instruction %016x\n", data);

	switch (BIT(data, 30, 2))
	{
	case 0:
		if (BIT(data, 29))
		{
			m_ta_list_page[2] = BIT(data, 0, 20);
			m_ta_list_page[3] = BIT(data, 32, 20);
		}
		else if (!BIT(data, 63))
		{
			m_ta_list_page[0] = BIT(data, 0, 20);
			m_ta_list_page[1] = BIT(data, 32, 20);
		}
		else
		{
			// the list is gone through as the primary core takes the status words; the known software sends 3ff, 7ff and fff
			// here for lists of one, two and four pages
			m_ta_list_words = BIT(data, 0, 12) + 1;
			m_ta_list_position = 0;
			m_ta_strip = ta_box();
			m_ta_tile_x = m_ta_strip.left;
			m_ta_tile_y = m_ta_strip.top;
			m_ta_tiling = true;
			if (m_ta_setup & (TA_AA_X | TA_AA_Y))
				logerror("TA: anti-aliasing is not emulated\n");
			if (m_ta_setup & TA_MTILE_OLAP)
				logerror("TA: macro tile overlap is not emulated\n");
			if (!(m_ta_setup & TA_COPY))
			{
				logerror("TA: tiling without a copy of the strips is not emulated\n");
				m_ta_tiling = false;
			}
			ta_next_status();
		}
		break;

	case 1:
		// what is copied goes on from here until this instruction comes again, however many lists are tiled
		// FIXME: figure 2 of the specification has this count too as one less than the words, the guide as the words
		m_ta_copy_dest = BIT(data, 34, 30) << 2;
		m_ta_copy_limit = BIT(data, 0, 21) + 1;
		m_ta_copy_words = 0;
		break;

	case 2:
		m_ta_setup = data;
		break;

	case 3:
		m_ta_clip = data;
		break;
	}
}

u64 powervr250_device::ta_status_r()
{
	const u64 status = m_ta_status;
	if (!machine().side_effects_disabled())
	{
		LOGMASKED(LOG_TA, "TA status %016x\n", status);
		ta_next_status();
	}
	return status;
}

void powervr250_device::ta_next_status()
{
	// with nothing to tile the list is at its end
	m_ta_status = TA_EOL;
	if (!m_ta_tiling)
		return;

	// the next tile of the strip in hand, or the first one of the next strip that is in any
	u32 triangles = ta_find_tile();
	while (!triangles)
	{
		if (!ta_next_strip())
		{
			m_ta_tiling = false;
			return;
		}
		triangles = ta_find_tile();
	}

	// Tilenum = (TileXPos - MacroTileMinX) + ((TileYPos - MacroTileMinY) * (MacroTileMaxX + 1 - MacroTileMinX)), specification 2.3
	const u32 left = BIT(m_ta_setup, 56, 6);
	const u32 across = BIT(m_ta_setup, 48, 6) + 1 - left;
	const u32 tile = (m_ta_tile_x - left) + (m_ta_tile_y - BIT(m_ta_setup, 40, 7)) * across;
	m_ta_status = (u64(m_ta_pointer | triangles) << 32) | (tile & 0x1fff);
	m_ta_tile_x++;
}

u32 powervr250_device::ta_find_tile()
{
	// TA_CLIP keeps the strips out of all tiles, out of those outside a rectangle or out of those inside it
	const u32 clip = BIT(m_ta_clip, 62, 2);
	const u32 clip_left = BIT(m_ta_clip, 56, 6);
	const u32 clip_right = BIT(m_ta_clip, 48, 6);
	const u32 clip_top = BIT(m_ta_clip, 40, 7);
	const u32 clip_bottom = BIT(m_ta_clip, 32, 7);

	// from the tile that was got to, across and then down those the strip in hand reaches, to one that has some of its triangles
	for ( ; m_ta_tile_y <= m_ta_strip.bottom; m_ta_tile_y++, m_ta_tile_x = m_ta_strip.left)
	{
		for ( ; m_ta_tile_x <= m_ta_strip.right; m_ta_tile_x++)
		{
			const bool inside = m_ta_tile_x >= clip_left && m_ta_tile_x <= clip_right && m_ta_tile_y >= clip_top && m_ta_tile_y <= clip_bottom;
			if (clip == 1 || (clip == 2 && !inside) || (clip == 3 && inside))
				continue;

			// the triangles as an object pointer has them, the first in bit 30
			u32 triangles = 0;
			for (u32 i = 0; i < 6; i++)
			{
				const ta_box &box = m_ta_triangle[i];
				if (m_ta_tile_x >= box.left && m_ta_tile_x <= box.right && m_ta_tile_y >= box.top && m_ta_tile_y <= box.bottom)
					triangles |= 0x4000'0000 >> i;
			}
			if (triangles)
				return triangles;
		}
	}
	return 0;
}

bool powervr250_device::ta_list_word(u32 &data)
{
	if (m_ta_list_position >= m_ta_list_words)
		return false;

	// the list is in up to four pages of 1024 words, wherever those are
	const u32 address = (m_ta_list_page[m_ta_list_position >> 10] << 12) | ((m_ta_list_position & 0x3ff) << 2);
	const u32 *const source = memory(address, 1);
	if (!source)
	{
		logerror("TA: control list at %08x is outside the frame buffer\n", address);
		return false;
	}
	data = *source;
	m_ta_list_position++;
	return true;
}

bool powervr250_device::ta_next_strip()
{
	// the tiles of the macro tile that are on the screen
	const s32 first_x = BIT(m_ta_setup, 56, 6);
	const s32 last_x = std::min(BIT(m_ta_setup, 48, 6), BIT(m_ta_setup, 24, 6));
	const s32 first_y = BIT(m_ta_setup, 40, 7);
	const s32 last_y = std::min(BIT(m_ta_setup, 32, 7), BIT(m_ta_setup, 16, 7));

	// the tile, of so many pixels, that a coordinate is in; what is far off counts as just off the largest screen
	const auto tile_of = [] (float coordinate, int size) { return s32(std::clamp(std::floor(coordinate / float(size)), -1.0f, 128.0f)); };

	u32 pointer;
	while (ta_list_word(pointer))
	{
		// with the shadow bit a strip has two sets of parameters, unless shadows are of the cheap kind
		// FIXME: the guide has TA_SHAD both as what enables cheap shadows and as 0 for enabled, 1 for disabled; the values are taken
		const bool two_volumes = BIT(pointer, 24) && (m_ta_setup & TA_SHAD);
		if (BIT(pointer, 31))
		{
			if (!ta_list_word(m_ta_vertex_page[0]) || !ta_list_word(m_ta_vertex_page[1]))
				return false;
			m_ta_header_words = two_volumes ? 5 : 3;
			for (u32 i = 0; i < m_ta_header_words; i++)
			{
				if (!ta_list_word(m_ta_header[i]))
					return false;
			}
		}

		// a strip without triangles is passed over, as its size is not known (specification, 2.2)
		const u32 mask = BIT(pointer, 25, 6);
		if (!mask)
			continue;

		// it has the vertices that its last triangle needs, each of X, Y and 1/W and the words that the skip field counts, once for
		// each set of parameters; when there is Z as well as 1/W that is one word more (Programming Guide, "3D Object Data")
		const u32 size = 3 + BIT(pointer, 21, 3) * (two_volumes ? 2 : 1) + ((m_ta_setup & TA_Z) ? 1 : 0);
		const u32 words = (8 - std::countr_zero(mask)) * size;

		// what does not fit in its page carries on at the start of the next
		const u32 start = BIT(pointer, 0, 10);
		const u32 here = std::min(words, 0x400 - start);
		const u32 *const source = memory((m_ta_vertex_page[0] & 0xffff'f000) | (start << 2), here);
		const u32 *const more = (here < words) ? memory(m_ta_vertex_page[1] & 0xffff'f000, words - here) : nullptr;
		if (!source || (here < words && !more))
		{
			logerror("TA: strip %08x in pages %08x and %08x is outside the frame buffer\n", pointer, m_ta_vertex_page[0], m_ta_vertex_page[1]);
			return false;
		}
		u32 vertices[TA_STRIP_WORDS];
		std::copy_n(source, here, vertices);
		if (here < words)
			std::copy_n(more, words - here, vertices + here);

		// a triangle is in the tiles that the rectangle around its corners touches, as far as the macro tile and the screen go;
		// one with a coordinate that is not a finite number is in none
		m_ta_strip = ta_box();
		for (u32 i = 0; i < 6; i++)
		{
			ta_box &box = m_ta_triangle[i];
			box = ta_box();
			if (!BIT(pointer, 30 - i))
				continue;

			const float x[3] = { u2f(vertices[i * size]), u2f(vertices[(i + 1) * size]), u2f(vertices[(i + 2) * size]) };
			const float y[3] = { u2f(vertices[i * size + 1]), u2f(vertices[(i + 1) * size + 1]), u2f(vertices[(i + 2) * size + 1]) };
			if (!std::isfinite(x[0]) || !std::isfinite(x[1]) || !std::isfinite(x[2]) || !std::isfinite(y[0]) || !std::isfinite(y[1]) || !std::isfinite(y[2]))
				continue;

			const s32 left = std::max(first_x, tile_of(std::min({ x[0], x[1], x[2] }), TILE_WIDTH));
			const s32 right = std::min(last_x, tile_of(std::max({ x[0], x[1], x[2] }), TILE_WIDTH));
			const s32 top = std::max(first_y, tile_of(std::min({ y[0], y[1], y[2] }), TILE_HEIGHT));
			const s32 bottom = std::min(last_y, tile_of(std::max({ y[0], y[1], y[2] }), TILE_HEIGHT));
			if (left > right || top > bottom)
				continue;

			box.left = left;
			box.right = right;
			box.top = top;
			box.bottom = bottom;
			if (m_ta_strip.left > m_ta_strip.right)
			{
				m_ta_strip = box;
			}
			else
			{
				m_ta_strip.left = std::min(m_ta_strip.left, box.left);
				m_ta_strip.right = std::max(m_ta_strip.right, box.right);
				m_ta_strip.top = std::min(m_ta_strip.top, box.top);
				m_ta_strip.bottom = std::max(m_ta_strip.bottom, box.bottom);
			}
		}

		// FIXME: that a strip which is in no tile is not copied is a guess, from the guide having the clipping done after the tiling
		// and just before the strip is written
		m_ta_tile_x = m_ta_strip.left;
		m_ta_tile_y = m_ta_strip.top;
		if (!ta_find_tile())
			continue;

		if ((m_ta_header_words + words) > (m_ta_copy_limit - m_ta_copy_words))
		{
			// nothing more is written and the primary core is sent no more strips
			m_ta_status |= TA_CO;
			return false;
		}
		const u32 address = m_ta_copy_dest + 4 * m_ta_copy_words;
		u32 *const dest = memory(address, m_ta_header_words + words);
		if (!dest)
		{
			logerror("TA: copy of a strip to %08x is outside the frame buffer\n", address);
			return false;
		}
		settle(address & 0x03ff'fffc, 4 * (m_ta_header_words + words));
		std::copy_n(m_ta_header, m_ta_header_words, dest);
		std::copy_n(vertices, words, dest + m_ta_header_words);
		written(address & 0x03ff'fffc, 4 * (m_ta_header_words + words));

		// the pointer to the first copy has bits 19-2 of the address of the copy, those after it count on from that (specification, figure 3)
		m_ta_pointer = (pointer & 0x01e0'0000) | ((BIT(m_ta_copy_dest, 2, 18) + m_ta_copy_words) & 0x001f'ffff);
		m_ta_copy_words += m_ta_header_words + words;
		return true;
	}
	return false;
}


//**************************************************************************
//  3D core
//**************************************************************************

/*
    A render goes through the region headers at Region Header Base, five
    words for each pass over a tile of 32 by 16 pixels:
    0  31 the last one, 30 keep the depths of the pass before, 29 the
       translucent objects are in order already, 28 write the tile out
       after the pass, 14-8 and 7-2 its place down and across in tiles
    1  opaque objects
    2  volumes that modify those
    3  translucent objects
    4  volumes that modify those
    A list is the address of its first object pointer, or bit 31 for none.
    Object pointers:
    31 = 0     30-25 the triangles of the strip that are in the tile, the
               first in bit 30; 24 the strip has two sets of parameters;
               23-21 the words of a vertex past its first three; 20-0
               where the strip is, in words from Object Base
    31-29 = 7  more pointers at the address in 27-2, or with bit 28 no more

    A strip is the ISP/TSP instruction word, the TSP instruction word and
    the texture control word, then its vertices: X, Y and 1/W (with Z
    before 1/W when the ISP sorts by Z), then as the first word asks U and
    V, the base color and the offset color.

    As the chip does, each line of a tile is first given to the ISP's part,
    which finds the opaque triangle that is seen at each pixel, and only
    that one is then textured and shaded by the TSP's part.  Translucent
    triangles follow: in their order when they come sorted, else the
    farthest that is not drawn yet at each pixel, time after time until
    none is left.
*/

namespace {

// a MIP-mapped texture is twiddled and has its levels one after another, the smallest first.  Where a level starts, for sides
// of 1, 2, 4 and so on up to 1024 texels: counted in texels, and for a VQ texture in the codes that follow its code book
constexpr u32 MIP_OFFSET[11] = { 0x3, 0x4, 0x8, 0x18, 0x58, 0x158, 0x558, 0x1558, 0x5558, 0x15558, 0x55558 };
constexpr u32 VQ_MIP_OFFSET[11] = { 0x0, 0x1, 0x2, 0x6, 0x16, 0x56, 0x156, 0x556, 0x1556, 0x5556, 0x15556 };

// the bits of a coordinate moved to every other bit
constexpr u32 spread(u32 value)
{
	value = (value | (value << 8)) & 0x00ff'00ff;
	value = (value | (value << 4)) & 0x0f0f'0f0f;
	value = (value | (value << 2)) & 0x3333'3333;
	return (value | (value << 1)) & 0x5555'5555;
}

// where a texel is in a twiddled texture: the bits of X and Y that both sides have alternate, with those of Y the lower, and
// what the longer side has more of goes above them (Programming Guide, appendix A, "Twiddled Textures")
constexpr u32 twiddle(u32 x, u32 y, u32 side)
{
	const u32 common = util::make_bitmask<u32>(side);
	return (((x | y) >> side) << (2 * side)) | (spread(x & common) << 1) | spread(y & common);
}

constexpr u32 mul8(u32 a, u32 b)
{
	const u32 product = a * b + 128;
	return (product + (product >> 8)) >> 8;
}

inline int to_byte(float value)
{
	// also takes what is not a number to zero
	return (value > 0.0f) ? ((value < 255.0f) ? int(value + 0.5f) : 255) : 0;
}

inline bool depth_test(u32 mode, float depth, float against)
{
	switch (mode)
	{
	case 0: return false;
	case 1: return depth < against;
	case 2: return depth == against;
	case 3: return depth <= against;
	case 4: return depth > against;
	case 5: return depth != against;
	case 6: return depth >= against;
	default: return true;
	}
}

// a texture coordinate past the sides of its texture, whose size is a power of two
inline s32 wrap(s32 coordinate, s32 size, u32 mode)
{
	switch (mode)
	{
	case 1: // flip: every other repeat is mirrored
		return ((coordinate & size) ? ~coordinate : coordinate) & (size - 1);
	case 2: // clamp
		return std::clamp(coordinate, 0, size - 1);
	default: // repeat; with wrap the coordinates were moved together when the triangle was set up
		return coordinate & (size - 1);
	}
}

// the integer that is not above a number that is in range
inline s32 floor_int(float value)
{
	const s32 result = s32(value);
	return (value < float(result)) ? (result - 1) : result;
}

// what a source and a destination pixel are multiplied by before they are added
inline u32 blend_factor(u32 instruction, u32 source, u32 dest, u32 other, u32 shift)
{
	switch (instruction)
	{
	case 0: return 0;
	case 1: return 255;
	case 2: return BIT(other, shift, 8);
	case 3: return 255 - BIT(other, shift, 8);
	case 4: return source >> 24;
	case 5: return 255 - (source >> 24);
	case 6: return dest >> 24;
	default: return 255 - (dest >> 24);
	}
}

inline u32 blend(u32 source, u32 dest, u32 source_instruction, u32 dest_instruction)
{
	u32 result = 0;
	if (source_instruction == 4 && dest_instruction == 5)
	{
		// by the alpha of the source
		const u32 alpha = source >> 24;
		for (u32 shift = 0; shift < 32; shift += 8)
			result |= std::min(mul8(BIT(source, shift, 8), alpha) + mul8(BIT(dest, shift, 8), 255 - alpha), 255U) << shift;
		return result;
	}

	for (u32 shift = 0; shift < 32; shift += 8)
	{
		const u32 sum = mul8(BIT(source, shift, 8), blend_factor(source_instruction, source, dest, dest, shift))
				+ mul8(BIT(dest, shift, 8), blend_factor(dest_instruction, source, dest, source, shift));
		result |= std::min(sum, 255U) << shift;
	}
	return result;
}

} // anonymous namespace


powervr250_device::renderer::renderer(powervr250_device &device)
	: poly_manager<float, tile_pass, 0>(device.machine())
	, m_device(device)
	, m_mask(device.m_vram_size - 1)
	, m_drawing(false)
	, m_kept_texels(0)
	, m_width(0)
	, m_height(0)
	, m_param_base(0)
	, m_pointer_base(0)
	, m_object_base(0)
	, m_zsort(false)
	, m_intensity_volumes(false)
	, m_isp_sample(0.0f)
	, m_tsp_sample(0.0f)
	, m_texel_sample(0.0f)
	, m_cull_value(0.0f)
	, m_background_depth(0.0f)
	, m_texture_stride(0)
	, m_fog_table_color(0)
	, m_fog_vertex_color(0)
	, m_fog_scale(0.0f)
	, m_clamp_max(0)
	, m_clamp_min(0)
	, m_target(~u32(0))
	, m_target_stride(0)
	, m_pack_mode(0)
	, m_alpha_threshold(0)
	, m_kval(0)
	, m_clip_left(0)
	, m_clip_right(0)
	, m_clip_top(0)
	, m_clip_bottom(0)
	, m_host_memory(false)
	, m_modifier_volumes(false)
{
	std::fill(std::begin(m_fog_table), std::end(m_fog_table), 0);
}


void powervr250_device::renderer::finish()
{
	if (!m_drawing)
		return;

	wait("render");
	m_drawing = false;
	for (u32 page : m_pages_used)
		m_device.m_page_in_use[page] = 0;
	m_pages_used.clear();

	// let go of what was not used when a lot is kept
	if (m_kept_texels > MAX_KEPT_TEXELS)
	{
		for (auto it = m_textures.begin(); it != m_textures.end(); )
		{
			if (it->second.used != m_device.m_render_serial)
			{
				m_kept_texels -= it->second.texels;
				it = m_textures.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

void powervr250_device::renderer::use(u32 offset, u32 bytes)
{
	if (!bytes)
		return;

	const u32 last = ((offset + bytes - 1) & m_mask) >> PAGE_SHIFT;
	for (u32 page = (offset & m_mask) >> PAGE_SHIFT; ; page = (page + 1) & (m_mask >> PAGE_SHIFT))
	{
		if (!m_device.m_page_in_use[page])
		{
			m_device.m_page_in_use[page] = 1;
			m_pages_used.push_back(page);
		}
		if (page == last)
			break;
	}
}

void powervr250_device::renderer::render()
{
	finish();
	// the registers hold while the chip renders
	const u32 base = m_device.m_implicit[REG_3D_ADDRESS_BASE];
	m_param_base = BIT(base, 12, 4) << 28;
	m_pointer_base = BIT(base, 8, 4) << 28;
	m_object_base = m_device.reg3d(REG3D_OBJECT_BASE) & 0x0ff0'0000;
	m_zsort = BIT(m_device.reg3d(REG3D_SORT_CONTROL), 1);
	m_intensity_volumes = BIT(m_device.reg3d(REG3D_SHADOW_SCALE), 8);

	const u32 sample = m_device.reg3d(REG3D_HALF_OFFSET);
	m_isp_sample = BIT(sample, 0) ? 0.5f : 0.0f;
	m_tsp_sample = BIT(sample, 1) ? 0.5f : 0.0f;
	m_texel_sample = BIT(sample, 2) ? 0.5f : 0.0f;
	m_cull_value = u2f(m_device.reg3d(REG3D_CULL_VALUE) & 0x7fff'ffff);
	m_background_depth = u2f(m_device.reg3d(REG3D_BACKGROUND_DEPTH) & 0xffff'fff0);

	// TEX_STRIDE is the length of a line, in units of 32 texels, for the textures in scan order that take it from here
	// (Programming Guide, appendix A, "Scan Order Textures")
	m_texture_stride = BIT(m_device.reg3d(REG3D_TEXTURE_CONTROL), 0, 6) * 32;

	// FOG_DENSITY is the number that 1/W is multiplied by on its way to the fog table: 15-8 a mantissa with 1.0 at bit 15,
	// 7-0 the power of two that goes with it, in two's complement (Programming Guide, "Fog Implementation")
	const u32 density = m_device.reg3d(REG3D_FOG_DENSITY);
	m_fog_scale = std::ldexp(float(BIT(density, 8, 8)), util::sext(density, 8) - 7);
	for (u32 i = 0; i < std::size(m_fog_table); i++)
		m_fog_table[i] = m_device.reg3d(REG3D_FOG_TABLE + 4 * i);
	m_fog_table_color = m_device.reg3d(REG3D_FOG_TABLE_COLOR) & 0x00ff'ffff;
	m_fog_vertex_color = m_device.reg3d(REG3D_FOG_VERTEX_COLOR) & 0x00ff'ffff;
	m_clamp_max = m_device.reg3d(REG3D_FOG_CLAMP_MAX);
	m_clamp_min = m_device.reg3d(REG3D_FOG_CLAMP_MIN);

	// where the pixels go: the address of the first is in units of eight bytes, the length of a line in units of sixteen
	const u32 control = m_device.reg3d(REG3D_WRITE_CONTROL1);
	m_pack_mode = BIT(control, 0, 3);
	m_kval = BIT(control, 8, 8);
	m_alpha_threshold = BIT(control, 16, 8);
	m_target_stride = BIT(m_device.reg3d(REG3D_WRITE_STRIDE), 0, 9) * 16;
	m_clip_left = BIT(m_device.reg3d(REG3D_WRITE_X_CLIP), 0, 11);
	m_clip_right = BIT(m_device.reg3d(REG3D_WRITE_X_CLIP), 16, 11);
	m_clip_top = BIT(m_device.reg3d(REG3D_WRITE_Y_CLIP), 0, 11);
	m_clip_bottom = BIT(m_device.reg3d(REG3D_WRITE_Y_CLIP), 16, 11);
	const u32 target = BIT(m_device.reg3d(REG3D_WRITE_CONTROL2), 0, 29) << 3;
	m_target = m_device.memory(target, 1) ? (target & 0x03ff'ffff) : ~u32(0);
	if (m_target == ~u32(0))
		m_device.logerror("3D pixels go to %08x, outside the frame buffer\n", target);

	// textures kept from earlier renders are good while their memory has not been written since; this render writes its pixels
	m_device.m_render_serial++;
	if (m_target != ~u32(0) && m_clip_bottom >= m_clip_top)
	{
		m_device.written((m_target + m_clip_top * m_target_stride) & m_mask, (m_clip_bottom - m_clip_top + 1) * m_target_stride);
		use(m_target + m_clip_top * m_target_stride, (m_clip_bottom - m_clip_top + 1) * m_target_stride);
	}
	if (m_device.reg3d(REG3D_SCALER_CONTROL) & 0x0001'0800)
		m_device.logerror("3D scaler is not emulated\n");

	m_passes.clear();
	m_tris.clear();
	m_surfaces.clear();
	m_strips.clear();
	m_lists.clear();
	m_strip_index.clear();
	m_host_memory = false;
	m_modifier_volumes = false;

	// the background is where its tag says, laid out as a strip is; the tag has the fields of an object pointer three bits up
	const u32 tag = m_device.reg3d(REG3D_BACKGROUND_TAG);
	s32 background = -1;
	const s32 behind = find_strip(BIT(tag, 3, 25));
	if (behind >= 0)
		background = setup_triangle(m_strips[behind], std::min<u32>(BIT(tag, 0, 3), 5), true);
	if (background < 0)
	{
		// nothing to read it from: black
		triangle plain = triangle();
		plain.depth.c = m_background_depth;
		plain.invw.c = 1.0f;
		plain.surface = m_surfaces.size();
		plain.whole = true;
		m_surfaces.emplace_back();
		background = m_tris.size();
		m_tris.push_back(plain);
	}

	// take the scene apart, tile after tile
	const bool presorted = BIT(m_device.reg3d(REG3D_SORT_CONTROL), 0);
	u32 address = (BIT(base, 4, 4) << 28) | (m_device.reg3d(REG3D_REGION_BASE) & 0x0fff'fffc);
	int width = 0;
	int height = 0;
	for (u32 regions = 0; regions < MAX_REGIONS; regions++, address += 20)
	{
		const u32 *const header = m_device.memory(address, 5);
		if (!header)
		{
			m_host_memory = true;
			break;
		}

		tile_pass tile;
		tile.x = BIT(header[0], 2, 6) * TILE_WIDTH;
		tile.y = BIT(header[0], 8, 7) * TILE_HEIGHT;
		tile.background = BIT(header[0], 30) ? -1 : background;
		tile.presorted = presorted || BIT(header[0], 29);
		tile.flush = BIT(header[0], 28);
		tile.opaque = m_lists.size();
		read_list(header[1]);
		tile.opaque_count = m_lists.size() - tile.opaque;
		tile.translucent = m_lists.size();
		read_list(header[3]);
		tile.translucent_count = m_lists.size() - tile.translucent;
		if (!BIT(header[2], 31) || !BIT(header[4], 31))
			m_modifier_volumes = true;
		m_passes.push_back(tile);

		width = std::max(width, tile.x + TILE_WIDTH);
		height = std::max(height, tile.y + TILE_HEIGHT);
		if (BIT(header[0], 31))
			break;
	}

	if (m_host_memory)
		m_device.logerror("3D scene has parts outside the frame buffer, those are not drawn\n");
	if (m_modifier_volumes)
		m_device.logerror("3D scene has modifier volumes, those are not emulated\n");
	LOGMASKED(LOG_3D, "3D render to %08x: %u passes over tiles, %u strips, %u triangles\n", target, m_passes.size(), m_strips.size(), m_tris.size());

	if (width > m_width || height > m_height)
	{
		m_width = std::max(m_width, width);
		m_height = std::max(m_height, height);
		m_accumulation[0].assign(m_width * m_height, 0);
		m_accumulation[1].assign(m_width * m_height, 0);
		m_depth.assign(m_width * m_height, 0.0f);
	}

	// nothing is added to the scene from here on, so the lines of the tiles can be handed out; they are drawn while the machine runs on
	const rectangle bounds(0, m_width - 1, 0, m_height - 1);
	for (const tile_pass &tile : m_passes)
	{
		extent_t extents[TILE_HEIGHT];
		for (extent_t &extent : extents)
		{
			extent.startx = tile.x;
			extent.stopx = tile.x + TILE_WIDTH;
			extent.userdata = nullptr;
		}
		object_data().next() = tile;
		render_extents<0>(bounds, render_delegate(&renderer::render_row, this), tile.y, TILE_HEIGHT, extents);
	}
	m_drawing = true;
}

void powervr250_device::renderer::read_list(u32 pointer)
{
	if (BIT(pointer, 31))
		return;

	u32 address = m_pointer_base | (pointer & 0x0fff'fffc);
	for (u32 pointers = 0; pointers < MAX_POINTERS; pointers++)
	{
		const u32 *const source = m_device.memory(address, 1);
		if (!source)
		{
			m_host_memory = true;
			return;
		}

		const u32 object = *source;
		if (BIT(object, 29, 3) == 7)
		{
			if (BIT(object, 28))
				return;
			address = m_pointer_base | (object & 0x0fff'fffc);
			continue;
		}
		address += 4;
		if (BIT(object, 31))
			continue;

		const s32 found = find_strip(object);
		if (found < 0)
			continue;
		for (u32 i = 0; i < 6; i++)
		{
			if (BIT(object, 30 - i))
			{
				// the strip is not added to in here
				strip &from = m_strips[found];
				if (from.triangle[i] == -2)
					from.triangle[i] = setup_triangle(from, i, false);
				if (from.triangle[i] >= 0)
					m_lists.push_back(from.triangle[i]);
			}
		}
	}
}

s32 powervr250_device::renderer::find_strip(u32 pointer)
{
	// two tiles with the same strip have the same pointer but for the triangles
	const auto known = m_strip_index.find(pointer & 0x01ff'ffff);
	if (known != m_strip_index.end())
		return known->second;

	const bool two_volumes = BIT(pointer, 24) && !m_intensity_volumes;
	const u32 words = two_volumes ? 5 : 3;
	const u32 address = m_param_base | ((m_object_base + (BIT(pointer, 0, 21) << 2)) & 0x0fff'fffc);
	const u32 *const control = m_device.memory(address, words);
	if (!control)
	{
		m_host_memory = true;
		return -1;
	}

	strip result;
	result.surface = m_surfaces.size();
	result.vertices = address + 4 * words;

	// the skip field leaves out the Z that goes before 1/W when the ISP sorts by Z, as the tile accelerator has it too
	result.size = 3 + (m_zsort ? 1 : 0) + BIT(pointer, 21, 3) * (two_volumes ? 2 : 1);
	std::fill(std::begin(result.triangle), std::end(result.triangle), -2);
	m_surfaces.push_back(make_surface(control[0], control[1], control[2]));
	m_strips.push_back(result);
	m_strip_index.emplace(pointer & 0x01ff'ffff, m_strips.size() - 1);
	return m_strips.size() - 1;
}

powervr250_device::renderer::surface powervr250_device::renderer::make_surface(u32 isp, u32 tsp, u32 tcw)
{
	/*
	    ISP/TSP instruction word: 31-29 depth compare, 28-27 culling, 26
	    depth write disable, 25 texture, 24 offset, 23 Gouraud, 22 16-bit
	    UV, 21 cache bypass, 20 how D is found for the choice of level

	    TSP instruction word: 31-29 and 28-26 what the source and the
	    destination of a blend are multiplied by, 25 and 24 the second
	    accumulation buffer is that source or destination, 23-22 fog,
	    21 clamp the color, 20 use alpha, 19 ignore the alpha of the
	    texture, 18-17 and 16-15 what U and V past the sides do, 14 pass
	    B, 13 bilinear, 12 supersample, 11-8 by how much D is adjusted,
	    7-6 how texture and colors combine, 5-3 and 2-0 U and V size

	    texture control word: 31 has all its levels, 30 VQ, 29-27 format,
	    26 not twiddled, 25 length of a line from the register, 24-23
	    color key, 22 filter in several passes, 21 palettised, 20-0
	    address in units of 32 bytes
	*/
	surface result;
	result.isp = isp;
	result.textured = BIT(isp, 25);
	result.offset = BIT(isp, 24);
	result.source_blend = BIT(tsp, 29, 3);
	result.dest_blend = BIT(tsp, 26, 3);
	result.source_select = BIT(tsp, 25);
	result.dest_select = BIT(tsp, 24);
	result.fog = BIT(tsp, 22, 2);
	result.color_clamp = BIT(tsp, 21);
	result.use_alpha = BIT(tsp, 20);
	result.ignore_texture_alpha = BIT(tsp, 19);
	result.mode_u = BIT(tsp, 17, 2);
	result.mode_v = BIT(tsp, 15, 2);
	result.bilinear = BIT(tsp, 13);
	result.lod_adjust = BIT(tsp, 8, 4);
	result.shading = BIT(tsp, 6, 2);
	if (!result.textured)
		return result;

	result.vq = BIT(tcw, 30);
	result.format = BIT(tcw, 27, 3);
	result.scan_order = BIT(tcw, 26);
	result.indexed = result.vq && BIT(tcw, 21);
	result.mipmapped = BIT(tcw, 31) && !result.scan_order;
	result.log_u = BIT(tsp, 3, 3) + 3;
	result.log_v = result.mipmapped ? result.log_u : (BIT(tsp, 0, 3) + 3);
	result.width = 1U << result.log_u;
	result.height = 1U << result.log_v;
	result.stride = (result.scan_order && BIT(tcw, 25)) ? m_texture_stride : result.width;

	// a texture is in the chip's memory unless its address is past the reference, what selects from a code book likewise
	const u32 address = BIT(tcw, 0, 21) << 5;
	const u32 texels = address + (m_device.reg3d(REG3D_TEXTURE_FB_OFFSET) << 3);
	const u32 indexes = address + 0x800 + (m_device.reg3d(REG3D_INDEX_FB_OFFSET) << 3);
	if ((address >> 16) > BIT(m_device.reg3d(REG3D_TEXTURE_REF), 0, 10) || !m_device.memory(texels, 1)
			|| (result.vq && ((address >> 16) > BIT(m_device.reg3d(REG3D_INDEX_REF), 0, 10) || !m_device.memory(indexes, 1))))
	{
		m_host_memory = true;
		result.textured = false;
		return result;
	}
	result.texels = texels & 0x03ff'ffff;
	result.indexes = indexes & 0x03ff'ffff;
	if (!result.scan_order)
	{
		result.decoded = decoded_texture(result);
	}
	else if (result.indexed)
	{
		// read from memory as it is drawn
		use(result.texels, 0x800);
		use(result.indexes, u32(result.stride) * result.height);
	}
	else
	{
		use(result.texels, u32(result.stride) * result.height * ((result.format == 5 || result.format == 7) ? 4 : 2));
	}
	return result;
}

const u32 *powervr250_device::renderer::decoded_texture(const surface &surf)
{
	const u32 serial = m_device.m_render_serial;
	texture &kept = m_textures[std::make_pair((u64(surf.indexes) << 32) | surf.texels,
			(u64(surf.log_v) << 16) | (u64(surf.log_u) << 8) | (surf.format << 4) | (surf.mipmapped ? 4 : 0) | (surf.indexed ? 2 : 0) | (surf.vq ? 1 : 0))];
	if (kept.used == serial)
		return kept.pixels.get();

	if (kept.pixels)
	{
		// still good when none of its memory has been written since it was made
		bool good = true;
		for (u32 i = 0; good && i < 2; i++)
		{
			if (kept.bytes[i])
			{
				const u32 last = ((kept.first[i] + kept.bytes[i] - 1) & m_mask) >> PAGE_SHIFT;
				for (u32 page = (kept.first[i] & m_mask) >> PAGE_SHIFT; good; page = (page + 1) & (m_mask >> PAGE_SHIFT))
				{
					good = m_device.m_page_written[page] < kept.made;
					if (page == last)
						break;
				}
			}
		}
		if (good)
		{
			kept.used = serial;
			return kept.pixels.get();
		}
		m_kept_texels -= kept.texels;
	}

	// the levels one after another from the largest, each line after line
	const u32 levels = surf.mipmapped ? (surf.log_u + 1) : 1;
	u32 texels = 0;
	for (u32 level = 0; level < levels; level++)
		texels += std::max(surf.width >> level, 1) * std::max(surf.height >> level, 1);
	if (!kept.pixels || kept.texels != texels)
		kept.pixels = std::make_unique<u32 []>(texels);
	u32 *dest = kept.pixels.get();
	for (u32 level = 0; level < levels; level++)
	{
		const s32 width = std::max(surf.width >> level, 1);
		const s32 height = std::max(surf.height >> level, 1);
		for (s32 y = 0; y < height; y++)
		{
			for (s32 x = 0; x < width; x++)
				*dest++ = texel(surf, level, x, y);
		}
	}

	// what it was made from: a code book or palette and what selects from it, or the texels themselves
	const u32 bytes = (surf.format == 5 || surf.format == 7) ? 4 : 2;
	if (surf.vq)
	{
		kept.first[0] = surf.texels;
		kept.bytes[0] = 0x800;
		kept.first[1] = surf.indexes;
		if (surf.indexed)
			kept.bytes[1] = (surf.mipmapped ? MIP_OFFSET[surf.log_u] : 0) + (u32(surf.width) * surf.height);
		else
			kept.bytes[1] = (surf.mipmapped ? VQ_MIP_OFFSET[surf.log_u] : 0) + std::max(u32(surf.width) * surf.height / 4, 1U);
	}
	else
	{
		kept.first[0] = surf.texels;
		kept.bytes[0] = ((surf.mipmapped ? MIP_OFFSET[surf.log_u] : 0) + (u32(surf.width) * surf.height)) * bytes;
		kept.bytes[1] = 0;
	}
	kept.texels = texels;
	kept.made = serial;
	kept.used = serial;
	m_kept_texels += texels;
	return kept.pixels.get();
}

s32 powervr250_device::renderer::setup_triangle(strip &from, u32 number, bool background)
{
	const surface &surf = m_surfaces[from.surface];
	const u32 *const data = m_device.memory(from.vertices + 4 * from.size * number, from.size * 2 + 8);
	if (!data)
	{
		m_host_memory = true;
		return -1;
	}

	vertex corner[3];
	bool finite = true;
	for (u32 i = 0; i < 3; i++)
	{
		const u32 *source = data + from.size * i;
		vertex &v = corner[i];
		v.x = u2f(*source++);
		v.y = u2f(*source++);
		v.depth = u2f(*source);
		if (m_zsort)
			source++;
		v.invw = u2f(*source++);
		v.u = 0.0f;
		v.v = 0.0f;
		if (BIT(surf.isp, 25))
		{
			if (BIT(surf.isp, 22))
			{
				// the top halves of the two numbers in one word
				v.u = u2f(*source & 0xffff'0000);
				v.v = u2f(*source++ << 16);
			}
			else
			{
				v.u = u2f(*source++);
				v.v = u2f(*source++);
			}
			if (!std::isfinite(v.u) || !std::isfinite(v.v))
				v.u = v.v = 0.0f;
		}
		v.base = *source++;
		v.offset = BIT(surf.isp, 24) ? *source : 0;
		if (!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.depth) || !std::isfinite(v.invw))
			finite = false;
	}

	// every other triangle of a strip has its first two corners the other way round, so that all turn the same way
	if (number & 1)
		std::swap(corner[0], corner[1]);

	const double x0 = corner[0].x;
	const double y0 = corner[0].y;
	const double ax = corner[1].x - x0;
	const double ay = corner[1].y - y0;
	const double bx = corner[2].x - x0;
	const double by = corner[2].y - y0;
	const double det = ax * by - bx * ay;
	bool flat = false;
	if (!background)
	{
		// a triangle can be left out for being small, or by the way it turns
		const u32 cull = BIT(surf.isp, 27, 2);
		if (!finite || det == 0.0 || (cull && std::abs(det) < m_cull_value) || (cull == 2 && det < 0.0) || (cull == 3 && det > 0.0))
			return -1;
	}
	else
	{
		// the background is the plane through the corners, or when there is no such plane what its last corner has
		flat = !finite || det == 0.0;
		for (vertex &v : corner)
		{
			if (!(v.invw > 0.0f) || !std::isfinite(v.invw))
				corner[0].invw = corner[1].invw = corner[2].invw = 1.0f;
		}
	}

	const double scale = flat ? 0.0 : (1.0 / det);
	const auto through = [x0, y0, ax, ay, bx, by, scale, flat] (double p0, double p1, double p2)
	{
		plane result;
		if (flat)
		{
			result.c = float(p2);
			return result;
		}
		const double dx = ((p1 - p0) * by - (p2 - p0) * ay) * scale;
		const double dy = ((p2 - p0) * ax - (p1 - p0) * bx) * scale;
		result.dx = float(dx);
		result.dy = float(dy);
		result.c = float(p0 - dx * x0 - dy * y0);
		return result;
	};

	triangle tri;
	tri.surface = from.surface;
	tri.compare = BIT(surf.isp, 29, 3);
	tri.depth_write = !BIT(surf.isp, 26);
	tri.whole = background;
	tri.invw = through(corner[0].invw, corner[1].invw, corner[2].invw);
	if (background)
	{
		tri.depth = plane();
		tri.depth.c = m_background_depth;
	}
	else
	{
		tri.depth = through(corner[0].depth, corner[1].depth, corner[2].depth);
	}

	// with wrap, U and V go the shorter way round between two corners
	for (u32 i = 1; i < 3; i++)
	{
		if (surf.mode_u == 3)
			corner[i].u -= std::round(corner[i].u - corner[0].u);
		if (surf.mode_v == 3)
			corner[i].v -= std::round(corner[i].v - corner[0].v);
	}
	tri.u = through(double(corner[0].u) * corner[0].invw, double(corner[1].u) * corner[1].invw, double(corner[2].u) * corner[2].invw);
	tri.v = through(double(corner[0].v) * corner[0].invw, double(corner[1].v) * corner[1].invw, double(corner[2].v) * corner[2].invw);

	// without Gouraud shading the colors are those of the last corner
	const bool gouraud = BIT(surf.isp, 23);
	for (u32 channel = 0; channel < 4; channel++)
	{
		double base[3], offset[3];
		for (u32 i = 0; i < 3; i++)
		{
			base[i] = double(BIT(corner[gouraud ? i : 2].base, 8 * channel, 8)) * corner[i].invw;
			offset[i] = double(BIT(corner[gouraud ? i : 2].offset, 8 * channel, 8)) * corner[i].invw;
		}
		tri.base[channel] = through(base[0], base[1], base[2]);
		tri.offset[channel] = through(offset[0], offset[1], offset[2]);
	}

	// FIXME: how the chip gets D, the number of texels to a pixel, from the gradients is not documented, the longer one is taken
	tri.lod = 0.0f;
	if (surf.textured && surf.mipmapped)
	{
		// the gradients are those at the origin of the screen or at the first corner, but for the division by 1/W squared
		const double x = BIT(surf.isp, 20) ? x0 : 0.0;
		const double y = BIT(surf.isp, 20) ? y0 : 0.0;
		const double r = tri.invw.at(x, y);
		const double cu = tri.u.at(x, y);
		const double cv = tri.v.at(x, y);
		const double dudx = (tri.u.dx * r - tri.invw.dx * cu) * surf.width;
		const double dvdx = (tri.v.dx * r - tri.invw.dx * cv) * surf.height;
		const double dudy = (tri.u.dy * r - tri.invw.dy * cu) * surf.width;
		const double dvdy = (tri.v.dy * r - tri.invw.dy * cv) * surf.height;
		tri.lod = float(std::sqrt(std::max(dudx * dudx + dvdx * dvdx, dudy * dudy + dvdy * dvdy)) * surf.lod_adjust / 4.0);
	}

	// the corners from the top down
	u32 order[3] = { 0, 1, 2 };
	if (corner[order[0]].y > corner[order[1]].y)
		std::swap(order[0], order[1]);
	if (corner[order[1]].y > corner[order[2]].y)
		std::swap(order[1], order[2]);
	if (corner[order[0]].y > corner[order[1]].y)
		std::swap(order[0], order[1]);
	for (u32 i = 0; i < 3; i++)
	{
		tri.x[i] = finite ? corner[order[i]].x : 0.0f;
		tri.y[i] = finite ? corner[order[i]].y : 0.0f;
	}
	tri.slope_long = (tri.y[2] > tri.y[0]) ? ((tri.x[2] - tri.x[0]) / (tri.y[2] - tri.y[0])) : 0.0f;
	tri.slope_upper = (tri.y[1] > tri.y[0]) ? ((tri.x[1] - tri.x[0]) / (tri.y[1] - tri.y[0])) : 0.0f;
	tri.slope_lower = (tri.y[2] > tri.y[1]) ? ((tri.x[2] - tri.x[1]) / (tri.y[2] - tri.y[1])) : 0.0f;

	m_tris.push_back(tri);
	return m_tris.size() - 1;
}

bool powervr250_device::renderer::span(const triangle &tri, float y, int left, int right, int &first, int &end) const
{
	if (tri.whole)
	{
		first = left;
		end = right;
		return true;
	}
	if (!(y >= tri.y[0] && y < tri.y[2]))
		return false;

	// a pixel is in when the place it is sampled at is, the left and top edges included and the others not
	const float long_edge = tri.x[0] + (y - tri.y[0]) * tri.slope_long;
	const float short_edge = (y < tri.y[1]) ? (tri.x[0] + (y - tri.y[0]) * tri.slope_upper) : (tri.x[1] + (y - tri.y[1]) * tri.slope_lower);
	const float from = std::max(std::min(long_edge, short_edge) - m_isp_sample, float(left));
	const float to = std::min(std::max(long_edge, short_edge) - m_isp_sample, float(right));
	if (!(from < to))
		return false;

	first = int(std::ceil(from));
	end = int(std::ceil(to));
	return first < end;
}

void powervr250_device::renderer::render_row(s32 scanline, const extent_t &extent, const tile_pass &tile, int threadid)
{
	const int left = extent.startx;
	const int right = extent.stopx;
	const int count = right - left;
	if (count <= 0 || count > TILE_WIDTH)
		return;

	// the ISP: of the opaque triangles the one that is seen at each pixel, over the background or what an earlier pass left
	float depth[TILE_WIDTH];
	s32 tag[TILE_WIDTH];
	float *const kept = &m_depth[scanline * m_width + left];
	if (tile.background >= 0)
	{
		std::fill_n(depth, count, m_background_depth);
		std::fill_n(tag, count, tile.background);
	}
	else
	{
		std::copy_n(kept, count, depth);
		std::fill_n(tag, count, -1);
	}

	const float y = scanline + m_isp_sample;
	for (u32 i = 0; i < tile.opaque_count; i++)
	{
		const s32 number = m_lists[tile.opaque + i];
		const triangle &tri = m_tris[number];
		int first, end;
		if (!span(tri, y, left, right, first, end))
			continue;

		float z = tri.depth.at(first + m_isp_sample, y);
		for (int x = first - left; x < end - left; x++, z += tri.depth.dx)
		{
			if (depth_test(tri.compare, z, depth[x]))
			{
				if (tri.depth_write)
					depth[x] = z;
				tag[x] = number;
			}
		}
	}
	shade_tags(scanline, left, count, tag);

	if (tile.translucent_count && tile.presorted)
	{
		// in the order given, each against the depths as they then are
		for (u32 i = 0; i < tile.translucent_count; i++)
		{
			const s32 number = m_lists[tile.translucent + i];
			const triangle &tri = m_tris[number];
			int first, end;
			if (!span(tri, y, left, right, first, end))
				continue;

			std::fill_n(tag, count, -1);
			float z = tri.depth.at(first + m_isp_sample, y);
			for (int x = first - left; x < end - left; x++, z += tri.depth.dx)
			{
				if (depth_test(tri.compare, z, depth[x]))
				{
					if (tri.depth_write)
						depth[x] = z;
					tag[x] = number;
				}
			}
			shade_tags(scanline, left, count, tag);
		}
	}
	else if (tile.translucent_count)
	{
		// from the back: at each pixel the farthest that is in front of the opaque one and not drawn yet, those at one depth in their order
		float drawn[TILE_WIDTH];
		s32 drawn_at[TILE_WIDTH];
		std::fill_n(drawn, count, -std::numeric_limits<float>::infinity());
		std::fill_n(drawn_at, count, -1);
		for (u32 layer = 0; layer < tile.translucent_count; layer++)
		{
			float farthest[TILE_WIDTH];
			s32 farthest_at[TILE_WIDTH];
			std::fill_n(farthest_at, count, -1);
			bool found = false;
			for (u32 i = 0; i < tile.translucent_count; i++)
			{
				const triangle &tri = m_tris[m_lists[tile.translucent + i]];
				int first, end;
				if (!span(tri, y, left, right, first, end))
					continue;

				float z = tri.depth.at(first + m_isp_sample, y);
				for (int x = first - left; x < end - left; x++, z += tri.depth.dx)
				{
					if (!depth_test(tri.compare, z, depth[x]))
						continue;
					if (!(z > drawn[x] || (z == drawn[x] && s32(i) > drawn_at[x])))
						continue;
					if (farthest_at[x] < 0 || z < farthest[x])
					{
						farthest[x] = z;
						farthest_at[x] = i;
						found = true;
					}
				}
			}
			if (!found)
				break;

			for (int x = 0; x < count; x++)
			{
				if (farthest_at[x] >= 0)
				{
					drawn[x] = farthest[x];
					drawn_at[x] = farthest_at[x];
					tag[x] = m_lists[tile.translucent + farthest_at[x]];
				}
				else
				{
					drawn[x] = std::numeric_limits<float>::infinity();
					tag[x] = -1;
				}
			}
			shade_tags(scanline, left, count, tag);
		}
	}

	std::copy_n(depth, count, kept);
	if (tile.flush)
		write_row(scanline, left, count);
}

void powervr250_device::renderer::shade_tags(s32 scanline, int left, int count, const s32 *tag)
{
	for (int first = 0; first < count; )
	{
		int end = first + 1;
		while (end < count && tag[end] == tag[first])
			end++;
		if (tag[first] >= 0)
			shade(m_tris[tag[first]], scanline, left + first, left + end);
		first = end;
	}
}

void powervr250_device::renderer::shade(const triangle &tri, s32 scanline, int first, int end)
{
	// the TSP: texture and colors combined, fogged, and blended into an accumulation buffer
	const surface &surf = m_surfaces[tri.surface];
	const u32 *const source = surf.source_select ? &m_accumulation[1][scanline * m_width] : nullptr;
	u32 *const dest = &m_accumulation[surf.dest_select ? 1 : 0][scanline * m_width];
	const float y = scanline + m_tsp_sample;

	for (int x = first; x < end; x++)
	{
		const float px = x + m_tsp_sample;
		const float invw = tri.invw.at(px, y);
		const float w = 1.0f / invw;

		int color[4];
		int offset[4] = { 0, 0, 0, 0 };
		for (u32 channel = 0; channel < 4; channel++)
			color[channel] = to_byte(tri.base[channel].at(px, y) * w);
		if (!surf.use_alpha)
			color[3] = 255;
		if (surf.offset)
		{
			for (u32 channel = 0; channel < 4; channel++)
				offset[channel] = to_byte(tri.offset[channel].at(px, y) * w);
		}

		int pixel[4];
		if (surf.textured)
		{
			u32 level = 0;
			if (surf.mipmapped)
			{
				// a level for each doubling of the texels to a pixel
				const float d = tri.lod * w * w;
				if (d >= 2.0f)
					level = (d < 1024.0f) ? std::min<u32>(BIT(f2u(d), 23, 8) - 127, surf.log_u) : surf.log_u;
			}

			const u32 texture = sample(surf, tri.u.at(px, y) * w, tri.v.at(px, y) * w, level);
			const int texture_alpha = surf.ignore_texture_alpha ? 255 : (texture >> 24);
			switch (surf.shading)
			{
			case 0: // decal
				for (u32 channel = 0; channel < 3; channel++)
					pixel[channel] = BIT(texture, 8 * channel, 8) + offset[channel];
				pixel[3] = texture_alpha;
				break;
			case 1: // modulate
				for (u32 channel = 0; channel < 3; channel++)
					pixel[channel] = mul8(color[channel], BIT(texture, 8 * channel, 8)) + offset[channel];
				pixel[3] = texture_alpha;
				break;
			case 2: // decal alpha
				for (u32 channel = 0; channel < 3; channel++)
					pixel[channel] = mul8(BIT(texture, 8 * channel, 8), texture_alpha) + mul8(color[channel], 255 - texture_alpha) + offset[channel];
				pixel[3] = color[3];
				break;
			default: // modulate alpha
				for (u32 channel = 0; channel < 3; channel++)
					pixel[channel] = mul8(color[channel], BIT(texture, 8 * channel, 8)) + offset[channel];
				pixel[3] = mul8(color[3], texture_alpha);
				break;
			}
		}
		else
		{
			for (u32 channel = 0; channel < 3; channel++)
				pixel[channel] = color[channel] + offset[channel];
			pixel[3] = color[3];
		}
		for (u32 channel = 0; channel < 4; channel++)
		{
			pixel[channel] = std::min(pixel[channel], 255);
			if (surf.color_clamp)
				pixel[channel] = std::clamp<int>(pixel[channel], BIT(m_clamp_min, 8 * channel, 8), std::max(BIT(m_clamp_min, 8 * channel, 8), BIT(m_clamp_max, 8 * channel, 8)));
		}

		switch (surf.fog)
		{
		case 0: // from the table, by depth
			{
				const u32 density = fog_density(invw);
				for (u32 channel = 0; channel < 3; channel++)
					pixel[channel] = mul8(pixel[channel], 255 - density) + mul8(BIT(m_fog_table_color, 8 * channel, 8), density);
			}
			break;
		case 1: // from the alpha of the offset color
			if (surf.offset)
			{
				for (u32 channel = 0; channel < 3; channel++)
					pixel[channel] = mul8(pixel[channel], 255 - offset[3]) + mul8(BIT(m_fog_vertex_color, 8 * channel, 8), offset[3]);
			}
			break;
		case 2: // none
			break;
		case 3: // from the table, as a pixel of the fog's color to blend
			for (u32 channel = 0; channel < 3; channel++)
				pixel[channel] = BIT(m_fog_table_color, 8 * channel, 8);
			pixel[3] = fog_density(invw);
			break;
		}

		const u32 result = source ? source[x] : ((u32(pixel[3]) << 24) | (u32(pixel[2]) << 16) | (u32(pixel[1]) << 8) | u32(pixel[0]));
		dest[x] = (surf.source_blend == 1 && surf.dest_blend == 0) ? result : blend(result, dest[x], surf.source_blend, surf.dest_blend);
	}
}

u32 powervr250_device::renderer::fog_density(float invw) const
{
	// the scaled 1/W has entry 0 of the table at 1.0 and sixteen entries to each doubling from there, as far as 256.0.  What is
	// not within that, or not a number, is taken as the end it is past (Programming Guide, "Mapping a depth to a Fog Table entry")
	const float scaled = invw * m_fog_scale;
	if (!(scaled > 1.0f))
		return m_fog_table[0] >> 8;

	// the bits of the number less those of 1.0: 25-23 count the doublings and 22-0 are the fraction, so that 25-19 are the
	// number of the entry.  An entry has the fog at its own place in its upper byte and the fog at the next one's in its lower
	const u32 place = (scaled < 256.0f) ? (f2u(scaled) - f2u(1.0f)) : 0x03ff'ffff;
	const u32 entry = m_fog_table[BIT(place, 19, 7)];

	// FIXME: how many of the fraction's other bits the chip takes to go from the one to the other is not documented, eight here
	const u32 weight = BIT(place, 11, 8);
	return (BIT(entry, 8, 8) * (256 - weight) + BIT(entry, 0, 8) * weight) >> 8;
}

void powervr250_device::renderer::write_row(s32 scanline, int left, int count)
{
	if (m_target == ~u32(0) || scanline < m_clip_top || scanline > m_clip_bottom)
		return;

	// TODO: dither, the pattern is not documented
	u32 *const memory = m_device.m_vram.get();
	const auto put = [this, memory] (u32 offset, u32 value, u32 bits)
	{
		u32 &word = memory[(offset & m_mask) >> 2];
		const u32 shift = 8 * (offset & 3);
		word = (word & ~(util::make_bitmask<u32>(bits) << shift)) | (value << shift);
	};

	const u32 *const source = &m_accumulation[0][scanline * m_width];
	const u32 line = m_target + scanline * m_target_stride;
	for (int x = std::max(left, m_clip_left); x < left + count && x <= m_clip_right; x++)
	{
		const u32 pixel = source[x];
		const u32 a = BIT(pixel, 24, 8);
		const u32 r = BIT(pixel, 16, 8);
		const u32 g = BIT(pixel, 8, 8);
		const u32 b = BIT(pixel, 0, 8);
		switch (m_pack_mode)
		{
		case 0: // 555
			put(line + x * 2, ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3), 16);
			break;
		case 1: // 565
			put(line + x * 2, ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3), 16);
			break;
		case 2: // 4444
			put(line + x * 2, ((a >> 4) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4), 16);
			break;
		case 3: // 1555
			put(line + x * 2, ((a > m_alpha_threshold) ? 0x8000 : 0) | ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3), 16);
			break;
		case 4: // 888 in three bytes
			put(line + x * 3, b, 8);
			put(line + x * 3 + 1, g, 8);
			put(line + x * 3 + 2, r, 8);
			break;
		case 5: // 888 in four bytes
			memory[((line + x * 4) & m_mask) >> 2] = pixel & 0x00ff'ffff;
			break;
		case 6: // 8888, with the alpha from the register
			memory[((line + x * 4) & m_mask) >> 2] = (u32(m_kval) << 24) | (pixel & 0x00ff'ffff);
			break;
		default:
			break;
		}
	}
}

u32 powervr250_device::renderer::sample(const surface &surf, float u, float v, u32 level) const
{
	// keep what becomes an integer in range, and what is not a number out
	const auto bound = [] (float value) { return (value > -32768.0f) ? ((value < 32768.0f) ? value : 32768.0f) : -32768.0f; };

	const s32 width = std::max(surf.width >> level, 1);
	const s32 height = std::max(surf.height >> level, 1);
	float s = bound(u) * width;
	float t = bound(v) * height;

	// from the pixels kept of it, where the levels follow each other from the largest, or from memory
	const u32 *const decoded = surf.decoded ? (surf.decoded + (u32(surf.width) * surf.width - u32(width) * width) * 4 / 3) : nullptr;
	const auto fetch = [this, &surf, level, width, decoded] (s32 x, s32 y) { return decoded ? decoded[y * width + x] : texel(surf, level, x, y); };
	if (!surf.bilinear)
		return fetch(wrap(floor_int(s), width, surf.mode_u), wrap(floor_int(t), height, surf.mode_v));

	// the four texels around, by how near each is
	s -= m_texel_sample;
	t -= m_texel_sample;
	const s32 across = floor_int(s);
	const s32 down = floor_int(t);
	const s32 x0 = wrap(across, width, surf.mode_u);
	const s32 x1 = wrap(across + 1, width, surf.mode_u);
	const s32 y0 = wrap(down, height, surf.mode_v);
	const s32 y1 = wrap(down + 1, height, surf.mode_v);
	const u32 right = u32((s - float(across)) * 256.0f);
	const u32 below = u32((t - float(down)) * 256.0f);
	const u32 texels[4] = { fetch(x0, y0), fetch(x1, y0), fetch(x0, y1), fetch(x1, y1) };

	// two channels at a time, each with 32 bits of room: blue and red, then green and alpha
	const auto mix = [&texels, right, below] (u32 shift)
	{
		const auto pair = [shift] (u32 value) { return u64(BIT(value, shift, 8)) | (u64(BIT(value, shift + 16, 8)) << 32); };
		const u64 upper = pair(texels[0]) * (256 - right) + pair(texels[1]) * right;
		const u64 lower = pair(texels[2]) * (256 - right) + pair(texels[3]) * right;
		const u64 both = ((upper * (256 - below) + lower * below) >> 16) & 0x0000'00ff'0000'00ff;
		return u32(both) | u32(both >> 16);
	};
	return mix(0) | (mix(8) << 8);
}

u32 powervr250_device::renderer::texel(const surface &surf, u32 level, s32 x, s32 y) const
{
	const u32 bytes = (surf.format == 5 || surf.format == 7) ? 4 : 2;
	if (surf.scan_order)
	{
		// line after line; colors are widened with zeroes
		if (surf.indexed)
			return decode(surf.format, surf.texels + byte(surf.indexes + y * surf.stride + x) * 8, false);
		return decode(surf.format, surf.texels + (y * surf.stride + x) * bytes, false);
	}

	const u32 log_u = (surf.log_u > level) ? (surf.log_u - level) : 0;
	const u32 log_v = (surf.log_v > level) ? (surf.log_v - level) : 0;
	const u32 side = std::min(log_u, log_v);
	if (surf.indexed)
	{
		// palettised: one byte to a texel, the number of a palette entry.  The palette is stored the way a VQ code book is,
		// with the color in the first of the texels that an entry has room for
		const u32 index = surf.indexes + (surf.mipmapped ? MIP_OFFSET[log_u] : 0) + twiddle(x, y, side);
		return decode(surf.format, surf.texels + byte(index) * 8, true);
	}
	if (surf.vq)
	{
		// VQ compressed: every square of four texels is one byte, the number of the code book entry that holds the four
		const u32 index = surf.indexes + (surf.mipmapped ? VQ_MIP_OFFSET[log_u] : 0) + twiddle(x >> 1, y >> 1, side ? (side - 1) : 0);
		return decode(surf.format, surf.texels + byte(index) * 8 + twiddle(x & 1, y & 1, 1) * 2, true);
	}
	return decode(surf.format, surf.texels + ((surf.mipmapped ? MIP_OFFSET[log_u] : 0) + twiddle(x, y, side)) * bytes, true);
}

u32 powervr250_device::renderer::decode(u32 format, u32 offset, bool replicate) const
{
	// to eight bits: with the top bits again at the bottom, or with zeroes there
	const auto widen = [replicate] (u32 value, u32 bits) { return (value << (8 - bits)) | (replicate ? (value >> (2 * bits - 8)) : 0); };

	switch (format)
	{
	case 0: // 1555
		{
			const u16 pixel = word(offset);
			return (BIT(pixel, 15) ? 0xff00'0000 : 0) | (widen(BIT(pixel, 10, 5), 5) << 16) | (widen(BIT(pixel, 5, 5), 5) << 8) | widen(BIT(pixel, 0, 5), 5);
		}
	case 1: // 565
		{
			const u16 pixel = word(offset);
			return 0xff00'0000 | (widen(BIT(pixel, 11, 5), 5) << 16) | (widen(BIT(pixel, 5, 6), 6) << 8) | widen(BIT(pixel, 0, 5), 5);
		}
	case 2: // 4444
		{
			const u16 pixel = word(offset);
			return (widen(BIT(pixel, 12, 4), 4) << 24) | (widen(BIT(pixel, 8, 4), 4) << 16) | (widen(BIT(pixel, 4, 4), 4) << 8) | widen(BIT(pixel, 0, 4), 4);
		}
	case 5: // 8888
		return dword(offset);
	case 7: // 888
		return 0xff00'0000 | dword(offset);
	default: // TODO: YUV422 and bump maps
		return 0xff80'8080;
	}
}


//**************************************************************************
//  DAC feed
//**************************************************************************

u64 powervr250_device::vram_qword(u32 address) const
{
	const u32 index = (address & (m_vram_size - 1)) >> 2;
	return (u64(m_vram[(index + 1) & ((m_vram_size / 4) - 1)]) << 32) | m_vram[index];
}

int powervr250_device::dac_busy_r()
{
	return m_dac_busy ? 1 : 0;
}

void powervr250_device::dac_feed_w(u64 data)
{
	if (BIT(data, 62, 2) == 1)
	{
		// load register: 1 cursor, 2 page of the display list words, 3 frame buffer stride and base
		LOGMASKED(LOG_DAC, "DAC feed register %u = %08x\n", BIT(data, 32, 2), u32(data));
		m_dac_reg[BIT(data, 32, 2)] = u32(data);
		return;
	}
	if (BIT(data, 62, 2) != 0)
	{
		logerror("DAC feed instruction %016x is not known\n", data);
		return;
	}

	// span: so many lines of a width in 64-bit words, shown as its display list word says
	const u64 dlw = vram_qword((m_dac_reg[2] & 0xffff'ff00) + 8 * BIT(data, 55, 5));
	const u32 format = BIT(dlw, 40, 3);
	const bool hsync = BIT(dlw, 39);
	const u32 lines = BIT(data, 42, 11) + 1;
	const u32 words = BIT(data, 32, 10);

	// a line runs from byte DAC_STARTPIX of its first word to the pixel that starts at byte DAC_ENDPIX of its last word
	// (Programming Guide, "DAC Display List Word"); the bytes of a pixel are those of its DAC_PIXFMT
	static constexpr u8 PIXEL_BYTES[8] = { 1, 2, 2, 3, 4, 2, 3, 2 };
	const u32 first = BIT(dlw, 20, 3);
	u32 last = 8 * words + BIT(dlw, 24, 3);

	// a pixel of three bytes can start in one word and end in the next, which is then the last: DAC_ENDPIX is in the word before it
	if (PIXEL_BYTES[format] == 3 && BIT(dlw, 24, 3) >= 6 && words)
		last -= 8;
	const u32 pixels = (last >= first) ? ((last - first) / PIXEL_BYTES[format] + 1) : 1;
	if (BIT(dlw, 37))
		logerror("DAC feed span %016x asks for the scaler, which is not emulated\n", data);

	// a span with DAC_HS has the horizontal sync before its pixels
	// FIXME: the 3 is not from the guide, the front porch, the back porch and the sync each have to take a clock more than
	// their registers say for the frame rate of the known games to come out right
	u32 clocks = pixels;
	if (hsync)
		clocks += BIT(m_implicit[REG_DAC_HS_FRONT], 0, 11) + BIT(m_implicit[REG_DAC_HS_BACK], 0, 11) + BIT(m_implicit[REG_DAC_HS_WIDTH], 0, 11) + 3;

	// a frame is taken to start with a span that has bit 61 after a blank one: the Aristocrat MK6 shows its picture as a
	// span of 472 lines and eight of one line, all with bit 61
	const bool frame_start = BIT(data, 61) && m_dac_after_blank;
	m_dac_after_blank = BIT(data, 25) || BIT(dlw, 38);
	if (frame_start)
	{
		// start of frame: what was fed is what is shown
		std::copy_n(m_dac_feeding, m_dac_feeding_count, m_dac_showing);
		m_dac_showing_count = m_dac_feeding_count;

		u32 top = ~0U, bottom = 0, width = 0;
		for (u32 i = 0; i < m_dac_showing_count; i++)
		{
			const dac_span &span = m_dac_showing[i];
			if (!span.blank)
			{
				top = std::min<u32>(top, span.line);
				bottom = std::max<u32>(bottom, span.line + span.lines - 1);
				width = std::max<u32>(width, span.pixels);
			}
		}
		if (width && m_dac_htotal >= width && m_dac_line > bottom)
		{
			const rectangle visible(0, width - 1, top, bottom);
			const attotime period = attotime::from_ticks(u64(m_dac_htotal) * m_dac_line, pixel_frequency());
			if (screen().width() != m_dac_htotal || screen().height() != m_dac_line || screen().visible_area() != visible || screen().frame_period() != period)
			{
				LOGMASKED(LOG_DAC, "display %ux%u of %ux%u at %uHz\n", width, bottom - top + 1, m_dac_htotal, m_dac_line, pixel_frequency());
				screen().configure(m_dac_htotal, m_dac_line, visible, period);
				screen().reset_origin();
			}
		}
		m_dac_line = 0;
		m_dac_htotal = 0;
		m_dac_feeding_count = 0;
	}

	LOGMASKED(LOG_DAC, "DAC feed span %016x: line %u, %u lines of %u pixels, format %u%s%s%s\n", data, m_dac_line, lines, pixels, format,
			hsync ? ", hsync" : "", BIT(dlw, 38) ? ", vsync" : "", BIT(data, 25) ? ", blank" : "");

	if (m_dac_feeding_count < MAX_SPANS)
	{
		dac_span &span = m_dac_feeding[m_dac_feeding_count++];
		span.address = (BIT(m_dac_reg[3], 0, 7) << 25) | (u32(data) & 0x01ff'fff8);
		span.line = m_dac_line;
		span.lines = lines;
		span.pixels = pixels;
		span.format = format;
		span.blank = BIT(data, 25) || BIT(dlw, 38);
	}
	m_dac_line += lines;
	m_dac_htotal = std::max<u32>(m_dac_htotal, clocks);

	if (BIT(dlw, 38))
		set_irq(IRQ_DAC_VS);

	// while the display is sent the span, all the clocks of all its lines, the feed is busy and accepts nothing else
	m_dac_busy = true;
	m_dac_timer->adjust(attotime::from_ticks(u64(clocks) * lines, pixel_frequency()));
}

//**************************************************************************
//  2D core
//**************************************************************************

u32 powervr250_device::port_r(offs_t offset)
{
	// TODO: the MK6 code asks port register ff, with a number or an address, after it writes the control of an operation
	// there, and port register 54 now and then
	return 0;
}

void powervr250_device::port_w(offs_t offset, u32 data)
{
	m_2d[offset & 0xff] = data;
	if ((offset & 0xff) == P2D_START)
		draw_2d();
}

void powervr250_device::draw_2d()
{
	// the operation is done at once, as soon as it is started
	const u32 mask = m_vram_size - 1;
	const u32 lines = m_2d[P2D_START] & 0xffff;
	const u32 width = m_2d[P2D_SIZE] >> 16;
	const u32 x = m_2d[P2D_DEST_POS] >> 16, y = m_2d[P2D_DEST_POS] & 0xffff;
	const u32 sx = m_2d[P2D_SRC_POS] >> 16, sy = m_2d[P2D_SRC_POS] & 0xffff;
	// bit 16 of a pitch register is set by the MK6 code for some operations and is not part of the pitch
	// FIXME: what bit 16 of a pitch means is not known
	const u32 dest_pitch = m_2d[P2D_DEST_PITCH] & 0xffff, pitch_a = m_2d[P2D_PITCH_A] & 0xffff, pitch_b = m_2d[P2D_PITCH_B] & 0xffff;
	const u32 operation = m_2d[P2D_OPERATION];
	switch (operation >> 28)
	{
	case 2:
		// copy, with the positions and the width in bytes
		for (u32 line = 0; line < lines; line++)
		{
			const u32 to = m_2d[P2D_DEST] + (y + line) * dest_pitch + x;
			const u32 from = m_2d[P2D_SRC_A] + (sy + line) * pitch_a + sx;
			settle(to & mask, width);
			for (u32 i = 0; i < width; i++)
			{
				const u32 s = (from + i) & mask, d = (to + i) & mask;
				const u32 shift = (d & 3) * 8;
				m_vram[d >> 2] = (m_vram[d >> 2] & ~(0xffU << shift)) | (BIT(m_vram[s >> 2], (s & 3) * 8, 8) << shift);
			}
			written(to & mask, width);
		}
		break;

	case 1:
		// source B over source A with the alpha of source B, in pixels of 32 bits; source A is at the destination position
		// FIXME: the rounding of the blend is not known
		for (u32 line = 0; line < lines; line++)
		{
			const u32 to = m_2d[P2D_DEST] + (y + line) * dest_pitch + x * 4;
			const u32 back = m_2d[P2D_SRC_A] + (y + line) * pitch_a + x * 4;
			const u32 front = m_2d[P2D_SRC_B] + (sy + line) * pitch_b + sx * 4;
			settle(to & mask, width * 4);
			for (u32 i = 0; i < width; i++)
			{
				const u32 f = m_vram[((front + i * 4) & mask) >> 2];
				const u32 b = m_vram[((back + i * 4) & mask) >> 2];
				const u32 alpha = f >> 24;
				u32 result = f;
				if (alpha != 0xff)
				{
					result = f & 0xff00'0000;
					for (int shift = 0; shift < 24; shift += 8)
						result |= ((BIT(f, shift, 8) * alpha + BIT(b, shift, 8) * (255 - alpha) + 127) / 255) << shift;
				}
				m_vram[((to + i * 4) & mask) >> 2] = result;
			}
			written(to & mask, width * 4);
		}
		break;

	case 0:
		if ((operation & 0xff) == 0x16)
		{
			// a 1-bit image, the leftmost pixel in the top bit of each byte and each row in whole bytes, with set bits drawn
			// in the fill colour and clear bits left alone; only the destination clip of a line limits it
			// FIXME: bit 11 of the control is taken to draw the clear bits in the background colour; the MK6 code sets it
			// for its text (000208cc), and the known games look the same with the clear bits left alone
			// FIXME: the bits start two bytes past the address in source B, which the MK6 code points at the last two bytes
			// of the header of its glyphs; how the chip is told to pass over them is not known (the source position is 0)
			const u32 bits = m_2d[P2D_SRC_B] + 2;
			const u32 row_bytes = ((m_2d[P2D_MONO_WIDTH] & 0xffff) + 7) / 8;
			const bool opaque = BIT(m_2d[P2D_CONTROL], 11);
			const u32 clip_width = m_2d[P2D_CLIP] & 0xffff, clip_height = m_2d[P2D_CLIP] >> 16;
			for (u32 line = 0; line < lines; line++)
			{
				if (y + line >= clip_height)
					break;
				const u32 to = m_2d[P2D_DEST] + (y + line) * dest_pitch + x * 4;
				settle(to & mask, width * 4);
				for (u32 i = 0; (i < width) && (x + i < clip_width); i++)
				{
					const u32 s = (bits + line * row_bytes + i / 8) & mask;
					if (BIT(m_vram[s >> 2], (s & 3) * 8 + (~i & 7)))
						m_vram[((to + i * 4) & mask) >> 2] = m_2d[P2D_FILL_COLOR];
					else if (opaque)
						m_vram[((to + i * 4) & mask) >> 2] = m_2d[P2D_BACK_COLOR];
				}
				written(to & mask, width * 4);
			}
			break;
		}
		if ((operation & 0xfff) == 0x190)
		{
			// copy of source B, the high half of the start register the number of 64-bit words of each line; the MK6
			// code moves frames of symbol animations with it
			const u32 words = m_2d[P2D_START] >> 16;
			for (u32 line = 0; line < lines; line++)
			{
				const u32 to = m_2d[P2D_DEST] + (y + line) * dest_pitch + x * 4;
				const u32 from = m_2d[P2D_SRC_B] + (sy + line) * pitch_b + sx * 4;
				settle(to & mask, words * 8);
				for (u32 i = 0; i < words * 2; i++)
					m_vram[((to + i * 4) & mask) >> 2] = m_vram[((from + i * 4) & mask) >> 2];
				written(to & mask, words * 8);
			}
			break;
		}
		if ((operation & 0xfff) != 0x090)
		{
			logerror("2D operation %08x is not known\n", operation);
			break;
		}
		// fill, in pixels of 32 bits
		for (u32 line = 0; line < lines; line++)
		{
			const u32 to = m_2d[P2D_DEST] + (y + line) * dest_pitch + x * 4;
			settle(to & mask, width * 4);
			for (u32 i = 0; i < width; i++)
				m_vram[((to + i * 4) & mask) >> 2] = m_2d[P2D_FILL_COLOR];
			written(to & mask, width * 4);
		}
		break;

	case 7:
		{
			// line, a pixel of 32 bits at a time, moving along the main axis while the error term is negative and
			// diagonally otherwise
			const u32 count = m_2d[P2D_START] >> 16;
			const s64 diagonal_add = s16(m_2d[P2D_LINE_ERROR]);
			const s64 step_add = diagonal_add + 2 * (s64(count) - 1);
			s64 error = s16(m_2d[P2D_START]);
			s32 px = s16(m_2d[P2D_LINE_START] >> 16), py = s16(m_2d[P2D_LINE_START]);
			const s32 clip_width = m_2d[P2D_CLIP] & 0xffff, clip_height = m_2d[P2D_CLIP] >> 16;
			for (u32 i = 0; i < count; i++)
			{
				if ((px >= 0) && (py >= 0) && (px < clip_width) && (py < clip_height))
				{
					const u32 to = (m_2d[P2D_DEST] + py * dest_pitch + px * 4) & mask;
					settle(to, 4);
					m_vram[to >> 2] = m_2d[P2D_LINE_COLOR];
					written(to, 4);
				}
				const u32 step = (error >= 0) ? m_2d[P2D_LINE_DIAGONAL] : m_2d[P2D_LINE_STEP];
				px += s16(step >> 16);
				py += s16(step);
				error += (error >= 0) ? diagonal_add : step_add;
			}
		}
		break;

	default:
		logerror("2D operation %08x is not known\n", operation);
		break;
	}
}

TIMER_CALLBACK_MEMBER(powervr250_device::dac_span_done)
{
	m_dac_busy = false;
	m_core->dac_ready();
}

u32 powervr250_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	const u32 stride = BIT(m_dac_reg[3], 7, 15);
	const u32 mask = m_vram_size - 1;

	// the display shows a span in the frame it is fed in: the spans fed so far in this frame, then the rest of the frame
	// before
	const u32 count = std::max(m_dac_showing_count, m_dac_feeding_count);
	const auto shown = [this] (u32 i) -> const dac_span & { return (i < m_dac_feeding_count) ? m_dac_feeding[i] : m_dac_showing[i]; };

	// a render that is still being drawn has to be done first when what is shown is where its pixels go
	bool in_use = false;
	for (u32 i = 0; !in_use && i < count; i++)
	{
		const dac_span &span = shown(i);
		if (!span.blank && span.lines)
		{
			const u32 last = ((span.address + (span.lines - 1) * stride + span.pixels * 4) & mask) >> PAGE_SHIFT;
			for (u32 page = (span.address & mask) >> PAGE_SHIFT; !in_use; page = (page + 1) & (mask >> PAGE_SHIFT))
			{
				in_use = m_page_in_use[page];
				if (page == last)
					break;
			}
		}
	}
	if (in_use)
		m_renderer->finish();
	const auto byte = [this, mask] (u32 address) -> u8 { return m_vram[(address & mask) >> 2] >> (8 * (address & 3)); };
	const auto word = [this, mask] (u32 address) -> u16 { return m_vram[(address & mask) >> 2] >> (8 * (address & 2)); };

	for (u32 i = 0; i < count; i++)
	{
		const dac_span &span = shown(i);
		if (span.blank)
			continue;

		const int right = std::min<int>(cliprect.right(), span.pixels - 1);
		for (int y = std::max<int>(cliprect.top(), span.line); y <= std::min<int>(cliprect.bottom(), span.line + span.lines - 1); y++)
		{
			const u32 row = span.address + (y - span.line) * stride;
			u32 *const dest = &bitmap.pix(y);
			for (int x = cliprect.left(); x <= right; x++)
			{
				switch (span.format)
				{
				case 1: // 555
					{
						const u16 pixel = (row & 1) ? (byte(row + x * 2) | (byte(row + x * 2 + 1) << 8)) : word(row + x * 2);
						dest[x] = rgb_t(pal5bit(BIT(pixel, 10, 5)), pal5bit(BIT(pixel, 5, 5)), pal5bit(BIT(pixel, 0, 5)));
					}
					break;
				case 2: // 565
					{
						const u16 pixel = (row & 1) ? (byte(row + x * 2) | (byte(row + x * 2 + 1) << 8)) : word(row + x * 2);
						dest[x] = rgb_t(pal5bit(BIT(pixel, 11, 5)), pal6bit(BIT(pixel, 5, 6)), pal5bit(BIT(pixel, 0, 5)));
					}
					break;
				case 3: // 888
					dest[x] = rgb_t(byte(row + x * 3 + 2), byte(row + x * 3 + 1), byte(row + x * 3));
					break;
				case 4: // x888
					dest[x] = rgb_t(byte(row + x * 4 + 2), byte(row + x * 4 + 1), byte(row + x * 4));
					break;
				default: // TODO: 8-bit indexed and the YUV formats
					break;
				}
			}
		}
	}
	return 0;
}

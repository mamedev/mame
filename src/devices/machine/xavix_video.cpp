// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    XaviX, XaviX2000 and SuperXaviX video devices. Define their register maps,
    graphics and palette storage, device lifecycle and sprite RAM DMA transfers.
    Rendering and picture timing are implemented in the companion rendering
    and timing files.

    TODO:
    - Model shared-bus contention and sprite RAM read/write timing.
    - Establish shared-bus bitmap-palette access by the renderer and DMA.
    - Establish the reset behaviour of the bitmap source-bottom
      registers.

***************************************************************************/

#include "emu.h"
#include "xavix_video.h"

#include "util/multibyte.h"

#include <algorithm>
#include <array>

#define LOG_TILEMAP       (1U << 1)
#define LOG_SPRITES       (1U << 3)
#define LOG_REGISTERS     (1U << 4)

#define VERBOSE 0
#include "logmacro.h"


DEFINE_DEVICE_TYPE(XAVIX_VIDEO, xavix_video_device, "xavix_video", "XaviX Video")
DEFINE_DEVICE_TYPE(XAVIX2000_VIDEO, xavix2000_video_device, "xavix2000_video", "XaviX2000 Video")
DEFINE_DEVICE_TYPE(SUPERXAVIX_VIDEO, superxavix_video_device, "superxavix_video", "SuperXaviX Video")

xavix_video_device::xavix_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_video_device(mconfig, XAVIX_VIDEO, tag, owner, clock, 1)
{
}

xavix_video_device::xavix_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int video_hres_scale)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_gfx_interface(mconfig, *this)
	, m_video_read_cb(*this)
	, m_irq_cb(*this)
	, m_nmi_cb(*this)
	, m_sprite_dma_source_cb(*this, 0xff)
	, m_spriteram(*this, "spriteram")
	, m_tilemap1_regs(*this, "tilemap1_regs")
	, m_tilemap2_regs(*this, "tilemap2_regs")
	, m_graphics_palram_sh(*this, "graphics_palram_sh")
	, m_graphics_palram_l(*this, "graphics_palram_l")
	, m_raster_irq_x(*this, "raster_irq_x")
	, m_raster_irq_y(*this, "raster_irq_y")
	, m_segment_regs(*this, "segment_regs")
	, m_video_standard(video_standard::NTSC)
	, m_video_hres_scale(video_hres_scale)
	, m_gfx_region(*this, finder_base::DUMMY_TAG)
{
}

xavix2000_video_device::xavix2000_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_video_device(mconfig, XAVIX2000_VIDEO, tag, owner, clock, 1)
{
}

superxavix_video_device::superxavix_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_video_device(mconfig, SUPERXAVIX_VIDEO, tag, owner, clock, PICTURE_PIXEL_CLOCKS / OUTPUT_PIXEL_CLOCKS)
	, m_graphics_read_cb(*this)
	, m_bitmap_read_cb(*this)
	, m_extended_segment_regs(*this, "extended_segment_regs")
	, m_video_timing_regs(*this, "video_timing_regs")
	, m_video_position_regs(*this, "video_position_regs")
	, m_bitmap_palram_sh(*this, "bitmap_palram_sh")
	, m_bitmap_palram_l(*this, "bitmap_palram_l")
	, m_bitmap_params(*this, "bitmap_params")
{
}

void xavix_video_device::map(address_map &map)
{
	// Memory emulator / text array
	map(0x4000, 0x4fff).rw(FUNC(xavix_video_device::text_array_r), FUNC(xavix_video_device::text_array_w));

	// Sprite RAM (aka Fragment RAM)
	map(0x6000, 0x67ff).ram().rw(FUNC(xavix_video_device::spriteram_r), FUNC(xavix_video_device::spriteram_cpu_w)).share(m_spriteram);

	// Graphics palette RAM
	map(0x6800, 0x68ff).ram().rw(FUNC(xavix_video_device::graphics_palram_r<palette_plane::SH>), FUNC(xavix_video_device::graphics_palram_w<palette_plane::SH>)).share(m_graphics_palram_sh);
	map(0x6900, 0x69ff).ram().rw(FUNC(xavix_video_device::graphics_palram_r<palette_plane::L>), FUNC(xavix_video_device::graphics_palram_w<palette_plane::L>)).share(m_graphics_palram_l);

	// Segment RAM
	map(0x6a00, 0x6a1f).ram().share(m_segment_regs); // test mode, pass flag 0x20

	// Possible pixel-decoder resynchronisation control; enabled behaviour is unverified.
	map(0x6fc0, 0x6fc0).w(FUNC(xavix_video_device::pixel_decoder_ctrl_w));

	// Tilemap 1 registers
	map(0x6fc8, 0x6fcf).ram().w(FUNC(xavix_video_device::tilemap_regs_w<0>)).share(m_tilemap1_regs);

	// Tilemap 2 registers
	map(0x6fd0, 0x6fd7).ram().w(FUNC(xavix_video_device::tilemap_regs_w<1>)).share(m_tilemap2_regs);

	// Sprite control
	map(0x6fd8, 0x6fd8).lr8(NAME([this] () -> u8 { return m_sprite_address_mode; })).w(FUNC(xavix_video_device::sprite_address_mode_w));

	// Sprite DMA
	map(0x6fe0, 0x6fe0).lr8(NAME([this] () -> u8 { return m_sprite_dma_ctrl; })).w(FUNC(xavix_video_device::sprite_dma_ctrl_w));
	map(0x6fe1, 0x6fe2).lrw8(NAME([this] (offs_t offset) -> u8 { return m_sprite_dma_src[offset]; }), NAME([this] (offs_t offset, u8 data) { m_sprite_dma_src[offset] = data & (offset ? 0x7f : 0xff); }));
	map(0x6fe5, 0x6fe5).lrw8(NAME([this] () -> u8 { return m_sprite_dma_dst; }), NAME([this] (u8 data) { m_sprite_dma_dst = data & (SPRITE_RAM_PLANES - 1); }));
	// Reads expose the active byte count; writes supply the next reload value.
	map(0x6fe6, 0x6fe6).lrw8(NAME([this] () -> u8 { return m_sprite_dma_count; }), NAME([this] (u8 data) { m_sprite_dma_len = data; }));

	// Arena window and status
	map(0x6fe8, 0x6fe8).rw(FUNC(xavix_video_device::arena_start_r), FUNC(xavix_video_device::arena_start_w)); // r/w tested
	map(0x6fe9, 0x6fe9).rw(FUNC(xavix_video_device::arena_end_r), FUNC(xavix_video_device::arena_end_w)); // r/w tested
	map(0x6fea, 0x6fea).rw(FUNC(xavix_video_device::arena_ctrl_r), FUNC(xavix_video_device::arena_ctrl_w));

	// Video noise and colour effects
	map(0x6fec, 0x6fee).lr8(NAME([this] (offs_t offset) -> u8 { return m_video_noise_feedback[offset]; })).w(FUNC(xavix_video_device::video_noise_feedback_w));
	map(0x6ff0, 0x6ff2).lr8(NAME([this] (offs_t offset) -> u8 { return m_color_effect[offset]; })).w(FUNC(xavix_video_device::color_effect_w));
	map(0x6ff4, 0x6ff6).r(FUNC(xavix_video_device::video_noise_state_r));

	// Video control and interrupt status
	map(0x6ff8, 0x6ff8).rw(FUNC(xavix_video_device::video_ctrl_r), FUNC(xavix_video_device::video_ctrl_w)); // always seems to be a read/store or read/modify/store

	// Video-system status
	map(0x6ff9, 0x6ff9).r(FUNC(xavix_video_device::video_status_r));

	// Raster-interrupt position
	map(0x6ffa, 0x6ffa).ram().w(FUNC(xavix_video_device::raster_irq_x_w)).share(m_raster_irq_x);
	map(0x6ffb, 0x6ffb).ram().w(FUNC(xavix_video_device::raster_irq_y_w)).share(m_raster_irq_y); // increases / decreases when you jump in snowboard (snowboard, used to blank ground)
}

void superxavix_video_device::map(address_map &map)
{
	xavix_video_device::map(map);

	// Direct reads also select the indexed graphics palette address.
	map(0x6800, 0x68ff).r(FUNC(superxavix_video_device::graphics_palram_r<palette_plane::SH>));
	map(0x6900, 0x69ff).r(FUNC(superxavix_video_device::graphics_palram_r<palette_plane::L>));

	// Extended segment registers (16 x 32-bit)
	map(0x6a40, 0x6a7f).ram().w(FUNC(superxavix_video_device::extended_segment_regs_w)).share(m_extended_segment_regs);

	// Bitmap palette RAM
	map(0x6c00, 0x6cff).ram().rw(FUNC(superxavix_video_device::bitmap_palram_r<palette_plane::SH>), FUNC(superxavix_video_device::bitmap_palram_w<palette_plane::SH>)).share(m_bitmap_palram_sh);
	map(0x6d00, 0x6dff).ram().rw(FUNC(superxavix_video_device::bitmap_palram_r<palette_plane::L>), FUNC(superxavix_video_device::bitmap_palram_w<palette_plane::L>)).share(m_bitmap_palram_l);

	// Graphics and bitmap palette controls
	map(0x6f78, 0x6f78).rw(FUNC(superxavix_video_device::palette_index_r<palette_bank::GRAPHICS>), FUNC(superxavix_video_device::palette_index_w<palette_bank::GRAPHICS>));
	map(0x6f79, 0x6f79).rw(NAME((&superxavix_video_device::palette_component_r<palette_bank::GRAPHICS, palette_component::HUE>)), NAME((&superxavix_video_device::palette_component_w<palette_bank::GRAPHICS, palette_component::HUE>)));
	map(0x6f7a, 0x6f7a).rw(NAME((&superxavix_video_device::palette_component_r<palette_bank::GRAPHICS, palette_component::SATURATION>)), NAME((&superxavix_video_device::palette_component_w<palette_bank::GRAPHICS, palette_component::SATURATION>)));
	map(0x6f7b, 0x6f7b).rw(NAME((&superxavix_video_device::palette_component_r<palette_bank::GRAPHICS, palette_component::LIGHTNESS>)), NAME((&superxavix_video_device::palette_component_w<palette_bank::GRAPHICS, palette_component::LIGHTNESS>)));

	map(0x6f7c, 0x6f7c).rw(FUNC(superxavix_video_device::palette_index_r<palette_bank::BITMAP>), FUNC(superxavix_video_device::palette_index_w<palette_bank::BITMAP>));
	map(0x6f7d, 0x6f7d).rw(NAME((&superxavix_video_device::palette_component_r<palette_bank::BITMAP, palette_component::HUE>)), NAME((&superxavix_video_device::palette_component_w<palette_bank::BITMAP, palette_component::HUE>)));
	map(0x6f7e, 0x6f7e).rw(NAME((&superxavix_video_device::palette_component_r<palette_bank::BITMAP, palette_component::SATURATION>)), NAME((&superxavix_video_device::palette_component_w<palette_bank::BITMAP, palette_component::SATURATION>)));
	map(0x6f7f, 0x6f7f).rw(NAME((&superxavix_video_device::palette_component_r<palette_bank::BITMAP, palette_component::LIGHTNESS>)), NAME((&superxavix_video_device::palette_component_w<palette_bank::BITMAP, palette_component::LIGHTNESS>)));

	// Video timing, position and graphics mode
	map(0x6f80, 0x6f99).ram().w(FUNC(superxavix_video_device::video_timing_regs_w)).share(m_video_timing_regs);
	// Reserved timing addresses read as zero and ignore writes.
	map(0x6f81, 0x6f81).lr8(NAME([] () -> u8 { return 0; })).nopw();
	map(0x6f89, 0x6f89).lr8(NAME([] () -> u8 { return 0; })).nopw();
	map(0x6f8f, 0x6f8f).lr8(NAME([] () -> u8 { return 0; })).nopw();
	map(0x6f91, 0x6f91).lr8(NAME([] () -> u8 { return 0; })).nopw();
	map(0x6fa0, 0x6fa7).ram().w(FUNC(superxavix_video_device::video_position_regs_w)).share(m_video_position_regs);
	map(0x6fa8, 0x6fa8).lrw8(NAME([this] () -> u8 { return m_graphics_mode; }), NAME([this] (u8 data) { m_graphics_mode = data; }));

	// Bitmap layer parameters
	map(0x6fb0, 0x6fc4).ram().w(FUNC(superxavix_video_device::bitmap_params_w)).share(m_bitmap_params);
	map(0x6fc5, 0x6fc7).nopw(); // reserved
}

// ROM graphics layouts for the graphics viewer, not the live renderer.
const gfx_layout xavix_video_device::TILE_16X16_2BPP_LAYOUT =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0,1 },
	{ 6,4,2,0,14,12,10,8,22,20,18,16,30,28,26,24 },
	{ STEP16(0,16*2) },
	16*16*2
};

void xavix_video_device::device_start()
{
	decode_graphics();
	m_video_read_cb.resolve();
	screen().register_screen_bitmap(m_pen_bitmap);
	m_color_effect_palette = std::make_unique<pen_t[]>(palette().entries() * COLOR_EFFECT_NOISE_LEVELS);

	m_raster_irq_timer = timer_alloc(FUNC(xavix_video_device::raster_irq_gen), this);
	m_arena_irq_timer = timer_alloc(FUNC(xavix_video_device::arena_irq_gen), this);
	m_sprite_dma_timer = timer_alloc(FUNC(xavix_video_device::sprite_dma_tick), this);
	m_video_counter_timer = timer_alloc(FUNC(xavix_video_device::counter_field_start), this);

	save_item(NAME(m_text_array));
	save_item(NAME(m_sprite_read_latch));
	save_item(NAME(m_graphics_palette_read_latch));
	save_item(NAME(m_sprite_dma_src));
	save_item(NAME(m_sprite_dma_dst));
	save_item(NAME(m_sprite_dma_len));
	save_item(NAME(m_sprite_dma_count));
	save_item(NAME(m_sprite_dma_row));
	save_item(NAME(m_sprite_dma_ctrl));
	save_item(NAME(m_arena_start));
	save_item(NAME(m_arena_end));
	save_item(NAME(m_arena_ctrl));
	save_item(NAME(m_arena_irq_pending));
	save_item(NAME(m_arena.flag));
	save_item(NAME(m_arena.output));
	save_item(NAME(m_arena.compare));
	save_item(NAME(m_video_counters.horizontal));
	save_item(NAME(m_video_counters.vertical));
	save_item(NAME(m_video_counters.picture_x));
	save_item(NAME(m_video_counters.picture_y));
	save_item(NAME(m_video_counter_ticks));
	save_item(NAME(m_color_effect));
	save_item(NAME(m_video_noise_state));
	save_item(NAME(m_video_noise_feedback));
	save_item(NAME(m_video_noise_ticks));
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_sprite_address_mode));
	save_item(NAME(m_transparent_pens));
}

void superxavix_video_device::device_start()
{
	xavix_video_device::device_start();
	m_graphics_read_cb.resolve();
	m_bitmap_read_cb.resolve();
	screen().register_screen_bitmap(m_output_bitmap);
	m_video_field_timer = timer_alloc(FUNC(superxavix_video_device::video_field_start), this);
	m_picture_nmi_timer = timer_alloc(FUNC(superxavix_video_device::picture_nmi), this);

	save_item(STRUCT_MEMBER(m_palette_ports, index));
	save_item(STRUCT_MEMBER(m_palette_ports, color_latch));
	save_item(NAME(m_bitmap_palette_read_latch));
	save_item(NAME(m_video_timing.horizontal_total));
	save_item(NAME(m_video_timing.half_lines));
	save_item(NAME(m_video_timing.active_width));
	save_item(NAME(m_video_timing.active_height));
	save_item(NAME(m_video_timing.horizontal_start));
	save_item(NAME(m_video_timing.vertical_origin));
	save_item(NAME(m_video_field_phase));
	save_item(NAME(m_graphics_mode));
}

void xavix_video_device::device_reset()
{
	m_sprite_read_latch = 0;
	m_graphics_palette_read_latch = 0;
	m_video_ctrl = 0;
	m_raster_irq_x[0] = 0;
	m_raster_irq_y[0] = 0;
	m_raster_irq_timer->adjust(attotime::never);

	m_arena_ctrl = 0;
	m_arena_start = 0;
	m_arena_end = 0;
	m_arena_irq_pending = false;
	m_arena = {};
	m_arena_irq_timer->adjust(attotime::never);
	m_color_effect[0] = 0;
	m_color_effect[1] = 0;
	m_color_effect[2] = COLOR_EFFECT_HUE | COLOR_EFFECT_SATURATION | COLOR_EFFECT_LIGHTNESS;
	m_color_effect_palette_valid = false;
	m_video_noise_state = 1;
	// Feedback-register reset value, low byte first.
	m_video_noise_feedback[0] = 0x22;
	m_video_noise_feedback[1] = 0x04;
	m_video_noise_feedback[2] = 0x80;
	m_video_noise_ticks = machine().time().as_ticks(clock());

	m_sprite_address_mode = 0;
	m_transparent_pens.fill(-1);

	std::ranges::fill(m_sprite_dma_src, 0x00);
	m_sprite_dma_dst = 0;
	m_sprite_dma_len = 0;
	m_sprite_dma_count = 0;
	m_sprite_dma_row = 0;
	m_sprite_dma_ctrl = 0;
	m_sprite_dma_timer->adjust(attotime::never);
	std::fill_n(&m_tilemap1_regs[0], TILEMAP_REGISTER_COUNT, 0x00);
	std::fill_n(&m_tilemap2_regs[0], TILEMAP_REGISTER_COUNT, 0x00);
	std::ranges::fill(m_segment_regs, 0x00);
	std::ranges::fill(m_text_array, 0x00);

	update_video_irq_source();
	m_nmi_cb(0);
	reset_video_timing();
	schedule_raster_irq();
}

void superxavix_video_device::device_reset()
{
	m_video_field_phase = false;
	m_graphics_mode = 0;
	m_palette_ports = {};
	m_bitmap_palette_read_latch = 0;
	m_picture_nmi_timer->adjust(attotime::never);
	m_video_field_timer->adjust(attotime::never);
	// Preserve the source-bottom pair at 0x6fba/0x6fbb until its reset behaviour is known.
	std::fill(&m_bitmap_params[BITMAP_TOP], &m_bitmap_params[BITMAP_ADDRESS_TOP] + 2, 0x00);
	std::fill(&m_bitmap_params[BITMAP_ADDRESS_LEFT], &m_bitmap_params[BITMAP_CTRL] + 1, 0x00);
	m_bitmap_params[BITMAP_RESOLUTION] = 0xf0;
	xavix_video_device::device_reset();
}

void xavix_video_device::device_post_load()
{
	m_color_effect_palette_valid = false;
}

void xavix_video_device::decode_graphics(memory_region *extra_region)
{
	const char *const region = m_gfx_region->name().c_str();
	GFXDECODE_START(gfxinfo)
		GFXDECODE_ENTRY(region, 0, TILE_16X16_2BPP_LAYOUT, 0, 64)
		GFXDECODE_ENTRY(region, 0, gfx_8x8x4_packed_lsb, 0, 16)
		GFXDECODE_ENTRY(region, 0, gfx_16x16x4_packed_lsb, 0, 16)
		GFXDECODE_ENTRY(region, 0, gfx_8x8x8_raw, 0, 1)
		GFXDECODE_ENTRY(region, 0, gfx_16x16x8_raw, 0, 1)
	GFXDECODE_END
	if (extra_region)
	{
		// Decode the cartridge independently of the motherboard ROM.
		constexpr unsigned region_entries = std::size(gfxinfo) - 1;
		std::array<gfx_decode_entry, region_entries * 2 + 1> entries{};
		std::copy_n(gfxinfo, region_entries, entries.begin());
		std::copy_n(gfxinfo, region_entries, entries.begin() + region_entries);
		for (unsigned index = region_entries; index < region_entries * 2; ++index)
			entries[index].memory_region = extra_region->name().c_str();
		decode_gfx(entries.data());
	}
	else
		decode_gfx(gfxinfo);
}

template <xavix_video_device::palette_plane Plane>
u8 xavix_video_device::graphics_palram_r(offs_t offset)
{
	u8 const data = (Plane == palette_plane::SH ? m_graphics_palram_sh : m_graphics_palram_l)[offset];
	if (machine().side_effects_disabled())
		return data;

	// Both byte planes return the preceding CPU access's data.
	return std::exchange(m_graphics_palette_read_latch, data);
}

template <xavix_video_device::palette_plane Plane>
void xavix_video_device::graphics_palram_w(offs_t offset, u8 data)
{
	screen().update_partial(screen().vpos());

	m_graphics_palette_read_latch = data;
	if constexpr (Plane == palette_plane::SH)
	{
		// Direct SH writes select one transparent pen per 16-colour block. Indexed
		// palette writes change the colour but leave this selection untouched.
		auto &transparent_pen = m_transparent_pens[offset >> 4];
		if ((data & 0x1f) >= TRANSPARENT_HUE_MIN)
			transparent_pen = offset;
		else if (transparent_pen == offset)
			transparent_pen = -1;
	}

	set_palette_data(offset, Plane == palette_plane::SH
			? data | (u16(m_graphics_palram_l[offset]) << 8)
			: m_graphics_palram_sh[offset] | (u16(data) << 8));
}

template <xavix_video_device::palette_plane Plane>
u8 superxavix_video_device::graphics_palram_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_palette_ports[unsigned(palette_bank::GRAPHICS)].index = offset;
	return xavix_video_device::graphics_palram_r<Plane>(offset);
}

template <xavix_video_device::palette_plane Plane>
u8 superxavix_video_device::bitmap_palram_r(offs_t offset)
{
	u8 const data = (Plane == palette_plane::SH ? m_bitmap_palram_sh : m_bitmap_palram_l)[offset];
	if (machine().side_effects_disabled())
		return data;

	m_palette_ports[unsigned(palette_bank::BITMAP)].index = offset;
	return std::exchange(m_bitmap_palette_read_latch, data);
}

template <xavix_video_device::palette_plane Plane>
void superxavix_video_device::bitmap_palram_w(offs_t offset, u8 data)
{
	screen().update_partial(screen().vpos());

	m_bitmap_palette_read_latch = data;
	set_palette_data(offset + PALETTE_BANK_ENTRIES, Plane == palette_plane::SH
			? data | (u16(m_bitmap_palram_l[offset]) << 8)
			: m_bitmap_palram_sh[offset] | (u16(data) << 8));
}

u16 xavix_video_device::palette_data(int pen) const
{
	pen &= PALETTE_BANK_ENTRIES - 1;
	return u16(m_graphics_palram_sh[pen]) | (u16(m_graphics_palram_l[pen]) << 8);
}

u16 superxavix_video_device::palette_data(int pen) const
{
	if (pen < PALETTE_BANK_ENTRIES)
		return xavix_video_device::palette_data(pen);

	pen &= PALETTE_BANK_ENTRIES - 1;
	return u16(m_bitmap_palram_sh[pen]) | (u16(m_bitmap_palram_l[pen]) << 8);
}

void xavix_video_device::set_palette_data(int pen, u16 data)
{
	int const index = pen & (PALETTE_BANK_ENTRIES - 1);
	m_graphics_palram_sh[index] = data;
	m_graphics_palram_l[index] = data >> 8;
	update_pen(pen, data);
}

void superxavix_video_device::set_palette_data(int pen, u16 data)
{
	// Direct and component writes share an address and capture the merged colour.
	if (pen < PALETTE_BANK_ENTRIES)
	{
		m_palette_ports[unsigned(palette_bank::GRAPHICS)] = { u8(pen), data };
		xavix_video_device::set_palette_data(pen, data);
	}
	else
	{
		int const index = pen & (PALETTE_BANK_ENTRIES - 1);
		m_palette_ports[unsigned(palette_bank::BITMAP)] = { u8(index), data };
		m_bitmap_palram_sh[index] = data;
		m_bitmap_palram_l[index] = data >> 8;
		update_pen(pen, data);
	}
}

u8 xavix_video_device::spriteram_r(offs_t offset)
{
	if (machine().side_effects_disabled())
		return m_spriteram[offset];

	return std::exchange(m_sprite_read_latch, m_spriteram[offset]);
}

void xavix_video_device::spriteram_cpu_w(offs_t offset, u8 data)
{
	m_sprite_read_latch = data;
	spriteram_w(offset, data);
}

void xavix_video_device::spriteram_w(offs_t offset, u8 data)
{
	auto const set_sprite_x_high = [this, offset](bool high)
	{
		offs_t const index = offset & (SPRITE_COUNT - 1);
		m_spriteram[SPRITE_ATTR0 + index] = (m_spriteram[SPRITE_ATTR0 + index] & 0xfe) | high;
		m_spriteram[SPRITE_X_HIGH + index] = high;
	};

	if (offset < SPRITE_ATTR1) // 0x000 - 0x0ff
	{
		m_spriteram[offset] = data & 0xfe;
		set_sprite_x_high(data & 0x01);
	}
	else if (offset < SPRITE_X) // 0x100-0x2ff
	{
		m_spriteram[offset] = data;
	}
	else if (offset < SPRITE_X_HIGH) // 0x300-0x3ff
	{
		// copy sprite x bit 7 into bit 8 (which is stored at 0x4xx, and can also be seen in the low bit of 0x0xx)
		// pl1000 has explicit codepaths where it doesn't bother to set the 0x400-0x4ff range if it isn't needed
		// and we can't be using old values from there
		m_spriteram[offset] = data;
		set_sprite_x_high(data & 0x80);
	}
	else if (offset < SPRITE_ADDRESS_LOW) // 0x400-0x4ff
	{
		set_sprite_x_high(data & 0x01);
	}
	else // 0x500-0x7ff
	{
		m_spriteram[offset] = data;
	}
}

void superxavix_video_device::bitmap_params_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: bitmap_params_w %02x %02x\n", machine().describe_context(), offset, data);
	switch (offset)
	{
	case BITMAP_TOP + 1:
	case BITMAP_BOTTOM + 1:
		data &= 0x01;
		break;
	case BITMAP_LEFT + 1:
	case BITMAP_RIGHT + 1:
	case BITMAP_ADDRESS_LEFT + 1:
	case BITMAP_ADDRESS_RIGHT + 1:
		data &= 0x03;
		break;
	}

	if (data != m_bitmap_params[offset])
	{
		screen().update_partial(screen().vpos());
		m_bitmap_params[offset] = data;
	}
}

void superxavix_video_device::extended_segment_regs_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: extended_segment_regs_w %02x %02x\n", machine().describe_context(), offset, data);
	m_extended_segment_regs[offset] = data;
}

template <superxavix_video_device::palette_bank Bank>
void superxavix_video_device::palette_index_w(u8 data)
{
	auto &port = m_palette_ports[unsigned(Bank)];
	port.index = data;
	port.color_latch = palette_data(unsigned(Bank) * PALETTE_BANK_ENTRIES + data);
}

template <superxavix_video_device::palette_bank Bank>
u8 superxavix_video_device::palette_index_r()
{
	return m_palette_ports[unsigned(Bank)].index;
}

template <superxavix_video_device::palette_bank Bank, superxavix_video_device::palette_component Component>
u8 superxavix_video_device::palette_component_r()
{
	u16 const color = m_palette_ports[unsigned(Bank)].color_latch;
	if constexpr (Component == palette_component::HUE)
		return pen_hue(color) << 2;
	else if constexpr (Component == palette_component::SATURATION)
		return pen_saturation(color) << 4;
	else
		return pen_lightness(color) << 2;
}

template <superxavix_video_device::palette_bank Bank, superxavix_video_device::palette_component Component>
void superxavix_video_device::palette_component_w(u8 data)
{
	screen().update_partial(screen().vpos());

	(Bank == palette_bank::BITMAP ? m_bitmap_palette_read_latch : m_graphics_palette_read_latch) = data;

	int const pen = unsigned(Bank) * PALETTE_BANK_ENTRIES + m_palette_ports[unsigned(Bank)].index;
	u16 color = palette_data(pen);

	if constexpr (Component == palette_component::HUE)
		color = replace_pen_hue(color, data >> 2);
	else if constexpr (Component == palette_component::SATURATION)
		color = replace_pen_saturation(color, data >> 4);
	else
		color = replace_pen_lightness(color, data >> 2);

	set_palette_data(pen, color);
}

void xavix_video_device::update_pen(int pen, u16 data)
{
	palette().set_pen_color(pen, pen_color(data));
	m_color_effect_palette_valid = false;
}

void xavix_video_device::sprite_dma_ctrl_w(u8 data)
{
	bool const start = (data & SPRITE_DMA_RUN) && !(m_sprite_dma_ctrl & SPRITE_DMA_RUN);
	// The completion flag is write-one-to-clear. Bits 3 and 4 are not part of
	// the control field; software uses both 0x40 and 0x48 for eight-block copies.
	if ((data & SPRITE_DMA_COMPLETE) || !(data & SPRITE_DMA_IRQ_ENABLE))
		m_sprite_dma_ctrl &= ~SPRITE_DMA_COMPLETE;

	m_sprite_dma_ctrl = (m_sprite_dma_ctrl & (SPRITE_DMA_RUN | SPRITE_DMA_COMPLETE))
			| (data & (SPRITE_DMA_RUN | SPRITE_DMA_IRQ_ENABLE | SPRITE_DMA_BLOCK_COUNT_MASK));
	// Every control write reloads the byte count and row. Clearing Start does
	// not abort a transfer, and reasserting it does not postpone the next byte.
	m_sprite_dma_count = m_sprite_dma_len;
	m_sprite_dma_row = 0;
	if (start)
		m_sprite_dma_timer->adjust(clocks_to_attotime(SPRITE_DMA_BYTE_CLOCKS));
	update_video_irq_source();

	LOG("%s: sprite DMA ctrl=%02x src=%04x plane=%u count=%u\n",
		machine().describe_context(), m_sprite_dma_ctrl, get_u16le(m_sprite_dma_src), m_sprite_dma_dst, m_sprite_dma_count);
}

TIMER_CALLBACK_MEMBER(xavix_video_device::sprite_dma_tick)
{
	// Finish displayed rows before changing sprite RAM; mapped source reads
	// may return through video registers, whose readback must not advance DMA.
	screen().update_partial(screen().vpos());
	u16 const src = get_u16le(m_sprite_dma_src);
	u16 const dst = m_sprite_dma_dst * SPRITE_COUNT + m_sprite_dma_row;
	spriteram_w(dst, m_sprite_dma_source_cb(src));
	put_u16le(m_sprite_dma_src, (src + 1) & 0x7fff);
	++m_sprite_dma_row;
	if (!--m_sprite_dma_count)
	{
		m_sprite_dma_count = m_sprite_dma_len;
		m_sprite_dma_row = 0;
		m_sprite_dma_dst = (m_sprite_dma_dst + 1) & (SPRITE_RAM_PLANES - 1);
		u8 const blocks = (m_sprite_dma_ctrl - 1) & SPRITE_DMA_BLOCK_COUNT_MASK;
		m_sprite_dma_ctrl = (m_sprite_dma_ctrl & ~SPRITE_DMA_BLOCK_COUNT_MASK) | blocks;
		if (!blocks)
		{
			m_sprite_dma_ctrl &= ~SPRITE_DMA_RUN;
			if (m_sprite_dma_ctrl & SPRITE_DMA_IRQ_ENABLE)
				m_sprite_dma_ctrl |= SPRITE_DMA_COMPLETE;
			update_video_irq_source();
		}
	}

	// Transfer on alternate master clocks without bus contention.
	// TODO: Model shared-bus contention and sprite RAM read/write timing.
	if (m_sprite_dma_ctrl & SPRITE_DMA_RUN)
		m_sprite_dma_timer->adjust(clocks_to_attotime(SPRITE_DMA_BYTE_CLOCKS));
}

void xavix_video_device::pixel_decoder_ctrl_w(u8 data)
{
	if (data)
		logerror("%s: unimplemented video control 6fc0 = %02x\n", machine().describe_context(), data);
}

template <unsigned Layer>
void xavix_video_device::tilemap_regs_w(offs_t offset, u8 data, u8 mem_mask)
{
	/*
	   0x0 pointer to low tile bits
	   0x1 pointer to middle tile bits
	   0x2 pointer to upper tile bits or per-tile attributes

	   0x3 lltt bbb-  ll = pixel layout
	                  tt = tile/tilemap size
	                  b = bpp
	                  - = unused

	   0x4 scroll
	   0x5 scroll

	   0x6 pppp zzzz  p = palette
	                  z = priority

	   0x7 e--a ammm  e = enable
	                  aa = attribute source (0 = register, 1 = array, 2 = inline header)
	                  m = addressing mode

	   modes are
	    ---0 0000 (00) 8-bit tile number
	    ---0 0001 (01) 16-bit tile number
	    ---0 0010 (02) 16-bit 8-byte-aligned address
	    ---0 0011 (03) 16-bit segmented byte address
	    ---0 0100 (04) 24-bit byte address
	    ---0 0101 (05) extended-segment tile number
	    ---0 0110 (06) extended-segment 8-byte-aligned address
	    ---0 0111 (07) 24-bit 8-byte-aligned address
	    ---0 1000 (08) 8-bit tile number with 8-bit attributes
	    ---0 1001 (09) 16-bit tile number with 8-bit attributes
	    ---0 1010 (0a) 16-bit 8-byte-aligned address with 8-bit attributes
	    ---0 1011 (0b) 16-bit segmented byte address with 8-bit attributes
	    ---0 1100 (0c) 24-bit byte address with 8-bit attributes
	    ---0 1101 (0d) extended-segment tile number with 8-bit attributes
	    ---0 1110 (0e) extended-segment 8-byte-aligned address with 8-bit attributes
	    ---0 1111 (0f) 24-bit 8-byte-aligned address with 8-bit attributes
	    ---1 0011 (13) 16-bit segmented byte address with inline header
	    ---1 0100 (14) 24-bit byte address with inline header

	*/

	if ((offset != TILEMAP_SCROLL_X) && (offset != TILEMAP_SCROLL_Y))
	{
		LOGMASKED(LOG_TILEMAP, "%s: tilemap_regs_w layer %u offset %02x data %02x\n", machine().describe_context(), Layer, offset, data);
	}

	static_assert(Layer < 2);
	auto &regs = Layer ? m_tilemap2_regs : m_tilemap1_regs;
	COMBINE_DATA(&regs[offset]);
	regs[offset] &= TILEMAP_REGISTER_MASK[offset];
}


void xavix_video_device::sprite_address_mode_w(u8 data)
{
	LOGMASKED(LOG_SPRITES, "%s: sprite_address_mode_w %02x\n", machine().describe_context(), data);
	/*
	    Bits 2-0 select the sprite address mode; upper bits are not retained.

	    ---- -000 (00) 8-bit tile number
	    ---- -001 (01) 16-bit tile number
	    ---- -010 (02) 16-bit 8-byte-aligned address
	    ---- -011 (03) 16-bit byte address
	    ---- -100 (04) 24-bit byte address
	    ---- -101 (05) 16-bit tile number, extended segment
	    ---- -110 (06) 16-bit 8-byte-aligned address, extended segment
	    ---- -111 (07) 24-bit 8-byte-aligned address
	*/
	// epo_bowl and tak_hamr write 0x84, selecting canonical mode 4.
	m_sprite_address_mode = data & 0x07;
}

// The Text Array / Memory Emulator acts as a memory area that you can point the tilemap sources at to get a fixed pattern of data
u8 xavix_video_device::text_array_r(offs_t offset)
{
	if (offset < 0x400)
	{
		// Swap the address nibbles, then invert either nibble according to the page.
		return bitswap<8>(offset, 3, 2, 1, 0, 7, 6, 5, 4)
				^ (BIT(offset, 8) ? 0x0f : 0x00) ^ (BIT(offset, 9) ? 0xf0 : 0x00);
	}
	if (offset < 0x1000)
		return m_text_array[(offset >> 10) - 1];

	return 0xff;
}

void xavix_video_device::text_array_w(offs_t offset, u8 data)
{
	// The four fixed-pattern pages ignore writes.
	if ((offset >= 0x400) && (offset < 0x1000))
		m_text_array[(offset >> 10) - 1] = data;
}


u8 xavix_video_device::read_low_bus_bypass(u32 address)
{
	address &= 0x7fff;

	if ((address >= 0x4000) && (address < 0x5000))
	{
		address &= 0xfff;
		return text_array_r(address);
	}
	else if ((address >= 0x6000) && (address < 0x6800))
	{
		address &= 0x7ff;
		return m_spriteram[address];
	}
	else if ((address >= 0x6800) && (address < 0x6900))
	{
		address &= 0xff;
		return m_graphics_palram_sh[address];
	}
	else if ((address >= 0x6900) && (address < 0x6a00))
	{
		address &= 0xff;
		return m_graphics_palram_l[address];
	}
	else if ((address >= 0x6a00) && (address < 0x6a20))
	{
		address &= 0x1f;
		return m_segment_regs[address];
	}
	// Super XaviX bitmap palette RAM at 0x6c00-0x6dff is on the shared internal bus.
	// TODO: Model its latched reads and timing when native renderer/DMA use is observed.

	return 0x00;
}

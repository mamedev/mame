// license:BSD-3-Clause
// copyright-holders:David Haywood, MagikalUnicorn
/***************************************************************************

    Graphics decoding and rendering for the XaviX video devices. Implements
    tile and sprite decoding, layer composition, bitmap mixing and RGB output,
    including palette transparency, colour effects and noise. Graphics data
    is fetched through the configured bus callbacks.

    TODO:
    - Retain intra-line arena gate history for live writes and counter
      reloads.
    - Account for retained picture-row phase in progressive modes and
      per-line counter changes.
    - Verify output delay, sampling phase and NTSC/PAL bitmap origins on
      hardware.
    - Determine the purpose of inline-header bit 1 and zero low-page
      tile-source reads.
    - Recheck the remaining arena-boundary discrepancies in Monster
      Truck, Snowboard and Taito Nostalgia.

***************************************************************************/

#include "emu.h"
#include "xavix_video.h"

#include "drawgfx.h"

#include "util/multibyte.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <numbers>

#define LOG_TILEMAP       (1U << 1)
#define LOG_INLINE_HEADER (1U << 2)
#define LOG_SPRITES       (1U << 3)

#define VERBOSE 0
#include "logmacro.h"


u8 xavix_video_device::packed_reader::read(unsigned bits)
{
	assert(bits >= 1 && bits <= 8);
	// Retain unused low-first bits, fetching each mapped byte only when needed.
	if (m_bits < bits)
	{
		m_data |= u32(m_read(m_address++ & 0x07ffffff)) << m_bits;
		m_bits += 8;
	}
	u8 const data = m_data & ((1U << bits) - 1);
	m_data >>= bits;
	m_bits -= bits;
	return data;
}

//-------------------------------------------------
//  Colour Conversion and Output
//-------------------------------------------------

u16 xavix_video_device::replace_pen_lightness(u16 palette_data, u16 lightness)
{
	lightness &= 0x3f;
	palette_data &= ~0x1f00;
	palette_data &= ~0x8000;
	palette_data |= (lightness & 0x3e) << 7;
	palette_data |= (lightness & 0x01) << 15;
	return palette_data;
}

u16 xavix_video_device::replace_pen_saturation(u16 palette_data, u16 saturation)
{
	saturation &= 0x0f;
	palette_data &= ~0x00e0;
	palette_data &= ~0x4000;
	palette_data |= (saturation & 0x0e) << 4;
	palette_data |= (saturation & 0x01) << 14;
	return palette_data;
}

u16 xavix_video_device::replace_pen_hue(u16 palette_data, u16 hue)
{
	hue &= 0x3f;
	palette_data &= ~0x001f;
	palette_data &= ~0x2000;
	palette_data |= (hue & 0x3e) >> 1;
	palette_data |= (hue & 0x01) << 13;
	return palette_data;
}

u8 xavix_video_device::pen_lightness(u16 palette_data)
{
	return bitswap<6>(palette_data, 12, 11, 10, 9, 8, 15);
}

u8 xavix_video_device::pen_saturation(u16 palette_data)
{
	return bitswap<4>(palette_data, 7, 6, 5, 14);
}

u8 xavix_video_device::pen_hue(u16 palette_data)
{
	return bitswap<6>(palette_data, 4, 3, 2, 1, 0, 13);
}

const std::array<std::array<double, 2>, 48> xavix_video_device::HUE_IQ = []
{
	std::array<std::array<double, 2>, 48> result;
	for (unsigned hue = 0; hue < result.size(); ++hue)
	{
		double const angle = (18.0 - 7.5 * hue) * (std::numbers::pi / 180.0);
		result[hue] = { std::cos(angle), std::sin(angle) };
	}
	return result;
}();

rgb_t xavix_video_device::pen_color(u16 palette_data)
{
	u8 const lightness = pen_lightness(palette_data);
	u8 const saturation = pen_saturation(palette_data);
	u8 const hue = pen_hue(palette_data) % HUE_IQ.size();

	double const y = lightness / 47.0;
	double const chroma = (2.0 * saturation) / 47.0;
	auto const [i_scale, q_scale] = HUE_IQ[hue];
	double const i = chroma * i_scale;
	double const q = chroma * q_scale;
	double const r = y + 0.9563 * i + 0.6210 * q;
	double const g = y - 0.2721 * i - 0.6474 * q;
	double const b = y - 1.1070 * i + 1.7046 * q;

	auto const component = [] (double value) { return u8(255.0 * std::clamp(value, 0.0, 1.0)); };
	return rgb_t(component(r), component(g), component(b));
}

rectangle xavix_video_device::arena_effect_area() const
{
	/* All generations gate post-composition colour effects, not individual layers.
	   Black replacement supplies rad_mtrk/rad_snow borders, taitons2 Slap Fight
	   transitions and taitons1 Gladiator portrait masks. Hardware footage shows
	   asymmetric rad_mtrk borders and partly cut taitons1 Gladiator portraits.
	   With settled blanking, the comparisons delimit the inverted gate within
	   picture Y16-239. A missed trailing comparison retains the interval to blanking.
	   Equal endpoints have zero width here; their fractional output pulse is not
	   modelled in RGB.
	   TODO: Retain intra-line gate history for live writes and active-display counter reloads. */

	bool const side = bool(m_arena_ctrl & ARENA_SIDE);
	int const first = side ? m_arena_end : m_arena_start;
	int const second = side ? m_arena_start : m_arena_end;
	auto const [xoffset, yoffset] = counter_offset();
	// The fallback follows picture output, not the counter origin or the wider output raster.
	int const end = (second >= first) ? (second * m_video_hres_scale - xoffset)
			: (TILEMAP_WRAP_WIDTH * m_video_hres_scale - picture_offset().first);
	return rectangle(first * m_video_hres_scale - xoffset, end - 1,
			PICTURE_FIRST_ROW - yoffset, PICTURE_END_ROW - 1 - yoffset);
}

u16 xavix_video_device::apply_color_effect(u16 data, u8 noise) const
{
	u8 const control = m_color_effect[2];
	u8 hue = pen_hue(data);
	u8 saturation = pen_saturation(data);
	u8 lightness = pen_lightness(data);

	if (control & COLOR_EFFECT_HUE)
		hue = ((m_color_effect[0] & 0x1f) << 1) | BIT(m_color_effect[1], 5);
	if (control & COLOR_EFFECT_INVERT)
	{
		u8 const section = hue >> 3;
		hue = (((section + ((section < 3) ? 3 : 5)) & 7) << 3) | (hue & 7);
	}

	if (control & COLOR_EFFECT_SATURATION)
		saturation = ((m_color_effect[0] >> 5) << 1) | BIT(m_color_effect[1], 6);
	if (control & COLOR_EFFECT_DIM)
		saturation >>= 1;

	if (control & COLOR_EFFECT_LIGHTNESS)
		lightness = ((m_color_effect[1] & 0x1f) << 1) | BIT(m_color_effect[1], 7);
	if (control & COLOR_EFFECT_DIM)
		lightness = (lightness >> 1) & 0x0f;
	if (control & COLOR_EFFECT_INVERT)
		lightness = (~lightness - 16) & 0x3f;
	lightness ^= noise & (control >> 5);

	data = replace_pen_hue(data, hue);
	data = replace_pen_saturation(data, saturation);
	return replace_pen_lightness(data, lightness);
}

void xavix_video_device::draw_output(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (cliprect.empty())
		return;

	pen_t const *const palette = this->palette().pens();
	bool const side = bool(m_arena_ctrl & ARENA_SIDE);
	rectangle const area = arena_effect_area() & cliprect;
	if (!m_color_effect[2] || (!side && area.empty()))
	{
		for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
			draw_scanline16(bitmap, cliprect.min_x, y, cliprect.width(), &m_pen_bitmap.pix(y, cliprect.min_x), palette);
		return;
	}

	// Each row has at most three palette spans; outside the rectangle the gate is preset.
	std::array<int, 4> const edges{
		cliprect.min_x,
		area.empty() ? cliprect.min_x : area.min_x,
		area.empty() ? cliprect.min_x : area.max_x + 1,
		cliprect.max_x + 1
	};
	pen_t const *const outside = side ? m_color_effect_palette.get() : palette;
	pen_t const *const inside = side ? palette : m_color_effect_palette.get();
	auto const span_palette = [&area, inside, outside](unsigned span, int y)
	{
		return ((span == 1) && (y >= area.min_y) && (y <= area.max_y)) ? inside : outside;
	};

	unsigned const entries = this->palette().entries();
	if (!m_color_effect_palette_valid)
	{
		unsigned const noise_levels = (m_color_effect[2] >> 5) ? COLOR_EFFECT_NOISE_LEVELS : 1;
		for (unsigned noise = 0; noise < noise_levels; ++noise)
		{
			for (unsigned pen = 0; pen < entries; ++pen)
				m_color_effect_palette[noise * entries + pen] = pen_color(apply_color_effect(palette_data(pen), noise));
		}
		m_color_effect_palette_valid = true;
	}

	sync_video_noise();
	if (!(m_color_effect[2] >> 5))
	{
		for (unsigned span = 0; span < edges.size() - 1; ++span)
		{
			if (edges[span] == edges[span + 1])
				continue;
			for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
				draw_scanline16(bitmap, edges[span], y, edges[span + 1] - edges[span], &m_pen_bitmap.pix(y, edges[span]), span_palette(span, y));
		}
		return;
	}

	u32 state = m_video_noise_state;
	// Retain fractional master clocks scaled by ATTOSECONDS_PER_SECOND.
	u64 clock_phase = 0;
	u64 const scaled_clocks_per_pixel = u64(screen.pixel_period().as_attoseconds()) * clock();

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		u32 *const pixels = &bitmap.pix(y);
		u16 const *const pens = &m_pen_bitmap.pix(y);

		for (unsigned span = 0; span < edges.size() - 1; ++span)
		{
			pen_t const *const colors = span_palette(span, y);
			unsigned const noise_stride = (colors == palette) ? 0 : entries;
			for (int x = edges[span]; x < edges[span + 1]; ++x)
			{
				u8 const noise = bitswap<COLOR_EFFECT_NOISE_BITS>(state, 11, 7, 0);
				pixels[x] = colors[noise * noise_stride + pens[x]];

				clock_phase += scaled_clocks_per_pixel;
				unsigned const clocks = clock_phase / ATTOSECONDS_PER_SECOND;
				clock_phase %= ATTOSECONDS_PER_SECOND;
				state = advance_video_noise_state(state, clocks);
			}
		}
	}
}

//-------------------------------------------------
//  Graphics Addressing and Tilemaps
//-------------------------------------------------

u32 xavix_video_device::classic_segment_base(unsigned segment) const
{
	return u32(get_u16le(&m_segment_regs[segment * 2])) << 8;
}

u32 xavix_video_device::graphics_segment_base(unsigned segment) const
{
	return classic_segment_base(segment);
}

u32 superxavix_video_device::graphics_segment_base(unsigned segment) const
{
	u32 const base = classic_segment_base(segment);
	// domdance uses both tables concurrently, while ndpmj and ban_bkgj use the
	// extended entry with the ordinary one clear.
	return base ? base : get_u32le(&m_extended_segment_regs[segment * 4]);
}

u32 xavix_video_device::extended_graphics_segment_base(unsigned segment) const
{
	u16 const segment_data = get_u16le(&m_segment_regs[segment * 2]);
	return (u32(segment_data & 0x07) << 24) | (u32(segment_data >> 3) << 11);
}

u32 superxavix_video_device::extended_graphics_segment_base(unsigned segment) const
{
	return get_u32le(&m_extended_segment_regs[segment * 4]) & 0x07fff800;
}

u32 xavix_video_device::graphics_address(graphics_source source, u8 mode, u32 entry, unsigned tile_bytes) const
{
	bool const sprite = source == graphics_source::SPRITE;

	switch (mode)
	{
	case 0x00: // 8-bit tile number
	case 0x01: // 16-bit tile number
		// Tile-number addressing scales by tile dimensions and colour depth.
		entry = entry * tile_bytes + (sprite ? graphics_segment_base(0) : classic_segment_base(0));
		return sprite ? (entry & 0x07ffffff) : entry;

	case 0x02: // Segmented address in eight-byte units
	case 0x03: // Segmented byte address; epo_stad uses this for its dialog backdrop after repeated strikes.
	{
		unsigned const shift = (mode == 0x02) ? 16 : 12;
		if (mode == 0x02)
			entry *= 8;
		unsigned const segment = (entry >> shift) & 0x0f;
		// Segmented sprites use the classic table; tilemaps also allow the extended fallback.
		return (entry & ((1U << shift) - 1)) + (sprite ? classic_segment_base(segment) : graphics_segment_base(segment));
	}

	case 0x04: // 24-bit byte address
		return entry;

	case 0x05: // Extended-segment tile number
		// ban_bkgj and xavgolf exercise the extended tile-number mode.
		return (entry * tile_bytes + extended_graphics_segment_base(0)) & 0x07ffffff;

	case 0x06: // Extended-segment address in eight-byte units
		return (extended_graphics_segment_base(BIT(entry, 13, 3)) + ((entry & 0x1fff) << 3)) & 0x07ffffff;

	case 0x07: // 24-bit address in eight-byte units
		// anpanmdx exercises the 24-bit eight-byte-aligned path.
		return entry * 8;
	}
	return entry;
}

void xavix_video_device::decode_inline_header(tile_data &tile)
{
	u8 header = m_video_read_cb(tile.address++ & 0x07ffffff);
	// Only the first command supplies palette and orientation attributes.
	tile.palette = header >> 4;
	// Bit 0 enables the horizontal and vertical orientation fields in bits 2 and 3.
	bool const flip_x = BIT(header, 0) && BIT(header, 2);
	bool const flip_y = BIT(header, 0) && BIT(header, 3);
	tile.layout.flip_x ^= flip_x;
	tile.layout.flip_y ^= flip_y;
	// TODO: Determine the purpose of bit 1; both values currently have the same orientation.
	LOGMASKED(LOG_INLINE_HEADER, "[inline] first=%02x palette=%x flip=%d,%d\n", header, tile.palette, flip_x, flip_y);

	// Pixel data starts after a command with low nibble 0x06.
	while ((header & 0x0f) != INLINE_HEADER_END)
	{
		header = m_video_read_cb(tile.address++ & 0x07ffffff);
		LOGMASKED(LOG_INLINE_HEADER, "[inline] command=%02x\n", header);
	}
}

void xavix_video_device::draw_tilemap(const rectangle &cliprect, int layer)
{
	u8 const *const regs = layer ? m_tilemap2_regs : m_tilemap1_regs;
	// Bit 7 gates the entire layer.
	if (!(regs[TILEMAP_CTRL] & TILEMAP_ENABLE))
		return;

	u8 const format = regs[TILEMAP_FORMAT];
	u8 const mode = regs[TILEMAP_CTRL] & 0x07;
	u8 const attribute_source = regs[TILEMAP_CTRL] & (TILEMAP_ATTRIBUTES | TILEMAP_INLINE_HEADER);
	bool const use_inline_header = attribute_source == TILEMAP_INLINE_HEADER;

	tile_data const defaults{
		.address = 0,
		.bpp = ((format & 0x0e) >> 1) + 1,
		.width = 8 << BIT(format, 4),
		.height = 8 << BIT(format, 5),
		.palette = regs[TILEMAP_PALETTE_PRIORITY] >> 4,
		.priority = regs[TILEMAP_PALETTE_PRIORITY] & 0x0f,
		.layout = decode_pixel_layout(BIT(format, 6, 2))
	};
	int const map_columns = TILEMAP_WRAP_WIDTH / defaults.width;

	int const scroll_x = regs[TILEMAP_SCROLL_X];
	int const scroll_y = regs[TILEMAP_SCROLL_Y];
	auto const [xoffset, yoffset] = picture_offset();
	LOGMASKED(LOG_TILEMAP, "[tilemap] layer=%d pages=%02x/%02x/%02x format=%02x scroll=%02x,%02x palette=%02x control=%02x\n",
			layer, regs[TILEMAP_PLANE_LOW], regs[TILEMAP_PLANE_MIDDLE], regs[TILEMAP_PLANE_HIGH], format,
			regs[TILEMAP_SCROLL_X], regs[TILEMAP_SCROLL_Y], regs[TILEMAP_PALETTE_PRIORITY], regs[TILEMAP_CTRL]);

	for (int line = cliprect.min_y; line <= cliprect.max_y; ++line)
	{
		int const source_line = (line + yoffset) & 0x01ff;
		int const source_y = (source_line + scroll_y) & (TILEMAP_WRAP_HEIGHT - 1);
		int const tile_row = source_y / defaults.height;
		int const tile_line = source_y & (defaults.height - 1);
		// No new tile row is fetched at picture Y240 or below, but a scrolled tile
		// fetched above the boundary can finish.  Test its top before scroll wrapping.
		if (source_line - tile_line >= PICTURE_END_ROW)
			continue;

		// rad_mtrk test mode points the tile planes at the fixed text-array window
		// supplied by the memory map rather than ordinary RAM.
		for (int tile_column = 0; tile_column < map_columns; tile_column++)
		{
			int const tile_index = (tile_row * map_columns) + tile_column;
			auto const read_plane = [this, tile_index](u8 page)
			{
				offs_t const address = ((u16(page) << 8) + tile_index) & 0x7fff;
				return m_video_read_cb(address);
			};

			u32 tile = 0;
			// Every observed zero low-page selector is part of a transient blank setup with all tile-plane
			// selectors zero.  Treat it as a clear byte; whether hardware instead reads page zero remains unknown.
			if (regs[TILEMAP_PLANE_LOW] != 0x00)
				tile = read_plane(regs[TILEMAP_PLANE_LOW]);

			// The 8-bit tile-number mode has no middle tile plane.
			if (mode != 0x00)
				tile |= u32(read_plane(regs[TILEMAP_PLANE_MIDDLE])) << 8;

			// Non-header 24-bit modes use the third plane for the address high byte.
			if (!use_inline_header && ((mode == 0x04) || (mode == 0x07)))
				tile |= u32(read_plane(regs[TILEMAP_PLANE_HIGH])) << 16;

			// Tile entry zero is empty in every observed mode, and inline mode has no
			// valid data packet there. A separate solid-fill meaning has not been found.
			if (tile == 0)
				continue;

			tile_data data = defaults;
			if (!use_inline_header)
			{
				data.address = graphics_address(graphics_source::TILEMAP, mode, tile, data.height * data.width * data.bpp / 8);

				// Attribute modes take palette and priority from the third tile plane.
				if (attribute_source == TILEMAP_ATTRIBUTES)
				{
					u8 const attributes = read_plane(regs[TILEMAP_PLANE_HIGH]);
					data.palette = attributes >> 4;
					data.priority = attributes & 0x0f;
				}
			}
			else
			{
				LOGMASKED(LOG_INLINE_HEADER, "[inline] layer=%d entry=%04x position=%d,%d\n",
						layer, tile, ((tile_column * data.width) + scroll_x) & 0xff, ((tile_row * data.height) + scroll_y) & 0xff);

				if (mode == 0x04)
				{
					// multiplt uses 24-bit inline packets for its 3D driving sections.
					data.address = tile | (u32(read_plane(regs[TILEMAP_PLANE_HIGH])) << 16);
				}
				else
				{
					// rad_mtrk uses 16-bit segmented inline packets.
					unsigned const segment = (tile >> 12) & 0x0f;
					data.address = (tile & 0x0fff)
							+ (u32(get_u16le(&m_segment_regs[segment * 2])) << 8);
				}

				decode_inline_header(data);
			}

			draw_tile_line(cliprect, data, (tile_column * data.width) + scroll_x, line, tile_line, xoffset);
			// Tilemap X positions wrap at 256 pixels.
			draw_tile_line(cliprect, data, ((tile_column * data.width) + scroll_x) - TILEMAP_WRAP_WIDTH, line, tile_line, xoffset);
		}
	}
}

//-------------------------------------------------
//  Pixel Layout and Positioning
//-------------------------------------------------

auto xavix_video_device::decode_pixel_layout(u8 flags) const -> pixel_layout
{
	// Earlier generations use conventional horizontal and vertical orientation.
	return {
		.flip_x = bool(BIT(flags, 0)),
		.flip_y = bool(BIT(flags, 1)),
		.pixel_phase = 0,
		.pixel_width = m_video_hres_scale
	};
}

auto superxavix_video_device::decode_pixel_layout(u8 flags) const -> pixel_layout
{
	// Graphics mode bit 0 changes layouts 2 and 3 from flips to even/odd output columns.
	if (BIT(m_graphics_mode, 0) && BIT(flags, 1))
	{
		return {
			.flip_x = false,
			.flip_y = false,
			.pixel_phase = BIT(flags, 0) * (PAIRED_PIXEL_CLOCKS / OUTPUT_PIXEL_CLOCKS),
			.pixel_width = PAIRED_PIXEL_CLOCKS / OUTPUT_PIXEL_CLOCKS
		};
	}

	return xavix_video_device::decode_pixel_layout(flags);
}

int xavix_video_device::aligned_output_position(int clocks, int alignment_clocks, int sample_clocks)
{
	// Round towards the next alignment boundary, including negative positions.
	int const position = clocks / alignment_clocks + int(clocks % alignment_clocks > 0);
	return position * (alignment_clocks / sample_clocks);
}

bool superxavix_video_device::horizontal_reload_in_blanking(counter_config const &config) const
{
	u16 const reference = config.horizontal_reference;
	int const position = reference & 0x07ff;
	int const field_right = 0x0500 | m_video_timing_regs[TIMING_FIELD_RIGHT];
	return BIT(reference, 11) && (position <= (config.horizontal_period >> 1)) && (position >= field_right);
}

int superxavix_video_device::output_clock_offset() const
{
	// Preserve the existing placement in the later paired-column mode until
	// its output sampling is established independently.
	if (BIT(m_graphics_mode, 0))
		return 0;

	counter_config const config = counter_parameters();
	if (!horizontal_reload_in_blanking(config))
		return 0;

	int const field_left = get_u16le(&m_video_timing_regs[TIMING_FIELD_LEFT]);
	// The first unblanked colour latch consumes the preceding even picture
	// clock. Retain every two-clock sample from this common aperture origin.
	return BIT(config.left + (config.horizontal_period >> 1) - (config.horizontal_reference & 0x07ff) + field_left, 0);
}

int superxavix_video_device::output_position(int clocks) const
{
	// The paired-column path keeps its former four-clock boundaries, doubled
	// onto this finer output grid. Ordinary output retains half-column phase.
	int const alignment_clocks = BIT(m_graphics_mode, 0) ? PAIRED_PIXEL_CLOCKS : OUTPUT_PIXEL_CLOCKS;
	return aligned_output_position(clocks, alignment_clocks, OUTPUT_PIXEL_CLOCKS);
}

int superxavix_video_device::output_origin(int clocks)
{
	// Stream-origin alignment shared by both graphics modes.
	return aligned_output_position(clocks, OUTPUT_ORIGIN_CLOCKS, OUTPUT_PIXEL_CLOCKS);
}

std::pair<int, int> xavix_video_device::counter_offset() const
{
	// Counter comparisons use the fixed eight-clock raster reference.
	counter_config const defaults = xavix_video_device::counter_parameters();
	int const left = util::sext(defaults.left, 12);
	int const xoffset = -aligned_output_position(PICTURE_OUTPUT_DELAY_CLOCKS - left, PICTURE_PIXEL_CLOCKS, PICTURE_PIXEL_CLOCKS);
	return { xoffset, util::sext(defaults.top, 9) };
}

std::pair<int, int> superxavix_video_device::counter_offset() const
{
	// Counter comparisons and picture pixels share the local bitmap origin.
	return picture_offset();
}

std::pair<int, int> xavix_video_device::picture_offset() const
{
	// Fixed picture origin on the rounded eight-clock output grid.
	// TODO: Verify the provisional picture delay on earlier chips.
	counter_config const defaults = xavix_video_device::counter_parameters();
	int const left = util::sext(defaults.left, 12);
	int const xoffset = -aligned_output_position(EARLY_PICTURE_OUTPUT_DELAY_CLOCKS - left, PICTURE_PIXEL_CLOCKS, PICTURE_PIXEL_CLOCKS);
	return { xoffset, util::sext(defaults.top, 9) };
}

std::pair<int, int> superxavix_video_device::picture_offset() const
{
	bool const pal = m_video_standard == video_standard::PAL;
	// TODO: Account for retained picture-row phase in progressive modes.
	int const field_offset = BIT(m_video_timing.half_lines, 0) ? int(m_video_field_phase) - int(pal) : 0;
	counter_config const defaults = xavix_video_device::counter_parameters();
	int const default_top = util::sext(defaults.top, 9);
	counter_config const config = counter_parameters();
	int const field_left = get_u16le(&m_video_timing_regs[TIMING_FIELD_LEFT]);

	// Position picture output relative to the programmed blanking edge.
	// A late second-half reload leaves the remaining clocks before the next line
	// added to the picture preload. Display and palette output follow eight clocks later.
	// TODO: Sample retained counters per line for active-display or unreachable
	// references and write-to-reload latching. Verify the output delay and
	// sampling phase in high-resolution modes.
	if (!horizontal_reload_in_blanking(config))
		return { 0, field_offset };

	int const remaining_clocks = (config.horizontal_period >> 1) - (config.horizontal_reference & 0x07ff);
	int const xclocks = util::sext(config.left + remaining_clocks + field_left + output_clock_offset() - PICTURE_OUTPUT_DELAY_CLOCKS, 12);
	int yoffset = field_offset;
	if (config.vertical_reference < defaults.vertical_reference - default_top + PICTURE_FIRST_ROW)
		yoffset += util::sext(config.top - default_top + defaults.vertical_reference - config.vertical_reference, 9);

	// Round the source offset down so each edge starts at the next output sample.
	return { -output_origin(-xclocks), yoffset };
}

std::pair<int, int> xavix_video_device::render_offset() const
{
	// Earlier chips render directly in screen raster coordinates.
	return { 0, 0 };
}

std::pair<int, int> superxavix_video_device::render_offset() const
{
	// Place the local picture bitmap within the screen raster.
	bool const pal = m_video_standard == video_standard::PAL;
	counter_config const defaults = xavix_video_device::counter_parameters();
	// Picture Y reloads at the end of the preceding raster row.
	int const row = defaults.vertical_reference - util::sext(defaults.top, 9) + 1
			- int(pal && BIT(m_video_timing.half_lines, 0));
	return { screen().visible_area().min_x, row - m_video_timing.vertical_origin };
}

//-------------------------------------------------
//  Sprites and Tile Rows
//-------------------------------------------------

void xavix_video_device::draw_sprites(const rectangle &cliprect)
{
	u8 const mode = m_sprite_address_mode;
	auto const [xoffset, yoffset] = picture_offset();
	LOGMASKED(LOG_SPRITES, "[sprite] address mode=%02x\n", mode);

	/* attribute 0 bits
	   pppp bbbx    p = palette, b = bpp, x = horizontal position high-bit mirror

	   attribute 1 bits
	   zzzz ssYX    z = priority, s = size, YX = sprite layout
	*/

	// Decode RAM descriptors once for this partial update, without caching across writes.
	std::array<sprite_data, SPRITE_COUNT> sprites;
	for (int i = 0; i < SPRITE_COUNT; ++i)
	{
		// Every mode uses the low address plane, all but mode 0 use the middle plane,
		// and the two 24-bit modes also use the high plane.
		u32 entry = m_spriteram[SPRITE_ADDRESS_LOW + i];
		if (mode != 0x00)
			entry |= u32(m_spriteram[SPRITE_ADDRESS_MIDDLE + i]) << 8;
		if ((mode == 0x04) || (mode == 0x07))
			entry |= u32(m_spriteram[SPRITE_ADDRESS_HIGH + i]) << 16;

		u8 const attr0 = m_spriteram[SPRITE_ATTR0 + i];
		u8 const attr1 = m_spriteram[SPRITE_ATTR1 + i];
		// Taito Nostalgia logo sprites use 0x84/0x80/0x88/0x8c here. rad_mtrk
		// uses 0xec/0xfc/0xdc/0x4c/0x5c/0x6c; its last six in-game entries are
		// 0x00/0x00/0xf0/0xf0/0xf0/0xf0 and may be radar-related.
		int const bpp = ((attr0 & 0x0e) >> 1) + 1;
		int const width = BIT(attr1, 2) ? 16 : 8;
		int const height = BIT(attr1, 3) ? 16 : 8;

		// Positions describe the sprite centre; Y is stored in one's-complement form.
		int const xpos = m_spriteram[SPRITE_X + i] | ((m_spriteram[SPRITE_X_HIGH + i] & 1) << 8);
		sprites[i] = {
			.tile = {
				.address = graphics_address(graphics_source::SPRITE, mode, entry, width * height * bpp / 8),
				.bpp = bpp,
				.width = width,
				.height = height,
				.palette = attr0 >> 4,
				.priority = attr1 >> 4,
				.layout = decode_pixel_layout(attr1)
			},
			.x = (xpos ^ 0x100) - 0x80 - (width / 2),
			.y = s8(m_spriteram[SPRITE_Y + i] ^ 0xff) + 129 - (height / 2)
		};
	}

	// Keep pixel fetches in scanline order: graphics can come from mapped memory.
	for (int line = cliprect.min_y; line <= cliprect.max_y; ++line)
	{
		int const source_line = (line + yoffset) & 0x01ff;
		// Sprite evaluation is limited to picture rows 16-239, independently of
		// the output field's row origin.
		if ((source_line < PICTURE_FIRST_ROW) || (source_line >= PICTURE_END_ROW))
			continue;

		// Priority comes from attr1. Lower-numbered equal-priority sprites win;
		// epo_golf requires this order to assemble the hole correctly.
		for (int i = SPRITE_COUNT - 1; i >= 0; --i)
		{
			auto const &sprite = sprites[i];
			int const tile_line = source_line - sprite.y;
			if ((tile_line < 0) || (tile_line >= sprite.tile.height))
				continue;

			u8 const ypos = m_spriteram[SPRITE_Y + i];
			if ((tile_line == 0) && (ypos != 0x81) && (ypos != 0x80) && (ypos != 0x00))
			{
				LOGMASKED(LOG_SPRITES, "[sprite] index=%02x y=%02x attr=%02x/%02x x=%02x address=%02x/%02x/%02x\n",
						i, ypos, m_spriteram[SPRITE_ATTR0 + i], m_spriteram[SPRITE_ATTR1 + i], m_spriteram[SPRITE_X + i],
						m_spriteram[SPRITE_ADDRESS_LOW + i], m_spriteram[SPRITE_ADDRESS_MIDDLE + i], m_spriteram[SPRITE_ADDRESS_HIGH + i]);
			}

			draw_tile_line(cliprect, sprite.tile, sprite.x, line, tile_line, xoffset);
		}
	}
}

void xavix_video_device::draw_tile_line(const rectangle &cliprect, const tile_data &tile, int xpos, int ypos, int line, int xoffset)
{
	if ((ypos > cliprect.max_y) || (ypos < cliprect.min_y))
		return;

	int const first_x = xpos * m_video_hres_scale + tile.layout.pixel_phase - xoffset;
	int const last_x = first_x + (tile.width - 1) * m_video_hres_scale + tile.layout.pixel_width - 1;
	if ((first_x > cliprect.max_x) || (last_x < cliprect.min_x))
		return;

	// Palette selectors name 16-colour blocks; wider formats ignore unaligned low bits.
	// ttv_lotr selects 0x0f for 5-bpp data and expects the aligned 0x0e block.
	int const palette_base = (tile.palette << 4) & ~((1 << tile.bpp) - 1);
	int const bits_per_tileline = tile.width * tile.bpp;
	int const source_line = tile.layout.flip_y ? (tile.height - 1) - line : line;
	int const bit_offset = source_line * bits_per_tileline;
	offs_t address = tile.address + bit_offset / 8;
	// Row offsets wrap within the low 24 bits, preserving the upper bank.
	address = (tile.address & 0x07000000) | (address & 0x00ffffff);
	packed_reader pixels(tile_read_callback(), address);

	u8 *const priority_line = &screen().priority().pix(ypos);
	u16 *const pen_line = &m_pen_bitmap.pix(ypos);
	int output_x = first_x + (tile.layout.flip_x ? (tile.width - 1) * m_video_hres_scale : 0);
	int const output_step = tile.layout.flip_x ? -m_video_hres_scale : m_video_hres_scale;

	for (int x = 0; x < tile.width; ++x, output_x += output_step)
	{
		u8 const pixel = pixels.read(tile.bpp);
		int const pen = pixel + palette_base;
		// Only the selected transparent pen is discarded before picture priority.
		if (m_transparent_pens[pen >> 4] == pen)
			continue;

		int const first = std::max(output_x, cliprect.min_x);
		int const end = std::min(output_x + tile.layout.pixel_width, cliprect.max_x + 1);
		for (int destination = first; destination < end; ++destination)
		{
			if (tile.priority >= priority_line[destination])
			{
				pen_line[destination] = pen;
				priority_line[destination] = tile.priority;
			}
		}
	}
}

//-------------------------------------------------
//  Field Composition
//-------------------------------------------------

u32 xavix_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Output one field at a time. Interlaced field reconstruction belongs to the display.
	m_pen_bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);
	// Picture-domain clipping precedes placement: shifting the origin must not
	// reveal sprite pixels that lie outside the picture's horizontal range.
	int const xoffset = picture_offset().first;
	rectangle const picture_clip = cliprect & rectangle(-xoffset, TILEMAP_WRAP_WIDTH * m_video_hres_scale - 1 - xoffset, cliprect.min_y, cliprect.max_y);
	if (!picture_clip.empty())
	{
		draw_tilemap(picture_clip, 0);
		draw_tilemap(picture_clip, 1);
		draw_sprites(picture_clip);
	}
	mix_bitmap_layer(screen, cliprect);
	draw_output(screen, bitmap, cliprect);
	return 0;
}

rectangle xavix_video_device::picture_area() const
{
	// Source rows 16-239 occupy raster rows 29-252 (NTSC) or 57-280 (PAL).
	auto const [xoffset, yoffset] = picture_offset();
	return rectangle(-xoffset, TILEMAP_WRAP_WIDTH - 1 - xoffset, PICTURE_FIRST_ROW - yoffset, PICTURE_END_ROW - 1 - yoffset);
}

rectangle superxavix_video_device::picture_area() const
{
	bool const interlaced = BIT(m_video_timing.half_lines, 0);
	bool const pal = m_video_standard == video_standard::PAL;
	// Keep both native picture intervals, including their different edge rows.
	int const bottom = std::min<int>(PICTURE_END_ROW - 1 + int(interlaced && pal), m_video_timing.active_height - 1);
	int const top = std::min(PICTURE_FIRST_ROW - int(interlaced && !pal), bottom);
	rectangle area(0, m_video_timing.active_width - 1, top, bottom);
	auto const [xoffset, yoffset] = render_offset();
	area.offset(xoffset, yoffset);
	return area;
}

u32 superxavix_video_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);
	auto const [xoffset, yoffset] = render_offset();
	rectangle source_clip = cliprect & picture_area();
	source_clip.offset(-xoffset, -yoffset);
	source_clip &= m_output_bitmap.cliprect();
	if (!source_clip.empty())
	{
		xavix_video_device::screen_update(screen, m_output_bitmap, source_clip);
		rectangle destination_clip = source_clip;
		destination_clip.offset(xoffset, yoffset);
		copybitmap(bitmap, m_output_bitmap, 0, 0, xoffset, yoffset, destination_clip);
	}
	return 0;
}

device_delegate<u8 (u32)> &xavix_video_device::tile_read_callback()
{
	return m_video_read_cb;
}

device_delegate<u8 (u32)> &superxavix_video_device::tile_read_callback()
{
	return m_graphics_read_cb;
}

void xavix_video_device::mix_bitmap_layer(screen_device &screen, const rectangle &cliprect)
{
}

void superxavix_video_device::mix_bitmap_layer(screen_device &screen, const rectangle &cliprect)
{
	auto const parameter = [this](unsigned offset) { return get_u16le(&m_bitmap_params[offset]); };

	u8 const mode = m_bitmap_params[BITMAP_CTRL];
	if (!BIT(mode, 0))
		return;

	// Full-frame software uses different vertical counter origins for 525- and
	// 625-line output.  TODO: Verify these standard-dependent origins on hardware.
	int const vertical_origin = (m_video_standard == video_standard::PAL) ? BITMAP_VERTICAL_ORIGIN_PAL : BITMAP_VERTICAL_ORIGIN_NTSC;
	// Keep the source origin independent of the presentation crop.
	int const vertical_offset = PICTURE_FIRST_ROW - vertical_origin;
	// X bounds are bitmap samples; Y bounds are screen rows. Register ends are exclusive.
	rectangle const window(
			parameter(BITMAP_LEFT) & 0x03ff,
			(parameter(BITMAP_RIGHT) & 0x03ff) - 1,
			(parameter(BITMAP_TOP) & 0x01ff) + vertical_offset,
			(parameter(BITMAP_BOTTOM) & 0x01ff) + vertical_offset - 1);
	if (window.empty())
		return;

	u32 const address_top = u32(parameter(BITMAP_ADDRESS_TOP)) << 3;
	u32 const address_left = parameter(BITMAP_ADDRESS_LEFT) & 0x03ff;
	u32 const address_right = parameter(BITMAP_ADDRESS_RIGHT) & 0x03ff;
	// xavgolf and anpanmdx use bitmap bases beyond the normal 24-bit data window.
	u32 const base = u32(parameter(BITMAP_BASE)) << 11;
	u32 const stride = u32(m_bitmap_params[BITMAP_STRIDE]) << 3;
	u8 const resolution = m_bitmap_params[BITMAP_RESOLUTION];
	int const bits_per_pixel = ((mode & 0x0e) >> 1) + 1;
	int const priority = mode >> 4;
	int const pixel_clocks = (resolution >> 4) + 1;
	int const horizontal_origin = get_u16le(&m_video_timing_regs[TIMING_FIELD_LEFT]) + output_clock_offset();
	int const fine_clocks = resolution & 0x0f;
	int const first_clock = fine_clocks + (window.min_x + BITMAP_OUTPUT_DELAY_PIXELS) * pixel_clocks - horizontal_origin;
	int const origin_delta = output_origin(first_clock) - output_position(first_clock);

	int source_width = window.width();
	// The source window can be shorter than the display window.
	if (address_right > address_left)
		source_width = std::min(source_width, int((address_right - address_left) * 8 / bits_per_pixel));
	if (!source_width)
		return;

	int const first_y = std::max(window.min_y, cliprect.min_y);
	int const last_y = std::min(window.max_y, cliprect.max_y);
	for (int y = first_y; y <= last_y; y++)
	{
		int const line = y - window.min_y;
		u32 const row_offset = (address_top + line * stride) & 0x7ffff;
		packed_reader pixels(m_bitmap_read_cb, base + row_offset + address_left);

		u8 const *const priorities = &screen.priority().pix(y);
		u16 *const pens = &m_pen_bitmap.pix(y);
		// The horizontal gate and colour latch add two bitmap pixel periods.
		int source_clock = first_clock;
		for (int source_x = 0; source_x < source_width; ++source_x, source_clock += pixel_clocks)
		{
			u8 const pen = pixels.read(bits_per_pixel);

			// A transparent bitmap always leaves the completed picture visible,
			// including when both inputs are transparent.
			if ((m_bitmap_palram_sh[pen] & 0x1f) >= TRANSPARENT_HUE_MIN)
				continue;

			int const first_x = std::max(output_position(source_clock) + origin_delta, cliprect.min_x);
			int const end_x = std::min(output_position(source_clock + pixel_clocks) + origin_delta, cliprect.max_x + 1);
			for (int x = first_x; x < end_x; ++x)
			{
				// Picture wins equal depths, unless its final colour is transparent.
				if ((priority > priorities[x]) || ((m_graphics_palram_sh[pens[x]] & 0x1f) >= TRANSPARENT_HUE_MIN))
					pens[x] = pen + PALETTE_BANK_ENTRIES;
			}
		}
	}
}

//-------------------------------------------------
//  Colour Effects and Noise
//-------------------------------------------------

u32 xavix_video_device::advance_video_noise_state(u32 state, u64 clocks) const
{
	constexpr unsigned bits = sizeof(decltype(m_video_noise_feedback)) * 8;
	constexpr u32 mask = make_bitmask<u32>(bits);
	u32 const feedback = get_u24le(m_video_noise_feedback);
	auto const clock = [feedback] (u32 value)
	{
		return ((value >> 1) ^ (BIT(value, 0) ? feedback : 0)) & mask;
	};

	state &= mask;
	if (clocks < bits)
	{
		while (clocks--)
			state = clock(state);
		return state;
	}

	using linear_map = std::array<u32, bits>;
	auto const apply = [] (linear_map const &map, u32 value)
	{
		u32 result = 0;

		while (value)
		{
			result ^= map[std::countr_zero(value)];
			value &= value - 1;
		}

		return result;
	};

	// Exponentiate the one-clock state transition over GF(2).
	linear_map jump;
	for (unsigned bit = 0; bit < bits; ++bit)
		jump[bit] = clock(u32(1) << bit);

	while (clocks)
	{
		if (BIT(clocks, 0))
			state = apply(jump, state);

		clocks >>= 1;
		if (clocks)
		{
			linear_map squared;
			for (unsigned bit = 0; bit < bits; ++bit)
				squared[bit] = apply(jump, jump[bit]);
			jump = squared;
		}
	}

	return state;
}

void xavix_video_device::sync_video_noise()
{
	u64 const ticks = machine().time().as_ticks(clock());

	if (ticks > m_video_noise_ticks)
		m_video_noise_state = advance_video_noise_state(m_video_noise_state, ticks - m_video_noise_ticks);

	m_video_noise_ticks = ticks;
}

void xavix_video_device::video_noise_feedback_w(offs_t offset, u8 data)
{
	sync_video_noise();

	if (data != m_video_noise_feedback[offset])
	{
		screen().update_partial(screen().vpos());
		m_video_noise_feedback[offset] = data;
	}
}

void xavix_video_device::color_effect_w(offs_t offset, u8 data)
{
	if (data != m_color_effect[offset])
	{
		screen().update_partial(screen().vpos());
		m_color_effect[offset] = data;
		m_color_effect_palette_valid = false;
	}
}

u8 xavix_video_device::video_noise_state_r(offs_t offset)
{
	u32 state = m_video_noise_state;
	if (machine().side_effects_disabled())
	{
		u64 const ticks = machine().time().as_ticks(clock());
		if (ticks > m_video_noise_ticks)
			state = advance_video_noise_state(state, ticks - m_video_noise_ticks);
	}
	else
	{
		sync_video_noise();
		state = m_video_noise_state;
	}

	return BIT(state, offset * 8, 8);
}

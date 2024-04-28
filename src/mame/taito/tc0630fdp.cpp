#include "emu.h"
#include "tc0630fdp.h"

#include <algorithm>
#include <variant>

constexpr int TAITOF3_VIDEO_DEBUG = 0;

DEFINE_DEVICE_TYPE(TC0630FDP, FDP, "tc0630fdp", "Taito TC0630FDP")

FDP::FDP(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0630FDP, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo, "palette")
	, m_palette(*this, "palette")
	, m_palette_12bit(*this, "palette_12bit")
	, m_spriteram(*this, "spriteram", 0x10000, ENDIANNESS_BIG)
	, m_pfram(*this, "pfram", 0xc000, ENDIANNESS_BIG)
	, m_textram(*this, "textram", 0x2000, ENDIANNESS_BIG)
	, m_charram(*this, "charram", 0x2000, ENDIANNESS_BIG)
	, m_lineram(*this, "lineram", 0x10000, ENDIANNESS_BIG)
	, m_pivotram(*this, "pivotram", 0x10000, ENDIANNESS_BIG)
{
}

void FDP::device_start() {
}

void FDP::device_add_mconfig(machine_config &config) {
	PALETTE(config, m_palette);
	m_palette->set_entries(0x2000);
	set_palette(m_palette); // i guess..
	PALETTE(config, m_palette_12bit);
	m_palette_12bit->set_entries(0x2000);//->set_format(RRRRGGGGBBBBxxxx, 0x2000);
}

#define INTERLEAVE44(start)  4+(start), 0+(start), 12+(start), 8+(start)
#define STEP8_INV(START,STEP)  STEP4_INV((START)+4*(STEP),STEP),STEP4_INV(START,STEP)

static const gfx_layout layout_pivot = {
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ INTERLEAVE44(16), INTERLEAVE44(0) },
	{ STEP8(0,4*8) },
	32*8
};

// the roms connected in parallel to a 48-bit bus:
// [AAAA AAAA AAAA AAAA|BBBB BBBB BBBB BBBB|CCCC CCCC CCCC CCCC]
// each item contains data for 8 pixels (6 bits per pixel)
// [0000 1111 2222 3333|4444 5555 6666 7777|0123 4567 0123 4567]
// note that the upper two bitplanes are stored in a different order than the first four
#define NEXT 48

static const gfx_layout layout_tile_low = {
 	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{
		INTERLEAVE44(0     ), INTERLEAVE44(4*4     ),
		INTERLEAVE44(0+NEXT), INTERLEAVE44(4*4+NEXT),
	},
	{ STEP16(0, NEXT*2) },
	NEXT*2*16
};

static const gfx_layout layout_tile_hi = {
	16,16,
	RGN_FRAC(1,1),
	6,
	{ STEP2_INV(32, 8), 0,0,0,0, },
	{ STEP8_INV(0, 1), STEP8_INV(NEXT, 1) },
	{ STEP16(0, NEXT*2) },
	NEXT*2*16
};

// the roms are connected in parallel to a 24-bit bus:
// [AAAA AAAA|BBBB BBBB|CCCC CCCC]
// each item contains data for 4 pixels (6 bits per pixel)
// [1111 0000|3333 2222|3322 1100] pixels
// [0123 0123|0123 0123|4545 4545] planes
// note that the upper two bitplanes are stored in a different order than the first four
#undef NEXT
#define NEXT 24

static const gfx_layout layout_sprite_low = {
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{
		INTERLEAVE44(NEXT*0),
		INTERLEAVE44(NEXT*1),
		INTERLEAVE44(NEXT*2),
		INTERLEAVE44(NEXT*3),
	},
	{ STEP16(0, NEXT*4) },
	NEXT*4*16,
};

static const gfx_layout layout_sprite_hi = {
	16,16,
	RGN_FRAC(1,1),
	6,
	{ STEP2(16, 1), 0,0,0,0 },
	{ 
		STEP4_INV(NEXT*0, 2),
		STEP4_INV(NEXT*1, 2),
		STEP4_INV(NEXT*2, 2),
		STEP4_INV(NEXT*3, 2),
	},
	{ STEP16(0, NEXT*4) },
	NEXT*4*16,
};

static const gfx_layout bubsympb_sprite_layout = {
	16,16,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(0,6), RGN_FRAC(1,6), RGN_FRAC(2,6), RGN_FRAC(3,6), RGN_FRAC(4,6), RGN_FRAC(5,6) },
	{ STEP16(15,-1) },
	{ STEP16(0,16) },
	16*16
};

GFXDECODE_MEMBER( FDP::gfxinfo )
	GFXDECODE_DEVICE( nullptr,   0, layout_pivot,      0x0000, 0x0400>>4 ) /* Dynamically modified */
	GFXDECODE_DEVICE( nullptr,   0, layout_pivot,      0x0000, 0x0400>>4 ) /* Dynamically modified */
	GFXDECODE_DEVICE( "sprites", 0, layout_sprite_low, 0x1000, 0x1000>>4 ) // low 4bpp of 6bpp sprite data
	GFXDECODE_DEVICE( "tiles",   0, layout_tile_low,   0x0000, 0x2000>>4 ) // low 4bpp of 6bpp tilemap data
	GFXDECODE_DEVICE( "tiles",   0, layout_tile_hi,    0x0000, 0x2000>>4 ) // hi 2bpp of 6bpp tilemap data
	GFXDECODE_DEVICE( "sprites", 0, layout_sprite_hi,  0x1000, 0x1000>>4 ) // hi 2bpp of 6bpp sprite data
GFXDECODE_END

GFXDECODE_MEMBER( FDP::gfx_bubsympb )
	GFXDECODE_DEVICE( nullptr,    0, layout_pivot,                 0x0000, 0x0400>>4) /* Dynamically modified */
	GFXDECODE_DEVICE( nullptr,    0, layout_pivot,                 0x0000, 0x0400>>4) /* Dynamically modified */
	GFXDECODE_DEVICE( "sprites",  0, layout_sprite_low,       0x1000, 0x1000>>4) /* Sprites area (6bpp planar) */
	GFXDECODE_DEVICE( "tiles",    0, layout_tile_low,              0x0000, 0x2000>>4) // low 4bpp of 5bpp tilemap data
	GFXDECODE_DEVICE( "tiles",    0, layout_tile_hi,               0x0000, 0x2000>>4) // hi 1bpp of 5bpp tilemap data
	GFXDECODE_DEVICE( "sprites",  0, layout_sprite_hi,       0x1000, 0x1000>>4) // dummy gfx duplicate for avoid crash
GFXDECODE_END

// the upper 2 bitplanes are interleaved differently than the lower 4, so they have to be merged manually
void FDP::decode_hi(int low, int high, std::unique_ptr<u8[]> &decoded)
{
	gfx_element *gfx_lo = gfx(low);
	gfx_element *gfx_hi = gfx(high);

	// allocate memory for the assembled data
	decoded = std::make_unique<u8[]>(gfx_lo->elements() * gfx_lo->width() * gfx_lo->height());

	// loop over elements
	u8 *dest = decoded.get();
	for (int c = 0; c < gfx_lo->elements(); c++) {
		const u8 *c1base = gfx_lo->get_data(c);
		const u8 *c3base = gfx_hi->get_data(c);

		// loop over height
		for (int y = 0; y < gfx_lo->height(); y++) {
			const u8 *c1 = c1base;
			const u8 *c3 = c3base;

			/* Expand 2bits into 4bits format */
			for (int x = 0; x < gfx_lo->width(); x++)
				*dest++ = (*c1++ & 0xf) | (*c3++ & 0x30);

			c1base += gfx_lo->rowbytes();
			c3base += gfx_hi->rowbytes();
		}
	}

	gfx_lo->set_raw_layout(decoded.get(), gfx_lo->width(), gfx_lo->height(), gfx_lo->elements(), 8 * gfx_lo->width(), 8 * gfx_lo->width() * gfx_lo->height());
	set_gfx(high, nullptr);
}

void FDP::tile_decode()
{
	// all but bubsymphb (bootleg board with different sprite gfx layout), 2mindril (no sprite gfx roms)
	if (gfx(5))
		decode_hi(2, 5, m_decoded_gfx5);
	if (gfx(4))
		decode_hi(3, 4, m_decoded_gfx4);
}

void FDP::map_ram(address_map &map) {
	map(0x00000, 0x0ffff).rw(FUNC(FDP::spriteram_r), FUNC(FDP::spriteram_w));
	map(0x10000, 0x1bfff).rw(FUNC(FDP::pfram_r), FUNC(FDP::pfram_w));
	map(0x1c000, 0x1dfff).rw(FUNC(FDP::textram_r), FUNC(FDP::textram_w));
	map(0x1e000, 0x1ffff).rw(FUNC(FDP::charram_r), FUNC(FDP::charram_w));
	map(0x20000, 0x2ffff).rw(FUNC(FDP::lineram_r), FUNC(FDP::lineram_w));
	map(0x30000, 0x3ffff).rw(FUNC(FDP::pivotram_r), FUNC(FDP::pivotram_w));
}

void FDP::map_control(address_map &map) {
	map(0x00, 0x0f).w(FUNC(FDP::control_0_w));
	map(0x10, 0x1f).w(FUNC(FDP::control_1_w));
}

/******************************************************************************/

template<unsigned Layer>
TILE_GET_INFO_MEMBER(FDP::get_tile_info)
{
	u16 *tilep = &m_pf_data[Layer][tile_index * 2];
	// tile info:
	// [yx?? ddac cccc cccc]
	// yx: x/y flip
	// ?: upper bits of tile number?
	// d: bpp
	// a: blend select
	// c: color

	const u16 palette_code = BIT(tilep[0],  0, 9);
	const u8 blend_sel     = BIT(tilep[0],  9, 1);
	const u8 extra_planes  = BIT(tilep[0], 10, 2); // 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp

	tileinfo.set(3,
			tilep[1],
			palette_code,
			TILE_FLIPYX(BIT(tilep[0], 14, 2)));

	tileinfo.category = blend_sel; // blend value select
	// gfx extra planes and palette code set the same bits of color address
	// we need to account for tilemap.h combining using "+" instead of "|"
	tileinfo.pen_mask = ((extra_planes & ~palette_code) << 4) | 0x0f;
}

TILE_GET_INFO_MEMBER(FDP::get_tile_info_text)
{
	const u16 vram_tile = m_textram[tile_index];
	// text tile info:
	// [yccc cccx tttt tttt]
	// y: y flip
	// c: palette
	// x: x flip
	// t: tile number

	u8 flags = 0;
	if (BIT(vram_tile,  8)) flags |= TILE_FLIPX;
	if (BIT(vram_tile, 15)) flags |= TILE_FLIPY;

	tileinfo.set(0,
			vram_tile & 0xff,
			BIT(vram_tile, 9, 6),
			flags);
}

TILE_GET_INFO_MEMBER(FDP::get_tile_info_pixel)
{
	/* attributes are shared with VRAM layer */
	// convert the index:
	// pixel: [0xxxxxxyyyyy]
	//  text: [?yyyyyxxxxxx]
	const int x = BIT(tile_index, 5, 6);
	int y = BIT(tile_index, 0, 5);
	// HACK: [legacy implementation of scroll offset check for pixel palette mirroring]
	// the pixel layer is 256px high, but uses the palette from the text layer which is twice as long
	// so normally it only uses the first half of textram, BUT if you scroll down, you get
	//   an alternate version of the pixel layer which gets its palette data from the second half of textram.
	// we simulate this using a hack, checking scroll offset to determine which version of the pixel layer is visible.
	// this means we SHOULD dirty parts of the pixel layer, if the scroll or flipscreen changes.. but we don't.
	// (really we should just apply the palette during rendering instead of this ?)
	int y_offs = y * 8 + m_control_1[5];
	if (m_flipscreen)
		y_offs += 0x100; // this could just as easily be ^= 0x100 or -= 0x100
	if ((y_offs & 0x1ff) >= 256)
		y += 32;

	const u16 vram_tile = m_textram[y << 6 | x];

	const int tile = tile_index;
	const u8 palette = BIT(vram_tile, 9, 6);
	u8 flags = 0;
	if (BIT(vram_tile, 8))  flags |= TILE_FLIPX;
	if (BIT(vram_tile, 15)) flags |= TILE_FLIPY;

	tileinfo.set(1, tile, palette, flags);
}

/******************************************************************************/

void FDP::create_tilemaps(bool extend)
{
	m_extend = extend;
	// TODO: extend can be changed at runtime.. how do we deal with that?
	if (m_extend) {
		m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[3] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 32);
		m_tilemap[4] = m_tilemap[5] = m_tilemap[6] = m_tilemap[7] = nullptr;
	} else {
		m_tilemap[0] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[1] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[2] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[3] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[4] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<4>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[5] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<5>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[6] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<6>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
		m_tilemap[7] = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info<7>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	}
	for (int i = 0; i < 8; i++) {
		if (m_tilemap[i])
			m_tilemap[i]->set_transparent_pen(0);
	}

	if (m_extend) {
		m_width_mask = 0x3ff; // 10 bits
		for (int i = 0; i < 4; i++)
			m_pf_data[i] = &m_pfram[(0x2000 * i) / 2];
	} else {
		m_width_mask = 0x1ff; // 9 bits
		for (int i = 0; i < 8; i++)
			m_pf_data[i] = &m_pfram[(0x1000 * i) / 2];
	}
	
	m_vram_layer = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info_text)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);
	m_pixel_layer = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(FDP::get_tile_info_pixel)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
	m_vram_layer->set_transparent_pen(0);
	m_pixel_layer->set_transparent_pen(0);
	
	gfx(0)->set_source_and_total(reinterpret_cast<u8 *>(m_charram.target()), 256);
	gfx(1)->set_source_and_total(reinterpret_cast<u8 *>(m_pivotram.target()), 2048);
	
	m_spritelist = std::make_unique<tempsprite[]>(0x400);
	m_sprite_end = &m_spritelist[0];

	save_item(NAME(m_control_0));
	save_item(NAME(m_control_1));
	
	// Palettes have 4 bpp indexes despite up to 6 bpp data. The unused top bits in the gfx data are cleared later.
	gfx(2)->set_granularity(16);
	gfx(3)->set_granularity(16);

	m_flipscreen = false;
	m_sprite_bank = false;
	m_sprite_trails = false;
}

/******************************************************************************/

u16 FDP::spriteram_r(offs_t offset)
{
	return m_spriteram[offset];
}

void FDP::spriteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

u16 FDP::pfram_r(offs_t offset)
{
	return m_pf_ram[offset];
}

void FDP::pfram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pf_ram[offset]);

	if (m_game_config->extend) {
		if (offset < 0x4000) {
			m_tilemap[offset >> 12]->mark_tile_dirty((offset & 0xfff) >> 1);
		}
	} else {
		if (offset < 0x4000)
			m_tilemap[offset >> 11]->mark_tile_dirty((offset & 0x7ff) >> 1);
	}
}

u16 FDP::textram_r(offs_t offset)
{
	return m_textram[offset];
}

void FDP::textram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_textram[offset]);

	m_vram_layer->mark_tile_dirty(offset);

	// dirty the pixel layer too, since it uses palette etc. from text layer
	// convert the position (x and y are swapped, and the upper bit of y is ignored)
	//  text: [Yyyyyyxxxxxx]
	// pixel: [0xxxxxxyyyyy]
	const int y = BIT(offset, 6, 5);
	const int x = BIT(offset, 0, 6);
	const int col_off = x << 5 | y;

	m_pixel_layer->mark_tile_dirty(col_off);
}

u16 FDP::charram_r(offs_t offset)
{
	return m_charram[offset];
}

void FDP::charram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_charram[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
}

u16 FDP::lineram_r(offs_t offset)
{
	return m_line_ram[offset];
}

void FDP::lineram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_line_ram[offset]);
}

u16 FDP::pivotram_r(offs_t offset)
{
	return m_pivot_ram[offset];
}

void FDP::pivotram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pivot_ram[offset]);
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 4);
}

void FDP::control_0_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_control_0[offset]);
}

void FDP::control_1_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_control_1[offset]);
}

/******************************************************************************/

// line: [latched] line info from previous call, will modify in-place
// y should be called 0->255 for non-flipscreen, 255->0 for flipscreen
void FDP::read_line_ram(f3_line_inf &line, int y)
{
	const auto latched_addr =
		[this, y] (u8 section, u8 subsection) -> offs_t {
			const u16 latches = m_line_ram[(section * 0x200)/2 + y];
			// NOTE: this may actually be computed from the upper byte? i.e.:
			//offs_t base = 0x400 * BIT(latches, 8, 8) + 0x200 * subsection;
			const offs_t base = 0x4000 + 0x1000 * section + 0x200 * subsection;
			if (BIT(latches, subsection + 4))
				return (base + 0x800) / 2 + y;
			else if (BIT(latches, subsection))
				return (base) / 2 + y;
			return 0;
		};

	// 4000 **********************************
	for (const int i : { 2, 3 }) {
		if (const offs_t where = latched_addr(0, i)) {
			const u16 colscroll = m_line_ram[where];
			line.pf[i].colscroll   = colscroll & 0x1ff;
			line.pf[i].alt_tilemap = colscroll & 0x200;
			line.clip[2*(i-2) + 0].set_upper(BIT(colscroll, 12), BIT(colscroll, 13));
			line.clip[2*(i-2) + 1].set_upper(BIT(colscroll, 14), BIT(colscroll, 15));
		}
	}

	// 5000 **********************************
	// renderer needs to adjust clip by -48
	for (const int i : { 0, 1, 2, 3 }) {
		if (const offs_t where = latched_addr(1, i)) {
			const u16 clip_lows = m_line_ram[where];
			line.clip[i].set_lower(BIT(clip_lows, 0, 8), BIT(clip_lows, 8, 8));
		}
	}

	// 6000 **********************************
	if (const offs_t where = latched_addr(2, 0)) { // sprite blend modes, pivot blend select, ?
		// old code called first value "sync register", is special handling necessary?
		const u16 line_6000 = m_line_ram[where];

		line.pivot.blend_select_v = BIT(line_6000, 9);
		line.pivot.pivot_control = BIT(line_6000, 8, 8);
		if (TAITOF3_VIDEO_DEBUG == 1) {
			// arabianm: 0a, scfinals: a2, busymph: a2, recalh: 0c, bubblem: 03/f0
			if (line.pivot.pivot_control & 0b01011101) // check if unknown pivot control bits set
				logerror("unknown 6000 pivot ctrl bits: %02x__ at %04x\n", line.pivot.pivot_control, 0x6000 + y*2);
		}

		for (int sp_group = 0; sp_group < NUM_SPRITEGROUPS; sp_group++) {
			line.sp[sp_group].mix_value &= 0x3fff;
			line.sp[sp_group].mix_value |= BIT(line_6000, sp_group * 2, 2) << 14;
		}
	}
	if (const offs_t where = latched_addr(2, 1)) { // blend values
		const u16 blend_vals = m_line_ram[where];
		for (int idx = 0; idx < 4; idx++) {
			const u8 alpha = BIT(blend_vals, 4 * idx, 4);
			line.blend[idx] = std::min(255, (0xf - alpha) * 32);
		}
	}
	if (const offs_t where = latched_addr(2, 2)) { // mosaic, palette depth effects
		const u16 x_mosaic = m_line_ram[where];

		line.x_sample = 16 - BIT(x_mosaic, 4, 4);

		for (int pf_num = 0; pf_num < NUM_PLAYFIELDS; pf_num++) {
			line.pf[pf_num].x_sample_enable = BIT(x_mosaic, pf_num);
		}

		for (auto &sp : line.sp) {
			sp.x_sample_enable = BIT(x_mosaic, 8);
		}
		line.pivot.x_sample_enable = BIT(x_mosaic, 9);

		line.fx_6400 = (x_mosaic & 0xfc00) >> 8; // palette interpretation [unimplemented]
		if (TAITOF3_VIDEO_DEBUG == 1) {
			// gseeker(intro):40, ringrage/arabianm:30/33, ridingf:30/31, spcinvdj:30, gunlock:78
			if (line.fx_6400 && line.fx_6400 != 0x70) // check if unknown effect bits set
				logerror("unknown 6400 fx bits: %02x__ at %04x\n", line.fx_6400, 0x6400 + y*2);
		}
	}
	if (const offs_t where = latched_addr(2, 3)) { // bg palette? [unimplemented]
		line.bg_palette = m_line_ram[where];
		if (TAITOF3_VIDEO_DEBUG == 1) {
			// gunlock: 0000
			if (line.bg_palette) // check if unknown effect bits set
				logerror("unknown 6600 bg palette: %04x at %04x\n", line.bg_palette, 0x6600 + y*2);
		}
	}

	// 7000 **********************************
	if (const offs_t where = latched_addr(3, 0)) { // ? [unimplemented]
		const u16 line_7000 = m_line_ram[where];
		line.pivot.pivot_enable = line_7000;
		if (TAITOF3_VIDEO_DEBUG == 1) {
			// ridingf/commandw/trstar: c000, gunlock: 0000, recalh: 4000, quizhuhu: 00ff
			// puchicar/ktiger2/gekiridn: 0001, dariusg: 0001 on zone H boss pool effect lines
			if (line_7000) // check if confusing pivot enable bits are set
				logerror("unknown 7000 'pivot enable' bits: %04x at %04x\n", line_7000, 0x7000 + y*2);
		}
	}
	if (const offs_t where = latched_addr(3, 1)) { // pivot layer mix info word
		line.pivot.set_mix(m_line_ram[where]);
	}
	if (const offs_t where = latched_addr(3, 2)) { // sprite clip info, blend select
		const u16 sprite_mix = m_line_ram[where];

		if (TAITOF3_VIDEO_DEBUG == 1) {
			// many: _8__, exceptions: pbobble3/puchicar/pbobble4/gunlock
			const u16 unknown = BIT(sprite_mix, 10, 2);
			if (unknown)
				logerror("unknown sprite mix bits: _%01x__ at %04x\n", unknown << 2, 0x7400 + y*2);
		}

		for (int group = 0; group < NUM_SPRITEGROUPS; group++) {
			line.sp[group].mix_value = (line.sp[group].mix_value & 0xc00f)
					| BIT(sprite_mix, 0, 10) << 4;
			line.sp[group].blend_select_v = BIT(sprite_mix, 12 + group, 1);
		}
	}
	if (const offs_t where = latched_addr(3, 3)) { // sprite priority
		const u16 sprite_prio = m_line_ram[where];
		for (int group = 0; group < NUM_SPRITEGROUPS; group++) {
			line.sp[group].set_prio(BIT(sprite_prio, group * 4, 4));
		}
	}

	// 8000 **********************************
	for (const int i : { 0, 1, 2, 3 }) { // playfield zoom
		if (const offs_t where = latched_addr(4, i)) {
			const u16 pf_scale = m_line_ram[where];
			// y zooms are interleaved
			const int FIX_Y[] = { 0, 3, 2, 1 };
			line.pf[i].x_scale = 256 - BIT(pf_scale, 8, 8);
			line.pf[FIX_Y[i]].y_scale = BIT(pf_scale, 0, 8)<<1;
		}
	}

	// 9000 **********************************
	for (const int i : { 0, 1, 2, 3 }) { // playfield palette addition
		if (const offs_t where = latched_addr(5, i)) {
			const u16 pf_pal_add = m_line_ram[where];
			line.pf[i].pal_add = pf_pal_add * 16;
		}
	}

	// A000 **********************************
	// iiii iiii iiff ffff
	// fractional part is negative (allegedly). i wonder if it's supposed to be inverted instead?
	// and then we just subtract (1<<8) to get almost the same value..
	for (const int i : { 0, 1, 2, 3 }) { // playfield rowscroll
		if (const offs_t where = latched_addr(6, i)) {
			const fixed8 rowscroll = m_line_ram[where] << (8-6);
			line.pf[i].rowscroll = (rowscroll & 0xffffff00) - (rowscroll & 0x000000ff);
			// ((i ^ 0b111111) - 0b111111) << (8-6);
		}
	}

	// B000 **********************************
	for (const int i : { 0, 1, 2, 3 }) { // playfield mix info
		if (const offs_t where = latched_addr(7, i)) {
			line.pf[i].set_mix(m_line_ram[where]);
		}
	}
}

void FDP::get_pf_scroll(int pf_num, fixed8 &reg_sx, fixed8 &reg_sy)
{
	// x: iiii iiii iiFF FFFF
	// y: iiii iiii ifff ffff

	// x scroll is stored as fixed10.6, with fractional bits inverted.
	// we convert this to regular fixed24.8

	s16 sx_raw = m_control_0[pf_num];
	s16 sy_raw = m_control_0[pf_num + 4];

	// why don't we need to do the 24 adjustment for pf 1 and 2 ?
	sy_raw += (1 << 7); // 9.7

	if (m_flipscreen) {
		sx_raw += 320 << 6; // 10.6
		sx_raw += (512 + 192) << 6; // 10.6

		sy_raw = -sy_raw;
	}

	sx_raw += (40 - 4*pf_num) << 6; // 10.6

	fixed8 sx = sx_raw << (8-6); // 10.6 to 24.8
	fixed8 sy = sy_raw << (8-7); // 9.7 to 24.8
	sx ^= 0b1111'1100;
	if (m_flipscreen) {
		sx = sx - (H_START << 8);
		sy = -sy;
	} else {
		sx = sx - (H_START << 8);
	}

	reg_sx = sx;
	reg_sy = sy;
}

template<typename Mix>
std::vector<FDP::clip_plane_inf>
FDP::calc_clip(
		const clip_plane_inf (&clip)[NUM_CLIPPLANES],
		const Mix line)
{
	constexpr s16 INF_L = H_START;
	constexpr s16 INF_R = H_START + H_VIS;

	std::bitset<4> normal_planes = line->clip_enable() & ~line->clip_inv();
	std::bitset<4> invert_planes = line->clip_enable() & line->clip_inv();
	if (!line->clip_inv_mode())
		std::swap(normal_planes, invert_planes);

	// start with a visible region spanning the entire space
	std::vector<clip_plane_inf> ranges{1, clip_plane_inf{INF_L, INF_R}};
	std::vector<clip_plane_inf> new_ranges;
	for (int plane = 0; plane < NUM_CLIPPLANES; plane++) {
		const s16 clip_l = clip[plane].l - 1;
		const s16 clip_r = clip[plane].r - 2;

		if (normal_planes[plane]) {
			// check and clip all existing ranges
			for (auto it = ranges.begin(); it != ranges.end(); ) {
				// if this clip is <1 px wide, clip entire line
				// remove ranges outside normal clip intersection
				if (clip_l > clip_r || it->r < clip_l || it->l > clip_r) {
					it = ranges.erase(it);
				} else { // otherwise intersect normally
					it->l = std::max(it->l, clip_l);
					it->r = std::min(it->r, clip_r);
					++it;
				}
			}
		} else if (invert_planes[plane] && (clip_l <= clip_r)) {
			// ASSUMING: only up to two clip settings legal at a time,
			// can get up to 3 ranges; figure out which one it *isn't* later
			new_ranges.reserve(2 * ranges.size());
			new_ranges.insert(new_ranges.end(), ranges.size(), clip_plane_inf{INF_L, clip_l});
			new_ranges.insert(new_ranges.end(), ranges.size(), clip_plane_inf{clip_r, INF_R});

			for (auto it = new_ranges.begin(); it != new_ranges.end(); ) {
				auto n = std::next(it);
				for (const auto &range : ranges) {
					it->l = std::max(range.l, it->l);
					it->r = std::max(range.l, it->r);
					if (it->l >= it->r) {
						n = new_ranges.erase(it);
						break; // goto...
					}
				}
				it = n;
			}
			ranges = std::move(new_ranges);
			new_ranges.clear();
		}
	}
	return ranges;
}

static int mosaic(int x, u8 sample)
{
	int x_count = x - 46 + 114;
	// hw quirk: the counter resets 2 px from the right edge...
	x_count = (x_count >= 432) ? (x_count - 432) : x_count;
	return x - (x_count % sample);
}

inline bool FDP::mixable::layer_enable() const
{
	return (mix_value & 0x2000) && blend_mask() != 0b11;
}
inline int FDP::mixable::x_index(int x) const
{
	return x;
}
inline int FDP::mixable::y_index(int y) const
{
	return y;
}
inline bool FDP::sprite_inf::layer_enable() const
{
	return (mix_value & 0x2000) && blend_mask() != 0b00;
}
inline u16 FDP::playfield_inf::palette_adjust(u16 pal) const
{
	return pal + pal_add;
}
inline int FDP::playfield_inf::x_index(int x) const
{
	return (((reg_fx_x + (x - H_START) * x_scale)>>8) + H_START) & width_mask;
}
inline int FDP::playfield_inf::y_index(int y) const
{
	return ((reg_fx_y >> 8) + colscroll) & 0x1ff;
}
inline int FDP::pivot_inf::x_index(int x) const
{
	return (x + reg_sx) & 0x1FF;
}
inline int FDP::pivot_inf::y_index(int y) const
{
	return (reg_sy + y) & (use_pix() ? 0xff : 0x1ff);
}

template<typename Mix>
bool FDP::mix_line(Mix &gfx, mix_pix *z, pri_mode *pri, const f3_line_inf &line, const clip_plane_inf &range)
{
	const int y = gfx.y_index(line.y);
	const u16 *src = &gfx.bitmap.src->pix(y);
	const u8 *flags = gfx.bitmap.flags ? &gfx.bitmap.flags->pix(y) : nullptr;

	for (int x = range.l; x < range.r; x++) {
		if (gfx.blend_mask() == pri[x].src_blendmode)
			continue; // note that layers cannot blend against the same blend mode

		const int real_x = gfx.x_sample_enable ? mosaic(x, line.x_sample) : x;
		const int gfx_x = gfx.x_index(real_x);
		// tilemap transparent flag
		if (flags && !(flags[gfx_x] & 0xf0))
			continue;

		if (gfx.prio > pri[x].src_prio) {
			// submit src pix
			if (const u16 pal = gfx.palette_adjust(src[gfx_x])) {
				// could be pulled out of loop for pivot and sprite
				u8 sel = gfx.blend_select(flags, gfx_x);

				switch (gfx.blend_mask()) {
				case 0b01: // normal blend
					sel = 2 + sel;
					[[fallthrough]];
				case 0b10: // reverse blend
					if (line.blend[sel] == 0)
						continue; // could be early return for pivot and sprite
					z[x].src_blend = line.blend[sel];
					break;
				case 0b00: case 0b11: default: // opaque layer
					if (line.blend[sel] + line.blend[2 + sel] == 0)
						continue; // could be early return for pivot and sprite
					z[x].src_blend = line.blend[2 + sel];
					z[x].dst_blend = line.blend[sel];
					pri[x].dst_prio = gfx.prio;
					z[x].dst_pal = pal;
					break;
				}

				// lock in source color for blending and update the prio test buffer
				z[x].src_pal = pal;
				pri[x].src_blendmode = gfx.blend_mask();
				pri[x].src_prio = gfx.prio;
			}
		} else if (gfx.prio >= pri[x].dst_prio) {
			// submit dest pix
			if (const u16 pal = gfx.palette_adjust(src[gfx_x])) {
				if (gfx.prio != pri[x].dst_prio)
					z[x].dst_pal = pal;
				else // prio conflict = color line conflict? (dariusg, bubblem)
					z[x].dst_pal = 0;
				pri[x].dst_prio = gfx.prio;

				const bool sel = gfx.blend_select(flags, gfx_x);
				switch (pri[x].src_blendmode) {
				case 0b01:
					z[x].dst_blend = line.blend[sel];
					break;
				case 0b10: case 0b00: case 0b11: default:
					z[x].dst_blend = line.blend[2 + sel];
					break;
				}
			}
		}
	}

	constexpr int DEBUG_X = 50 + H_START;
	constexpr int DEBUG_Y = 180 + V_START;
	if (TAITOF3_VIDEO_DEBUG && line.y == DEBUG_Y) {
		logerror("[%X] %s%d: %d,%d (%d)\n   {pal: %x/%x, blend: %x/%x, prio: %x/%x}\n",
				 gfx.prio, gfx.debug_name(), gfx.debug_index,
				 gfx.blend_b(), gfx.blend_a(), gfx.blend_select(flags, 82),
				 z[DEBUG_X].src_pal, z[DEBUG_X].dst_pal,
				 z[DEBUG_X].src_blend, z[DEBUG_X].dst_blend,
				 pri[DEBUG_X].src_prio, pri[DEBUG_X].dst_prio);
	}

	return false; // TODO: determine when we can stop drawing?
}

void FDP::render_line(pen_t *dst, const mix_pix (&z)[H_TOTAL])
{
	const pen_t *clut = &m_palette->pen(0);
	for (int x = H_START; x < H_START + H_VIS; x++) {
		const mix_pix mix = z[x];
		rgb_t s_rgb = clut[mix.src_pal];
		rgb_t d_rgb = clut[mix.dst_pal];
		dst[x] = (s_rgb.scale8(mix.src_blend) + d_rgb.scale8(mix.dst_blend)).set_a(255);
	}
}

void FDP::scanline_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto prio = [] (const auto &obj) -> u8 { return obj->prio; };

	// acquire sprite rendering layers, playfield tilemaps, playfield scroll
	f3_line_inf line_data{};
	for (int i=0; i < NUM_SPRITEGROUPS; i++) {
		line_data.sp[i].bitmap = draw_source(&m_sprite_framebuffers[i]);
		line_data.sp[i].sprite_pri_usage = &m_sprite_pri_row_usage;
		line_data.sp[i].debug_index = i;
	}
	for (int pf = 0; pf < NUM_PLAYFIELDS; ++pf) {
		get_pf_scroll(pf, line_data.pf[pf].reg_sx, line_data.pf[pf].reg_sy);

		line_data.pf[pf].reg_fx_y = line_data.pf[pf].reg_sy;
		line_data.pf[pf].width_mask = m_width_mask;
		line_data.pf[pf].debug_index = pf;
	}
	if (m_flipscreen) {
		line_data.pivot.reg_sx = m_control_1[4] - 12;
		line_data.pivot.reg_sy = m_control_1[5];
	} else {
		line_data.pivot.reg_sx = -m_control_1[4] - 5;
		line_data.pivot.reg_sy = -m_control_1[5];
	}

	for (int screen_y = 0; screen_y != 256; screen_y += 1) {
		const int y = m_flipscreen ? (255 - screen_y) : screen_y;
		read_line_ram(line_data, y);
		line_data.y = screen_y;

		// some tilemap source selection depends on current line data
		for (int pf_num = 0; pf_num < NUM_PLAYFIELDS; ++pf_num) {
			auto &pf = line_data.pf[pf_num];
			int tmap_number = pf_num;
			if (!m_extend && pf.alt_tilemap)
				tmap_number += 2;
			pf.bitmap = draw_source(m_tilemap[tmap_number]);
			// what is with this calculation...
			pf.reg_fx_x = pf.reg_sx + pf.rowscroll;
			pf.reg_fx_x += 10 * ((pf.x_scale) - (1<<8));
		}
		if (line_data.pivot.use_pix()) {
			line_data.pivot.bitmap = draw_source(m_pixel_layer);
		} else {
			line_data.pivot.bitmap = draw_source(m_vram_layer);
		}

		mix_pix line_buf[H_TOTAL]{};
		pri_mode line_pri[H_TOTAL]{};
		// background palette -- what contributions should this default to?
		std::fill_n(line_buf, H_TOTAL, mix_pix{0, line_data.bg_palette, 0x0, 0xff});

		// sort layers
		std::array<std::variant<pivot_inf*, sprite_inf*, playfield_inf*>,
				NUM_SPRITEGROUPS + NUM_PLAYFIELDS + 1> layers = {
			&line_data.pivot, // this order seems ok for dariusg/gseeker/bubblem conflicts?
			&line_data.sp[0], &line_data.pf[0],
			&line_data.sp[3], &line_data.pf[3],
			&line_data.sp[2], &line_data.pf[2],
			&line_data.sp[1], &line_data.pf[1]
		};
		std::stable_sort(layers.begin(), layers.end(),
				[prio] (auto a, auto b) -> bool {
					return std::visit(prio, a) > std::visit(prio, b);
				});

		// draw layers to framebuffer (currently top to bottom)
		if (screen_y >= cliprect.min_y && screen_y <= cliprect.max_y) {
			for (auto gfx : layers) {
				std::visit(
						[this, &line_data, &line_buf, &line_pri] (auto &&arg) {
							if (arg->layer_enable() && arg->used(line_data.y)) {
								const auto clip_ranges = calc_clip(line_data.clip, arg);
								for (const auto &clip : clip_ranges) {
									mix_line(*arg, &line_buf[0], &line_pri[0], line_data, clip);
								}
							}
						},
						gfx);
			}
			if (TAITOF3_VIDEO_DEBUG == 1) {
				if (y == 100) {
					logerror("{pal: %x/%x, blend: %x/%x, prio: %x/%x}\n",
							 line_buf[180].src_pal, line_buf[180].dst_pal,
							 line_buf[180].src_blend, line_buf[180].dst_blend,
							 line_pri[180].src_prio, line_pri[180].dst_prio);
					logerror("-' [%hhu,%hhu,%hhu,%hhu] '------------------------- 100\n", line_data.blend[0],
							 line_data.blend[1], line_data.blend[2], line_data.blend[3]);
				}
			}

			render_line(&bitmap.pix(screen_y), line_buf);
		}

		if (screen_y != 0) {
			// update registers
			for (auto &pf : line_data.pf) {
				pf.reg_fx_y += pf.y_scale;
			}
		}
	}
}

/******************************************************************************/

inline void FDP::f3_drawgfx(const tempsprite &sprite)
{
	bitmap_ind16 &dest_bmp = m_sprite_framebuffers[sprite.pri];

	gfx_element *gfx = m_gfxdecode->gfx(2);
	const u8 *code_base = gfx->get_data(sprite.code % gfx->elements());

	const u8 flipx = sprite.flip_x ? 0xF : 0;
	const u8 flipy = sprite.flip_y ? 0xF : 0;

	fixed8 dy8 = (sprite.y);
	if (!m_flipscreen)
		dy8 += 255; // round up in non-flipscreen mode?
	// maybe flipscreen coordinate adjustments should be done after all this math, during final rendering?
	// y scaling testcases: elvactr mission # text (!flip), kaiserknj attract text (flip)

	for (u8 y = 0; y < 16; y++) {
		const int dy = dy8 >> 8;
		dy8 += sprite.scale_y;
		if (dy < V_START || dy >= V_END)
			continue;
		u8 *pri = &m_pri_alp_bitmap.pix(dy);
		u16 *dest = &dest_bmp.pix(dy);
		auto &usage = m_sprite_pri_row_usage[dy];
		const u8 *src = &code_base[(y ^ flipy) * 16];

		fixed8 dx8 = (sprite.x) + 128; // 128 is ½ in fixed.8
		for (u8 x = 0; x < 16; x++) {
			const int dx = dx8 >> 8;
			dx8 += sprite.scale_x;
			// is this necessary with the large margins outside visarea?
			if (dx < H_START || dx >= H_END)
				continue;
			if (dx == dx8 >> 8) // if the next pixel would be in the same column, skip this one
				continue;
			const u8 c = src[(x ^ flipx)] & m_sprite_pen_mask;
			if (c && !pri[dx]) {
				dest[dx] = gfx->colorbase() + (sprite.color<<4 | c);
				pri[dx] = 1;
				usage |= 1<<sprite.pri;
			}
		}
	}
}

void FDP::get_sprite_info()
{
	const u16 *spriteram16_ptr = m_spriteram.target();

	struct sprite_axis {
		fixed8 block_scale = 1 << 8;
		fixed8 pos = 0, block_pos = 0;
		s16 global = 0, subglobal = 0;
		void update(u8 scroll, u16 posw, bool multi, u8 block_ctrl, u8 new_zoom)
		{
			s16 new_pos = util::sext(posw, 12);
			// set scroll offsets
			if (BIT(scroll, 0))
				subglobal = new_pos;
			if (BIT(scroll, 1))
				global = new_pos;
			// add scroll offsets
			if (!BIT(scroll, 3)) {
				new_pos += global;
				if (!BIT(scroll, 2))
					new_pos += subglobal;
			}


			switch (block_ctrl) {
			case 0b00:
				if (!multi) {
					block_pos = new_pos << 8;
					block_scale = (0x100 - new_zoom);
				}
				[[fallthrough]];
			case 0b10:
				pos = block_pos;
				break;
			case 0b11:
				pos += block_scale * 16;
				break;
			}
		}
	};
	sprite_axis x, y;
	u8 color = 0;
	bool multi = false;

	tempsprite *sprite_ptr = &m_spritelist[0];
	int total_sprites = 0;

	for (int offs = 0; offs < 0x400 && (total_sprites < 0x400); offs++) {
		total_sprites++; // prevent infinite loops
		const int bank = m_sprite_bank ? 0x4000 : 0;
		const u16 *spr = &spriteram16_ptr[bank + (offs * 8)];

		// Check if special command bit is set
		if (BIT(spr[3], 15)) {
			const u16 cntrl = spr[5];
			m_flipscreen = BIT(cntrl, 13);

			/*
			    ??f? ??dd ???? ??tb

			    cntrl bit 12(0x1000) = disabled?  (From F2 driver, doesn't seem used anywhere)
			    cntrl bit 4 (0x0010) = ???
			    cntrl bit 5 (0x0020) = ???
			*/

			m_sprite_extra_planes = BIT(cntrl, 8, 2); // 00 = 4bpp, 01 = 5bpp, 10 = nonsense, 11 = 6bpp
			m_sprite_pen_mask = (m_sprite_extra_planes << 4) | 0x0f;
			m_sprite_trails = BIT(cntrl, 1);

			if (cntrl & 0b1101'1100'1111'1100) {
				logerror("unknown sprite command bits: %4x\n", cntrl);
			}

			// Sprite bank select
			m_sprite_bank = BIT(cntrl, 0);
		} else {
			if (spr[5] >> 1) {
				logerror("unknown word 5 bits: %4x\n", spr[5]);
			}
		}

		if (spr[3] & 0b0110'0000'0000'0000) {
			logerror("unknown sprite y upper bits: %4x\n", spr[3]);
		}
		if (spr[6] & 0b0111'1100'0000'0000) {
			// landmakrj: 8BFF ?
			logerror("unknown sprite jump bits: %4x\n", spr[6]);
		}
		if (spr[7] != 0) {
			logerror("unknown sprite word 7: %4x\n", spr[7]);
		}

		// Check if the sprite list jump bit is set
		// we have to check this AFTER processing sprite commands because recalh uses a sprite command and jump in the same sprite
		// i wonder if this should go after other sprite processsing as well?  can a regular sprite have a jump ?
		if (BIT(spr[6], 15)) {
			const int new_offs = BIT(spr[6], 0, 10);
			if (new_offs == offs) // could this be ≤ ? -- NO! RECALH USES BACKWARDS JUMPS!!
				break; // optimization, edge cases to watch for: looped sprite block commands?

			offs = new_offs - 1; // subtract because we increment in the for loop
		}

		const u8 spritecont = spr[4] >> 8;
		const bool lock = BIT(spritecont, 2);
		if (!lock)
			color = spr[4] & 0xFF;
		const u8 scroll_mode = BIT(spr[2], 12, 4);
		const u16 zooms = spr[1];
		x.update(scroll_mode, spr[2] & 0xFFF, multi, BIT(spritecont, 4 + 2, 2), zooms & 0xFF);
		y.update(scroll_mode, spr[3] & 0xFFF, multi, BIT(spritecont, 4 + 0, 2), zooms >> 8);
		multi = BIT(spritecont, 3);

		const int tile = spr[0] | (BIT(spr[5], 0) << 16);
		if (!tile)
			continue; // todo: is this the correct way to tell if a sprite exists?

		const fixed8 tx = m_flipscreen ? (512<<8) - x.block_scale*16 - x.pos : x.pos;
		const fixed8 ty = m_flipscreen ? (256<<8) - y.block_scale*16 - y.pos : y.pos;

		if (tx + x.block_scale*16 <= H_START<<8 || tx >= H_END<<8 || ty + y.block_scale*16 <= V_START<<8 || ty >= V_END<<8)
			continue;

		const bool flip_x = BIT(spritecont, 0);
		const bool flip_y = BIT(spritecont, 1);

		sprite_ptr->x = tx;
		sprite_ptr->y = ty;
		sprite_ptr->flip_x = m_flipscreen ? !flip_x : flip_x;
		sprite_ptr->flip_y = m_flipscreen ? !flip_y : flip_y;
		sprite_ptr->code = tile;
		sprite_ptr->color = color;
		sprite_ptr->scale_x = x.block_scale;
		sprite_ptr->scale_y = y.block_scale;
		sprite_ptr->pri = BIT(color, 6, 2);
		sprite_ptr++;
	}
	m_sprite_end = sprite_ptr;
}


void FDP::draw_sprites()
{
	if (!m_sprite_trails) {
		m_pri_alp_bitmap.fill(0);
		std::fill_n(m_sprite_pri_row_usage, 256, 0);
		for (auto &sp_bitmap : m_sprite_framebuffers) {
			sp_bitmap.fill(0);
		}
	}

	for (const auto *spr = m_sprite_end; spr-- != &m_spritelist[0]; ) {
		f3_drawgfx();
	}
}

/******************************************************************************/


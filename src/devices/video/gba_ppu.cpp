// license:BSD-3-Clause
// copyright-holders:R. Belmont,Ryan Holtz
/***************************************************************************

    gba_ppu.cpp

    Nintendo Game Boy Advance / DS 2D graphics engine
	Derived from gba_lcd.cpp by R. Belmont and Ryan Holtz

    The engine renders one line at a time into a caller-supplied BGR555
    buffer.  The host supplies the memories it fetches from (palette RAM,
    OAM, and VRAM as 16K pages), owns the LCD timing and DISPSTAT, and
    converts each line for its display.

	TODOs: 3D, display capture, and per-line OBJ cycle budgeting.

***************************************************************************/

#include "emu.h"
#include "gba_ppu.h"

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(NDS_PPU_A, gba_ppu_nds_a_device, "nds_ppu_a", "Nintendo DS 2D engine A")
DEFINE_DEVICE_TYPE(NDS_PPU_B, gba_ppu_nds_b_device, "nds_ppu_b", "Nintendo DS 2D engine B")

namespace {

// line buffer pixels are BGR555 with bit 15 set when opaque
constexpr uint16_t PIX_OPAQUE = 0x8000;

// m_objattr flags
constexpr uint8_t OBJ_SEMI = 0x01;      // semi-transparent (mode 1)
constexpr uint8_t OBJ_BITMAP = 0x02;    // bitmap OBJ, alpha in bits 4-7
constexpr uint8_t OBJ_MOSAIC = 0x04;    // covered by a mosaic OBJ

constexpr int LAYER_OBJ = 4;
constexpr int LAYER_BACKDROP = 5;

// what each BG is in each video mode
enum : uint8_t
{
	BG_NONE = 0,
	BG_TEXT,
	BG_AFFINE,      // 8-bit map entries, 256 colours
	BG_EXTENDED,    // DS: BGxCNT decides between 16-bit map entries and a bitmap
	BG_LARGE,       // DS mode 6: one 512x1024 / 1024x512 256-colour bitmap
	BG_GBA_MODE3,   // GBA: 240x160 direct colour bitmap
	BG_GBA_MODE4,   // GBA: 240x160 256 colour bitmap, two frames
	BG_GBA_MODE5    // GBA: 160x128 direct colour bitmap, two frames
};

constexpr uint8_t gba_bg_kind[8][4] =
{
	{ BG_TEXT, BG_TEXT, BG_TEXT,      BG_TEXT },
	{ BG_TEXT, BG_TEXT, BG_AFFINE,    BG_NONE },
	{ BG_NONE, BG_NONE, BG_AFFINE,    BG_AFFINE },
	{ BG_NONE, BG_NONE, BG_GBA_MODE3, BG_NONE },
	{ BG_NONE, BG_NONE, BG_GBA_MODE4, BG_NONE },
	{ BG_NONE, BG_NONE, BG_GBA_MODE5, BG_NONE },
	{ BG_NONE, BG_NONE, BG_NONE,      BG_NONE },
	{ BG_NONE, BG_NONE, BG_NONE,      BG_NONE }
};

constexpr uint8_t nds_bg_kind[8][4] =
{
	{ BG_TEXT, BG_TEXT, BG_TEXT,     BG_TEXT },
	{ BG_TEXT, BG_TEXT, BG_TEXT,     BG_AFFINE },
	{ BG_TEXT, BG_TEXT, BG_AFFINE,   BG_AFFINE },
	{ BG_TEXT, BG_TEXT, BG_TEXT,     BG_EXTENDED },
	{ BG_TEXT, BG_TEXT, BG_AFFINE,   BG_EXTENDED },
	{ BG_TEXT, BG_TEXT, BG_EXTENDED, BG_EXTENDED },
	{ BG_NONE, BG_NONE, BG_LARGE,    BG_NONE },
	{ BG_NONE, BG_NONE, BG_NONE,     BG_NONE }
};

// blending coefficients saturate at 16
constexpr int coeff[32] =
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16
};

inline uint16_t blend(uint16_t a, uint16_t b, int eva, int evb)
{
	const int r = std::min(31, (((a >>  0) & 0x1f) * eva + ((b >>  0) & 0x1f) * evb) >> 4);
	const int g = std::min(31, (((a >>  5) & 0x1f) * eva + ((b >>  5) & 0x1f) * evb) >> 4);
	const int bl = std::min(31, (((a >> 10) & 0x1f) * eva + ((b >> 10) & 0x1f) * evb) >> 4);
	return (bl << 10) | (g << 5) | r;
}

inline uint16_t brighten(uint16_t c, int evy)
{
	const int r = ((c >>  0) & 0x1f), g = ((c >>  5) & 0x1f), b = ((c >> 10) & 0x1f);
	return ((b + (((31 - b) * evy) >> 4)) << 10) | ((g + (((31 - g) * evy) >> 4)) << 5) | (r + (((31 - r) * evy) >> 4));
}

inline uint16_t darken(uint16_t c, int evy)
{
	const int r = ((c >>  0) & 0x1f), g = ((c >>  5) & 0x1f), b = ((c >> 10) & 0x1f);
	return ((b - ((b * evy) >> 4)) << 10) | ((g - ((g * evy) >> 4)) << 5) | (r - ((r * evy) >> 4));
}

// WINxH/WINxV: left/top in the high byte, right/bottom + 1 in the low byte
inline bool in_window_h(uint16_t reg, int x)
{
	const int lo = reg >> 8, hi = reg & 0xff;
	if (lo <= hi)
	{
		return (x >= lo) && (x < hi);
	}
	return (x >= lo) || (x < hi);
}

inline bool in_window_v(uint16_t reg, int y)
{
	const int lo = reg >> 8, hi = reg & 0xff;
	if ((lo == hi) && (lo >= 0xe8))
	{
		return true;
	}
	if (lo <= hi)
	{
		return (y >= lo) && (y < hi);
	}
	return (y >= lo) || (y < hi);
}

char const *const reg_names[] =
{
	"DISPCNT",     "GRNSWAP",     "DISPSTAT",     "VCOUNT",
	"BG0CNT",      "BG1CNT",      "BG2CNT",       "BG3CNT",
	"BG0HOFS",     "BG0VOFS",     "BG1HOFS",      "BG1VOFS",
	"BG2HOFS",     "BG2VOFS",     "BG3HOFS",      "BG3VOFS",
	"BG2PA",       "BG2PB",       "BG2PC",        "BG2PD",
	"BG2X_L",      "BG2X_H",      "BG2Y_L",       "BG2Y_H",
	"BG3PA",       "BG3PB",       "BG3PC",        "BG3PD",
	"BG3X_L",      "BG3X_H",      "BG3Y_L",       "BG3Y_H",
	"WIN0H",       "WIN1H",       "WIN0V",        "WIN1V",
	"WININ",       "WINOUT",      "MOSAIC",       "Unused",
	"BLDCNT",      "BLDALPHA",    "BLDY",         "Unused",
	"Unused",      "Unused",      "Unused",       "Unused",
	"DISP3DCNT",   "Unused",      "DISPCAPCNT_L", "DISPCAPCNT_H",
	"DISP_MMEM_FIFO_L", "DISP_MMEM_FIFO_H", "MASTER_BRIGHT", "Unused"
};

} // anonymous namespace


gba_ppu_device::gba_ppu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_width(240)
	, m_pram(nullptr)
	, m_oam(nullptr)
	, m_display_vram(nullptr)
{
	unmap_vram();
	configure();
}

void gba_ppu_device::device_start()
{
	configure();

	save_item(NAME(m_regs));
	save_item(NAME(m_bg_ref));
	save_item(NAME(m_bg_ref_mosaic));
}

void gba_ppu_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	std::fill_n(&m_bg_ref[0][0], 4, 0);
	std::fill_n(&m_bg_ref_mosaic[0][0], 4, 0);
}

void gba_ppu_device::device_post_load()
{
	configure();
}

/***************************************************************************
    Per-variant configuration and behaviour (base = GBA)
***************************************************************************/

void gba_ppu_device::configure()
{
	m_width = 240;
	m_vram_mask[VRAM_BG] = 0xffff;       // 64K, followed by
	m_vram_mask[VRAM_OBJ] = 0x7fff;      // 32K of OBJ tiles
	m_vram_mask[VRAM_BG_EXTPAL] = 0x7fff;
	m_vram_mask[VRAM_OBJ_EXTPAL] = 0x1fff;
}

uint8_t gba_ppu_device::bg_kind_for(int mode, int bg) const
{
	return gba_bg_kind[mode][bg];
}

int gba_ppu_device::display_mode(uint32_t dispcnt) const
{
	return 1;   // the GBA is always in the graphics display mode
}

uint32_t gba_ppu_device::char_base_ext(uint32_t dispcnt) const
{
	return 0;
}

uint32_t gba_ppu_device::map_base_ext(uint32_t dispcnt) const
{
	return 0;
}

bool gba_ppu_device::bg_extpal_enabled(uint32_t dispcnt) const
{
	return false;
}

bool gba_ppu_device::obj_extpal_enabled(uint32_t dispcnt) const
{
	return false;
}

bool gba_ppu_device::bitmap_uses_alpha() const
{
	return false;   // GBA bitmap BGs are always opaque; bit 15 is an alpha flag only on the DS
}

bool gba_ppu_device::obj_map_1d(uint32_t dispcnt) const
{
	return BIT(dispcnt, 6);
}

uint32_t gba_ppu_device::obj_tile_boundary(uint32_t dispcnt) const
{
	return 32;
}

uint32_t gba_ppu_device::obj_bmp_boundary(uint32_t dispcnt) const
{
	return 128;
}

int gba_ppu_device::obj_min_tile(uint32_t dispcnt) const
{
	// in the GBA's bitmap modes the first 512 tiles are under the bitmap and cannot be used
	return ((dispcnt & 7) >= 3) ? 512 : 0;
}

bool gba_ppu_device::obj_allow_bitmap() const
{
	return false;   // bitmap OBJ are a DS feature
}

bool gba_ppu_device::bg0_is_3d(uint32_t dispcnt) const
{
	return false;
}

bool gba_ppu_device::has_master_bright() const
{
	return false;
}

/***************************************************************************
    System hookups
***************************************************************************/

void gba_ppu_device::set_vram_page(int region, int page, const uint32_t *data)
{
	if ((region >= 0) && (region < VRAM_REGION_COUNT) && (page >= 0) && (page < VRAM_MAX_PAGES))
	{
		m_vram_page[region][page] = data;
	}
}

void gba_ppu_device::unmap_vram()
{
	std::fill_n(&m_vram_page[0][0], VRAM_REGION_COUNT * VRAM_MAX_PAGES, nullptr);
}

/***************************************************************************
    Memory access for the renderers
***************************************************************************/

// unmapped VRAM reads as zero
inline uint32_t gba_ppu_device::vram_read32(int region, uint32_t addr) const
{
	addr &= m_vram_mask[region];
	const uint32_t *const page = m_vram_page[region][addr >> VRAM_PAGE_SHIFT];
	return page ? page[(addr & ((1 << VRAM_PAGE_SHIFT) - 1)) >> 2] : 0;
}

inline uint16_t gba_ppu_device::vram_read16(int region, uint32_t addr) const
{
	return vram_read32(region, addr) >> ((addr & 2) * 8);
}

inline uint8_t gba_ppu_device::vram_read8(int region, uint32_t addr) const
{
	return vram_read32(region, addr) >> ((addr & 3) * 8);
}

// standard palettes: 512 bytes of BG and 512 bytes of OBJ colours
inline uint16_t gba_ppu_device::bg_pal(int index) const
{
	return m_pram[index >> 1] >> ((index & 1) * 16);
}

inline uint16_t gba_ppu_device::obj_pal(int index) const
{
	return m_pram[0x80 + (index >> 1)] >> ((index & 1) * 16);
}

// extended palettes: 16 palettes of 256 colours per 8K slot
inline uint16_t gba_ppu_device::bg_extpal(int slot, int palette, int index) const
{
	return vram_read16(VRAM_BG_EXTPAL, (slot * 0x2000) + (palette * 512) + (index * 2));
}

inline uint16_t gba_ppu_device::obj_extpal(int palette, int index) const
{
	return vram_read16(VRAM_OBJ_EXTPAL, (palette * 512) + (index * 2));
}

// OAM as halfwords
inline uint16_t gba_ppu_device::oam_r(int index) const
{
	return m_oam[index >> 1] >> ((index & 1) * 16);
}

/***************************************************************************
    Registers
***************************************************************************/

uint32_t gba_ppu_device::regs_r(offs_t offset)
{
	if (offset >= std::size(m_regs))
	{
		return 0;
	}

	const uint32_t data = m_regs[offset];
	LOG("%s: read %s = %04x\n", machine().describe_context(), reg_names[offset * 2], data & 0xffff);
	LOG("%s: read %s = %04x\n", machine().describe_context(), reg_names[(offset * 2) + 1], data >> 16);
	return data;
}

void gba_ppu_device::regs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= std::size(m_regs))
	{
		return;
	}

	COMBINE_DATA(&m_regs[offset]);

	if (ACCESSING_BITS_0_15)
	{
		LOG("%s: write %s = %04x\n", machine().describe_context(), reg_names[offset * 2], data & 0xffff);
	}
	if (ACCESSING_BITS_16_31)
	{
		LOG("%s: write %s = %04x\n", machine().describe_context(), reg_names[(offset * 2) + 1], data >> 16);
	}

	// writing either half of BG2X/Y or BG3X/Y reloads the internal reference point counter
	switch (offset)
	{
		case 0x28/4:
			reload_bg_ref(0, 0);
			break;
		case 0x2c/4:
			reload_bg_ref(0, 1);
			break;
		case 0x38/4:
			reload_bg_ref(1, 0);
			break;
		case 0x3c/4:
			reload_bg_ref(1, 1);
			break;
	}
}

// the reference points are 28-bit signed with 8 fractional bits
void gba_ppu_device::reload_bg_ref(int bg, int xy)
{
	m_bg_ref[bg][xy] = int32_t(m_regs[(0x28/4) + (bg * 4) + xy] << 4) >> 4;
}

/***************************************************************************
    Backgrounds
***************************************************************************/

void gba_ppu_device::draw_text_bg(int bg, int line)
{
	const uint32_t dispcnt = m_regs[0];
	const uint16_t cnt = m_regs[2 + (bg >> 1)] >> ((bg & 1) * 16);
	uint16_t *const dest = m_bgline[bg];

	// DS engine A can offset all of its tile and map bases in 64K steps
	const uint32_t charbase = (((cnt >> 2) & 0xf) * 0x4000) + char_base_ext(dispcnt);
	const uint32_t mapbase = (((cnt >> 8) & 0x1f) * 0x800) + map_base_ext(dispcnt);
	const int width = BIT(cnt, 14) ? 512 : 256;
	const int height = BIT(cnt, 15) ? 512 : 256;
	const bool color256 = BIT(cnt, 7);
	const bool extpal = bg_extpal_enabled(dispcnt);
	const int slot = ((bg < 2) && BIT(cnt, 13)) ? (bg + 2) : bg;    // BG0/BG1 may use slots 2/3 instead
	const int hofs = m_regs[4 + bg] & 0x1ff;
	const int vofs = (m_regs[4 + bg] >> 16) & 0x1ff;

	// vertical mosaic holds the line within each block
	int y = line;
	if (BIT(cnt, 6))
	{
		const int mosaic_v = ((m_regs[0x4c/4] >> 4) & 0xf) + 1;
		y -= y % mosaic_v;
	}
	const int yy = (y + vofs) & (height - 1);

	uint32_t last_mapaddr = ~0;
	uint16_t entry = 0;
	uint32_t tileaddr = 0;
	uint16_t palbase = 0;

	for (int x = 0; x < m_width; x++)
	{
		const int xx = (x + hofs) & (width - 1);

		// 32x32-tile 2K map blocks, arranged [0][1] over [2][3]
		uint32_t mapaddr = mapbase + (((yy >> 3) & 31) * 64) + (((xx >> 3) & 31) * 2);
		if (xx >= 256)
		{
			mapaddr += 0x800;
		}
		if (yy >= 256)
		{
			mapaddr += (width == 512) ? 0x1000 : 0x800;
		}

		if (mapaddr != last_mapaddr)
		{
			last_mapaddr = mapaddr;
			entry = vram_read16(VRAM_BG, mapaddr);
			const int ty = BIT(entry, 11) ? (7 - (yy & 7)) : (yy & 7);
			tileaddr = charbase + ((entry & 0x3ff) * (color256 ? 64 : 32)) + (ty * (color256 ? 8 : 4));
			palbase = (entry >> 12) << 4;
		}

		const int tx = BIT(entry, 10) ? (7 - (xx & 7)) : (xx & 7);
		if (color256)
		{
			const uint8_t index = vram_read8(VRAM_BG, tileaddr + tx);
			if (index)
			{
				dest[x] = (extpal ? bg_extpal(slot, entry >> 12, index) : bg_pal(index)) | PIX_OPAQUE;
			}
		}
		else
		{
			uint8_t index = vram_read8(VRAM_BG, tileaddr + (tx >> 1));
			index = (tx & 1) ? (index >> 4) : (index & 0xf);
			if (index)
			{
				dest[x] = bg_pal(palbase | index) | PIX_OPAQUE;
			}
		}
	}
}

// all the ROZ layer types
void gba_ppu_device::draw_affine_bg(int bg, int line, int kind)
{
	const uint32_t dispcnt = m_regs[0];
	const uint16_t cnt = m_regs[2 + (bg >> 1)] >> ((bg & 1) * 16);
	uint16_t *const dest = m_bgline[bg];
	const bool extpal = bg_extpal_enabled(dispcnt);
	const bool bmp_alpha = bitmap_uses_alpha();
	bool wrap = BIT(cnt, 13);
	const int size = (cnt >> 14) & 3;

	enum { AFFINE, EXTMAP, BITMAP8, BITMAP16 };
	int type;
	int width, height;
	uint32_t base;
	switch (kind)
	{
		case BG_GBA_MODE3:
		case BG_GBA_MODE4:
		case BG_GBA_MODE5:
			// fixed layouts in VRAM, no wrapping, frame select in DISPCNT bit 4
			type = (kind == BG_GBA_MODE4) ? BITMAP8 : BITMAP16;
			width = (kind == BG_GBA_MODE5) ? 160 : 240;
			height = (kind == BG_GBA_MODE5) ? 128 : 160;
			base = ((kind != BG_GBA_MODE3) && BIT(dispcnt, 4)) ? 0xa000 : 0;
			wrap = false;
			break;

		case BG_EXTENDED:
			if (!BIT(cnt, 7))
			{
				type = EXTMAP;
				width = height = 128 << size;
				base = (((cnt >> 8) & 0x1f) * 0x800) + map_base_ext(dispcnt);
			}
			else
			{
				static constexpr int bitmap_size[4][2] = { { 128, 128 }, { 256, 256 }, { 512, 256 }, { 512, 512 } };
				type = BIT(cnt, 2) ? BITMAP16 : BITMAP8;
				width = bitmap_size[size][0];
				height = bitmap_size[size][1];
				base = ((cnt >> 8) & 0x1f) * 0x4000;
			}
			break;

		case BG_LARGE:
			type = BITMAP8;
			width = size ? 1024 : 512;
			height = size ? 512 : 1024;
			base = 0;
			break;

		default:
			type = AFFINE;
			width = height = 128 << size;
			base = (((cnt >> 8) & 0x1f) * 0x800) + map_base_ext(dispcnt);
			break;
	}
	const uint32_t charbase = (((cnt >> 2) & 0xf) * 0x4000) + char_base_ext(dispcnt);

	// vertical mosaic holds the reference point of the first line of each block
	const int par = 8 + ((bg - 2) * 4);
	const int32_t pa = int16_t(m_regs[par] & 0xffff);
	const int32_t pc = int16_t(m_regs[par + 1] & 0xffff);
	const int32_t *const ref = BIT(cnt, 6) ? m_bg_ref_mosaic[bg - 2] : m_bg_ref[bg - 2];
	int32_t px = ref[0];
	int32_t py = ref[1];

	for (int x = 0; x < m_width; x++, px += pa, py += pc)
	{
		int xx = px >> 8;
		int yy = py >> 8;
		if (wrap)
		{
			xx &= width - 1;
			yy &= height - 1;
		}
		else if ((xx < 0) || (xx >= width) || (yy < 0) || (yy >= height))
		{
			continue;
		}

		switch (type)
		{
			case AFFINE:
			{
				const uint8_t tile = vram_read8(VRAM_BG, base + ((yy >> 3) * (width >> 3)) + (xx >> 3));
				const uint8_t index = vram_read8(VRAM_BG, charbase + (tile * 64) + ((yy & 7) * 8) + (xx & 7));
				if (index)
				{
					dest[x] = (extpal ? bg_extpal(bg, 0, index) : bg_pal(index)) | PIX_OPAQUE;
				}
				break;
			}

			case EXTMAP:
			{
				const uint16_t entry = vram_read16(VRAM_BG, base + ((((yy >> 3) * (width >> 3)) + (xx >> 3)) * 2));
				const int tx = BIT(entry, 10) ? (7 - (xx & 7)) : (xx & 7);
				const int ty = BIT(entry, 11) ? (7 - (yy & 7)) : (yy & 7);
				const uint8_t index = vram_read8(VRAM_BG, charbase + ((entry & 0x3ff) * 64) + (ty * 8) + tx);
				if (index)
				{
					dest[x] = (extpal ? bg_extpal(bg, entry >> 12, index) : bg_pal(index)) | PIX_OPAQUE;
				}
				break;
			}

			case BITMAP8:
			{
				const uint8_t index = vram_read8(VRAM_BG, base + (yy * width) + xx);
				if (index)
				{
					dest[x] = bg_pal(index) | PIX_OPAQUE;
				}
				break;
			}

			case BITMAP16:
			{
				const uint16_t color = vram_read16(VRAM_BG, base + (((yy * width) + xx) * 2));
				if (!bmp_alpha || BIT(color, 15))
				{
					dest[x] = color | PIX_OPAQUE;
				}
				break;
			}
		}
	}
}

// horizontal mosaic repeats the first pixel of each block
void gba_ppu_device::apply_bg_mosaic(int bg)
{
	const int mosaic_h = (m_regs[0x4c/4] & 0xf) + 1;
	if (mosaic_h <= 1)
	{
		return;
	}

	uint16_t *const dest = m_bgline[bg];
	for (int x = 0; x < m_width; x++)
	{
		if (x % mosaic_h)
		{
			dest[x] = dest[x - (x % mosaic_h)];
		}
	}
}

/***************************************************************************
    Sprites
***************************************************************************/

void gba_ppu_device::draw_obj(int line)
{
	static constexpr int8_t obj_size[3][4][2] =
	{
		{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
		{ { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
		{ { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } }
	};

	const uint32_t dispcnt = m_regs[0];
	const bool map1d = obj_map_1d(dispcnt);
	const uint32_t boundary = obj_tile_boundary(dispcnt);
	const bool bmp1d = BIT(dispcnt, 6);
	const bool bmp256wide = BIT(dispcnt, 5);
	const uint32_t bmp_boundary = obj_bmp_boundary(dispcnt);
	const bool extpal = obj_extpal_enabled(dispcnt);
	const int min_tile = obj_min_tile(dispcnt);
	const bool allow_bmp = obj_allow_bitmap();
	const uint16_t mosaic = m_regs[0x4c/4];
	const int mosaic_h = ((mosaic >> 8) & 0xf) + 1;
	const int mosaic_v = ((mosaic >> 12) & 0xf) + 1;
	bool any_mosaic = false;

	for (int i = 0; i < 128; i++)
	{
		const uint16_t attr0 = oam_r(i * 4);
		const uint16_t attr1 = oam_r((i * 4) + 1);
		const uint16_t attr2 = oam_r((i * 4) + 2);

		const bool rot = BIT(attr0, 8);
		if (!rot && BIT(attr0, 9))
		{
			continue;                                   // disabled
		}
		const int shape = attr0 >> 14;
		if (shape == 3)
		{
			continue;                                   // prohibited
		}
		const int mode = (attr0 >> 10) & 3;
		const int alpha = attr2 >> 12;
		if ((mode == 3) && ((alpha == 0) || !allow_bmp))
		{
			continue;                                   // bitmap OBJ with alpha 0 is not displayed; prohibited on the GBA
		}
		if ((attr2 & 0x3ff) < min_tile)
		{
			continue;
		}

		const int w = obj_size[shape][attr1 >> 14][0];
		const int h = obj_size[shape][attr1 >> 14][1];
		const bool dbl = rot && BIT(attr0, 9);
		const int fw = dbl ? (w * 2) : w;
		const int fh = dbl ? (h * 2) : h;

		// mosaic holds the whole block at its first line
		const bool obj_mosaic = BIT(attr0, 12);
		const int dy = ((obj_mosaic ? (line - (line % mosaic_v)) : line) - (attr0 & 0xff)) & 0xff;
		if (dy >= fh)
		{
			continue;
		}
		int x0 = attr1 & 0x1ff;
		if (x0 >= 256)
		{
			x0 -= 512;
		}
		if (((x0 + fw) <= 0) || (x0 >= m_width))
		{
			continue;
		}

		int32_t pa = 0x100, pb = 0, pc = 0, pd = 0x100;
		if (rot)
		{
			// the parameters live in the upper halfwords of a group of four entries
			const int p = ((attr1 >> 9) & 0x1f) * 16;
			pa = int16_t(oam_r(p + 3));
			pb = int16_t(oam_r(p + 7));
			pc = int16_t(oam_r(p + 11));
			pd = int16_t(oam_r(p + 15));
		}

		const int prio = (attr2 >> 10) & 3;
		const bool color256 = BIT(attr0, 13);
		const int tile = attr2 & 0x3ff;
		uint8_t attr = 0;
		if (mode == 1)
		{
			attr = OBJ_SEMI;
		}
		else if (mode == 3)
		{
			attr = OBJ_BITMAP | (alpha << 4);
		}
		if (obj_mosaic)
		{
			attr |= OBJ_MOSAIC;
			any_mosaic = true;
		}

		for (int sx = 0; sx < fw; sx++)
		{
			const int x = x0 + sx;
			if ((x < 0) || (x >= m_width))
			{
				continue;
			}

			int tx, ty;
			if (rot)
			{
				const int cx = sx - (fw / 2);
				const int cy = dy - (fh / 2);
				tx = (((pa * cx) + (pb * cy)) >> 8) + (w / 2);
				ty = (((pc * cx) + (pd * cy)) >> 8) + (h / 2);
				if ((tx < 0) || (tx >= w) || (ty < 0) || (ty >= h))
				{
					continue;
				}
			}
			else
			{
				tx = BIT(attr1, 12) ? (w - 1 - sx) : sx;
				ty = BIT(attr1, 13) ? (h - 1 - dy) : dy;
			}

			uint16_t color;
			bool opaque;
			if (mode == 3)
			{
				uint32_t addr;
				if (bmp1d)
				{
					addr = (tile * bmp_boundary) + (((ty * w) + tx) * 2);
				}
				else if (bmp256wide)
				{
					addr = ((tile & 0x1f) * 16) + ((tile >> 5) * 0x1000) + (ty * 512) + (tx * 2);
				}
				else
				{
					addr = ((tile & 0x0f) * 16) + ((tile >> 4) * 0x800) + (ty * 256) + (tx * 2);
				}
				color = vram_read16(VRAM_OBJ, addr);
				opaque = BIT(color, 15);
			}
			else
			{
				uint32_t addr;
				if (map1d)
				{
					addr = (tile * boundary) + ((((ty >> 3) * (w >> 3)) + (tx >> 3)) * (color256 ? 64 : 32));
				}
				else
				{
					addr = ((tile & (color256 ? 0x1e : 0x1f)) * 32) + ((tile >> 5) * 1024) + ((ty >> 3) * 1024) + ((tx >> 3) * (color256 ? 64 : 32));
				}

				if (color256)
				{
					const uint8_t index = vram_read8(VRAM_OBJ, addr + ((ty & 7) * 8) + (tx & 7));
					opaque = index != 0;
					color = opaque ? (extpal ? obj_extpal(attr2 >> 12, index) : obj_pal(index)) : 0;
				}
				else
				{
					uint8_t index = vram_read8(VRAM_OBJ, addr + ((ty & 7) * 4) + ((tx & 7) >> 1));
					index = (tx & 1) ? (index >> 4) : (index & 0xf);
					opaque = index != 0;
					color = opaque ? obj_pal(((attr2 >> 12) << 4) | index) : 0;
				}
			}

			if (mode == 2)
			{
				if (opaque)
				{
					m_objwin[x] = 1;
				}
				continue;
			}

			if (obj_mosaic)
			{
				m_objattr[x] |= OBJ_MOSAIC;
			}

			if (opaque)
			{
				// the best priority wins, the lowest OAM index among equals
				if (!(m_objline[x] & PIX_OPAQUE) || (prio < m_objprio[x]))
				{
					m_objline[x] = color | PIX_OPAQUE;
					m_objprio[x] = prio;
					m_objattr[x] = attr | (m_objattr[x] & OBJ_MOSAIC);
				}
			}
			else if ((m_objline[x] & PIX_OPAQUE) && (prio < m_objprio[x]))
			{
				// a transparent pixel of a better OBJ still raises the priority of what shows through it
				m_objprio[x] = prio;
			}
		}
	}

	// horizontal mosaic repeats the first pixel of each block over what a mosaic OBJ covers
	if (any_mosaic && (mosaic_h > 1))
	{
		for (int x = 0; x < m_width; x++)
		{
			if ((x % mosaic_h) && (m_objattr[x] & OBJ_MOSAIC))
			{
				const int src = x - (x % mosaic_h);
				m_objline[x] = m_objline[src];
				m_objprio[x] = m_objprio[src];
				m_objattr[x] = m_objattr[src];
			}
		}
	}
}

/***************************************************************************
    Line composition
***************************************************************************/

void gba_ppu_device::draw_scanline(int line, uint16_t *dest)
{
	// the reference point counters restart from the registers each frame
	if (line == 0)
	{
		for (int bg = 0; bg < 2; bg++)
		{
			reload_bg_ref(bg, 0);
			reload_bg_ref(bg, 1);
		}
	}

	// and are held for the vertical mosaic at the top of each block
	const int mosaic_v = ((m_regs[0x4c/4] >> 4) & 0xf) + 1;
	if ((line % mosaic_v) == 0)
	{
		std::copy_n(&m_bg_ref[0][0], 4, &m_bg_ref_mosaic[0][0]);
	}

	render_line(line, dest);

	// step the reference points by one line
	for (int bg = 0; bg < 2; bg++)
	{
		const int par = 8 + (bg * 4);
		m_bg_ref[bg][0] += int16_t(m_regs[par] >> 16);        // PB
		m_bg_ref[bg][1] += int16_t(m_regs[par + 1] >> 16);    // PD
	}
}

void gba_ppu_device::render_line(int line, uint16_t *dest)
{
	const uint32_t dispcnt = m_regs[0];
	const int width = m_width;
	const int dispmode = display_mode(dispcnt);

	// forced blank and the display off both show white
	if ((dispmode == 0) || BIT(dispcnt, 7))
	{
		std::fill_n(dest, width, 0x7fff);
		return;
	}

	if (dispmode == 2)
	{
		// one of banks A-D as a 256x192 direct colour bitmap
		const uint32_t *const bank = m_display_vram ? &m_display_vram[((dispcnt >> 18) & 3) * (0x20000 / 4)] : nullptr;
		for (int x = 0; x < width; x++)
		{
			dest[x] = bank ? ((bank[((line * MAX_WIDTH) + x) >> 1] >> ((x & 1) * 16)) & 0x7fff) : 0;
		}
	}
	else if (dispmode == 3)
	{
		// main memory display FIFO: not emulated
		std::fill_n(dest, width, 0);
	}
	else
	{
		// --- graphics display ---
		const int mode = dispcnt & 7;
		bool bgon[4];
		int bgprio[4];
		std::fill_n(&m_bgline[0][0], 4 * MAX_WIDTH, 0);
		for (int bg = 0; bg < 4; bg++)
		{
			const uint16_t cnt = m_regs[2 + (bg >> 1)] >> ((bg & 1) * 16);
			int kind = bg_kind_for(mode, bg);
			if ((bg == 0) && bg0_is_3d(dispcnt))
			{
				kind = BG_NONE;                             // 3D output: not emulated
			}
			bgon[bg] = BIT(dispcnt, 8 + bg) && (kind != BG_NONE);
			bgprio[bg] = cnt & 3;
			if (!bgon[bg])
			{
				continue;
			}
			if (kind == BG_TEXT)
			{
				draw_text_bg(bg, line);
			}
			else
			{
				draw_affine_bg(bg, line, kind);
			}
			if (BIT(cnt, 6))
			{
				apply_bg_mosaic(bg);
			}
		}

		std::fill_n(m_objline, MAX_WIDTH, 0);
		std::fill_n(m_objattr, MAX_WIDTH, 0);
		std::fill_n(m_objwin, MAX_WIDTH, 0);
		const bool objon = BIT(dispcnt, 12);
		if (objon || BIT(dispcnt, 15))
		{
			draw_obj(line);
		}

		// window control per pixel: bits 0-3 BG0-3, bit 4 OBJ, bit 5 colour effects
		uint8_t win[MAX_WIDTH];
		const bool win0 = BIT(dispcnt, 13), win1 = BIT(dispcnt, 14), winobj = BIT(dispcnt, 15);
		if (!win0 && !win1 && !winobj)
		{
			std::fill_n(win, width, 0x3f);
		}
		else
		{
			const uint16_t win0h = m_regs[0x40/4] & 0xffff, win1h = m_regs[0x40/4] >> 16;
			const uint16_t win0v = m_regs[0x44/4] & 0xffff, win1v = m_regs[0x44/4] >> 16;
			const uint16_t winin = m_regs[0x48/4] & 0xffff, winout = m_regs[0x48/4] >> 16;
			const bool in0 = win0 && in_window_v(win0v, line);
			const bool in1 = win1 && in_window_v(win1v, line);
			for (int x = 0; x < width; x++)
			{
				uint8_t control = winout & 0x3f;
				if (winobj && m_objwin[x])
				{
					control = (winout >> 8) & 0x3f;
				}
				if (in1 && in_window_h(win1h, x))
				{
					control = (winin >> 8) & 0x3f;
				}
				if (in0 && in_window_h(win0h, x))
				{
					control = winin & 0x3f;
				}
				win[x] = control;
			}
		}

		const uint16_t bldcnt = m_regs[0x50/4] & 0xffff;
		const uint16_t bldalpha = m_regs[0x50/4] >> 16;
		const int sfx = (bldcnt >> 6) & 3;
		const uint8_t target1 = bldcnt & 0x3f;
		const uint8_t target2 = (bldcnt >> 8) & 0x3f;
		const int eva = coeff[bldalpha & 0x1f];
		const int evb = coeff[(bldalpha >> 8) & 0x1f];
		const int evy = coeff[m_regs[0x54/4] & 0x1f];
		const uint16_t backdrop = bg_pal(0) & 0x7fff;

		for (int x = 0; x < width; x++)
		{
			const uint8_t control = win[x];

			// the two topmost opaque layers at this pixel
			int l1 = LAYER_BACKDROP, l2 = LAYER_BACKDROP;
			uint16_t c1 = backdrop, c2 = backdrop;
			auto push = [&](int layer, uint16_t color)
			{
				if (l1 == LAYER_BACKDROP)
				{
					l1 = layer;
					c1 = color;
				}
				else
				{
					l2 = layer;
					c2 = color;
				}
			};
			for (int p = 0; (p < 4) && (l2 == LAYER_BACKDROP); p++)
			{
				if (objon && (control & 0x10) && (m_objline[x] & PIX_OPAQUE) && (m_objprio[x] == p))
				{
					push(LAYER_OBJ, m_objline[x]);
				}
				for (int bg = 0; (bg < 4) && (l2 == LAYER_BACKDROP); bg++)
				{
					if (bgon[bg] && (bgprio[bg] == p) && (control & (1 << bg)) && (m_bgline[bg][x] & PIX_OPAQUE))
					{
						push(bg, m_bgline[bg][x]);
					}
				}
			}

			uint16_t color = c1 & 0x7fff;
			if (control & 0x20)
			{
				const bool second = (target2 & (1 << l2)) != 0;
				if ((l1 == LAYER_OBJ) && (m_objattr[x] & OBJ_BITMAP))
				{
					if (second)
					{
						color = blend(color, c2, m_objattr[x] >> 4, 16 - (m_objattr[x] >> 4));
					}
				}
				else if ((l1 == LAYER_OBJ) && (m_objattr[x] & OBJ_SEMI) && second)
				{
					color = blend(color, c2, eva, evb);
				}
				else if (target1 & (1 << l1))
				{
					switch (sfx)
					{
						case 1:
							if (second)
							{
								color = blend(color, c2, eva, evb);
							}
							break;
						case 2:
							color = brighten(color, evy);
							break;
						case 3:
							color = darken(color, evy);
							break;
					}
				}
			}

			dest[x] = color;
		}
	}

	// apply the DS master brightness
	if (has_master_bright())
	{
		const uint16_t master = m_regs[0x6c/4] & 0xffff;
		const int factor = coeff[master & 0x1f];
		switch ((master >> 14) & 3)
		{
			case 1:
				for (int x = 0; x < width; x++)
				{
					dest[x] = brighten(dest[x], factor);
				}
				break;
			case 2:
				for (int x = 0; x < width; x++)
				{
					dest[x] = darken(dest[x], factor);
				}
				break;
			default:
				break;
		}
	}
}


/***************************************************************************
    DS engine A: the full feature set, plus the runtime GBA-compatibility mode
***************************************************************************/

gba_ppu_nds_a_device::gba_ppu_nds_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gba_ppu_device(mconfig, NDS_PPU_A, tag, owner, clock)
	, m_gba_mode(false)
{
	configure();
}

void gba_ppu_nds_a_device::device_start()
{
	gba_ppu_device::device_start();

	save_item(NAME(m_gba_mode));
}

void gba_ppu_nds_a_device::set_gba_mode(bool gba_mode)
{
	m_gba_mode = gba_mode;
	configure();
}

void gba_ppu_nds_a_device::configure()
{
	if (m_gba_mode)
	{
		gba_ppu_device::configure();
		return;
	}

	m_width = MAX_WIDTH;
	m_vram_mask[VRAM_BG] = 0x7ffff;      // 512K
	m_vram_mask[VRAM_OBJ] = 0x3ffff;     // 256K
	m_vram_mask[VRAM_BG_EXTPAL] = 0x7fff;
	m_vram_mask[VRAM_OBJ_EXTPAL] = 0x1fff;
}

// each accessor reverts to the base (GBA) behaviour while in GBA mode
uint8_t gba_ppu_nds_a_device::bg_kind_for(int mode, int bg) const
{
	return m_gba_mode ? gba_ppu_device::bg_kind_for(mode, bg) : nds_bg_kind[mode][bg];
}

int gba_ppu_nds_a_device::display_mode(uint32_t dispcnt) const
{
	return m_gba_mode ? gba_ppu_device::display_mode(dispcnt) : ((dispcnt >> 16) & 3);
}

uint32_t gba_ppu_nds_a_device::char_base_ext(uint32_t dispcnt) const
{
	return m_gba_mode ? 0 : (((dispcnt >> 24) & 7) * 0x10000);
}

uint32_t gba_ppu_nds_a_device::map_base_ext(uint32_t dispcnt) const
{
	return m_gba_mode ? 0 : (((dispcnt >> 27) & 7) * 0x10000);
}

bool gba_ppu_nds_a_device::bg_extpal_enabled(uint32_t dispcnt) const
{
	return !m_gba_mode && BIT(dispcnt, 30);
}

bool gba_ppu_nds_a_device::obj_extpal_enabled(uint32_t dispcnt) const
{
	return !m_gba_mode && BIT(dispcnt, 31);
}

bool gba_ppu_nds_a_device::bitmap_uses_alpha() const
{
	return !m_gba_mode;
}

bool gba_ppu_nds_a_device::obj_map_1d(uint32_t dispcnt) const
{
	return m_gba_mode ? gba_ppu_device::obj_map_1d(dispcnt) : BIT(dispcnt, 4);
}

uint32_t gba_ppu_nds_a_device::obj_tile_boundary(uint32_t dispcnt) const
{
	return m_gba_mode ? gba_ppu_device::obj_tile_boundary(dispcnt) : (32 << ((dispcnt >> 20) & 3));
}

uint32_t gba_ppu_nds_a_device::obj_bmp_boundary(uint32_t dispcnt) const
{
	return m_gba_mode ? gba_ppu_device::obj_bmp_boundary(dispcnt) : (128 << BIT(dispcnt, 22));
}

int gba_ppu_nds_a_device::obj_min_tile(uint32_t dispcnt) const
{
	return m_gba_mode ? gba_ppu_device::obj_min_tile(dispcnt) : 0;
}

bool gba_ppu_nds_a_device::obj_allow_bitmap() const
{
	return !m_gba_mode;
}

bool gba_ppu_nds_a_device::bg0_is_3d(uint32_t dispcnt) const
{
	return !m_gba_mode && BIT(dispcnt, 3);
}

bool gba_ppu_nds_a_device::has_master_bright() const
{
	return !m_gba_mode;
}


/***************************************************************************
    DS engine B: no base offsets, no VRAM display mode, no 3D
***************************************************************************/

gba_ppu_nds_b_device::gba_ppu_nds_b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gba_ppu_device(mconfig, NDS_PPU_B, tag, owner, clock)
{
	configure();
}

void gba_ppu_nds_b_device::configure()
{
	m_width = MAX_WIDTH;
	m_vram_mask[VRAM_BG] = 0x1ffff;      // 128K
	m_vram_mask[VRAM_OBJ] = 0x1ffff;     // 128K
	m_vram_mask[VRAM_BG_EXTPAL] = 0x7fff;
	m_vram_mask[VRAM_OBJ_EXTPAL] = 0x1fff;
}

uint8_t gba_ppu_nds_b_device::bg_kind_for(int mode, int bg) const
{
	return nds_bg_kind[mode][bg];
}

int gba_ppu_nds_b_device::display_mode(uint32_t dispcnt) const
{
	return (dispcnt >> 16) & 1;     // engine B has only the off and graphics display modes
}

bool gba_ppu_nds_b_device::bg_extpal_enabled(uint32_t dispcnt) const
{
	return BIT(dispcnt, 30);
}

bool gba_ppu_nds_b_device::obj_extpal_enabled(uint32_t dispcnt) const
{
	return BIT(dispcnt, 31);
}

bool gba_ppu_nds_b_device::bitmap_uses_alpha() const
{
	return true;
}

bool gba_ppu_nds_b_device::obj_map_1d(uint32_t dispcnt) const
{
	return BIT(dispcnt, 4);
}

uint32_t gba_ppu_nds_b_device::obj_tile_boundary(uint32_t dispcnt) const
{
	return 32 << ((dispcnt >> 20) & 3);
}

uint32_t gba_ppu_nds_b_device::obj_bmp_boundary(uint32_t dispcnt) const
{
	return 128 << BIT(dispcnt, 22);
}

int gba_ppu_nds_b_device::obj_min_tile(uint32_t dispcnt) const
{
	return 0;
}

bool gba_ppu_nds_b_device::obj_allow_bitmap() const
{
	return true;
}

bool gba_ppu_nds_b_device::has_master_bright() const
{
	return true;
}

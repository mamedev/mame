// license:BSD-3-Clause
// copyright-holders:Luca Elia, Nicola Salmoria
/***************************************************************************

    The CPU controls a video blitter that can read data from the ROMs
    (instructions to draw pixel by pixel, in a compressed form) and write to
    up to 8 frame buffers.

    hanamai:
    There are four scrolling layers. Each layer consists of 2 frame buffers.
    The 2 images are interleaved to form the final picture sent to the screen.

    drgpunch:
    There are three scrolling layers. Each layer consists of 2 frame buffers.
    The 2 images are interleaved to form the final picture sent to the screen.

    mjdialq2:
    Two scrolling layers.

***************************************************************************/

#include "emu.h"
#include "includes/dynax.h"

// Log Blitter
//#define VERBOSE 1
#include "logmacro.h"


// x B01234 G01234 R01234
void dynax_state::sprtmtch_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		int const x = (color_prom[i] << 8) | color_prom[0x200 + i];
		// The bits are in reverse order!
		int const r = bitswap<5>((x >>  0) & 0x1f, 0, 1, 2, 3, 4);
		int const g = bitswap<5>((x >>  5) & 0x1f, 0, 1, 2, 3, 4);
		int const b = bitswap<5>((x >> 10) & 0x1f, 0, 1, 2, 3, 4);
		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

// x xB0123 xG0123 xR0123
void dynax_state::janyuki_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		int const x = (color_prom[i] << 8) + color_prom[0x200 + i];
		// The bits are in reverse order!
		int const r = bitswap<4>((x >>  0) & 0x0f, 0, 1, 2, 3);
		int const g = bitswap<4>((x >>  5) & 0x0f, 0, 1, 2, 3);
		int const b = bitswap<4>((x >> 10) & 0x0f, 0, 1, 2, 3);
		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

/***************************************************************************


                                Video Blitter(s)


***************************************************************************/

#define LAYOUT_HANAMAI  0   // 4 layers, interleaved
#define LAYOUT_HNORIDUR 1   // same as hanamai but some bits are inverted and layer order is reversed
#define LAYOUT_DRGPUNCH 2   // 3 couples of layers, interleaved
#define LAYOUT_MJDIALQ2 3   // 2 layers
#define LAYOUT_JANTOUKI 4   // 2 x (4 couples of layers, interleaved)

void dynax_state::dynax_extra_scrollx_w(uint8_t data)
{
	m_extra_scroll_x = data;
}

void dynax_state::dynax_extra_scrolly_w(uint8_t data)
{
	m_extra_scroll_y = data;
}


/* Destination Layers */
void dynax_state::dynax_blit_dest_w(uint8_t data)
{
	m_blit_dest = data;
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit_dest = bitswap<8>(m_blit_dest ^ 0x0f, 7, 6, 5, 4, 0, 1, 2, 3);

	LOG("D=%02X ", data);
}

void dynax_state::dynax_blit2_dest_w(uint8_t data)
{
	m_blit2_dest = data;
	LOG("D'=%02X ", data);
}

void dynax_state::tenkai_blit_dest_w(uint8_t data)
{
	dynax_blit_dest_w(bitswap<8>(data, 7, 6, 5, 4, 0, 1, 2, 3));
}

/*
mjelctrn:   7 d e -> 1 - 4 8
mjembase:   b d e -> - 2 4 8
*/
void dynax_state::mjembase_blit_dest_w(uint8_t data)
{
	dynax_blit_dest_w(bitswap<8>(data, 7, 6, 5, 4, 2, 3, 1, 0));
}


/* Background Color */
void dynax_state::dynax_blit_backpen_w(uint8_t data)
{
	m_blit_backpen = data;
	LOG("B=%02X ", data);
}


/* Layers 0&1 Palettes (Low Bits) */
void dynax_state::dynax_blit_palette01_w(uint8_t data)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit_palettes = (m_blit_palettes & 0x00ff) | ((data & 0x0f) << 12) | ((data & 0xf0) << 4);
	else
		m_blit_palettes = (m_blit_palettes & 0xff00) | data;
	LOG("P01=%02X ", data);
}

void dynax_state::tenkai_blit_palette01_w(uint8_t data)
{
	m_blit_palettes = (m_blit_palettes & 0xff00) | data;
	LOG("P01=%02X ", data);
}


/* Layers 4&5 Palettes (Low Bits) */
void dynax_state::dynax_blit_palette45_w(uint8_t data)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit2_palettes = (m_blit2_palettes & 0x00ff) | ((data & 0x0f) << 12) | ((data & 0xf0) << 4);
	else
		m_blit2_palettes = (m_blit2_palettes & 0xff00) | data;
	LOG("P45=%02X ", data);
}


/* Layer 2&3 Palettes (Low Bits) */
void dynax_state::dynax_blit_palette23_w(uint8_t data)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit_palettes = (m_blit_palettes & 0xff00) | ((data & 0x0f) << 4) | ((data & 0xf0) >> 4);
	else
		m_blit_palettes = (m_blit_palettes & 0x00ff) | (data << 8);
	LOG("P23=%02X ", data);
}

void dynax_state::tenkai_blit_palette23_w(uint8_t data)
{
	m_blit_palettes = (m_blit_palettes & 0x00ff) | ((data & 0x0f) << 12) | ((data & 0xf0) << 4);
	LOG("P23=%02X ", data);
}

void dynax_state::mjembase_blit_palette23_w(uint8_t data)
{
	dynax_blit_palette23_w(bitswap<8>(data, 3, 2, 1, 0, 7, 6, 5, 4));
}


/* Layer 6&7 Palettes (Low Bits) */
void dynax_state::dynax_blit_palette67_w(uint8_t data)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit2_palettes = (m_blit2_palettes & 0xff00) | ((data & 0x0f) << 4) | ((data & 0xf0) >> 4);
	else
		m_blit2_palettes = (m_blit2_palettes & 0x00ff) | (data << 8);
	LOG("P67=%02X ", data);
}


/* Layers Palettes (High Bits) */
WRITE_LINE_MEMBER(dynax_state::blit_palbank_w)
{
	m_blit_palbank = state;
	LOG("PB=%d ", state);
}

WRITE_LINE_MEMBER(dynax_state::blit2_palbank_w)
{
	m_blit2_palbank = state;
	LOG("PB'=%d ", state);
}

void dynax_state::hnoridur_palbank_w(uint8_t data)
{
	m_palbank = data & 0x0f;
	m_blit_palbank = data; // ???
}


/* Which half of the layers to write to (interleaved games only) */
WRITE_LINE_MEMBER(dynax_state::layer_half_w)
{
	m_hanamai_layer_half = !state;
	LOG("H=%d ", state);
}


/* Write to both halves of the layers (interleaved games only) */
WRITE_LINE_MEMBER(dynax_state::layer_half2_w)
{
	m_hnoridur_layer_half2 = !state;
	LOG("H2=%d ", state);
}

WRITE_LINE_MEMBER(dynax_state::mjdialq2_blit_dest0_w)
{
	m_blit_dest &= ~1;
	if (!state)
		m_blit_dest |= 1;
}

WRITE_LINE_MEMBER(dynax_state::mjdialq2_blit_dest1_w)
{
	m_blit_dest &= ~2;
	if (!state)
		m_blit_dest |= 2;
}


/* Layers Enable */
void dynax_state::dynax_layer_enable_w(uint8_t data)
{
	m_layer_enable = data;
	LOG("E=%02X ", data);
}

void dynax_state::jantouki_layer_enable_w(offs_t offset, uint8_t data)
{
	int mask = 1 << (7 - offset);
	m_layer_enable = (m_layer_enable & ~mask) | ((data & 1) ? mask : 0);
	m_layer_enable |= 1;
}

WRITE_LINE_MEMBER(dynax_state::mjdialq2_layer0_enable_w)
{
	m_layer_enable &= ~1;
	if (!state)
		m_layer_enable |= 1;
}

WRITE_LINE_MEMBER(dynax_state::mjdialq2_layer1_enable_w)
{
	m_layer_enable &= ~2;
	if (!state)
		m_layer_enable |= 2;
}


WRITE_LINE_MEMBER(dynax_state::flipscreen_w)
{
	m_flipscreen = state;
	LOG("F=%d ", state);
}




void dynax_state::dynax_blit_romregion_w(uint8_t data)
{
	if (data < 8)
		m_blitter->set_rom_bank(data);
	LOG("GFX%X ", data + 1);
}

void dynax_state::dynax_blit2_romregion_w(uint8_t data)
{
	if (data + 1 < 8)
		m_blitter2->set_rom_bank(data);
	LOG("GFX%X' ", data + 2);
}


void dynax_state::hanamai_blit_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;

	for (int layer = 0; layer < 4; layer++)
	{
		if (BIT(m_blit_dest, layer))
		{
			if (BIT(m_hanamai_priority, 4))
				m_pixmap[layer][m_hanamai_layer_half ^ m_flipscreen][offset] = data;
			else
				m_pixmap[layer][0][offset] = m_pixmap[layer][1][offset] = data;
		}
	}
}

void dynax_state::cdracula_blit_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;

	for (int layer = 0; layer < 4; layer++)
	{
		if (BIT(m_blit_dest, layer + 4))
			m_pixmap[layer][1 ^ m_flipscreen][offset] = data;
		if (BIT(m_blit_dest, layer))
			m_pixmap[layer][0 ^ m_flipscreen][offset] = data;
	}
}

void dynax_state::hnoridur_blit_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;
	if (BIT(m_blit_dest, 4))
		data |= (m_blitter->blit_pen() & 0x08) << 1;

	for (int layer = 0; layer < 4; layer++)
	{
		if (BIT(m_blit_dest, layer))
		{
			m_pixmap[layer][m_hanamai_layer_half ^ m_flipscreen][offset] = data;
			if (m_hnoridur_layer_half2)
				m_pixmap[layer][1 ^ m_hanamai_layer_half ^ m_flipscreen][offset] = data;
		}
	}
}

void dynax_state::drgpunch_blit_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;

	for (int layer = 2; layer >= 0; layer--)
	{
		if (BIT(m_blit_dest, 2 * layer + 1))
			m_pixmap[layer][1 ^ m_flipscreen][offset] = data;
		if (BIT(m_blit_dest, 2 * layer))
			m_pixmap[layer][0 ^ m_flipscreen][offset] = data;
	}
}

// two blitters/screens
void dynax_state::jantouki_blit_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;

	for (int layer = 3; layer >= 0; layer--)
	{
		if (BIT(m_blit_dest, 2 * layer + 1))
			m_pixmap[layer][1 ^ m_flipscreen][offset] = data;
		if (BIT(m_blit_dest, 2 * layer))
			m_pixmap[layer][0 ^ m_flipscreen][offset] = data;
	}
}

void dynax_state::jantouki_blit2_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;

	for (int layer = 3; layer >= 0; layer--)
	{
		if (BIT(m_blit2_dest, 2 * layer + 1))
			m_pixmap[layer + 4][1 ^ m_flipscreen][offset] = data;
		if (BIT(m_blit2_dest, 2 * layer))
			m_pixmap[layer + 4][0 ^ m_flipscreen][offset] = data;
	}
}

void dynax_state::mjdialq2_blit_pixel_w(offs_t offset, uint8_t data)
{
	if (m_flipscreen)
		offset ^= 0xffff;

	for (int layer = 0; layer < 2; layer++)
		if (BIT(m_blit_dest, layer))
			m_pixmap[layer][0][offset] = data;
}



void dynax_state::dynax_blit_scrollx_w(uint8_t data)
{
	m_blit_scroll_x = data;
}

void dynax_state::dynax_blit_scrolly_w(uint8_t data)
{
	m_blit_scroll_y = data;
}

void dynax_state::dynax_blit2_scrollx_w(uint8_t data)
{
	m_blit2_scroll_x = data;
}

void dynax_state::dynax_blit2_scrolly_w(uint8_t data)
{
	m_blit2_scroll_y = data;
}

// inverted scroll values
void dynax_state::tenkai_blit_scrollx_w(uint8_t data)
{
	m_blit_scroll_x = ((data ^ 0xff) + 1) & 0xff;
}

void dynax_state::tenkai_blit_scrolly_w(uint8_t data)
{
	m_blit_scroll_y = data ^ 0xff;
}


/***************************************************************************


                                Video Init


***************************************************************************/

//                                          0       1       2       3       4       5       6       7
static const int priority_hnoridur[8] = { 0x0231, 0x2103, 0x3102, 0x2031, 0x3021, 0x1302, 0x2310, 0x1023 };
static const int priority_mcnpshnt[8] = { 0x3210, 0x2103, 0x3102, 0x2031, 0x3021, 0x1302, 0x2310, 0x1023 };
static const int priority_mjelctrn[8] = { 0x0231, 0x0321, 0x2031, 0x2301, 0x3021, 0x3201 ,0x0000, 0x0000 }; // this game doesn't use (hasn't?) layer 1
static const int priority_mjembase[8] = { 0x0231, 0x2031, 0x0321, 0x3021, 0x2301, 0x3201 ,0x0000, 0x0000 }; // this game doesn't use (hasn't?) layer 1


void dynax_state::dynax_common_reset()
{
	m_blit_dest = -1;
	m_blit2_dest = -1;
	m_blit_palbank = 0;
	m_blit2_palbank = 0;
	m_blit_palettes = 0;
	m_blit2_palettes = 0;
	m_layer_enable = -1;
	m_blit_backpen = 0;

	m_extra_scroll_x = 0;
	m_extra_scroll_y = 0;

	m_hnoridur_layer_half2 = 0;

	m_blit_scroll_x = 0;
	m_blit2_scroll_x = 0;
	m_blit_scroll_y = 0;
	m_blit2_scroll_y = 0;
	m_hanamai_layer_half = 0;
	m_flipscreen = 0;
	m_hanamai_priority = 0;

	save_item(NAME(m_blit_dest));
	save_item(NAME(m_blit2_dest));
	save_item(NAME(m_blit_palbank));
	save_item(NAME(m_blit2_palbank));
	save_item(NAME(m_blit_palettes));
	save_item(NAME(m_blit2_palettes));
	save_item(NAME(m_layer_enable));
	save_item(NAME(m_blit_backpen));
	save_item(NAME(m_extra_scroll_x));
	save_item(NAME(m_extra_scroll_y));
	save_item(NAME(m_hnoridur_layer_half2));

	save_item(NAME(m_blit_scroll_x));
	save_item(NAME(m_blit2_scroll_x));
	save_item(NAME(m_blit_scroll_y));
	save_item(NAME(m_blit2_scroll_y));
	save_item(NAME(m_hanamai_layer_half));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_hanamai_priority));
}

VIDEO_START_MEMBER(dynax_state,hanamai)
{
	m_pixmap[0][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[0][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[3][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[3][1] = std::make_unique<uint8_t[]>(256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_HANAMAI;

	save_pointer(NAME(m_pixmap[0][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[0][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[3][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[3][1]), 256 * 256);
}

VIDEO_START_MEMBER(dynax_state,hnoridur)
{
	m_pixmap[0][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[0][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[3][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[3][1] = std::make_unique<uint8_t[]>(256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_HNORIDUR;

	m_priority_table = priority_hnoridur;

	save_pointer(NAME(m_pixmap[0][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[0][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[3][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[3][1]), 256 * 256);
}

VIDEO_START_MEMBER(dynax_state,mcnpshnt)
{
	VIDEO_START_CALL_MEMBER(hnoridur);
	m_priority_table = priority_mcnpshnt;
}

VIDEO_START_MEMBER(dynax_state,sprtmtch)
{
	m_pixmap[0][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[0][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][1] = std::make_unique<uint8_t[]>(256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_DRGPUNCH;

	save_pointer(NAME(m_pixmap[0][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[0][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][1]), 256 * 256);
}

VIDEO_START_MEMBER(dynax_state,jantouki)
{
	m_pixmap[0][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[0][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[2][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[3][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[3][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[4][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[4][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[5][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[5][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[6][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[6][1] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[7][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[7][1] = std::make_unique<uint8_t[]>(256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_JANTOUKI;

	save_pointer(NAME(m_pixmap[0][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[0][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[2][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[3][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[3][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[4][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[4][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[5][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[5][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[6][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[6][1]), 256 * 256);
	save_pointer(NAME(m_pixmap[7][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[7][1]), 256 * 256);
}

VIDEO_START_MEMBER(dynax_state,mjdialq2)
{
	m_pixmap[0][0] = std::make_unique<uint8_t[]>(256 * 256);
	m_pixmap[1][0] = std::make_unique<uint8_t[]>(256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_MJDIALQ2;

	save_pointer(NAME(m_pixmap[0][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][0]), 256 * 256);
}

VIDEO_START_MEMBER(dynax_state,mjelctrn)
{
	VIDEO_START_CALL_MEMBER(hnoridur);

	m_priority_table = priority_mjelctrn;
}

VIDEO_START_MEMBER(dynax_state,mjembase)
{
	VIDEO_START_CALL_MEMBER(hnoridur);

	m_priority_table = priority_mjembase;
}

VIDEO_START_MEMBER(dynax_state,neruton)
{
	VIDEO_START_CALL_MEMBER(hnoridur);

//  m_priority_table = priority_mjelctrn;
}

/***************************************************************************


                                Screen Drawing


***************************************************************************/

void dynax_state::hanamai_copylayer(bitmap_ind16 &bitmap, const rectangle &cliprect, int i )
{
	int color;

	switch (i)
	{
		case 0: color = (m_blit_palettes >>  0) & 0x0f;  break;
		case 1: color = (m_blit_palettes >>  4) & 0x0f;  break;
		case 2: color = (m_blit_palettes >>  8) & 0x0f;  break;
		case 3: color = (m_blit_palettes >> 12) & 0x0f;  break;
		default:    return;
	}

	color += (m_blit_palbank & 0x0f) * 16;

	int scrollx = m_blit_scroll_x;
	int scrolly = m_blit_scroll_y;

	if (i == 1 && (m_layer_layout == LAYOUT_HANAMAI  || m_layer_layout == LAYOUT_HNORIDUR))
	{
		scrollx = m_extra_scroll_x;
		scrolly = m_extra_scroll_y;
	}

	{
		uint8_t const *src1 = m_pixmap[i][1].get();
		uint8_t const *src2 = m_pixmap[i][0].get();

		int const palbase = 16 * color;

		for (int dy = 0; dy < 256; dy++)
		{
			int length, pen;
			uint16_t *dst;
			uint16_t *const dstbase = &bitmap.pix((dy - scrolly) & 0xff);

			length = scrollx;
			dst = dstbase + 2 * (256 - length);
			while (length--)
			{
				pen = *(src1++);
				if (pen) *dst = palbase + pen;
				pen = *(src2++);
				if (pen) *(dst + 1) = palbase + pen;
				dst += 2;
			}

			length = 256 - scrollx;
			dst = dstbase;
			while (length--)
			{
				pen = *(src1++);
				if (pen) *dst = palbase + pen;
				pen = *(src2++);
				if (pen) *(dst + 1) = palbase + pen;
				dst += 2;
			}
		}
	}
}


void dynax_state::jantouki_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i, int y )
{
	int color, scrollx, scrolly, palettes, palbank;

	if (i < 4)
	{
		scrollx  = m_blit_scroll_x;
		scrolly  = m_blit_scroll_y;
		palettes = m_blit_palettes;
		palbank  = m_blit_palbank;
	}
	else
	{
		scrollx  = m_blit2_scroll_x;
		scrolly  = m_blit2_scroll_y;
		palettes = m_blit2_palettes;
		palbank  = m_blit2_palbank;
	}

	switch (i % 4)
	{
		case 0: color = ((palettes  >> 12) & 0x0f) + ((palbank & 1) << 4);  break;
		case 1: color = ((palettes  >>  8) & 0x0f) + ((palbank & 1) << 4);  break;
		case 2: color = ((palettes  >>  4) & 0x0f) + ((palbank & 1) << 4);  break;
		case 3: color = ((palettes  >>  0) & 0x0f) + ((palbank & 1) << 4);  break;
		default: return;
	}

	{
		uint8_t const *src1 = m_pixmap[i][1].get();
		uint8_t const *src2 = m_pixmap[i][0].get();

		int const palbase = 16 * color;

		for (int dy = 0; dy < 256; dy++)
		{
			int length, pen;
			int const sy = ((dy - scrolly) & 0xff) + y;
			uint16_t *dst;
			uint16_t *const dstbase = &bitmap.pix(sy);

			if ((sy < cliprect.top()) || (sy > cliprect.bottom()))
			{
				src1 += 256;
				src2 += 256;
				continue;
			}

			length = scrollx;
			dst = dstbase + 2 * (256 - length);
			while (length--)
			{
				pen = *(src1++);
				if (pen) *dst = palbase + pen;
				pen = *(src2++);
				if (pen) *(dst + 1) = palbase + pen;
				dst += 2;
			}

			length = 256 - scrollx;
			dst = dstbase;
			while (length--)
			{
				pen = *(src1++);
				if (pen) *dst = palbase + pen;
				pen = *(src2++);
				if (pen) *(dst + 1) = palbase + pen;
				dst += 2;
			}
		}
	}
}


void dynax_state::mjdialq2_copylayer( bitmap_ind16 &bitmap, const rectangle &cliprect, int i )
{
	int color;

	switch (i)
	{
		case 0: color = (m_blit_palettes >> 4) & 0x0f;   break;
		case 1: color = (m_blit_palettes >> 0) & 0x0f;   break;
		default:    return;
	}

	color += (m_blit_palbank & 1) * 16;

	int const scrollx = m_blit_scroll_x;
	int const scrolly = m_blit_scroll_y;

	{
		uint8_t const *src = m_pixmap[i][0].get();

		int const palbase = 16 * color;

		for (int dy = 0; dy < 256; dy++)
		{
			int length, pen;
			uint16_t *dst;
			uint16_t *const dstbase = &bitmap.pix((dy - scrolly) & 0xff);

			length = scrollx;
			dst = dstbase + 256 - length;
			while (length--)
			{
				pen = *(src++);
				if (pen) *dst = palbase + pen;
				dst++;
			}

			length = 256 - scrollx;
			dst = dstbase;
			while (length--)
			{
				pen = *(src++);
				if (pen) *dst = palbase + pen;
				dst++;
			}
		}
	}
}

void dynax_state::hanamai_priority_w(uint8_t data)
{
	m_hanamai_priority = data;
}

void dynax_state::tenkai_priority_w(uint8_t data)
{
	m_hanamai_priority = bitswap<8>(data, 3, 2, 1, 0, 4, 7, 5, 6);
}

/*
mjembase:   priority: 00 08 10 18 20 28; enable: 1,2,4
Convert to:
mjelctrn:   priority: 00 20 10 40 30 50; enable: 1,2,8
*/
void dynax_state::mjembase_priority_w(uint8_t data)
{
	m_hanamai_priority = bitswap<8>(data, 6, 5, 4, 3, 2, 7, 1, 0);
}


int dynax_state::debug_mask()
{
#ifdef MAME_DEBUG
	int msk = 0;
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		if (machine().input().code_pressed(KEYCODE_Q))    msk |= 0x01;    // layer 0
		if (machine().input().code_pressed(KEYCODE_W))    msk |= 0x02;    // layer 1
		if (machine().input().code_pressed(KEYCODE_E))    msk |= 0x04;    // layer 2
		if (machine().input().code_pressed(KEYCODE_R))    msk |= 0x08;    // layer 3
		if (machine().input().code_pressed(KEYCODE_A))    msk |= 0x10;    // layer 4
		if (machine().input().code_pressed(KEYCODE_S))    msk |= 0x20;    // layer 5
		if (machine().input().code_pressed(KEYCODE_D))    msk |= 0x40;    // layer 6
		if (machine().input().code_pressed(KEYCODE_F))    msk |= 0x80;    // layer 7
		if (msk != 0)   return msk;
	}
#endif
	return -1;
}

/*  A primitive gfx viewer:

    T          -  Toggle viewer
    I,O        -  Change palette (-,+)
    J,K & N,M  -  Change "tile"  (-,+, slow & fast)
    R          -  move "tile" to the next 1/8th of the gfx  */
int dynax_state::debug_viewer(bitmap_ind16 &bitmap, const rectangle &cliprect )
{
#if 0 // needs rewrite
	static int toggle;
	if (machine().input().code_pressed_once(KEYCODE_T))   toggle = 1 - toggle;
	if (m_blitter_gfx.found() && toggle)
	{
		uint8_t *RAM = &m_blitter_gfx[0];
		size_t size = m_blitter_gfx.bytes();
		static int i = 0, c = 0, r = 0;

		if (machine().input().code_pressed_once(KEYCODE_I))   c = (c - 1) & 0x1f;
		if (machine().input().code_pressed_once(KEYCODE_O))   c = (c + 1) & 0x1f;
		if (machine().input().code_pressed_once(KEYCODE_R))   { r = (r + 1) & 0x7;    i = size / 8 * r; }
		if (machine().input().code_pressed(KEYCODE_M) | machine().input().code_pressed_once(KEYCODE_K))
		{
			while (i < size && RAM[i]) i++;
			while (i < size && !RAM[i]) i++;
		}
		if (machine().input().code_pressed(KEYCODE_N) | machine().input().code_pressed_once(KEYCODE_J))
		{
			if (i >= 2) i -= 2;
			while (i > 0 && RAM[i]) i--;
			i++;
		}

		m_blit_palettes = (c & 0xf) * 0x111;
		m_blit_palbank  = (c >>  4) & 1;

		bitmap.fill(0, cliprect);
		memset(m_pixmap[0][0].get(), 0, sizeof(uint8_t) * 0x100 * 0x100);

		if (m_layer_layout != LAYOUT_MJDIALQ2)
			memset(m_pixmap[0][1].get(), 0, sizeof(uint8_t) * 0x100 * 0x100);
		for (m_hanamai_layer_half = 0; m_hanamai_layer_half < 2; m_hanamai_layer_half++)
			blitter_drawgfx(0, 1, m_blitter_gfx, i, 0, cliprect.left(), cliprect.top(), 3, 0);

		if (m_layer_layout != LAYOUT_MJDIALQ2)
			hanamai_copylayer(bitmap, cliprect, 0);
		else
			mjdialq2_copylayer(bitmap, cliprect, 0);

		popmessage("%06X C%02X", i, c);

		return 1;
	}
#endif
	return 0;
}



uint32_t dynax_state::screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~m_layer_enable;
	int lay[4];

	if (debug_viewer(bitmap, cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 1) * 256, cliprect);

	/* bit 4 = display enable? */
	if (!(m_hanamai_priority & 0x10))
		return 0;

	switch (m_hanamai_priority)
	{
		default:    popmessage("unknown priority %02x", m_hanamai_priority);
		[[fallthrough]]; // FIXME: really?
		case 0x10:  lay[0] = 0; lay[1] = 1; lay[2] = 2; lay[3] = 3; break;
		case 0x11:  lay[0] = 0; lay[1] = 3; lay[2] = 2; lay[3] = 1; break;
		case 0x12:  lay[0] = 0; lay[1] = 1; lay[2] = 3; lay[3] = 2; break;
		case 0x13:  lay[0] = 0; lay[1] = 3; lay[2] = 1; lay[3] = 2; break;
		case 0x14:  lay[0] = 0; lay[1] = 2; lay[2] = 1; lay[3] = 3; break;
		case 0x15:  lay[0] = 0; lay[1] = 2; lay[2] = 3; lay[3] = 1; break;
	}

	if (BIT(layers_ctrl, lay[0]))   hanamai_copylayer(bitmap, cliprect, lay[0]);
	if (BIT(layers_ctrl, lay[1]))   hanamai_copylayer(bitmap, cliprect, lay[1]);
	if (BIT(layers_ctrl, lay[2]))   hanamai_copylayer(bitmap, cliprect, lay[2]);
	if (BIT(layers_ctrl, lay[3]))   hanamai_copylayer(bitmap, cliprect, lay[3]);
	return 0;
}


uint32_t dynax_state::screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~bitswap<8>(m_hanamai_priority, 7, 6, 5, 4, 0, 1, 2, 3);
	int lay[4];
	int pri;

	if (debug_viewer(bitmap, cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 0x0f) * 256, cliprect);

	pri = m_hanamai_priority >> 4;

	if (pri > 7)
	{
		popmessage("unknown priority %02x", m_hanamai_priority);
		pri = 0;
	}

	pri = m_priority_table[pri];
	lay[0] = (pri >> 12) & 3;
	lay[1] = (pri >>  8) & 3;
	lay[2] = (pri >>  4) & 3;
	lay[3] = (pri >>  0) & 3;

	if (BIT(layers_ctrl, lay[0]))   hanamai_copylayer(bitmap, cliprect, lay[0]);
	if (BIT(layers_ctrl, lay[1]))   hanamai_copylayer(bitmap, cliprect, lay[1]);
	if (BIT(layers_ctrl, lay[2]))   hanamai_copylayer(bitmap, cliprect, lay[2]);
	if (BIT(layers_ctrl, lay[3]))   hanamai_copylayer(bitmap, cliprect, lay[3]);

	return 0;
}


uint32_t dynax_state::screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~m_layer_enable;

	if (debug_viewer(bitmap,cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 1) * 256, cliprect);

	if (BIT(layers_ctrl, 0))   hanamai_copylayer(bitmap, cliprect, 0);
	if (BIT(layers_ctrl, 1))   hanamai_copylayer(bitmap, cliprect, 1);
	if (BIT(layers_ctrl, 2))   hanamai_copylayer(bitmap, cliprect, 2);
	return 0;
}

uint32_t dynax_state::screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = m_layer_enable;

	if (debug_viewer(bitmap, cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 1) * 256, cliprect);

//  if (BIT(layers_ctrl, 0))   jantouki_copylayer(bitmap, cliprect, 3, 0);
	if (BIT(layers_ctrl, 1))   jantouki_copylayer(bitmap, cliprect, 2, 0);
	if (BIT(layers_ctrl, 2))   jantouki_copylayer(bitmap, cliprect, 1, 0);
	if (BIT(layers_ctrl, 3))   jantouki_copylayer(bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t dynax_state::screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = m_layer_enable;

	if (debug_viewer(bitmap, cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 1) * 256, cliprect);

	if (BIT(layers_ctrl, 0))   jantouki_copylayer(bitmap, cliprect, 3, 0);
	if (BIT(layers_ctrl, 4))   jantouki_copylayer(bitmap, cliprect, 7, 0);
	if (BIT(layers_ctrl, 5))   jantouki_copylayer(bitmap, cliprect, 6, 0);
	if (BIT(layers_ctrl, 6))   jantouki_copylayer(bitmap, cliprect, 5, 0);
	if (BIT(layers_ctrl, 7))   jantouki_copylayer(bitmap, cliprect, 4, 0);
	return 0;
}


uint32_t dynax_state::screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~m_layer_enable;

	if (debug_viewer(bitmap, cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 1) * 256, cliprect);

	if (BIT(layers_ctrl, 0))   mjdialq2_copylayer(bitmap, cliprect, 0);
	if (BIT(layers_ctrl, 1))   mjdialq2_copylayer(bitmap, cliprect, 1);
	return 0;
}


uint32_t dynax_state::screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~m_layer_enable;

	if (debug_viewer(bitmap,cliprect))
		return 0;

	layers_ctrl &= debug_mask();

	bitmap.fill((m_blit_backpen & 0xff) + (m_blit_palbank & 1) * 256, cliprect);

	m_extra_scroll_y = -8;

	if (BIT(layers_ctrl, 3))   hanamai_copylayer(bitmap, cliprect, 3);
	if (BIT(layers_ctrl, 1))   hanamai_copylayer(bitmap, cliprect, 1);
	if (BIT(layers_ctrl, 2))   hanamai_copylayer(bitmap, cliprect, 2);
	if (BIT(layers_ctrl, 0))   hanamai_copylayer(bitmap, cliprect, 0);

	return 0;
}

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
#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/* x B01234 G01234 R01234 */
PALETTE_INIT_MEMBER(dynax_state,sprtmtch)
{
	const UINT8 *color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		int x = (color_prom[i] << 8) + color_prom[0x200 + i];
		/* The bits are in reverse order! */
		int r = BITSWAP8((x >>  0) & 0x1f, 7, 6, 5, 0, 1, 2, 3, 4);
		int g = BITSWAP8((x >>  5) & 0x1f, 7, 6, 5, 0, 1, 2, 3, 4);
		int b = BITSWAP8((x >> 10) & 0x1f, 7, 6, 5, 0, 1, 2, 3, 4);
		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

/* x xB0123 xG0123 xR0123 */
PALETTE_INIT_MEMBER(dynax_state,janyuki)
{
	const UINT8 *color_prom = memregion("proms")->base();
	if (!color_prom)
		return;

	for (int i = 0; i < palette.entries(); i++)
	{
		int x = (color_prom[i] << 8) + color_prom[0x200 + i];
		/* The bits are in reverse order! */
		int r = BITSWAP8((x >>  0) & 0x0f, 7, 6, 5, 4, 0, 1, 2, 3);
		int g = BITSWAP8((x >>  5) & 0x0f, 7, 6, 5, 4, 0, 1, 2, 3);
		int b = BITSWAP8((x >> 10) & 0x0f, 7, 6, 5, 4, 0, 1, 2, 3);
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

WRITE8_MEMBER(dynax_state::dynax_extra_scrollx_w)
{
	m_extra_scroll_x = data;
}

WRITE8_MEMBER(dynax_state::dynax_extra_scrolly_w)
{
	m_extra_scroll_y = data;
}


/* Destination Pen */
WRITE8_MEMBER(dynax_state::dynax_blit_pen_w)
{
	m_blit_pen = data;
	LOG(("P=%02X ", data));
}

WRITE8_MEMBER(dynax_state::dynax_blit2_pen_w)
{
	m_blit2_pen = data;
	LOG(("P'=%02X ", data));
}


/* Destination Layers */
WRITE8_MEMBER(dynax_state::dynax_blit_dest_w)
{
	m_blit_dest = data;
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit_dest = BITSWAP8(m_blit_dest ^ 0x0f, 7, 6, 5, 4, 0, 1, 2, 3);

	LOG(("D=%02X ", data));
}

WRITE8_MEMBER(dynax_state::dynax_blit2_dest_w)
{
	m_blit2_dest = data;
	LOG(("D'=%02X ", data));
}

WRITE8_MEMBER(dynax_state::tenkai_blit_dest_w)
{
	dynax_blit_dest_w(space, 0, BITSWAP8(data, 7, 6, 5, 4, 0, 1, 2, 3));
}

/*
mjelctrn:   7 d e -> 1 - 4 8
mjembase:   b d e -> - 2 4 8
*/
WRITE8_MEMBER(dynax_state::mjembase_blit_dest_w)
{
	dynax_blit_dest_w(space, 0, BITSWAP8(data, 7, 6, 5, 4, 2, 3, 1, 0));
}


/* Background Color */
WRITE8_MEMBER(dynax_state::dynax_blit_backpen_w)
{
	m_blit_backpen = data;
	LOG(("B=%02X ", data));
}


/* Layers 0&1 Palettes (Low Bits) */
WRITE8_MEMBER(dynax_state::dynax_blit_palette01_w)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit_palettes = (m_blit_palettes & 0x00ff) | ((data & 0x0f) << 12) | ((data & 0xf0) << 4);
	else
		m_blit_palettes = (m_blit_palettes & 0xff00) | data;
	LOG(("P01=%02X ", data));
}

WRITE8_MEMBER(dynax_state::tenkai_blit_palette01_w)
{
	m_blit_palettes = (m_blit_palettes & 0xff00) | data;
	LOG(("P01=%02X ", data));
}


/* Layers 4&5 Palettes (Low Bits) */
WRITE8_MEMBER(dynax_state::dynax_blit_palette45_w)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit2_palettes = (m_blit2_palettes & 0x00ff) | ((data & 0x0f) << 12) | ((data & 0xf0) << 4);
	else
		m_blit2_palettes = (m_blit2_palettes & 0xff00) | data;
	LOG(("P45=%02X ", data));
}


/* Layer 2&3 Palettes (Low Bits) */
WRITE8_MEMBER(dynax_state::dynax_blit_palette23_w)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit_palettes = (m_blit_palettes & 0xff00) | ((data & 0x0f) << 4) | ((data & 0xf0) >> 4);
	else
		m_blit_palettes = (m_blit_palettes & 0x00ff) | (data << 8);
	LOG(("P23=%02X ", data));
}

WRITE8_MEMBER(dynax_state::tenkai_blit_palette23_w)
{
	m_blit_palettes = (m_blit_palettes & 0x00ff) | ((data & 0x0f) << 12) | ((data & 0xf0) << 4);
	LOG(("P23=%02X ", data));
}

WRITE8_MEMBER(dynax_state::mjembase_blit_palette23_w)
{
	dynax_blit_palette23_w(space, offset, BITSWAP8(data, 3, 2, 1, 0, 7, 6, 5, 4), mem_mask);
}


/* Layer 6&7 Palettes (Low Bits) */
WRITE8_MEMBER(dynax_state::dynax_blit_palette67_w)
{
	if (m_layer_layout == LAYOUT_HNORIDUR)
		m_blit2_palettes = (m_blit2_palettes & 0xff00) | ((data & 0x0f) << 4) | ((data & 0xf0) >> 4);
	else
		m_blit2_palettes = (m_blit2_palettes & 0x00ff) | (data << 8);
	LOG(("P67=%02X ", data));
}


/* Layers Palettes (High Bits) */
WRITE8_MEMBER(dynax_state::dynax_blit_palbank_w)
{
	m_blit_palbank = data;
	LOG(("PB=%02X ", data));
}

WRITE8_MEMBER(dynax_state::dynax_blit2_palbank_w)
{
	m_blit2_palbank = data;
	LOG(("PB'=%02X ", data));
}


/* Which half of the layers to write to (interleaved games only) */
WRITE8_MEMBER(dynax_state::hanamai_layer_half_w)
{
	m_hanamai_layer_half = (~data) & 1;
	LOG(("H=%02X ", data));
}


/* Write to both halves of the layers (interleaved games only) */
WRITE8_MEMBER(dynax_state::hnoridur_layer_half2_w)
{
	m_hnoridur_layer_half2 = (~data) & 1;
	LOG(("H2=%02X ", data));
}

WRITE8_MEMBER(dynax_state::mjdialq2_blit_dest_w)
{
	int mask = (2 >> offset);   /* 1 or 2 */

	m_blit_dest &= ~mask;

	if (~data & 1)
		m_blit_dest |= mask;
}


/* Layers Enable */
WRITE8_MEMBER(dynax_state::dynax_layer_enable_w)
{
	m_layer_enable = data;
	LOG(("E=%02X ", data));
}

WRITE8_MEMBER(dynax_state::jantouki_layer_enable_w)
{
	int mask = 1 << (7 - offset);
	m_layer_enable = (m_layer_enable & ~mask) | ((data & 1) ? mask : 0);
	m_layer_enable |= 1;
}

WRITE8_MEMBER(dynax_state::mjdialq2_layer_enable_w)
{
	int mask = (2 >> offset);   /* 1 or 2 */

	m_layer_enable &= ~mask;
	if (~data & 1)
		m_layer_enable |= mask;
}


WRITE8_MEMBER(dynax_state::dynax_flipscreen_w)
{
	m_flipscreen = data & 1;
	if (data & ~1)
		logerror("CPU#0 PC %06X: Warning, flip screen <- %02X\n", space.device().safe_pc(), data);
	LOG(("F=%02X ", data));
}


static const char *const gfxregions[] = { "gfx1", "gfx2", "gfx3", "gfx4", "gfx5", "gfx6", "gfx7", "gfx8" };

WRITE8_MEMBER(dynax_state::dynax_blit_romregion_w)
{
	if (data < ARRAY_LENGTH(gfxregions))
		m_blit_romregion = data;
	LOG(("GFX%X ", data + 1));
}

WRITE8_MEMBER(dynax_state::dynax_blit2_romregion_w)
{
	if (data + 1 < ARRAY_LENGTH(gfxregions))
		m_blit2_romregion = data + 1;
	LOG(("GFX%X' ", data + 2));
}

/***************************************************************************

                            Blitter Data Format

    The blitter reads its commands from the gfx ROMs. They are
    instructions to draw an image pixel by pixel (in a compressed
    form) in a frame buffer.

    Fetch 1 Byte from the ROM:

    7654 ----   Pen to draw with
    ---- 3210   Command

    Other bytes may follow, depending on the command

    Commands:

    0       Stop.
    1-b     Draw 1-b pixels along X.
    c       Followed by 1 byte (N): draw N pixels along X.
    d       Followed by 2 bytes (X,N): skip X pixels, draw N pixels along X.
    e       ? unused
    f       Increment Y

***************************************************************************/


/* Plot a pixel (in the pixmaps specified by dynax_blit_dest) */
void dynax_state::blitter_plot_pixel( int layer, int mask, int x, int y, int pen, int wrap, int flags )
{
	int addr;

	if ((y > 0xff) && (!(wrap & 2))) return;    // fixes mjdialq2 & mjangels title screens
	if ((x > 0xff) && (!(wrap & 1))) return;

	x &= 0xff;  // confirmed by some mjdialq2 gfx and especially by mjfriday, which
				// uses the front layer to mask out the right side of the screen as
				// it draws stuff on the left, when it shows the girls scrolling
				// horizontally after you win.
	y &= 0xff;  // seems confirmed by mjdialq2 last picture of gal 6, but it breaks
				// mjdialq2 title screen so there's something we are missing. <fixed, see above>

	/* "Flip Screen" just means complement the coordinates to 255 */
	if (m_flipscreen)    { x ^= 0xff; y ^= 0xff; }

	/* Rotate: rotation = SWAPXY + FLIPY */
	if (flags & 0x08)   { int t = x; x = y; y = t; }

	addr = x + (y << 8);

	switch (m_layer_layout)
	{
		case LAYOUT_HANAMAI:
			if (BIT(mask, 0)) m_pixmap[layer + 0][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 1)) m_pixmap[layer + 1][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 2)) m_pixmap[layer + 2][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 3)) m_pixmap[layer + 3][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			break;

		case LAYOUT_HNORIDUR:
			if (BIT(mask, 0)) m_pixmap[layer + 0][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 1)) m_pixmap[layer + 1][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 2)) m_pixmap[layer + 2][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 3)) m_pixmap[layer + 3][m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (!m_hnoridur_layer_half2)
				break;
			if (BIT(mask, 0)) m_pixmap[layer + 0][1 ^ m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 1)) m_pixmap[layer + 1][1 ^ m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 2)) m_pixmap[layer + 2][1 ^ m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 3)) m_pixmap[layer + 3][1 ^ m_hanamai_layer_half ^ m_flipscreen][addr] = pen;
			break;

		case LAYOUT_JANTOUKI:
			if (BIT(mask, 7)) m_pixmap[layer + 3][1 ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 6)) m_pixmap[layer + 3][0 ^ m_flipscreen][addr] = pen;
		case LAYOUT_DRGPUNCH:
			if (BIT(mask, 5)) m_pixmap[layer + 2][1 ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 4)) m_pixmap[layer + 2][0 ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 3)) m_pixmap[layer + 1][1 ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 2)) m_pixmap[layer + 1][0 ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 1)) m_pixmap[layer + 0][1 ^ m_flipscreen][addr] = pen;
			if (BIT(mask, 0)) m_pixmap[layer + 0][0 ^ m_flipscreen][addr] = pen;
			break;

		case LAYOUT_MJDIALQ2:
			if (BIT(mask, 0)) m_pixmap[layer + 0][0][addr] = pen;
			if (BIT(mask, 1)) m_pixmap[layer + 1][0][addr] = pen;
			break;
	}
}

/*
    Flags:

    7654 ----   -
    ---- 3---   Rotation = SWAPXY + FLIPY
    ---- -2--   -
    ---- --1-   0 = Ignore the pens specified in ROM, draw everything with the pen supplied as parameter
    ---- ---0   Clear
*/
int dynax_state::blitter_drawgfx( int layer, int mask, const char *gfx, int src, int pen, int x, int y, int wrap, int flags )
{
	UINT8 cmd;
	UINT8 *ROM = memregion(gfx)->base();
	size_t ROM_size = memregion(gfx)->bytes();

	int sx;

	if (m_layer_layout == LAYOUT_HNORIDUR)   // e.g. yarunara
		pen = ((pen >> 4) & 0xf) | ((mask & 0x10) ? ((pen & 0x08) << 1) : 0);
	else
		pen = (pen >> 4) & 0xf;

	if (flags & 0xf4)
		popmessage("flags %02x", flags);

	if (flags & 1)
	{
		int start, len;

		/* Clear the buffer(s) starting from the given scanline and exit */

		int addr = x + (y << 8);

		if (m_flipscreen)
			start = 0;
		else
			start = addr;

		len = 0x10000 - addr;

		switch (m_layer_layout)
		{
			case LAYOUT_HANAMAI:
				if (BIT(mask, 0)) memset(&m_pixmap[layer + 0][0][start], pen, len);
				if (BIT(mask, 0)) memset(&m_pixmap[layer + 0][1][start], pen, len);
				if (BIT(mask, 1)) memset(&m_pixmap[layer + 1][0][start], pen, len);
				if (BIT(mask, 1)) memset(&m_pixmap[layer + 1][1][start], pen, len);
				if (BIT(mask, 2)) memset(&m_pixmap[layer + 2][0][start], pen, len);
				if (BIT(mask, 2)) memset(&m_pixmap[layer + 2][1][start], pen, len);
				if (BIT(mask, 3)) memset(&m_pixmap[layer + 3][0][start], pen, len);
				if (BIT(mask, 3)) memset(&m_pixmap[layer + 3][1][start], pen, len);
				break;

			case LAYOUT_HNORIDUR:
				if (BIT(mask, 0)) memset(&m_pixmap[layer + 0][m_hanamai_layer_half][start], pen, len);
				if (BIT(mask, 1)) memset(&m_pixmap[layer + 1][m_hanamai_layer_half][start], pen, len);
				if (BIT(mask, 2)) memset(&m_pixmap[layer + 2][m_hanamai_layer_half][start], pen, len);
				if (BIT(mask, 3)) memset(&m_pixmap[layer + 3][m_hanamai_layer_half][start], pen, len);
				if (!m_hnoridur_layer_half2)
					break;
				if (BIT(mask, 0)) memset(&m_pixmap[layer + 0][1 - m_hanamai_layer_half][start], pen, len);
				if (BIT(mask, 1)) memset(&m_pixmap[layer + 1][1 - m_hanamai_layer_half][start], pen, len);
				if (BIT(mask, 2)) memset(&m_pixmap[layer + 2][1 - m_hanamai_layer_half][start], pen, len);
				if (BIT(mask, 3)) memset(&m_pixmap[layer + 3][1 - m_hanamai_layer_half][start], pen, len);
				break;

			case LAYOUT_JANTOUKI:
				if (BIT(mask, 7)) memset(&m_pixmap[layer + 3][1][start], pen, len);
				if (BIT(mask, 6)) memset(&m_pixmap[layer + 3][0][start], pen, len);
			case LAYOUT_DRGPUNCH:
				if (BIT(mask, 5)) memset(&m_pixmap[layer + 2][1][start], pen, len);
				if (BIT(mask, 4)) memset(&m_pixmap[layer + 2][0][start], pen, len);
				if (BIT(mask, 3)) memset(&m_pixmap[layer + 1][1][start], pen, len);
				if (BIT(mask, 2)) memset(&m_pixmap[layer + 1][0][start], pen, len);
				if (BIT(mask, 1)) memset(&m_pixmap[layer + 0][1][start], pen, len);
				if (BIT(mask, 0)) memset(&m_pixmap[layer + 0][0][start], pen, len);
				break;

			case LAYOUT_MJDIALQ2:
				if (BIT(mask, 0)) memset(&m_pixmap[layer + 0][0][start], pen, len);
				if (BIT(mask, 1)) memset(&m_pixmap[layer + 1][0][start], pen, len);
				break;
		}

		return src;
	}

	sx = x;

	src &= 0xfffff;

	for ( ;; )
	{
		if (src >= ROM_size)
		{
			popmessage("GFXROM %s OVER %08x",gfx,src);
			LOG(("\nGFXROM %s OVER %08x",gfx,src));
			return src;
		}

		cmd = ROM[src++];
		src &= 0xfffff;
		if (!(flags & 0x02))    // Ignore the pens specified in ROM, draw everything with the pen supplied as parameter
			pen = (pen & 0xf0) | ((cmd & 0xf0) >> 4);
		cmd = (cmd & 0x0f);

		switch (cmd)
		{
		case 0xf:   // Increment Y
			/* Rotate: rotation = SWAPXY + FLIPY */
			if (flags & 0x08)
				y--;
			else
				y++;
			x = sx;
			break;

		case 0xe:   // unused ? was "change dest mask" in the "rev1" blitter
			popmessage("Blitter unknown command %06X: %02X\n", src - 1, cmd);

		case 0xd:   // Skip X pixels
			if (src >= ROM_size)
			{
				popmessage("GFXROM %s OVER %08x",gfx,src);
				LOG(("\nGFXROM %s OVER %08x",gfx,src));
				return src;
			}
			x = sx + ROM[src++];
			src &= 0xfffff;
			/* fall through into next case */

		case 0xc:   // Draw N pixels
			if (src >= ROM_size)
			{
				popmessage("GFXROM %s OVER %08x",gfx,src);
				LOG(("\nGFXROM %s OVER %08x",gfx,src));
				return src;
			}
			cmd = ROM[src++];
			src &= 0xfffff;
			/* fall through into next case */

		case 0xb:
		case 0xa:
		case 0x9:
		case 0x8:
		case 0x7:
		case 0x6:
		case 0x5:
		case 0x4:
		case 0x3:
		case 0x2:
		case 0x1:   // Draw N pixels
			while (cmd--)
				blitter_plot_pixel(layer, mask, x++, y, pen, wrap, flags);
			break;

		case 0x0:   // Stop
			return src;
		}
	}
}



void dynax_state::dynax_blitter_start(int flags )
{
	int blit_newsrc;

	LOG(("XY=%X,%X SRC=%X BLIT=%X\n", m_blit_x, m_blit_y, m_blit_src, flags));

	blit_newsrc = blitter_drawgfx(
			0,                      // layer
			m_blit_dest,     // layer mask
			gfxregions[m_blit_romregion],    // rom region
			m_blit_src,              // rom address
			m_blit_pen,          // pen
			m_blit_x, m_blit_y,           // x,y
			m_blit_wrap_enable,  // wrap around
			flags                   // flags
		);

	m_blit_src = (m_blit_src & ~0x0fffff) | (blit_newsrc & 0x0fffff);

	/* Generate an IRQ */
	if (m_update_irq_func)
	{
		m_blitter_irq = 1;
		(this->*m_update_irq_func)();
	}
}

void dynax_state::jantouki_blitter_start( int flags )
{
	int blit_newsrc;

	LOG(("XY=%X,%X SRC=%X BLIT=%X\n", m_blit_x, m_blit_y, m_blit_src, flags));

	blit_newsrc = blitter_drawgfx(
			0,                      // layer
			m_blit_dest,     // layer mask
			gfxregions[m_blit_romregion],    // rom region
			m_blit_src,              // rom address
			m_blit_pen,          // pen
			m_blit_x, m_blit_y,           // x,y
			m_blit_wrap_enable,  // wrap around
			flags                   // flags
		);

	m_blit_src = (m_blit_src & ~0x0fffff) | (blit_newsrc & 0x0fffff);

	/* Generate an IRQ */
	if (m_update_irq_func)
	{
		m_blitter_irq = 1;
		(this->*m_update_irq_func)();
	}
}

void dynax_state::jantouki_blitter2_start( int flags )
{
	int blit2_newsrc;

	LOG(("XY'=%X,%X SRC'=%X BLIT'=%02X\n", m_blit2_x, m_blit2_y, m_blit2_src, flags));

	blit2_newsrc = blitter_drawgfx(
			4,                          // layer
			m_blit2_dest,            // layer mask
			gfxregions[m_blit2_romregion],       // rom region
			m_blit2_src,                 // rom address
			m_blit2_pen,         // pen
			m_blit2_x, m_blit2_y,         // x,y
			m_blit2_wrap_enable, // wrap around
			flags                       // flags
		);

	m_blit2_src = (m_blit2_src & ~0x0fffff) | (blit2_newsrc & 0x0fffff);

	/* Generate an IRQ */
	if (m_update_irq_func)
	{
		m_blitter2_irq = 1;
		(this->*m_update_irq_func)();
	}
}



WRITE8_MEMBER(dynax_state::dynax_blit_scroll_w)
{
	switch (m_blit_src & 0xc00000)
	{
		case 0x000000:  m_blit_scroll_x = data;
						LOG(("SX=%02X ", data));
						break;
		case 0x400000:  m_blit_scroll_y = data;
						LOG(("SY=%02X ", data));
						break;
		case 0x800000:
		case 0xc00000:  m_blit_wrap_enable = data;
						LOG(("WE=%02X ", data));
						break;
	}
}

// inverted scroll values
WRITE8_MEMBER(dynax_state::tenkai_blit_scroll_w)
{
	switch (m_blit_src & 0xc00000)
	{
		case 0x000000:  m_blit_scroll_x = ((data ^ 0xff) + 1) & 0xff;
						LOG(("SX=%02X ", data));
						break;
		case 0x400000:  m_blit_scroll_y = data ^ 0xff;
						LOG(("SY=%02X ", data));
						break;
		case 0x800000:
		case 0xc00000:  m_blit_wrap_enable = data;
						LOG(("WE=%02X ", data));
						break;
	}
}

WRITE8_MEMBER(dynax_state::dynax_blit2_scroll_w)
{
	switch (m_blit2_src & 0xc00000)
	{
		case 0x000000:  m_blit2_scroll_x = data;
						LOG(("SX'=%02X ", data));
						break;
		case 0x400000:  m_blit2_scroll_y = data;
						LOG(("SY'=%02X ", data));
						break;
		case 0x800000:
		case 0xc00000:  m_blit2_wrap_enable = data;
						LOG(("WE'=%02X ", data));
						break;
	}
}

WRITE8_MEMBER(dynax_state::dynax_blitter_rev2_w)
{
	switch (offset)
	{
		case 0: dynax_blitter_start(data); break;
		case 1: m_blit_x = data; break;
		case 2: m_blit_y = data; break;
		case 3: m_blit_src = (m_blit_src & 0xffff00) | (data <<  0); break;
		case 4: m_blit_src = (m_blit_src & 0xff00ff) | (data <<  8); break;
		case 5: m_blit_src = (m_blit_src & 0x00ffff) | (data << 16); break;
		case 6: dynax_blit_scroll_w(space, 0, data); break;
	}
}

// different scroll_w
WRITE8_MEMBER(dynax_state::tenkai_blitter_rev2_w)
{
	switch (offset)
	{
		case 0: dynax_blitter_start(data); break;
		case 1: m_blit_x = data; break;
		case 2: m_blit_y = data; break;
		case 3: m_blit_src = (m_blit_src & 0xffff00) | (data <<  0); break;
		case 4: m_blit_src = (m_blit_src & 0xff00ff) | (data <<  8); break;
		case 5: m_blit_src = (m_blit_src & 0x00ffff) | (data << 16); break;
		case 6: tenkai_blit_scroll_w(space, 0, data); break;
	}
}


// two blitters/screens
WRITE8_MEMBER(dynax_state::jantouki_blitter_rev2_w)
{
	switch (offset)
	{
		case 0: jantouki_blitter_start(data); break;
		case 1: m_blit_x = data; break;
		case 2: m_blit_y = data; break;
		case 3: m_blit_src = (m_blit_src & 0xffff00) | (data <<  0); break;
		case 4: m_blit_src = (m_blit_src & 0xff00ff) | (data <<  8); break;
		case 5: m_blit_src = (m_blit_src & 0x00ffff) | (data << 16); break;
		case 6: dynax_blit_scroll_w(space, 0, data); break;
	}
}

WRITE8_MEMBER(dynax_state::jantouki_blitter2_rev2_w)
{
	switch (offset)
	{
		case 0: jantouki_blitter2_start(data); break;
		case 1: m_blit2_x = data; break;
		case 2: m_blit2_y = data; break;
		case 3: m_blit2_src = (m_blit2_src & 0xffff00) | (data <<  0); break;
		case 4: m_blit2_src = (m_blit2_src & 0xff00ff) | (data <<  8); break;
		case 5: m_blit2_src = (m_blit2_src & 0x00ffff) | (data << 16); break;
		case 6: dynax_blit2_scroll_w(space, 0, data); break;
	}
}

// first register does not trigger a blit, it sets the destination
WRITE8_MEMBER(dynax_state::cdracula_blitter_rev2_w)
{
	switch (offset)
	{
		case 0:
			LOG(("DD=%02X ", data));

			if (data & 0xf0)
			{
				hanamai_layer_half_w(space, offset, 0, mem_mask);
				dynax_blit_dest_w(space, offset, data >> 4, mem_mask);
			}
			if (data & 0x0f)
			{
				hanamai_layer_half_w(space, offset, 1, mem_mask);
				dynax_blit_dest_w(space, offset, data & 0x0f, mem_mask);
			}
			break;
		case 1: m_blit_x = data; break;
		case 2: m_blit_y = data; break;
		case 3: m_blit_src = (m_blit_src & 0xffff00) | (data <<  0); break;
		case 4: m_blit_src = (m_blit_src & 0xff00ff) | (data <<  8); break;
		case 5: m_blit_src = (m_blit_src & 0x00ffff) | (data << 16); break;
		case 6: dynax_blit_scroll_w(space, 0, data); break;
	}
}

WRITE8_MEMBER(dynax_state::dynax_blit_flags_w)
{
	LOG(("FLG=%02X ", data));

	int flags = (data & 0x08) ? 0x08 : 0x00;
	flags    |= (data & 0x10) ? 0x02 : 0x00;
	flags    |= (data & 0x80) ? 0x01 : 0x00;

	dynax_blitter_start(flags);
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
	m_blit_romregion = 0;
	m_blit2_romregion = 0;
	m_blit_dest = -1;
	m_blit2_dest = -1;
	m_blit_pen = 0x7;
	m_blit2_pen = 0x7;
	m_blit_palbank = 0;
	m_blit2_palbank = 0;
	m_blit_palettes = 0;
	m_blit2_palettes = 0;
	m_layer_enable = -1;
	m_blit_backpen = 0;

	m_extra_scroll_x = 0;
	m_extra_scroll_y = 0;

	m_hnoridur_layer_half2 = 0;

	m_update_irq_func = &dynax_state::sprtmtch_update_irq;

	m_blit_scroll_x = 0;
	m_blit2_scroll_x = 0;
	m_blit_scroll_y = 0;
	m_blit2_scroll_y = 0;
	m_blit_wrap_enable = 0;
	m_blit2_wrap_enable = 0;
	m_blit_x = 0;
	m_blit_y = 0;
	m_blit2_x = 0;
	m_blit2_y = 0;
	m_blit_src = 0;
	m_blit2_src = 0;
	m_hanamai_layer_half = 0;
	m_flipscreen = 0;
	m_hanamai_priority = 0;

	save_item(NAME(m_blit_romregion));
	save_item(NAME(m_blit2_romregion));
	save_item(NAME(m_blit_dest));
	save_item(NAME(m_blit2_dest));
	save_item(NAME(m_blit_pen));
	save_item(NAME(m_blit2_pen));
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
	save_item(NAME(m_blit_wrap_enable));
	save_item(NAME(m_blit2_wrap_enable));
	save_item(NAME(m_blit_x));
	save_item(NAME(m_blit_y));
	save_item(NAME(m_blit2_x));
	save_item(NAME(m_blit2_y));
	save_item(NAME(m_blit_src));
	save_item(NAME(m_blit2_src));
	save_item(NAME(m_hanamai_layer_half));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_hanamai_priority));
}

VIDEO_START_MEMBER(dynax_state,hanamai)
{
	m_pixmap[0][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[0][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[3][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[3][1] = auto_alloc_array(machine(), UINT8, 256 * 256);

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
	m_pixmap[0][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[0][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[3][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[3][1] = auto_alloc_array(machine(), UINT8, 256 * 256);

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
	m_pixmap[0][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[0][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][1] = auto_alloc_array(machine(), UINT8, 256 * 256);

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
	m_pixmap[0][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[0][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[2][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[3][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[3][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[4][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[4][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[5][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[5][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[6][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[6][1] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[7][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[7][1] = auto_alloc_array(machine(), UINT8, 256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_JANTOUKI;
	m_update_irq_func = &dynax_state::jantouki_update_irq;

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
	m_pixmap[0][0] = auto_alloc_array(machine(), UINT8, 256 * 256);
	m_pixmap[1][0] = auto_alloc_array(machine(), UINT8, 256 * 256);

	dynax_common_reset();
	m_layer_layout = LAYOUT_MJDIALQ2;
	m_update_irq_func = 0;

	save_pointer(NAME(m_pixmap[0][0]), 256 * 256);
	save_pointer(NAME(m_pixmap[1][0]), 256 * 256);
}

VIDEO_START_MEMBER(dynax_state,mjelctrn)
{
	VIDEO_START_CALL_MEMBER(hnoridur);

	m_priority_table = priority_mjelctrn;
	m_update_irq_func = &dynax_state::mjelctrn_update_irq;
}

VIDEO_START_MEMBER(dynax_state,mjembase)
{
	VIDEO_START_CALL_MEMBER(hnoridur);

	m_priority_table = priority_mjembase;
	m_update_irq_func = &dynax_state::mjelctrn_update_irq;
}

VIDEO_START_MEMBER(dynax_state,neruton)
{
	VIDEO_START_CALL_MEMBER(hnoridur);

//  m_priority_table = priority_mjelctrn;
	m_update_irq_func = &dynax_state::neruton_update_irq;
}

/***************************************************************************


                                Screen Drawing


***************************************************************************/

void dynax_state::hanamai_copylayer(bitmap_ind16 &bitmap, const rectangle &cliprect, int i )
{
	int color;
	int scrollx, scrolly;

	switch (i)
	{
		case 0: color = (m_blit_palettes >>  0) & 0x0f;  break;
		case 1: color = (m_blit_palettes >>  4) & 0x0f;  break;
		case 2: color = (m_blit_palettes >>  8) & 0x0f;  break;
		case 3: color = (m_blit_palettes >> 12) & 0x0f;  break;
		default:    return;
	}

	color += (m_blit_palbank & 0x0f) * 16;

	scrollx = m_blit_scroll_x;
	scrolly = m_blit_scroll_y;

	if (i == 1 && (m_layer_layout == LAYOUT_HANAMAI  || m_layer_layout == LAYOUT_HNORIDUR))
	{
		scrollx = m_extra_scroll_x;
		scrolly = m_extra_scroll_y;
	}

	{
		int dy, length, pen;
		UINT8 *src1 = m_pixmap[i][1];
		UINT8 *src2 = m_pixmap[i][0];

		int palbase = 16 * color;

		for (dy = 0; dy < 256; dy++)
		{
			UINT16 *dst;
			UINT16 *dstbase = &bitmap.pix16((dy - scrolly) & 0xff);

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
		int dy, length, pen;
		UINT8 *src1 = m_pixmap[i][1];
		UINT8 *src2 = m_pixmap[i][0];

		int palbase = 16 * color;

		for (dy = 0; dy < 256; dy++)
		{
			int sy = ((dy - scrolly) & 0xff) + y;
			UINT16 *dst;
			UINT16 *dstbase = &bitmap.pix16(sy);

			if ((sy < cliprect.min_y) || (sy > cliprect.max_y))
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
	int scrollx, scrolly;

	switch (i)
	{
		case 0: color = (m_blit_palettes >> 4) & 0x0f;   break;
		case 1: color = (m_blit_palettes >> 0) & 0x0f;   break;
		default:    return;
	}

	color += (m_blit_palbank & 1) * 16;

	scrollx = m_blit_scroll_x;
	scrolly = m_blit_scroll_y;

	{
		int dy, length, pen;
		UINT8 *src = m_pixmap[i][0];

		int palbase = 16 * color;

		for (dy = 0; dy < 256; dy++)
		{
			UINT16 *dst;
			UINT16 *dstbase = &bitmap.pix16((dy - scrolly) & 0xff);

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

WRITE8_MEMBER(dynax_state::hanamai_priority_w)
{
	m_hanamai_priority = data;
}

WRITE8_MEMBER(dynax_state::tenkai_priority_w)
{
	m_hanamai_priority = BITSWAP8(data, 3, 2, 1, 0, 4, 7, 5, 6);
}

/*
mjembase:   priority: 00 08 10 18 20 28; enable: 1,2,4
Convert to:
mjelctrn:   priority: 00 20 10 40 30 50; enable: 1,2,8
*/
WRITE8_MEMBER(dynax_state::mjembase_priority_w)
{
	m_hanamai_priority = BITSWAP8(data, 6, 5, 4, 3, 2, 7, 1, 0);
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
#ifdef MAME_DEBUG
	static int toggle;
	if (machine().input().code_pressed_once(KEYCODE_T))   toggle = 1 - toggle;
	if (toggle)
	{
		UINT8 *RAM = memregion( "gfx1" )->base();
		size_t size = memregion( "gfx1" )->bytes();
		static int i = 0, c = 0, r = 0;

		if (machine().input().code_pressed_once(KEYCODE_I))   c = (c - 1) & 0x1f;
		if (machine().input().code_pressed_once(KEYCODE_O))   c = (c + 1) & 0x1f;
		if (machine().input().code_pressed_once(KEYCODE_R))   { r = (r + 1) & 0x7;    i = size / 8 * r; }
		if (machine().input().code_pressed(KEYCODE_M) | machine().input().code_pressed_once(KEYCODE_K))
		{
			while (i < size && RAM[i]) i++;     while (i < size && !RAM[i]) i++;
		}
		if (machine().input().code_pressed(KEYCODE_N) | machine().input().code_pressed_once(KEYCODE_J))
		{
			if (i >= 2) i -= 2; while (i > 0 && RAM[i]) i--;    i++;
		}

		m_blit_palettes = (c & 0xf) * 0x111;
		m_blit_palbank  = (c >>  4) & 1;

		bitmap.fill(0, cliprect);
		memset(m_pixmap[0][0], 0, sizeof(UINT8) * 0x100 * 0x100);

		if (m_layer_layout != LAYOUT_MJDIALQ2)
			memset(m_pixmap[0][1], 0, sizeof(UINT8) * 0x100 * 0x100);
		for (m_hanamai_layer_half = 0; m_hanamai_layer_half < 2; m_hanamai_layer_half++)
			blitter_drawgfx(0, 1, "gfx1", i, 0, cliprect.min_x, cliprect.min_y, 3, 0);

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



UINT32 dynax_state::screen_update_hanamai(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


UINT32 dynax_state::screen_update_hnoridur(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = ~BITSWAP8(m_hanamai_priority, 7, 6, 5, 4, 0, 1, 2, 3);
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


UINT32 dynax_state::screen_update_sprtmtch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

UINT32 dynax_state::screen_update_jantouki_top(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

UINT32 dynax_state::screen_update_jantouki_bottom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


UINT32 dynax_state::screen_update_mjdialq2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


UINT32 dynax_state::screen_update_cdracula(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

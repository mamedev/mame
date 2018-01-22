// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/**********************************************************************

 General Instruments AY-3-8900-1 a.k.a. Standard Television Interface Chip
 (STIC) emulation for Mattel Intellivision

 *********************************************************************/

#include "emu.h"
#include "video/stic.h"
#include "screen.h"


/****************************************************************************
 *                                                                          *
 * MXR      0000[..0007]    MOB X REGISTER                  RW              *
 *                                                                          *
 * (MSB)                                       (LSB)                        *
 * +-+-+-+-+-+-----+---+---+--+--+--+--+--+--+--+--+                        *
 * | | | | | |XSIZE|VIS|COL|X7|X6|X5|X4|X3|X2|X1|X0|                        *
 * +-+-+-+-+-+-----+---+---+--+--+--+--+--+--+--+--+                        *
 *                                                                          *
 * Xn       x-position                      [0..255]                        *
 * COL      collision detection             [1=enable]                      *
 * VIS      visibility                      [1=visible]                     *
 * XSIZE    double width                    [1=double]                      *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. If X=0, the sprite is not visible and does not register collisions.  *
 *  2. The horizontal handle is 8 pixels from the left of the sprite,       *
 *     regardless of width.                                                 *
 *                                                                          *
 ****************************************************************************/
#define STIC_MXR                    0x0000

#define STIC_MXR_XSIZE              0x0400
#define STIC_MXR_VIS                0x0200
#define STIC_MXR_COL                0x0100
#define STIC_MXR_X                  0x00FF

/****************************************************************************
 *                                                                          *
 * MYR      0008[..000F]    MOB Y REGISTER                  RW              *
 *                                                                          *
 * (MSB)                                                 (LSB)              *
 * +-+-+-+-+-----+-----+-----+-----+----+--+--+--+--+--+--+--+              *
 * | | | | |YFLIP|XFLIP|YSIZE|YFULL|YRES|Y6|Y5|Y4|Y3|Y2|Y1|Y0|              *
 * +-+-+-+-+-----+-----+-----+-----+----+--+--+--+--+--+--+--+              *
 *                                                                          *
 * Yn       y-position                      [0..127]                        *
 * YRES     y-resolution                    [0=8 scanlines,1=16 scanlines]  *
 * YFULL    double scanline height          [1=double]                      *
 * YSIZE    quadruple scanline height       [1=quadruple]                   *
 * XFLIP    horizontally flip image         [1=flip]                        *
 * YFLIP    vertically flip image           [1=flip]                        *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. A sprite with YRES=1 will display two cards.                         *
 *  2. A sprite with YFULL=0 and YSIZE=0 will have scanlines half the       *
 *     height of a background card.                                         *
 *  3, The minimum size of a sprite is 8x8 (XSIZE=YRES=YSIZE=YFULL=0).      *
 *  4. The maximum size of a sprite is 16x128 (XSIZE=YRES=YSIZE=YFULL=1).   *
 *  5. The Y-position is measured in double height scanlines.               *
 *  6. The vertical handle is 8 double height scanlines from the top of the *
 *     sprite, regardless of height.                                        *
 *                                                                          *
 ****************************************************************************/
#define STIC_MYR                    0x0008

#define STIC_MYR_YFLIP              0x0800
#define STIC_MYR_XFLIP              0x0400
#define STIC_MYR_YSIZE              0x0200
#define STIC_MYR_YFULL              0x0100
#define STIC_MYR_YRES               0x0080
#define STIC_MYR_Y                  0x007F

/****************************************************************************
 *                                                                          *
 * SAR      0010[..0017]    MOB ATTRIBUTE REGISTER          RW              *
 *                                                                          *
 * (MSB)                                         (LSB)                      *
 * +-+-+---+---+---+-+-+--+--+--+--+--+--+---+---+---+                      *
 * | | |PRI|FG3|SEL| | |C5|C4|C3|C2|C1|C0|FG2|FG1|FG0|                      *
 * +-+-+---+---+---+-+-+--+--+--+--+--+--+---+---+---+                      *
 *                                                                          *
 * FGn      foreground color                [0..15]                         *
 * Cn       card #                          [0..63]                         *
 * SEL      card memory select              [0=GROM, 1=GRAM]                *
 * PRI      sprite priority                 [1=behind set background bit]   *
 *                                                                          *
 ****************************************************************************/
#define STIC_MAR                    0x0010

#define STIC_MAR_PRI                0x2000
#define STIC_MAR_FG3                0x1000
#define STIC_MAR_SEL                0x0800
#define STIC_MAR_C                  0x07F8
#define STIC_MAR_FG20               0x0007

/****************************************************************************
 *                                                                          *
 * SCR      0018[..001F]    MOB COLLISION REGISTER          RW              *
 *                                                                          *
 * (MSB)                                                     (LSB)          *
 * +-+-+-+-+-+-+----+----+----+----+----+----+----+----+----+----+          *
 * | | | | | | |BRDR|BKGD|SPR7|SPR6|SPR5|SPR4|SPR3|SPR2|SPR1|SPR0|          *
 * +-+-+-+-+-+-+----+----+----+----+----+----+----+----+----+----+          *
 *                                                                          *
 * SPRn         1=collision with sprite #n                                  *
 * BKGD         1=collision with set background bit                         *
 * BRDR         1=collision with screen border                              *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. All collisions are latched.  Successive reads read from the latch.   *
 *     A write will reset the latch.                                        *
 *  2. Sprites with VIS=0 register collisions.                              *
 *  3. Sprites with X=0 do not register collisions.                         *
 *  4. Two overlapping sprites with different priorities, one completely    *
 *     hidden behind the background, still register a collision.            *
 *                                                                          *
 ****************************************************************************/
#define STIC_MCR                    0x0018

#define STIC_MCR_BRDR               0x0200
#define STIC_MCR_BKGD               0x0100
#define STIC_MCR_SPR7               0x0080
#define STIC_MCR_SPR6               0x0040
#define STIC_MCR_SPR5               0x0020
#define STIC_MCR_SPR4               0x0010
#define STIC_MCR_SPR3               0x0008
#define STIC_MCR_SPR2               0x0004
#define STIC_MCR_SPR1               0x0002
#define STIC_MCR_SPR0               0x0001

/****************************************************************************
 *                                                                          *
 * DER      0021            DISPLAY ENABLE REGISTER         W               *
 *                                                                          *
 * (MSB)                       (LSB)                                        *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                                        *
 * | | | | | | | | | | | | | | | | |                                        *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                                        *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. Any write during VBLANK enables STIC output.                         *
 *                                                                          *
 ****************************************************************************/
#define STIC_DER                    0x0020

/****************************************************************************
 *                                                                          *
 * GMR      0021            GRAPHICS MODE REGISTER          RW              *
 *                                                                          *
 * (MSB)                       (LSB)                                        *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                                        *
 * | | | | | | | | | | | | | | | | |                                        *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                                        *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. Any write during VBLANK enables FOREGROUND/BACKGROUND mode.          *
 *  2. Any read during VBLANK enables COLOR STACK/COLORED SQUARES mode.     *
 *                                                                          *
 ****************************************************************************/
#define STIC_GMR                    0x0021

/****************************************************************************
 *                                                                          *
 * CSR      0028            COLOR STACK REGISTER            RW              *
 *                                                                          *
 * (MSB)                               (LSB)                                *
 * +-+-+-+-+-+-+-+-+-+-+-+-+---+---+---+---+                                *
 * | | | | | | | | | | | | |BG3|BG2|BG1|BG0|                                *
 * +-+-+-+-+-+-+-+-+-+-+-+-+---+---+---+---+                                *
 *                                                                          *
 * BGn      background color                [0..15]                         *
 *                                                                          *
 ****************************************************************************/
#define STIC_CSR                    0x0028

#define STIC_CSR_BG                 0x000F

/****************************************************************************
 *                                                                          *
 * BCR      002C            BORDER COLOR REGISTER           RW              *
 *                                                                          *
 * (MSB)                               (LSB)                                *
 * +-+-+-+-+-+-+-+-+-+-+-+-+---+---+---+---+                                *
 * | | | | | | | | | | | | |BC3|BC2|BC1|BC0|                                *
 * +-+-+-+-+-+-+-+-+-+-+-+-+---+---+---+---+                                *
 *                                                                          *
 * BCn      border color                    [0..15]                         *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. Affects overscan, and column and row blockouts.                      *
 *                                                                          *
 ****************************************************************************/
#define STIC_BCR                    0x002C

#define STIC_BCR_BC                 0x000F

/****************************************************************************
 *                                                                          *
 * HDR      0030            HORIZONTAL DELAY REGISTER       RW              *
 *                                                                          *
 * (MSB)                                (LSB)                               *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+----+----+----+                               *
 * | | | | | | | | | | | | | |DEL2|DEL1|DEL0|                               *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+----+----+----+                               *
 *                                                                          *
 * DELn     horizontal delay                [0..7]                          *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. Affects both BACKTAB and MOBs.                                       *
 *                                                                          *
 ****************************************************************************/
#define STIC_HDR                    0x0030

#define STIC_HDR_DEL                0x0007

/****************************************************************************
 *                                                                          *
 * STIC_VDR     0030            VERTICAL DELAY REGISTER         RW          *
 *                                                                          *
 * (MSB)                                (LSB)                               *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+----+----+----+                               *
 * | | | | | | | | | | | | | |DEL2|DEL1|DEL0|                               *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+----+----+----+                               *
 *                                                                          *
 * DELn     vertical delay                  [0..7]                          *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. Affects both BACKTAB and MOBs.                                       *
 *                                                                          *
 ****************************************************************************/
#define STIC_VDR                    0x0031

#define STIC_VDR_DEL                0x0007

/****************************************************************************
 *                                                                          *
 * STIC_CBR     0032            CARD BLOCKOUT REGISTER          RW          *
 *                                                                          *
 * (MSB)                           (LSB)                                    *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+---+---+                                    *
 * | | | | | | | | | | | | | | |ROW|COL|                                    *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+---+---+                                    *
 *                                                                          *
 * COL      blockout first card column      [1=blockout]                    *
 * ROW      blockout first card row         [1=blockout]                    *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *  NOTES                                                                   *
 *                                                                          *
 *  1. Generally used in conjunction with HDR and VDR registers, to achieve *
 *     smooth scrolling.                                                    *
 *                                                                          *
 ****************************************************************************/
#define STIC_CBR                    0x0032

#define STIC_CBR_ROW                0x0002
#define STIC_CBR_COL                0x0001

/****************************************************************************
 *                                                                          *
 * FOREGROUND/BACKGROUND MODE                                               *
 *                                                                          *
 * (MSB)                                             (LSB)                  *
 * +-+-+---+---+---+---+---+--+--+--+--+--+--+---+---+---+                  *
 * | | |BG2|BG3|SEL|BG1|BG0|C5|C4|C3|C2|C1|C0|FG2|FG1|FG0|                  *
 * +-+-+---+---+---+---+---+--+--+--+--+--+--+---+---+---+                  *
 *                                                                          *
 * FGn      foreground color                [0..7]                          *
 * Cn       card #                          [0..63]                         *
 * SEL      card memory select              [0=GROM, 1=GRAM]                *
 * BGn      background color                [0..15]                         *
 *                                                                          *
 ****************************************************************************/
#define STIC_FBM_BG2                0x2000
#define STIC_FBM_BG310              0x1600
#define STIC_FBM_SEL                0x0800
#define STIC_FBM_C                  0x01F8
#define STIC_FBM_FG                 0x0007

/****************************************************************************
 *                                                                          *
 * COLOR STACK MODE                                                         *
 *                                                                          *
 * (MSB)                                           (LSB)                    *
 * +-+-+---+---+---+--+--+--+--+--+--+--+--+---+---+---+                    *
 * | | |ADV|FG3|SEL|C7|C6|C5|C4|C3|C2|C1|C0|FG2|FG1|FG0|                    *
 * +-+-+---+---+---+--+--+--+--+--+--+--+--+---+---+---+                    *
 *                                                                          *
 * FGn      foreground color                [0..15]                         *
 * Cn       card #                          [GROM=0..212, GRAM=0..63]       *
 * SEL      card memory select              [0=GROM, 1=GRAM]                *
 * ADV      advance color stack index       [1=advance]                     *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 * NOTES:                                                                   *
 *                                                                          *
 * 1. When FG3=1 and SEL=0, COLORED SQUARES MODE is enabled.                *
 * 2. When SEL=1, C7 and C6 can be used as user-defined flags.              *
 *                                                                          *
 ****************************************************************************/
#define STIC_CSTM_ADV               0x2000
#define STIC_CSTM_FG3               0x1000
#define STIC_CSTM_SEL               0x0800
#define STIC_CSTM_C7                0x0400
#define STIC_CSTM_C6                0x0200
#define STIC_CSTM_C50               0x01F8
#define STIC_CSTM_FG20              0x0007

#define STIC_CSTM_C                 (STIC_CSTM_C7|STIC_CSTM_C6|STIC_CSTM_C50)

/****************************************************************************
 *                                                                          *
 * COLORED SQUARES MODE                                                     *
 *                                                                          *
 * (MSB)                                   (LSB)                            *
 * +-+-+--+-+-+--+--+--+--+--+--+--+--+--+--+--+                            *
 * | | |D2|1|0|D1|D0|C2|C1|C0|B2|B1|B0|A2|A1|A0|                            *
 * +-+-+--+-+-+--+--+--+--+--+--+--+--+--+--+--+                            *
 *                                                                          *
 * An       color a                         [0..7]                          *
 * Bn       color b                         [0..7]                          *
 * Cn       color c                         [0..7]                          *
 * Dn       color d                         [0..7]                          *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 * NOTES:                                                                   *
 *                                                                          *
 * 1. Each color corresponds to one of the following 4 x 4 squares:         *
 *                                                                          *
 *      +---+---+                                                           *
 *      | a | b |                                                           *
 *      +---+---+                                                           *
 *      | c | d |                                                           *
 *      +---+---+                                                           *
 *                                                                          *
 * 2. When color 7 is specified, the color is taken from the color stack.   *
 *                                                                          *
 ****************************************************************************/
#define STIC_CSQM_D2                0x2000
#define STIC_CSQM_D10               0x0600
#define STIC_CSQM_C                 0x01C0
#define STIC_CSQM_B                 0x0038
#define STIC_CSQM_A                 0x0007

#define STIC_CSQM_WIDTH             (CARD_WIDTH / 2)
#define STIC_CSQM_HEIGHT            (CARD_HEIGHT / 2)



DEFINE_DEVICE_TYPE(STIC, stic_device, "stic", "AY-3-8900-1 STIC");

//-------------------------------------------------
//  stic_device - constructor
//-------------------------------------------------

stic_device::stic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, STIC, tag, owner, clock),
	device_video_interface(mconfig, *this),
	m_grom(*this, "grom"),
	m_x_scale(1),
	m_y_scale(1)
{
}


//-------------------------------------------------
//  ~stic_device - destructor
//-------------------------------------------------

stic_device::~stic_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void stic_device::device_start()
{
	screen().register_screen_bitmap(m_bitmap);

	save_item(NAME(m_stic_registers));
	save_item(NAME(m_gramdirty));
	save_item(NAME(m_gram));
	save_item(NAME(m_gramdirtybytes));
	save_item(NAME(m_color_stack_mode));
	save_item(NAME(m_color_stack_offset));
	save_item(NAME(m_stic_handshake));
	save_item(NAME(m_border_color));
	save_item(NAME(m_col_delay));
	save_item(NAME(m_row_delay));
	save_item(NAME(m_left_edge_inhibit));
	save_item(NAME(m_top_edge_inhibit));
	save_item(NAME(m_backtab_buffer));
	for (int sp = 0; sp < MOBS; sp++)
	{
		save_item(m_sprite[sp].visible, "STIC sprite/m_sprite[sp].visible", sp);
		save_item(m_sprite[sp].xpos, "STIC sprite/m_sprite[sp].xpos", sp);
		save_item(m_sprite[sp].ypos, "STIC sprite/m_sprite[sp].ypos", sp);
		save_item(m_sprite[sp].coll, "STIC sprite/m_sprite[sp].coll", sp);
		save_item(m_sprite[sp].collision, "STIC sprite/m_sprite[sp].collision", sp);
		save_item(m_sprite[sp].doublex, "STIC sprite/m_sprite[sp].doublex", sp);
		save_item(m_sprite[sp].doubley, "STIC sprite/m_sprite[sp].doubley", sp);
		save_item(m_sprite[sp].quady, "STIC sprite/m_sprite[sp].quady", sp);
		save_item(m_sprite[sp].xflip, "STIC sprite/m_sprite[sp].xflip", sp);
		save_item(m_sprite[sp].yflip, "STIC sprite/m_sprite[sp].yflip", sp);
		save_item(m_sprite[sp].behind_foreground, "STIC sprite/m_sprite[sp].behind_foreground", sp);
		save_item(m_sprite[sp].grom, "STIC sprite/m_sprite[sp].grom", sp);
		save_item(m_sprite[sp].card, "STIC sprite/m_sprite[sp].card", sp);
		save_item(m_sprite[sp].color, "STIC sprite/m_sprite[sp].color", sp);
		save_item(m_sprite[sp].doubleyres, "STIC sprite/m_sprite[sp].doubleyres", sp);
		save_item(m_sprite[sp].dirty, "STIC sprite/m_sprite[sp].dirty", sp);
		save_item(m_sprite_buffers[sp], "STIC sprite/m_sprite[sp].sprite_buffers", sp);
	}
}

void stic_device::device_reset()
{
	for (int i = 0; i < MOBS; i++)
	{
		intv_sprite_type* s = &m_sprite[i];
		s->visible = 0;
		s->xpos = 0;
		s->ypos = 0;
		s->coll = 0;
		s->collision = 0;
		s->doublex = 0;
		s->doubley = 0;
		s->quady = 0;
		s->xflip = 0;
		s->yflip = 0;
		s->behind_foreground = 0;
		s->grom = 0;
		s->card = 0;
		s->color = 0;
		s->doubleyres = 0;
		s->dirty = 1;
		for (int j = 0; j < 16; j++)
		{
			for (int k = 0; k < 128; k++)
			{
				m_sprite_buffers[i][j][k] = 0;
			}
		}
	}

	memset(m_stic_registers, 0, sizeof(m_stic_registers));
	m_gramdirty = 1;
	for (int i = 0; i < 64; i++)
	{
		m_gram[i] = 0;
		m_gramdirtybytes[i] = 1;
	}

	m_color_stack_mode = 0;
	m_color_stack_offset = 0;
	m_stic_handshake = 0;
	m_border_color = 0;
	m_col_delay = 0;
	m_row_delay = 0;
	m_left_edge_inhibit = 0;
	m_top_edge_inhibit = 0;
}

ROM_START( stic_grom )
	ROM_REGION( 0x800, "grom", ROMREGION_ERASEFF )
	ROM_LOAD( "ro-3-9503-003.u21", 0, 0x0800, CRC(683a4158) SHA1(f9608bb4ad1cfe3640d02844c7ad8e0bcd974917))
ROM_END

const tiny_rom_entry *stic_device::device_rom_region() const
{
	return ROM_NAME( stic_grom );
}



#define FOREGROUND_BIT 0x0010

// conversion from Intellivision color to internal representation
#define SET_COLOR(c)    ((c * 2) + 1)
#define GET_COLOR(c)    ((c - 1) / 2)

/* initialized to non-zero, because we divide by it */

void stic_device::intv_set_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color)
{
	int w, h;

	// output scaling
	x *= m_x_scale;
	y *= m_y_scale;
	color = SET_COLOR(color);

	for (h = 0; h < m_y_scale; h++)
		for (w = 0; w < m_x_scale; w++)
			bitmap.pix16(y + h, x + w) = color;
}

uint32_t stic_device::intv_get_pixel(bitmap_ind16 &bitmap, int x, int y)
{
	return GET_COLOR(bitmap.pix16(y * m_y_scale, x * m_x_scale));
}

void stic_device::intv_plot_box(bitmap_ind16 &bitmap, int x, int y, int w, int h, int color)
{
	bitmap.plot_box(x * m_x_scale, y * m_y_scale, w * m_x_scale, h * m_y_scale, SET_COLOR(color));
}


bool stic_device::sprites_collide(int spriteNum1, int spriteNum2)
{
	int16_t x0, y0, w0, h0, x1, y1, w1, h1, x2, y2, w2, h2;

	intv_sprite_type* s1 = &m_sprite[spriteNum1];
	intv_sprite_type* s2 = &m_sprite[spriteNum2];

	x0 = OVERSCAN_LEFT_WIDTH + m_col_delay - CARD_WIDTH;
	y0 = OVERSCAN_TOP_HEIGHT + m_row_delay - CARD_HEIGHT;
	x1 = (s1->xpos + x0) * X_SCALE; y1 = (s1->ypos + y0) * Y_SCALE;
	x2 = (s2->xpos + x0) * X_SCALE; y2 = (s2->ypos + y0) * Y_SCALE;
	w1 = (s1->doublex ? 2 : 1) * CARD_WIDTH;
	w2 = (s2->doublex ? 2 : 1) * CARD_WIDTH;
	h1 = (s1->quady ? 4 : 1) * (s1->doubley ? 2 : 1) * (s1->doubleyres ? 2 : 1) * CARD_HEIGHT;
	h2 = (s2->quady ? 4 : 1) * (s2->doubley ? 2 : 1) * (s2->doubleyres ? 2 : 1) * CARD_HEIGHT;

	if ((x1 >= x2 + w2) || (y1 >= y2 + h2) ||
		(x2 >= x1 + w1) || (y2 >= y1 + h1))
		return false;

	// iterate over the intersecting bits to see if any touch
	x0 = std::max(x1, x2);
	y0 = std::max(y1, y2);
	w0 = std::min(x1 + w1, x2 + w2) - x0;
	h0 = std::min(y1 + h1, y2 + h2) - y0;
	x1 = x0 - x1;
	y1 = y0 - y1;
	x2 = x0 - x2;
	y2 = y0 - y2;
	for (x0 = 0; x0 < w0; x0++)
	{
		for (y0 = 0; y0 < h0; y0++)
		{
			if (m_sprite_buffers[spriteNum1][x0 + x1][y0 + y1] &&
				m_sprite_buffers[spriteNum2][x0 + x2][y0 + y2])
				return true;
		}
	}

	return false;
}

void stic_device::determine_sprite_collisions()
{
	// check sprite to sprite collisions
	for (int i = 0; i < MOBS - 1; i++)
	{
		intv_sprite_type* s1 = &m_sprite[i];
		if (s1->xpos == 0 || !s1->coll)
			continue;

		for (int j = i + 1; j < MOBS; j++)
		{
			intv_sprite_type* s2 = &m_sprite[j];
			if (s2->xpos == 0 || !s2->coll)
				continue;

			if (sprites_collide(i, j))
			{
				s1->collision |= (1 << j);
				s2->collision |= (1 << i);
			}
		}
	}
}

void stic_device::render_sprites()
{
	int32_t cardMemoryLocation, pixelSize;
	int32_t spritePixelHeight;
	int32_t nextMemoryLocation;
	int32_t nextData;
	int32_t nextX;
	int32_t nextY;
	int32_t xInc;

	for (int i = 0; i < MOBS; i++)
	{
		intv_sprite_type* s = &m_sprite[i];

		if (s->grom)
			cardMemoryLocation = (s->card * CARD_HEIGHT);
		else
			cardMemoryLocation = ((s->card & 0x003F) * CARD_HEIGHT);

		pixelSize = (s->quady ? 4 : 1) * (s->doubley ? 2 : 1);
		spritePixelHeight = pixelSize * (s->doubleyres ? 2 : 1) * CARD_HEIGHT;

		for (int j = 0; j < spritePixelHeight; j++)
		{
			nextMemoryLocation = (cardMemoryLocation + (j/pixelSize));
			if (s->grom)
				nextData = m_grom[nextMemoryLocation];
			else if (nextMemoryLocation < 0x200)
				nextData = m_gram[nextMemoryLocation];
			else
				nextData = 0xFFFF;
			nextX = (s->xflip ? ((s->doublex ? 2 : 1) * CARD_WIDTH - 1) : 0);
			nextY = (s->yflip ? (spritePixelHeight - j - 1) : j);
			xInc = (s->xflip ? -1: 1);

			for (int k = 0; k < CARD_WIDTH * (1 + s->doublex); k++)
			{
				m_sprite_buffers[i][nextX + k * xInc][nextY] = (nextData & (1 << ((CARD_WIDTH - 1) - k / (1 + s->doublex)))) != 0;
			}
		}
	}
}

void stic_device::render_line(bitmap_ind16 &bitmap, uint8_t nextByte, uint16_t x, uint16_t y, uint8_t fgcolor, uint8_t bgcolor)
{
	uint32_t color;

	for (int i = 0; i < CARD_WIDTH; i++)
	{
		color = (nextByte & (1 << ((CARD_WIDTH - 1) - i)) ? fgcolor : bgcolor);
		intv_set_pixel(bitmap, x+i, y, color);
		intv_set_pixel(bitmap, x+i, y+1, color);
	}
}

void stic_device::render_colored_squares(bitmap_ind16 &bitmap, uint16_t x, uint16_t y, uint8_t color0, uint8_t color1, uint8_t color2, uint8_t color3)
{
	intv_plot_box(bitmap, x, y, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, color0);
	intv_plot_box(bitmap, x + STIC_CSQM_WIDTH * X_SCALE, y, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, color1);
	intv_plot_box(bitmap, x, y + STIC_CSQM_HEIGHT * Y_SCALE, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, color2);
	intv_plot_box(bitmap, x + STIC_CSQM_WIDTH * X_SCALE, y + STIC_CSQM_HEIGHT * Y_SCALE, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, color3);
}

void stic_device::render_color_stack_mode(bitmap_ind16 &bitmap)
{
	int16_t w, h, nextx, nexty;
	uint8_t csPtr = 0;
	uint16_t nextCard;

	for (h = 0, nexty = (OVERSCAN_TOP_HEIGHT + m_row_delay) * Y_SCALE;
			h < BACKTAB_HEIGHT;
			h++, nexty += CARD_HEIGHT * Y_SCALE)
	{
		for (w = 0, nextx = (OVERSCAN_LEFT_WIDTH + m_col_delay) * X_SCALE;
				w < BACKTAB_WIDTH;
				w++, nextx += CARD_WIDTH * X_SCALE)
		{
			nextCard = m_backtab_buffer[h][w];

			// colored squares mode
			if ((nextCard & (STIC_CSTM_FG3|STIC_CSTM_SEL)) == STIC_CSTM_FG3)
			{
				uint8_t csColor = m_stic_registers[STIC_CSR + csPtr];
				uint8_t color0 = nextCard & STIC_CSQM_A;
				uint8_t color1 = (nextCard & STIC_CSQM_B) >> 3;
				uint8_t color2 = (nextCard & STIC_CSQM_C) >> 6;
				uint8_t color3 = ((nextCard & STIC_CSQM_D2) >> 11) |
				((nextCard & (STIC_CSQM_D10)) >> 9);
				render_colored_squares(bitmap, nextx, nexty,
										(color0 == 7 ? csColor : (color0 | FOREGROUND_BIT)),
										(color1 == 7 ? csColor : (color1 | FOREGROUND_BIT)),
										(color2 == 7 ? csColor : (color2 | FOREGROUND_BIT)),
										(color3 == 7 ? csColor : (color3 | FOREGROUND_BIT)));
			}
			//color stack mode
			else
			{
				uint8_t isGrom;
				uint16_t memoryLocation, fgcolor, bgcolor;
				uint8_t* memory;

				//advance the color pointer, if necessary
				if (nextCard & STIC_CSTM_ADV)
					csPtr = (csPtr+1) & (CSRS - 1);

				fgcolor = ((nextCard & STIC_CSTM_FG3) >> 9) |
				(nextCard & (STIC_CSTM_FG20)) | FOREGROUND_BIT;
				bgcolor = m_stic_registers[STIC_CSR + csPtr] & STIC_CSR_BG;

				isGrom = !(nextCard & STIC_CSTM_SEL);
				if (isGrom)
				{
					memoryLocation = nextCard & STIC_CSTM_C;
					memory = m_grom;
					for (int j = 0; j < CARD_HEIGHT; j++)
						render_line(bitmap, memory[memoryLocation + j],
									nextx, nexty + j * Y_SCALE, fgcolor, bgcolor);
				}
				else
				{
					memoryLocation = nextCard & STIC_CSTM_C50;
					memory = m_gram;
					for (int j = 0; j < CARD_HEIGHT; j++)
						render_line(bitmap, memory[memoryLocation + j],
									nextx, nexty + j * Y_SCALE, fgcolor, bgcolor);
				}
			}
		}
	}
}

void stic_device::render_fg_bg_mode(bitmap_ind16 &bitmap)
{
	int16_t w, h, nextx, nexty;
	uint8_t isGrom, fgcolor, bgcolor;
	uint16_t nextCard, memoryLocation;
	uint8_t* memory;

	for (h = 0, nexty = (OVERSCAN_TOP_HEIGHT + m_row_delay) * Y_SCALE;
			h < BACKTAB_HEIGHT;
			h++, nexty += CARD_HEIGHT * Y_SCALE)
	{
		for (w = 0, nextx = (OVERSCAN_LEFT_WIDTH + m_col_delay) * X_SCALE;
				w < BACKTAB_WIDTH;
				w++, nextx += CARD_WIDTH * X_SCALE)
		{
			nextCard = m_backtab_buffer[h][w];
			fgcolor = (nextCard & STIC_FBM_FG) | FOREGROUND_BIT;
			bgcolor = ((nextCard & STIC_FBM_BG2) >> 11) |
			((nextCard & STIC_FBM_BG310) >> 9);

			isGrom = !(nextCard & STIC_FBM_SEL);
			if (isGrom)
			{
				memoryLocation = nextCard & STIC_FBM_C;
				memory = m_grom;
				for (int j = 0; j < CARD_HEIGHT; j++)
					render_line(bitmap, memory[memoryLocation + j],
								nextx, nexty + j * Y_SCALE, fgcolor, bgcolor);
			}
			else
			{
				memoryLocation = nextCard & STIC_FBM_C;
				memory = m_gram;
				for (int j = 0; j < CARD_HEIGHT; j++)
					render_line(bitmap, memory[memoryLocation + j],
								nextx, nexty + j * Y_SCALE, fgcolor, bgcolor);
			}
		}
	}
}

void stic_device::copy_sprites_to_background(bitmap_ind16 &bitmap)
{
	uint8_t width, currentPixel;
	uint8_t borderCollision, foregroundCollision;
	uint8_t spritePixelHeight, x, y;
	int16_t leftX, nextY;
	int16_t leftBorder, rightBorder, topBorder, bottomBorder;
	int32_t nextX;

	for (int i = MOBS - 1; i >= 0; i--)
	{
		intv_sprite_type *s = &m_sprite[i];
		if (s->xpos == 0 || (!s->coll && !s->visible))
			continue;

		borderCollision = false;
		foregroundCollision = false;

		spritePixelHeight = (s->quady ? 4 : 1) * (s->doubley ? 2 : 1) * (s->doubleyres ? 2 : 1) * CARD_HEIGHT;
		width = (s->doublex ? 2 : 1) * CARD_WIDTH;

		leftX = (s->xpos - CARD_WIDTH + OVERSCAN_LEFT_WIDTH + m_col_delay) * X_SCALE;
		nextY = (s->ypos - CARD_HEIGHT + OVERSCAN_TOP_HEIGHT + m_row_delay) * Y_SCALE;

		leftBorder =  (OVERSCAN_LEFT_WIDTH + (m_left_edge_inhibit ? CARD_WIDTH : 0)) * X_SCALE;
		rightBorder = (OVERSCAN_LEFT_WIDTH + BACKTAB_WIDTH * CARD_WIDTH - 1 - 1) * X_SCALE;
		topBorder = (OVERSCAN_TOP_HEIGHT + (m_top_edge_inhibit ? CARD_HEIGHT : 0)) * Y_SCALE;
		bottomBorder = (OVERSCAN_TOP_HEIGHT + BACKTAB_HEIGHT * CARD_HEIGHT) * Y_SCALE - 1;

		for (y = 0; y < spritePixelHeight; y++)
		{
			for (x = 0; x < width; x++)
			{
				//if this sprite pixel is not on, then don't paint it
				if (!m_sprite_buffers[i][x][y])
					continue;

				nextX = leftX + x;
				//if the next pixel location is on the border, then we
				//have a border collision and we can ignore painting it
				if ((nextX < leftBorder) || (nextX > rightBorder) ||
					(nextY < topBorder) || (nextY > bottomBorder))
				{
					borderCollision = true;
					continue;
				}

				currentPixel = intv_get_pixel(bitmap, nextX, nextY);

				//check for foreground collision
				if (currentPixel & FOREGROUND_BIT)
				{
					foregroundCollision = true;
					if (s->behind_foreground)
						continue;
				}

				if (s->visible)
				{
					intv_set_pixel(bitmap, nextX, nextY, s->color | (currentPixel & FOREGROUND_BIT));
				}
			}
			nextY++;
		}

		//update the collision bits
		if (s->coll)
		{
			if (foregroundCollision)
				s->collision |= STIC_MCR_BKGD;
			if (borderCollision)
				s->collision |= STIC_MCR_BRDR;
		}
	}
}

void stic_device::render_background(bitmap_ind16 &bitmap)
{
	if (m_color_stack_mode)
		render_color_stack_mode(bitmap);
	else
		render_fg_bg_mode(bitmap);
}

#ifdef UNUSED_CODE
void stic_device::draw_background(bitmap_ind16 &bitmap, int transparency)
{
	// First, draw the background
	int offs = 0;
	int value = 0;
	int row,col;
	int fgcolor,bgcolor = 0;
	int code;

	int colora, colorb, colorc, colord;

	int n_bit;
	int p_bit;
	int g_bit;

	int j;

	int x0 = OVERSCAN_LEFT_WIDTH + m_col_delay;
	int y0 = OVERSCAN_TOP_HEIGHT + m_row_delay;

	if (m_color_stack_mode == 1)
	{
		m_color_stack_offset = 0;
		for(row = 0; row < BACKTAB_HEIGHT; row++)
		{
			for(col = 0; col < BACKTAB_WIDTH; col++)
			{
				value = m_ram16[offs];

				n_bit = value & STIC_CSTM_ADV;
				p_bit = value & STIC_CSTM_FG3;
				g_bit = value & STIC_CSTM_SEL;

				if (p_bit && (!g_bit)) // colored squares mode
				{
					colora = value & STIC_CSQM_A;
					colorb = (value & STIC_CSQM_B) >> 3;
					colorc = (value & STIC_CSQM_C) >> 6;
					colord = ((n_bit & STIC_CSQM_D2) >> 11) + ((value & STIC_CSQM_D10) >> 9);
					// color 7 if the top of the color stack in this mode
					if (colora == 7) colora = m_stic_registers[STIC_CSR + CSR3];
					if (colorb == 7) colorb = m_stic_registers[STIC_CSR + CSR3];
					if (colorc == 7) colorc = m_stic_registers[STIC_CSR + CSR3];
					if (colord == 7) colord = m_stic_registers[STIC_CSR + CSR3];
					intv_plot_box(bitmap, (x0 + col * CARD_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT) * Y_SCALE, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, colora);
					intv_plot_box(bitmap, (x0 + col * CARD_WIDTH + STIC_CSQM_WIDTH)) * X_SCALE, (y0 + row * CARD_HEIGHT) * Y_SCALE, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, colorb);
					intv_plot_box(bitmap, (x0 + col * CARD_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT + STIC_CSQM_HEIGHT) * Y_SCALE, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, colorc);
					intv_plot_box(bitmap, (x0 + col * CARD_WIDTH + STIC_CSQM_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT + STIC_CSQM_HEIGHT) * Y_SCALE, STIC_CSQM_WIDTH * X_SCALE, STIC_CSQM_HEIGHT * Y_SCALE, colord);
				}
				else // normal color stack mode
				{
					if (n_bit) // next color
					{
						m_color_stack_offset += 1;
						m_color_stack_offset &= (CSRS - 1);
					}

					if (p_bit) // pastel color set
						fgcolor = (value & STIC_CSTM_FG20) + 8;
					else
						fgcolor = value & STIC_CSTM_FG20;

					bgcolor = m_stic_registers[STIC_CSR + m_color_stack_offset];
					code = (value & STIC_CSTM_C)>>3;

					if (g_bit) // read from gram
					{
						code &= (STIC_CSTM_C50 >> 3);  // keep from going outside the array
						//if (m_gramdirtybytes[code] == 1)
						{
							decodechar(m_gfxdecode->gfx(1),
										code,
										m_gram,
										machine().config()->gfxdecodeinfo[1].gfxlayout);
							m_gramdirtybytes[code] = 0;
						}
						// Draw GRAM char
						drawgfx(bitmap,m_gfxdecode->gfx(1),
								code,
								bgcolor*16+fgcolor,
								0,0, (x0 + col * CARD_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT) * Y_SCALE,
								0,transparency,bgcolor);

						for(j=0;j<8;j++)
						{
							//intv_set_pixel(bitmap, (x0 + col * CARD_WIDTH + j) * X_SCALE, (y0 + row * CARD_HEIGHT + 7) * Y_SCALE + 1, 1);
						}

					}
					else // read from grom
					{
						drawgfx(bitmap,m_gfxdecode->gfx(0),
								code,
								bgcolor*16+fgcolor,
								0,0, (x0 + col * CARD_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT) * Y_SCALE,
								0,transparency,bgcolor);

						for(j=0;j<8;j++)
						{
							//intv_set_pixel(bitmap, (x0 + col * CARD_WIDTH + j) * X_SCALE, (y0 + row * CARD_HEIGHT + 7) * Y_SCALE + 1, 2);
						}
					}
				}
				offs++;
			} // next col
		} // next row
	}
	else
	{
		// fg/bg mode goes here
		for(row = 0; row < BACKTAB_HEIGHT; row++)
		{
			for(col = 0; col < BACKTAB_WIDTH; col++)
			{
				value = m_ram16[offs];
				fgcolor = value & STIC_FBM_FG;
				bgcolor = ((value & STIC_FBM_BG2) >> 11) + ((value & STIC_FBM_BG310) >> 9);
				code = (value & STIC_FBM_C) >> 3;

				if (value & STIC_FBM_SEL) // read for GRAM
				{
					//if (m_gramdirtybytes[code] == 1)
					{
						decodechar(m_gfxdecode->gfx(1),
									code,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						m_gramdirtybytes[code] = 0;
					}
					// Draw GRAM char
					drawgfx(bitmap,m_gfxdecode->gfx(1),
							code,
							bgcolor*16+fgcolor,
							0,0, (x0 + col * CARD_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT) * Y_SCALE,
							0,transparency,bgcolor);
				}
				else // read from GROM
				{
					drawgfx(bitmap,m_gfxdecode->gfx(0),
							code,
							bgcolor*16+fgcolor,
							0,0, (x0 + col * CARD_WIDTH) * X_SCALE, (y0 + row * CARD_HEIGHT) * Y_SCALE,
							0,transparency,bgcolor);
				}
				offs++;
			} // next col
		} // next row
	}
}
#endif

/* TBD: need to handle sprites behind foreground? */
#ifdef UNUSED_FUNCTION
void stic_device::draw_sprites(bitmap_ind16 &bitmap, int behind_foreground)
{
	int code;
	int x0 = OVERSCAN_LEFT_WIDTH + m_col_delay - CARD_WIDTH;
	int y0 = OVERSCAN_TOP_HEIGHT + m_row_delay - CARD_HEIGHT;

	for (int i = MOBS - 1; i >= 0; --i)
	{
		intv_sprite_type *s = &m_sprite[i];
		if (s->visible && (s->behind_foreground == behind_foreground))
		{
			code = s->card;
			if (!s->grom)
			{
				code %= 64;  // keep from going outside the array
				if (s->yres == 1)
				{
					//if (m_gramdirtybytes[code] == 1)
					{
						decodechar(m_gfxdecode->gfx(1),
									code,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						m_gramdirtybytes[code] = 0;
					}
					// Draw GRAM char
					m_gfxdecode->gfx(1)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * X_SCALE, (s->ypos + y0) * Y_SCALE,
											0x8000 * s->xsize, 0x8000 * s->ysize,0);
				}
				else
				{
					//if ((m_gramdirtybytes[code] == 1) || (m_gramdirtybytes[code+1] == 1))
					{
						decodechar(m_gfxdecode->gfx(1),
									code,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						decodechar(m_gfxdecode->gfx(1),
									code+1,
									m_gram,
									machine().config()->gfxdecodeinfo[1].gfxlayout);
						m_gramdirtybytes[code] = 0;
						m_gramdirtybytes[code+1] = 0;
					}
					// Draw GRAM char
					m_gfxdecode->gfx(1)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * X_SCALE, (s->ypos + y0) * Y_SCALE + s->yflip * s->ysize * CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
					m_gfxdecode->gfx(1)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code+1,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * X_SCALE, (s->ypos + y0) * Y_SCALE + (1 - s->yflip) * s->ysize * CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
				}
			}
			else
			{
				if (s->yres == 1)
				{
					// Draw GROM char
					m_gfxdecode->gfx(0)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * X_SCALE, (s->ypos + y0) * Y_SCALE,
											0x8000*s->xsize, 0x8000*s->ysize,0);
				}
				else
				{
					m_gfxdecode->gfx(0)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * X_SCALE, (s->ypos + y0) * Y_SCALE + s->yflip * s->ysize * CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
					m_gfxdecode->gfx(0)->zoom_transpen(bitmap,&machine().screen[0].visarea,
											code+1,
											s->color,
											s->xflip,s->yflip,
											(s->xpos + x0) * X_SCALE, (s->ypos + y0) * Y_SCALE + (1 - s->yflip) * s->ysize * CARD_HEIGHT,
											0x8000*s->xsize, 0x8000*s->ysize,0);
				}
			}
		}
	}
}
#endif

void stic_device::draw_borders(bitmap_ind16 &bitmap)
{
	intv_plot_box(bitmap, 0, 0, (OVERSCAN_LEFT_WIDTH + (m_left_edge_inhibit ? CARD_WIDTH : m_col_delay)) * X_SCALE, (OVERSCAN_TOP_HEIGHT + BACKTAB_HEIGHT * CARD_HEIGHT + OVERSCAN_BOTTOM_HEIGHT) * Y_SCALE, m_border_color);
	intv_plot_box(bitmap, (OVERSCAN_LEFT_WIDTH + BACKTAB_WIDTH * CARD_WIDTH - 1) * X_SCALE, 0, OVERSCAN_RIGHT_WIDTH, (OVERSCAN_TOP_HEIGHT + BACKTAB_HEIGHT * CARD_HEIGHT + OVERSCAN_BOTTOM_HEIGHT) * Y_SCALE, m_border_color);

	intv_plot_box(bitmap, 0, 0, (OVERSCAN_LEFT_WIDTH + BACKTAB_WIDTH * CARD_WIDTH - 1 + OVERSCAN_RIGHT_WIDTH) * X_SCALE, (OVERSCAN_TOP_HEIGHT + (m_top_edge_inhibit ? CARD_HEIGHT : m_row_delay)) * Y_SCALE, m_border_color);
	intv_plot_box(bitmap, 0, (OVERSCAN_TOP_HEIGHT + BACKTAB_HEIGHT * CARD_HEIGHT) * Y_SCALE, (OVERSCAN_LEFT_WIDTH + BACKTAB_WIDTH * CARD_WIDTH - 1 + OVERSCAN_RIGHT_WIDTH) * X_SCALE, OVERSCAN_BOTTOM_HEIGHT * Y_SCALE, m_border_color);
}

void stic_device::screenrefresh()
{
	if (m_stic_handshake != 0)
	{
		m_stic_handshake = 0;
		// Render the background
		render_background(m_bitmap);
		// Render the sprites into their buffers
		render_sprites();
		for (auto & elem : m_sprite)
			elem.collision = 0;
		// Copy the sprites to the background
		copy_sprites_to_background(m_bitmap);
		determine_sprite_collisions();
		for (int i = 0; i < MOBS; i++)
			m_stic_registers[STIC_MCR + i] |= m_sprite[i].collision;
		/* draw the screen borders if enabled */
		draw_borders(m_bitmap);
	}
	else
	{
		/* STIC disabled, just fill with border color */
		m_bitmap.fill(SET_COLOR(m_border_color));
	}
}



READ16_MEMBER( stic_device::read )
{
	//logerror("%x = stic_r(%x)\n",0,offset);
	switch (offset)
	{
		case STIC_MXR + MOB0:
		case STIC_MXR + MOB1:
		case STIC_MXR + MOB2:
		case STIC_MXR + MOB3:
		case STIC_MXR + MOB4:
		case STIC_MXR + MOB5:
		case STIC_MXR + MOB6:
		case STIC_MXR + MOB7:
			return 0x3800 | (m_stic_registers[offset] & 0x07FF);
		case STIC_MYR + MOB0:
		case STIC_MYR + MOB1:
		case STIC_MYR + MOB2:
		case STIC_MYR + MOB3:
		case STIC_MYR + MOB4:
		case STIC_MYR + MOB5:
		case STIC_MYR + MOB6:
		case STIC_MYR + MOB7:
			return 0x3000 | (m_stic_registers[offset] & 0x0FFF);
		case STIC_MAR + MOB0:
		case STIC_MAR + MOB1:
		case STIC_MAR + MOB2:
		case STIC_MAR + MOB3:
		case STIC_MAR + MOB4:
		case STIC_MAR + MOB5:
		case STIC_MAR + MOB6:
		case STIC_MAR + MOB7:
			return m_stic_registers[offset] & 0x3FFF;
		case STIC_MCR + MOB0:
		case STIC_MCR + MOB1:
		case STIC_MCR + MOB2:
		case STIC_MCR + MOB3:
		case STIC_MCR + MOB4:
		case STIC_MCR + MOB5:
		case STIC_MCR + MOB6:
		case STIC_MCR + MOB7:
			return 0x3C00 | (m_stic_registers[offset] & 0x03FF);
		case STIC_GMR:
			m_color_stack_mode = 1;
			//logerror("Setting color stack mode\n");
			/*** fall through ***/
		case STIC_DER:
			return 0x3FFF;
		case STIC_CSR + CSR0:
		case STIC_CSR + CSR1:
		case STIC_CSR + CSR2:
		case STIC_CSR + CSR3:
		case STIC_BCR:
			return 0x3FF0 | (m_stic_registers[offset] & 0x000F);
		case STIC_HDR:
		case STIC_VDR:
			return 0x3FF8 | (m_stic_registers[offset] & 0x0007);
		case STIC_CBR:
			return 0x3FFC | (m_stic_registers[offset] & 0x0003);
		default:
			//logerror("unmapped read from STIC register %02X\n", offset);
			return 0x3FFF;
	}
}

WRITE16_MEMBER( stic_device::write )
{
	intv_sprite_type *s;

	//logerror("stic_w(%x) = %x\n",offset,data);
	switch (offset)
	{
		// X Positions
		case STIC_MXR + MOB0:
		case STIC_MXR + MOB1:
		case STIC_MXR + MOB2:
		case STIC_MXR + MOB3:
		case STIC_MXR + MOB4:
		case STIC_MXR + MOB5:
		case STIC_MXR + MOB6:
		case STIC_MXR + MOB7:
			s =  &m_sprite[offset & (MOBS - 1)];
			s->doublex = !!(data & STIC_MXR_XSIZE);
			s->visible = !!(data & STIC_MXR_VIS);
			s->coll = !!(data & STIC_MXR_COL);
			s->xpos = (data & STIC_MXR_X);
			break;
		// Y Positions
		case STIC_MYR + MOB0:
		case STIC_MYR + MOB1:
		case STIC_MYR + MOB2:
		case STIC_MYR + MOB3:
		case STIC_MYR + MOB4:
		case STIC_MYR + MOB5:
		case STIC_MYR + MOB6:
		case STIC_MYR + MOB7:
			s =  &m_sprite[offset & (MOBS - 1)];
			s->yflip = !!(data & STIC_MYR_YFLIP);
			s->xflip = !!(data & STIC_MYR_XFLIP);
			s->quady = !!(data & STIC_MYR_YSIZE);
			s->doubley = !!(data & STIC_MYR_YFULL);
			s->doubleyres = !!(data & STIC_MYR_YRES);
			s->ypos = (data & STIC_MYR_Y);
			break;
		// Attributes
		case STIC_MAR + MOB0:
		case STIC_MAR + MOB1:
		case STIC_MAR + MOB2:
		case STIC_MAR + MOB3:
		case STIC_MAR + MOB4:
		case STIC_MAR + MOB5:
		case STIC_MAR + MOB6:
		case STIC_MAR + MOB7:
			s =  &m_sprite[offset & (MOBS - 1)];
			s->behind_foreground = !!(data & STIC_MAR_PRI);
			s->grom = !(data & STIC_MAR_SEL);
			s->card = ((data & STIC_MAR_C) >> 3);
			s->color = ((data & STIC_MAR_FG3) >> 9) | (data & STIC_MAR_FG20);
			break;
		// Collision Detection - TBD
		case STIC_MCR + MOB0:
		case STIC_MCR + MOB1:
		case STIC_MCR + MOB2:
		case STIC_MCR + MOB3:
		case STIC_MCR + MOB4:
		case STIC_MCR + MOB5:
		case STIC_MCR + MOB6:
		case STIC_MCR + MOB7:
			// a MOB's own collision bit is *always* zero, even if a
			// one is poked into it
			data &= ~(1 << (offset & (MOBS - 1)));
			break;
		// Display enable
		case STIC_DER:
			//logerror("***Writing a %x to the STIC handshake\n",data);
			m_stic_handshake = 1;
			break;
		// Graphics Mode
		case STIC_GMR:
			m_color_stack_mode = 0;
			break;
		// Color Stack
		case STIC_CSR + CSR0:
		case STIC_CSR + CSR1:
		case STIC_CSR + CSR2:
		case STIC_CSR + CSR3:
			//logerror("Setting color_stack[%x] = %x (%s)\n", offset & (CSRS - 1),data & STIC_CSR_BG, m_maincpu->pc());
			break;
		// Border Color
		case STIC_BCR:
			//logerror("***Writing a %x to the border color\n",data);
			m_border_color = data & STIC_BCR_BC;
			break;
		// Framing
		case STIC_HDR:
			m_col_delay = data & STIC_HDR_DEL;
			break;
		case STIC_VDR:
			m_row_delay = data & STIC_VDR_DEL;
			break;
		case STIC_CBR:
			m_left_edge_inhibit = (data & STIC_CBR_COL);
			m_top_edge_inhibit = (data & STIC_CBR_ROW) >> 1;
			break;
		default:
			//logerror("unmapped write to STIC register %02X: %04X\n", offset, data);
			break;
	}

	if (offset < ARRAY_LENGTH(m_stic_registers))
		m_stic_registers[offset] = data;
}


READ16_MEMBER( stic_device::gram_read )
{
	return m_gram[offset];
}

WRITE16_MEMBER( stic_device::gram_write )
{
	data &= 0xff;
	m_gram[offset] = data;
	m_gramdirtybytes[offset] = 1;
	m_gramdirty = 1;
}



uint32_t stic_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

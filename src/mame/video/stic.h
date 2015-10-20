// license:BSD-3-Clause
// copyright-holders:Nathan Woods,Frank Palazzolo
/*****************************************************************************
 *
 * includes/stic.h
 *
 ****************************************************************************/

#ifndef STIC_H_
#define STIC_H_

// GROM/GRAM cards are 8x8
#define STIC_CARD_WIDTH             8
#define STIC_CARD_HEIGHT            8

// Intellivision resolution is 20x12 BACKTAB CARDs, minus the rightmost column,
// for an effective resolution of (19 * 8 + 1 * 7) x (12 * 8) == 159x96.
//
// MOB scanline height can be half of a card scanline height, so y-coordinates
// are scaled by 2.
#define STIC_X_SCALE                1
#define STIC_Y_SCALE                2

// the Intellivision emulation scales to match the output format at the last
// step. The Intellivision keyboard component appears to be 320x96, but can
// also run Intellivision carts, so x-coordinates are conditionally scaled
// by 2.
#define INTV_X_SCALE                1
#define INTV_Y_SCALE                1
#define INTVKBD_X_SCALE             2
#define INTVKBD_Y_SCALE             INTV_Y_SCALE

// overscan sizes in intv pixels
// these values are approximate.
#define STIC_OVERSCAN_LEFT_WIDTH    13
#define STIC_OVERSCAN_RIGHT_WIDTH   17
#define STIC_OVERSCAN_TOP_HEIGHT    12
#define STIC_OVERSCAN_BOTTOM_HEIGHT 12

//Timing constants based on  Joe Zbiciak's documentation
#define STIC_CYCLES_PER_SCANLINE 57
#define STIC_ROW_BUSRQ 110 // CPU paused during backtab row buffering
#define STIC_FRAME_BUSRQ 42 // CPU paused just after end of vblank and before first row fetch (approximate)
#define STIC_VBLANK_END 3790
#define STIC_FIRST_FETCH 3933

/*** STIC registers *********************************************************/

// number of STIC registers
#define STIC_REGISTERS              0x33

// STIC MOBs (Moveable OBjects)
enum
{
	STIC_MOB0,
	STIC_MOB1,
	STIC_MOB2,
	STIC_MOB3,
	STIC_MOB4,
	STIC_MOB5,
	STIC_MOB6,
	STIC_MOB7,

	STIC_MOBS
};

// STIC Color Stack
enum
{
	STIC_CSR0,
	STIC_CSR1,
	STIC_CSR2,
	STIC_CSR3,

	STIC_CSRS
};

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
 * BKGD         1=collsion with set background bit                          *
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

/*** BACKTAB ****************************************************************/

// BACKTAB is made up of 20x12 cards
// (the first 19 columns are 8x8, the 20th column is 7x8)
#define STIC_BACKTAB_WIDTH          20
#define STIC_BACKTAB_HEIGHT         12

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

#define STIC_CSQM_WIDTH             (STIC_CARD_WIDTH / 2)
#define STIC_CSQM_HEIGHT            (STIC_CARD_HEIGHT / 2)



struct intv_sprite_type
{
	int visible;
	int xpos;
	int ypos;
	int coll;
	int collision;
	int doublex;
	int doubley;
	int quady;
	int xflip;
	int yflip;
	int behind_foreground;
	int grom;
	int card;
	int color;
	int doubleyres;
	int dirty;
};


// ======================> stic_device

class stic_device :  public device_t
{
public:
	// construction/destruction
	stic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~stic_device();

	DECLARE_READ16_MEMBER(read);
	DECLARE_READ16_MEMBER(gram_read);
	DECLARE_READ16_MEMBER(grom_read) { if (offset > 0x800) printf("help! %X\n", offset); return (0xff00 | m_grom[offset]); }
	DECLARE_WRITE16_MEMBER(write);
	DECLARE_WRITE16_MEMBER(gram_write);

	void write_to_btb(int h, int w, UINT16 data) { m_backtab_buffer[h][w] = data; }
	int read_row_delay() { return m_row_delay; }
	int read_stic_handshake() { return m_stic_handshake; }
	void set_x_scale(int val) { m_x_scale = val; }
	void set_y_scale(int val) { m_y_scale = val; }

	// device-level overrides
	virtual void device_start();
	virtual const rom_entry *device_rom_region() const;
	virtual void device_reset();

	void screenrefresh();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:

	required_region_ptr<UINT8> m_grom;

	void intv_set_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	UINT32 intv_get_pixel(bitmap_ind16 &bitmap, int x, int y);
	void intv_plot_box(bitmap_ind16 &bm, int x, int y, int w, int h, int color);
	int sprites_collide(int spriteNum1, int spriteNum2);
	void determine_sprite_collisions();
	void render_sprites();
	void render_line(bitmap_ind16 &bitmap, UINT8 nextByte, UINT16 x, UINT16 y, UINT8 fgcolor, UINT8 bgcolor);
	void render_colored_squares(bitmap_ind16 &bitmap, UINT16 x, UINT16 y, UINT8 color0, UINT8 color1, UINT8 color2, UINT8 color3);
	void render_color_stack_mode(bitmap_ind16 &bitmap);
	void render_fg_bg_mode(bitmap_ind16 &bitmap);
	void copy_sprites_to_background(bitmap_ind16 &bitmap);
	void render_background(bitmap_ind16 &bitmap);
	void draw_borders(bitmap_ind16 &bitmap);

#ifdef UNUSED_CODE
	void draw_background(bitmap_ind16 &bitmap, int transparency);
	void draw_sprites(bitmap_ind16 &bitmap, int behind_foreground);
#endif

	bitmap_ind16 m_bitmap;

	intv_sprite_type m_sprite[STIC_MOBS];
	UINT8 m_sprite_buffers[STIC_MOBS][STIC_CARD_WIDTH * 2][STIC_CARD_HEIGHT * 4 * 2 * 2];
	UINT16 m_backtab_buffer[STIC_BACKTAB_HEIGHT][STIC_BACKTAB_WIDTH];
	int m_color_stack_mode;
	int m_stic_registers[STIC_REGISTERS];
	int m_color_stack_offset;
	int m_stic_handshake;
	int m_border_color;
	int m_col_delay;
	int m_row_delay;
	int m_left_edge_inhibit;
	int m_top_edge_inhibit;
	int m_x_scale;
	int m_y_scale;

	UINT8 m_gramdirty;
	UINT8 m_gram[512];
	UINT8 m_gramdirtybytes[512];
};

// device type definition
extern const device_type STIC;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_STIC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, STIC, 0)


#endif /* STIC_H_ */

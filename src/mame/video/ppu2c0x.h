/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/
#ifndef __PPU_2C03B_H__
#define __PPU_2C03B_H__

/* increment to use more chips */
#define MAX_PPU 2

/* mirroring types */
#define PPU_MIRROR_NONE		0
#define PPU_MIRROR_VERT		1
#define PPU_MIRROR_HORZ		2
#define PPU_MIRROR_HIGH		3
#define PPU_MIRROR_LOW		4
#define PPU_MIRROR_4SCREEN	5	// Same effect as NONE, but signals that we should never mirror

/* registers definition */
enum
{
	PPU_CONTROL0 = 0,
	PPU_CONTROL1,
	PPU_STATUS,
	PPU_SPRITE_ADDRESS,
	PPU_SPRITE_DATA,
	PPU_SCROLL,
	PPU_ADDRESS,
	PPU_DATA,
	PPU_MAX_REG
};

/* bit definitions for (some of) the registers */
enum
{
	PPU_CONTROL0_INC			= 0x04,
	PPU_CONTROL0_SPR_SELECT		= 0x08,
	PPU_CONTROL0_CHR_SELECT		= 0x10,
	PPU_CONTROL0_SPRITE_SIZE	= 0x20,
	PPU_CONTROL0_NMI			= 0x80,

	PPU_CONTROL1_DISPLAY_MONO	= 0x01,
	PPU_CONTROL1_BACKGROUND_L8	= 0x02,
	PPU_CONTROL1_SPRITES_L8		= 0x04,
	PPU_CONTROL1_BACKGROUND		= 0x08,
	PPU_CONTROL1_SPRITES		= 0x10,
	PPU_CONTROL1_COLOR_EMPHASIS	= 0xe0,

	PPU_STATUS_8SPRITES			= 0x20,
	PPU_STATUS_SPRITE0_HIT		= 0x40,
	PPU_STATUS_VBLANK			= 0x80
};

enum
{
	PPU_NTSC_SCANLINES_PER_FRAME	= 262,
	PPU_PAL_SCANLINES_PER_FRAME		= 312,

	PPU_BOTTOM_VISIBLE_SCANLINE		= 239,
	PPU_VBLANK_FIRST_SCANLINE		= 241,
	PPU_VBLANK_LAST_SCANLINE_NTSC	= 260,
	PPU_VBLANK_LAST_SCANLINE_PAL	= 310

	// Both the sacnline immediately before and immediately after VBLANK
	// are non-rendering and non-vblank.
};

typedef enum
{
	PPU_2C02,	// NTSC NES
	PPU_2C03B,	// Playchoice 10
	PPU_2C04,	// Vs. Unisystem
	PPU_2C05,	// Vs. Unisystem, Famicom Titler
	PPU_2C07	// PAL NES
} ppu_t;

/* callback datatypes */
typedef void (*ppu2c0x_scanline_cb)( int num, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_hblank_cb)( int num, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_nmi_cb)( int num, int *ppu_regs );
typedef int  (*ppu2c0x_vidaccess_cb)( int num, int address, int data );

typedef struct _ppu2c0x_interface ppu2c0x_interface;
struct _ppu2c0x_interface
{
	ppu_t			type;							// model/version of the PPU
	int				num;							/* number of chips ( 1 to MAX_PPU ) */
	int				vrom_region[MAX_PPU];			/* region id of gfx vrom (or REGION_INVALID if none) */
	int				gfx_layout_number[MAX_PPU];		/* gfx layout number used by each chip */
	int				color_base[MAX_PPU];			/* color base to use per ppu */
	int				mirroring[MAX_PPU];				/* mirroring options (PPU_MIRROR_* flag) */
	ppu2c0x_nmi_cb	nmi_handler[MAX_PPU];			/* NMI handler */
};

/* routines */
void ppu2c0x_init_palette(running_machine *machine, int first_entry );
void ppu2c0x_init(running_machine *machine, const ppu2c0x_interface *interface );

void ppu2c0x_reset( int num, int scan_scale );
void ppu2c0x_set_videorom_bank( int num, int start_page, int num_pages, int bank, int bank_size );
void ppu2c0x_spriteram_dma(int num, const UINT8 page );
void ppu2c0x_render( int num, bitmap_t *bitmap, int flipx, int flipy, int sx, int sy );
int ppu2c0x_get_pixel( int num, int x, int y );
int ppu2c0x_get_colorbase( int num );
int ppu2c0x_get_current_scanline( int num );
void ppu2c0x_set_mirroring( int num, int mirroring );
void ppu2c0x_set_scanline_callback( int num, ppu2c0x_scanline_cb cb );
void ppu2c0x_set_hblank_callback( int num, ppu2c0x_scanline_cb cb );
void ppu2c0x_set_vidaccess_callback( int num, ppu2c0x_vidaccess_cb cb );
void ppu2c0x_set_scanlines_per_frame( int num, int scanlines );

//27/12/2002
extern void (*ppu_latch)( offs_t offset );

void ppu2c0x_w( int num, offs_t offset, UINT8 data );
int ppu2c0x_r( int num, offs_t offset );

/* accesors */
READ8_HANDLER( ppu2c0x_0_r );
READ8_HANDLER( ppu2c0x_1_r );

WRITE8_HANDLER( ppu2c0x_0_w );
WRITE8_HANDLER( ppu2c0x_1_w );

#endif /* __PPU_2C0X_H__ */

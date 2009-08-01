/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#ifndef __PPU_2C03B_H__
#define __PPU_2C03B_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

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

#define PPU_2C02	DEVICE_GET_INFO_NAME(ppu2c02)	// NTSC NES
#define PPU_2C03B	DEVICE_GET_INFO_NAME(ppu2c03b)	// Playchoice 10
#define PPU_2C04	DEVICE_GET_INFO_NAME(ppu2c04)	// Vs. Unisystem
#define PPU_2C05	DEVICE_GET_INFO_NAME(ppu2c05)	// Vs. Unisystem, Famicom Titler
#define PPU_2C07	DEVICE_GET_INFO_NAME(ppu2c07)	// PAL NES

/* callback datatypes */
typedef void (*ppu2c0x_scanline_cb)( const device_config *device, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_hblank_cb)( const device_config *device, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_nmi_cb)( const device_config *device, int *ppu_regs );
typedef int  (*ppu2c0x_vidaccess_cb)( const device_config *device, int address, int data );

typedef struct _ppu2c0x_interface ppu2c0x_interface;
struct _ppu2c0x_interface
{
	int				gfx_layout_number;		/* gfx layout number used by each chip */
	int				color_base;				/* color base to use per ppu */
	int				mirroring;				/* mirroring options (PPU_MIRROR_* flag) */
	ppu2c0x_nmi_cb	nmi_handler;			/* NMI handler */
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO(ppu2c02);
DEVICE_GET_INFO(ppu2c03b);
DEVICE_GET_INFO(ppu2c04);
DEVICE_GET_INFO(ppu2c05);
DEVICE_GET_INFO(ppu2c07);

/* routines */
void ppu2c0x_init_palette(running_machine *machine, int first_entry ) ATTR_NONNULL(1);

void ppu2c0x_spriteram_dma(const address_space *space, const device_config *device, const UINT8 page ) ATTR_NONNULL(1);
void ppu2c0x_render( const device_config *device, bitmap_t *bitmap, int flipx, int flipy, int sx, int sy ) ATTR_NONNULL(1);
int ppu2c0x_get_pixel( const device_config *device, int x, int y ) ATTR_NONNULL(1);
int ppu2c0x_get_colorbase( const device_config *device ) ATTR_NONNULL(1);
int ppu2c0x_get_current_scanline( const device_config *device ) ATTR_NONNULL(1);
void ppu2c0x_set_scanline_callback( const device_config *device, ppu2c0x_scanline_cb cb ) ATTR_NONNULL(1);
void ppu2c0x_set_hblank_callback( const device_config *device, ppu2c0x_scanline_cb cb ) ATTR_NONNULL(1);
void ppu2c0x_set_vidaccess_callback( const device_config *device, ppu2c0x_vidaccess_cb cb ) ATTR_NONNULL(1);
void ppu2c0x_set_scanlines_per_frame( const device_config *device, int scanlines ) ATTR_NONNULL(1);

//27/12/2002 (HACK!)
extern void (*ppu_latch)( const device_config *device, offs_t offset );

WRITE8_DEVICE_HANDLER( ppu2c0x_w );
READ8_DEVICE_HANDLER( ppu2c0x_r );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PPU2C02_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPU_2C02, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPU2C03B_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPU_2C03B, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPU2C04_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPU_2C04, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPU2C05_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPU_2C05, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PPU2C07_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PPU_2C07, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#endif /* __PPU_2C0X_H__ */

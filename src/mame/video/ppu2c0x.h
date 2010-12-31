/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#ifndef __PPU_2C03B_H__
#define __PPU_2C03B_H__

#include "devlegcy.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* mirroring types */
#define PPU_MIRROR_NONE       0
#define PPU_MIRROR_VERT       1
#define PPU_MIRROR_HORZ       2
#define PPU_MIRROR_HIGH       3
#define PPU_MIRROR_LOW        4
#define PPU_MIRROR_4SCREEN    5	// Same effect as NONE, but signals that we should never mirror

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
	PPU_CONTROL0_INC              = 0x04,
	PPU_CONTROL0_SPR_SELECT       = 0x08,
	PPU_CONTROL0_CHR_SELECT       = 0x10,
	PPU_CONTROL0_SPRITE_SIZE      = 0x20,
	PPU_CONTROL0_NMI              = 0x80,

	PPU_CONTROL1_DISPLAY_MONO     = 0x01,
	PPU_CONTROL1_BACKGROUND_L8    = 0x02,
	PPU_CONTROL1_SPRITES_L8       = 0x04,
	PPU_CONTROL1_BACKGROUND       = 0x08,
	PPU_CONTROL1_SPRITES          = 0x10,
	PPU_CONTROL1_COLOR_EMPHASIS   = 0xe0,

	PPU_STATUS_8SPRITES           = 0x20,
	PPU_STATUS_SPRITE0_HIT        = 0x40,
	PPU_STATUS_VBLANK             = 0x80
};

enum
{
	PPU_NTSC_SCANLINES_PER_FRAME  = 262,
	PPU_PAL_SCANLINES_PER_FRAME   = 312,

	PPU_BOTTOM_VISIBLE_SCANLINE   = 239,
	PPU_VBLANK_FIRST_SCANLINE     = 241,
	PPU_VBLANK_LAST_SCANLINE_NTSC = 260,
	PPU_VBLANK_LAST_SCANLINE_PAL  = 310

	// Both the sacnline immediately before and immediately after VBLANK
	// are non-rendering and non-vblank.
};

/* callback datatypes */
typedef void (*ppu2c0x_scanline_cb)( device_t *device, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_hblank_cb)( device_t *device, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_nmi_cb)( device_t *device, int *ppu_regs );
typedef int  (*ppu2c0x_vidaccess_cb)( device_t *device, int address, int data );

typedef struct _ppu2c0x_interface ppu2c0x_interface;
struct _ppu2c0x_interface
{
	int               gfx_layout_number;		/* gfx layout number used by each chip */
	int               color_base;				/* color base to use per ppu */
	int               mirroring;				/* mirroring options (PPU_MIRROR_* flag) */
	ppu2c0x_nmi_cb    nmi_handler;			/* NMI handler */
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C02, ppu2c02);		// NTSC NES
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C03B, ppu2c03b);		// Playchoice 10
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C04, ppu2c04);		// Vs. Unisystem
// The PPU_2C05 variants have different protection value, set at DEVICE_START, but otherwise are all the same...
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C05_01, ppu2c05_01);	// Vs. Unisystem (Ninja Jajamaru Kun)
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C05_02, ppu2c05_02);	// Vs. Unisystem (Mighty Bomb Jack)
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C05_03, ppu2c05_03);	// Vs. Unisystem (Gumshoe)
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C05_04, ppu2c05_04);	// Vs. Unisystem (Top Gun)
DECLARE_LEGACY_MEMORY_DEVICE(PPU_2C07, ppu2c07);		// PAL NES

/* routines */
void ppu2c0x_init_palette(running_machine *machine, int first_entry ) ATTR_NONNULL(1);
void ppu2c0x_init_palette_rgb(running_machine *machine, int first_entry ) ATTR_NONNULL(1);

void ppu2c0x_spriteram_dma(address_space *space, device_t *device, const UINT8 page ) ATTR_NONNULL(1);
void ppu2c0x_render( device_t *device, bitmap_t *bitmap, int flipx, int flipy, int sx, int sy ) ATTR_NONNULL(1);
int ppu2c0x_get_pixel( device_t *device, int x, int y ) ATTR_NONNULL(1);
int ppu2c0x_get_colorbase( device_t *device ) ATTR_NONNULL(1);
int ppu2c0x_get_current_scanline( device_t *device ) ATTR_NONNULL(1);
int ppu2c0x_is_sprite_8x16( device_t *device ) ATTR_NONNULL(1);
void ppu2c0x_set_scanline_callback( device_t *device, ppu2c0x_scanline_cb cb ) ATTR_NONNULL(1);
void ppu2c0x_set_hblank_callback( device_t *device, ppu2c0x_scanline_cb cb ) ATTR_NONNULL(1);
void ppu2c0x_set_vidaccess_callback( device_t *device, ppu2c0x_vidaccess_cb cb ) ATTR_NONNULL(1);
void ppu2c0x_set_scanlines_per_frame( device_t *device, int scanlines ) ATTR_NONNULL(1);

//27/12/2002 (HACK!)
extern void (*ppu_latch)( device_t *device, offs_t offset );

WRITE8_DEVICE_HANDLER( ppu2c0x_w );
READ8_DEVICE_HANDLER( ppu2c0x_r );


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PPU2C02_ADD(_tag, _intrf)   \
	MCFG_DEVICE_ADD(_tag, PPU_2C02, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C03B_ADD(_tag, _intrf)   \
	MCFG_DEVICE_ADD(_tag, PPU_2C03B, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C04_ADD(_tag, _intrf)   \
	MCFG_DEVICE_ADD(_tag, PPU_2C04, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C05_01_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PPU_2C05_01, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C05_02_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PPU_2C05_02, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C05_03_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PPU_2C05_03, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C05_04_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, PPU_2C05_04, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C07_ADD(_tag, _intrf)   \
	MCFG_DEVICE_ADD(_tag, PPU_2C07, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* __PPU_2C0X_H__ */

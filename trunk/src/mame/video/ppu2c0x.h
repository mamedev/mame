/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#pragma once

#ifndef __PPU_2C03B_H__
#define __PPU_2C03B_H__

#include "devlegcy.h"


///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

// mirroring types
#define PPU_MIRROR_NONE       0
#define PPU_MIRROR_VERT       1
#define PPU_MIRROR_HORZ       2
#define PPU_MIRROR_HIGH       3
#define PPU_MIRROR_LOW        4
#define PPU_MIRROR_4SCREEN    5	// Same effect as NONE, but signals that we should never mirror

// registers definition
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

// bit definitions for (some of) the registers
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

/*----------- defined in video/ppu2c0x.c -----------*/

///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_PPU2C0X_ADD(_tag, _type, _intrf)   \
	MCFG_DEVICE_ADD(_tag, _type, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_PPU2C02_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C02, _intrf) \

#define MCFG_PPU2C03B_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C03B, _intrf) \

#define MCFG_PPU2C04_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C04, _intrf) \

#define MCFG_PPU2C07_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C07, _intrf) \

#define MCFG_PPU2C05_01_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_01, _intrf) \

#define MCFG_PPU2C05_02_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_02, _intrf) \

#define MCFG_PPU2C05_03_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_03, _intrf) \

#define MCFG_PPU2C05_04_ADD(_tag, _intrf)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_04, _intrf) \


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// callback datatypes
typedef void (*ppu2c0x_scanline_cb)( device_t *device, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_hblank_cb)( device_t *device, int scanline, int vblank, int blanked );
typedef void (*ppu2c0x_nmi_cb)( device_t *device, int *ppu_regs );
typedef int  (*ppu2c0x_vidaccess_cb)( device_t *device, int address, int data );


// ======================> ppu2c0x_interface

struct ppu2c0x_interface
{
	const char        *cpu_tag;
	const char        *screen_tag;
	int               gfx_layout_number;		/* gfx layout number used by each chip */
	int               color_base;				/* color base to use per ppu */
	int               mirroring;				/* mirroring options (PPU_MIRROR_* flag) */
	ppu2c0x_nmi_cb    nmi_handler;			/* NMI handler */
};


// ======================> ppu2c0x_device

class ppu2c0x_device :	public device_t,
						public device_memory_interface,
						public ppu2c0x_interface
{
public:
    // construction/destruction
	ppu2c0x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( palette_read );
	DECLARE_WRITE8_MEMBER( palette_write );

	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();
	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	// address space configurations
	const address_space_config		m_space_config;


	/* routines */
	void init_palette( running_machine &machine, int first_entry );
	void init_palette_rgb( running_machine &machine, int first_entry );

	void draw_background( UINT8 *line_priority );
	void draw_sprites( UINT8 *line_priority );
	void render_scanline();
	void update_scanline();

	void spriteram_dma(address_space *space, const UINT8 page );
	void render( bitmap_ind16 &bitmap, int flipx, int flipy, int sx, int sy );
	int get_pixel( int x, int y );

	int get_colorbase() { return m_color_base; };
	int get_current_scanline() { return m_scanline; };
	int is_sprite_8x16() { return BIT(m_regs[0], 5); };	// MMC5 has to be able to check this
	void set_scanline_callback( ppu2c0x_scanline_cb cb ) { if (cb != NULL) m_scanline_callback_proc = cb; };
	void set_hblank_callback( ppu2c0x_scanline_cb cb ) { if (cb != NULL) m_hblank_callback_proc = cb; };
	void set_vidaccess_callback( ppu2c0x_vidaccess_cb cb ) { if (cb != NULL) m_vidaccess_callback_proc = cb; };
	void set_scanlines_per_frame( int scanlines ) { m_scanlines_per_frame = scanlines; };

	//27/12/2002 (HACK!)
	void set_latch( void (*ppu_latch_t)( device_t *device, offs_t offset ) );

	//  void update_screen(bitmap_t &bitmap, const rectangle &cliprect);

	cpu_device					*m_cpu;
	screen_device				*m_screen;
	bitmap_ind16				*m_bitmap;			/* target bitmap */
	UINT8                       *m_spriteram;			/* sprite ram */
	pen_t                       *m_colortable;			/* color table modified at run time */
	pen_t                       *m_colortable_mono;		/* monochromatic color table modified at run time */
	int                         m_scanline;			/* scanline count */
	ppu2c0x_scanline_cb         m_scanline_callback_proc;	/* optional scanline callback */
	ppu2c0x_hblank_cb           m_hblank_callback_proc;	/* optional hblank callback */
	ppu2c0x_vidaccess_cb        m_vidaccess_callback_proc;	/* optional video access callback */
	ppu2c0x_nmi_cb              m_nmi_callback_proc;		/* nmi access callback from interface */
	int                         m_regs[PPU_MAX_REG];		/* registers */
	int                         m_refresh_data;			/* refresh-related */
	int                         m_refresh_latch;		/* refresh-related */
	int                         m_x_fine;				/* refresh-related */
	int                         m_toggle;				/* used to latch hi-lo scroll */
	int                         m_add;				/* vram increment amount */
	int                         m_videomem_addr;		/* videomem address pointer */
	int                         m_data_latch;			/* latched videomem data */
	int                         m_buffered_data;
	int                         m_tile_page;			/* current tile page */
	int                         m_sprite_page;			/* current sprite page */
	int                         m_back_color;			/* background color */
	int                         m_color_base;
	UINT8                       m_palette_ram[0x20];		/* shouldn't be in main memory! */
	int                         m_scan_scale;			/* scan scale */
	int                         m_scanlines_per_frame;	/* number of scanlines per frame */
	int                         m_security_value;		/* 2C05 protection */
	void (*m_latch)( device_t *device, offs_t offset );

	// timers
	emu_timer                   *m_hblank_timer;		/* hblank period at end of each scanline */
	emu_timer                   *m_nmi_timer;			/* NMI timer */
	emu_timer                   *m_scanline_timer;		/* scanline timer */

	const char        *m_cpu_tag;
	const char        *m_screen_tag;

private:
	static const device_timer_id TIMER_HBLANK = 0;
	static const device_timer_id TIMER_NMI = 1;
	static const device_timer_id TIMER_SCANLINE = 2;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);
};

class ppu2c02_device : public ppu2c0x_device {
public:
	ppu2c02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c03b_device : public ppu2c0x_device {
public:
	ppu2c03b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c04_device : public ppu2c0x_device {
public:
	ppu2c04_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c07_device : public ppu2c0x_device {
public:
	ppu2c07_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c05_01_device : public ppu2c0x_device {
public:
	ppu2c05_01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c05_02_device : public ppu2c0x_device {
public:
	ppu2c05_02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c05_03_device : public ppu2c0x_device {
public:
	ppu2c05_03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class ppu2c05_04_device : public ppu2c0x_device {
public:
	ppu2c05_04_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
//extern const device_type PPU_2C0X;
extern const device_type PPU_2C02;	// NTSC NES
extern const device_type PPU_2C03B;	// Playchoice 10
extern const device_type PPU_2C04;	// Vs. Unisystem
extern const device_type PPU_2C07;	// PAL NES
extern const device_type PPU_2C05_01;	// Vs. Unisystem (Ninja Jajamaru Kun)
extern const device_type PPU_2C05_02;	// Vs. Unisystem (Mighty Bomb Jack)
extern const device_type PPU_2C05_03;	// Vs. Unisystem (Gumshoe)
extern const device_type PPU_2C05_04;	// Vs. Unisystem (Top Gun)


#endif

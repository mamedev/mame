// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#pragma once

#ifndef __PPU_2C03B_H__
#define __PPU_2C03B_H__

///*************************************************************************
//  MACROS / CONSTANTS
///*************************************************************************

// mirroring types
#define PPU_MIRROR_NONE       0
#define PPU_MIRROR_VERT       1
#define PPU_MIRROR_HORZ       2
#define PPU_MIRROR_HIGH       3
#define PPU_MIRROR_LOW        4
#define PPU_MIRROR_4SCREEN    5 // Same effect as NONE, but signals that we should never mirror

#define PPU_DRAW_BG       0
#define PPU_DRAW_OAM      1


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

	// Both the scanline immediately before and immediately after VBLANK
	// are non-rendering and non-vblank.
};

/*----------- defined in video/ppu2c0x.c -----------*/

///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_PPU2C0X_ADD(_tag, _type)   \
	MCFG_DEVICE_ADD(_tag, _type, 0)

#define MCFG_PPU2C02_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C02)
#define MCFG_PPU2C03B_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C03B)
#define MCFG_PPU2C04_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C04)
#define MCFG_PPU2C07_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C07)
#define MCFG_PPU2C05_01_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_01)
#define MCFG_PPU2C05_02_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_02)
#define MCFG_PPU2C05_03_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_03)
#define MCFG_PPU2C05_04_ADD(_tag)   \
	MCFG_PPU2C0X_ADD(_tag, PPU_2C05_04)

#define MCFG_PPU2C0X_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#define MCFG_PPU2C0X_CPU(_tag) \
	ppu2c0x_device::set_cpu_tag(*device, "^" _tag);

#define MCFG_PPU2C0X_COLORBASE(_color) \
	ppu2c0x_device::set_color_base(*device, _color);

#define MCFG_PPU2C0X_SET_NMI(_class, _method) \
	ppu2c0x_device::set_nmi_delegate(*device, ppu2c0x_nmi_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_PPU2C0X_IGNORE_SPRITE_WRITE_LIMIT \
	ppu2c0x_device::use_sprite_write_limitation_disable(*device);

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************
typedef device_delegate<void (int scanline, int vblank, int blanked)> ppu2c0x_scanline_delegate;
typedef device_delegate<void (int scanline, int vblank, int blanked)> ppu2c0x_hblank_delegate;
typedef device_delegate<void (int *ppu_regs)> ppu2c0x_nmi_delegate;
typedef device_delegate<int (int address, int data)> ppu2c0x_vidaccess_delegate;
typedef device_delegate<void (offs_t offset)> ppu2c0x_latch_delegate;


// ======================> ppu2c0x_device

class ppu2c0x_device :  public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	ppu2c0x_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( palette_read );
	DECLARE_WRITE8_MEMBER( palette_write );

	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_config_complete() override;
	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
	// address space configurations
	const address_space_config      m_space_config;

	static void set_cpu_tag(device_t &device, std::string tag) { downcast<ppu2c0x_device &>(device).m_cpu.set_tag(tag); }
	static void set_color_base(device_t &device, int colorbase) { downcast<ppu2c0x_device &>(device).m_color_base = colorbase; }
	static void set_nmi_delegate(device_t &device, ppu2c0x_nmi_delegate cb);

	/* routines */
	void init_palette( palette_device &palette, int first_entry );
	void init_palette_rgb( palette_device &palette, int first_entry );

	void draw_background( UINT8 *line_priority );
	void draw_sprites( UINT8 *line_priority );
	void render_scanline();
	void update_scanline();

	void spriteram_dma(address_space &space, const UINT8 page );
	void render( bitmap_ind16 &bitmap, int flipx, int flipy, int sx, int sy );
	int get_pixel( int x, int y );

	int get_colorbase() { return m_color_base; };
	int get_current_scanline() { return m_scanline; };
	void set_scanline_callback( ppu2c0x_scanline_delegate cb ) { m_scanline_callback_proc = cb; m_scanline_callback_proc.bind_relative_to(*owner()); };
	void set_hblank_callback( ppu2c0x_hblank_delegate cb ) { m_hblank_callback_proc = cb; m_hblank_callback_proc.bind_relative_to(*owner()); };
	void set_vidaccess_callback( ppu2c0x_vidaccess_delegate cb ) { m_vidaccess_callback_proc = cb; m_vidaccess_callback_proc.bind_relative_to(*owner()); };
	void set_scanlines_per_frame( int scanlines ) { m_scanlines_per_frame = scanlines; };

	// MMC5 has to be able to check this
	int is_sprite_8x16() { return m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE; };
	int get_draw_phase() { return m_draw_phase; };
	int get_tilenum() { return m_tilecount; };

	//27/12/2002 (HACK!)
	void set_latch( ppu2c0x_latch_delegate cb ) { m_latch = cb; m_latch.bind_relative_to(*owner()); };

	//  void update_screen(bitmap_t &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_cpu;

	std::unique_ptr<bitmap_ind16>                m_bitmap;          /* target bitmap */
	std::unique_ptr<UINT8[]>    m_spriteram;           /* sprite ram */
	std::unique_ptr<pen_t[]>    m_colortable;          /* color table modified at run time */
	std::unique_ptr<pen_t[]>    m_colortable_mono;     /* monochromatic color table modified at run time */
	int                         m_scanline;         /* scanline count */
	ppu2c0x_scanline_delegate   m_scanline_callback_proc;   /* optional scanline callback */
	ppu2c0x_hblank_delegate     m_hblank_callback_proc; /* optional hblank callback */
	ppu2c0x_vidaccess_delegate  m_vidaccess_callback_proc;  /* optional video access callback */
	ppu2c0x_nmi_delegate        m_nmi_callback_proc;        /* nmi access callback from interface */
	int                         m_regs[PPU_MAX_REG];        /* registers */
	int                         m_refresh_data;         /* refresh-related */
	int                         m_refresh_latch;        /* refresh-related */
	int                         m_x_fine;               /* refresh-related */
	int                         m_toggle;               /* used to latch hi-lo scroll */
	int                         m_add;              /* vram increment amount */
	int                         m_videomem_addr;        /* videomem address pointer */
	int                         m_data_latch;           /* latched videomem data */
	int                         m_buffered_data;
	int                         m_tile_page;            /* current tile page */
	int                         m_sprite_page;          /* current sprite page */
	int                         m_back_color;           /* background color */
	int                         m_color_base;
	UINT8                       m_palette_ram[0x20];        /* shouldn't be in main memory! */
	int                         m_scan_scale;           /* scan scale */
	int                         m_scanlines_per_frame;  /* number of scanlines per frame */
	int                         m_security_value;       /* 2C05 protection */
	int                         m_tilecount;            /* MMC5 can change attributes to subsets of the 34 visibile tiles */
	int                         m_draw_phase;           /* MMC5 uses different regs for BG and OAM */
	ppu2c0x_latch_delegate      m_latch;

	// timers
	emu_timer                   *m_hblank_timer;        /* hblank period at end of each scanline */
	emu_timer                   *m_nmi_timer;           /* NMI timer */
	emu_timer                   *m_scanline_timer;      /* scanline timer */

	// some bootleg / clone hardware appears to ignore this
	static void use_sprite_write_limitation_disable(device_t &device)
	{
		ppu2c0x_device &dev = downcast<ppu2c0x_device &>(device);
		dev.m_use_sprite_write_limitation = false;
	}

	bool m_use_sprite_write_limitation;
private:
	static const device_timer_id TIMER_HBLANK = 0;
	static const device_timer_id TIMER_NMI = 1;
	static const device_timer_id TIMER_SCANLINE = 2;

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

};

class ppu2c02_device : public ppu2c0x_device {
public:
	ppu2c02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c03b_device : public ppu2c0x_device {
public:
	ppu2c03b_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c04_device : public ppu2c0x_device {
public:
	ppu2c04_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c07_device : public ppu2c0x_device {
public:
	ppu2c07_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c05_01_device : public ppu2c0x_device {
public:
	ppu2c05_01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c05_02_device : public ppu2c0x_device {
public:
	ppu2c05_02_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c05_03_device : public ppu2c0x_device {
public:
	ppu2c05_03_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class ppu2c05_04_device : public ppu2c0x_device {
public:
	ppu2c05_04_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


// device type definition
//extern const device_type PPU_2C0X;
extern const device_type PPU_2C02;  // NTSC NES
extern const device_type PPU_2C03B; // Playchoice 10
extern const device_type PPU_2C04;  // Vs. Unisystem
extern const device_type PPU_2C07;  // PAL NES
extern const device_type PPU_2C05_01;   // Vs. Unisystem (Ninja Jajamaru Kun)
extern const device_type PPU_2C05_02;   // Vs. Unisystem (Mighty Bomb Jack)
extern const device_type PPU_2C05_03;   // Vs. Unisystem (Gumshoe)
extern const device_type PPU_2C05_04;   // Vs. Unisystem (Top Gun)


#endif

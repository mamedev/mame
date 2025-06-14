// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi,Brad Oliver
/******************************************************************************

    Nintendo 2C0x PPU emulation.

    Written by Ernesto Corvi.
    This code is heavily based on Brad Oliver's MESS implementation.

******************************************************************************/

#ifndef MAME_VIDEO_PPU2C0X_H
#define MAME_VIDEO_PPU2C0X_H

#pragma once

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

/* constant definitions */
#define VISIBLE_SCREEN_WIDTH         (32*8) /* Visible screen width */
#define VISIBLE_SCREEN_HEIGHT        (30*8) /* Visible screen height */

///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> ppu2c0x_device

class ppu2c0x_device :  public device_t,
						public device_memory_interface,
						public device_video_interface,
						public device_palette_interface
{
public:
	typedef device_delegate<void (int scanline, bool vblank, bool blanked)> scanline_delegate;
	typedef device_delegate<void (int scanline, bool vblank, bool blanked)> hblank_delegate;
	typedef device_delegate<void (int *ppu_regs)> nmi_delegate;
	typedef device_delegate<int (int address, int data)> vidaccess_delegate;
	typedef device_delegate<void (offs_t offset)> latch_delegate;

	enum
	{
		NTSC_SCANLINES_PER_FRAME     = 262,
		PAL_SCANLINES_PER_FRAME      = 312,
		VS_CLONE_SCANLINES_PER_FRAME = 280,

		BOTTOM_VISIBLE_SCANLINE        = 239,
		VBLANK_FIRST_SCANLINE          = 241,
		VBLANK_FIRST_SCANLINE_PALC     = 291,
		VBLANK_FIRST_SCANLINE_VS_CLONE = 240,
		VBLANK_LAST_SCANLINE_NTSC      = 260,
		VBLANK_LAST_SCANLINE_PAL       = 310,
		VBLANK_LAST_SCANLINE_VS_CLONE  = 279

		// Both the scanline immediately before and immediately after VBLANK
		// are non-rendering and non-vblank.
	};

	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);
	virtual u8 palette_read(offs_t offset);
	virtual void palette_write(offs_t offset, u8 data);

	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }
	auto int_callback() { return m_int_callback.bind(); }

	void spriteram_dma(address_space &space, const u8 page);
	void set_spriteram_value(offs_t offset, u8 data) { m_spriteram[offset] = data; }

	void render(bitmap_rgb32 &bitmap, bool flipx, bool flipy, int sx, int sy, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	int get_current_scanline() { return m_scanline; }
	template <typename... T> void set_scanline_callback(T &&... args) { m_scanline_callback_proc.set(std::forward<T>(args)...); m_scanline_callback_proc.resolve(); /* FIXME: if this is supposed to be set at config time, it should be resolved on start */ }
	template <typename... T> void set_hblank_callback(T &&... args) { m_hblank_callback_proc.set(std::forward<T>(args)...); m_hblank_callback_proc.resolve(); /* FIXME: if this is supposed to be set at config time, it should be resolved on start */ }
	template <typename... T> void set_vidaccess_callback(T &&... args) { m_vidaccess_callback_proc.set(std::forward<T>(args)...); m_vidaccess_callback_proc.resolve(); /* FIXME: if this is supposed to be set at config time, it should be resolved on start */ }
	void set_scanlines_per_frame(int scanlines) { m_scanlines_per_frame = scanlines; }

	// MMC5 has to be able to check this
	int is_sprite_8x16() { return m_regs[PPU_CONTROL0] & PPU_CONTROL0_SPRITE_SIZE; }
	int get_draw_phase() { return m_draw_phase; }
	int get_tilenum() { return m_tilecount; }

	//27/12/2002 (HACK!)
	template <typename... T> void set_latch(T &&... args) { m_latch.set(std::forward<T>(args)...); m_latch.resolve(); /* FIXME: if this is supposed to be set at config time, it should be resolved on start */ }

	//  void update_screen(bitmap_t &bitmap, const rectangle &cliprect);

	u16 get_vram_dest();
	void set_vram_dest(u16 dest);

	bool in_vblanking() { return (m_scanline >= m_vblank_first_scanline - 1); }
protected:
	ppu2c0x_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, u32 clock, address_map_constructor internal_map);

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
		PPU_CONTROL0_INC               = 0x04,
		PPU_CONTROL0_SPR_SELECT        = 0x08,
		PPU_CONTROL0_CHR_SELECT        = 0x10,
		PPU_CONTROL0_SPRITE_SIZE       = 0x20,
		PPU_CONTROL0_NMI               = 0x80,

		PPU_CONTROL1_DISPLAY_MONO      = 0x01,
		PPU_CONTROL1_BACKGROUND_L8     = 0x02,
		PPU_CONTROL1_SPRITES_L8        = 0x04,
		PPU_CONTROL1_BACKGROUND        = 0x08,
		PPU_CONTROL1_SPRITES           = 0x10,
		PPU_CONTROL1_COLOR_EMPHASIS    = 0xe0,

		PPU_STATUS_8SPRITES            = 0x20,
		PPU_STATUS_SPRITE0_HIT         = 0x40,
		PPU_STATUS_VBLANK              = 0x80
	};

	// construction/destruction
	ppu2c0x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual u32 palette_entries() const noexcept override { return 0x40 * 8; }

	TIMER_CALLBACK_MEMBER(hblank_tick);
	TIMER_CALLBACK_MEMBER(nmi_tick);
	TIMER_CALLBACK_MEMBER(scanline_tick);

	/* routines */
	void apply_color_emphasis_and_clamp(bool is_pal_or_dendy, int color_emphasis, double& R, double& G, double& B);
	rgb_t nespal_to_RGB(int color_intensity, int color_num, int color_emphasis, bool is_pal_or_dendy);
	virtual void init_palette_tables();

	virtual void write_to_spriteram_with_increment(u8 data);

	virtual void read_tile_plane_data(int address, int color);
	virtual void shift_tile_plane_data(u8 &pix);
	virtual void draw_tile_pixel(u8 pix, int color, u32 back_pen, u32 *&dest);
	virtual void draw_tile(u8 *line_priority, int color_byte, int color_bits, int address, int start_x, u32 back_pen, u32 *&dest);
	virtual void draw_background( u8 *line_priority );
	virtual void draw_back_pen(u32* dst, int back_pen);
	void draw_background_pen();

	virtual void read_sprite_plane_data(int address);
	virtual void make_sprite_pixel_data(u8 &pixel_data, bool flipx);
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap);
	virtual bool is_spritepixel_opaque(int pixel_data, int color);
	virtual void draw_sprite_pixel_low(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, u8* line_priority);
	virtual void draw_sprite_pixel_high(bitmap_rgb32& bitmap, int pixel_data, int pixel, int sprite_xpos, int color, int sprite_index, u8* line_priority);
	virtual void read_extra_sprite_bits(int sprite_index);

	virtual int apply_sprite_pattern_page(int index1, int size);
	virtual void draw_sprites(u8 *line_priority);
	void render_scanline();
	virtual void scanline_increment_fine_ycounter();
	void update_visible_enabled_scanline();
	void update_visible_disabled_scanline();
	void update_visible_scanline();
	void update_scanline();

	// address space configurations
	const address_space_config      m_space_config;

	required_device<cpu_device> m_cpu;

	void start_nopalram();

	int                         m_scanlines_per_frame;  /* number of scanlines per frame */
	int                         m_security_value;       /* 2C05 protection */
	int                         m_vblank_first_scanline;  /* the very first scanline where VBLANK occurs */

	// used in rendering
	u8 m_planebuf[16]; // temp buffer used for fetching tile data
	s32                    m_scanline;         /* scanline count */
	std::unique_ptr<u8[]>  m_spriteram;           /* sprite ram */

	int m_videoram_addr_mask;
	int m_global_refresh_mask;
	int m_line_write_increment_large;
	bool m_paletteram_in_ppuspace; // sh6578 doesn't have the palette in PPU space, so various side-effects don't apply
	std::vector<u8>        m_palette_ram;          /* shouldn't be in main memory! */
	bitmap_rgb32                m_bitmap;          /* target bitmap */
	u8                          m_regs[PPU_MAX_REG];        /* registers */
	u32                         m_tile_page;            /* current tile page */
	u8                          m_back_color;           /* background color */
	s32                         m_refresh_data;         /* refresh-related */
	u8                          m_x_fine;               /* refresh-related */
	bool                        m_toggle;               /* used to latch hi-lo scroll */
	s32                         m_tilecount;            /* MMC5 can change attributes to subsets of the 34 visible tiles */
	latch_delegate              m_latch;

	u16                         m_spriteramsize;

	u8 readbyte(offs_t address);

	void ppu2c0x(address_map &map) ATTR_COLD;

private:
	inline void writebyte(offs_t address, u8 data);
	inline u16 apply_grayscale_and_emphasis(u8 color);

	scanline_delegate           m_scanline_callback_proc;   /* optional scanline callback */
	hblank_delegate             m_hblank_callback_proc; /* optional hblank callback */
	vidaccess_delegate          m_vidaccess_callback_proc;  /* optional video access callback */
	devcb_write_line            m_int_callback;         /* nmi access callback from interface */

	s32                         m_refresh_latch;        /* refresh-related */
	s32                         m_add;              /* vram increment amount */
	u32                         m_videomem_addr;        /* videomem address pointer */
	u8                          m_data_latch;           /* latched videomem data */
	u8                          m_buffered_data;
	u32                         m_sprite_page;          /* current sprite page */
	s32                         m_scan_scale;           /* scan scale */
	s32                         m_draw_phase;           /* MMC5 uses different regs for BG and OAM */

	// timers
	emu_timer                   *m_hblank_timer;        /* hblank period at end of each scanline */
	emu_timer                   *m_nmi_timer;           /* NMI timer */
	emu_timer                   *m_scanline_timer;      /* scanline timer */
};

class ppu2c0x_rgb_device : public ppu2c0x_device {
protected:
	ppu2c0x_rgb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock = 0);

	virtual void init_palette_tables() override;

private:
	required_region_ptr<u8> m_palette_data;
};

class ppu2c02_device : public ppu2c0x_device {
public:
	ppu2c02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c03b_device : public ppu2c0x_rgb_device {
public:
	ppu2c03b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c04_device : public ppu2c0x_rgb_device {
public:
	ppu2c04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c07_device : public ppu2c0x_device {
public:
	ppu2c07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppupalc_device : public ppu2c0x_device {
public:
	ppupalc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_01_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_02_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_02_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_03_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c05_04_device : public ppu2c0x_rgb_device {
public:
	ppu2c05_04_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

class ppu2c04_clone_device : public ppu2c0x_device {
public:
	ppu2c04_clone_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual u8 read(offs_t offset) override;
	virtual void write(offs_t offset, u8 data) override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return 0x40 * 2; }

	virtual void draw_background(u8 *line_priority) override;
	virtual void draw_sprite_pixel(int sprite_xpos, int color, int pixel, u8 pixel_data, bitmap_rgb32 &bitmap) override;
	virtual void draw_sprites(u8 *line_priority) override;

	virtual void init_palette_tables() override;

private:
	required_region_ptr<u8> m_palette_data;

	std::unique_ptr<u8[]>   m_spritebuf; /* buffered sprite ram for next frame */
};

// device type definition
DECLARE_DEVICE_TYPE(PPU_2C02,    ppu2c02_device)       // NTSC NES
DECLARE_DEVICE_TYPE(PPU_2C03B,   ppu2c03b_device)      // Playchoice 10
DECLARE_DEVICE_TYPE(PPU_2C04,    ppu2c04_device)       // Vs. Unisystem
DECLARE_DEVICE_TYPE(PPU_2C07,    ppu2c07_device)       // PAL NES
DECLARE_DEVICE_TYPE(PPU_PALC,    ppupalc_device)       // PAL Clones
DECLARE_DEVICE_TYPE(PPU_2C05_01, ppu2c05_01_device)    // Vs. Unisystem (Ninja Jajamaru Kun)
DECLARE_DEVICE_TYPE(PPU_2C05_02, ppu2c05_02_device)    // Vs. Unisystem (Mighty Bomb Jack)
DECLARE_DEVICE_TYPE(PPU_2C05_03, ppu2c05_03_device)    // Vs. Unisystem (Gumshoe)
DECLARE_DEVICE_TYPE(PPU_2C05_04, ppu2c05_04_device)    // Vs. Unisystem (Top Gun)
DECLARE_DEVICE_TYPE(PPU_2C04C,   ppu2c04_clone_device) // Vs. Unisystem (Super Mario Bros. bootlegs)

#endif // MAME_VIDEO_PPU2C0X_H

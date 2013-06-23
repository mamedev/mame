/*****************************************************************************
 *
 * includes/gb.h
 *
 ****************************************************************************/

#ifndef GB_H_
#define GB_H_

#include "audio/gb.h"
#include "machine/gb_slot.h"
#include "machine/ram.h"

/* Interrupts */
#define VBL_INT               0       /* V-Blank    */
#define LCD_INT               1       /* LCD Status */
#define TIM_INT               2       /* Timer      */
#define SIO_INT               3       /* Serial I/O */
#define EXT_INT               4       /* Joypad     */

#ifdef TIMER
#undef TIMER
#endif

/* Cartridge types */
#define CART_RAM    0x01    /* Cartridge has RAM                             */
#define BATTERY     0x02    /* Cartridge has a battery to save RAM           */
#define TIMER       0x04    /* Cartridge has a real-time-clock (MBC3 only)   */
#define RUMBLE      0x08    /* Cartridge has a rumble motor (MBC5 only)      */
#define SRAM        0x10    /* Cartridge has SRAM                            */
#define UNKNOWN     0x80    /* Cartridge is of an unknown type               */

#define DMG_FRAMES_PER_SECOND   59.732155
#define SGB_FRAMES_PER_SECOND   61.17


#define MAX_ROMBANK 512
#define MAX_RAMBANK 256


#define _NR_GB_VID_REGS     0x40




class gb_state : public driver_device
{
public:
	gb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cartslot(*this, "gbslot"),
		m_maincpu(*this, "maincpu"),
		m_custom(*this, "custom"),
		m_region_maincpu(*this, "maincpu"),
		m_rambank(*this, "cgb_ram"),
		m_inputs(*this, "INPUTS"),
		m_ram(*this, RAM_TAG) { }

	//gb_state driver_data;
	UINT8       m_gb_io[0x10];

	/* Timer related */
	UINT16      m_divcount;
	UINT8       m_shift;
	UINT16      m_shift_cycles;
	UINT8       m_triggering_irq;
	UINT8       m_reloading;

	/* Serial I/O related */
	UINT32      m_sio_count;             /* Serial I/O counter */
	emu_timer   *m_gb_serial_timer;

	/* SGB variables */
	INT8 m_sgb_packets;
	UINT8 m_sgb_bitcount;
	UINT8 m_sgb_bytecount;
	UINT8 m_sgb_start;
	UINT8 m_sgb_rest;
	UINT8 m_sgb_controller_no;
	UINT8 m_sgb_controller_mode;
	UINT8 m_sgb_data[0x100];

	/* CGB variables */
	UINT8       *m_gbc_rammap[8];           /* (CGB) Addresses of internal RAM banks */
	UINT8       m_gbc_rambank;          /* (CGB) Current CGB RAM bank */

	int m_bios_disable;

	DECLARE_WRITE8_MEMBER(gb_io_w);
	DECLARE_WRITE8_MEMBER(gb_io2_w);
	DECLARE_WRITE8_MEMBER(sgb_io_w);
	DECLARE_READ8_MEMBER(gb_ie_r);
	DECLARE_WRITE8_MEMBER(gb_ie_w);
	DECLARE_READ8_MEMBER(gb_io_r);
	DECLARE_WRITE8_MEMBER(gbc_io2_w);
	DECLARE_READ8_MEMBER(gbc_io2_r);
	DECLARE_MACHINE_START(gb);
	DECLARE_MACHINE_RESET(gb);
	DECLARE_PALETTE_INIT(gb);
	DECLARE_MACHINE_START(sgb);
	DECLARE_MACHINE_RESET(sgb);
	DECLARE_PALETTE_INIT(sgb);
	DECLARE_MACHINE_START(gbpocket);
	DECLARE_MACHINE_RESET(gbpocket);
	DECLARE_PALETTE_INIT(gbp);
	DECLARE_MACHINE_START(gbc);
	DECLARE_MACHINE_RESET(gbc);
	DECLARE_PALETTE_INIT(gbc);
	INTERRUPT_GEN_MEMBER(gb_scanline_interrupt);
	TIMER_CALLBACK_MEMBER(gb_serial_timer_proc);
	DECLARE_WRITE8_MEMBER(gb_timer_callback);

	DECLARE_READ8_MEMBER(gb_cart_r);
	DECLARE_READ8_MEMBER(gbc_cart_r);
	DECLARE_WRITE8_MEMBER(gb_bank_w);
	DECLARE_READ8_MEMBER(gb_ram_r);
	DECLARE_WRITE8_MEMBER(gb_ram_w);
	DECLARE_READ8_MEMBER(gb_echo_r);
	DECLARE_WRITE8_MEMBER(gb_echo_w);
	optional_device<gb_cart_slot_device> m_cartslot;

protected:
	required_device<lr35902_cpu_device> m_maincpu;
	required_device<gameboy_sound_device> m_custom;
	required_memory_region m_region_maincpu;
	optional_memory_bank m_rambank;   // cgb
	required_ioport m_inputs;
	optional_device<ram_device> m_ram;

	void gb_timer_increment();
	void gb_timer_check_irq();
	void gb_init();
	void gb_init_regs();
	void gb_video_reset(int mode);
	void gb_video_start(int mode);

	void save_gb_base();
	void save_gbc_only();
	void save_sgb_only();
};


class megaduck_state : public gb_state
{
public:
	megaduck_state(const machine_config &mconfig, device_type type, const char *tag)
		: gb_state(mconfig, type, tag)
		, m_cartslot(*this, "duckslot")
	{ }

	DECLARE_READ8_MEMBER(megaduck_video_r);
	DECLARE_WRITE8_MEMBER(megaduck_video_w);
	DECLARE_WRITE8_MEMBER(megaduck_sound_w1);
	DECLARE_READ8_MEMBER(megaduck_sound_r1);
	DECLARE_WRITE8_MEMBER(megaduck_sound_w2);
	DECLARE_READ8_MEMBER(megaduck_sound_r2);
	DECLARE_MACHINE_START(megaduck);
	DECLARE_MACHINE_RESET(megaduck);
	DECLARE_PALETTE_INIT(megaduck);

	DECLARE_READ8_MEMBER(cart_r);
	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(bank2_w);
	optional_device<megaduck_cart_slot_device> m_cartslot;
};


/*----------- defined in machine/gb.c -----------*/

/* -- Super Game Boy specific -- */
#define SGB_BORDER_PAL_OFFSET   64  /* Border colours stored from pal 4-7   */
#define SGB_XOFFSET             48  /* GB screen starts at column 48        */
#define SGB_YOFFSET             40  /* GB screen starts at row 40           */


/*----------- defined in video/gb.c -----------*/

enum
{
	GB_VIDEO_DMG = 1,
	GB_VIDEO_MGB,
	GB_VIDEO_SGB,
	GB_VIDEO_CGB
};


struct layer_struct {
	UINT8  enabled;
	UINT8  *bg_tiles;
	UINT8  *bg_map;
	UINT8  xindex;
	UINT8  xshift;
	UINT8  xstart;
	UINT8  xend;
	/* GBC specific */
	UINT8  *gbc_map;
	INT16  bgline;
};


class gb_lcd_device : public device_t
{
public:
	gb_lcd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(oam_r);
	DECLARE_WRITE8_MEMBER(oam_w);
	virtual DECLARE_READ8_MEMBER(video_r);
	virtual DECLARE_WRITE8_MEMBER(video_w);

	// FIXME: remove it when proper sgb support is added
	void set_sgb_hack(bool val) { m_sgb_border_hack = val ? 1 : 0; }
	
protected:
	inline void plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	
	void select_sprites();
	virtual void update_sprites();
	virtual void update_scanline();

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	void common_start();
	void common_reset();

	// pointer to the main system
	cpu_device *m_maincpu;
	screen_device *m_screen;
	
	// state variables
	bitmap_ind16 m_bitmap;

	UINT8 m_sgb_atf_data[4050];       /* (SGB) Attributes files */
	UINT32 m_sgb_atf;
	UINT16 m_sgb_pal_data[4096];
	UINT8 m_sgb_pal_map[20][18];
	UINT16 m_sgb_pal[128];
	UINT8 *m_sgb_tile_data;
	UINT8 m_sgb_tile_map[2048];
	UINT8 m_sgb_window_mask;
	
	// this is temporarily needed for a bunch of games which draw the border differently...
	int m_sgb_border_hack;
	
	int m_window_lines_drawn;
	
	UINT8   m_vid_regs[_NR_GB_VID_REGS];
	UINT8   m_bg_zbuf[160];
	
	UINT16  m_cgb_bpal[32];   /* CGB current background palette table */
	UINT16  m_cgb_spal[32];   /* CGB current sprite palette table */
	
	UINT8   m_gb_bpal[4];     /* Background palette */
	UINT8   m_gb_spal0[4];    /* Sprite 0 palette */
	UINT8   m_gb_spal1[4];    /* Sprite 1 palette */
	
	/* Things used to render current line */
	int m_current_line;       /* Current line */
	int m_cmp_line;           /* Compare line */
	int m_sprCount;           /* Number of sprites on current line */
	int m_sprite[10];         /* References to sprites to draw on current line */
	int m_previous_line;      /* Previous line we've drawn in */
	int m_start_x;            /* Pixel to start drawing from (inclusive) */
	int m_end_x;              /* Pixel to end drawing (exclusive) */
	int m_mode;               /* Keep track of internal STAT mode */
	int m_state;              /* Current state of the video state machine */
	int m_lcd_irq_line;
	int m_triggering_line_irq;
	int m_line_irq;
	int m_triggering_mode_irq;
	int m_mode_irq;
	int m_delayed_line_irq;
	int m_sprite_cycles;
	int m_scrollx_adjust;
	int m_oam_locked;
	int m_vram_locked;
	int m_pal_locked;
	int m_hdma_enabled;
	int m_hdma_possible;
	struct layer_struct m_layer[2];
	emu_timer *m_lcd_timer;
	int m_gbc_mode;
	
	UINT8   *m_vram;     // Pointer to VRAM
	UINT8   *m_oam;      // Pointer to OAM memory
	UINT8   m_gb_tile_no_mod;
	UINT32  m_gb_chrgen_offs;     // GB Character generator
	UINT32  m_gb_bgdtab_offs;     // GB Background character table
	UINT32  m_gb_wndtab_offs;     // GB Window character table
	UINT32  m_gbc_chrgen_offs;    // CGB Character generator
	UINT32  m_gbc_bgdtab_offs;    // CGB Background character table
	UINT32  m_gbc_wndtab_offs;    // CGB Window character table
	int     m_vram_bank;

	TIMER_CALLBACK_MEMBER(video_init_vbl);
	virtual TIMER_CALLBACK_MEMBER(lcd_timer_proc);
	virtual void videoptr_restore();
	void save_gb_video();
	void increment_scanline();
	void lcd_switch_on();
};


class mgb_lcd_device : public gb_lcd_device
{
public:
	mgb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
};


class sgb_lcd_device : public gb_lcd_device
{
public:
	sgb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	void sgb_io_write_pal(int offs, UINT8 *data);

protected:
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	virtual void update_sprites();
	virtual void update_scanline();
	void refresh_border();
};


class cgb_lcd_device : public gb_lcd_device
{
public:
	cgb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	virtual DECLARE_READ8_MEMBER(video_r);
	virtual DECLARE_WRITE8_MEMBER(video_w);

protected:
	
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	
	virtual void update_sprites();
	virtual void update_scanline();

	virtual TIMER_CALLBACK_MEMBER(lcd_timer_proc);
	virtual void videoptr_restore();
	void hdma_trans(UINT16 length);
};


extern const device_type GB_LCD_DMG;
extern const device_type GB_LCD_MGB;
extern const device_type GB_LCD_SGB;
extern const device_type GB_LCD_CGB;


#define MCFG_GB_LCD_DMG_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_DMG, 0 )

#define MCFG_GB_LCD_MGB_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_MGB, 0 )

#define MCFG_GB_LCD_SGB_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_SGB, 0 )

#define MCFG_GB_LCD_CGB_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_CGB, 0 )


#endif /* GB_H_ */

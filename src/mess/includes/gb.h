/*****************************************************************************
 *
 * includes/gb.h
 *
 ****************************************************************************/

#ifndef GB_H_
#define GB_H_

#include "machine/gb_slot.h"


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

struct gb_lcd_t {
	int window_lines_drawn;

	UINT8   gb_vid_regs[_NR_GB_VID_REGS];
	UINT8   bg_zbuf[160];

	UINT16  cgb_bpal[32];   /* CGB current background palette table */
	UINT16  cgb_spal[32];   /* CGB current sprite palette table */

	UINT8   gb_bpal[4];     /* Background palette */
	UINT8   gb_spal0[4];    /* Sprite 0 palette */
	UINT8   gb_spal1[4];    /* Sprite 1 palette */

	/* Things used to render current line */
	int current_line;       /* Current line */
	int cmp_line;           /* Compare line */
	int sprCount;           /* Number of sprites on current line */
	int sprite[10];         /* References to sprites to draw on current line */
	int previous_line;      /* Previous line we've drawn in */
	int start_x;            /* Pixel to start drawing from (inclusive) */
	int end_x;              /* Pixel to end drawing (exclusive) */
	int mode;               /* Keep track of internal STAT mode */
	int state;              /* Current state of the video state machine */
	int lcd_irq_line;
	int triggering_line_irq;
	int line_irq;
	int triggering_mode_irq;
	int mode_irq;
	int delayed_line_irq;
	int sprite_cycles;
	int scrollx_adjust;
	int oam_locked;
	int vram_locked;
	int pal_locked;
	int hdma_enabled;
	int hdma_possible;
	struct layer_struct layer[2];
	emu_timer *lcd_timer;
	int gbc_mode;

	memory_region *gb_vram;     /* Pointer to VRAM */
	memory_region *gb_oam;      /* Pointer to OAM memory */
	UINT8   *gb_vram_ptr;
	UINT8   *gb_chrgen;     /* Character generator */
	UINT8   *gb_bgdtab;     /* Background character table */
	UINT8   *gb_wndtab;     /* Window character table */
	UINT8   gb_tile_no_mod;
	UINT8   *gbc_chrgen;    /* CGB Character generator */
	UINT8   *gbc_bgdtab;    /* CGB Background character table */
	UINT8   *gbc_wndtab;    /* CGB Window character table */
};



class gb_state : public driver_device
{
public:
	gb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cartslot(*this, "gbslot")
		, m_maincpu(*this, "maincpu")
		, m_custom(*this, "custom")
		, m_region_maincpu(*this, "maincpu")
		, m_rambank(*this, "cgb_ram")
		, m_inputs(*this, "INPUTS")
	{ }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT16 m_sgb_pal_data[4096];
	UINT8 m_sgb_pal_map[20][18];
	UINT16 m_sgb_pal[128];
	UINT8 *m_sgb_tile_data;
	UINT8 m_sgb_tile_map[2048];
	UINT8 m_sgb_window_mask;
	//gb_state driver_data;
	UINT8       m_gb_io[0x10];

	/* Timer related */
	UINT16      m_divcount;
	UINT8       m_shift;
	UINT16      m_shift_cycles;
	UINT8       m_triggering_irq;
	UINT8       m_reloading;

	/* Serial I/O related */
	UINT32      m_SIOCount;             /* Serial I/O counter */
	emu_timer   *m_gb_serial_timer;

	/* SGB variables */
	UINT8       m_sgb_atf_data[4050];       /* (SGB) Attributes files */
	INT8 m_sgb_packets;
	UINT8 m_sgb_bitcount;
	UINT8 m_sgb_bytecount;
	UINT8 m_sgb_start;
	UINT8 m_sgb_rest;
	UINT8 m_sgb_controller_no;
	UINT8 m_sgb_controller_mode;
	UINT8 m_sgb_data[0x100];
	UINT32 m_sgb_atf;

	/* CGB variables */
	UINT8       *m_GBC_RAMMap[8];           /* (CGB) Addresses of internal RAM banks */
	UINT8       m_GBC_RAMBank;          /* (CGB) Current CGB RAM bank */


	gb_lcd_t m_lcd;
	void (gb_state::*update_scanline) ();
	bool m_bios_disable;

	bitmap_ind16 m_bitmap;
	DECLARE_WRITE8_MEMBER(gb_io_w);
	DECLARE_WRITE8_MEMBER(gb_io2_w);
	DECLARE_WRITE8_MEMBER(sgb_io_w);
	DECLARE_READ8_MEMBER(gb_ie_r);
	DECLARE_WRITE8_MEMBER(gb_ie_w);
	DECLARE_READ8_MEMBER(gb_io_r);
	DECLARE_WRITE8_MEMBER(gbc_io2_w);
	DECLARE_READ8_MEMBER(gbc_io2_r);
	DECLARE_READ8_MEMBER(gb_video_r);
	DECLARE_READ8_MEMBER(gb_vram_r);
	DECLARE_WRITE8_MEMBER(gb_vram_w);
	DECLARE_READ8_MEMBER(gb_oam_r);
	DECLARE_WRITE8_MEMBER(gb_oam_w);
	DECLARE_WRITE8_MEMBER(gb_video_w);
	DECLARE_READ8_MEMBER(gbc_video_r);
	DECLARE_WRITE8_MEMBER(gbc_video_w);
	DECLARE_MACHINE_START(gb);
	DECLARE_MACHINE_RESET(gb);
	DECLARE_PALETTE_INIT(gb);
	DECLARE_MACHINE_START(sgb);
	DECLARE_MACHINE_RESET(sgb);
	DECLARE_PALETTE_INIT(sgb);
	DECLARE_MACHINE_RESET(gbpocket);
	DECLARE_PALETTE_INIT(gbp);
	DECLARE_MACHINE_START(gbc);
	DECLARE_MACHINE_RESET(gbc);
	DECLARE_PALETTE_INIT(gbc);
	DECLARE_MACHINE_START(gb_video);
	DECLARE_MACHINE_START(gbc_video);
	INTERRUPT_GEN_MEMBER(gb_scanline_interrupt);
	TIMER_CALLBACK_MEMBER(gb_serial_timer_proc);
	TIMER_CALLBACK_MEMBER(gb_video_init_vbl);
	TIMER_CALLBACK_MEMBER(gb_lcd_timer_proc);
	TIMER_CALLBACK_MEMBER(gbc_lcd_timer_proc);
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
	required_device<device_t> m_custom;
	required_memory_region m_region_maincpu;
	optional_memory_bank m_rambank;   // cgb
	required_ioport m_inputs;

	void gb_timer_increment();
	void gb_timer_check_irq();
	void gb_init();
	void gb_init_regs();
	void gb_select_sprites();
	void gb_update_sprites();
	void gb_update_scanline();
	void sgb_update_sprites();
	void sgb_refresh_border();
	void sgb_update_scanline();
	void cgb_update_sprites();
	void cgb_update_scanline();
	void gb_video_reset( int mode );
	void gbc_hdma(UINT16 length);
	void gb_increment_scanline();
	void gb_lcd_switch_on();
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


#endif /* GB_H_ */

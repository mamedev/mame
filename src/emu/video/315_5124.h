/*************************************************************************

    sega315_5124.h

    Implementation of Sega VDP chips used in System E, Master System and Game Gear

**************************************************************************/

#ifndef __SEGA315_5124_H__
#define __SEGA315_5124_H__

#include "devcb.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SEGA315_5124_WIDTH                      342     /* 342 pixels */
#define SEGA315_5124_HEIGHT_NTSC                262     /* 262 lines */
#define SEGA315_5124_HEIGHT_PAL                 313     /* 313 lines */
#define SEGA315_5124_LBORDER_START              (1 + 2 + 14 + 8)
#define SEGA315_5124_LBORDER_WIDTH              13      /* 13 pixels */
#define SEGA315_5124_RBORDER_WIDTH              15      /* 15 pixels */
#define SEGA315_5124_TBORDER_START              (3 + 13)
#define SEGA315_5124_NTSC_192_TBORDER_HEIGHT    (0x1b)  /* 27 lines */
//#define SEGA315_5124_NTSC_192_BBORDER_HEIGHT  (0x18)  /* 24 lines */
#define SEGA315_5124_NTSC_224_TBORDER_HEIGHT    (0x0b)  /* 11 lines */
//#define SEGA315_5124_NTSC_224_BBORDER_HEIGHT  (0x08)  /* 8 lines */
//#define SEGA315_5124_PAL_192_TBORDER_HEIGHT   (0x36)  /* 54 lines */
//#define SEGA315_5124_PAL_192_BBORDER_HEIGHT   (0x30)  /* 48 lines */
//#define SEGA315_5124_PAL_224_TBORDER_HEIGHT   (0x26)  /* 38 lines */
//#define SEGA315_5124_PAL_224_BBORDER_HEIGHT   (0x20)  /* 32 lines */
#define SEGA315_5124_PAL_240_TBORDER_HEIGHT     (0x1e)  /* 30 lines */
//#define SEGA315_5124_PAL_240_BBORDER_HEIGHT   (0x18)  /* 24 lines */


#define SEGA315_5124_PALETTE_SIZE	(64+16)
#define SEGA315_5378_PALETTE_SIZE	4096

PALETTE_INIT( sega315_5124 );
PALETTE_INIT( sega315_5378 );


#define SEGA315_5378_CRAM_SIZE    0x40	/* 32 colors x 2 bytes per color = 64 bytes */
#define SEGA315_5124_CRAM_SIZE    0x20	/* 32 colors x 1 bytes per color = 32 bytes */

#define VRAM_SIZE             0x4000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct sega315_5124_interface
{
	bool               m_is_pal;             /* false = NTSC, true = PAL */
	const char         *m_screen_tag;
	devcb_write_line   m_int_callback;       /* Interrupt callback function */
	devcb_write_line   m_pause_callback;     /* Pause callback function */
};


extern const device_type SEGA315_5124;		/* aka SMS1 vdp */
extern const device_type SEGA315_5246;		/* aka SMS2 vdp */
extern const device_type SEGA315_5378;		/* aka Gamegear vdp */


class sega315_5124_device : public device_t,
                            public sega315_5124_interface,
                            public device_memory_interface
{
public:
	// construction/destruction
	sega315_5124_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	sega315_5124_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 cram_size, UINT8 palette_offset, bool supports_224_240);

	DECLARE_READ8_MEMBER( vram_read );
	DECLARE_WRITE8_MEMBER( vram_write );
	DECLARE_READ8_MEMBER( register_read );
	DECLARE_WRITE8_MEMBER( register_write );
	DECLARE_READ8_MEMBER( vcount_read );
	DECLARE_READ8_MEMBER( hcount_latch_read );
	DECLARE_WRITE8_MEMBER( hcount_latch_write );

	bitmap_rgb32 &get_bitmap() { return m_tmpbitmap; };
	bitmap_ind8 &get_y1_bitmap() { return m_y1_bitmap; };

	/* update the screen */
	UINT32 screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	virtual void set_sega315_5124_compatibility_mode( bool sega315_5124_compatibility_mode ) { };

protected:
	void set_display_settings();
	virtual void update_palette();
	virtual void draw_scanline( int pixel_offset_x, int pixel_plot_y, int line );
	virtual UINT16 get_name_table_address();
	void process_line_timer();
	void draw_scanline_mode4( int *line_buffer, int *priority_selected, int line );
	void draw_sprites_mode4( int *line_buffer, int *priority_selected, int pixel_plot_y, int line );
	void draw_sprites_tms9918_mode( int *line_buffer, int pixel_plot_y, int line );
	void draw_scanline_mode2( int *line_buffer, int line );
	void draw_scanline_mode0( int *line_buffer, int line );
	void select_sprites( int pixel_plot_y, int line );

	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_0) ? &m_space_config : NULL; }

	UINT8            m_reg[16];                  /* All the registers */
	UINT8            m_status;                   /* Status register */
	UINT8            m_reg9copy;                 /* Internal copy of register 9 */
	UINT8            m_addrmode;                 /* Type of VDP action */
	UINT16           m_addr;                     /* Contents of internal VDP address register */
	UINT8            m_cram_size;                /* CRAM size */
	UINT8            m_cram_mask;                /* Mask to switch between SMS and GG CRAM sizes */
	int              m_cram_dirty;               /* Have there been any changes to the CRAM area */
	int              m_pending;
	UINT8            m_buffer;
	bool             m_sega315_5124_compatibility_mode;    /* Shrunk SMS screen on GG lcd mode flag */
	int              m_irq_state;                /* The status of the IRQ line of the VDP */
	int              m_vdp_mode;                 /* Current mode of the VDP: 0,1,2,3,4 */
	int              m_y_pixels;                 /* 192, 224, 240 */
	int              m_draw_time;
	UINT8            m_line_counter;
	UINT8            m_hcounter;
	memory_region    *m_CRAM;                    /* Pointer to CRAM */
	const UINT8      *m_frame_timing;
	bitmap_rgb32     m_tmpbitmap;
	bitmap_ind8  m_y1_bitmap;
	UINT8            m_collision_buffer[SEGA315_5124_WIDTH];
	UINT8            m_palette_offset;
	bool             m_supports_224_240;
	UINT16           m_sprite_base;
	int              m_selected_sprite[8];
	int              m_sprite_count;
	int              m_sprite_height;
	int              m_sprite_zoom;

	/* line_buffer will be used to hold 5 lines of line data. Line #0 is the regular blitting area.
       Lines #1-#4 will be used as a kind of cache to be used for vertical scaling in the gamegear
       sms compatibility mode. */
	int              *m_line_buffer;
	int              m_current_palette[32];
	devcb_resolved_write_line	m_cb_int;
	devcb_resolved_write_line   m_cb_pause;
	emu_timer        *m_display_timer;
	emu_timer        *m_set_status_vint_timer;
	emu_timer        *m_set_status_sprovr_timer;
	emu_timer        *m_set_status_sprcol_timer;
	emu_timer        *m_check_hint_timer;
	emu_timer        *m_check_vint_timer;
	emu_timer        *m_draw_timer;
	screen_device    *m_screen;

	const address_space_config  m_space_config;

	/* Timers */
	static const device_timer_id TIMER_LINE = 0;
	static const device_timer_id TIMER_SET_STATUS_VINT = 1;
	static const device_timer_id TIMER_SET_STATUS_SPROVR = 2;
	static const device_timer_id TIMER_CHECK_HINT = 3;
	static const device_timer_id TIMER_CHECK_VINT = 4;
	static const device_timer_id TIMER_SET_STATUS_SPRCOL = 5;
	static const device_timer_id TIMER_DRAW = 6;
};


class sega315_5246_device : public sega315_5124_device
{
public:
	sega315_5246_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual UINT16 get_name_table_address();
};


class sega315_5378_device : public sega315_5124_device
{
public:
	sega315_5378_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual void device_reset();

	virtual void set_sega315_5124_compatibility_mode( bool sega315_5124_compatibility_mode );

protected:
	virtual void update_palette();
	virtual void draw_scanline( int pixel_offset_x, int pixel_plot_y, int line );
	virtual UINT16 get_name_table_address();
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SEGA315_5124_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, SEGA315_5124, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_SEGA315_5246_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, SEGA315_5246, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_SEGA315_5378_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, SEGA315_5378, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif /* __SEGA315_5124_H__ */

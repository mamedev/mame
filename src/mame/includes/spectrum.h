// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/spectrum.h
 *
 ****************************************************************************/

#ifndef __SPECTRUM_H__
#define __SPECTRUM_H__

#include "machine/upd765.h"
#include "sound/speaker.h"
#include "machine/ram.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

/* Spectrum crystals */

#define X1 XTAL_14MHz       // Main clock (48k Spectrum)
#define X1_128_AMSTRAD  35469000 // Main clock (Amstrad 128K model, +2A?)
#define X1_128_SINCLAIR 17734475 // Main clock (Sinclair 128K model)

#define X2 XTAL_4_433619MHz // PAL color subcarrier

/* Spectrum screen size in pixels */
#define SPEC_UNSEEN_LINES  16   /* Non-visible scanlines before first border
                                   line. Some of these may be vertical retrace. */
#define SPEC_TOP_BORDER    48   /* Number of border lines before actual screen */
#define SPEC_DISPLAY_YSIZE 192  /* Vertical screen resolution */
#define SPEC_BOTTOM_BORDER 56   /* Number of border lines at bottom of screen */
#define SPEC_SCREEN_HEIGHT (SPEC_TOP_BORDER + SPEC_DISPLAY_YSIZE + SPEC_BOTTOM_BORDER)

#define SPEC_LEFT_BORDER   48   /* Number of left hand border pixels */
#define SPEC_DISPLAY_XSIZE 256  /* Horizontal screen resolution */
#define SPEC_RIGHT_BORDER  48   /* Number of right hand border pixels */
#define SPEC_SCREEN_WIDTH (SPEC_LEFT_BORDER + SPEC_DISPLAY_XSIZE + SPEC_RIGHT_BORDER)

#define SPEC_LEFT_BORDER_CYCLES   24   /* Cycles to display left hand border */
#define SPEC_DISPLAY_XSIZE_CYCLES 128  /* Horizontal screen resolution */
#define SPEC_RIGHT_BORDER_CYCLES  24   /* Cycles to display right hand border */
#define SPEC_RETRACE_CYCLES       48   /* Cycles taken for horizonal retrace */
#define SPEC_CYCLES_PER_LINE      224  /* Number of cycles to display a single line */

/* 128K machines take an extra 4 cycles per scan line - add this to retrace */
#define SPEC128_UNSEEN_LINES    15
#define SPEC128_RETRACE_CYCLES  52
#define SPEC128_CYCLES_PER_LINE 228

/* Border sizes for TS2068. These are guesses based on the number of cycles
   available per frame. */
#define TS2068_TOP_BORDER    32
#define TS2068_BOTTOM_BORDER 32
#define TS2068_SCREEN_HEIGHT (TS2068_TOP_BORDER + SPEC_DISPLAY_YSIZE + TS2068_BOTTOM_BORDER)

/* Double the border sizes to maintain ratio of screen to border */
#define TS2068_LEFT_BORDER   96   /* Number of left hand border pixels */
#define TS2068_DISPLAY_XSIZE 512  /* Horizontal screen resolution */
#define TS2068_RIGHT_BORDER  96   /* Number of right hand border pixels */
#define TS2068_SCREEN_WIDTH (TS2068_LEFT_BORDER + TS2068_DISPLAY_XSIZE + TS2068_RIGHT_BORDER)

struct EVENT_LIST_ITEM
{
	/* driver defined ID for this write */
	int Event_ID;
	/* driver defined data for this write */
	int Event_Data;
	/* time at which this write occurred */
	int Event_Time;
};


enum
{
	TIMEX_CART_NONE,
	TIMEX_CART_DOCK,
	TIMEX_CART_EXROM,
	TIMEX_CART_HOME
};


class spectrum_state : public driver_device
{
public:
	spectrum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_speaker(*this, "speaker"),
		m_cart(*this, "cartslot"),
		m_dock(*this, "dockslot"),
		m_upd765(*this, "upd765"),
		m_upd765_0(*this, "upd765:0"),
		m_upd765_1(*this, "upd765:1"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_line5(*this, "LINE5"),
		m_io_line6(*this, "LINE6"),
		m_io_line7(*this, "LINE7"),
		m_io_nmi(*this, "NMI"),
		m_io_config(*this, "CONFIG"),
		m_io_joy_intf(*this, "JOY_INTF"),
		m_io_kempston(*this, "KEMPSTON"),
		m_io_fuller(*this, "FULLER"),
		m_io_mikrogen(*this, "MIKROGEN"),
		m_io_plus0(*this, "PLUS0"),
		m_io_plus1(*this, "PLUS1"),
		m_io_plus2(*this, "PLUS2"),
		m_io_plus3(*this, "PLUS3"),
		m_io_plus4(*this, "PLUS4") { }

	int m_port_fe_data;
	int m_port_7ffd_data;
	int m_port_1ffd_data;   /* scorpion and plus3 */
	int m_port_ff_data; /* Display enhancement control */
	int m_port_f4_data; /* Horizontal Select Register */

	int m_floppy;

	/* video support */
	int m_frame_invert_count;
	int m_frame_number;    /* Used for handling FLASH 1 */
	int m_flash_invert;
	optional_shared_ptr<UINT8> m_video_ram;
	UINT8 *m_screen_location;

	int m_ROMSelection;


	EVENT_LIST_ITEM *m_pCurrentItem;
	int m_NumEvents;
	int m_TotalEvents;
	char *m_pEventListBuffer;
	int m_LastFrameStartTime;
	int m_CyclesPerFrame;

	UINT8 *m_ram_0000;
	UINT8 m_ram_disabled_by_beta;
	DECLARE_WRITE8_MEMBER(spectrum_port_fe_w);
	DECLARE_READ8_MEMBER(spectrum_port_fe_r);
	DECLARE_READ8_MEMBER(spectrum_port_1f_r);
	DECLARE_READ8_MEMBER(spectrum_port_7f_r);
	DECLARE_READ8_MEMBER(spectrum_port_df_r);
	DECLARE_READ8_MEMBER(spectrum_port_ula_r);

	DECLARE_WRITE8_MEMBER(spectrum_128_port_7ffd_w);
	DECLARE_READ8_MEMBER(spectrum_128_ula_r);

	DECLARE_WRITE8_MEMBER(spectrum_plus3_port_3ffd_w);
	DECLARE_READ8_MEMBER(spectrum_plus3_port_3ffd_r);
	DECLARE_READ8_MEMBER(spectrum_plus3_port_2ffd_r);
	DECLARE_WRITE8_MEMBER(spectrum_plus3_port_7ffd_w);
	DECLARE_WRITE8_MEMBER(spectrum_plus3_port_1ffd_w);

	DECLARE_READ8_MEMBER(ts2068_port_f4_r);
	DECLARE_WRITE8_MEMBER(ts2068_port_f4_w);
	DECLARE_READ8_MEMBER(ts2068_port_ff_r);
	DECLARE_WRITE8_MEMBER(ts2068_port_ff_w);
	DECLARE_WRITE8_MEMBER(tc2048_port_ff_w);

	DECLARE_DRIVER_INIT(spectrum);
	DECLARE_DRIVER_INIT(plus2);
	DECLARE_DRIVER_INIT(plus3);
	DECLARE_MACHINE_RESET(spectrum);
	DECLARE_VIDEO_START(spectrum);
	DECLARE_PALETTE_INIT(spectrum);
	DECLARE_MACHINE_RESET(tc2048);
	DECLARE_VIDEO_START(spectrum_128);
	DECLARE_MACHINE_RESET(spectrum_128);
	DECLARE_MACHINE_RESET(spectrum_plus3);
	DECLARE_MACHINE_RESET(ts2068);
	DECLARE_VIDEO_START(ts2068);
	UINT32 screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tc2048(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ts2068(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_spectrum(screen_device &screen, bool state);
	void screen_eof_timex(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(spec_interrupt);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( spectrum_cart );

	// for timex cart only
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( timex_cart );
	int m_dock_cart_type, m_ram_chunks;
	memory_region *m_dock_crt;

	unsigned int m_previous_border_x, m_previous_border_y;
	bitmap_ind16 m_border_bitmap;
	unsigned int m_previous_screen_x, m_previous_screen_y;
	bitmap_ind16 m_screen_bitmap;

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	void spectrum_128_update_memory();
	void spectrum_plus3_update_memory();
	void ts2068_update_memory();

	DECLARE_SNAPSHOT_LOAD_MEMBER( spectrum );
	DECLARE_QUICKLOAD_LOAD_MEMBER( spectrum );

	required_device<cpu_device> m_maincpu;

protected:
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<speaker_sound_device> m_speaker;
	optional_device<generic_slot_device> m_cart;
	optional_device<generic_slot_device> m_dock;
	optional_device<upd765a_device> m_upd765;
	optional_device<floppy_connector> m_upd765_0;
	optional_device<floppy_connector> m_upd765_1;

	// Regular spectrum ports; marked as optional because of other subclasses
	optional_ioport m_io_line0;
	optional_ioport m_io_line1;
	optional_ioport m_io_line2;
	optional_ioport m_io_line3;
	optional_ioport m_io_line4;
	optional_ioport m_io_line5;
	optional_ioport m_io_line6;
	optional_ioport m_io_line7;
	optional_ioport m_io_nmi;
	optional_ioport m_io_config;
	optional_ioport m_io_joy_intf;
	optional_ioport m_io_kempston;
	optional_ioport m_io_fuller;
	optional_ioport m_io_mikrogen;
	// Plus ports
	optional_ioport m_io_plus0;
	optional_ioport m_io_plus1;
	optional_ioport m_io_plus2;
	optional_ioport m_io_plus3;
	optional_ioport m_io_plus4;

	void spectrum_UpdateBorderBitmap();
	void spectrum_UpdateScreenBitmap(bool eof = false);
	inline unsigned char get_display_color(unsigned char color, int invert);
	inline void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);
	void ts2068_hires_scanline(bitmap_ind16 &bitmap, int y, int borderlines);
	void ts2068_64col_scanline(bitmap_ind16 &bitmap, int y, int borderlines, unsigned short inkcolor);
	void ts2068_lores_scanline(bitmap_ind16 &bitmap, int y, int borderlines, int screen);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


/*----------- defined in drivers/spectrum.c -----------*/

INPUT_PORTS_EXTERN( spectrum );
INPUT_PORTS_EXTERN( spec_plus );

MACHINE_CONFIG_EXTERN( spectrum );

/*----------- defined in drivers/spec128.c -----------*/

MACHINE_CONFIG_EXTERN( spectrum_128 );

#endif /* __SPECTRUM_H__ */

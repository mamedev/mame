// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * includes/spectrum.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_SPECTRUM_H
#define MAME_INCLUDES_SPECTRUM_H

#pragma once

#include "machine/spec_snqk.h"
#include "machine/bankdev.h"
#include "bus/spectrum/exp.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"

/* Spectrum crystals */

#define X1 XTAL(14'000'000)       // Main clock (48k Spectrum)
#define X1_128_AMSTRAD  35469000 // Main clock (Amstrad 128K model, +2A?)
#define X1_128_SINCLAIR 17734475 // Main clock (Sinclair 128K model)

#define X2 XTAL(4'433'619) // PAL color subcarrier

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
#define SPEC_RETRACE_CYCLES       48   /* Cycles taken for horizontal retrace */
#define SPEC_CYCLES_PER_LINE      224  /* Number of cycles to display a single line */

struct EVENT_LIST_ITEM
{
	/* driver defined ID for this write */
	int Event_ID;
	/* driver defined data for this write */
	int Event_Data;
	/* time at which this write occurred */
	int Event_Time;
};




class spectrum_state : public driver_device
{
public:
	spectrum_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_specmem(*this, "specmem"),
		m_speaker(*this, "speaker"),
		m_exp(*this, "exp"),
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
		m_io_plus0(*this, "PLUS0"),
		m_io_plus1(*this, "PLUS1"),
		m_io_plus2(*this, "PLUS2"),
		m_io_plus3(*this, "PLUS3"),
		m_io_plus4(*this, "PLUS4"),
		m_io_joy1(*this, "JOY1"),
		m_io_joy2(*this, "JOY2")
	{ }

	void spectrum_common(machine_config &config);
	void spectrum(machine_config &config);
	void spectrum_clone(machine_config &config);

	void init_spectrum();

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// until machine/spec_snqk.cpp gets somehow disentangled
	virtual void plus3_update_memory() { }
	virtual void spectrum_128_update_memory() { }
	virtual void ts2068_update_memory() { }

	enum
	{
		TIMER_IRQ_ON,
		TIMER_IRQ_OFF,
		TIMER_SCANLINE
	};

	int m_port_fe_data;
	int m_port_7ffd_data;
	int m_port_1ffd_data;   /* scorpion and plus3 */
	int m_port_ff_data; /* Display enhancement control */
	int m_port_f4_data; /* Horizontal Select Register */

	/* video support */
	int m_frame_invert_count;
	int m_frame_number;    /* Used for handling FLASH 1 */
	int m_flash_invert;
	optional_shared_ptr<uint8_t> m_video_ram;
	uint8_t *m_screen_location;

	int m_ROMSelection;

	emu_timer *m_irq_off_timer;

	// Build up the screen bitmap line-by-line as the z80 uses CPU cycles.
	// Elimiates sprite flicker on various games (E.g. Marauder and
	// Stormlord) and makes Firefly playable.
	emu_timer *m_scanline_timer;

	EVENT_LIST_ITEM *m_pCurrentItem;
	int m_NumEvents;
	int m_TotalEvents;
	char *m_pEventListBuffer;
	int m_LastFrameStartTime;
	int m_CyclesPerLine;

	uint8_t m_ram_disabled_by_beta;
	uint8_t pre_opcode_fetch_r(offs_t offset);
	void spectrum_rom_w(offs_t offset, uint8_t data);
	uint8_t spectrum_rom_r(offs_t offset);
	uint8_t spectrum_data_r(offs_t offset);
	void spectrum_data_w(offs_t offset, uint8_t data);

	void spectrum_ula_w(offs_t offset, uint8_t data);
	uint8_t spectrum_ula_r(offs_t offset);
	void spectrum_port_w(offs_t offset, uint8_t data);
	virtual uint8_t spectrum_port_r(offs_t offset);
	virtual uint8_t floating_bus_r();
	uint8_t spectrum_clone_port_r(offs_t offset);

	void spectrum_palette(palette_device &palette) const;
	virtual uint32_t screen_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_spectrum);
	INTERRUPT_GEN_MEMBER(spec_interrupt);

	unsigned int m_previous_border_x, m_previous_border_y;
	bitmap_ind16 m_border_bitmap;
	unsigned int m_previous_screen_x, m_previous_screen_y;
	bitmap_ind16 m_screen_bitmap;

	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	void spectrum_io(address_map &map);
	void spectrum_clone_io(address_map &map);
	void spectrum_opcodes(address_map &map);
	void spectrum_map(address_map &map);
	void spectrum_data(address_map &map);

	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	optional_device<address_map_bank_device> m_specmem;
	required_device<speaker_sound_device> m_speaker;
	optional_device<spectrum_expansion_slot_device> m_exp;

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

	// Plus ports
	optional_ioport m_io_plus0;
	optional_ioport m_io_plus1;
	optional_ioport m_io_plus2;
	optional_ioport m_io_plus3;
	optional_ioport m_io_plus4;

	// Joystick ports
	optional_ioport m_io_joy1;
	optional_ioport m_io_joy2;

	virtual void spectrum_UpdateBorderBitmap();
	virtual u16 get_border_color();
	virtual void spectrum_UpdateScreenBitmap(bool eof = false);
	inline unsigned char get_display_color(unsigned char color, int invert);
	inline void spectrum_plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);

	// snapshot helpers
	void update_paging();
	void page_basicrom();
	void border_update(int data);
	void setup_sp(uint8_t *snapdata, uint32_t snapsize);
	void setup_sna(uint8_t *snapdata, uint32_t snapsize);
	void setup_ach(uint8_t *snapdata, uint32_t snapsize);
	void setup_prg(uint8_t *snapdata, uint32_t snapsize);
	void setup_plusd(uint8_t *snapdata, uint32_t snapsize);
	void setup_sem(uint8_t *snapdata, uint32_t snapsize);
	void setup_sit(uint8_t *snapdata, uint32_t snapsize);
	void setup_zx(uint8_t *snapdata, uint32_t snapsize);
	void setup_snp(uint8_t *snapdata, uint32_t snapsize);
	void snx_decompress_block(address_space &space, uint8_t *source, uint16_t dest, uint16_t size);
	void setup_snx(uint8_t *snapdata, uint32_t snapsize);
	void setup_frz(uint8_t *snapdata, uint32_t snapsize);
	void z80_decompress_block(address_space &space, uint8_t *source, uint16_t dest, uint16_t size);
	void setup_z80(uint8_t *snapdata, uint32_t snapsize);

	// quickload helpers
	void log_quickload(const char *type, uint32_t start, uint32_t length, uint32_t exec, const char *exec_format);
	void setup_scr(uint8_t *quickdata, uint32_t quicksize);
	void setup_raw(uint8_t *quickdata, uint32_t quicksize);
};

class spectrum_128_state : public spectrum_state
{
public:
	spectrum_128_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectrum_state(mconfig, type, tag)
		{ }

	void spectrum_128(machine_config &config);

protected:
	virtual void video_start() override;
	virtual void machine_reset() override;

	virtual void spectrum_128_update_memory() override;

private:
	uint8_t spectrum_128_pre_opcode_fetch_r(offs_t offset);
	void spectrum_128_bank1_w(offs_t offset, uint8_t data);
	uint8_t spectrum_128_bank1_r(offs_t offset);
	void spectrum_128_port_7ffd_w(offs_t offset, uint8_t data);
	virtual uint8_t spectrum_port_r(offs_t offset) override;
	virtual uint8_t floating_bus_r() override;
	//uint8_t spectrum_128_ula_r();

	void spectrum_128_io(address_map &map);
	void spectrum_128_mem(address_map &map);
	void spectrum_128_fetch(address_map &map);
};

/*----------- defined in drivers/spectrum.cpp -----------*/

INPUT_PORTS_EXTERN( spectrum );
INPUT_PORTS_EXTERN( spec128 );
INPUT_PORTS_EXTERN( spec_plus );

#endif // MAME_INCLUDES_SPECTRUM_H

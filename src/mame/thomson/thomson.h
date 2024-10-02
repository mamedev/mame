// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#ifndef MAME_THOMSON_THOMSON_H
#define MAME_THOMSON_THOMSON_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/mc6846.h"
#include "machine/ram.h"
#include "sound/dac.h"
#include "bus/thomson/extension.h"
#include "bus/thomson/nanoreseau.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"


/* 6821 PIAs */
#define THOM_PIA_SYS    "pia_0"  /* system PIA */
#define THOM_PIA_GAME   "pia_1"  /* music & game PIA (joypad + sound) */

/* bank-switching */
#define THOM_CART_BANK  "bank2" /* cartridge ROM */
#define THOM_RAM_BANK   "bank3" /* data RAM */
#define THOM_BASE_BANK  "bank5" /* system RAM */

/* bank-switching */
#define TO8_SYS_LO      "bank5" /* system RAM low 2 Kb */
#define TO8_SYS_HI      "bank6" /* system RAM hi 2 Kb */
#define TO8_DATA_LO     "bank7" /* data RAM low 2 Kb */
#define TO8_DATA_HI     "bank8" /* data RAM hi 2 Kb */
#define TO8_BIOS_BANK   "bank9" /* BIOS ROM */
#define MO6_CART_LO     "bank10"
#define MO6_CART_HI     "bank11"


/* original screen dimension (may be different from emulated screen!) */
#define THOM_ACTIVE_WIDTH  320
#define THOM_BORDER_WIDTH   56
#define THOM_ACTIVE_HEIGHT 200
#define THOM_BORDER_HEIGHT  47
#define THOM_TOTAL_WIDTH   432
#define THOM_TOTAL_HEIGHT  294

/* Emulated screen dimension may be doubled to allow hi-res 640x200 mode.
   Emulated screen can have smaller borders.
 */

/* page 0 is banked */
#define THOM_VRAM_BANK "bank1"


struct thom_vsignal {
	unsigned count;  /* pixel counter */
	unsigned init;   /* 1 -> active vertical windos, 0 -> border/VBLANK */
	unsigned inil;   /* 1 -> active horizontal window, 0 -> border/HBLANK */
	unsigned lt3;    /* bit 3 of us counter */
	unsigned line;   /* line counter */
};


class thomson_state : public driver_device
{
public:
	thomson_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_dac(*this, "dac"),
		m_pia_sys(*this, THOM_PIA_SYS),
		m_pia_game(*this, THOM_PIA_GAME),
		m_ram(*this, RAM_TAG),
		m_mc6846(*this, "mc6846"),
		m_screen(*this, "screen"),
		m_mainirq(*this, "mainirq"),
		m_mainfirq(*this, "mainfirq"),
		m_extension(*this, "extension"),
		m_io_game_port_directions(*this, "game_port_directions"),
		m_io_game_port_buttons(*this, "game_port_buttons"),
		m_io_mouse_x(*this, "mouse_x"),
		m_io_mouse_y(*this, "mouse_y"),
		m_io_mouse_button(*this, "mouse_button"),
		m_io_lightpen_x(*this, "lightpen_x"),
		m_io_lightpen_y(*this, "lightpen_y"),
		m_io_lightpen_button(*this, "lightpen_button"),
		m_io_config(*this, "config"),
		m_io_vconfig(*this, "vconfig"),
		m_io_keyboard(*this, "keyboard.%u", 0),
		m_vrambank(*this, THOM_VRAM_BANK),
		m_cartbank(*this, THOM_CART_BANK),
		m_rambank(*this, THOM_RAM_BANK),
		m_basebank(*this, THOM_BASE_BANK),
		m_cart_rom(*this, "cartridge"),
		m_caps_led(*this, "led0")
	{
	}

	void to7_base(machine_config &config, bool is_mo);
	void to7(machine_config &config);
	void to770a(machine_config &config);
	void t9000(machine_config &config);
	void to770(machine_config &config);

	void to770_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5alt_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4alt_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4althalf_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page1_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page2_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlayhalf_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay3_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16alt_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void to770_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5alt_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4alt_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4althalf_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page1_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page2_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlayhalf_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay3_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16alt_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void to9_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_to9_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void to9_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_to9_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );

protected:
	emu_timer* m_mo5_periodic_timer = nullptr;
	uint8_t m_mo5_reg_cart = 0; /* 0xa7cb bank switch */

	virtual void video_start() override ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( to7_cartridge );

	void to7_set_cassette_motor(int state);
	void thom_dev_irq_0(int state);
	void to7_cartridge_w(offs_t offset, uint8_t data);
	uint8_t to7_cartridge_r(offs_t offset);
	void to7_timer_port_out(uint8_t data);
	uint8_t to7_timer_port_in();
	void to7_set_cassette(int state);
	void to7_sys_cb2_out(int state);
	void to7_sys_portb_out(uint8_t data);
	uint8_t to7_sys_porta_in();
	uint8_t to7_sys_portb_in();
	uint8_t to7_game_porta_in();
	uint8_t to7_game_portb_in();
	void to7_game_portb_out(uint8_t data);
	void to7_game_cb2_out(int state);
	TIMER_CALLBACK_MEMBER( to7_game_update_cb );
	DECLARE_MACHINE_RESET( to7 );
	DECLARE_MACHINE_START( to7 );
	void to770_sys_cb2_out(int state);
	uint8_t to770_sys_porta_in();
	void to7_update_cart_bank_postload();
	void to770_update_ram_bank_postload();
	void to770_sys_portb_out(uint8_t data);
	void to770_timer_port_out(uint8_t data);
	uint8_t to770_gatearray_r(offs_t offset);
	void to770_gatearray_w(offs_t offset, uint8_t data);
	DECLARE_MACHINE_RESET( to770 );
	DECLARE_MACHINE_START( to770 );
	void to7_lightpen_cb( int step );

	TIMER_CALLBACK_MEMBER( thom_lightpen_step );
	TIMER_CALLBACK_MEMBER( thom_scanline_start );
	uint32_t screen_update_thom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void to7_vram_w(offs_t offset, uint8_t data);
	void to770_vram_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER( thom_set_init );

	void thom_vblank(int state);

	uint8_t to7_5p14_r(offs_t offset);
	void to7_5p14_w(offs_t offset, uint8_t data);
	uint8_t to7_5p14sd_r(offs_t offset);
	void to7_5p14sd_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER( ans4 );
	TIMER_CALLBACK_MEMBER( ans3 );
	TIMER_CALLBACK_MEMBER( ans2 );
	TIMER_CALLBACK_MEMBER( ans );
	void thom_palette(palette_device &palette);

	void to7_map(address_map &map) ATTR_COLD;
	void to770_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_byte_interface> m_dac;
	required_device<pia6821_device> m_pia_sys;
	required_device<pia6821_device> m_pia_game;
	required_device<ram_device> m_ram;
	optional_device<mc6846_device> m_mc6846;
	required_device<screen_device> m_screen;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_mainfirq;
	required_device<thomson_extension_device> m_extension;
	required_ioport m_io_game_port_directions;
	required_ioport m_io_game_port_buttons;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;
	required_ioport m_io_mouse_button;
	required_ioport m_io_lightpen_x;
	required_ioport m_io_lightpen_y;
	required_ioport m_io_lightpen_button;
	required_ioport m_io_config;
	required_ioport m_io_vconfig;
	required_ioport_array<10> m_io_keyboard;
	required_memory_bank m_vrambank;
	optional_memory_bank m_cartbank;
	optional_memory_bank m_rambank;
	required_memory_bank m_basebank;
	required_region_ptr<uint8_t> m_cart_rom;

	output_finder<> m_caps_led;

	/* bank logging and optimisations */
	int m_old_cart_bank = 0;
	int m_old_cart_bank_was_read_only = 0;
	int m_old_ram_bank = 0;
	/* buffer storing demodulated bits, only for k7 and with speed hack */
	uint32_t m_to7_k7_bitsize = 0;
	uint8_t* m_to7_k7_bits = 0;
	/* ------------ cartridge ------------ */
	uint8_t m_thom_cart_nb_banks = 0; /* number of 16 KB banks (up to 4) */
	uint8_t m_thom_cart_bank = 0;     /* current bank */
	uint8_t m_to7_lightpen_step = 0;
	uint8_t m_to7_lightpen = 0;
	/* calls to7_game_update_cb periodically */
	emu_timer* m_to7_game_timer = nullptr;
	uint8_t m_to7_game_sound = 0;
	uint8_t m_to7_game_mute = 0;

	/* We allow choosing dynamically:
	   - the border size
	   - whether we use 640 pixels or 320 pixels in an active row
	   (now this is automatically chosen by default for each frame)
	*/
	uint16_t m_thom_bwidth = 0;
	uint16_t m_thom_bheight = 0;
	/* border size */
	uint8_t  m_thom_hires = 0;
	/* 0 = low res: 320x200 active area (faster)
	   1 = hi res:  640x200 active area (can represent all video modes)
	*/
	uint8_t m_thom_hires_better = 0;
	/* 1 = a 640 mode was used in the last frame */
	/* we use our own video timing to precisely cope with VBLANK and HBLANK */
	emu_timer* m_thom_video_timer = nullptr; /* time elapsed from beginning of frame */
	/* number of lightpen call-backs per frame */
	int m_thom_lightpen_nb = 0;
	/* called thom_lightpen_nb times */
	emu_timer *m_thom_lightpen_timer = nullptr;
	/* lightpen callback function to call from timer */
	std::function<void (int)> m_thom_lightpen_cb;
	uint8_t* m_thom_vram = nullptr; /* pointer to video memory */
	emu_timer* m_thom_scanline_timer = nullptr; /* scan-line update */
	uint16_t m_thom_last_pal[16]{};   /* palette at last scanline start */
	uint16_t m_thom_pal[16]{};        /* current palette */
	bool     m_thom_pal_changed = false;    /* whether pal != old_pal */
	uint8_t  m_thom_border_index = 0;   /* current border color index */
	/* the left and right border color for each row (including top and bottom
	   border rows); -1 means unchanged wrt last scanline
	*/
	int16_t m_thom_border_l[THOM_TOTAL_HEIGHT+1]{};
	int16_t m_thom_border_r[THOM_TOTAL_HEIGHT+1]{};
	/* active area, updated one scan-line at a time every 64us,
	   then blitted in screen_update
	*/
	uint16_t m_thom_vbody[640*200]{};
	uint8_t m_thom_vmode = 0; /* current vide mode */
	uint8_t m_thom_vpage = 0; /* current video page */
	/* this stores the video mode & page at each GPL in the current line
	   (-1 means unchanged)
	*/
	int16_t m_thom_vmodepage[41]{};
	uint8_t m_thom_vmodepage_changed = 0;
	/* one dirty flag for each video memory line */
	bool m_thom_vmem_dirty[205]{};
	/* set to 1 if undirty scanlines need to be redrawn due to other video state
	   changes */
	bool m_thom_vstate_dirty = false;
	bool m_thom_vstate_last_dirty = false;
	uint32_t m_thom_mode_point = 0;
	emu_timer *m_thom_init_timer = nullptr;
	void (thomson_state::*m_thom_init_cb)( int init );

	int to7_get_cassette();
	void thom_irq_reset();
	void to7_update_cart_bank();
	void to7_set_init( int init );
	uint8_t to7_get_mouse_signal();
	void to7_game_sound_update();
	void to7_game_init();
	void to7_game_reset();
	void to770_update_ram_bank();

	TIMER_CALLBACK_MEMBER( mo5_periodic_cb );
	int mo5_get_cassette();
	void mo5_set_cassette( int data );
	void mo5_init_timer();

	bool update_screen_size();
	unsigned thom_video_elapsed();
	struct thom_vsignal thom_get_vsignal();
	void thom_get_lightpen_pos( int*x, int* y );
	struct thom_vsignal thom_get_lightpen_vsignal( int xdec, int ydec, int xdec2 );
	void thom_set_lightpen_callback( int nb );
	int thom_mode_is_hires( int mode );
	void thom_border_changed();
	void thom_gplinfo_changed();
	void thom_set_border_color( unsigned index );
	void thom_set_palette( unsigned index, uint16_t color );
	void thom_set_video_mode( unsigned mode );
	void thom_set_video_page( unsigned page );
	void thom_set_mode_point( int point );
	unsigned to7_lightpen_gpl( int decx, int decy );
	void thom_configure_palette( double gamma, const uint16_t* pal, palette_device& palette );

	void mo5_set_cassette_motor(int state);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( mo5_cartridge );
};

class mo5_state : public thomson_state
{
public:
	mo5_state(const machine_config &mconfig, device_type type, const char *tag) :
		thomson_state(mconfig, type, tag)
	{
	}

	void mo5(machine_config &config);
	void mo5e(machine_config &config);

protected:
	void mo5_lightpen_cb( int step );
	void mo5_sys_porta_out(uint8_t data);
	uint8_t mo5_sys_porta_in();
	uint8_t mo5_sys_portb_in();
	uint8_t mo5_gatearray_r(offs_t offset);
	void mo5_gatearray_w(offs_t offset, uint8_t data);
	void mo5_update_cart_bank_postload();
	void mo5_cartridge_w(offs_t offset, uint8_t data);
	uint8_t mo5_cartridge_r(offs_t offset);
	void mo5_ext_w(uint8_t data);
	DECLARE_MACHINE_RESET( mo5 );
	DECLARE_MACHINE_START( mo5 );

	void mo5_palette(palette_device &palette);

	void mo5_map(address_map &map) ATTR_COLD;

	void mo5_update_cart_bank();
};

class to9_state : public thomson_state
{
public:
	to9_state(const machine_config &mconfig, device_type type, const char *tag) :
		thomson_state(mconfig, type, tag),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_syslobank(*this, TO8_SYS_LO),
		m_syshibank(*this, TO8_SYS_HI),
		m_datalobank(*this, TO8_DATA_LO),
		m_datahibank(*this, TO8_DATA_HI),
		m_biosbank(*this, TO8_BIOS_BANK)
	{
	}

	void to8(machine_config &config);
	void to8d(machine_config &config);
	void to9(machine_config &config);
	void to9p(machine_config &config);

protected:
	optional_device<centronics_device> m_centronics;
	optional_device<output_latch_device> m_cent_data_out;

	required_memory_bank m_syslobank;
	optional_memory_bank m_syshibank;
	optional_memory_bank m_datalobank;
	optional_memory_bank m_datahibank;
	optional_memory_bank m_biosbank;

	int m_centronics_busy = 0;

	uint8_t  m_to8_kbd_ack = 0;       /* 1 = cpu inits / accepts transfers */
	uint16_t m_to8_kbd_data = 0;      /* data to transmit */
	uint16_t m_to8_kbd_step = 0;      /* transmission automaton state */
	uint8_t  m_to8_kbd_last_key = 0;  /* last key (for repetition) */
	uint32_t m_to8_kbd_key_count = 0; /* keypress time (for repetition)  */
	uint8_t  m_to8_kbd_caps = 0;      /* caps lock */
	emu_timer* m_to8_kbd_timer = nullptr;   /* bit-send */
	emu_timer* m_to8_kbd_signal = nullptr;  /* signal from CPU */
	uint8_t m_to8_data_vpage = 0;
	uint8_t m_to8_cart_vpage = 0;
	uint8_t  m_to8_reg_ram = 0;
	uint8_t  m_to8_reg_cart = 0;
	uint8_t  m_to8_reg_sys1 = 0;
	uint8_t  m_to8_reg_sys2 = 0;
	uint8_t  m_to8_lightpen_intr = 0;
	uint8_t  m_to8_soft_select = 0;
	uint8_t  m_to8_soft_bank = 0;
	uint8_t  m_to8_bios_bank = 0;

	TIMER_CALLBACK_MEMBER( to8_kbd_timer_cb );
	void to8_update_ram_bank_postload();
	void to8_update_cart_bank_postload();
	void to8_cartridge_w(offs_t offset, uint8_t data);
	uint8_t to8_cartridge_r(offs_t offset);
	uint8_t to8_gatearray_r(offs_t offset);
	void to8_gatearray_w(offs_t offset, uint8_t data);
	uint8_t to8_vreg_r(offs_t offset);
	void to8_vreg_w(offs_t offset, uint8_t data);
	uint8_t to8_sys_porta_in();
	void to8_sys_portb_out(uint8_t data);
	uint8_t to8_timer_port_in();
	void to8_timer_port_out(uint8_t data);
	void to8_timer_cp2_out(int state);
	void to8_lightpen_cb( int step );
	DECLARE_MACHINE_RESET( to8 );
	DECLARE_MACHINE_START( to8 );

	void write_centronics_busy(int state);

	void to8_sys_lo_w(offs_t offset, uint8_t data);
	void to8_sys_hi_w(offs_t offset, uint8_t data);
	void to8_data_lo_w(offs_t offset, uint8_t data);
	void to8_data_hi_w(offs_t offset, uint8_t data);
	void to8_vcart_w(offs_t offset, uint8_t data);

	int to8_kbd_ktest();
	int to8_kbd_get_key();
	void to8_kbd_timer_func();
	void to8_kbd_set_ack( int data );
	void to8_kbd_reset();
	void to8_kbd_init();
	void to8_update_ram_bank();
	void to8_update_cart_bank();

	void to9_ieee_w(offs_t offset, uint8_t data);
	uint8_t to9_ieee_r(offs_t offset);
	uint8_t to9_gatearray_r(offs_t offset);
	void to9_gatearray_w(offs_t offset, uint8_t data);
	uint8_t to9_vreg_r(offs_t offset);
	void to9_vreg_w(offs_t offset, uint8_t data);
	void to9_update_cart_bank_postload();
	void to9_cartridge_w(offs_t offset, uint8_t data);
	uint8_t to9_cartridge_r(offs_t offset);
	void to9_update_ram_bank_postload();
	uint8_t to9_kbd_r(offs_t offset);
	void to9_kbd_w(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER( to9_kbd_timer_cb );
	uint8_t to9_sys_porta_in();
	void to9_sys_porta_out(uint8_t data);
	void to9_sys_portb_out(uint8_t data);
	void to9_timer_port_out(uint8_t data);
	DECLARE_MACHINE_RESET( to9 );
	DECLARE_MACHINE_START( to9 );
	uint8_t to9p_timer_port_in();
	void to9p_timer_port_out(uint8_t data);
	DECLARE_MACHINE_RESET( to9p );
	DECLARE_MACHINE_START( to9p );

	void to8_map(address_map &map) ATTR_COLD;
	void to9_map(address_map &map) ATTR_COLD;
	void to9p_map(address_map &map) ATTR_COLD;

	uint8_t m_to9_palette_data[32]{};
	uint8_t m_to9_palette_idx = 0;
	uint8_t m_to9_soft_bank = 0;
	uint8_t  m_to9_kbd_parity = 0;  /* 0=even, 1=odd, 2=no parity */
	uint8_t  m_to9_kbd_intr = 0;    /* interrupt mode */
	uint8_t  m_to9_kbd_in = 0;      /* data from keyboard */
	uint8_t  m_to9_kbd_status = 0;  /* status */
	uint8_t  m_to9_kbd_overrun = 0; /* character lost */
	uint8_t  m_to9_kbd_periph = 0;     /* peripheral mode */
	uint8_t  m_to9_kbd_byte_count = 0; /* byte-count in peripheral mode */
	uint16_t m_to9_mouse_x = 0;
	uint16_t m_to9_mouse_y = 0;
	uint8_t  m_to9_kbd_last_key = 0;  /* for key repetition */
	uint16_t m_to9_kbd_key_count = 0;
	uint8_t  m_to9_kbd_caps = 0;  /* caps-lock */
	uint8_t  m_to9_kbd_pad = 0;   /* keypad outputs special codes */
	emu_timer* m_to9_kbd_timer = nullptr;

	void to9_set_video_mode( uint8_t data, int style );
	void to9_palette_init();
	void to9_update_cart_bank();
	void to9_update_ram_bank();
	int to9_kbd_ktest();
	void to9_kbd_update_irq();
	void to9_kbd_send( uint8_t data, int parity );
	int to9_kbd_get_key();
	void to9_kbd_reset();
	void to9_kbd_init();
};

class mo6_state : public to9_state
{
public:
	mo6_state(const machine_config &mconfig, device_type type, const char *tag) :
		to9_state(mconfig, type, tag),
		m_cartlobank(*this, MO6_CART_LO),
		m_carthibank(*this, MO6_CART_HI)
	{
	}

	void mo6(machine_config &config);
	void pro128(machine_config &config);

	DECLARE_MACHINE_RESET( mo6 );
	DECLARE_MACHINE_START( mo6 );

protected:
	optional_memory_bank m_cartlobank;
	optional_memory_bank m_carthibank;

	void mo6_update_ram_bank_postload();
	void mo6_update_cart_bank_postload();
	void mo6_cartridge_w(offs_t offset, uint8_t data);
	uint8_t mo6_cartridge_r(offs_t offset);
	void mo6_ext_w(uint8_t data);
	void mo6_centronics_busy(int state);
	void mo6_game_porta_out(uint8_t data);
	void mo6_game_cb2_out(int state);
	TIMER_CALLBACK_MEMBER( mo6_game_update_cb );
	uint8_t mo6_sys_porta_in();
	uint8_t mo6_sys_portb_in();
	void mo6_sys_porta_out(uint8_t data);
	void mo6_sys_cb2_out(int state);
	uint8_t mo6_gatearray_r(offs_t offset);
	void mo6_gatearray_w(offs_t offset, uint8_t data);
	uint8_t mo6_vreg_r(offs_t offset);
	void mo6_vreg_w(offs_t offset, uint8_t data);
	void mo6_vcart_lo_w(offs_t offset, uint8_t data);
	void mo6_vcart_hi_w(offs_t offset, uint8_t data);
	void mo6_map(address_map &map) ATTR_COLD;
	void mo6_update_ram_bank();
	void mo6_update_cart_bank();
	void mo6_game_init();
	void mo6_game_reset();
};

class mo5nr_state : public mo6_state
{
public:
	mo5nr_state(const machine_config &mconfig, device_type type, const char *tag) :
		mo6_state(mconfig, type, tag),
		m_nanoreseau(*this, "nanoreseau"),
		m_nanoreseau_config(*this, "nanoreseau_config"),
		m_extension_view(*this, "extension_view")
	{
	}

	void mo5nr(machine_config &config);

	DECLARE_MACHINE_RESET( mo5nr );
	DECLARE_MACHINE_START( mo5nr );

protected:
	required_device<nanoreseau_device> m_nanoreseau;
	required_ioport m_nanoreseau_config;
	memory_view m_extension_view;

	void mo5nr_map(address_map &map) ATTR_COLD;

	void mo5nr_game_init();
	void mo5nr_game_reset();

	uint8_t id_r();

	uint8_t mo5nr_net_r(offs_t offset);
	void mo5nr_net_w(offs_t offset, uint8_t data);
	uint8_t mo5nr_sys_portb_in();
	void mo5nr_sys_porta_out(uint8_t data);
};

/*----------- defined in video/thomson.cpp -----------*/

/*
   TO7 video:
   one line (64 us) =
      56 left border pixels ( 7 us)
   + 320 active pixels (40 us)
   +  56 right border pixels ( 7 us)
   +     horizontal retrace (10 us)

   one image (20 ms) =
      47 top border lines (~3 ms)
   + 200 active lines (12.8 ms)
   +  47 bottom border lines (~3 ms)
   +     vertical retrace (~1 ms)

   TO9 and up introduced a half (160 pixels) and double (640 pixels)
   horizontal mode, but still in 40 us (no change in refresh rate).
*/


/***************************** dimensions **************************/


/*********************** video signals *****************************/

/***************************** commons *****************************/


/* video modes */
#define THOM_VMODE_TO770       0
#define THOM_VMODE_MO5         1
#define THOM_VMODE_BITMAP4     2
#define THOM_VMODE_BITMAP4_ALT 3
#define THOM_VMODE_80          4
#define THOM_VMODE_BITMAP16    5
#define THOM_VMODE_PAGE1       6
#define THOM_VMODE_PAGE2       7
#define THOM_VMODE_OVERLAY     8
#define THOM_VMODE_OVERLAY3    9
#define THOM_VMODE_TO9        10
#define THOM_VMODE_80_TO9     11
#define THOM_VMODE_BITMAP4_ALT_HALF 12
#define THOM_VMODE_MO5_ALT    13
#define THOM_VMODE_OVERLAY_HALF     14
#define THOM_VMODE_BITMAP16_ALT 15
#define THOM_VMODE_NB         16



#endif // MAME_THOMSON_THOMSON_H

// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#ifndef MAME_THOMSON_THOMSON_H
#define MAME_THOMSON_THOMSON_H

#pragma once

#include "to_kbd.h"
#include "to_video.h"

#include "cpu/m6809/m6809.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/mc6846.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/dac.h"
#include "bus/thomson/extension.h"
#include "bus/thomson/nanoreseau.h"

#include "bus/centronics/ctronics.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"


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

/* page 0 is banked */
#define THOM_VRAM_BANK "bank1"


class thomson_state : public driver_device
{
public:
	thomson_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "video"),
		m_cassette(*this, "cassette"),
		m_dac(*this, "dac"),
		m_pia_sys(*this, THOM_PIA_SYS),
		m_pia_game(*this, THOM_PIA_GAME),
		m_ram(*this, RAM_TAG),
		m_mc6846(*this, "mc6846"),
		m_mainirq(*this, "mainirq"),
		m_mainfirq(*this, "mainfirq"),
		m_extension(*this, "extension"),
		m_io_game_port_directions(*this, "game_port_directions"),
		m_io_game_port_buttons(*this, "game_port_buttons"),
		m_io_mouse_x(*this, "mouse_x"),
		m_io_mouse_y(*this, "mouse_y"),
		m_io_mouse_button(*this, "mouse_button"),
		m_io_lightpen_button(*this, "lightpen_button"),
		m_io_config(*this, "config"),
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

protected:
	uint8_t m_mo5_reg_cart = 0; /* 0xa7cb bank switch */

	virtual void video_start() override ATTR_COLD;

	uint8_t *get_vram_page(int page) { return m_thom_vram + page * 0x4000; }

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( to7_cartridge );

	void to7_lightpen_cb( int step );

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
	DECLARE_MACHINE_RESET( to770 );
	DECLARE_MACHINE_START( to770 );

	void to7_vram_w(offs_t offset, uint8_t data);
	void to770_vram_w(offs_t offset, uint8_t data);

	void to9_ieee_w(offs_t offset, uint8_t data);
	uint8_t to9_ieee_r(offs_t offset);

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
	required_device<thomson_video_device> m_video;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_byte_interface> m_dac;
	required_device<pia6821_device> m_pia_sys;
	required_device<pia6821_device> m_pia_game;
	required_device<ram_device> m_ram;
	optional_device<mc6846_device> m_mc6846;
	required_device<input_merger_device> m_mainirq;
	required_device<input_merger_device> m_mainfirq;
	required_device<thomson_extension_device> m_extension;
	required_ioport m_io_game_port_directions;
	required_ioport m_io_game_port_buttons;
	required_ioport m_io_mouse_x;
	required_ioport m_io_mouse_y;
	required_ioport m_io_mouse_button;
	required_ioport m_io_lightpen_button;
	required_ioport m_io_config;
	optional_ioport_array<9> m_io_keyboard;
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
	/* calls to7_game_update_cb periodically */
	emu_timer* m_to7_game_timer = nullptr;
	uint8_t m_to7_game_sound = 0;
	uint8_t m_to7_game_mute = 0;

	uint8_t* m_thom_vram = nullptr; /* pointer to video memory */
	uint32_t m_thom_mode_point = 0;

	uint8_t m_to9_palette_data[32]{};
	uint8_t m_to9_palette_idx = 0;

	int to7_get_cassette();
	void to7_update_cart_bank();
	void to7_set_init( int init );
	uint8_t to7_get_mouse_signal();
	void to7_game_sound_update();
	void to7_game_init();
	void to7_game_reset();
	void to770_update_ram_bank();

	int mo5_get_cassette();
	void mo5_set_cassette( int data );
	void mo5_init_timer();

	void thom_set_mode_point( int point );
	void thom_configure_palette( double gamma, const uint16_t* pal, palette_device& palette );
	void to9_palette_init();

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
		m_to9_kbd(*this, "to9_kbd"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:%u", 0U),
		m_centronics(*this, "centronics")
	{
	}

	void to9(machine_config &config);

protected:
	required_device<to9_keyboard_device> m_to9_kbd;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;

	int m_centronics_busy = 0;

	void write_centronics_busy(int state);

	uint8_t to9_vreg_r(offs_t offset);
	void to9_vreg_w(offs_t offset, uint8_t data);
	void to9_update_cart_bank_postload();
	void to9_cartridge_w(offs_t offset, uint8_t data);
	uint8_t to9_cartridge_r(offs_t offset);
	void to9_update_ram_bank_postload();
	uint8_t to9_sys_porta_in();
	void to9_sys_porta_out(uint8_t data);
	void to9_sys_portb_out(uint8_t data);
	void to9_timer_port_out(uint8_t data);
	uint8_t to9_fdc_r(offs_t offset);
	void to9_fdc_w(offs_t offset, uint8_t data);
	uint8_t to9_floppy_control_r();
	void to9_floppy_control_w(uint8_t data);
	DECLARE_MACHINE_RESET( to9 );
	DECLARE_MACHINE_START( to9 );

	void to9_map(address_map &map) ATTR_COLD;

	uint8_t m_to9_floppy_control = 0;
	uint8_t m_to9_soft_bank = 0;

	void to9_update_cart_bank();
	void to9_update_ram_bank();
};

class to8_state : public thomson_state
{
public:
	to8_state(const machine_config &mconfig, device_type type, const char *tag) :
		thomson_state(mconfig, type, tag),
		m_to8_kbd(*this, "to8_kbd"),
		m_to9_kbd(*this, "to9_kbd"),
		m_centronics(*this, "centronics"),
		m_video(*this, "video"),
		m_syslobank(*this, TO8_SYS_LO),
		m_syshibank(*this, TO8_SYS_HI),
		m_datalobank(*this, TO8_DATA_LO),
		m_datahibank(*this, TO8_DATA_HI),
		m_biosbank(*this, TO8_BIOS_BANK)
	{
	}

	void to8(machine_config &config);
	void to8d(machine_config &config);
	void to9p(machine_config &config);

protected:
	optional_device<to8_keyboard_device> m_to8_kbd;
	optional_device<to9_keyboard_device> m_to9_kbd;
	optional_device<centronics_device> m_centronics;
	required_device<to8_video_device> m_video;

	required_memory_bank m_syslobank;
	required_memory_bank m_syshibank;
	required_memory_bank m_datalobank;
	required_memory_bank m_datahibank;
	required_memory_bank m_biosbank;

	int m_centronics_busy = 0;

	uint8_t m_to8_data_vpage = 0;
	uint8_t m_to8_cart_vpage = 0;
	uint8_t  m_to8_soft_select = 0;
	uint8_t  m_to8_soft_bank = 0;
	uint8_t  m_to8_bios_bank = 0;

	void to8_update_ram_bank_postload();
	void to8_update_cart_bank_postload();
	void to8_cartridge_w(offs_t offset, uint8_t data);
	uint8_t to8_cartridge_r(offs_t offset);
	uint8_t to8_vreg_r(offs_t offset);
	void to8_vreg_w(offs_t offset, uint8_t data);
	uint8_t to8_sys_porta_in();
	void to8_sys_porta_out(uint8_t data);
	void to8_sys_portb_out(uint8_t data);
	uint8_t to8_timer_port_in();
	void to8_timer_port_out(uint8_t data);
	void to8_timer_cp2_out(int state);
	DECLARE_MACHINE_RESET( to8 );
	DECLARE_MACHINE_START( to8 );

	void write_centronics_busy(int state);

	void to8_sys_lo_w(offs_t offset, uint8_t data);
	void to8_sys_hi_w(offs_t offset, uint8_t data);
	void to8_data_lo_w(offs_t offset, uint8_t data);
	void to8_data_hi_w(offs_t offset, uint8_t data);
	void to8_vcart_w(offs_t offset, uint8_t data);

	void to8_update_ram_bank();
	void to8_update_cart_bank();

	uint8_t to9p_sys_porta_in();
	uint8_t to9p_timer_port_in();
	void to9p_timer_port_out(uint8_t data);
	DECLARE_MACHINE_START( to9p );

	void to8_map(address_map &map) ATTR_COLD;
	void to9p_map(address_map &map) ATTR_COLD;
};

class mo6_state : public to8_state
{
public:
	mo6_state(const machine_config &mconfig, device_type type, const char *tag) :
		to8_state(mconfig, type, tag),
		m_cartlobank(*this, MO6_CART_LO),
		m_carthibank(*this, MO6_CART_HI),
		m_cent_data_out(*this, "cent_data_out")
	{
	}

	void mo6(machine_config &config);
	void pro128(machine_config &config);

	DECLARE_MACHINE_RESET( mo6 );
	DECLARE_MACHINE_START( mo6 );

protected:
	optional_memory_bank m_cartlobank;
	optional_memory_bank m_carthibank;

	optional_device<output_latch_device> m_cent_data_out;

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

#endif // MAME_THOMSON_THOMSON_H

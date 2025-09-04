// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/*****************************************************************************
 *
 * BBC Model B/B+/Master/Compact
 *
 * Driver by Gordon Jefferyes <mess_bbc@romvault.com>
 *
 ****************************************************************************/

#ifndef MAME_ACORN_BBC_H
#define MAME_ACORN_BBC_H

#pragma once

#include "bus/rs232/rs232.h"
#include "bus/centronics/ctronics.h"
#include "bus/econet/econet.h"
#include "bus/bbc/rom/slot.h"
#include "bus/bbc/fdc/fdc.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/internal/internal.h"
#include "bus/bbc/tube/tube.h"
#include "bus/bbc/userport/userport.h"
#include "bus/bbc/exp/exp.h"
#include "bus/bbc/cart/slot.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/74259.h"
#include "machine/mc6854.h"
#include "machine/input_merger.h"
#include "machine/ram.h"
#include "machine/upd7002.h"
#include "sound/sn76496.h"
#include "sound/samples.h"
#include "video/mc6845.h"
#include "video/saa5050.h"

#include "emupal.h"
#include "screen.h"


INPUT_PORTS_EXTERN(bbc_config);


class bbc_state : public driver_device
{
public:
	bbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_irqs(*this, "irqs")
		, m_palette(*this, "palette")
		, m_samples(*this, "samples")
		, m_trom(*this, "saa5050")
		, m_cassette(*this, "cassette")
		, m_latch(*this, "latch")
		, m_sysvia(*this, "sysvia")
		, m_uservia(*this, "uservia")
		, m_analog(*this, "analogue")
		, m_tube(*this, "tube")
		, m_intube(*this, "intube")
		, m_extube(*this, "extube")
		, m_1mhzbus(*this, "1mhzbus")
		, m_userport(*this, "userport")
		, m_internal(*this, "internal")
		, m_exp(*this, "exp")
		, m_fdc(*this, "fdc")
		, m_rom(*this, "romslot%u", 0U)
		, m_cart(*this, "cartslot%u", 1U)
		, m_region_mos(*this, "mos")
		, m_region_rom(*this, "rom")
		, m_bbcconfig(*this, "BBCCONFIG")
		, m_motor_led(*this, "motor_led")
	{ }

	static void floppy_formats(format_registration &fr);

	uint8_t mos_r(offs_t offset);
	void mos_w(offs_t offset, uint8_t data);

	int get_analogue_input(int channel_number);

	void init_bbc();
	void init_ltmp();
	void init_cfa();

	DECLARE_INPUT_CHANGED_MEMBER(reset_palette);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<hd6845s_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<input_merger_device> m_irqs;
	required_device<palette_device> m_palette;
	optional_device<samples_device> m_samples;
	optional_device<saa5050_device> m_trom;
	optional_device<cassette_image_device> m_cassette;
	required_device<ls259_device> m_latch;
	required_device<via6522_device> m_sysvia;
	optional_device<via6522_device> m_uservia;
	optional_device<bbc_analogue_slot_device> m_analog;
	optional_device<bbc_tube_slot_device> m_tube;
	optional_device<bbc_tube_slot_device> m_intube;
	optional_device<bbc_tube_slot_device> m_extube;
	optional_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	optional_device<bbc_userport_slot_device> m_userport;
	optional_device<bbc_internal_slot_device> m_internal;
	optional_device<bbc_exp_slot_device> m_exp;
	optional_device<bbc_fdc_slot_device> m_fdc;
	optional_device_array<bbc_romslot_device, 16> m_rom;
	optional_device_array<bbc_cartslot_device, 2> m_cart;

	required_memory_region m_region_mos;
	required_memory_region m_region_rom;
	optional_ioport m_bbcconfig;

	output_finder<> m_motor_led;

	void set_cpu_clock(offs_t offset);
	void update_nmi();
	void adlc_irq_w(int state);
	void bus_nmi_w(int state);
	void econet_int_enable(int enabled);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	void cassette_motor(int state);

	void video_ula_w(offs_t offset, uint8_t data);
	void hsync_changed(int state);
	void vsync_changed(int state);
	void de_changed(int state);
	MC6845_RECONFIGURE(crtc_reconfigure);
	MC6845_UPDATE_ROW(crtc_update_row);

	enum class monitor_type
	{
		COLOUR,
		BLACKWHITE,
		GREEN,
		AMBER
	};

	void update_palette(monitor_type monitor_type);

	std::string get_rom_name(uint8_t* header);
	void insert_device_rom(memory_region *rom);
	void setup_device_roms();

	int m_romsel = 0;           // This is the latch that holds the sideways ROM bank to read
	int m_paged_ram = 0;        // BBC B+ memory handling
	int m_vdusel = 0;           // BBC B+ memory handling

	// interrupt state
	int m_adlc_irq = 0;
	int m_adlc_ie = 0;
	int m_fdc_irq = 0;
	int m_fdc_drq = 0;
	int m_bus_nmi = 0;


	/**************************************
	  Video Code
	***************************************/

	// this is the real location of the start of the BBC's ram in the emulation
	// it can be changed if shadow ram is being used to point at the upper 32K of RAM
	uint8_t *m_video_ram = nullptr;
	uint8_t m_pixel_bits[256]{};

	uint8_t m_teletext_latch = 0;
	uint8_t m_vula_ctrl = 0;

	struct video_nula {
		uint8_t palette_mode = 0;
		uint8_t horiz_offset = 0;
		uint8_t left_blank = 0;
		uint8_t disable = 0;
		uint8_t attr_mode = 0;
		uint8_t attr_text = 0;
		uint8_t flash[8]{};
		uint8_t palette_byte = 0;
		uint8_t palette_write = 0;
	} m_vnula;

	uint8_t m_pixels_per_byte = 0;
	uint8_t m_pixel_width = 0;
	uint8_t m_cursor_size = 0;

	uint8_t m_vula_palette[16]{};
	uint8_t m_vula_palette_lookup[16]{};

	void setvideoshadow(int vdusel);
	uint8_t bus_video_data();
	uint16_t calculate_video_address(uint16_t ma, uint8_t ra);
};


#endif // MAME_ACORN_BBC_H

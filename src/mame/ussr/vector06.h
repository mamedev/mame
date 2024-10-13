// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/vector06.h
 *
 ****************************************************************************/
#ifndef MAME_USSR_VECTOR06_H
#define MAME_USSR_VECTOR06_H

#pragma once

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "cpu/i8085/i8085.h"

#include "imagedev/cassette.h"
#include "imagedev/floppy.h"

#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

#include "sound/ay8910.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"

class vector06_state : public driver_device
{
public:
	vector06_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_cart(*this, "cartslot")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_ay(*this, "aysnd")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_pit(*this, "pit")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_io_keyboard(*this, "LINE.%u", 0U)
		, m_io_reset(*this, "RESET")
	{ }

	void vector06(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(f11_button);
	DECLARE_INPUT_CHANGED_MEMBER(f12_button);

private:
	static void floppy_formats(format_registration &fr);

	uint8_t ppi1_portb_r();
	uint8_t ppi1_portc_r();
	void ppi1_porta_w(uint8_t data);
	void ppi1_portb_w(uint8_t data);
	void color_set(uint8_t data);
	uint8_t ppi2_portb_r();
	void ppi2_portb_w(uint8_t data);
	void ppi2_porta_w(uint8_t data);
	void ppi2_portc_w(uint8_t data);
	void disc_w(uint8_t data);
	void status_callback(uint8_t data);
	void ramdisk_w(uint8_t data);
	void speaker_w(int state);
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_mem();

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<i8080_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart;
	required_device<kr1818vg93_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<ay8910_device> m_ay;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<i8255_device> m_ppi1, m_ppi2;
	required_device<pit8253_device> m_pit;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport_array<9> m_io_keyboard;
	required_ioport m_io_reset;

	uint8_t m_keyboard_mask = 0;
	uint8_t m_color_index = 0;
	uint8_t m_romdisk_msb = 0;
	uint8_t m_romdisk_lsb = 0;
	uint8_t m_vblank_state = 0;
	uint8_t m_rambank = 0;
	uint8_t m_aylatch = 0;
	bool m_video_mode = false;
	bool m_stack_state = false;
	bool m_romen = false;
};

#endif // MAME_USSR_VECTOR06_H

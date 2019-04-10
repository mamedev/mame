// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/vector06.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_VECTOR06_H
#define MAME_INCLUDES_VECTOR06_H

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
	vector06_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_cart(*this, "cartslot"),
		m_fdc(*this, "wd1793"),
		m_floppy0(*this, "wd1793:0"),
		m_floppy1(*this, "wd1793:1"),
		m_ay(*this, "aysnd"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_ppi8255(*this, "ppi8255"),
		m_ppi8255_2(*this, "ppi8255_2"),
		m_pit8253(*this, "pit8253"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_region_maincpu(*this, "maincpu"),
		m_line(*this, "LINE.%u", 0),
		m_reset(*this, "RESET")
	{ }

	void vector06(machine_config &config);

private:
	DECLARE_FLOPPY_FORMATS(floppy_formats);

	DECLARE_READ8_MEMBER(vector06_8255_portb_r);
	DECLARE_READ8_MEMBER(vector06_8255_portc_r);
	DECLARE_WRITE8_MEMBER(vector06_8255_porta_w);
	DECLARE_WRITE8_MEMBER(vector06_8255_portb_w);
	DECLARE_WRITE8_MEMBER(vector06_color_set);
	DECLARE_READ8_MEMBER(vector06_romdisk_portb_r);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_portb_w);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_porta_w);
	DECLARE_WRITE8_MEMBER(vector06_romdisk_portc_w);
	DECLARE_WRITE8_MEMBER(vector06_disc_w);
	DECLARE_WRITE8_MEMBER(vector06_status_callback);
	DECLARE_WRITE8_MEMBER(vector06_ramdisk_w);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	void vector06_set_video_mode(int width);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_vector06(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vector06_interrupt);
	TIMER_CALLBACK_MEMBER(reset_check_callback);
	IRQ_CALLBACK_MEMBER(vector06_irq_callback);

	void vector06_io(address_map &map);
	void vector06_mem(address_map &map);

	required_device<i8080_cpu_device> m_maincpu;
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
	required_device<i8255_device> m_ppi8255, m_ppi8255_2;
	required_device<pit8253_device> m_pit8253;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_memory_region m_region_maincpu;
	required_ioport_array<9> m_line;
	required_ioport m_reset;

	uint8_t m_keyboard_mask;
	uint8_t m_color_index;
	uint8_t m_video_mode;
	uint8_t m_romdisk_msb;
	uint8_t m_romdisk_lsb;
	uint8_t m_vblank_state;
	uint8_t m_rambank;
	uint8_t m_aylatch;
	bool m_stack_state;
	bool m_romen;
	emu_timer *m_reset_check_timer;

	void update_mem();
};

#endif // MAME_INCLUDES_VECTOR06_H

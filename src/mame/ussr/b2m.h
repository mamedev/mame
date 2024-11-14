// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/b2m.h
 *
 ****************************************************************************/

#ifndef MAME_USSR_B2M_H
#define MAME_USSR_B2M_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "emupal.h"

class b2m_state : public driver_device
{
public:
	b2m_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_pit(*this, "pit")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_fdc(*this, "fdc")
		, m_fd(*this, "fdc:%u", 0U)
		, m_pic(*this, "pic")
	{ }

	void b2mrom(machine_config &config);
	void b2m(machine_config &config);

private:
	uint8_t keyboard_r(offs_t offset);
	void palette_w(offs_t offset, uint8_t data);
	uint8_t palette_r(offs_t offset);
	void localmachine_w(uint8_t data);
	uint8_t localmachine_r();

	void b2m_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	void pit_out1(int state);
	void ppi1_porta_w(uint8_t data);
	void ppi1_portb_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);
	uint8_t ppi1_portb_r();
	void ppi2_portc_w(uint8_t data);
	uint8_t romdisk_porta_r();
	void romdisk_portb_w(uint8_t data);
	void romdisk_portc_w(uint8_t data);
	void fdc_drq(int state);
	static void b2m_floppy_formats(format_registration &fr);

	void b2m_io(address_map &map) ATTR_COLD;
	void b2m_mem(address_map &map) ATTR_COLD;
	void b2m_rom_io(address_map &map) ATTR_COLD;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	void postload();
	void set_bank(int bank);

	uint8_t m_porta = 0U;
	uint8_t m_video_scroll = 0U;
	uint8_t m_portc = 0U;

	uint8_t m_video_page = 0U;

	uint8_t m_romdisk_lsb = 0U;
	uint8_t m_romdisk_msb = 0U;

	uint8_t m_color[4]{};
	uint8_t m_localmachine = 0U;
	uint8_t m_vblank_state = 0U;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pit8253_device> m_pit;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	/* devices */
	optional_device<fd1793_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_fd;
	optional_device<pic8259_device> m_pic;
};

#endif // MAME_USSR_B2M_H

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/b2m.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_B2M_H
#define MAME_INCLUDES_B2M_H

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
		, m_pit(*this, "pit8253")
		, m_ram(*this, RAM_TAG)
		, m_palette(*this, "palette")
		, m_fdc(*this, "fd1793")
		, m_fd(*this, "fd%u", 0U)
		, m_pic(*this, "pic8259")
	{ }

	DECLARE_READ8_MEMBER(b2m_keyboard_r);
	DECLARE_WRITE8_MEMBER(b2m_palette_w);
	DECLARE_READ8_MEMBER(b2m_palette_r);
	DECLARE_WRITE8_MEMBER(b2m_localmachine_w);
	DECLARE_READ8_MEMBER(b2m_localmachine_r);
	void init_b2m();

	void b2m_palette(palette_device &palette) const;
	uint32_t screen_update_b2m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(b2m_vblank_interrupt);
	DECLARE_WRITE_LINE_MEMBER(bm2_pit_out1);
	DECLARE_WRITE8_MEMBER(b2m_8255_porta_w);
	DECLARE_WRITE8_MEMBER(b2m_8255_portb_w);
	DECLARE_WRITE8_MEMBER(b2m_8255_portc_w);
	DECLARE_READ8_MEMBER(b2m_8255_portb_r);
	DECLARE_WRITE8_MEMBER(b2m_ext_8255_portc_w);
	DECLARE_READ8_MEMBER(b2m_romdisk_porta_r);
	DECLARE_WRITE8_MEMBER(b2m_romdisk_portb_w);
	DECLARE_WRITE8_MEMBER(b2m_romdisk_portc_w);
	DECLARE_WRITE_LINE_MEMBER(b2m_fdc_drq);
	DECLARE_FLOPPY_FORMATS( b2m_floppy_formats );

	void b2mrom(machine_config &config);
	void b2m(machine_config &config);
	void b2m_io(address_map &map);
	void b2m_mem(address_map &map);
	void b2m_rom_io(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void b2m_postload();
	void b2m_set_bank(int bank);

	uint8_t m_b2m_8255_porta;
	uint8_t m_b2m_video_scroll;
	uint8_t m_b2m_8255_portc;

	uint8_t m_b2m_video_page;
	uint8_t m_b2m_drive;
	uint8_t m_b2m_side;

	uint8_t m_b2m_romdisk_lsb;
	uint8_t m_b2m_romdisk_msb;

	uint8_t m_b2m_color[4];
	uint8_t m_b2m_localmachine;
	uint8_t m_vblank_state;
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

#endif // MAME_INCLUDES_B2M_H

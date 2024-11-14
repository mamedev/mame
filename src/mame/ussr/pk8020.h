// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, AJR
/*****************************************************************************
 *
 * includes/pk8020.h
 *
 ****************************************************************************/
#ifndef MAME_USSR_PK8020_H
#define MAME_USSR_PK8020_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/i8251.h"
#include "machine/pic8259.h"
#include "machine/pla.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "emupal.h"


class pk8020_state : public driver_device
{
public:
	pk8020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_decplm(*this, "decplm")
		, m_devbank(*this, "devbank")
		, m_ram(*this, RAM_TAG)
		, m_ios(*this, "ios%u", 1U)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_cass(*this, "cassette")
		, m_inr(*this, "inr")
		, m_speaker(*this, "speaker")
		, m_printer(*this, "printer")
		, m_region_maincpu(*this, "maincpu")
		, m_region_gfx1(*this, "gfx1")
		, m_io_port(*this, "LINE%u", 0U)
		, m_palette(*this, "palette")
	{ }

	void pk8020(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);

	uint8_t keyboard_r(offs_t offset);
	void sysreg_w(offs_t offset, uint8_t data);
	void color_w(uint8_t data);
	void palette_w(uint8_t data);
	void video_page_w(uint8_t data);
	uint8_t text_r(offs_t offset);
	void text_w(offs_t offset, uint8_t data);
	uint8_t gzu_r(offs_t offset);
	void gzu_w(offs_t offset, uint8_t data);
	uint8_t devices_r(offs_t offset);
	void devices_w(offs_t offset, uint8_t data);
	uint8_t memory_r(offs_t offset);
	void memory_w(offs_t offset, uint8_t data);
	void pk8020_palette(palette_device &palette) const;
	uint32_t screen_update_pk8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(pk8020_interrupt);
	uint8_t ppi_porta_r();
	void floppy_control_w(uint8_t data);
	void ppi_2_portc_w(uint8_t data);
	void pit_out0(int state);

	static const char *plm_select_name(uint8_t data);
	void log_bank_select(uint8_t bank, offs_t start, offs_t end, uint8_t rdecplm, uint8_t wdecplm);

	void pk8020_io(address_map &map) ATTR_COLD;
	void pk8020_mem(address_map &map) ATTR_COLD;
	void devices_map(address_map &map) ATTR_COLD;

	uint8_t m_bank_select = 0;
	uint8_t m_color = 0;
	uint8_t m_video_page = 0;
	uint8_t m_wide = 0;
	uint8_t m_font = 0;
	uint8_t m_attr = 0;
	uint8_t m_text_attr = 0;
	uint8_t m_takt = 0;
	uint8_t m_video_page_access = 0;
	uint8_t m_sound_gate = 0;
	uint8_t m_sound_level = 0;

	required_device<cpu_device> m_maincpu;
	required_device<pls100_device> m_decplm;
	required_device<address_map_bank_device> m_devbank;
	required_device<ram_device> m_ram;
	required_device_array<i8251_device, 2> m_ios;
	required_device<kr1818vg93_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<cassette_image_device> m_cass;
	required_device<pic8259_device> m_inr;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_printer;
	required_memory_region m_region_maincpu;
	required_region_ptr<uint8_t> m_region_gfx1;
	required_ioport_array<16> m_io_port;
	required_device<palette_device> m_palette;
};

#endif // MAME_USSR_PK8020_H

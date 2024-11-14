// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/orion.h
 *
 ****************************************************************************/

#ifndef MAME_RADIO_ORION_H
#define MAME_RADIO_ORION_H

#pragma once

#include "radio86.h"

#include "cpu/i8085/i8085.h"
#include "cpu/z80/z80.h"

#include "imagedev/floppy.h"

#include "machine/i8255.h"
#include "machine/mc146818.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

#include "sound/ay8910.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"

class orion_state : public radio86_state
{
public:
	orion_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag)
		, m_fdc(*this, "fd1793")
		, m_ram(*this, RAM_TAG)
		, m_fd0(*this, "fd0")
		, m_fd1(*this, "fd1")
		, m_fd2(*this, "fd2")
		, m_fd3(*this, "fd3")
		, m_rtc(*this, "rtc")
		, m_speaker(*this, "speaker")
		, m_ay8912(*this, "ay8912")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_bank5(*this, "bank5")
		, m_bank6(*this, "bank6")
		, m_bank7(*this, "bank7")
		, m_bank8(*this, "bank8")
		, m_screen(*this, "screen")
	{ }

	void orion128ms(machine_config &config);
	void orion128(machine_config &config);

protected:
	uint8_t orion128_system_r(offs_t offset);
	void orion128_system_w(offs_t offset, uint8_t data);
	uint8_t orion128_romdisk_r(offs_t offset);
	void orion128_romdisk_w(offs_t offset, uint8_t data);
	void orion128_video_mode_w(uint8_t data);
	void orion128_video_page_w(uint8_t data);
	void orion128_memory_page_w(uint8_t data);
	void orion_disk_control_w(uint8_t data);
	uint8_t orion128_floppy_r(offs_t offset);
	void orion128_floppy_w(offs_t offset, uint8_t data);
	uint8_t orionz80_floppy_rtc_r(offs_t offset);
	void orionz80_floppy_rtc_w(offs_t offset, uint8_t data);
	void orionz80_sound_w(uint8_t data);
	void orionz80_sound_fe_w(uint8_t data);
	void orionz80_memory_page_w(uint8_t data);
	void orionz80_dispatcher_w(uint8_t data);
	uint8_t orionz80_io_r(offs_t offset);
	void orionz80_io_w(offs_t offset, uint8_t data);
	void orionpro_memory_page_w(uint8_t data);
	uint8_t orionpro_io_r(offs_t offset);
	void orionpro_io_w(offs_t offset, uint8_t data);
	DECLARE_MACHINE_START(orion128);
	void orion128_palette(palette_device &palette) const;
	uint32_t screen_update_orion128(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(orionz80_interrupt);
	uint8_t orion_romdisk_porta_r();
	void orion_romdisk_portb_w(uint8_t data);
	void orion_romdisk_portc_w(uint8_t data);
	static void orion_floppy_formats(format_registration &fr);
	void machine_start() override ATTR_COLD;

	uint8_t m_orion128_video_mode = 0;
	uint8_t m_orion128_video_page = 0;
	uint8_t m_orion128_video_width = 0;
	uint8_t m_video_mode_mask = 0;
	uint8_t m_orionpro_pseudo_color = 0;
	uint8_t m_romdisk_lsb = 0;
	uint8_t m_romdisk_msb = 0;
	uint8_t m_orion128_memory_page = 0;
	uint8_t m_orionz80_memory_page = 0;
	uint8_t m_orionz80_dispatcher = 0;
	uint8_t m_speaker_data = 0;
	uint8_t m_orionpro_ram0_segment = 0;
	uint8_t m_orionpro_ram1_segment = 0;
	uint8_t m_orionpro_ram2_segment = 0;
	uint8_t m_orionpro_page = 0;
	uint8_t m_orionpro_128_page = 0;
	uint8_t m_orionpro_rom2_segment = 0;
	uint8_t m_orionpro_dispatcher = 0;

	required_device<fd1793_device> m_fdc;

	required_device<ram_device> m_ram;
	required_device<floppy_connector> m_fd0;
	required_device<floppy_connector> m_fd1;
	required_device<floppy_connector> m_fd2;
	required_device<floppy_connector> m_fd3;
	optional_device<mc146818_device> m_rtc;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<ay8910_device> m_ay8912;
	required_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	optional_memory_bank m_bank5;
	optional_memory_bank m_bank6;
	optional_memory_bank m_bank7;
	optional_memory_bank m_bank8;
	required_device<screen_device> m_screen;

	void orionz80_switch_bank();
	void orion_set_video_mode(int width);
	void orionpro_bank_switch();

private:
	void machine_reset() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};

class orion_z80_state : public orion_state
{
public:
	orion_z80_state(const machine_config &mconfig, device_type type, const char *tag)
		: orion_state(mconfig, type, tag)
	{ }

	void orionz80(machine_config &config);
	void orionz80ms(machine_config &config);

private:
	void machine_reset() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};

class orion_pro_state : public orion_state
{
public:
	orion_pro_state(const machine_config &mconfig, device_type type, const char *tag)
		: orion_state(mconfig, type, tag)
	{ }

	void orionpro(machine_config &config);

private:
	void machine_reset() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
};

#endif // MAME_RADIO_ORION_H

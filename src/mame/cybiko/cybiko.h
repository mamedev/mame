// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*****************************************************************************
 *
 * includes/cybiko.h
 *
 * Cybiko Wireless Inter-tainment System
 *
 * (c) 2001-2007 Tim Schuerewegen
 *
 * Cybiko Classic (V1)
 * Cybiko Classic (V2)
 * Cybiko Xtreme
 *
 ****************************************************************************/

#ifndef MAME_CYBIKO_CYBIKO_H
#define MAME_CYBIKO_CYBIKO_H

#include "bus/rs232/rs232.h"

#include "cpu/h8/h8s2245.h"
#include "cpu/h8/h8s2329.h"

#include "imagedev/snapquik.h"

#include "machine/at45dbxx.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "machine/pcf8593.h"
#include "machine/ram.h"

#include "sound/spkrdev.h"

#include "video/hd66421.h"


class cybiko_state : public driver_device
{
public:
	cybiko_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "hd66421")
		, m_speaker(*this, "speaker")
		, m_rtc(*this, "rtc")
		, m_ram(*this, RAM_TAG)
		, m_flash1(*this, "flash1")
		, m_nvram(*this, "nvram")
		, m_input(*this, "A.%u", 0)
		, m_debug_serial(*this, "debug_serial")
	{ }

	void init_cybikoxt();
	void init_cybiko();
	void cybikov1_base(machine_config &config);
	void cybikov1_flash(machine_config &config);
	void cybikov1_debug_serial(machine_config &config);
	void cybikov1(machine_config &config);
	void cybikov2(machine_config &config);
	void cybikoxt(machine_config &config);

private:
	void serflash_w(uint8_t data);
	uint8_t clock_r();
	void clock_w(uint8_t data);
	uint8_t xtclock_r();
	void xtclock_w(uint8_t data);
	uint8_t xtpower_r();
	uint16_t adc1_r();
	uint16_t adc2_r();
	uint8_t port0_r();

	uint16_t cybiko_lcd_r(offs_t offset, uint16_t mem_mask = ~0);
	void cybiko_lcd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cybikov1_key_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t cybikov2_key_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t cybikoxt_key_r(offs_t offset, uint16_t mem_mask = ~0);
	void cybiko_usb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	int cybiko_key_r(offs_t offset, int mem_mask);

	required_device<h8_device> m_maincpu;
	required_device<hd66421_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<pcf8593_device> m_rtc;
	required_device<ram_device> m_ram;
	optional_device<at45db041_device> m_flash1;
	required_device<nvram_device>   m_nvram;
	optional_ioport_array<15> m_input;
	required_device<rs232_port_device> m_debug_serial;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cybiko);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cybikoxt);

	void cybikov1_mem(address_map &map) ATTR_COLD;
	void cybikov2_mem(address_map &map) ATTR_COLD;
	void cybikoxt_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_CYBIKO_CYBIKO_H

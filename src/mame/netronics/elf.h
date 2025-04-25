// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_NETRONICS_ELF_H
#define MAME_NETRONICS_ELF_H

#pragma once


#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/mm74c922.h"
#include "video/cdp1861.h"
#include "video/dm9368.h"
#include "machine/rescap.h"
#include "machine/ram.h"

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "a6"
#define CDP1861_TAG     "a14"
#define MM74C923_TAG    "a10"
#define DM9368_L_TAG    "a12"
#define DM9368_H_TAG    "a8"

class elf2_state : public driver_device
{
public:
	elf2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_vdc(*this, CDP1861_TAG)
		, m_kb(*this, MM74C923_TAG)
		, m_led_l(*this, DM9368_L_TAG)
		, m_led_h(*this, DM9368_H_TAG)
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_special(*this, "SPECIAL")
	{ }

	void elf2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(load_w);
	DECLARE_INPUT_CHANGED_MEMBER(input_w);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t dispon_r();
	uint8_t data_r();
	void data_w(uint8_t data);
	void memory_w(offs_t offset, uint8_t data);
	void sc_w(uint8_t data);
	void da_w(int state);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void elf2_io(address_map &map) ATTR_COLD;
	void elf2_mem(address_map &map) ATTR_COLD;

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1861_device> m_vdc;
	required_device<mm74c922_device> m_kb;
	required_device<dm9368_device> m_led_l;
	required_device<dm9368_device> m_led_h;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_ioport m_special;

	// internal state
	uint8_t m_data = 0;
	uint8_t m_sc = 0;
	uint8_t m_dmain = 0;
};

#endif // MAME_NETRONICS_ELF_H

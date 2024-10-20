// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/mikro80.h
 *
 ****************************************************************************/

#ifndef MAME_USSR_MIKRO80_H
#define MAME_USSR_MIKRO80_H

#pragma once

#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "sound/dac.h"

class mikro80_state : public driver_device
{
public:
	mikro80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_aram(*this, "attrram")
		, m_vram(*this, "videoram")
		, m_ppi(*this, "ppi8255")
		, m_cassette(*this, "cassette")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_dac(*this, "dac")
		, m_maincpu(*this, "maincpu")
	{ }

	void kristall(machine_config &config);
	void radio99(machine_config &config);
	void mikro80(machine_config &config);

	void init_radio99();
	void init_mikro80();

private:
	u8 m_keyboard_mask = 0;
	u8 m_key_mask = 0;
	void sound_w(u8 data);
	u8 portb_r();
	u8 portc_r();
	u8 kristall2_portc_r();
	void porta_w(u8 data);
	void portc_w(u8 data);
	void tape_w(u8 data);
	u8 tape_r();
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	u32 screen_update_mikro80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void kristall_io(address_map &map) ATTR_COLD;
	void mikro80_io(address_map &map) ATTR_COLD;
	void mikro80_mem(address_map &map) ATTR_COLD;
	void radio99_io(address_map &map) ATTR_COLD;

	memory_passthrough_handler m_rom_shadow_tap;
	required_shared_ptr<uint8_t> m_aram;
	required_shared_ptr<uint8_t> m_vram;
	required_device<i8255_device> m_ppi;
	required_device<cassette_image_device> m_cassette;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_ioport_array<9> m_io_keyboard;
	optional_device<dac_bit_interface> m_dac;
	required_device<cpu_device> m_maincpu;
};

#endif // MAME_USSR_MIKRO80_H

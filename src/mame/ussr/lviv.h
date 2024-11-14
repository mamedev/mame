// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * includes/lviv.h
 *
 ****************************************************************************/
#ifndef MAME_USSR_LVIV_H
#define MAME_USSR_LVIV_H

#pragma once

#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"

class lviv_state : public driver_device
{
public:
	lviv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_ppi(*this, "ppi8255_%u", 0U)
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_maincpu_region(*this, "maincpu")
		, m_bank(*this, "bank%u", 1U)
		, m_key(*this, "KEY%u", 0U)
		, m_joy_port(*this, "JOY")
	{ }

	void lviv(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

private:
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	void lviv_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	uint8_t ppi0_porta_r();
	uint8_t ppi0_portb_r();
	uint8_t ppi0_portc_r();
	uint8_t ppi1_porta_r();
	uint8_t ppi1_portb_r();
	uint8_t ppi1_portc_r();

	void ppi0_porta_w(uint8_t data);
	void ppi0_portb_w(uint8_t data);
	void ppi0_portc_w(uint8_t data);
	void ppi1_porta_w(uint8_t data);
	void ppi1_portb_w(uint8_t data);
	void ppi1_portc_w(uint8_t data);

	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	void update_palette(uint8_t pal);

	void update_memory();
	void setup_snapshot(uint8_t *data);
	void dump_registers();
	std::pair<std::error_condition, std::string> verify_snapshot(const uint8_t * data, uint32_t size);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_region m_maincpu_region;
	required_memory_bank_array<4> m_bank;
	required_ioport_array<12> m_key;
	required_ioport m_joy_port;

	uint8_t* m_vram = nullptr;
	uint16_t m_colortable[1][4]{};
	uint8_t m_ppi_port_outputs[2][3]{};
	uint8_t m_startup_mem_map = 0U;

	/*----------- defined in video/lviv.cpp -----------*/
	static const rgb_t s_palette[8];
};

#endif // MAME_USSR_LVIV_H

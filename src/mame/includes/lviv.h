// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * includes/lviv.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_LVIV_H
#define MAME_INCLUDES_LVIV_H

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
	virtual void machine_reset() override;

	void lviv_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);

	DECLARE_READ8_MEMBER(ppi_0_porta_r);
	DECLARE_READ8_MEMBER(ppi_0_portb_r);
	DECLARE_READ8_MEMBER(ppi_0_portc_r);
	DECLARE_READ8_MEMBER(ppi_1_porta_r);
	DECLARE_READ8_MEMBER(ppi_1_portb_r);
	DECLARE_READ8_MEMBER(ppi_1_portc_r);

	DECLARE_WRITE8_MEMBER(ppi_0_porta_w);
	DECLARE_WRITE8_MEMBER(ppi_0_portb_w);
	DECLARE_WRITE8_MEMBER(ppi_0_portc_w);
	DECLARE_WRITE8_MEMBER(ppi_1_porta_w);
	DECLARE_WRITE8_MEMBER(ppi_1_portb_w);
	DECLARE_WRITE8_MEMBER(ppi_1_portc_w);

	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	void update_palette(uint8_t pal);

	void update_memory();
	void setup_snapshot(uint8_t * data);
	void dump_registers();
	image_verify_result verify_snapshot(uint8_t * data, uint32_t size);

	void io_map(address_map &map);
	void mem_map(address_map &map);

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

	uint8_t* m_video_ram;
	uint16_t m_colortable[1][4];
	uint8_t m_ppi_port_outputs[2][3];
	uint8_t m_startup_mem_map;

	/*----------- defined in video/lviv.c -----------*/
	static const rgb_t s_palette[8];
};

#endif // MAME_INCLUDES_LVIV_H

// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Krzysztof Strzecha, Robbbert
/*****************************************************************************
 *
 * ZX-80/ZX-81 and derivatives
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_ZX_H
#define MAME_INCLUDES_ZX_H

#pragma once

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"

#include "formats/tzx_cas.h"
#include "formats/zx81_p.h"


class zx_state : public driver_device
{
public:
	zx_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_cassette(*this, "cassette"),
		m_softlist(*this, "cass_list"),
		m_speaker(*this, "speaker"),
		m_region_maincpu(*this, "maincpu"),
		m_region_gfx1(*this, "gfx1"),
		m_io_row0(*this, "ROW0"),
		m_io_row1(*this, "ROW1"),
		m_io_row2(*this, "ROW2"),
		m_io_row3(*this, "ROW3"),
		m_io_row4(*this, "ROW4"),
		m_io_row5(*this, "ROW5"),
		m_io_row6(*this, "ROW6"),
		m_io_row7(*this, "ROW7"),
		m_io_config(*this, "CONFIG"),
		m_screen(*this, "screen")
	{ }

	void zx81(machine_config &config);
	void zx81_spk(machine_config &config);
	void ts1000(machine_config &config);
	void pc8300(machine_config &config);
	void pow3000(machine_config &config);
	void ts1500(machine_config &config);
	void zx80(machine_config &config);

	void init_zx();

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(ula_high_r);
	DECLARE_READ8_MEMBER(ula_low_r);
	DECLARE_WRITE8_MEMBER(refresh_w);
	DECLARE_READ8_MEMBER(zx80_io_r);
	DECLARE_READ8_MEMBER(zx81_io_r);
	DECLARE_READ8_MEMBER(pc8300_io_r);
	DECLARE_READ8_MEMBER(pow3000_io_r);
	DECLARE_WRITE8_MEMBER(zx80_io_w);
	DECLARE_WRITE8_MEMBER(zx81_io_w);

	virtual void machine_reset() override;
	virtual void video_start() override;
	void zx_tape_input();
	void zx_ula_hsync();

	void pc8300_io_map(address_map &map);
	void pow3000_io_map(address_map &map);
	void ula_map(address_map &map);
	void zx80_io_map(address_map &map);
	void zx80_map(address_map &map);
	void zx81_io_map(address_map &map);
	void zx81_map(address_map &map);

	enum
	{
		TIMER_TAPE_INPUT,
		TIMER_ULA_HSYNC
	};

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<software_list_device> m_softlist;
	optional_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_gfx1;
	required_ioport m_io_row0;
	required_ioport m_io_row1;
	required_ioport m_io_row2;
	required_ioport m_io_row3;
	required_ioport m_io_row4;
	required_ioport m_io_row5;
	required_ioport m_io_row6;
	required_ioport m_io_row7;
	optional_ioport m_io_config;
	required_device<screen_device> m_screen;

	address_space *m_program;
	emu_timer *m_tape_input, *m_ula_hsync;

	bool m_vsync_active, m_hsync_active, m_nmi_on, m_nmi_generator_active;
	uint64_t m_base_vsync_clock, m_vsync_start_time;
	uint32_t m_ypos;

	uint8_t m_prev_refresh;
	uint8_t m_speaker_state;

	std::unique_ptr<bitmap_ind16> m_bitmap_render;
	std::unique_ptr<bitmap_ind16> m_bitmap_buffer;

	uint16_t m_ula_char_buffer;
	double m_cassette_cur_level;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void drop_sync();
	void recalc_hsync();
};

#endif // MAME_INCLUDES_ZX_H

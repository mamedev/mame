// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Krzysztof Strzecha, Robbbert
/*****************************************************************************
 *
 * ZX-80/ZX-81 and derivatives
 *
 ****************************************************************************/

#ifndef MAME_SINCLAIR_ZX_H
#define MAME_SINCLAIR_ZX_H

#pragma once

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

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
		m_io_row(*this, "ROW%u", 0U),
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

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t ula_high_r(offs_t offset);
	uint8_t ula_low_r(offs_t offset);
	void refresh_w(offs_t offset, uint8_t data);
	uint8_t zx80_io_r(offs_t offset);
	uint8_t zx81_io_r(offs_t offset);
	uint8_t pc8300_io_r(offs_t offset);
	uint8_t pow3000_io_r(offs_t offset);
	void zx80_io_w(offs_t offset, uint8_t data);
	void zx81_io_w(offs_t offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(zx_tape_input);
	TIMER_CALLBACK_MEMBER(zx_ula_hsync);

	void pc8300_io_map(address_map &map) ATTR_COLD;
	void pow3000_io_map(address_map &map) ATTR_COLD;
	void ula_map(address_map &map) ATTR_COLD;
	void zx80_io_map(address_map &map) ATTR_COLD;
	void zx80_map(address_map &map) ATTR_COLD;
	void zx81_io_map(address_map &map) ATTR_COLD;
	void zx81_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<cassette_image_device> m_cassette;
	required_device<software_list_device> m_softlist;
	optional_device<speaker_sound_device> m_speaker;
	required_memory_region m_region_maincpu;
	optional_memory_region m_region_gfx1;
	required_ioport_array<8> m_io_row;
	optional_ioport m_io_config;
	required_device<screen_device> m_screen;

	address_space *m_program = nullptr;
	emu_timer *m_tape_input = nullptr;
	emu_timer *m_ula_hsync = nullptr;

	bool m_vsync_active = false, m_hsync_active = false;
	bool m_nmi_on = false, m_nmi_generator_active = false;
	uint64_t m_base_vsync_clock = 0, m_vsync_start_time = 0;
	uint32_t m_ypos = 0;

	uint8_t m_prev_refresh = 0;
	uint8_t m_speaker_state = 0;

	std::unique_ptr<bitmap_ind16> m_bitmap_render;
	std::unique_ptr<bitmap_ind16> m_bitmap_buffer;

	uint16_t m_ula_char_buffer = 0;
	double m_cassette_cur_level = 0;

	void drop_sync();
	void recalc_hsync();
};

#endif // MAME_SINCLAIR_ZX_H

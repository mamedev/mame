// license:BSD-3-Clause
// copyright-holders:Robbbert
#ifndef MAME_AUSNZ_SUPER80_H
#define MAME_AUSNZ_SUPER80_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/buffer.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "sound/samples.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


/* Bits in m_portf0 variable:
    d5 cassette LED
    d4 super80v rom or pcg bankswitch (1=pcg ram, 0=char gen rom)
    d2 super80v video or colour bankswitch (1=video ram, 0=colour ram)
    d2 super80 screen off (=2mhz) or on (bursts of 2mhz at 50hz = 1mhz) */

class super80_state : public driver_device
{
public:
	super80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_p_chargen(*this, "chargen")
		, m_pio(*this, "z80pio")
		, m_cassette(*this, "cassette")
		, m_samples(*this, "samples")
		, m_speaker(*this, "speaker")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_io_dsw(*this, "DSW")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "KEY.%u", 0)
		, m_cass_led(*this, "cass_led")
	{ }

	void super80m(machine_config &config);
	void super80(machine_config &config);
	void super80e(machine_config &config);
	void super80d(machine_config &config);

protected:
	void machine_reset_common();
	void machine_start_common();
	void cassette_motor(bool data);
	void screen_vblank_super80m(bool state);
	void portf0_w(u8 data);
	void portdc_w(u8 data);
	void pio_port_a_w(u8 data);
	u8 pio_port_b_r();
	u8 portf2_r();
	u8 m_portf0 = 0U;
	u8 m_s_options = 0U;
	u8 m_palette_index = 0U;
	u8 m_keylatch = 0U;
	u8 m_cass_data[4]{};
	u8 m_key_pressed = 0U;
	u8 m_last_data = 0U;
	bool m_boot_in_progress = false;
	void super80m_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	memory_passthrough_handler m_rom_shadow_tap;
	required_shared_ptr<u8> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cassette;
	required_device<samples_device> m_samples;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_ioport m_io_dsw;
	required_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
	output_finder<> m_cass_led;

private:
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	void portf1_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_h);

	uint32_t screen_update_super80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80e(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void super80_io(address_map &map) ATTR_COLD;
	void super80_map(address_map &map) ATTR_COLD;
	void super80e_io(address_map &map) ATTR_COLD;
	void super80m_map(address_map &map) ATTR_COLD;

	u8 m_int_sw = 0;
	u16 m_vidpg = 0;
	bool m_current_charset = false;
};


class super80v_state : public super80_state
{
public:
	super80v_state(const machine_config &mconfig, device_type type, const char *tag)
		: super80_state(mconfig, type, tag)
		, m_crtc(*this, "crtc")
		, m_dma(*this, "dma")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
	{ }

	void super80v(machine_config &config);

protected:
	void super80v_map(address_map &map) ATTR_COLD;
	void super80v_io(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	void port3f_w(u8 data);
	u8 port3e_r();
	std::unique_ptr<u8[]> m_vram;
	uint8_t memory_read_byte(offs_t offset);
	void memory_write_byte(offs_t offset, uint8_t data);
	uint8_t io_read_byte(offs_t offset);
	void io_write_byte(offs_t offset, uint8_t data);
	MC6845_UPDATE_ROW(crtc_update_row);
	uint32_t screen_update_super80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<mc6845_device> m_crtc;
	required_device<z80dma_device> m_dma;
	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;

private:
	void low_w(offs_t offset, u8 data);
	void high_w(offs_t offset, u8 data);
	u8 low_r(offs_t offset);
	u8 high_r(offs_t offset);
};

class super80r_state : public super80v_state
{
public:
	using super80v_state::super80v_state;

	void super80r(machine_config &config);

private:
	void super80r_map(address_map &map) ATTR_COLD;
	void low_w(offs_t offset, u8 data);
	void high_w(offs_t offset, u8 data);
	u8 low_r(offs_t offset);
	u8 high_r(offs_t offset);
};


#endif // MAME_AUSNZ_SUPER80_H

// license:BSD-3-Clause
// copyright-holders:Robbbert
#ifndef MAME_INCLUDES_SUPER80_H
#define MAME_INCLUDES_SUPER80_H

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
#include "sound/wave.h"
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
		, m_p_ram(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_colorram(*this, "colorram")
		, m_p_videoram(*this, "videoram")
		, m_pio(*this, "z80pio")
		, m_cassette(*this, "cassette")
		, m_wave(*this, "wave")
		, m_samples(*this, "samples")
		, m_speaker(*this, "speaker")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_io_dsw(*this, "DSW")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "KEY.%u", 0)
		, m_crtc(*this, "crtc")
		, m_dma(*this, "dma")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_cass_led(*this, "cass_led")
	{ }

	void super80m(machine_config &config);
	void super80(machine_config &config);
	void super80r(machine_config &config);
	void super80e(machine_config &config);
	void super80d(machine_config &config);
	void super80v(machine_config &config);

	void init_super80();

private:
	void machine_start() override;

	DECLARE_READ8_MEMBER(super80v_low_r);
	DECLARE_READ8_MEMBER(super80v_high_r);
	DECLARE_WRITE8_MEMBER(super80v_low_w);
	DECLARE_WRITE8_MEMBER(super80v_high_w);
	DECLARE_WRITE8_MEMBER(super80v_10_w);
	DECLARE_WRITE8_MEMBER(super80v_11_w);
	DECLARE_READ8_MEMBER(port3e_r);
	DECLARE_WRITE8_MEMBER(port3f_w);
	DECLARE_WRITE8_MEMBER(super80_f1_w);
	DECLARE_READ8_MEMBER(super80_f2_r);
	DECLARE_WRITE8_MEMBER(super80_dc_w);
	DECLARE_WRITE8_MEMBER(super80_f0_w);
	DECLARE_WRITE8_MEMBER(super80r_f0_w);
	DECLARE_READ8_MEMBER(super80_read_ff);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_WRITE8_MEMBER(pio_port_a_w);
	DECLARE_READ8_MEMBER(pio_port_b_r);
	DECLARE_MACHINE_RESET(super80);
	DECLARE_MACHINE_RESET(super80r);
	DECLARE_VIDEO_START(super80);
	void super80m_palette(palette_device &palette) const;
	DECLARE_QUICKLOAD_LOAD_MEMBER(super80);
	MC6845_UPDATE_ROW(crtc_update_row);
	uint32_t screen_update_super80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80e(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_super80m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_super80m);
	TIMER_CALLBACK_MEMBER(super80_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_h);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);

	void super80_io(address_map &map);
	void super80_map(address_map &map);
	void super80e_io(address_map &map);
	void super80m_map(address_map &map);
	void super80r_io(address_map &map);
	void super80v_io(address_map &map);
	void super80v_map(address_map &map);

	uint8_t m_s_options;
	uint8_t m_portf0;
	uint8_t m_mc6845_cursor[16];
	uint8_t m_palette_index;
	uint8_t m_keylatch;
	uint8_t m_cass_data[4];
	uint8_t m_int_sw;
	uint8_t m_last_data;
	uint8_t m_key_pressed;
	uint16_t m_vidpg;
	uint8_t m_current_charset;
	uint8_t m_mc6845_reg[32];
	uint8_t m_mc6845_ind;
	void mc6845_cursor_configure();
	void super80_cassette_motor(bool data);
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_ram;
	optional_region_ptr<u8> m_p_chargen;
	optional_region_ptr<u8> m_p_colorram;
	optional_region_ptr<u8> m_p_videoram;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cassette;
	required_device<wave_device> m_wave;
	required_device<samples_device> m_samples;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_ioport m_io_dsw;
	required_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
	optional_device<mc6845_device> m_crtc;
	optional_device<z80dma_device> m_dma;
	optional_device<wd2793_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	output_finder<> m_cass_led;
};

#endif // MAME_INCLUDES_SUPER80_H

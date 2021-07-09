// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************
 *
 * includes/mbee.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_MBEE_H
#define MAME_INCLUDES_MBEE_H

#pragma once

#include "bus/centronics/ctronics.h"

#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"

#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"

#include "machine/buffer.h"
#include "machine/mc146818.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"

#include "sound/spkrdev.h"

#include "video/mc6845.h"

#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


class mbee_state : public driver_device
{
public:
	mbee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "z80pio")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_rtc(*this, "rtc")
		, m_pak(*this, "pak")
		, m_telcom(*this, "telcom")
		, m_basic(*this, "basic")
		, m_io_x7(*this, "X.7")
		, m_io_oldkb(*this, "X.%u", 0)
		, m_io_newkb(*this, "Y.%u", 0)
		, m_io_config(*this, "CONFIG")
		, m_screen(*this, "screen")
		, m_bankr(*this, "bankr%d", 0)
		, m_bankw(*this, "bankw%d", 0)
		{ }

	void mbee56(machine_config &config);
	void mbeeppc(machine_config &config);
	void mbee128(machine_config &config);
	void mbee256(machine_config &config);
	void mbee(machine_config &config);
	void mbeett(machine_config &config);
	void mbeeic(machine_config &config);
	void mbeepc(machine_config &config);
	void mbee128p(machine_config &config);

	void init_mbee()     { m_features = 0x00; };
	void init_mbeett()   { m_features = 0x0d; };
	void init_mbeeppc()  { m_features = 0x09; };
	void init_mbeepp()   { m_features = 0x39; };
	void init_mbeeic()   { m_features = 0x01; };
	void init_mbee56()   { m_features = 0x03; };
	void init_mbee128()  { m_features = 0x11; };
	void init_mbee128p() { m_features = 0x19; };
	void init_mbee256()  { m_features = 0x2d; };

private:
	void port04_w(uint8_t data);
	void port06_w(uint8_t data);
	uint8_t port07_r();
	uint8_t port08_r();
	void port08_w(uint8_t data);
	void port0a_w(uint8_t data);
	void port0b_w(uint8_t data);
	uint8_t port18_r();
	uint8_t port1c_r();
	void port1c_w(uint8_t data);
	void port50_w(uint8_t data);
	uint8_t telcom_low_r();
	uint8_t telcom_high_r();
	uint8_t speed_low_r();
	uint8_t speed_high_r();
	void m6545_index_w(uint8_t data);
	void m6545_data_w(uint8_t data);
	uint8_t video_low_r(offs_t offset);
	uint8_t video_high_r(offs_t offset);
	void video_low_w(offs_t offset, uint8_t data);
	void video_high_w(offs_t offset, uint8_t data);
	void pio_port_b_w(uint8_t data);
	uint8_t pio_port_b_r();
	DECLARE_WRITE_LINE_MEMBER(pio_ardy);
	DECLARE_WRITE_LINE_MEMBER(crtc_vs);
	uint8_t fdc_status_r();
	void fdc_motor_w(uint8_t data);
	void standard_palette(palette_device &palette) const;
	void premium_palette(palette_device &palette) const;
	uint32_t screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(newkb_timer);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	WRITE_LINE_MEMBER(rtc_irq_w);
	WRITE_LINE_MEMBER(fdc_intrq_w);
	WRITE_LINE_MEMBER(fdc_drq_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);
	void machine_start() override;
	void machine_reset() override;

	required_device<palette_device> m_palette;
	void mbee128_io(address_map &map);
	void mbee256_io(address_map &map);
	void mbee256_mem(address_map &map);
	void mbee56_io(address_map &map);
	void mbee56_mem(address_map &map);
	void mbee_io(address_map &map);
	void mbee_mem(address_map &map);
	void mbeeic_io(address_map &map);
	void mbeeic_mem(address_map &map);
	void mbeepc_io(address_map &map);
	void mbeepc_mem(address_map &map);
	void mbeeppc_io(address_map &map);
	void mbeeppc_mem(address_map &map);
	void mbeett_io(address_map &map);
	void mbeett_mem(address_map &map);

	u8 m_features = 0;
	u16 m_size = 0;
	u32 m_ramsize = 0;
	bool m_b7_rtc = 0;
	bool m_b7_vs = 0;
	bool m_b2 = 0;
	uint8_t m_framecnt = 0;
	uint8_t m_08 = 0;
	uint8_t m_0a = 0;
	uint8_t m_0b = 0;
	uint8_t m_1c = 0;
	uint8_t m_newkb_was_pressed[15] = { 0, };
	uint8_t m_newkb_q[20] = { 0, };
	uint8_t m_newkb_q_pos = 0;
	uint8_t m_sy6545_reg[32] = { 0, };
	uint8_t m_sy6545_ind = 0;
	uint8_t m_fdc_rq = 0;
	uint8_t m_bank_array[33] = { 0, };
	std::unique_ptr<u8[]> m_dummy; // black hole for writes to rom
	std::unique_ptr<u8[]> m_ram;   // main banked-switch ram, 128/256/pp
	std::unique_ptr<u8[]> m_vram;  // video ram, all models
	std::unique_ptr<u8[]> m_pram;  // pcg ram, all models
	std::unique_ptr<u8[]> m_cram;  // colour ram, all except mbee
	std::unique_ptr<u8[]> m_aram;  // attribute ram, ppc/128/256/pp/tt
	void setup_banks(uint8_t data, bool first_time, uint8_t b_mask);
	void oldkb_scan(uint16_t param);
	void oldkb_matrix_r(uint16_t offs);
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<mc6845_device> m_crtc;
	optional_device<wd2793_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<mc146818_device> m_rtc;
	optional_memory_bank m_pak;
	optional_memory_bank m_telcom;
	optional_memory_bank m_basic;
	optional_ioport m_io_x7;
	optional_ioport_array<8> m_io_oldkb;
	optional_ioport_array<15> m_io_newkb;
	required_ioport m_io_config;
	required_device<screen_device> m_screen;
	optional_memory_bank_array<16> m_bankr;
	optional_memory_bank_array<16> m_bankw;
};

#endif // MAME_INCLUDES_MBEE_H

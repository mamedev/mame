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

#include "machine/8530scc.h"
#include "machine/buffer.h"
#include "machine/mc146818.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"

#include "sound/spkrdev.h"

#include "video/mc6845.h"

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

	void init_mbeepc85();
	void init_mbee256();
	void init_mbee56();
	void init_mbeett();
	void init_mbeeppc();
	void init_mbee();
	void init_mbeepc();
	void init_mbeeic();
	void init_mbee128();

private:
	enum
	{
		TIMER_MBEE_NEWKB
	};

	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port0a_w);
	DECLARE_WRITE8_MEMBER(port0b_w);
	DECLARE_READ8_MEMBER(port18_r);
	DECLARE_READ8_MEMBER(port1c_r);
	DECLARE_WRITE8_MEMBER(port1c_w);
	DECLARE_WRITE8_MEMBER(mbee128_50_w);
	DECLARE_WRITE8_MEMBER(mbee256_50_w);
	DECLARE_READ8_MEMBER(telcom_low_r);
	DECLARE_READ8_MEMBER(telcom_high_r);
	DECLARE_READ8_MEMBER(speed_low_r);
	DECLARE_READ8_MEMBER(speed_high_r);
	DECLARE_WRITE8_MEMBER(m6545_index_w);
	DECLARE_WRITE8_MEMBER(m6545_data_w);
	DECLARE_READ8_MEMBER(video_low_r);
	DECLARE_READ8_MEMBER(video_high_r);
	DECLARE_WRITE8_MEMBER(video_low_w);
	DECLARE_WRITE8_MEMBER(video_high_w);
	DECLARE_WRITE8_MEMBER(pio_port_b_w);
	DECLARE_READ8_MEMBER(pio_port_b_r);
	DECLARE_WRITE_LINE_MEMBER(pio_ardy);
	DECLARE_WRITE_LINE_MEMBER(crtc_vs);
	DECLARE_READ8_MEMBER(fdc_status_r);
	DECLARE_WRITE8_MEMBER(fdc_motor_w);
	DECLARE_MACHINE_RESET(mbee);
	DECLARE_VIDEO_START(mono);
	DECLARE_VIDEO_START(standard);
	DECLARE_VIDEO_START(premium);
	void standard_palette(palette_device &palette) const;
	void premium_palette(palette_device &palette) const;
	DECLARE_MACHINE_RESET(mbee56);
	DECLARE_MACHINE_RESET(mbee128);
	DECLARE_MACHINE_RESET(mbee256);
	DECLARE_MACHINE_RESET(mbeett);
	uint32_t screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_newkb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_bee);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_bin);
	WRITE_LINE_MEMBER(rtc_irq_w);
	WRITE_LINE_MEMBER(fdc_intrq_w);
	WRITE_LINE_MEMBER(fdc_drq_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

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

	uint8_t *m_p_videoram;
	uint8_t *m_p_gfxram;
	uint8_t *m_p_colorram;
	uint8_t *m_p_attribram;
	bool m_is_premium;
	bool m_has_oldkb;
	size_t m_size;
	bool m_b7_rtc;
	bool m_b7_vs;
	bool m_b2;
	uint8_t m_framecnt;
	uint8_t m_08;
	uint8_t m_0a;
	uint8_t m_0b;
	uint8_t m_1c;
	uint8_t m_sy6545_cursor[16];
	uint8_t m_mbee256_was_pressed[15];
	uint8_t m_mbee256_q[20];
	uint8_t m_mbee256_q_pos;
	uint8_t m_sy6545_reg[32];
	uint8_t m_sy6545_ind;
	uint8_t m_fdc_rq;
	uint8_t m_bank_array[33];
	void setup_banks(uint8_t data, bool first_time, uint8_t b_mask);
	void sy6545_cursor_configure();
	void oldkb_scan(uint16_t param);
	void oldkb_matrix_r(uint16_t offs);
	void machine_reset_common();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
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
};

#endif // MAME_INCLUDES_MBEE_H

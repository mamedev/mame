// license:BSD-3-Clause
// copyright-holders:Robbbert
/*****************************************************************************
 *
 * includes/mbee.h
 *
 ****************************************************************************/

#ifndef MBEE_H_
#define MBEE_H_

#include "emu.h"
#include "imagedev/snapquik.h"
#include "machine/z80pio.h"
#include "machine/8530scc.h"
#include "imagedev/cassette.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "machine/mc146818.h"
#include "video/mc6845.h"
#include "sound/speaker.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/wave.h"
#include "machine/wd_fdc.h"


class mbee_state : public driver_device
{
public:
	enum
	{
		TIMER_MBEE_NEWKB
	};

	mbee_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "z80pio")
		, m_cassette(*this, "cassette")
		, m_wave(*this, WAVE_TAG)
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
		, m_io_oldkb(*this, "X")
		, m_io_newkb(*this, "Y")
		, m_io_config(*this, "CONFIG")
		, m_screen(*this, "screen")
	{ }

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
	DECLARE_DRIVER_INIT(mbeepc85);
	DECLARE_DRIVER_INIT(mbee256);
	DECLARE_DRIVER_INIT(mbee56);
	DECLARE_DRIVER_INIT(mbeett);
	DECLARE_DRIVER_INIT(mbeeppc);
	DECLARE_DRIVER_INIT(mbee);
	DECLARE_DRIVER_INIT(mbeepc);
	DECLARE_DRIVER_INIT(mbeeic);
	DECLARE_DRIVER_INIT(mbee128);
	DECLARE_MACHINE_RESET(mbee);
	DECLARE_VIDEO_START(mono);
	DECLARE_VIDEO_START(standard);
	DECLARE_VIDEO_START(premium);
	DECLARE_PALETTE_INIT(standard);
	DECLARE_PALETTE_INIT(premium);
	DECLARE_MACHINE_RESET(mbee56);
	DECLARE_MACHINE_RESET(mbee128);
	DECLARE_MACHINE_RESET(mbee256);
	DECLARE_MACHINE_RESET(mbeett);
	UINT32 screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(timer_newkb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(mbee);
	DECLARE_QUICKLOAD_LOAD_MEMBER(mbee_z80bin);
	WRITE_LINE_MEMBER(rtc_irq_w);
	WRITE_LINE_MEMBER(fdc_intrq_w);
	WRITE_LINE_MEMBER(fdc_drq_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	required_device<palette_device> m_palette;
private:
	UINT8 *m_p_videoram;
	UINT8 *m_p_gfxram;
	UINT8 *m_p_colorram;
	UINT8 *m_p_attribram;
	bool m_is_premium;
	bool m_has_oldkb;
	size_t m_size;
	bool m_b7_rtc;
	bool m_b7_vs;
	bool m_b2;
	UINT8 m_framecnt;
	UINT8 m_08;
	UINT8 m_0a;
	UINT8 m_0b;
	UINT8 m_1c;
	UINT8 m_sy6545_cursor[16];
	UINT8 m_mbee256_was_pressed[15];
	UINT8 m_mbee256_q[20];
	UINT8 m_mbee256_q_pos;
	UINT8 m_sy6545_reg[32];
	UINT8 m_sy6545_ind;
	UINT8 m_fdc_rq;
	UINT8 m_bank_array[33];
	void setup_banks(UINT8 data, bool first_time, UINT8 b_mask);
	void sy6545_cursor_configure();
	void oldkb_scan(UINT16 param);
	void oldkb_matrix_r(UINT16 offs);
	void machine_reset_common();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cassette;
	required_device<wave_device> m_wave;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<mc6845_device> m_crtc;
	optional_device<wd2793_t> m_fdc;
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

#endif /* MBEE_H_ */

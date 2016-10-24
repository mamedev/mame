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

	mbee_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_io_oldkb(*this, "X.%u", 0)
		, m_io_newkb(*this, "Y.%u", 0)
		, m_io_config(*this, "CONFIG")
		, m_screen(*this, "screen")
	{ }

	void port04_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port06_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port07_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port08_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port08_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port0a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void port0b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port18_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port1c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port1c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mbee128_50_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mbee256_50_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t telcom_low_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t telcom_high_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t speed_low_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t speed_high_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m6545_index_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m6545_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t video_low_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t video_high_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void video_low_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_high_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pio_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pio_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void pio_ardy(int state);
	void crtc_vs(int state);
	uint8_t fdc_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdc_motor_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_mbeepc85();
	void init_mbee256();
	void init_mbee56();
	void init_mbeett();
	void init_mbeeppc();
	void init_mbee();
	void init_mbeepc();
	void init_mbeeic();
	void init_mbee128();
	void machine_reset_mbee();
	void video_start_mono();
	void video_start_standard();
	void video_start_premium();
	void palette_init_standard(palette_device &palette);
	void palette_init_premium(palette_device &palette);
	void machine_reset_mbee56();
	void machine_reset_mbee128();
	void machine_reset_mbee256();
	void machine_reset_mbeett();
	uint32_t screen_update_mbee(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void timer_newkb(void *ptr, int32_t param);
	DECLARE_QUICKLOAD_LOAD_MEMBER(mbee);
	DECLARE_QUICKLOAD_LOAD_MEMBER(mbee_z80bin);
	void rtc_irq_w(int state);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	required_device<palette_device> m_palette;
private:
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

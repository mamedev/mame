// license:BSD-3-Clause
// copyright-holders:Robbbert
#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "sound/wave.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"
#include "video/mc6845.h"
#include "machine/z80pio.h"
#include "machine/z80dma.h"
#include "machine/wd_fdc.h"


/* Bits in m_portf0 variable:
    d5 cassette LED
    d4 super80v rom or pcg bankswitch (1=pcg ram, 0=char gen rom)
    d2 super80v video or colour bankswitch (1=video ram, 0=colour ram)
    d2 super80 screen off (=2mhz) or on (bursts of 2mhz at 50hz = 1mhz) */

class super80_state : public driver_device
{
public:
	super80_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "z80pio")
		, m_cassette(*this, "cassette")
		, m_wave(*this, WAVE_TAG)
		, m_speaker(*this, "speaker")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_io_dsw(*this, "DSW")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "KEY")
		, m_crtc(*this, "crtc")
		, m_dma(*this, "dma")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
	{ }

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
	DECLARE_DRIVER_INIT(super80);
	DECLARE_VIDEO_START(super80);
	DECLARE_VIDEO_START(super80v);
	DECLARE_PALETTE_INIT(super80m);
	DECLARE_QUICKLOAD_LOAD_MEMBER(super80);
	MC6845_UPDATE_ROW(crtc_update_row);
	UINT32 screen_update_super80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80e(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_super80m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_super80m(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(super80_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_h);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_k);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_p);
	UINT8 m_s_options;
	UINT8 m_portf0;
	UINT8 *m_p_videoram;
	UINT8 *m_p_colorram;
	UINT8 *m_p_pcgram;
	UINT8 m_mc6845_cursor[16];
	UINT8 m_palette_index;
	required_device<palette_device> m_palette;
private:
	virtual void machine_reset() override;
	UINT8 m_keylatch;
	UINT8 m_cass_data[4];
	UINT8 m_int_sw;
	UINT8 m_last_data;
	UINT8 m_key_pressed;
	UINT16 m_vidpg;
	UINT8 m_current_charset;
	const UINT8 *m_p_chargen;
	UINT8 m_mc6845_reg[32];
	UINT8 m_mc6845_ind;
	UINT8 *m_p_ram;
	void mc6845_cursor_configure();
	void super80_cassette_motor(UINT8 data);
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<cassette_image_device> m_cassette;
	required_device<wave_device> m_wave;
	required_device<speaker_sound_device> m_speaker;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_ioport m_io_dsw;
	required_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
	optional_device<mc6845_device> m_crtc;
	optional_device<z80dma_device> m_dma;
	optional_device<wd2793_t> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};

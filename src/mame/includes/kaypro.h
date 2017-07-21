// license:BSD-3-Clause
// copyright-holders:Robbbert
#ifndef MAME_INCLUDES_KAYPRO_H
#define MAME_INCLUDES_KAYPRO_H

#pragma once

#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/com8116.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/snapquik.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "machine/wd_fdc.h"

struct kay_kbd_t;

class kaypro_state : public driver_device
{
public:
	enum
	{
		TIMER_FLOPPY
	};

	kaypro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_pio_g(*this, "z80pio_g")
		, m_pio_s(*this, "z80pio_s")
		, m_sio(*this, "z80sio")
		, m_sio2x(*this, "z80sio_2x")
		, m_centronics(*this, "centronics")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_crtc(*this, "crtc")
		, m_beep(*this, "beeper")
	{}

	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_READ8_MEMBER(kaypro2x_87_r);
	DECLARE_READ8_MEMBER(kaypro2x_system_port_r);
	DECLARE_READ8_MEMBER(kaypro2x_status_r);
	DECLARE_READ8_MEMBER(kaypro2x_videoram_r);
	DECLARE_WRITE8_MEMBER(kaypro2x_system_port_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_index_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_register_w);
	DECLARE_WRITE8_MEMBER(kaypro2x_videoram_w);
	DECLARE_READ8_MEMBER(pio_system_r);
	DECLARE_WRITE8_MEMBER(kayproii_pio_system_w);
	DECLARE_WRITE8_MEMBER(kaypro4_pio_system_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_READ8_MEMBER(kaypro_videoram_r);
	DECLARE_WRITE8_MEMBER(kaypro_videoram_w);
	DECLARE_MACHINE_START(kayproii);
	DECLARE_MACHINE_RESET(kaypro);
	DECLARE_VIDEO_START(kaypro);
	DECLARE_PALETTE_INIT(kaypro);
	DECLARE_MACHINE_RESET(kay_kbd);
	DECLARE_DRIVER_INIT(kaypro);
	DECLARE_FLOPPY_FORMATS(kayproii_floppy_formats);
	DECLARE_FLOPPY_FORMATS(kaypro2x_floppy_formats);
	uint32_t screen_update_kayproii(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_kaypro2x(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_omni2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kay_kbd_interrupt);
	DECLARE_READ8_MEMBER(kaypro_sio_r);
	DECLARE_WRITE8_MEMBER(kaypro_sio_w);
	MC6845_UPDATE_ROW(kaypro2x_update_row);
	DECLARE_QUICKLOAD_LOAD_MEMBER(kaypro);
	TIMER_CALLBACK_MEMBER( kay_kbd_beepoff );

private:
	uint8_t m_mc6845_cursor[16];
	uint8_t m_mc6845_reg[32];
	uint8_t m_mc6845_ind;
	uint8_t m_framecnt;
	uint8_t *m_p_videoram;
	kay_kbd_t *m_kbd;
	int m_centronics_busy;
	void kay_kbd_in(uint8_t data );
	uint8_t kay_kbd_c_r();
	uint8_t kay_kbd_d_r();
	void kay_kbd_d_w( uint8_t data );
	bool m_is_motor_off;
	uint8_t m_fdc_rq;
	uint8_t m_system_port;
	uint16_t m_mc6845_video_address;
	floppy_image_device *m_floppy;
	void mc6845_cursor_configure();
	void mc6845_screen_configure();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	optional_device<z80pio_device> m_pio_g;
	optional_device<z80pio_device> m_pio_s;
	required_device<z80sio0_device> m_sio;
	optional_device<z80sio0_device> m_sio2x;
	required_device<centronics_device> m_centronics;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	optional_device<mc6845_device> m_crtc;
	required_device<beep_device> m_beep;
};

#endif // MAME_INCLUDES_KAYPRO_H

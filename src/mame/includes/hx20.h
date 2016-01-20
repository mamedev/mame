// license:BSD-3-Clause
// copyright-holders:Curt Coder
#pragma once

#ifndef __HX20__
#define __HX20__

#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6800.h"
#include "imagedev/cassette.h"
#include "bus/epson_sio/epson_sio.h"
#include "machine/mc146818.h"
#include "machine/ram.h"
#include "video/upd7227.h"
#include "sound/speaker.h"
#include "rendlay.h"

#define HD6301V1_MAIN_TAG   "8g"
#define HD6301V1_SLAVE_TAG  "6d"
#define MC146818_TAG        "6g"
#define UPD7227_0_TAG       "lcdc0"
#define UPD7227_1_TAG       "lcdc1"
#define UPD7227_2_TAG       "lcdc2"
#define UPD7227_3_TAG       "lcdc3"
#define UPD7227_4_TAG       "lcdc4"
#define UPD7227_5_TAG       "lcdc5"
#define SPEAKER_TAG         "speaker"
#define CASSETTE_TAG        "cassette"
#define RS232_TAG           "rs232"
#define SCREEN_TAG          "screen"

class hx20_state : public driver_device
{
public:
	hx20_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, HD6301V1_MAIN_TAG),
			m_subcpu(*this, HD6301V1_SLAVE_TAG),
			m_rtc(*this, MC146818_TAG),
			m_lcdc0(*this, UPD7227_0_TAG),
			m_lcdc1(*this, UPD7227_1_TAG),
			m_lcdc2(*this, UPD7227_2_TAG),
			m_lcdc3(*this, UPD7227_3_TAG),
			m_lcdc4(*this, UPD7227_4_TAG),
			m_lcdc5(*this, UPD7227_5_TAG),
			m_speaker(*this, SPEAKER_TAG),
			m_cassette(*this, CASSETTE_TAG),
			m_rs232(*this, RS232_TAG),
			m_sio(*this, "sio"),
			m_ksc0(*this, "KSC0"),
			m_ksc1(*this, "KSC1"),
			m_ksc2(*this, "KSC2"),
			m_ksc3(*this, "KSC3"),
			m_ksc4(*this, "KSC4"),
			m_ksc5(*this, "KSC5"),
			m_ksc6(*this, "KSC6"),
			m_ksc7(*this, "KSC7"),
			m_sw6(*this, "SW6"),
			m_slave_rx(1),
			m_slave_tx(1),
			m_slave_flag(1),
			m_rtc_irq(CLEAR_LINE),
			m_kbrequest(1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<mc146818_device> m_rtc;
	required_device<upd7227_device> m_lcdc0;
	required_device<upd7227_device> m_lcdc1;
	required_device<upd7227_device> m_lcdc2;
	required_device<upd7227_device> m_lcdc3;
	required_device<upd7227_device> m_lcdc4;
	required_device<upd7227_device> m_lcdc5;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<rs232_port_device> m_rs232;
	required_device<epson_sio_device> m_sio;
	required_ioport m_ksc0;
	required_ioport m_ksc1;
	required_ioport m_ksc2;
	required_ioport m_ksc3;
	required_ioport m_ksc4;
	required_ioport m_ksc5;
	required_ioport m_ksc6;
	required_ioport m_ksc7;
	required_ioport m_sw6;

	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(hx20);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER( ksc_w );
	DECLARE_READ8_MEMBER( krtn07_r );
	DECLARE_READ8_MEMBER( krtn89_r );
	DECLARE_WRITE8_MEMBER( lcd_cs_w );
	DECLARE_WRITE8_MEMBER( lcd_data_w );

	DECLARE_READ8_MEMBER( main_p1_r );
	DECLARE_WRITE8_MEMBER( main_p1_w );
	DECLARE_READ8_MEMBER( main_p2_r );
	DECLARE_WRITE8_MEMBER( main_p2_w );

	DECLARE_READ8_MEMBER( slave_p1_r );
	DECLARE_WRITE8_MEMBER( slave_p1_w );
	DECLARE_READ8_MEMBER( slave_p2_r );
	DECLARE_WRITE8_MEMBER( slave_p2_w );
	DECLARE_READ8_MEMBER( slave_p3_r );
	DECLARE_WRITE8_MEMBER( slave_p3_w );
	DECLARE_READ8_MEMBER( slave_p4_r );
	DECLARE_WRITE8_MEMBER( slave_p4_w );

	DECLARE_WRITE_LINE_MEMBER( rtc_irq_w );

	DECLARE_WRITE_LINE_MEMBER( sio_rx_w ) { m_sio_rx = state; }
	DECLARE_WRITE_LINE_MEMBER( sio_pin_w ) { m_sio_pin = state; }

	void update_interrupt();

	// CPU state
	int m_slave_sio;
	int m_slave_rx;
	int m_slave_tx;
	int m_slave_flag;
	int m_rtc_irq;

	// keyboard state
	UINT8 m_ksc;
	int m_kbrequest;

	// video state
	UINT8 m_lcd_data;

	// sio state
	int m_sio_rx;
	int m_sio_pin;
};

#endif

// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_INCLUDES_HX20_H
#define MAME_INCLUDES_HX20_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "imagedev/cassette.h"
#include "machine/mc146818.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "video/upd7227.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/epson_sio/epson_sio.h"
#include "bus/rs232/rs232.h"

#include "emupal.h"

#define HD6301V1_MAIN_TAG   "8g"
#define HD6301V1_SLAVE_TAG  "6d"
#define MC146818_TAG        "6g"
#define CASSETTE_TAG        "cassette"
#define RS232_TAG           "rs232"
#define SCREEN_TAG          "screen"

class hx20_state : public driver_device
{
public:
	hx20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, HD6301V1_MAIN_TAG)
		, m_subcpu(*this, HD6301V1_SLAVE_TAG)
		, m_rtc(*this, MC146818_TAG)
		, m_lcdc(*this, "lcdc%u", 0U)
		, m_speaker(*this, "speaker")
		, m_cassette(*this, CASSETTE_TAG)
		, m_rs232(*this, RS232_TAG)
		, m_sio(*this, "sio")
		, m_optrom(*this, "optrom")
		, m_ksc_io(*this, "KSC%u", 0U)
		, m_sw6(*this, "SW6")
		, m_slave_rx(1)
		, m_slave_tx(1)
		, m_slave_flag(1)
		, m_rtc_irq(CLEAR_LINE)
		, m_kbrequest(1)
	{ }

	void hx20(machine_config &config);
	void cm6032(machine_config &config);
	void cm6127(machine_config &config);

private:
	required_device<hd63701_cpu_device> m_maincpu;
	required_device<hd63701_cpu_device> m_subcpu;
	required_device<mc146818_device> m_rtc;
	required_device_array<upd7227_device, 6> m_lcdc;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<rs232_port_device> m_rs232;
	required_device<epson_sio_device> m_sio;
	optional_device<generic_slot_device> m_optrom;
	required_ioport_array<8> m_ksc_io;
	required_ioport m_sw6;

	virtual void machine_start() override;
	void hx20_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( optrom_load );
	DECLARE_READ8_MEMBER( optrom_r );

	void update_interrupt();

	// CPU state
	int m_slave_sio;
	int m_slave_rx;
	int m_slave_tx;
	int m_slave_flag;
	int m_rtc_irq;

	// keyboard state
	uint8_t m_ksc;
	int m_kbrequest;

	// video state
	uint8_t m_lcd_data;

	// sio state
	int m_sio_rx;
	int m_sio_pin;

	void hx20_mem(address_map &map);
	void hx20_sub_mem(address_map &map);
	void cm6032_mem(address_map &map);
	void cm6127_mem(address_map &map);
};

#endif // MAME_INCLUDES_HX20_H

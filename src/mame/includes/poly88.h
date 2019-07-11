// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/poly88.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_POLY88_H
#define MAME_INCLUDES_POLY88_H

#pragma once

#include "machine/i8251.h"
#include "machine/mm5307.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/timer.h"

class poly88_state : public driver_device
{
public:
	enum
	{
		TIMER_USART,
		TIMER_KEYBOARD
	};

	poly88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video_ram(*this, "video_ram")
		, m_maincpu(*this, "maincpu")
		, m_usart(*this, "usart")
		, m_brg(*this, "brg")
		, m_cassette(*this, "cassette")
		, m_linec(*this, "LINEC")
		, m_line0(*this, "LINE0")
		, m_line1(*this, "LINE1")
		, m_line2(*this, "LINE2")
		, m_line3(*this, "LINE3")
		, m_line4(*this, "LINE4")
		, m_line5(*this, "LINE5")
		, m_line6(*this, "LINE6")
	{ }

	void poly88(machine_config &config);
	void poly8813(machine_config &config);

	void init_poly88();

private:
	required_shared_ptr<uint8_t> m_video_ram;
	uint8_t *m_FNT;
	uint8_t m_last_code;
	uint8_t m_int_vector;
	emu_timer * m_usart_timer;
	emu_timer * m_keyboard_timer;
	void baud_rate_w(uint8_t data);
	uint8_t keyboard_r();
	void intr_w(uint8_t data);
	bool m_dtr, m_rts, m_txd, m_rxd, m_cassold;
	u8 m_cass_data[4];
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_poly88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(poly88_interrupt);
	TIMER_CALLBACK_MEMBER(keyboard_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	DECLARE_WRITE_LINE_MEMBER(cassette_clock_w);
	DECLARE_WRITE_LINE_MEMBER(usart_ready_w);
	IRQ_CALLBACK_MEMBER(poly88_irq_callback);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	void poly8813_io(address_map &map);
	void poly8813_mem(address_map &map);
	void poly88_io(address_map &map);
	void poly88_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart;
	required_device<mm5307_device> m_brg;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_linec;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	uint8_t row_number(uint8_t code);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_POLY88_H

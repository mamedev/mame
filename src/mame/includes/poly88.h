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

class poly88_state : public driver_device
{
public:
	enum
	{
		TIMER_USART,
		TIMER_KEYBOARD
	};

	poly88_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_usart(*this, "usart"),
		m_brg(*this, "brg"),
		m_cassette(*this, "cassette"),
		m_linec(*this, "LINEC"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4"),
		m_line5(*this, "LINE5"),
		m_line6(*this, "LINE6")
	{ }

	void poly88(machine_config &config);
	void poly8813(machine_config &config);

	void init_poly88();

private:
	required_shared_ptr<uint8_t> m_video_ram;
	uint8_t *m_FNT;
	uint8_t m_intr;
	uint8_t m_last_code;
	uint8_t m_int_vector;
	emu_timer * m_usart_timer;
	emu_timer * m_keyboard_timer;
	int m_previous_level;
	int m_clk_level_tape;
	void baud_rate_w(uint8_t data);
	uint8_t keyboard_r();
	void intr_w(uint8_t data);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_poly88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(poly88_interrupt);
	TIMER_CALLBACK_MEMBER(poly88_usart_timer_callback);
	TIMER_CALLBACK_MEMBER(keyboard_callback);
	DECLARE_WRITE_LINE_MEMBER(cassette_txc_rxc_w);
	DECLARE_WRITE_LINE_MEMBER(write_cas_tx);
	DECLARE_WRITE_LINE_MEMBER(poly88_usart_rxready);
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

	int m_cas_tx;
};

#endif // MAME_INCLUDES_POLY88_H

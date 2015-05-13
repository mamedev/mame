// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/poly88.h
 *
 ****************************************************************************/

#ifndef POLY88_H_
#define POLY88_H_

#include "machine/i8251.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"

class poly88_state : public driver_device
{
public:
	enum
	{
		TIMER_USART,
		TIMER_KEYBOARD,
		TIMER_CASSETTE
	};

	poly88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_maincpu(*this, "maincpu"),
		m_uart(*this, "uart"),
		m_cassette(*this, "cassette"),
		m_linec(*this, "LINEC"),
		m_line0(*this, "LINE0"),
		m_line1(*this, "LINE1"),
		m_line2(*this, "LINE2"),
		m_line3(*this, "LINE3"),
		m_line4(*this, "LINE4"),
		m_line5(*this, "LINE5"),
		m_line6(*this, "LINE6") { }

	required_shared_ptr<UINT8> m_video_ram;
	UINT8 *m_FNT;
	UINT8 m_intr;
	UINT8 m_last_code;
	UINT8 m_int_vector;
	emu_timer * m_cassette_timer;
	emu_timer * m_usart_timer;
	int m_previous_level;
	int m_clk_level;
	int m_clk_level_tape;
	DECLARE_WRITE8_MEMBER(poly88_baud_rate_w);
	DECLARE_READ8_MEMBER(poly88_keyboard_r);
	DECLARE_WRITE8_MEMBER(poly88_intr_w);
	DECLARE_DRIVER_INIT(poly88);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_poly88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(poly88_interrupt);
	TIMER_CALLBACK_MEMBER(poly88_usart_timer_callback);
	TIMER_CALLBACK_MEMBER(keyboard_callback);
	TIMER_CALLBACK_MEMBER(poly88_cassette_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(write_cas_tx);
	DECLARE_WRITE_LINE_MEMBER(poly88_usart_rxready);
	IRQ_CALLBACK_MEMBER(poly88_irq_callback);
	DECLARE_SNAPSHOT_LOAD_MEMBER( poly88 );

protected:
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_linec;
	required_ioport m_line0;
	required_ioport m_line1;
	required_ioport m_line2;
	required_ioport m_line3;
	required_ioport m_line4;
	required_ioport m_line5;
	required_ioport m_line6;
	UINT8 row_number(UINT8 code);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	int m_cas_tx;
};

#endif /* POLY88_H_ */

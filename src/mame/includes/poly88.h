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

	required_shared_ptr<uint8_t> m_video_ram;
	uint8_t *m_FNT;
	uint8_t m_intr;
	uint8_t m_last_code;
	uint8_t m_int_vector;
	emu_timer * m_cassette_timer;
	emu_timer * m_usart_timer;
	int m_previous_level;
	int m_clk_level;
	int m_clk_level_tape;
	void poly88_baud_rate_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t poly88_keyboard_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void poly88_intr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_poly88();
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_poly88(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void poly88_interrupt(device_t &device);
	void poly88_usart_timer_callback(void *ptr, int32_t param);
	void keyboard_callback(void *ptr, int32_t param);
	void poly88_cassette_timer_callback(void *ptr, int32_t param);
	void write_cas_tx(int state);
	void poly88_usart_rxready(int state);
	int poly88_irq_callback(device_t &device, int irqline);
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
	uint8_t row_number(uint8_t code);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	int m_cas_tx;
};

#endif /* POLY88_H_ */

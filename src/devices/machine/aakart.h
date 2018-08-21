// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Acorn Archimedes KART interface

***************************************************************************/

#ifndef MAME_MACHINE_AAKART_H
#define MAME_MACHINE_AAKART_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aakart_device

class aakart_device : public device_t
{
public:
	// construction/destruction
	aakart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto out_tx_callback() { return m_out_tx_cb.bind(); }
	auto out_rx_callback() { return m_out_rx_cb.bind(); }

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	void send_keycode_down(uint8_t row, uint8_t col);
	void send_keycode_up(uint8_t row, uint8_t col);
	void send_mouse(uint8_t x, uint8_t y);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum {
		STATUS_NORMAL = 0,
		STATUS_KEYUP,
		STATUS_KEYDOWN,
		STATUS_MOUSE,
		STATUS_HRST,
		STATUS_UNDEFINED
	};

	static const device_timer_id RX_TIMER = 1;
	static const device_timer_id TX_TIMER = 2;
	static const device_timer_id MOUSE_TIMER = 3;
	static const device_timer_id KEYB_TIMER = 4;
	emu_timer *         m_rxtimer;
	emu_timer *         m_txtimer;
	emu_timer *         m_mousetimer;
	emu_timer *         m_keybtimer;

	devcb_write_line        m_out_tx_cb;
	devcb_write_line        m_out_rx_cb;
	uint8_t m_tx_latch;
	//uint8_t m_rx_latch;
	uint8_t m_rx;
	uint8_t m_new_command;
	uint8_t m_status;
	uint8_t m_mouse_enable;
	uint8_t m_keyb_enable;
	int m_queue_size;
	uint16_t m_queue[0x10];
};


// device type definition
DECLARE_DEVICE_TYPE(AAKART, aakart_device)

#endif // MAME_MACHINE_AAKART_H

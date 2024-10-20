// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
#ifndef MAME_MACHINE_MICROTCH_H
#define MAME_MACHINE_MICROTCH_H

#pragma once

#include "diserial.h"

class microtouch_device :
		public device_t,
		public device_serial_interface
{
public:
	typedef device_delegate<int (int *, int *)> touch_cb;

	microtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto stx() { return m_out_stx_func.bind(); }
	template <typename... T> void set_touch_callback(T &&... args) { m_out_touch_cb.set(std::forward<T>(args)...); }

	void rx(int state) { device_serial_interface::rx_w(state); }
	DECLARE_INPUT_CHANGED_MEMBER(touch);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_serial_interface implementation
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	// ROM region (unused, as the device is HLE'd)
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_output);

private:
	int check_command( const char* commandtocheck, int command_len, uint8_t* command_data );
	void send_format_table_packet(uint8_t flag, int x, int y);
	void send_format_decimal_packet(int x, int y);
	void send_touch_packet();

	enum
	{
		FORMAT_UNKNOWN,
		FORMAT_TABLET,
		FORMAT_DECIMAL
	};

	enum
	{
		MODE_INACTIVE,
		MODE_STREAM,
		MODE_POINT
	};

	uint8_t     m_rx_buffer[16];
	int         m_rx_buffer_ptr;
	uint8_t     m_tx_buffer[16];
	uint8_t     m_tx_buffer_num;
	uint8_t     m_tx_buffer_ptr;
	int         m_reset_done;
	int         m_format;
	int         m_mode;
	int         m_last_touch_state;
	int         m_last_x;
	int         m_last_y;
	touch_cb    m_out_touch_cb;
	devcb_write_line m_out_stx_func;
	required_ioport m_touch;
	required_ioport m_touchx;
	required_ioport m_touchy;
	emu_timer*  m_timer;
	bool        m_output_valid;
	uint8_t     m_output;
};

DECLARE_DEVICE_TYPE(MICROTOUCH, microtouch_device)

#endif // MAME_MACHINE_MICROTCH_H

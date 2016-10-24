// license:BSD-3-Clause
// copyright-holders:F. Ulivi
//
#ifndef _RS232_PTY_H_
#define _RS232_PTY_H_

#include "rs232.h"

class pseudo_terminal_device : public device_t,
								public device_serial_interface,
								public device_rs232_port_interface,
								public device_pty_interface
{
public:
		pseudo_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		virtual void input_txd(int state) override {
				device_serial_interface::rx_w(state);
		}

		void update_serial(int state);

protected:
		virtual ioport_constructor device_input_ports() const override;
		virtual void device_start() override;
		virtual void device_stop() override;
		virtual void device_reset() override;
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

		virtual void tra_callback() override;
		virtual void tra_complete() override;
		virtual void rcv_complete() override;

private:
		required_ioport m_rs232_txbaud;
		required_ioport m_rs232_rxbaud;
		required_ioport m_rs232_startbits;
		required_ioport m_rs232_databits;
		required_ioport m_rs232_parity;
		required_ioport m_rs232_stopbits;

		uint8_t m_input_buffer[ 1024 ];
		uint32_t m_input_count;
		uint32_t m_input_index;
		emu_timer *m_timer_poll;

		void queue(void);
};

extern const device_type PSEUDO_TERMINAL;

#endif /* _RS232_PTY_H_ */

// license:BSD-3-Clause
// copyright-holders:smf
/*****************************************************************************
 *
 * machine/ser_mouse.h
 *
 ****************************************************************************/

#ifndef MAME_BUS_RS232_SER_MOUSE_H
#define MAME_BUS_RS232_SER_MOUSE_H

#include "rs232.h"
#include "diserial.h"

class serial_mouse_device :
		public device_t,
		public device_rs232_port_interface,
		public device_serial_interface
{
public:
	virtual ioport_constructor device_input_ports() const override;

protected:
	serial_mouse_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void mouse_trans(int dx, int dy, int nb, int mbc) = 0;
	virtual void set_frame() = 0;
	void set_mouse_enable(bool state);
	void queue_data(uint8_t data) {m_queue[m_head] = data; ++m_head %= 256;}
	uint8_t unqueue_data() {uint8_t ret = m_queue[m_tail]; ++m_tail %= 256; return ret;}
	virtual void tra_complete() override;
	virtual void tra_callback() override;
	void reset_mouse();

	virtual WRITE_LINE_MEMBER(input_dtr) override;
	virtual WRITE_LINE_MEMBER(input_rts) override;

	int m_dtr;
	int m_rts;

private:
	void check_state() { set_mouse_enable(!m_dtr && !m_rts); }

	uint8_t m_queue[256];
	uint8_t m_head, m_tail, m_mb;

	emu_timer *m_timer;
	bool m_enabled;

	required_ioport m_x;
	required_ioport m_y;
	required_ioport m_btn;
};

class microsoft_mouse_device : public serial_mouse_device
{
public:
	microsoft_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual WRITE_LINE_MEMBER(input_rts) override;

	virtual void set_frame() override { set_data_frame(1, 7, PARITY_NONE, STOP_BITS_2); }
	virtual void mouse_trans(int dx, int dy, int nb, int mbc) override;
};

DECLARE_DEVICE_TYPE(MSFT_SERIAL_MOUSE, microsoft_mouse_device)

class mouse_systems_mouse_device : public serial_mouse_device
{
public:
	mouse_systems_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void set_frame() override { set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2); }
	virtual void mouse_trans(int dx, int dy, int nb, int mbc) override;
};

DECLARE_DEVICE_TYPE(MSYSTEM_SERIAL_MOUSE, mouse_systems_mouse_device)

#endif // MAME_BUS_RS232_SER_MOUSE_H

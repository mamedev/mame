// license:BSD-3-Clause
// copyright-holders:smf
/*****************************************************************************
 *
 * machine/ser_mouse.h
 *
 ****************************************************************************/

#ifndef SER_MOUSE_H_
#define SER_MOUSE_H_

#include "rs232.h"

class serial_mouse_device :
		public device_t,
		public device_rs232_port_interface,
		public device_serial_interface
{
public:
	serial_mouse_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual ioport_constructor device_input_ports() const override;

protected:
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void mouse_trans(int dx, int dy, int nb, int mbc) = 0;
	virtual void set_frame() = 0;
	void set_mouse_enable(bool state);
	void queue_data(UINT8 data) {m_queue[m_head] = data; ++m_head %= 256;}
	UINT8 unqueue_data() {UINT8 ret = m_queue[m_tail]; ++m_tail %= 256; return ret;}
	virtual void tra_complete() override;
	virtual void tra_callback() override;
	void reset_mouse();

	virtual WRITE_LINE_MEMBER(input_dtr) override;
	virtual WRITE_LINE_MEMBER(input_rts) override;
	int m_dtr;
	int m_rts;

private:
	UINT8 m_queue[256];
	UINT8 m_head, m_tail, m_mb;

	emu_timer *m_timer;
	bool m_enabled;

	required_ioport m_x;
	required_ioport m_y;
	required_ioport m_btn;

	void check_state() { set_mouse_enable(!m_dtr && !m_rts); }
};

class microsoft_mouse_device : public serial_mouse_device
{
public:
	microsoft_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual WRITE_LINE_MEMBER(input_rts) override;

	virtual void set_frame() override { set_data_frame(1, 7, PARITY_NONE, STOP_BITS_2); }
	virtual void mouse_trans(int dx, int dy, int nb, int mbc) override;
};

extern const device_type MSFT_SERIAL_MOUSE;

class mouse_systems_mouse_device : public serial_mouse_device
{
public:
	mouse_systems_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void set_frame() override { set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2); }
	virtual void mouse_trans(int dx, int dy, int nb, int mbc) override;
};

extern const device_type MSYSTEM_SERIAL_MOUSE;

#endif /* SER_MOUSE_H_ */

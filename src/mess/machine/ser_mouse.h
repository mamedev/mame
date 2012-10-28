/*****************************************************************************
 *
 * machine/ser_mouse.h
 *
 ****************************************************************************/

#ifndef SER_MOUSE_H_
#define SER_MOUSE_H_

#include "emu.h"
#include "machine/serial.h"

class serial_mouse_device :
		public device_t,
		public device_rs232_port_interface,
		public device_serial_interface
{
public:
	serial_mouse_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void mouse_trans(int dx, int dy, int nb, int mbc) = 0;
	virtual void set_frame() = 0;
	void set_mouse_enable(bool state);
	void queue_data(UINT8 data) {m_queue[m_head] = data; ++m_head %= 256;}
	UINT8 unqueue_data() {UINT8 ret = m_queue[m_tail]; ++m_tail %= 256; return ret;}
	virtual void input_callback(UINT8 state) { m_input_state = state; }
	virtual void tra_complete();
	virtual void tra_callback();
private:
	UINT8 m_queue[256];
	UINT8 m_head, m_tail, m_mb;

	emu_timer *m_timer;
	rs232_port_device *m_owner;
	bool m_enabled;
};

class microsoft_mouse_device : public serial_mouse_device
{
public:
	microsoft_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void dtr_w(UINT8 state) { m_dtr = state; check_state(); }
	virtual void rts_w(UINT8 state) { m_rts = state; check_state(); m_old_rts = state; }
protected:
	virtual void set_frame() { set_data_frame(7, 2, SERIAL_PARITY_NONE); }
	virtual void mouse_trans(int dx, int dy, int nb, int mbc);
	virtual void device_reset() {m_old_rts = 0; serial_mouse_device::device_reset();}
	virtual void device_config_complete() { m_shortname = "microsoft_mouse"; }
private:
	void check_state();
	UINT8 m_old_rts;
};
extern const device_type MSFT_SERIAL_MOUSE;

class mouse_systems_mouse_device : public serial_mouse_device
{
public:
	mouse_systems_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual void dtr_w(UINT8 state) { m_dtr = state; check_state(); }
	virtual void rts_w(UINT8 state) { m_rts = state; check_state(); }
protected:
	virtual void set_frame() { set_data_frame(8, 2, SERIAL_PARITY_NONE); }
	virtual void mouse_trans(int dx, int dy, int nb, int mbc);
	virtual void device_config_complete() { m_shortname = "mouse_systems_mouse"; }
private:
	void check_state() { set_mouse_enable((m_dtr && m_rts)?true:false); }
};

extern const device_type MSYSTEM_SERIAL_MOUSE;

#endif /* SER_MOUSE_H_ */

#ifndef _MICROTOUCH_H
#define _MICROTOUCH_H

#include "emu.h"


class microtouch_device :
		public device_t
{
public:
	microtouch_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	microtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	template<class _Object> static devcb2_base &static_set_tx_callback(device_t &device, _Object object) { return downcast<microtouch_device &>(device).m_out_tx_func.set_callback(object); }

	virtual ioport_constructor device_input_ports() const;
	DECLARE_WRITE8_MEMBER(rx);
	DECLARE_INPUT_CHANGED_MEMBER(touch);

	typedef delegate<int (int *, int *)> touch_cb;
	static void static_set_touch_callback(device_t &device, touch_cb object) { downcast<microtouch_device &>(device).m_out_touch_cb = object; }
protected:
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void tx(UINT8 data) { m_out_tx_func(data); }
	emu_timer*  m_timer;
private:
	int check_command( const char* commandtocheck, int command_len, UINT8* command_data );
	void send_format_table_packet(UINT8 flag, int x, int y);
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
	UINT8       m_rx_buffer[16];
	int         m_rx_buffer_ptr;
	UINT8       m_tx_buffer[16];
	UINT8       m_tx_buffer_num;
	UINT8       m_tx_buffer_ptr;
	int         m_reset_done;
	int         m_format;
	int         m_mode;
	int         m_last_touch_state;
	int         m_last_x;
	int         m_last_y;
	devcb2_write8 m_out_tx_func;
	touch_cb m_out_touch_cb;
	required_ioport m_touch;
	required_ioport m_touchx;
	required_ioport m_touchy;
};

extern const device_type MICROTOUCH;

#define MCFG_MICROTOUCH_ADD(_tag, _devcb) \
	MCFG_DEVICE_ADD(_tag, MICROTOUCH, 0) \
	devcb = &microtouch_serial_device::static_set_tx_callback(*device, DEVCB2_##_devcb);


class microtouch_serial_device
	: public microtouch_device,
		public device_serial_interface
{
public:
	microtouch_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	template<class _Object> static devcb2_base &static_set_stx_callback(device_t &device, _Object object) { return downcast<microtouch_serial_device &>(device).m_out_stx_func.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER(rx) { device_serial_interface::rx_w(state); }
protected:
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void tx(UINT8 data);
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();
private:
	bool m_output_valid;
	UINT8 m_output;
	devcb2_write_line m_out_stx_func;
};

extern const device_type MICROTOUCH_SERIAL;

#define MCFG_MICROTOUCH_SERIAL_ADD(_tag, _clock, _devcb) \
	MCFG_DEVICE_ADD(_tag, MICROTOUCH_SERIAL, _clock) \
	devcb = &microtouch_serial_device::static_set_stx_callback(*device, DEVCB2_##_devcb);

#define MCFG_MICROTOUCH_TOUCH_CB(_class, _touch_cb) \
	microtouch_device::static_set_touch_callback(*device, microtouch_device::touch_cb(FUNC(_class::_touch_cb), (_class *)owner));


#endif //_MICROTOUCH_H

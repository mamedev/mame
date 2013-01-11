#ifndef _MICROTOUCH_H
#define _MICROTOUCH_H

#include "emu.h"

typedef int (*microtouch_touch_func)(int *touch_x, int *touch_y);
#define MICROTOUCH_TOUCH(name) int name(int *touch_x, int *touch_y)

struct microtouch_interface
{
	devcb_write8            m_out_tx_cb;
	microtouch_touch_func   m_out_touch_cb;
};

class microtouch_device :
		public device_t,
		public microtouch_interface
{
public:
	microtouch_device(const machine_config &mconfig, device_type type, const char* name, const char *tag, device_t *owner, UINT32 clock);
	microtouch_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;
	DECLARE_WRITE8_MEMBER(rx);
	DECLARE_INPUT_CHANGED_MEMBER(touch);
protected:
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();
	virtual void tx(UINT8 data) { m_out_tx_func(0, data); }
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
	devcb_resolved_write8 m_out_tx_func;
};

extern const device_type MICROTOUCH;

#define MCFG_MICROTOUCH_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MICROTOUCH, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

struct microtouch_serial_interface
{
	devcb_write_line m_out_stx_cb;
};

class microtouch_serial_device
	: public microtouch_device,
		public device_serial_interface,
		public microtouch_serial_interface
{
public:
	microtouch_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER(rx) { check_for_start(state); }
protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void tx(UINT8 data);
	virtual void input_callback(UINT8 state) { m_input_state = state; }
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();
private:
	bool m_output_valid;
	UINT8 m_output;
	devcb_resolved_write_line m_out_stx_func;
};

extern const device_type MICROTOUCH_SERIAL;

#define MCFG_MICROTOUCH_SERIAL_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, MICROTOUCH_SERIAL, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif //_MICROTOUCH_H

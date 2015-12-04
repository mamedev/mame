// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    pce220_ser.h

    Sharp PC-E220/PC-G850V Serial I/O

****************************************************************************/

#ifndef __PCE220_SER_H__
#define __PCE220_SER_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> pce220_serial_device

class pce220_serial_device :    public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	pce220_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~pce220_serial_device();

	// image-level overrides
	virtual bool call_load();
	virtual void call_unload();
	virtual bool call_create(int format_type, option_resolution *format_options);

	virtual iodevice_t image_type() const { return IO_SERIAL; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return nullptr; }
	virtual const char *file_extensions() const { return "txt,ihx"; }
	virtual const option_guide *create_option_guide() const { return nullptr; }

	// specific implementation
	UINT8 in_xin(void) { return m_xin & 0x01; }
	UINT8 in_din(void) { return m_din & 0x01; }
	UINT8 in_ack(void) { return m_ack & 0x01; }
	void out_busy(UINT8 state)  { m_busy = state & 0x01; }
	void out_dout(UINT8 state)  { m_dout = state & 0x01; }
	void out_xout(UINT8 state)  { m_xout = state & 0x01; }
	void enable_interface(UINT8 state)  { m_enabled = state & 0x01; }

protected:
	// internal helpers
	int calc_parity(UINT8 data);
	int get_next_state();

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal device state
	static const device_timer_id TIMER_SEND    = 0;
	static const device_timer_id TIMER_RECEIVE = 1;

	emu_timer*  m_send_timer;       // timer for send data
	emu_timer*  m_receive_timer;    // timer for receive data
	UINT8       m_state;            // transfer status
	UINT32      m_bytes_count;      // number of bytes transferred
	UINT8       m_current_byte;     // byte in transfer
	UINT8       m_enabled;          // enable/disable

	UINT8       m_busy;             // CTS
	UINT8       m_dout;             // DTR
	UINT8       m_xout;             // TXD
	UINT8       m_xin;              // RXD
	UINT8       m_din;              // DSR
	UINT8       m_ack;              // RTS
};

// device type definition
extern const device_type PCE220SERIAL;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define PCE220SERIAL_TAG        "serial"

#define MCFG_PCE220_SERIAL_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PCE220SERIAL, 0)
#endif /* __PCE220_SER_H__ */

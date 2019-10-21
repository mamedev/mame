// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    pce220_ser.h

    Sharp PC-E220/PC-G850V Serial I/O

****************************************************************************/

#ifndef MAME_MACHINE_PCE220_SER_H
#define MAME_MACHINE_PCE220_SER_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> pce220_serial_device

class pce220_serial_device :    public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	pce220_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~pce220_serial_device();

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual image_init_result call_create(int format_type, util::option_resolution *format_options) override;

	virtual iodevice_t image_type() const override { return IO_SERIAL; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 1; }
	virtual bool is_creatable() const override { return 1; }
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 0; }
	virtual const char *file_extensions() const override { return "txt,ihx"; }

	// specific implementation
	uint8_t in_xin(void) { return m_xin & 0x01; }
	uint8_t in_din(void) { return m_din & 0x01; }
	uint8_t in_ack(void) { return m_ack & 0x01; }
	void out_busy(uint8_t state)  { m_busy = state & 0x01; }
	void out_dout(uint8_t state)  { m_dout = state & 0x01; }
	void out_xout(uint8_t state)  { m_xout = state & 0x01; }
	void enable_interface(uint8_t state)  { m_enabled = state & 0x01; }

protected:
	// internal helpers
	int calc_parity(uint8_t data);
	int get_next_state();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// internal device state
	static const device_timer_id TIMER_SEND    = 0;
	static const device_timer_id TIMER_RECEIVE = 1;

	emu_timer*  m_send_timer;       // timer for send data
	emu_timer*  m_receive_timer;    // timer for receive data
	uint8_t       m_state;            // transfer status
	uint32_t      m_bytes_count;      // number of bytes transferred
	uint8_t       m_current_byte;     // byte in transfer
	uint8_t       m_enabled;          // enable/disable

	uint8_t       m_busy;             // CTS
	uint8_t       m_dout;             // DTR
	uint8_t       m_xout;             // TXD
	uint8_t       m_xin;              // RXD
	uint8_t       m_din;              // DSR
	uint8_t       m_ack;              // RTS
};

// device type definition
DECLARE_DEVICE_TYPE(PCE220SERIAL, pce220_serial_device)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define PCE220SERIAL_TAG        "serial"

#endif // MAME_MACHINE_PCE220_SER_H

// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/****************************************************************************

    pce220_ser.h

    Sharp PC-E220/PC-G850V Serial I/O

****************************************************************************/

#ifndef MAME_SHARP_PCE220_SER_H
#define MAME_SHARP_PCE220_SER_H

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

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "txt,ihx"; }
	virtual const char *image_type_name() const noexcept override { return "serial"; }
	virtual const char *image_brief_type_name() const noexcept override { return "serl"; }

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

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(send_tick);
	TIMER_CALLBACK_MEMBER(receive_tick);

private:
	// internal device state
	emu_timer*  m_send_timer;           // timer for send data
	emu_timer*  m_receive_timer;        // timer for receive data
	uint8_t     m_state;                // transfer status
	uint32_t    m_bytes_count;          // number of bytes transferred
	uint8_t     m_current_byte;         // byte in transfer
	uint8_t     m_enabled;              // enable/disable

	uint8_t     m_busy;                 // CTS
	uint8_t     m_dout;                 // DTR
	uint8_t     m_xout;                 // TXD
	uint8_t     m_xin;                  // RXD
	uint8_t     m_din;                  // DSR
	uint8_t     m_ack;                  // RTS
};

// device type definition
DECLARE_DEVICE_TYPE(PCE220SERIAL, pce220_serial_device)

#endif // MAME_SHARP_PCE220_SER_H

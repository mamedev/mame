// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Serial and parallel interface card
    See ti_rs232.c for documentation

    Michael Zapf
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_TI_RS232_H
#define MAME_BUS_TI99_PEB_TI_RS232_H

#pragma once

#include "peribox.h"
#include "machine/74259.h"
#include "machine/tms9902.h"

namespace bus::ti99::peb {

class ti_pio_attached_device;
class ti_rs232_attached_device;

class ti_rs232_pio_device : public device_t, public device_ti99_peribox_card_interface
{
	friend class ti_pio_attached_device;
	friend class ti_rs232_attached_device;

public:
	ti_rs232_pio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void int0_callback(int state);
	void int1_callback(int state);
	void rcv0_callback(int state);
	void rcv1_callback(int state);
	void xmit0_callback(uint8_t data);
	void xmit1_callback(uint8_t data);
	void ctrl0_callback(offs_t offset, uint8_t data);
	void ctrl1_callback(offs_t offset, uint8_t data);

	void selected_w(int state);
	void pio_direction_in_w(int state);
	void pio_handshake_out_w(int state);
	void pio_spareout_w(int state);
	void flag0_w(int state);
	void cts0_w(int state);
	void cts1_w(int state);
	void led_w(int state);

	void        incoming_dtr(int uartind, line_state value);
	void        transmit_data(int uartind, uint8_t value);
	uint8_t       map_lines_out(int uartind, uint8_t value);
	uint8_t       map_lines_in(int uartind, uint8_t value);
	void        receive_data_or_line_state(int uartind);
	void        set_bit(int uartind, int line, int value);

	void        configure_interface(int uartind, int type, int value);
	void        output_line_state(int uartind, int mask, uint8_t value);
	void        output_exception(int uartind, int param, uint8_t value);
	void        ctrl_callback(int uartind, int type, uint8_t data);

	required_device<ls259_device> m_crulatch;
	// Asynchronous receivers/transmitters
	required_device<tms9902_device> m_uart0;
	required_device<tms9902_device> m_uart1;

	// Connected images (file or socket connection) that represent the
	// devices that are connected to the serial adapters
	required_device<ti_rs232_attached_device> m_serdev0;
	required_device<ti_rs232_attached_device> m_serdev1;

	// Connected image (file) that represents the device connected to the
	// parallel interface
	required_device<ti_pio_attached_device> m_piodev;
	uint8_t*                      m_dsrrom;

	// Input buffer for each UART. We have to copy the contents of sdlsocket here
	// because the buffer in corefile will be lost on the next write operation
	std::unique_ptr<uint8_t[]>      m_recvbuf[2];
	int         m_bufpos[2], m_buflen[2];

	// Latches the state of the output lines for UART0/UART1
	uint8_t   m_signals[2];
	int     m_recv_mode[2];     // May be NORMAL or ESC

	// Baud rate management
	// not part of the real device, but required for the connection to the
	// real UART
	double  m_time_hold[2];

	// PIO flags
	bool    m_pio_direction_in;     // a.k.a. PIOOC pio in output mode if 0
	bool    m_pio_handshakeout;
	bool    m_pio_handshakein;
	bool    m_pio_spareout;
	bool    m_pio_sparein;
	bool    m_flag0;                // spare
	bool    m_led;              // a.k.a. flag3
	int     m_pio_out_buffer;
	int     m_pio_in_buffer;
	bool    m_pio_readable;
	bool    m_pio_writable;
	bool    m_pio_write;            // true if image is to be written to

	/* Keeps the value put on the bus when SENILA becomes active. */
	uint8_t   m_ila;
};

/****************************************************************************/

/*
    Defines the serial serdev. "TI99 RS232 attached serial device"
*/
class ti_rs232_attached_device : public device_t, public device_image_interface
{
public:
	ti_rs232_attached_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool is_readable()  const noexcept override           { return true; }
	bool is_writeable() const noexcept override           { return true; }
	bool is_creatable() const noexcept override           { return true; }
	bool is_reset_on_load() const noexcept override       { return false; }
	bool support_command_line_image_creation() const noexcept override { return true; }
	const char *image_type_name() const noexcept override { return "serial"; }
	const char *image_brief_type_name() const noexcept override { return "serl"; }
	const char *image_interface() const noexcept override { return ""; }
	const char *file_extensions() const noexcept override { return ""; }
	void connect(tms9902_device *dev) { m_uart = dev; }

protected:
	void device_start() override { }
	std::pair<std::error_condition, std::string>    call_load() override;
	void    call_unload() override;

private:
	int get_index_from_tagname();
	tms9902_device* m_uart;
};

/*
    Defines the PIO (parallel IO) "TI99 PIO attached device"
*/
class ti_pio_attached_device : public device_t, public device_image_interface
{
	friend class ti_rs232_pio_device;
public:
	ti_pio_attached_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool is_readable()  const noexcept override           { return true; }
	bool is_writeable() const noexcept override           { return true; }
	bool is_creatable() const noexcept override           { return true; }
	bool is_reset_on_load() const noexcept override       { return false; }
	bool support_command_line_image_creation() const noexcept override { return true; }
	const char *image_type_name() const noexcept override { return "parallel"; }
	const char *image_brief_type_name() const noexcept override { return "parl"; }
	const char *image_interface() const noexcept override { return ""; }
	const char *file_extensions() const noexcept override { return ""; }

protected:
	void    device_start() override { }
	std::pair<std::error_condition, std::string>    call_load() override;
	void    call_unload() override;

private:
	ti_rs232_pio_device* m_card;
	void set_card(ti_rs232_pio_device* card) { m_card = card; }
};

} // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_RS232,     bus::ti99::peb, ti_rs232_pio_device)
DECLARE_DEVICE_TYPE_NS(TI99_RS232_DEV, bus::ti99::peb, ti_rs232_attached_device)
DECLARE_DEVICE_TYPE_NS(TI99_PIO_DEV,   bus::ti99::peb, ti_pio_attached_device)

#endif // MAME_BUS_TI99_PEB_TI_RS232_H

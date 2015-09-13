// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __BUS_RS232_H__
#define __BUS_RS232_H__

#pragma once

#include "emu.h"

#define MCFG_RS232_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, RS232_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_RS232_RXD_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_rxd_handler(*device, DEVCB_##_devcb);

#define MCFG_RS232_DCD_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_dcd_handler(*device, DEVCB_##_devcb);

#define MCFG_RS232_DSR_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_dsr_handler(*device, DEVCB_##_devcb);

#define MCFG_RS232_RI_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_ri_handler(*device, DEVCB_##_devcb);

#define MCFG_RS232_CTS_HANDLER(_devcb) \
	devcb = &rs232_port_device::set_cts_handler(*device, DEVCB_##_devcb);

#define RS232_BAUD_110 (0x00)
#define RS232_BAUD_150 (0x01)
#define RS232_BAUD_300 (0x02)
#define RS232_BAUD_600 (0x03)
#define RS232_BAUD_1200 (0x04)
#define RS232_BAUD_2400 (0x05)
#define RS232_BAUD_4800 (0x06)
#define RS232_BAUD_9600 (0x07)
#define RS232_BAUD_14400 (0x08)
#define RS232_BAUD_19200 (0x09)
#define RS232_BAUD_28800 (0x0a)
#define RS232_BAUD_38400 (0x0b)
#define RS232_BAUD_57600 (0x0c)
#define RS232_BAUD_115200 (0x0d)

#define MCFG_RS232_BAUD(_tag, _default_baud, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, _default_baud, _description) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_BAUD_110, "110") \
	PORT_CONFSETTING( RS232_BAUD_150, "150") \
	PORT_CONFSETTING( RS232_BAUD_300, "300") \
	PORT_CONFSETTING( RS232_BAUD_600, "600") \
	PORT_CONFSETTING( RS232_BAUD_1200, "1200") \
	PORT_CONFSETTING( RS232_BAUD_2400, "2400") \
	PORT_CONFSETTING( RS232_BAUD_4800, "4800") \
	PORT_CONFSETTING( RS232_BAUD_9600, "9600") \
	PORT_CONFSETTING( RS232_BAUD_14400, "14400") \
	PORT_CONFSETTING( RS232_BAUD_19200, "19200") \
	PORT_CONFSETTING( RS232_BAUD_28800, "28800") \
	PORT_CONFSETTING( RS232_BAUD_38400, "38400") \
	PORT_CONFSETTING( RS232_BAUD_57600, "57600") \
	PORT_CONFSETTING( RS232_BAUD_115200, "115200")

#define RS232_STARTBITS_0 (0x00)
#define RS232_STARTBITS_1 (0x01)

#define MCFG_RS232_STARTBITS(_tag, _default_startbits, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, _default_startbits, _description) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_STARTBITS_0, "0") \
	PORT_CONFSETTING( RS232_STARTBITS_1, "1")

#define RS232_DATABITS_5 (0x00)
#define RS232_DATABITS_6 (0x01)
#define RS232_DATABITS_7 (0x02)
#define RS232_DATABITS_8 (0x03)

#define MCFG_RS232_DATABITS(_tag, _default_databits, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, _default_databits, _description) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_DATABITS_5, "5") \
	PORT_CONFSETTING( RS232_DATABITS_6, "6") \
	PORT_CONFSETTING( RS232_DATABITS_7, "7") \
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

#define RS232_PARITY_NONE (0x00)
#define RS232_PARITY_ODD (0x01)
#define RS232_PARITY_EVEN (0x02)
#define RS232_PARITY_MARK (0x03)
#define RS232_PARITY_SPACE (0x04)

#define MCFG_RS232_PARITY(_tag, _default_parity, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, _default_parity, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_PARITY_NONE, "None") \
	PORT_CONFSETTING( RS232_PARITY_ODD, "Odd") \
	PORT_CONFSETTING( RS232_PARITY_EVEN, "Even") \
	PORT_CONFSETTING( RS232_PARITY_MARK, "Mark") \
	PORT_CONFSETTING( RS232_PARITY_SPACE, "Space")

#define RS232_STOPBITS_0 (0x00)
#define RS232_STOPBITS_1 (0x01)
#define RS232_STOPBITS_1_5 (0x02)
#define RS232_STOPBITS_2 (0x03)

#define MCFG_RS232_STOPBITS(_tag, _default_stopbits, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, 0x01, _description) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_STOPBITS_0, "0") \
	PORT_CONFSETTING( RS232_STOPBITS_1, "1") \
	PORT_CONFSETTING( RS232_STOPBITS_1_5, "1.5") \
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")

class device_rs232_port_interface;

class rs232_port_device : public device_t,
	public device_slot_interface
{
	friend class device_rs232_port_interface;

public:
	rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	rs232_port_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	virtual ~rs232_port_device();

	// static configuration helpers
	template<class _Object> static devcb_base &set_rxd_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_rxd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dcd_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_dcd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dsr_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_dsr_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ri_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_ri_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_cts_handler(device_t &device, _Object object) { return downcast<rs232_port_device &>(device).m_cts_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );
	DECLARE_WRITE_LINE_MEMBER( write_dtr );
	DECLARE_WRITE_LINE_MEMBER( write_rts );
	DECLARE_WRITE_LINE_MEMBER( write_etc );

	DECLARE_READ_LINE_MEMBER( rxd_r ) { return m_rxd; }
	DECLARE_READ_LINE_MEMBER( dcd_r ) { return m_dcd; }
	DECLARE_READ_LINE_MEMBER( dsr_r ) { return m_dsr; }
	DECLARE_READ_LINE_MEMBER( ri_r )  { return m_ri; }
	DECLARE_READ_LINE_MEMBER( cts_r ) { return m_cts; }

protected:
	virtual void device_start();
	virtual void device_config_complete();

	int m_rxd;
	int m_dcd;
	int m_dsr;
	int m_ri;
	int m_cts;

	devcb_write_line m_rxd_handler;
	devcb_write_line m_dcd_handler;
	devcb_write_line m_dsr_handler;
	devcb_write_line m_ri_handler;
	devcb_write_line m_cts_handler;

private:
	device_rs232_port_interface *m_dev;
};

class device_rs232_port_interface : public device_slot_card_interface
{
	friend class rs232_port_device;

public:
	device_rs232_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_rs232_port_interface();

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_dtr ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_rts ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_etc ) {}

	DECLARE_WRITE_LINE_MEMBER( output_rxd ) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_dcd ) { m_port->m_dcd = state; m_port->m_dcd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_dsr ) { m_port->m_dsr = state; m_port->m_dsr_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_ri )  { m_port->m_ri = state; m_port->m_ri_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_cts ) { m_port->m_cts = state; m_port->m_cts_handler(state); }

protected:
	rs232_port_device *m_port;

	static int convert_baud(UINT8 baud)
	{
		static const int values[] =
		{
			110,
			150,
			300,
			600,
			1200,
			2400,
			4800,
			9600,
			14400,
			19200,
			28800,
			38400,
			57600,
			115200
		};

		return values[baud];
	}

	static int convert_startbits(UINT8 startbits)
	{
		static const int values[] =
		{
			0,
			1
		};

		return values[startbits];
	}

	static int convert_databits(UINT8 databits)
	{
		static const int values[] =
		{
			5,
			6,
			7,
			8
		};

		return values[databits];
	}

	static device_serial_interface::parity_t convert_parity(UINT8 parity)
	{
		static const device_serial_interface::parity_t values[] =
		{
			device_serial_interface::PARITY_NONE,
			device_serial_interface::PARITY_ODD,
			device_serial_interface::PARITY_EVEN,
			device_serial_interface::PARITY_MARK,
			device_serial_interface::PARITY_SPACE
		};

		return values[parity];
	}

	static device_serial_interface::stop_bits_t convert_stopbits(UINT8 stopbits)
	{
		static const device_serial_interface::stop_bits_t values[] =
		{
			device_serial_interface::STOP_BITS_0,
			device_serial_interface::STOP_BITS_1,
			device_serial_interface::STOP_BITS_1_5,
			device_serial_interface::STOP_BITS_2
		};

		return values[stopbits];
	}
};

extern const device_type RS232_PORT;

SLOT_INTERFACE_EXTERN( default_rs232_devices );

#endif

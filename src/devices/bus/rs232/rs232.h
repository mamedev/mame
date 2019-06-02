// license:BSD-3-Clause
// copyright-holders:smf,Vas Crabb
#ifndef MAME_BUS_RS232_RS232_H
#define MAME_BUS_RS232_RS232_H

#pragma once

#include "diserial.h"


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

class rs232_port_device : public device_t, public device_slot_interface
{
	friend class device_rs232_port_interface;

public:
	template <typename T>
	rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: rs232_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	rs232_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~rs232_port_device();

	// static configuration helpers
	auto rxd_handler() { return m_rxd_handler.bind(); }
	auto dcd_handler() { return m_dcd_handler.bind(); }
	auto dsr_handler() { return m_dsr_handler.bind(); }
	auto ri_handler() { return m_ri_handler.bind(); }
	auto si_handler() { return m_si_handler.bind(); }
	auto cts_handler() { return m_cts_handler.bind(); }
	auto rxc_handler() { return m_rxc_handler.bind(); }
	auto txc_handler() { return m_txc_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER( write_txd );
	DECLARE_WRITE_LINE_MEMBER( write_dtr );
	DECLARE_WRITE_LINE_MEMBER( write_rts );
	DECLARE_WRITE_LINE_MEMBER( write_etc );
	DECLARE_WRITE_LINE_MEMBER( write_spds );

	DECLARE_READ_LINE_MEMBER( rxd_r ) { return m_rxd; }
	DECLARE_READ_LINE_MEMBER( dcd_r ) { return m_dcd; }
	DECLARE_READ_LINE_MEMBER( dsr_r ) { return m_dsr; }
	DECLARE_READ_LINE_MEMBER( ri_r )  { return m_ri; }
	DECLARE_READ_LINE_MEMBER( si_r )  { return m_si; }
	DECLARE_READ_LINE_MEMBER( cts_r ) { return m_cts; }
	DECLARE_READ_LINE_MEMBER( rxc_r ) { return m_dce_rxc; }
	DECLARE_READ_LINE_MEMBER( txc_r ) { return m_dce_txc; }

protected:
	rs232_port_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_config_complete() override;

	int m_rxd;
	int m_dcd;
	int m_dsr;
	int m_ri;
	int m_si;
	int m_cts;
	int m_dce_rxc;
	int m_dce_txc;

	devcb_write_line m_rxd_handler;
	devcb_write_line m_dcd_handler;
	devcb_write_line m_dsr_handler;
	devcb_write_line m_ri_handler;
	devcb_write_line m_si_handler;
	devcb_write_line m_cts_handler;
	devcb_write_line m_rxc_handler;
	devcb_write_line m_txc_handler;

private:
	device_rs232_port_interface *m_dev;
};

class device_rs232_port_interface : public device_slot_card_interface
{
	friend class rs232_port_device;

public:
	virtual ~device_rs232_port_interface();

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_dtr ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_rts ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_etc ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_spds ) { }

	DECLARE_WRITE_LINE_MEMBER( output_rxd ) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_dcd ) { m_port->m_dcd = state; m_port->m_dcd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_dsr ) { m_port->m_dsr = state; m_port->m_dsr_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_ri )  { m_port->m_ri = state; m_port->m_ri_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_si )  { m_port->m_si = state; m_port->m_si_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_cts ) { m_port->m_cts = state; m_port->m_cts_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_rxc ) { m_port->m_dce_rxc = state; m_port->m_rxc_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_txc ) { m_port->m_dce_txc = state; m_port->m_txc_handler(state); }

protected:
	device_rs232_port_interface(const machine_config &mconfig, device_t &device);

	rs232_port_device *m_port;

	static int convert_baud(uint8_t baud)
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

	static int convert_startbits(uint8_t startbits)
	{
		static const int values[] =
		{
			0,
			1
		};

		return values[startbits];
	}

	static int convert_databits(uint8_t databits)
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

	static device_serial_interface::parity_t convert_parity(uint8_t parity)
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

	static device_serial_interface::stop_bits_t convert_stopbits(uint8_t stopbits)
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

template <uint32_t FIFO_LENGTH>
class buffered_rs232_device : public device_t, public device_buffered_serial_interface<FIFO_LENGTH>, public device_rs232_port_interface
{
public:
	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override
	{
		device_buffered_serial_interface<FIFO_LENGTH>::rx_w(state);
	}

protected:
	buffered_rs232_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_buffered_serial_interface<FIFO_LENGTH>(mconfig, *this)
		, device_rs232_port_interface(mconfig, *this)
	{
	}

	virtual void device_start() override
	{
	}

	virtual void tra_callback() override
	{
		output_rxd(this->transmit_register_get_data_bit());
	}
};

DECLARE_DEVICE_TYPE(RS232_PORT, rs232_port_device)

void default_rs232_devices(device_slot_interface &device);

#endif // MAME_BUS_RS232_RS232_H

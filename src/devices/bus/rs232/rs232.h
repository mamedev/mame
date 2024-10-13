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

#define RS232_BAUD_50 (0x0e)
#define RS232_BAUD_75 (0x0f)
#define RS232_BAUD_134_5 (0x10)
#define RS232_BAUD_200 (0x11)
#define RS232_BAUD_1800 (0x12)
#define RS232_BAUD_2000 (0x13)
#define RS232_BAUD_3600 (0x14)
#define RS232_BAUD_7200 (0x15)
#define RS232_BAUD_111900 (0x16)

#define PORT_RS232_BAUD(_tag, _default_baud, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, _default_baud, _description) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_BAUD_50, "50") \
	PORT_CONFSETTING( RS232_BAUD_75, "75") \
	PORT_CONFSETTING( RS232_BAUD_110, "110") \
	PORT_CONFSETTING( RS232_BAUD_134_5, "134.5") \
	PORT_CONFSETTING( RS232_BAUD_150, "150") \
	PORT_CONFSETTING( RS232_BAUD_200, "200") \
	PORT_CONFSETTING( RS232_BAUD_300, "300") \
	PORT_CONFSETTING( RS232_BAUD_600, "600") \
	PORT_CONFSETTING( RS232_BAUD_1200, "1200") \
	PORT_CONFSETTING( RS232_BAUD_1800, "1800") \
	PORT_CONFSETTING( RS232_BAUD_2000, "2000") \
	PORT_CONFSETTING( RS232_BAUD_2400, "2400") \
	PORT_CONFSETTING( RS232_BAUD_3600, "3600") \
	PORT_CONFSETTING( RS232_BAUD_4800, "4800") \
	PORT_CONFSETTING( RS232_BAUD_7200, "7200") \
	PORT_CONFSETTING( RS232_BAUD_9600, "9600") \
	PORT_CONFSETTING( RS232_BAUD_14400, "14400") \
	PORT_CONFSETTING( RS232_BAUD_19200, "19200") \
	PORT_CONFSETTING( RS232_BAUD_28800, "28800") \
	PORT_CONFSETTING( RS232_BAUD_38400, "38400") \
	PORT_CONFSETTING( RS232_BAUD_57600, "57600") \
	PORT_CONFSETTING( RS232_BAUD_111900, "111900") \
	PORT_CONFSETTING( RS232_BAUD_115200, "115200")

#define RS232_DATABITS_5 (0x00)
#define RS232_DATABITS_6 (0x01)
#define RS232_DATABITS_7 (0x02)
#define RS232_DATABITS_8 (0x03)

#define PORT_RS232_DATABITS(_tag, _default_databits, _description, _class, _write_line) \
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

#define PORT_RS232_PARITY(_tag, _default_parity, _description, _class, _write_line) \
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

#define PORT_RS232_STOPBITS(_tag, _default_stopbits, _description, _class, _write_line) \
	PORT_START(_tag) \
	PORT_CONFNAME(0xff, 0x01, _description) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, _class, _write_line) \
	PORT_CONFSETTING( RS232_STOPBITS_0, "0") \
	PORT_CONFSETTING( RS232_STOPBITS_1, "1") \
	PORT_CONFSETTING( RS232_STOPBITS_1_5, "1.5") \
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")

class device_rs232_port_interface;

class rs232_port_device : public device_t, public device_single_card_slot_interface<device_rs232_port_interface>
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

	void write_txd(int state);        // DB25 pin  2  V.24 circuit 103   Transmitted data
	void write_dtr(int state);        // DB25 pin 20  V.24 circuit 108/2 Data terminal ready
	void write_rts(int state);        // DB25 pin  4  V.24 circuit 105   Request to send
	void write_etc(int state);        // DB25 pin 24  V.24 circuit 113   Transmitter signal element timing (DTE)
	void write_spds(int state);       // DB25 pin 23  V.24 circuit 111   Data signal rate selector (DTE)

	int rxd_r() { return m_rxd; }     // DB25 pin  3  V.24 circuit 104   Received data
	int dcd_r() { return m_dcd; }     // DB25 pin  8  V.24 circuit 109   Data channel received line signal detector
	int dsr_r() { return m_dsr; }     // DB25 pin  6  V.24 circuit 107   Data set ready
	int ri_r()  { return m_ri; }      // DB25 pin 22  V.24 circuit 125   Calling indicator
	int si_r()  { return m_si; }      //              V.24 circuit 112   Data signal rate selector (DCE)
	int cts_r() { return m_cts; }     // DB25 pin  5  V.24 circuit 106   Ready for sending
	int rxc_r() { return m_dce_rxc; } // DB25 pin 17  V.24 circuit 115   Receiver signal element timing (DCE)
	int txc_r() { return m_dce_txc; } // DB25 pin 15  V.24 circuit 114   Transmitter signal element timing (DCE)

protected:
	rs232_port_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
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

class device_rs232_port_interface : public device_interface
{
	friend class rs232_port_device;

public:
	virtual ~device_rs232_port_interface();

	virtual void input_txd(int state) { }
	virtual void input_dtr(int state) { }
	virtual void input_rts(int state) { }
	virtual void input_etc(int state) { }
	virtual void input_spds(int state) { }

	void output_rxd(int state) { m_port->m_rxd = state; m_port->m_rxd_handler(state); }
	void output_dcd(int state) { m_port->m_dcd = state; m_port->m_dcd_handler(state); }
	void output_dsr(int state) { m_port->m_dsr = state; m_port->m_dsr_handler(state); }
	void output_ri(int state)  { m_port->m_ri = state; m_port->m_ri_handler(state); }
	void output_si(int state)  { m_port->m_si = state; m_port->m_si_handler(state); }
	void output_cts(int state) { m_port->m_cts = state; m_port->m_cts_handler(state); }
	void output_rxc(int state) { m_port->m_dce_rxc = state; m_port->m_rxc_handler(state); }
	void output_txc(int state) { m_port->m_dce_txc = state; m_port->m_txc_handler(state); }

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
			115200,

			50,
			75,
			134, // -0.37% error
			200,
			1800,
			2000,
			3600,
			7200,
			111900
		};

		return values[baud];
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
	virtual void input_txd(int state) override
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

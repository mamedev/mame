// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * NABU PC RS232 Card
 *
 *******************************************************************/

#include "emu.h"
#include "rs232.h"

#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* NABU PC RS232 Device */

class rs232_device : public device_t, public bus::nabupc::device_option_expansion_interface
{
public:
	// construction/destruction
	rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_option_expansion_interface implementation
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void rxrdy_w(int state);

private:
	required_device<i8251_device> m_i8251;
	required_device<pit8253_device> m_pit8253;
};

//-------------------------------------------------
//  rs232_device - constructor
//-------------------------------------------------
rs232_device::rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NABUPC_OPTION_RS232, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_i8251(*this, "i8251"),
	m_pit8253(*this, "i8253")
{
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void rs232_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_i8251, DERIVED_CLOCK(1, 2));
	m_i8251->rxrdy_handler().set(FUNC(rs232_device::rxrdy_w));
	m_i8251->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));

	PIT8253(config, m_pit8253, DERIVED_CLOCK(1, 2));
	m_pit8253->set_clk<0>(clock() / 2);
	m_pit8253->out_handler<0>().set(m_i8251, FUNC(i8251_device::write_txc));
	m_pit8253->set_clk<1>(clock() / 2);
	m_pit8253->out_handler<1>().set(m_i8251, FUNC(i8251_device::write_rxc));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_i8251, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_i8251, FUNC(i8251_device::write_cts));
	rs232.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void rs232_device::device_start()
{
}

void rs232_device::rxrdy_w(int state)
{
	get_slot()->int_w(!state);
}

uint8_t rs232_device::read(offs_t offset)
{
	uint8_t result = 0xff;

	switch (offset) {
	case 0x00:
	case 0x01:
		result = m_i8251->read(offset);
		break;
	case 0x04:
	case 0x05:
	case 0x07:
		result = m_pit8253->read(offset & 3);
		break;
	case 0x0f:
		result = 0x08;
	}
	return result;
}

void rs232_device::write(offs_t offset, uint8_t data)
{
	switch (offset) {
	case 0x00:
	case 0x01:
		m_i8251->write(offset, data);
		break;
	case 0x04:
	case 0x05:
	case 0x07:
		m_pit8253->write(offset & 3, data);
		break;
	}
}

} // anonymous namespace

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(NABUPC_OPTION_RS232, bus::nabupc::device_option_expansion_interface, rs232_device, "nabupc_option_rs232", "NABU PC RS232 Card")

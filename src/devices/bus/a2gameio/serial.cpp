// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    Serial printer interface for Apple II Game I/O connector

    This bitbanged RS-232 adapter connects annunciator #0 to a serial
    transmit line using either discrete transistors or a MC1488
    driver. Software drivers for this type of interface (which works
    best at low baud rates) are listed in the Apple II Reference
    Manual ("A Simple Serial Output," p. 114–121) and in various
    hobbyist magazine articles. A few commercial programs also
    support it.

*********************************************************************/

#include "emu.h"
#include "bus/a2gameio/serial.h"

#include "bus/rs232/rs232.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apple2_gameio_serial_device

class apple2_gameio_serial_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_gameio_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_a2gameio_interface overrides
	virtual int sw0_r() override;
	virtual void an0_w(int state) override;
	virtual bool has_sw0() const override { return true; }

private:
	// device finders
	required_device<rs232_port_device> m_rs232;

	// input ports
	required_ioport m_config;
};


//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static INPUT_PORTS_START(gameio_serial)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x01, "Ready input")
	/*
		"If your printer is not equipped with a read ready signal, connect Game I/O pin 2 to pin 1.
		 This causes continual output without interruption, a state that can be dealt with only by
		 the hardiest of printers."
		— Hayden Apple Assembly Language Development System manual, appendix A
	*/
	PORT_CONFSETTING(0x00, DEF_STR(None))
	PORT_CONFSETTING(0x01, "DSR")
INPUT_PORTS_END

ioport_constructor apple2_gameio_serial_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gameio_serial);
}

void apple2_gameio_serial_device::device_add_mconfig(machine_config &config)
{
	RS232_PORT(config, m_rs232, default_rs232_devices, "printer");
}


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

apple2_gameio_serial_device::apple2_gameio_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, APPLE2_GAMEIO_SERIAL, tag, owner, clock)
	, device_a2gameio_interface(mconfig, *this)
	, m_rs232(*this, "rs232")
	, m_config(*this, "CONFIG")
{
}

void apple2_gameio_serial_device::device_start()
{
}

void apple2_gameio_serial_device::an0_w(int state)
{
	// Apple II: $C058 = mark, $C059 = space
	m_rs232->write_txd(!state);
}

int apple2_gameio_serial_device::sw0_r()
{
	// 1 = ready, 0 = not ready
	switch (m_config->read())
	{
	case 0x01:
		return !m_rs232->dsr_r();

	default:
		return 1;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(APPLE2_GAMEIO_SERIAL, device_a2gameio_interface, apple2_gameio_serial_device, "a2gameio_serial", "Apple II Game I/O serial printer interface")

// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    =========== Hexbus peripheral bus ===========

    The Hexbus is a 4-bit peripheral bus with master/slave coordination. Bytes
    are written over the bus in two passes. Hexbus was the designated standard
    peripheral bus for TI computers before TI left the home computer market.

    The Hexbus was also named IPB = Intelligent Peripheral Bus [1]

    Existing devices are floppy drive, RS232 serial adapter, and
    a "Wafertape" drive (kind of tape streamer)

    Hexbus connector (console)
    +---+---+---+---+
    | 4 | 3 | 2 | 1 |      4 = L;    3 = BAV*; 2 = ADB1; 1 = ADB0
    +---+---+---+---+
    | 8 | 7 | 6 | 5 |      8 = ADB3; 7 = ADB2; 6 = nc;   5 = HSK*
    +---+---+---+---+

    ADBx = Hexbus data bit X (x=0..3)
    HSK* = Handshake
    BAV* = Bus available (0=yes)

    Theory of operation
    -------------------
    The bus connects all devices in a daisy chain. The lines are inactive high
    and pulled down by any device that sets them to 0.

    HSK* is a synchronization line for transmitting single bytes.
    BAV* is a synchronization line for complete request-response pairs

    Before a byte can be sent, the sender must check the BAV* line. When it is
    inactive (1), it pulls it down, and then starts transmitting. The line
    is raised again when the response has been fully received.

    For transmitting a byte, the HSK* line must first be pulled down by the
    sender. Then, the lower nibble is transmitted first, and the sender
    releases the HSK* line. Every receiver that has completely received the
    nibble and is ready for the next one releases the HSK* line. The sender
    must check the HSK* line until it goes high again (which means that the
    slowest receiver has got the nibble). This is repeated for the high nibble.

    When the BAV* signal is asserted (0), the HSK* line must not be high for
    more than 20 ms, or a bus timeout occurs.

    The device address is transmitted as part of the request header. All
    devices with a non-matching device code must ignore the rest of the
    message until BAV* goes inactive (1) again.

    If there is no matching device, HSK* will remain 1 after the end of the
    master's request, and after 20ms, a timeout will occur. Declaring a timeout
    is a matter of the devices, not of the bus.

    Message format
    --------------

    +-------------+-------------+-------------+-------------+
    | Device Code | Command code| LogUnitNum  |  RecNum LSB |
    +-------------+-------------+-------------+-------------+
    | RecNum MSB  | Buflen LSB  | Buflen MSB  | DataLen LSB |
    +-------------+-------------+-------------+-------------+
    | DataLen MSB |                                         |
    +-------------+                                         |
    |                        Data ...  (not padded)         |
    |                                                       |
    |                                                       |
    +-------------+-------------+-------------+-------------+

    Device codes
    ------------

    0       - all devices (but none will respond, forcing a timeout)
    1-8     - Tape mass storage
    10-17   - Printer / plotter
    20-27   - RS-232 interface
    30-37   - TV interface (color)
    40-47   - TV interface (B/W)
    50-57   - Centronics interface
    60-67   - Calculator in slave mode
    70-77   - Modem
    80-87   - GPIB interface (?)
    90-97   - Bar code reader
    100-107 - Floppy disk drive

    Usage in MAME
    -------------

    Single device usage:
    mame <driver> -hexbus <device>

    Multiple device usage:
    mame <driver> -hexbus chain -hexbus:chain:1 <device> ... -hexbus:chain:4 <device>

    We currently assume a maximum length of 4 devices which can be increased
    if desired. The chain positions should be used starting from 1 with no
    gaps, but this is not enforced.

    References
    ----------

    [1] Intelligent Peripheral Bus: Structure, Timing, and Protocol Specification
        Texas Instruments Inc., Consumer Products Group, Calculator Division,
        7/3/82, Revision 2.8

    Michael Zapf
    June 2017

*****************************************************************************/
#include "emu.h"
#include "hexbus.h"

// Devices
#include "hx5102.h"

// Hexbus instance
DEFINE_DEVICE_TYPE_NS(TI_HEXBUS, bus::ti99::hexbus, hexbus_device,  "ti_hexbus",  "Hexbus")

// Hexbus daisy chain
DEFINE_DEVICE_TYPE_NS(TI_HEXBUS_CHAIN, bus::ti99::hexbus, hexbus_chain_device, "ti_hexbus_chain", "Hexbus chain")

// Single slot of the Hexbus
DEFINE_DEVICE_TYPE_NS(TI_HEXBUS_SLOT, bus::ti99::hexbus, hexbus_slot_device, "ti_hexbus_slot", "Hexbus position")

namespace bus { namespace ti99 { namespace hexbus {

enum
{
	NOTCONN = -1
};

hexbus_device::hexbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI_HEXBUS, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_master(nullptr),
	m_slave(nullptr)
{
}

void hexbus_device::device_start()
{
}

void hexbus_device::device_config_complete()
{
	m_slave = dynamic_cast<device_ti_hexbus_interface*>(subdevices().first());
	if (m_slave != nullptr)
		m_slave->set_hexbus(this);
}

void hexbus_device::send()
{
	uint8_t sum = m_master->get_value();
	if (m_slave != nullptr)
	{
		sum &= m_slave->get_value();
		m_slave->receive(sum);
	}
	m_master->receive(sum);
}

// ------------------------------------------------------------------------

hexbus_chain_device::hexbus_chain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI_HEXBUS_CHAIN, tag, owner, clock),
	device_ti_hexbus_interface(mconfig, *this)
{
}

uint8_t hexbus_chain_device::get_value()
{
	uint8_t sum = 0xff;

	// Do the wired AND
	for (device_t &child : subdevices())
	{
		hexbus_slot_device* slot = downcast<hexbus_slot_device *>(&child);
		sum &= slot->get_value();
	}

	return sum;
}

void hexbus_chain_device::receive(uint8_t value)
{
	// Propagate
	for (device_t &child : subdevices())
	{
		hexbus_slot_device* slot = downcast<hexbus_slot_device *>(&child);
		slot->receive(value);
	}
}

void hexbus_chain_device::device_start()
{
}

SLOT_INTERFACE_START( ti_hexbus_chain_slot )
//  SLOT_INTERFACE("hx5102", TI_HX5102)  // not an option yet
SLOT_INTERFACE_END

MACHINE_CONFIG_MEMBER( hexbus_chain_device::device_add_mconfig )
	MCFG_HEXBUS_SLOT_ADD( "1", ti_hexbus_chain_slot )
	MCFG_HEXBUS_SLOT_ADD( "2", ti_hexbus_chain_slot )
	MCFG_HEXBUS_SLOT_ADD( "3", ti_hexbus_chain_slot )
	MCFG_HEXBUS_SLOT_ADD( "4", ti_hexbus_chain_slot )
MACHINE_CONFIG_END

// ------------------------------------------------------------------------

int hexbus_slot_device::get_index_from_tagname()
{
	const char *mytag = tag();
	int maxlen = strlen(mytag);
	int i;
	for (i=maxlen-1; i >=0; i--)
		if (mytag[i] < 48 || mytag[i] > 57) break;

	return atoi(mytag+i+1);
}

hexbus_slot_device::hexbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI_HEXBUS_SLOT, tag, owner, clock),
	  device_slot_interface(mconfig, *this),
	  m_hexbdev(nullptr)
{
}

/* Called from the Hexbus instance */
int hexbus_slot_device::get_index()
{
	if (m_hexbdev == nullptr) return NOTCONN;
	return get_index_from_tagname();
}

uint8_t hexbus_slot_device::get_value()
{
	return (m_hexbdev != nullptr)? m_hexbdev->get_value() : 0xff;
}

void hexbus_slot_device::receive(uint8_t value)
{
	if (m_hexbdev != nullptr)
		m_hexbdev->receive(value);
}

void hexbus_slot_device::device_start()
{
}

void hexbus_slot_device::device_config_complete()
{
	m_hexbus = dynamic_cast<hexbus_device*>(owner());
	m_hexbdev = dynamic_cast<device_ti_hexbus_interface *>(subdevices().first());
}

// ------------------------------------------------------------------------

void device_ti_hexbus_interface::interface_config_complete()
{
	m_hexbus = dynamic_cast<hexbus_device*>(device().owner());
}

void device_ti_hexbus_interface::hexbus_send(uint8_t value)
{
	m_value = value;
	m_hexbus->send();
}

// ------------------------------------------------------------------------

}   }   }  // end namespace bus::ti99::hexbus

SLOT_INTERFACE_START( ti_hexbus_conn )
//  SLOT_INTERFACE("hx5102", TI_HX5102)  // not an option yet
	SLOT_INTERFACE("chain", TI_HEXBUS_CHAIN)
SLOT_INTERFACE_END


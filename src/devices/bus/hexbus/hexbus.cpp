// license:BSD-3-Clause
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

    mame <driver> -hexbus <device>

    Usually the device offers an own hexbus socket, so we can chain devices:

    mame <driver> -hexbus <device1> -hexbus:<device1>:hexbus <device2> ...

    The direction towards the console is named "inbound" in the implementation,
    while the direction towards the end of the chain is named "outbound".

    Implementation
    --------------

    All lines (ADB0-3, HSK*, BAV*) are pull-down outputs with open collectors,
    and at the same time also inputs.

    The challenge, compared to other daisy-chained buses, is that the Hexbus
    is nondirectional. Writing on the bus is sensed by all devices in either
    direction. Also, reading from the bus is the product of all active output
    buffers. With no active device, the bus lines are pulled high.

    Every Hexbus device has an interface "device_hexbus_interface" which
    allows it to plug into a preceding Hexbus socket.

    Since the signal propagation is the same for all devices, there is a
    parent class "hexbus_chained_device" that calculates the current levels
    for all bus lines by fetching all values from the attached devices, ANDing
    them, and propagating them again. This must be done in both directions.

    Reading is simpler, because we assume that changes can only be done by
    writing to the bus.

    The "hexbus_chained_device" implements the "device_hexbus_interface"
    and holds references to up to two Hexbus instances, one for each direction.
    The computer console will not offer an inbound Hexbus connection, only
    an outbound one, and possibly there is some device that must be connected
    at the end, without further outbound connections.

    By default, instances of "hexbus_chained_device" own an outbound
    hexbus slot as a subdevice; this may be overwritten by subclasses.

        Line state received via the Hexbus
    +------+------+------+------+------+------+------+------+
    | ADB3 | ADB2 |  -   | HSK* |  0   | BAV* | ADB1 | ADB0 |
    +------+------+------+------+------+------+------+------+

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

#define LOG_WRITE          (1U<<1)   // Write operation

#define VERBOSE ( LOG_GENERAL )

#include "logmacro.h"

// Hexbus instance
DEFINE_DEVICE_TYPE_NS(HEXBUS, bus::hexbus, hexbus_device,  "hexbus",  "Hexbus connector")

namespace bus { namespace hexbus {

hexbus_device::hexbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HEXBUS, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_next_dev(nullptr)
{
}

void hexbus_device::device_start()
{
	m_next_dev = dynamic_cast<device_hexbus_interface *>(get_card_device());
}

/*
    Write to the hexbus. If the write operation comes from the plugged device,
    this is an inbound write; otherwise, the write comes from the owner of the
    hexbus connector, which means an outbound write.
*/
void hexbus_device::write(int dir, uint8_t data)
{
	if (dir == INBOUND)
		m_chain_element->bus_write(INBOUND, data);
	else
	{
		// Is there another Hexbus device?
		if (m_next_dev != nullptr)
			m_next_dev->bus_write(OUTBOUND, data);
	}
}

/*
    Read from the hexbus. If the read operation comes from the plugged device,
    this is an inbound read; otherwise, the read comes from the owner of the
    hexbus connector, which means an outbound read.
*/
uint8_t hexbus_device::read(int dir)
{
	// Default is: all lines pulled up
	uint8_t value = 0xff;

	if (dir == INBOUND)
		value = m_chain_element->bus_read(INBOUND);
	else
	{
		// Is there another Hexbus device?
		if (m_next_dev != nullptr)
			value = m_next_dev->bus_read(OUTBOUND);
	}
	return value;
}

// ------------------------------------------------------------------------

hexbus_chained_device::hexbus_chained_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, type, tag, owner, clock),
	device_hexbus_interface(mconfig, *this)
{
	m_hexbus_inbound = dynamic_cast<hexbus_device *>(owner);
	m_myvalue = 0xff;
}

void hexbus_chained_device::device_start()
{
	m_hexbus_outbound = static_cast<hexbus_device*>(subdevice("hexbus"));

	// Establish callback for inbound propagations
	m_hexbus_outbound->set_chain_element(this);
}

/*
    Called from the Hexbus user, that is, the device that subclasses
    hexbus_chained_device, like the HX5102
*/
void hexbus_chained_device::hexbus_write(uint8_t data)
{
	m_myvalue = data;

	uint8_t otherval = hexbus_get_levels();
	// LOGMASKED(LOG_WRITE, ".. otherval=%02x\n", otherval);

	// What is the new bus level?
	// The data lines are not supposed to be set by multiple devices
	// We assume that sending data overrides all data line levels.
	// This is emulated by pulling the data lines to ones.
	uint8_t newvalue = (otherval | 0xc3) & m_myvalue;

	// If it changed (with respect to HSK* or BAV*), propagate to both directions.
	if ((newvalue & (HEXBUS_LINE_HSK | HEXBUS_LINE_BAV)) != (m_current_bus_value & (HEXBUS_LINE_HSK | HEXBUS_LINE_BAV)))
	{
		LOGMASKED(LOG_WRITE, "Trying to write %02x, actually: %02x (current=%02x)\n", data, newvalue, m_current_bus_value);

		m_current_bus_value = newvalue;

		if (m_hexbus_inbound != nullptr)
			m_hexbus_inbound->write(INBOUND, m_current_bus_value);

		if (m_hexbus_outbound != nullptr)
			m_hexbus_outbound->write(OUTBOUND, m_current_bus_value);

	}
	else LOGMASKED(LOG_WRITE, "No change on hexbus\n");
}

/*
    Get levels from other devices (without changing anything).
*/
uint8_t hexbus_chained_device::hexbus_get_levels()
{
	uint8_t inbound_value = 0xff;
	uint8_t outbound_value = 0xff;

	// other devices left and right from us
	if (m_hexbus_inbound != nullptr)
		inbound_value = m_hexbus_inbound->read(INBOUND);

	if (m_hexbus_outbound != nullptr)
		outbound_value = m_hexbus_outbound->read(OUTBOUND);

	return (inbound_value & outbound_value);
}

/*
    Called from the Hexbus user, that is, the device that subclasses
    hexbus_chained_device, like the HX5102
*/
uint8_t hexbus_chained_device::hexbus_read()
{
	return m_current_bus_value;
}

/*
    Called from another hexbus device on the bus
*/
uint8_t hexbus_chained_device::bus_read(int dir)
{
	uint8_t tmpvalue = 0xff;
	hexbus_device* hexbuscont = (dir == INBOUND)? m_hexbus_inbound : m_hexbus_outbound;

	if (hexbuscont != nullptr)
		tmpvalue = hexbuscont->read(dir);

	return m_myvalue & tmpvalue;
}

/*
    Called from another hexbus device on the bus
*/
void hexbus_chained_device::bus_write(int dir, uint8_t data)
{
	hexbus_device* hexbuscont = (dir == INBOUND)? m_hexbus_inbound : m_hexbus_outbound;

	// Propagate this value first
	if (hexbuscont != nullptr)
		hexbuscont->write(dir, data);

	uint8_t oldvalue = m_current_bus_value;
	m_current_bus_value = data;

	// Notify device
	// Caution: Calling hexbus_value_changed may cause further activities that change the bus again
	// Data changes alone shall not trigger the callback
	if ((data & (HEXBUS_LINE_HSK | HEXBUS_LINE_BAV)) != (oldvalue & (HEXBUS_LINE_HSK | HEXBUS_LINE_BAV)))
	{
		LOGMASKED(LOG_WRITE, "Hexbus value changed: %02x -> %02x\n", oldvalue, data);
		hexbus_value_changed(data);
	}
}

/*
    Convenience function to calculate the bus value. Used by subclasses.
*/
uint8_t hexbus_chained_device::to_line_state(uint8_t data, bool bav, bool hsk)
{
	uint8_t lines = ((data & 0x0c)<<4) | (data & 0x03);
	if (!bav) lines |= 0x04;
	if (!hsk) lines |= 0x10;
	return lines;
}

// ------------------------------------------------------------------------

}   }   // end namespace bus::hexbus

void hexbus_options(device_slot_interface &device)
{
	device.option_add("hx5102", HX5102);
}

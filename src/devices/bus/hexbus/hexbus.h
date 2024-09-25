// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus peripheral bus
    See hexbus.cpp for documentation

    Michael Zapf
    June 2017

*****************************************************************************/

#ifndef MAME_BUS_HEXBUS_HEXBUS_H
#define MAME_BUS_HEXBUS_HEXBUS_H

#pragma once

namespace bus::hexbus {

enum
{
	INBOUND = 0,
	OUTBOUND = 1
};

/* Line */
enum
{
	HEXBUS_LINE_HSK = 0x10,
	HEXBUS_LINE_BAV = 0x04
};

class hexbus_device;
class hexbus_chained_device;

/********************************************************************
    Interface for a device that connects to the Hexbus
********************************************************************/

class device_hexbus_interface : public device_interface
{
public:
	virtual uint8_t bus_read(int dir) = 0;
	virtual void bus_write(int dir, uint8_t data) = 0;

protected:
	device_hexbus_interface(const machine_config &mconfig, device_t &device);
};

/********************************************************************
    Common parent class of all devices attached to the hexbus port
    This class implements the signal propagation in both directions
********************************************************************/
class hexbus_chained_device : public device_t, public device_hexbus_interface
{
	friend class hexbus_device;

protected:
	hexbus_chained_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void set_outbound_hexbus(hexbus_device *outbound) { m_hexbus_outbound = outbound; }

	virtual void device_resolve_objects() override ATTR_COLD;

	// Link to the inbound Hexbus (if not null, see Oso chip)
	hexbus_device *m_hexbus_inbound;

	// Link to the outbound Hexbus (if not null)
	hexbus_device *m_hexbus_outbound;

	// Common AND of all private values
	uint8_t m_current_bus_value;

	// From device_ti_hexbus_interface
	virtual uint8_t bus_read(int dir) override;
	virtual void bus_write(int dir, uint8_t data) override;

	// Methods to be used from subclasses
	// Write a byte to the Hexbus. Returns the actual line state.
	// This is important because the bus is a wired-and bus.
	void hexbus_write(uint8_t data);

	// Levels on the bus, except for this device
	uint8_t hexbus_get_levels();

	// Read hexbus
	uint8_t hexbus_read();

	// Callback for changes on the hexbus
	virtual void hexbus_value_changed(uint8_t data) { }

	// Enable or disable the Hexbus component
	bool m_enabled;
	void set_communication_enabled(bool set);

	// Levels of the lines at this device. 0 means pull down, 1 means release.
	uint8_t m_myvalue;

	// Set or release the HSK* and BAV* lines
	void set_hsk_line(line_state level);
	void set_bav_line(line_state level);

	// Set the data latch for upcoming transmission
	void set_data_latch(int value, int pos);

	// Convenience method to create a Hexbus line state
	static uint8_t to_line_state(uint8_t data, bool bav, bool hsk);

	// Latch HSK*=0
	void latch_hsk() { m_myvalue &= ~HEXBUS_LINE_HSK; }

	// Convenience function to check HSK/BAV lines on given values
	static line_state hsk_line(uint8_t lines) { return (lines & HEXBUS_LINE_HSK)? CLEAR_LINE : ASSERT_LINE; }
	static line_state bav_line(uint8_t lines) { return (lines & HEXBUS_LINE_BAV)? CLEAR_LINE : ASSERT_LINE; }

	// Return the HSK* level on the bus
	line_state bus_hsk_level() { return hsk_line(m_current_bus_value); }

	// Return the BAV* level on the bus
	line_state bus_bav_level() { return bav_line(m_current_bus_value); }

	// Return the HSK* level from this device
	line_state own_hsk_level() { return hsk_line(m_myvalue); }

	// Return the BAV* level from this device
	line_state own_bav_level() { return bav_line(m_myvalue); }

	// Data lines
	static int data_lines(uint8_t lines) { return (((lines & 0xc0) >> 4) | (lines & 0x03)); }

	// Return the selected data bit (0-3)
	int data_bit(int n);
};

// ------------------------------------------------------------------------

/********************************************************************
    Connector to the Hexbus, offers a slot for Hexbus-chained devices
********************************************************************/

class hexbus_device : public device_t, public device_single_card_slot_interface<device_hexbus_interface>
{
public:
	template <typename U>
	hexbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: hexbus_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	hexbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Used to establish the reverse link (inbound)
	void set_chain_element(hexbus_chained_device* chain) { m_chain_element = chain; }

	// Read and write operations on the bus
	uint8_t read(int dir);
	void write(int dir, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	device_hexbus_interface *m_next_dev;

private:
	// owner of this Hexbus socket; may be the owning component or another
	// component in the device hierarchy (see TI-99/8 where it belongs to Oso,
	// but the Hexbus is a subdevice of the driver itself)
	hexbus_chained_device*  m_chain_element;
};

}   // end namespace bus::hexbus

DECLARE_DEVICE_TYPE_NS(HEXBUS, bus::hexbus, hexbus_device)

void hexbus_options(device_slot_interface &device);

#endif // MAME_BUS_HEXBUS_HEXBUS_H

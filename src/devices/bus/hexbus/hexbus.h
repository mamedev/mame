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

namespace bus { namespace hexbus {

enum
{
	INBOUND = 0,
	OUTBOUND = 1
};

/* Line */
enum
{
	HEXBUS_LINE_HSK = 0x10,
	HEXBUS_LINE_BAV = 0x04,
	HEXBUS_LINE_BIT32 = 0xc0,
	HEXBUS_LINE_BIT10 = 0x03
};

class hexbus_device;
class hexbus_chained_device;

/********************************************************************
    Interface for a device that connects to the Hexbus
********************************************************************/

class device_hexbus_interface : public device_slot_card_interface
{
public:
	virtual uint8_t bus_read(int dir) =0;
	virtual void bus_write(int dir, uint8_t data) =0;

protected:
	device_hexbus_interface(const machine_config &mconfig, device_t &device) :
		device_slot_card_interface(mconfig, device) { }
};

/********************************************************************
    Common parent class of all devices attached to the hexbus port
    This class implements the signal propagation in both directions
********************************************************************/
class hexbus_chained_device : public device_t, public device_hexbus_interface
{
	friend class hexbus_device;

public:
	hexbus_chained_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override;

protected:
	void set_outbound_hexbus(hexbus_device *outbound) { m_hexbus_outbound = outbound; }

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
	virtual void hexbus_value_changed(uint8_t data) { };

	// Levels of the lines at this device. 0 means pull down, 1 means release.
	uint8_t m_myvalue;

	// Utility method to create a Hexbus line state
	uint8_t to_line_state(uint8_t data, bool bav, bool hsk);
};

// ------------------------------------------------------------------------

/********************************************************************
    Connector to the Hexbus, offers a slot for Hexbus-chained devices
********************************************************************/

class hexbus_device : public device_t, public device_slot_interface
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
	void device_start() override;
	device_hexbus_interface *m_next_dev;

private:
	// owner of this Hexbus socket; may be the owning component or another
	// component in the device hierarchy (see TI-99/8 where it belongs to Oso,
	// but the Hexbus is a subdevice of the driver itself)
	hexbus_chained_device*  m_chain_element;
};

}   }   // end namespace bus::hexbus

DECLARE_DEVICE_TYPE_NS(HEXBUS, bus::hexbus, hexbus_device)

void hexbus_options(device_slot_interface &device);

#endif // MAME_BUS_HEXBUS_HEXBUS_H

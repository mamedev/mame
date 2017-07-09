// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus peripheral bus
    See hexbus.cpp for documentation

    Michael Zapf
    June 2017

*****************************************************************************/

#ifndef MAME_BUS_TI99_HEXBUS_HEXBUS_H
#define MAME_BUS_TI99_HEXBUS_HEXBUS_H

#pragma once

#include "bus/ti99/ti99defs.h"

namespace bus { namespace ti99 { namespace hexbus {

enum
{
	INBOUND = 0,
	OUTBOUND = 1
};

class hexbus_device;
class hexbus_chained_device;

/********************************************************************
    Interface for a device that connects to the Hexbus
********************************************************************/

class device_ti_hexbus_interface : public device_slot_card_interface
{
public:
	virtual uint8_t bus_read(int dir) =0;
	virtual void bus_write(int dir, uint8_t data) =0;

protected:
	device_ti_hexbus_interface(const machine_config &mconfig, device_t &device) :
		device_slot_card_interface(mconfig, device) { }
};

/********************************************************************
    Common parent class of all devices attached to the hexbus port
    This class implements the signal propagation in both directions
********************************************************************/
class hexbus_chained_device : public device_t, public device_ti_hexbus_interface
{
	friend class hexbus_device;

public:
	hexbus_chained_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_start() override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

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
	void hexbus_write(uint8_t data);
	uint8_t hexbus_read();

	// For interrupts
	virtual void hexbus_value_changed(uint8_t data) { };

private:
	uint8_t m_myvalue;
};

// ------------------------------------------------------------------------

/********************************************************************
    Connector to the Hexbus, offers a slot for Hexbus-chained devices
********************************************************************/

class hexbus_device : public device_t, public device_slot_interface
{
public:
	hexbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Used to establish the reverse link (inbound)
	void set_chain_element(hexbus_chained_device* chain) { m_chain_element = chain; }

	// Read and write operations on the bus
	uint8_t read(int dir);
	void write(int dir, uint8_t data);

protected:
	void device_start() override;
	device_ti_hexbus_interface *m_next_dev;

private:
	// owner of this Hexbus socket; may be the owning component or another
	// component in the device hierarchy (see TI-99/8 where it belongs to Oso,
	// but the Hexbus is a subdevice of the driver itself)
	hexbus_chained_device*  m_chain_element;
};

#define MCFG_HEXBUS_ADD( _tag )  \
	MCFG_DEVICE_ADD(_tag, TI_HEXBUS, 0) \
	MCFG_DEVICE_SLOT_INTERFACE( ti_hexbus_conn, nullptr, false)

}   }   }  // end namespace bus::ti99::hexbus

SLOT_INTERFACE_EXTERN( ti_hexbus_conn );

DECLARE_DEVICE_TYPE_NS(TI_HEXBUS, bus::ti99::hexbus, hexbus_device)

#endif // MAME_BUS_TI99_HEXBUS_HEXBUS_H

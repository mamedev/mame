// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus peripheral bus
    See hexbus.cpp for documentation

    Michael Zapf
    June 2017

    -hexbus:1 hx5102 -hexbus:2 ...

*****************************************************************************/

#ifndef MAME_BUS_TI99_HEXBUS_HEXBUS_H
#define MAME_BUS_TI99_HEXBUS_HEXBUS_H

#pragma once

#include "bus/ti99/ti99defs.h"

namespace bus { namespace ti99 { namespace hexbus {

enum
{
	MAX_DEVICES = 4
};

class hexbus_attached_device;
class hexbus_slot_device;

class hexbus_device : public device_t, public device_slot_interface
{
	friend class hexbus_attached_device;

public:
	hexbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void connect_master(hexbus_attached_device *masterdev) { m_master = masterdev; }

protected:
	void device_start() override;
	void device_config_complete() override;

private:
	hexbus_attached_device *m_master;
	hexbus_attached_device *m_slave;

	// Called from a slot, samples all values from the devices, and propagates
	// the logical product to all connected devices
	void send();
};

/*****************************************************************************
    The parent class for all Hexbus devices
******************************************************************************/

class hexbus_attached_device : public device_t, public device_slot_card_interface
{
	friend class hexbus_slot_device;
	friend class hexbus_device;

protected:
	hexbus_attached_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	void device_start() override;

	void hexbus_send(uint8_t value);
	uint8_t hexbus_receive() { return m_busvalue; }

	virtual void receive(uint8_t value) { m_busvalue = value; }
	virtual uint8_t get_value() { return m_value; }

private:
	void set_hexbus(hexbus_device* hexbus) { m_hexbus = hexbus; }

	uint8_t m_value;
	uint8_t m_busvalue;
	hexbus_device *m_hexbus;        // Link to the Hexbus
};

// ------------------------------------------------------------------------

class hexbus_chain_device : public hexbus_attached_device
{
public:
	hexbus_chain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void device_add_mconfig(machine_config &config) override;

private:
	void device_start() override;
	void receive(uint8_t value) override;
	uint8_t get_value() override;
};

// ------------------------------------------------------------------------

class hexbus_slot_device : public device_t, public device_slot_interface
{
	friend class hexbus_attached_device;
	friend class hexbus_chain_device;

public:
	hexbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Called from the hexbus (direction to attached device)
	void receive(uint8_t value);

	// Called from the hexbus
	uint8_t get_value();

protected:
	void device_start() override;
	void device_config_complete() override;

	// Called from the Hexbus instance
	int get_index();

private:
	int get_index_from_tagname();

	hexbus_attached_device* m_hexbdev;
	hexbus_device* m_hexbus;
};


#define MCFG_HEXBUS_ADD( _tag )  \
	MCFG_DEVICE_ADD(_tag, TI_HEXBUS, 0) \
	MCFG_DEVICE_SLOT_INTERFACE( ti_hexbus_conn, nullptr, false)

#define MCFG_HEXBUS_SLOT_ADD(_tag, _slot_intf) \
	MCFG_DEVICE_ADD(_tag, TI_HEXBUS_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, nullptr, false)

}   }   }  // end namespace bus::ti99::hexbus

SLOT_INTERFACE_EXTERN( ti_hexbus_conn );

DECLARE_DEVICE_TYPE_NS(TI_HEXBUS,       bus::ti99::hexbus, hexbus_device)
DECLARE_DEVICE_TYPE_NS(TI_HEXBUS_CHAIN, bus::ti99::hexbus, hexbus_chain_device)
DECLARE_DEVICE_TYPE_NS(TI_HEXBUS_SLOT,  bus::ti99::hexbus, hexbus_slot_device)

#endif // MAME_BUS_TI99_HEXBUS_HEXBUS_H

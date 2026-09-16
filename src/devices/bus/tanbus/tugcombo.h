// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    TUG Combo Card / TUG EPROM Storage Card

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TUGCOMBO_H
#define MAME_BUS_TANBUS_TUGCOMBO_H

#pragma once

#include "tanbus.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tugcombo_device
	: public device_t
	, public device_memory_interface
	, public device_tanbus_interface
{
protected:
	tanbus_tugcombo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

	address_space_config m_space_config;
	address_space *m_space;

private:
	required_ioport m_links;
	required_device_array<pia6821_device, 2> m_pia;

	uint16_t m_addr;
};


// ======================> tanbus_tugcombo2716_device

class tanbus_tugcombo2716_device : public tanbus_tugcombo_device
{
public:
	tanbus_tugcombo2716_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	tanbus_tugcombo2716_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<generic_slot_device, 16> m_rom;

	void mem_map(address_map &map) ATTR_COLD;
};


// ======================> tanbus_tugcombo2732_device

class tanbus_tugcombo2732_device : public tanbus_tugcombo_device
{
public:
	tanbus_tugcombo2732_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	tanbus_tugcombo2732_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device_array<generic_slot_device, 16> m_rom;

	void mem_map(address_map &map) ATTR_COLD;
};


// ======================> tanbus_tugcombo6116_device

class tanbus_tugcombo6116_device : public tanbus_tugcombo_device
{
public:
	tanbus_tugcombo6116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	tanbus_tugcombo6116_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};


// ======================> tanbus_tugesc2716_device

class tanbus_tugesc2716_device : public tanbus_tugcombo2716_device
{
public:
	tanbus_tugesc2716_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> tanbus_tugesc2732_device

class tanbus_tugesc2732_device : public tanbus_tugcombo2732_device
{
public:
	tanbus_tugesc2732_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TUGCOMBO2716, tanbus_tugcombo2716_device)
DECLARE_DEVICE_TYPE(TANBUS_TUGCOMBO2732, tanbus_tugcombo2732_device)
DECLARE_DEVICE_TYPE(TANBUS_TUGCOMBO6116, tanbus_tugcombo6116_device)
DECLARE_DEVICE_TYPE(TANBUS_TUGESC2716, tanbus_tugesc2716_device)
DECLARE_DEVICE_TYPE(TANBUS_TUGESC2732, tanbus_tugesc2732_device)


#endif // MAME_BUS_TANBUS_TUGCOMBO_H

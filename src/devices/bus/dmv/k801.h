// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#ifndef MAME_BUS_DMV_K801_H
#define MAME_BUS_DMV_K801_H

#pragma once

#include "dmvbus.h"
#include "machine/mc2661.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k801_device

class dmv_k801_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dmv_k801_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// dmvcart_interface overrides
	virtual void io_read(int ifsel, offs_t offset, uint8_t &data) override;
	virtual void io_write(int ifsel, offs_t offset, uint8_t data) override;

	required_device<mc2661_device> m_epci;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_dsw;

private:
	DECLARE_WRITE_LINE_MEMBER(epci_irq_w);

	dmvcart_slot_device * m_bus;
};


// ======================> dmv_k211_device

class dmv_k211_device :
		public dmv_k801_device
{
public:
	// construction/destruction
	dmv_k211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dmv_k211_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// dmvcart_interface overrides
	virtual void io_read(int ifsel, offs_t offset, uint8_t &data) override;
	virtual void io_write(int ifsel, offs_t offset, uint8_t data) override;
};

// ======================> dmv_k212_device

class dmv_k212_device :
		public dmv_k211_device
{
public:
	// construction/destruction
	dmv_k212_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

// ======================> dmv_k213_device

class dmv_k213_device :
		public dmv_k211_device
{
public:
	// construction/destruction
	dmv_k213_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K801, dmv_k801_device)
DECLARE_DEVICE_TYPE(DMV_K211, dmv_k211_device)
DECLARE_DEVICE_TYPE(DMV_K212, dmv_k212_device)
DECLARE_DEVICE_TYPE(DMV_K213, dmv_k213_device)

#endif  // MAME_BUS_DMV_K801_H

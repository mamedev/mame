// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K801_H__
#define __DMV_K801_H__

#include "emu.h"
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
	dmv_k801_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	dmv_k801_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER(epci_irq_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// dmvcart_interface overrides
	virtual void io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data) override;
	virtual void io_write(address_space &space, int ifsel, offs_t offset, UINT8 data) override;

protected:
	required_device<mc2661_device> m_epci;
	required_ioport m_dsw;
	dmvcart_slot_device * m_bus;
};


// ======================> dmv_k211_device

class dmv_k211_device :
		public dmv_k801_device
{
public:
	// construction/destruction
	dmv_k211_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	dmv_k211_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// dmvcart_interface overrides
	virtual void io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data) override;
	virtual void io_write(address_space &space, int ifsel, offs_t offset, UINT8 data) override;
};

// ======================> dmv_k212_device

class dmv_k212_device :
		public dmv_k211_device
{
public:
	// construction/destruction
	dmv_k212_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};

// ======================> dmv_k213_device

class dmv_k213_device :
		public dmv_k211_device
{
public:
	// construction/destruction
	dmv_k213_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// device type definition
extern const device_type DMV_K801;
extern const device_type DMV_K211;
extern const device_type DMV_K212;
extern const device_type DMV_K213;

#endif  /* __DMV_K801_H__ */

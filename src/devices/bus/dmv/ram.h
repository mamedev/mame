// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
#ifndef MAME_BUS_DMV_RAM_H
#define MAME_BUS_DMV_RAM_H

#pragma once

#include "dmvbus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================>

class dmv_ram_device_base :
		public device_t,
		public device_dmvslot_interface
{
protected:
	// construction/destruction
	dmv_ram_device_base(const machine_config &mconfig, device_type type, uint32_t size, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// dmvcart_interface overrides
	virtual void ram_read(uint8_t cas, offs_t offset, uint8_t &data) override;
	virtual void ram_write(uint8_t cas, offs_t offset, uint8_t data) override;

private:
	uint8_t *     m_ram;
	uint8_t       m_size;
};


class dmv_k200_device : public dmv_ram_device_base
{
public:
	// construction/destruction
	dmv_k200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class dmv_k202_device : public dmv_ram_device_base
{
public:
	// construction/destruction
	dmv_k202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class dmv_k208_device : public dmv_ram_device_base
{
public:
	// construction/destruction
	dmv_k208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K200, dmv_k200_device)
DECLARE_DEVICE_TYPE(DMV_K202, dmv_k202_device)
DECLARE_DEVICE_TYPE(DMV_K208, dmv_k208_device)

#endif // MAME_BUS_DMV_RAM_H

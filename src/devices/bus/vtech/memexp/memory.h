// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansions

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_MEMORY_H
#define MAME_BUS_VTECH_MEMEXP_MEMORY_H

#pragma once

#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_laser110_16k_device

class vtech_laser110_16k_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_laser110_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
};

// ======================> vtech_laser210_16k_device

class vtech_laser210_16k_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_laser210_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
};

// ======================> vtech_laser310_16k_device

class vtech_laser310_16k_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_laser310_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
};

// ======================> vtech_laser_64k_device

class vtech_laser_64k_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_laser_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_memory_bank m_fixed_bank;
	required_memory_bank m_bank;

	std::unique_ptr<uint8_t[]> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_LASER110_16K, vtech_laser110_16k_device)
DECLARE_DEVICE_TYPE(VTECH_LASER210_16K, vtech_laser210_16k_device)
DECLARE_DEVICE_TYPE(VTECH_LASER310_16K, vtech_laser310_16k_device)
DECLARE_DEVICE_TYPE(VTECH_LASER_64K,    vtech_laser_64k_device)

#endif // MAME_BUS_VTECH_MEMEXP_MEMORY_H

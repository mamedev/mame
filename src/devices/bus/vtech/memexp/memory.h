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

class vtech_laser110_16k_device : public device_t, public device_vtech_memexp_interface
{
public:
	// construction/destruction
	vtech_laser110_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<uint8_t> m_ram;
};

// ======================> vtech_laser210_16k_device

class vtech_laser210_16k_device : public device_t, public device_vtech_memexp_interface
{
public:
	// construction/destruction
	vtech_laser210_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<uint8_t> m_ram;
};

// ======================> vtech_laser310_16k_device

class vtech_laser310_16k_device : public device_t, public device_vtech_memexp_interface
{
public:
	// construction/destruction
	vtech_laser310_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<uint8_t> m_ram;
};

// ======================> vtech_laser_64k_device

class vtech_laser_64k_device : public device_t, public device_vtech_memexp_interface
{
public:
	// construction/destruction
	vtech_laser_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void bankswitch_w(uint8_t data);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	memory_bank_creator m_bank;
	std::vector<uint8_t> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_LASER110_16K, vtech_laser110_16k_device)
DECLARE_DEVICE_TYPE(VTECH_LASER210_16K, vtech_laser210_16k_device)
DECLARE_DEVICE_TYPE(VTECH_LASER310_16K, vtech_laser310_16k_device)
DECLARE_DEVICE_TYPE(VTECH_LASER_64K,    vtech_laser_64k_device)

#endif // MAME_BUS_VTECH_MEMEXP_MEMORY_H

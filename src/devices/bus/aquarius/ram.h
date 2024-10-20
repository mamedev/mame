// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius Memory Cartridges

**********************************************************************/

#ifndef MAME_BUS_AQUARIUS_RAM_H
#define MAME_BUS_AQUARIUS_RAM_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aquarius_ram_device

class aquarius_ram_device :
	public device_t,
	public device_aquarius_cartridge_interface
{
protected:
	// construction/destruction
	aquarius_ram_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, uint16_t size);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;

private:
	uint16_t m_ram_size;

	std::unique_ptr<uint8_t[]> m_ram;
};

// ======================> aquarius_ram4_device

class aquarius_ram4_device : public aquarius_ram_device
{
public:
	// construction/destruction
	aquarius_ram4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> aquarius_ram16_device

class aquarius_ram16_device : public aquarius_ram_device
{
public:
	// construction/destruction
	aquarius_ram16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> aquarius_ram32_device

class aquarius_ram32_device : public aquarius_ram_device
{
public:
	// construction/destruction
	aquarius_ram32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> aquarius_ram16p_device

class aquarius_ram16p_device :
	public device_t,
	public device_aquarius_cartridge_interface
{
public:
	// construction/destruction
	aquarius_ram16p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_aquarius_cartridge_interface overrides
	virtual uint8_t mreq_ce_r(offs_t offset) override;
	virtual void mreq_ce_w(offs_t offset, uint8_t data) override;

private:
	std::unique_ptr<uint8_t[]> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_RAM4, aquarius_ram4_device)
DECLARE_DEVICE_TYPE(AQUARIUS_RAM16, aquarius_ram16_device)
DECLARE_DEVICE_TYPE(AQUARIUS_RAM32, aquarius_ram32_device)
DECLARE_DEVICE_TYPE(AQUARIUS_RAM16P, aquarius_ram16p_device)


#endif // MAME_BUS_AQUARIUS_RAM_H

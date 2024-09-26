// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Ralph Allen 32K Dynamic RAM Board

**********************************************************************/


#ifndef MAME_BUS_TANBUS_RA32K_H
#define MAME_BUS_TANBUS_RA32K_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "bus/tanbus/ra32k.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_ra32k_device : public device_t, public device_tanbus_interface
{
protected:
	tanbus_ra32k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_ioport_array<3> m_dsw;
	required_ioport m_link;

	bool block_enabled(offs_t offset, int inhrom, int inhram, int be);
};


// ======================> tanbus_ra32kram_device

class tanbus_ra32kram_device : public tanbus_ra32k_device
{
public:
	// construction/destruction
	tanbus_ra32kram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	std::unique_ptr<uint8_t[]> m_ram;
};


// ======================> tanbus_ra32krom_device

class tanbus_ra32krom_device : public tanbus_ra32k_device
{
public:
	// construction/destruction
	tanbus_ra32krom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;

private:
	required_region_ptr<uint8_t> m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_RA32KRAM, tanbus_ra32kram_device)
DECLARE_DEVICE_TYPE(TANBUS_RA32KROM, tanbus_ra32krom_device)


#endif // MAME_BUS_TANBUS_RA32K_H

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

class tanbus_ra32k_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_ra32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	required_memory_region m_rom;
	required_ioport_array<3> m_dsw;
	required_ioport m_link;

	offs_t m_addr_start;
	offs_t m_addr_end;
	std::unique_ptr<uint8_t[]> m_ram;

	bool block_enabled(offs_t offset, int inhrom, int inhram, int be);
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_RA32K, tanbus_ra32k_device)


#endif // MAME_BUS_TANBUS_RA32K_H

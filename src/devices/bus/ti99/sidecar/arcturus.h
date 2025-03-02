// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Arcturus sidecar cartridge
    Michael Zapf

*****************************************************************************/

#ifndef MAME_BUS_TI99_SIDECAR_ARCTURUS_H
#define MAME_BUS_TI99_SIDECAR_ARCTURUS_H

#pragma once

#include "bus/ti99/internal/ioport.h"
#include "machine/ram.h"

namespace bus::ti99::sidecar {

/*****************************************************************************
    Arcturus sidecar cartridge
******************************************************************************/

class arcturus_device : public bus::ti99::internal::ioport_attached_device
{
public:
	arcturus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void readz(offs_t offset, uint8_t *value) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// ROMs
	uint8_t* m_rom4;
	uint8_t* m_roma;
	uint8_t* m_romc;
	
	required_device<ram_device> m_ram;
};

} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_ARCTURUS, bus::ti99::sidecar, arcturus_device)

#endif // MAME_BUS_TI99_SIDECAR_ARCTURUS_H

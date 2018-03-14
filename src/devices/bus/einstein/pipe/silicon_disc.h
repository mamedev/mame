// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Silicon Disc

    A 256k RAM drive that gets accessed as drive 3.

***************************************************************************/

#ifndef MAME_BUS_EINSTEIN_SILICON_DISC_H
#define MAME_BUS_EINSTEIN_SILICON_DISC_H

#pragma once

#include "pipe.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> einstein_silicon_disc_device

class einstein_silicon_disc_device : public device_t, public device_tatung_pipe_interface
{
public:
	// construction/destruction
	einstein_silicon_disc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void map(address_map &map);

	required_memory_region m_rom;
	required_memory_region m_bios;

	std::unique_ptr<uint8_t[]> m_ram;
	uint16_t m_sector;

	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_WRITE8_MEMBER(sector_low_w);
	DECLARE_WRITE8_MEMBER(sector_high_w);
};

// device type definition
DECLARE_DEVICE_TYPE(EINSTEIN_SILICON_DISC, einstein_silicon_disc_device)

#endif // MAME_BUS_EINSTEIN_SILICON_DISC_H

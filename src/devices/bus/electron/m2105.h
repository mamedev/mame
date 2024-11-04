// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BT Merlin M2105

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_M2105_M
#define MAME_BUS_ELECTRON_M2105_M

#pragma once

#include "exp.h"
#include "machine/6522via.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "machine/input_merger.h"
#include "machine/tms6100.h"
#include "sound/tms5220.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_m2105_device:
	public device_t,
	public device_electron_expansion_interface

{
public:
	// construction/destruction
	electron_m2105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_exp_rom;
	required_device<nvram_device> m_nvram;
	required_device_array<via6522_device, 2> m_via;
	required_device<scn2681_device> m_duart;

	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_ram_page;
	uint8_t m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_M2105, electron_m2105_device)


#endif // MAME_BUS_ELECTRON_M2105_M

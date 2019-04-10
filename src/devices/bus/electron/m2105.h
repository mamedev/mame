// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BT Merlin M2105

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_M2105_M
#define MAME_BUS_ELECTRON_M2105_M

#pragma once

#include "exp.h"
#include "machine/ram.h"
#include "machine/6522via.h"
#include "machine/mc68681.h"
#include "machine/input_merger.h"
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
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_exp_rom;
	required_device<ram_device> m_ram;
	required_device<via6522_device> m_via6522_0;
	required_device<via6522_device> m_via6522_1;
	required_device<scn2681_device> m_duart;
	required_device<tms5220_device> m_tms;
	required_device<centronics_device> m_centronics;
	required_device<input_merger_device> m_irqs;

	uint8_t m_ram_page;
	uint8_t m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_M2105, electron_m2105_device)


#endif // MAME_BUS_ELECTRON_M2105_M

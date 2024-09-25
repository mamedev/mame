// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Thermal Printer Card emulation

**********************************************************************/

#ifndef MAME_BUS_COMX35_THERMAL_H
#define MAME_BUS_COMX35_THERMAL_H

#pragma once

#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_thm_device

class comx_thm_device : public device_t,
						public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_thm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_comx_expansion_card_interface overrides
	virtual uint8_t comx_mrd_r(offs_t offset, int *extrom) override;
	virtual uint8_t comx_io_r(offs_t offset) override;
	virtual void comx_io_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(COMX_THM, comx_thm_device)


#endif // MAME_BUS_COMX35_THERMAL_H

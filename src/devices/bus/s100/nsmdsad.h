// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    North Star MICRO-DISK System MDS-A-D (Double Density) emulation

**********************************************************************/

#ifndef MAME_BUS_S100_NSMDSAD_H
#define MAME_BUS_S100_NSMDSAD_H

#pragma once

#include "s100.h"
#include "imagedev/floppy.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_mds_ad_device

class s100_mds_ad_device : public device_t,
							public device_s100_card_interface
{
public:
	// construction/destruction
	s100_mds_ad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual uint8_t s100_smemr_r(offs_t offset) override;

private:
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_dsel_rom;
	required_memory_region m_dpgm_rom;
	required_memory_region m_dwe_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(S100_MDS_AD, s100_mds_ad_device)

#endif // MAME_BUS_S100_NSMDSAD_H

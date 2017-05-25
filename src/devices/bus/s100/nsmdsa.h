// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    North Star MICRO-DISK System MDS-A (Single Density) emulation

**********************************************************************/

#ifndef MAME_BUS_S100_NSMDSA_H
#define MAME_BUS_S100_NSMDSA_H

#pragma once

#include "s100.h"
#include "imagedev/floppy.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_mds_a_device

class s100_mds_a_device : public device_t,
							public device_s100_card_interface
{
public:
	// construction/destruction
	s100_mds_a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_s100_card_interface overrides
	virtual uint8_t s100_smemr_r(address_space &space, offs_t offset) override;

private:
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_psel_rom;
	required_memory_region m_pgm_rom;
};


// device type definition
DECLARE_DEVICE_TYPE(S100_MDS_A, s100_mds_a_device)

#endif // MAME_BUS_S100_NSMDSA_H

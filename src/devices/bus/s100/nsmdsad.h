// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    North Star MICRO-DISK System MDS-A-D (Double Density) emulation

**********************************************************************/

#pragma once

#ifndef __S100_MDS_AD__
#define __S100_MDS_AD__

#include "emu.h"
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
	s100_mds_ad_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_s100_card_interface overrides
	virtual UINT8 s100_smemr_r(address_space &space, offs_t offset);

private:
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_dsel_rom;
	required_memory_region m_dpgm_rom;
	required_memory_region m_dwe_rom;
};


// device type definition
extern const device_type S100_MDS_AD;



#endif

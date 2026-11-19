// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TeleNova Compis Winchester Disk Controller emulation

**********************************************************************/

#ifndef MAME_BUS_ISBX_COMPIS_HDC_H
#define MAME_BUS_ISBX_COMPIS_HDC_H

#pragma once

#include "isbx.h"
#include "machine/wd1002_hd0.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_hdc_device

class compis_hdc_device : public device_t,
							public device_isbx_card_interface
{
public:
	// construction/destruction
	compis_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_isbx_card_interface implementation
	virtual uint8_t mcs0_r(offs_t offset) override;
	virtual void mcs0_w(offs_t offset, uint8_t data) override;

private:
	required_device<wd1002_hd0_device> m_hdc;
};


// device type definition
DECLARE_DEVICE_TYPE(COMPIS_HDC, compis_hdc_device)


#endif // MAME_BUS_ISBX_COMPIS_HDC_H

// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    TIKI-100 Winchester controller card emulation

**********************************************************************/

#pragma once

#ifndef __TIKI100_HDC__
#define __TIKI100_HDC__

#include "emu.h"
#include "bus/tiki100/exp.h"
#include "imagedev/harddriv.h"
#include "machine/wd2010.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiki100_hdc_t

class tiki100_hdc_t : public device_t,
						public device_tiki100bus_card_interface
{
public:
	// construction/destruction
	tiki100_hdc_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_tiki100bus_card_interface overrides
	virtual UINT8 iorq_r(address_space &space, offs_t offset, UINT8 data) override;
	virtual void iorq_w(address_space &space, offs_t offset, UINT8 data) override;

private:
	required_device<wd2010_device> m_hdc;
};


// device type definition
extern const device_type TIKI100_HDC;


#endif

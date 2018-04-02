// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BeebSID emulation

**********************************************************************/

#ifndef MAME_BUS_BBC_1MHZBUS_BEEBSID_H
#define MAME_BUS_BBC_1MHZBUS_BEEBSID_H

#include "1mhzbus.h"
#include "sound/mos6581.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_beebsid_device

class bbc_beebsid_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_beebsid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<mos8580_device> m_sid;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_BEEBSID, bbc_beebsid_device)


#endif // MAME_BUS_BBC_1MHZBUS_BEEBSID_H

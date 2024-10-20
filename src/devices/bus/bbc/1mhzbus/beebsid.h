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
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_device<mos8580_device> m_sid;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_BEEBSID, bbc_beebsid_device)


#endif // MAME_BUS_BBC_1MHZBUS_BEEBSID_H

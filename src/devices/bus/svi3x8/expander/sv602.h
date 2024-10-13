// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-602 Single Slot Expander for SVI-318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_EXPANDER_SV602_H
#define MAME_BUS_SVI3X8_EXPANDER_SV602_H

#pragma once

#include "expander.h"
#include "bus/svi3x8/slot/slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv602_device

class sv602_device : public device_t, public device_svi_expander_interface
{
public:
	// construction/destruction
	sv602_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

	virtual void bk21_w(int state) override;
	virtual void bk22_w(int state) override;
	virtual void bk31_w(int state) override;
	virtual void bk32_w(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// from slots
	void int_w(int state);
	void romdis_w(int state);
	void ramdis_w(int state);

	required_device<svi_slot_bus_device> m_slotbus;
};

// device type definition
DECLARE_DEVICE_TYPE(SV602, sv602_device)

#endif // MAME_BUS_SVI3X8_EXPANDER_SV602_H

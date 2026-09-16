// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-601 Super Expander for SVI-318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_EXPANDER_SV601_H
#define MAME_BUS_SVI3X8_EXPANDER_SV601_H

#pragma once

#include "expander.h"
#include "bus/svi3x8/slot/slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv601_device

class sv601_device : public device_t, public device_svi_expander_interface
{
public:
	// construction/destruction
	sv601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from slots
	void int_w(int state);
	void romdis_w(int state);
	void ramdis_w(int state);

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
	required_device<svi_slot_bus_device> m_slotbus;
};

// device type definition
DECLARE_DEVICE_TYPE(SV601, sv601_device)

#endif // MAME_BUS_SVI3X8_EXPANDER_SV601_H

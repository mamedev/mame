// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-601 Super Expander for SVI-318/328

***************************************************************************/

#pragma once

#ifndef __SVI3X8_EXPANDER_SV601_H__
#define __SVI3X8_EXPANDER_SV601_H__

#include "emu.h"
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
	virtual uint8_t mreq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void mreq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	virtual void bk21_w(int state) override;
	virtual void bk22_w(int state) override;
	virtual void bk31_w(int state) override;
	virtual void bk32_w(int state) override;

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
	required_device<svi_slot_bus_device> m_slotbus;
};

// device type definition
extern const device_type SV601;

#endif // __SVI3X8_EXPANDER_SV601_H__

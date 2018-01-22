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
	WRITE_LINE_MEMBER( int_w );
	WRITE_LINE_MEMBER( romdis_w );
	WRITE_LINE_MEMBER( ramdis_w );

	// from host
	virtual DECLARE_READ8_MEMBER( mreq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( mreq_w ) override;
	virtual DECLARE_READ8_MEMBER( iorq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( iorq_w ) override;

	virtual void bk21_w(int state) override;
	virtual void bk22_w(int state) override;
	virtual void bk31_w(int state) override;
	virtual void bk32_w(int state) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device<svi_slot_bus_device> m_slotbus;
};

// device type definition
DECLARE_DEVICE_TYPE(SV601, sv601_device)

#endif // MAME_BUS_SVI3X8_EXPANDER_SV601_H

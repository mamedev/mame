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
	sv601_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
	required_device<svi_slot_bus_device> m_slotbus;
};

// device type definition
extern const device_type SV601;

#endif // __SVI3X8_EXPANDER_SV601_H__

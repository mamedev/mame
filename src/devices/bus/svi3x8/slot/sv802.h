// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-802 Centronics Printer Interface for SVI-318/328

***************************************************************************/

#pragma once

#ifndef __SVI3X8_SLOT_SV802_H__
#define __SVI3X8_SLOT_SV802_H__

#include "emu.h"
#include "slot.h"
#include "machine/buffer.h"
#include "bus/centronics/ctronics.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv802_device

class sv802_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER( iorq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( iorq_w ) override;

	DECLARE_WRITE_LINE_MEMBER( busy_w );

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

private:
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	int m_busy;
};

// device type definition
extern const device_type SV802;

#endif // __SVI3X8_SLOT_SV802_H__

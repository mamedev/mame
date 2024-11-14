// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-802 Centronics Printer Interface for SVI-318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SV802_H
#define MAME_BUS_SVI3X8_SLOT_SV802_H

#pragma once

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
	sv802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void busy_w(int state);

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	int m_busy;
};

// device type definition
DECLARE_DEVICE_TYPE(SV802, sv802_device)

#endif // MAME_BUS_SVI3X8_SLOT_SV802_H

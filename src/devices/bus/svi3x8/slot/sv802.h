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
	sv802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t iorq_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void iorq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

	void busy_w(int state);

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

// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-803 16k memory expansion for SVI-318

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SV803_H
#define MAME_BUS_SVI3X8_SLOT_SV803_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv803_device

class sv803_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv803_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER( mreq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( mreq_w ) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::unique_ptr<uint8_t[]> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(SV803, sv803_device)

#endif // MAME_BUS_SVI3X8_SLOT_SV803_H

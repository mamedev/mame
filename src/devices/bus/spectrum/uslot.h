// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Currah MicroSlot

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_USLOT_H
#define MAME_BUS_SPECTRUM_USLOT_H

#pragma once


#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_uslot_device

class spectrum_uslot_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_uslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual DECLARE_READ8_MEMBER(mreq_r) override;
	virtual DECLARE_WRITE8_MEMBER(mreq_w) override;
	virtual DECLARE_READ8_MEMBER(port_fe_r) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

private:
	required_device<spectrum_expansion_slot_device> m_exp1;
	required_device<spectrum_expansion_slot_device> m_exp2;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_USLOT, spectrum_uslot_device)


#endif // MAME_BUS_SPECTRUM_USLOT_H

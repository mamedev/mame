// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Didaktik Melodik

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_MELODIK_H
#define MAME_BUS_SPECTRUM_MELODIK_H

#pragma once


#include "exp.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_melodik_device

class spectrum_melodik_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_melodik_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	required_device<spectrum_expansion_slot_device> m_exp;
	required_device<ay8910_device> m_psg;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MELODIK, spectrum_melodik_device)


#endif // MAME_BUS_SPECTRUM_MELODIK_H

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual void post_opcode_fetch(offs_t offset) override;
	virtual void pre_data_fetch(offs_t offset) override;
	virtual void post_data_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

private:
	required_device<spectrum_expansion_slot_device> m_exp1;
	required_device<spectrum_expansion_slot_device> m_exp2;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_USLOT, spectrum_uslot_device)


#endif // MAME_BUS_SPECTRUM_USLOT_H

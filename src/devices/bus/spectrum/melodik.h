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

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;

	// passthru
	virtual void pre_opcode_fetch(offs_t offset) override { m_exp->pre_opcode_fetch(offset); }
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual uint8_t mreq_r(offs_t offset) override { return m_exp->romcs() ? m_exp->mreq_r(offset) : 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }
	virtual DECLARE_READ_LINE_MEMBER(romcs) override { return m_exp->romcs(); }

private:
	required_device<spectrum_expansion_slot_device> m_exp;
	required_device<ay8910_device> m_psg;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MELODIK, spectrum_melodik_device)


#endif // MAME_BUS_SPECTRUM_MELODIK_H

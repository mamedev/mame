// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Fuller Box Games Unit

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_FULLER_H
#define MAME_BUS_SPECTRUM_FULLER_H

#pragma once


#include "exp.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spectrum_fuller_device

class spectrum_fuller_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_fuller_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

private:
	required_device<spectrum_expansion_slot_device> m_exp;
	required_device<ay8910_device> m_psg;
	required_ioport m_joy;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_FULLER, spectrum_fuller_device)


#endif // MAME_BUS_SPECTRUM_FULLER_H

// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#ifndef MAME_MACHINE_PC9801_26_H
#define MAME_MACHINE_PC9801_26_H

#pragma once


#include "machine/pic8259.h"
#include "sound/2203intf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_26_device

class pc9801_26_device : public device_t
{
public:
	// construction/destruction
	pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_READ8_MEMBER(opn_porta_r);
	DECLARE_WRITE8_MEMBER(opn_portb_w);
	DECLARE_READ8_MEMBER(pc9801_26_r);
	DECLARE_WRITE8_MEMBER(pc9801_26_w);
	DECLARE_WRITE_LINE_MEMBER(pc9801_sound_irq);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler);

private:
//  required_device<cpu_device>  m_maincpu;
	required_device<ym2203_device>  m_opn;

	uint8_t m_joy_sel;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_26, pc9801_26_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_MACHINE_PC9801_26_H

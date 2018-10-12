// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#ifndef MAME_BUS_CBUS_PC9801_26_H
#define MAME_BUS_CBUS_PC9801_26_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
#include "sound/2203intf.h"
#include "pc9801_snd.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_26_device

class pc9801_26_device : public pc9801_snd_device
{
public:
	// construction/destruction
	pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(opn_r);
	DECLARE_WRITE8_MEMBER(opn_w);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<pc9801_slot_device> m_bus;
	required_device<ym2203_device>  m_opn;

	DECLARE_WRITE_LINE_MEMBER(sound_irq);
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_26, pc9801_26_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_BUS_CBUS_PC9801_26_H

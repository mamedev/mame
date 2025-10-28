// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-26 sound card

***************************************************************************/

#ifndef MAME_BUS_PC98_CBUS_PC9801_26_H
#define MAME_BUS_PC98_CBUS_PC9801_26_H

#pragma once

#include "slot.h"

#include "bus/msx/ctrl/ctrl.h"
#include "sound/ymopn.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_26_device

class pc9801_26_device : public device_t
{
public:
	// construction/destruction
	pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t opn_r(offs_t offset);
	void opn_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	u16 read_io_base();

private:
	required_device<pc98_cbus_slot_device> m_bus;
	required_device<ym2203_device>  m_opn;
	required_device_array<msx_general_purpose_port_device, 2U> m_joy;
	required_ioport m_irq_jp;

	u32 m_rom_base;
	u16 m_io_base;
	u8 m_joy_sel;
	u8 m_int_level;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_26, pc9801_26_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_BUS_PC98_CBUS_PC9801_26_H

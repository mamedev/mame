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
					   , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	pc9801_26_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	pc9801_26_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	u16 read_io_base();
	virtual void remap(int space_id, offs_t start, offs_t end) override;

	virtual void io_map(address_map &map) ATTR_COLD;

	required_device<ym2203_device>  m_opn;
private:
	required_device_array<msx_general_purpose_port_device, 2U> m_joy;
	required_memory_region m_bios;
	required_ioport m_irq_jp;

	u32 m_rom_base;
	u8 m_joy_sel;
	u8 m_int_level;
};


DECLARE_DEVICE_TYPE(PC9801_26, pc9801_26_device)


#endif // MAME_BUS_PC98_CBUS_PC9801_26_H

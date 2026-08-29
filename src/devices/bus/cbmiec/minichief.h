// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ICT Mini Chief MC-20 Floppy/Hard Disk Drive

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_MINICHIEF_H
#define MAME_BUS_CBMIEC_MINICHIEF_H

#pragma once

#include "c1571.h"
#include "bus/isa/isa.h"
#include "bus/isa/wd1002a_wx1.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mini_chief_device

class mini_chief_device :  public c1571_device
{
public:
	// construction/destruction
	mini_chief_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<isa8_device> m_isa;
	required_ioport m_boss;

	uint8_t via0_pa_r() { return c1571_device::via0_pa_r() | (m_boss->read() << 3); }
	uint8_t cia_pa_r() { return m_isa_data; }
	void cia_pa_w(uint8_t data) { m_isa_data = data; }
	void cia_pb_w(uint8_t data);

	void mini_chief_mem(address_map &map) ATTR_COLD;

	offs_t m_isa_offs;
	uint8_t m_isa_data;
};


// device type definition
DECLARE_DEVICE_TYPE(MINI_CHIEF, mini_chief_device)

#endif // MAME_BUS_CBMIEC_MINICHIEF_H

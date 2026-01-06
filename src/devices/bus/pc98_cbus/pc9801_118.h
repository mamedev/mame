// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-118

***************************************************************************/

#ifndef MAME_BUS_PC98_CBUS_PC9801_118_H
#define MAME_BUS_PC98_CBUS_PC9801_118_H

#pragma once

#include "sound/ymopn.h"

#include "slot.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_118_device

class pc9801_118_device : public device_t
						, public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	pc9801_118_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	u8 opn3_r(offs_t offset);
	void opn3_w(offs_t offset, u8 data);
	u8 id_r(offs_t offset);
	void ext_w(offs_t offset, u8 data);

protected:
	pc9801_118_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void io_map(address_map &map) ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

	required_device<ym2608_device> m_opn3;
private:

	u8 m_ext_reg;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_118, pc9801_118_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif // MAME_BUS_PC98_CBUS_PC9801_118_H

// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    System Sacom AMD-98 (AmuseMent boarD)

***************************************************************************/

#ifndef MAME_BUS_PC98_CBUS_AMD98_H
#define MAME_BUS_PC98_CBUS_AMD98_H

#pragma once

#include "slot.h"
#include "machine/pit8253.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> amd98_device

class amd98_device : public device_t
                   , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	amd98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	void io_map(address_map &map) ATTR_COLD;
	void ay3_address_w(u8 data);
	void ay3_data_latch_w(u8 data);

	u8 m_ay3_latch, m_ay3_ff;

	required_device<ay8910_device>  m_ay1;
	required_device<ay8910_device>  m_ay2;
	required_device<ay8910_device>  m_ay3;
	required_device<pit8253_device> m_pit;
};

// device type definition
DECLARE_DEVICE_TYPE(AMD98, amd98_device)

#endif // MAME_BUS_PC98_CBUS_AMD98_H

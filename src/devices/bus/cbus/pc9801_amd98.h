// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    System Sacom AMD-98 (AmuseMent boarD)

***************************************************************************/

#ifndef MAME_BUS_CBUS_PC9801_AMD98_H
#define MAME_BUS_CBUS_PC9801_AMD98_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
#include "sound/ay8910.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_amd98_device

class pc9801_amd98_device : public device_t
{
public:
	// construction/destruction
	pc9801_amd98_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void ay3_address_w(uint8_t data);
	void ay3_data_latch_w(uint8_t data);

	uint8_t m_ay3_latch;

	required_device<pc9801_slot_device> m_bus;
	required_device<ay8910_device>  m_ay1;
	required_device<ay8910_device>  m_ay2;
	required_device<ay8910_device>  m_ay3;

};

// device type definition
DECLARE_DEVICE_TYPE(PC9801_AMD98, pc9801_amd98_device)

#endif // MAME_BUS_CBUS_PC9801_AMD98_H

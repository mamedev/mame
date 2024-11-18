// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_CBUS_SB16_CT2720_H
#define MAME_BUS_CBUS_SB16_CT2720_H

#pragma once

#include "bus/cbus/pc9801_cbus.h"
//#include "bus/midi/midi.h"
//#include "bus/pc_joy/pc_joy.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
//#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sb16_ct2720_device : public device_t
{
public:
	// construction/destruction
	sb16_ct2720_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND | feature::MICROPHONE; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;

	required_device<pc9801_slot_device> m_bus;
	required_device<ymf262_device> m_opl3;
	required_device<dac_16bit_r2r_device> m_ldac;
	required_device<dac_16bit_r2r_device> m_rdac;
};


// device type definition
DECLARE_DEVICE_TYPE(SB16_CT2720, sb16_ct2720_device)

#endif // MAME_BUS_CBUS_SB16_CT2720_H

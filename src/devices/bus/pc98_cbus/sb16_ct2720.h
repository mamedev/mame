// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_SB16_CT2720_H
#define MAME_BUS_PC98_CBUS_SB16_CT2720_H

#pragma once

#include "slot.h"
//#include "bus/midi/midi.h"
#include "bus/pc_joy/pc_joy.h"
#include "machine/input_merger.h"
#include "sound/ct1741.h"
#include "sound/ct1745.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
//#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sb16_ct2720_device : public device_t
						 , public device_pc98_cbus_slot_interface
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

	virtual u8 dack_r(int line) override;
	virtual void dack_w(int line, u8 data) override;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
private:
	void io_map(address_map &map) ATTR_COLD;

	required_device<ymf262_device> m_opl3;
	required_device<ct1741_dsp_device> m_dsp;
	required_device<ct1745_mixer_device> m_mixer;
	required_device<dac_16bit_r2r_device> m_ldac;
	required_device<dac_16bit_r2r_device> m_rdac;
	required_device<input_merger_device> m_irqs;
	required_device<pc_joy_device> m_joy;

	bool m_irq8, m_irq16;
};


// device type definition
DECLARE_DEVICE_TYPE(SB16_CT2720, sb16_ct2720_device)

#endif // MAME_BUS_PC98_CBUS_SB16_CT2720_H

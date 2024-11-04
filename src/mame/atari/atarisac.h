// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atarisac.h

    Functions to emulate the Atari "SAC" audio board

***************************************************************************/

#ifndef MAME_ATARI_ATARI_SAC_H
#define MAME_ATARI_ATARI_SAC_H

#pragma once

#include "atarijsa.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(ATARI_SAC, atari_sac_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> atari_sac_device

class atari_sac_device : public atari_jsa_base_device
{
public:
	// construction/destruction
	atari_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal helpers
	virtual void update_all_volumes() override;

private:
	// read/write handlers
	u8 rdio_r();
	void wrio_w(u8 data);
	u8 pstat_r();
	u16 rdp8_r();
	void dac_w(offs_t offset, u16 data);

	// misc. helpers
	INTERRUPT_GEN_MEMBER(int_10k_gen);

	// address maps
	void sac_6502_map(address_map &map) ATTR_COLD;
	void sac_68k_map(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_daccpu;
	required_device<generic_latch_8_device> m_datin;
	required_device<generic_latch_8_device> m_datout;
	required_device<dac_word_interface> m_rdac;
	required_device<dac_word_interface> m_ldac;
	required_ioport m_inputs;

	// internal state
	bool m_68k_reset;
	bool m_10k_int;
};


#endif // MAME_ATARI_ATARI_SAC_H

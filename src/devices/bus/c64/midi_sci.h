// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sequential Circuits MIDI Interface cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_MIDI_SCI_H
#define MAME_BUS_C64_MIDI_SCI_H

#pragma once

#include "exp.h"
#include "machine/6850acia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_sequential_midi_cartridge_device

class c64_sequential_midi_cartridge_device : public device_t, public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_sequential_midi_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	void acia_irq_w(int state);
	void write_acia_clock(int state);

	required_device<acia6850_device> m_acia;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_MIDI_SCI, c64_sequential_midi_cartridge_device)


#endif // MAME_BUS_C64_MIDI_SCI_H

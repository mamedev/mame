// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Namesoft MIDI Interface cartridge emulation

**********************************************************************/

#pragma once

#ifndef __C64_MIDI_NAMESOFT__
#define __C64_MIDI_NAMESOFT__

#include "exp.h"
#include "machine/6850acia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_namesoft_midi_cartridge_device

class c64_namesoft_midi_cartridge_device : public device_t,
	public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_namesoft_midi_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );
	DECLARE_WRITE_LINE_MEMBER( write_acia_clock );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	required_device<acia6850_device> m_acia;
};


// device type definition
extern const device_type C64_MIDI_NAMESOFT;


#endif

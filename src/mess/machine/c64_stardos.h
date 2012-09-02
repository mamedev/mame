/**********************************************************************

    StarPoint Software StarDOS cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __STARDOS__
#define __STARDOS__


#include "emu.h"
#include "machine/c64exp.h"
#include "machine/6821pia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_stardos_cartridge_device

class c64_stardos_cartridge_device : public device_t,
									 public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_stardos_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( reset );

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_stardos"; }
	virtual void device_start();

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int roml, int romh, int io1, int io2);
	virtual int c64_game_r(offs_t offset, int ba, int rw, int hiram);

private:
	inline void charge_io1_capacitor();
	inline void charge_io2_capacitor();

	int m_io1_charge;
	int m_io2_charge;
};


// device type definition
extern const device_type C64_STARDOS;


#endif

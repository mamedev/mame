// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    StarPoint Software StarDOS cartridge emulation

**********************************************************************/

#pragma once

#ifndef __STARDOS__
#define __STARDOS__


#include "emu.h"
#include "exp.h"
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
	c64_stardos_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	inline void charge_io1_capacitor();
	inline void charge_io2_capacitor();

	int m_io1_charge;
	int m_io2_charge;
};


// device type definition
extern const device_type C64_STARDOS;


#endif

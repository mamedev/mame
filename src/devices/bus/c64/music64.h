// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wersi Wersiboard Music 64 / Siel CMK 49 Keyboard emulation

**********************************************************************/

#pragma once

#ifndef __MUSIC64__
#define __MUSIC64__

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_music64_cartridge_device

class c64_music64_cartridge_device : public device_t,
										public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_music64_cartridge_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	required_device<c64_expansion_slot_device> m_exp;
	required_ioport m_kb0;
	required_ioport m_kb1;
	required_ioport m_kb2;
	required_ioport m_kb3;
	required_ioport m_kb4;
	required_ioport m_kb5;
	required_ioport m_kb6;
};


// device type definition
extern const device_type C64_MUSIC64;


#endif

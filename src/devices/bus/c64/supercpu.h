// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD SuperCPU v2 + SuperRAM emulation

**********************************************************************/

#pragma once

#ifndef __SUPERCPU__
#define __SUPERCPU__

#include "emu.h"
#include "exp.h"
#include "cpu/g65816/g65816.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_supercpu_device

class c64_supercpu_device : public device_t,
							public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
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
	required_device<cpu_device> m_maincpu;
	required_device<c64_expansion_slot_device> m_exp;

	required_shared_ptr<UINT8> m_sram;
	required_shared_ptr<UINT8> m_dimm;
};


// device type definition
extern const device_type C64_SUPERCPU;



#endif

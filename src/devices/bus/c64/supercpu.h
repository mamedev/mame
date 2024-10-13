// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD SuperCPU v2 + SuperRAM emulation

**********************************************************************/

#ifndef MAME_BUS_C64_SUPERCPU_H
#define MAME_BUS_C64_SUPERCPU_H

#pragma once

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
	c64_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<c64_expansion_slot_device> m_exp;

	required_shared_ptr<uint8_t> m_sram;
	required_shared_ptr<uint8_t> m_dimm;

	void c64_supercpu_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_SUPERCPU, c64_supercpu_device)


#endif // MAME_BUS_C64_SUPERCPU_H

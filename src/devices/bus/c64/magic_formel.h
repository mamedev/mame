// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Magic Formel cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_MAGIC_FORMEL_H
#define MAME_BUS_C64_MAGIC_FORMEL_H

#pragma once

#include "exp.h"
#include "machine/6821pia.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_magic_formel_cartridge_device

class c64_magic_formel_cartridge_device : public device_t,
											public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_magic_formel_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( freeze );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	void pia_pa_w(uint8_t data);
	void pia_pb_w(uint8_t data);
	void pia_cb2_w(int state);

	required_device<pia6821_device> m_pia;
	memory_share_creator<uint8_t> m_ram;

	uint8_t m_rom_bank;
	uint8_t m_ram_bank;
	int m_ram_oe;
	int m_pb7;
	int m_u9a;
	int m_u9b;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_MAGIC_FORMEL, c64_magic_formel_cartridge_device)


#endif // MAME_BUS_C64_MAGIC_FORMEL_H

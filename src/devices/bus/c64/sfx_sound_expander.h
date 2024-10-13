// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore SFX Sound Expander cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_SFX_SOUND_EXPANDER_H
#define MAME_BUS_C64_SFX_SOUND_EXPANDER_H

#pragma once

#include "exp.h"
#include "sound/ymopl.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_sfx_sound_expander_cartridge_device

class c64_sfx_sound_expander_cartridge_device : public device_t,
												public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_sfx_sound_expander_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;

private:
	void opl_irq_w(int state);

	required_device<ym3526_device> m_opl;
	required_device<c64_expansion_slot_device> m_exp;
	required_ioport_array<8> m_kb;

	inline offs_t get_offset(offs_t offset, int rw);
};


// device type definition
DECLARE_DEVICE_TYPE(C64_SFX_SOUND_EXPANDER, c64_sfx_sound_expander_cartridge_device)


#endif // MAME_BUS_C64_SFX_SOUND_EXPANDER_H

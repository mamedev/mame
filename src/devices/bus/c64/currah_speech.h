// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Currah Speech 64 cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_CURRAH_SPEECH_H
#define MAME_BUS_C64_CURRAH_SPEECH_H

#pragma once


#include "exp.h"
#include "sound/sp0256.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_currah_speech_cartridge_device

class c64_currah_speech_cartridge_device : public device_t,
											public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_currah_speech_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;

private:
	inline void set_osc1(int voice, int intonation);

	required_device<sp0256_device> m_nsp;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_CURRAH_SPEECH, c64_currah_speech_cartridge_device)


#endif // MAME_BUS_C64_CURRAH_SPEECH_H

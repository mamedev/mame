// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Einstein Speech Synthesiser (J&K Software)

***************************************************************************/

#ifndef MAME_BUS_EINSTEIN_USERPORT_SPEECH_H
#define MAME_BUS_EINSTEIN_USERPORT_SPEECH_H

#pragma once

#include "userport.h"
#include "sound/sp0256.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> einstein_speech_device

class einstein_speech_device : public device_t, public device_einstein_userport_interface
{
public:
	// construction/destruction
	einstein_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual uint8_t read() override;
	virtual void write(uint8_t data) override;

private:
	required_device<sp0256_device> m_sp0256;
};

// device type definition
DECLARE_DEVICE_TYPE(EINSTEIN_SPEECH, einstein_speech_device)

#endif // MAME_BUS_EINSTEIN_USERPORT_SPEECH_H

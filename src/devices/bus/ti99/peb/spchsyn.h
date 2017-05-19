// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Speech Synthesizer
    See spchsyn.c for documentation

    Michael Zapf, October 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_SPCHSYN_H
#define MAME_BUS_TI99_PEB_SPCHSYN_H

#pragma once

#include "peribox.h"
#include "sound/tms5220.h"

DECLARE_DEVICE_TYPE(TI99_SPEECH, ti_speech_synthesizer_device)

class ti_speech_synthesizer_device : public ti_expansion_card_device
{
public:
	ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { }
	DECLARE_WRITE8_MEMBER(cruwrite) override { }

	DECLARE_WRITE_LINE_MEMBER( speech_ready );

protected:
	virtual void            device_start() override;
	virtual void            device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	address_space*  m_space;
	cd2501e_device* m_vsp;
	bool            m_reading;
	bool            m_sbe;          // Signal "Speech block enable"
};

#endif // MAME_BUS_TI99_PEB_SPCHSYN_H

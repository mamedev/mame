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
#include "machine/tms6100.h"

namespace bus { namespace ti99 { namespace peb {

class ti_speech_synthesizer_device : public device_t, public device_ti99_peribox_card_interface
{
public:
	ti_speech_synthesizer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	void write(offs_t offset, uint8_t data) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { }
	void cruwrite(offs_t offset, uint8_t data) override { }

protected:
	virtual void            device_start() override;
	virtual void            device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	DECLARE_WRITE_LINE_MEMBER( speech_ready );

	required_device<cd2501e_device> m_vsp;
	bool            m_reading;
	bool            m_sbe;          // Signal "Speech block enable"
};

} } } // end namespace bus::ti99::peb

DECLARE_DEVICE_TYPE_NS(TI99_SPEECH, bus::ti99::peb, ti_speech_synthesizer_device)

#endif // MAME_BUS_TI99_PEB_SPCHSYN_H

// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Speech Synthesizer
    See spchsyn.c for documentation

    Michael Zapf, October 2010
    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TISPEECH__
#define __TISPEECH__

#include "emu.h"
#include "peribox.h"
#include "sound/tms5220.h"

extern const device_type TI99_SPEECH;

class ti_speech_synthesizer_device : public ti_expansion_card_device
{
public:
	ti_speech_synthesizer_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_SETADDRESS_DBIN_MEMBER(setaddress_dbin) override;

	DECLARE_READ8Z_MEMBER(crureadz) override { };
	DECLARE_WRITE8_MEMBER(cruwrite) override { };

	DECLARE_WRITE_LINE_MEMBER( speech_ready );

protected:
	virtual void            device_start() override;
	virtual void            device_reset(void) override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void            device_config_complete() override;

private:
	cd2501e_device *m_vsp;
	bool            m_read_mode;
};

#endif

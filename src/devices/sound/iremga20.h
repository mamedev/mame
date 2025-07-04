// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont
/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#ifndef MAME_SOUND_IREMGA20_H
#define MAME_SOUND_IREMGA20_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> iremga20_device

class iremga20_device : public device_t,
						public device_sound_interface,
						public device_rom_interface<20>
{
public:
	iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	struct channel_def
	{
		uint32_t rate;
		uint32_t pos;
		uint32_t counter;
		uint32_t end;
		uint32_t volume;
		uint32_t play;
	};

	sound_stream *m_stream;
	uint8_t m_regs[0x20];
	channel_def m_channel[4];
};

DECLARE_DEVICE_TYPE(IREMGA20, iremga20_device)

#endif // MAME_SOUND_IREMGA20_H

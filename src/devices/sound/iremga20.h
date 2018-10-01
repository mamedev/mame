// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont
/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#ifndef MAME_SOUND_IREMGA20_H
#define MAME_SOUND_IREMGA20_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> iremga20_device

class iremga20_device : public device_t,
						public device_sound_interface,
						public device_rom_interface
{
public:
	iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( irem_ga20_w );
	DECLARE_READ8_MEMBER( irem_ga20_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// device_rom_interface overrides
	virtual void rom_bank_updated() override;

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

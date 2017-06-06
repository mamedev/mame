// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    M72 audio interface

****************************************************************************/
#ifndef MAME_AUDIO_M72_H
#define MAME_AUDIO_M72_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/dac.h"

class m72_audio_device : public device_t, public device_sound_interface
{
public:
	m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~m72_audio_device() {}

	enum
	{
		YM2151_ASSERT,
		YM2151_CLEAR,
		Z80_ASSERT,
		Z80_CLEAR
	};

	WRITE_LINE_MEMBER( ym2151_irq_handler );
	DECLARE_WRITE8_MEMBER( sound_command_byte_w );
	DECLARE_WRITE16_MEMBER( sound_command_w );
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w );
	DECLARE_READ8_MEMBER( sample_r );
	DECLARE_WRITE8_MEMBER( sample_w );

	/* the port goes to different address bits depending on the game */
	void set_sample_start( int start );
	DECLARE_WRITE8_MEMBER( vigilant_sample_addr_w );
	DECLARE_WRITE8_MEMBER( shisen_sample_addr_w );
	DECLARE_WRITE8_MEMBER( rtype2_sample_addr_w );
	DECLARE_WRITE8_MEMBER( poundfor_sample_addr_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	uint8_t m_irqvector;
	uint32_t m_sample_addr;
	optional_region_ptr<uint8_t> m_samples;
	uint32_t m_samples_size;
	address_space *m_space;
	optional_device<dac_byte_interface> m_dac;
	required_device<generic_latch_8_device> m_soundlatch;

	TIMER_CALLBACK_MEMBER( setvector_callback );
};

DECLARE_DEVICE_TYPE(IREM_M72_AUDIO, m72_audio_device)

#endif // MAME_AUDIO_M72_H

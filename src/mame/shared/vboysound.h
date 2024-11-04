// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    vboy.h - Virtual Boy audio emulation

    By Richard Bannister and Gil Pedersen.
    MAME device adaptation by R. Belmont
*/
#ifndef MAME_SHARED_VBOYSOUND_H
#define MAME_SHARED_VBOYSOUND_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vboysnd_device

class vboysnd_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	vboysnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 5'000'000);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	sound_stream *m_stream = nullptr;

protected:
	static constexpr unsigned CHANNELS        = 4;

	struct s_snd_channel {
		int8_t      playing = 0;    // the sound is playing

		// state when sound was enabled
		uint32_t    env_steptime = 0;       // Envelope step time
		uint8_t     env0 = 0;               // Envelope data
		uint8_t     env1 = 0;               // Envelope data
		uint8_t     volLeft = 0;            // Left output volume
		uint8_t     volRight = 0;           // Right output volume
		uint8_t     sample[580]{};        // sample to play
		int         sample_len = 0;         // length of sample

		// values that change, as the sample is played
		int         offset = 0;             // current offset in sample
		int         time = 0;               // the duration that this sample is to be played
		uint8_t     envelope = 0;           // Current envelope level (604)
		int         env_time = 0;           // The duration between envelope decay/grow (608)
	};

	struct s_regchan {
		int32_t sINT;
		int32_t sLRV;
		int32_t sFQL;
		int32_t sFQH;
		int32_t sEV0;
		int32_t sEV1;
		int32_t sRAM;
	};

	struct s_sreg {
		// Sound registers structure
		s_regchan c[4];
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(delayed_stream_update);

	s_snd_channel snd_channel[5];

	uint16_t waveFreq2LenTbl[2048];
	uint16_t waveTimer2LenTbl[32];
	uint16_t waveEnv2LenTbl[8];

	emu_timer *m_timer = nullptr;

	uint8_t m_aram[0x600];
};

// device type definition
DECLARE_DEVICE_TYPE(VBOYSND, vboysnd_device)

#endif //MAME_SHARED_VBOYSOUND_H

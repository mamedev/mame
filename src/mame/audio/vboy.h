// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    vboy.h - Virtual Boy audio emulation

    By Richard Bannister and Gil Pedersen.
    MESS device adaptation by R. Belmont
*/

#pragma once

#ifndef __VBOY_SND_H__
#define __VBOY_SND_H__

//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define AUDIO_FREQ      44100
#define CHANNELS        4

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VBOYSND_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, VBOYSND, AUDIO_FREQ)

#define MCFG_VBOYSND_REPLACE(_tag) \
	MCFG_DEVICE_REPLACE(_tag, VBOYSND, AUDIO_FREQ)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct s_snd_channel {
	INT8        playing;    // the sound is playing

	// state when sound was enabled
	UINT32      env_steptime;       // Envelope step time
	UINT8       env0;               // Envelope data
	UINT8       env1;               // Envelope data
	UINT8       volLeft;            // Left output volume
	UINT8       volRight;           // Right output volume
	UINT8       sample[580];        // sample to play
	int         sample_len;         // length of sample

	// values that change, as the sample is played
	int         offset;             // current offset in sample
	int         time;               // the duration that this sample is to be played
	UINT8       envelope;           // Current envelope level (604)
	int         env_time;           // The duration between envelope decay/grow (608)
};

struct s_regchan {
	INT32 sINT;
	INT32 sLRV;
	INT32 sFQL;
	INT32 sFQH;
	INT32 sEV0;
	INT32 sEV1;
	INT32 sRAM;
};

struct s_sreg {
	// Sound registers structure
	s_regchan c[4];
};

// ======================> vboysnd_device

class vboysnd_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	vboysnd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	sound_stream *m_stream;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	s_snd_channel snd_channel[5];

	UINT16 waveFreq2LenTbl[2048];
	UINT16 waveTimer2LenTbl[32];
	UINT16 waveEnv2LenTbl[8];

	emu_timer *m_timer;

	UINT8 m_aram[0x600];
};

// device type definition
extern const device_type VBOYSND;

#endif //__VBOY_SND_H__

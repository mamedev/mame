// license:BSD-3-Clause
// copyright-holders:Ville Linde
/* Ricoh RF5C400 emulator */

#pragma once

#ifndef __RF5C400_H__
#define __RF5C400_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RF5C400_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, RF5C400, _clock)
#define MCFG_RF5C400_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, RF5C400, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct rf5c400_channel
{
	rf5c400_channel() :
		startH(0),
		startL(0),
		freq(0),
		endL(0),
		endHloopH(0),
		loopL(0),
		pan(0),
		effect(0),
		volume(0),
		attack(0),
		decay(0),
		release(0),
		cutoff(0),
		pos(0),
		step(0),
		keyon(0),
		env_phase(0),
		env_level(0.0),
		env_step(0.0),
		env_scale(0.0)
	{ }

	UINT16  startH;
	UINT16  startL;
	UINT16  freq;
	UINT16  endL;
	UINT16  endHloopH;
	UINT16  loopL;
	UINT16  pan;
	UINT16  effect;
	UINT16  volume;

	UINT16  attack;
	UINT16  decay;
	UINT16  release;

	UINT16  cutoff;

	UINT64 pos;
	UINT64 step;
	UINT16 keyon;

	UINT8 env_phase;
	double env_level;
	double env_step;
	double env_scale;
};


// ======================> rf5c400_device

class rf5c400_device : public device_t,
						public device_sound_interface
{
public:
	rf5c400_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~rf5c400_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_READ16_MEMBER( rf5c400_r );
	DECLARE_WRITE16_MEMBER( rf5c400_w );

private:
	void rf5c400_init_chip();
	UINT8 decode80(UINT8 val);

private:
	required_region_ptr<INT16> m_rom;

	UINT32 m_rommask;

	sound_stream *m_stream;

	double m_env_ar_table[0x9f];
	double m_env_dr_table[0x9f];
	double m_env_rr_table[0x9f];

	rf5c400_channel m_channels[32];
};

extern const device_type RF5C400;


#endif /* __RF5C400_H__ */

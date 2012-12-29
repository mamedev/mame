#pragma once

#ifndef __GALELCO_H__
#define __GALELCO_H__

const int GAELCO_NUM_CHANNELS = 0x07;
const int VOLUME_LEVELS = 0x10;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GAELCO_GAE1_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, GAELCO_GAE1, _clock) \

#define MCFG_GAELCO_GAE1_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, GAELCO_GAE1, _clock) \

#define MCFG_GAELCO_CG1V_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, GAELCO_CG1V, _clock) \

#define MCFG_GAELCO_CG1V_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, GAELCO_CG1V, _clock) \


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> Sound Channel

struct gaelco_sound_channel
{
	int active;			// is it playing?
	int loop;			// = 0 no looping, = 1 looping
	int chunkNum;		// current chunk if looping
};


// ======================> External interface

struct gaelcosnd_interface
{
	const char *gfxregion;	/* shared gfx region name */
	int banks[4];			/* start of each ROM bank */
};


// ======================> gaelco_gae1_device

class gaelco_gae1_device : public device_t,
						   public device_sound_interface
{
public:
	gaelco_gae1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	gaelco_gae1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~gaelco_gae1_device() { }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_stop();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

public:
	DECLARE_WRITE16_MEMBER( gaelcosnd_w );
	DECLARE_READ16_MEMBER( gaelcosnd_r );

private:
	sound_stream *m_stream;									/* our stream */
	UINT8 *m_snd_data;										/* PCM data */
	int m_banks[4];											/* start of each ROM bank */
	gaelco_sound_channel m_channel[GAELCO_NUM_CHANNELS];	/* 7 stereo channels */

	UINT16 m_sndregs[0x38];

	// Table for converting from 8 to 16 bits with volume control
	INT16 m_volume_table[VOLUME_LEVELS][256];
};

extern const device_type GAELCO_GAE1;



// ======================> gaelco_cg1v_device

class gaelco_cg1v_device : public gaelco_gae1_device
{
public:
	gaelco_cg1v_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type GAELCO_CG1V;


#endif /* __GALELCO_H__ */

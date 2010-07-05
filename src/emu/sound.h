/***************************************************************************

    sound.h

    Core sound interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __SOUND_H__
#define __SOUND_H__


/***************************************************************************
    MACROS
***************************************************************************/

/* these functions are macros primarily due to include file ordering */
/* plus, they are very simple */
#define speaker_output_count(config)		(config)->m_devicelist.count(SPEAKER)
#define speaker_output_first(config)		(config)->m_devicelist.first(SPEAKER)
#define speaker_output_next(previous)		(previous)->typenext()



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> speaker_device_config

class speaker_device_config : public device_config
{
	friend class speaker_device;

	// construction/destruction
	speaker_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// indexes to inline data
	enum
	{
		INLINE_X,
		INLINE_Y,
		INLINE_Z
	};

protected:
	// device_config overrides
	virtual void device_config_complete();

	// internal state
	double		m_x;
	double		m_y;
	double		m_z;
};



// ======================> speaker_device

class speaker_device : public device_t
{
	friend class speaker_device_config;
	friend resource_pool_object<speaker_device>::~resource_pool_object();

	// construction/destruction
	speaker_device(running_machine &_machine, const speaker_device_config &config);
	virtual ~speaker_device();

public:
	// input properties
	int inputs() const { return m_inputs; }
	float input_gain(int index = 0) const { return m_input[index].m_gain; }
	float input_default_gain(int index = 0) const { return m_input[index].m_default_gain; }
	const char *input_name(int index = 0) const { return m_input[index].m_name; }
	void set_input_gain(int index, float gain);

	// internally for use by the sound system
	void mix(INT32 *leftmix, INT32 *rightmix, int &samples_this_update, bool suppress);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_post_load();

	// internal helpers
	static STREAM_UPDATE( static_mixer_update ) { downcast<speaker_device *>(device)->mixer_update(inputs, outputs, samples); }
	void mixer_update(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// a single input
	struct speaker_input
	{
		float			m_gain;					// current gain
		float			m_default_gain;			// default gain
		astring			m_name;					// name of this input
	};

	// internal state
	const speaker_device_config &m_config;
	sound_stream *		m_mixer_stream;			// mixing stream
	int					m_inputs;				// number of input streams
	speaker_input *		m_input;				// array of input information
#ifdef MAME_DEBUG
	INT32				m_max_sample;			// largest sample value we've seen
	INT32				m_clipped_samples;		// total number of clipped samples
	INT32				m_total_samples;		// total number of samples
#endif
};


// device type definition
extern const device_type SPEAKER;



/***************************************************************************
    SOUND DEVICE CONFIGURATION MACROS
***************************************************************************/

/* add/remove speakers */
#define MDRV_SPEAKER_ADD(_tag, _x, _y, _z) \
	MDRV_DEVICE_ADD(_tag, SPEAKER, 0) \
	MDRV_DEVICE_INLINE_DATA32(speaker_device_config::INLINE_X, (INT32)((_x) * (double)(1 << 24))) \
	MDRV_DEVICE_INLINE_DATA32(speaker_device_config::INLINE_Y, (INT32)((_y) * (double)(1 << 24))) \
	MDRV_DEVICE_INLINE_DATA32(speaker_device_config::INLINE_Z, (INT32)((_z) * (double)(1 << 24)))

#define MDRV_SPEAKER_STANDARD_MONO(_tag) \
	MDRV_SPEAKER_ADD(_tag, 0.0, 0.0, 1.0)

#define MDRV_SPEAKER_STANDARD_STEREO(_tagl, _tagr) \
	MDRV_SPEAKER_ADD(_tagl, -0.2, 0.0, 1.0) \
	MDRV_SPEAKER_ADD(_tagr, 0.2, 0.0, 1.0)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core interfaces */
void sound_init(running_machine *machine);



/* ----- sound device interface ----- */

/* global sound controls */
void sound_mute(running_machine *machine, int mute);
void sound_set_attenuation(running_machine *machine, int attenuation);
int sound_get_attenuation(running_machine *machine);
void sound_global_enable(running_machine *machine, int enable);

/* user gain controls on speaker inputs for mixing */
int sound_get_user_gain_count(running_machine *machine);
void sound_set_user_gain(running_machine *machine, int index, float gain);
float sound_get_user_gain(running_machine *machine, int index);
float sound_get_default_gain(running_machine *machine, int index);
const char *sound_get_user_gain_name(running_machine *machine, int index);


/* driver gain controls on chip outputs */
void sound_set_output_gain(device_t *device, int output, float gain);



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  speaker_count - return the number of speaker
//  devices in a machine_config
//-------------------------------------------------

inline int speaker_count(const machine_config &config)
{
	return config.m_devicelist.count(SPEAKER);
}


//-------------------------------------------------
//  speaker_first - return the first speaker
//  device config in a machine_config
//-------------------------------------------------

inline const speaker_device_config *speaker_first(const machine_config &config)
{
	return downcast<speaker_device_config *>(config.m_devicelist.first(SPEAKER));
}


//-------------------------------------------------
//  speaker_next - return the next speaker
//  device config in a machine_config
//-------------------------------------------------

inline const speaker_device_config *speaker_next(const speaker_device_config *previous)
{
	return downcast<speaker_device_config *>(previous->typenext());
}


//-------------------------------------------------
//  speaker_count - return the number of speaker
//  devices in a machine
//-------------------------------------------------

inline int speaker_count(running_machine &machine)
{
	return machine.m_devicelist.count(SPEAKER);
}


//-------------------------------------------------
//  speaker_first - return the first speaker
//  device in a machine
//-------------------------------------------------

inline speaker_device *speaker_first(running_machine &machine)
{
	return downcast<speaker_device *>(machine.m_devicelist.first(SPEAKER));
}


//-------------------------------------------------
//  speaker_next - return the next speaker
//  device in a machine
//-------------------------------------------------

inline speaker_device *speaker_next(speaker_device *previous)
{
	return downcast<speaker_device *>(previous->typenext());
}


#endif	/* __SOUND_H__ */

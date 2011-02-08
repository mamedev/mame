/***************************************************************************

    speaker.h

    Speaker output sound device.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __SPEAKER_H__
#define __SPEAKER_H__


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// add/remove speakers
#define MCFG_SPEAKER_ADD(_tag, _x, _y, _z) \
	MCFG_DEVICE_ADD(_tag, SPEAKER, 0) \
	speaker_device_config::static_set_position(device, _x, _y, _z); \

#define MCFG_SPEAKER_STANDARD_MONO(_tag) \
	MCFG_SPEAKER_ADD(_tag, 0.0, 0.0, 1.0)

#define MCFG_SPEAKER_STANDARD_STEREO(_tagl, _tagr) \
	MCFG_SPEAKER_ADD(_tagl, -0.2, 0.0, 1.0) \
	MCFG_SPEAKER_ADD(_tagr, 0.2, 0.0, 1.0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> speaker_device_config

class speaker_device_config : public device_config,
							  public device_config_sound_interface
{
	friend class speaker_device;

	// construction/destruction
	speaker_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;

	// inline configuration helpers
	speaker_device_config *next_speaker() const { return downcast<speaker_device_config *>(typenext()); }
	static void static_set_position(device_config *device, double x, double y, double z);

protected:
	// inline configuration state
	double		m_x;
	double		m_y;
	double		m_z;
};



// ======================> speaker_device

class speaker_device : public device_t,
					   public device_sound_interface
{
	friend class speaker_device_config;
	friend resource_pool_object<speaker_device>::~resource_pool_object();

	// construction/destruction
	speaker_device(running_machine &_machine, const speaker_device_config &config);
	virtual ~speaker_device();

public:
	// getters
	speaker_device *next_speaker() const { return downcast<speaker_device *>(typenext()); }

	// internally for use by the sound system
	void mix(INT32 *leftmix, INT32 *rightmix, int &samples_this_update, bool suppress);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_post_load();

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	const speaker_device_config &m_config;
	sound_stream *		m_mixer_stream;			// mixing stream
#ifdef MAME_DEBUG
	INT32				m_max_sample;			// largest sample value we've seen
	INT32				m_clipped_samples;		// total number of clipped samples
	INT32				m_total_samples;		// total number of samples
#endif
};


// device type definition
extern const device_type SPEAKER;


#endif	/* __SOUND_H__ */

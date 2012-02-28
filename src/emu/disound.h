/***************************************************************************

    disound.h

    Device sound interfaces.

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

#ifndef __DISOUND_H__
#define __DISOUND_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int ALL_OUTPUTS		= 65535;	// special value indicating all outputs for the current chip
const int AUTO_ALLOC_INPUT	= 65535;



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SOUND_ADD(_tag, _type, _clock) \
	MCFG_DEVICE_ADD(_tag, _type, _clock) \

#define MCFG_SOUND_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_SOUND_CLOCK(_clock) \
	MCFG_DEVICE_CLOCK(_clock)

#define MCFG_SOUND_REPLACE(_tag, _type, _clock) \
	MCFG_DEVICE_REPLACE(_tag, _type, _clock)

#define MCFG_SOUND_CONFIG(_config) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SOUND_ROUTE_EX(_output, _target, _gain, _input) \
	device_sound_interface::static_add_route(*device, _output, _target, _gain, _input); \

#define MCFG_SOUND_ROUTE(_output, _target, _gain) \
	MCFG_SOUND_ROUTE_EX(_output, _target, _gain, AUTO_ALLOC_INPUT)

#define MCFG_SOUND_ROUTES_RESET() \
	device_sound_interface::static_reset_routes(*device); \



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sound_stream;


// ======================> device_sound_interface

class device_sound_interface : public device_interface
{
public:
	class sound_route
	{
	public:
		sound_route(int output, int input, float gain, const char *target);

		const sound_route *next() const { return m_next; }

		sound_route *		m_next;				// pointer to next route
		UINT32				m_output;			// output index, or ALL_OUTPUTS
		UINT32				m_input;			// target input index
		float				m_gain;				// gain
		astring				m_target;			// target tag
	};

	// construction/destruction
	device_sound_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sound_interface();

	// configuration access
	const sound_route *first_route() const { return m_route_list.first(); }

	// static inline configuration helpers
	static void static_add_route(device_t &device, UINT32 output, const char *target, double gain, UINT32 input = AUTO_ALLOC_INPUT);
	static void static_reset_routes(device_t &device);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) = 0;

	// stream creation
	sound_stream *stream_alloc(int inputs, int outputs, int sample_rate);

	// helpers
	int inputs() const;
	int outputs() const;
	sound_stream *input_to_stream_input(int inputnum, int &stream_inputnum);
	sound_stream *output_to_stream_output(int outputnum, int &stream_outputnum);
	void set_output_gain(int outputnum, float gain);

protected:
	// optional operation overrides
	virtual void interface_validity_check(validity_checker &valid) const;
	virtual void interface_pre_start();
	virtual void interface_post_start();
	virtual void interface_pre_reset();

	// internal state
	simple_list<sound_route> m_route_list;		// list of sound routes
	int				m_outputs;					// number of outputs from this instance
	int				m_auto_allocated_inputs;	// number of auto-allocated inputs targeting us
};

// iterator
typedef device_interface_iterator<device_sound_interface> sound_interface_iterator;



// ======================> device_mixer_interface

class device_mixer_interface : public device_sound_interface
{
public:
	// construction/destruction
	device_mixer_interface(const machine_config &mconfig, device_t &device, int outputs = 1);
	virtual ~device_mixer_interface();

protected:
	// optional operation overrides
	virtual void interface_pre_start();
	virtual void interface_post_load();

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	UINT8				m_outputs;				// number of outputs
	sound_stream *		m_mixer_stream;			// mixing stream
};

// iterator
typedef device_interface_iterator<device_mixer_interface> mixer_interface_iterator;


#endif	/* __DISOUND_H__ */

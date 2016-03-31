// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    disound.h

    Device sound interfaces.

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

const int ALL_OUTPUTS       = 65535;    // special value indicating all outputs for the current chip
const int AUTO_ALLOC_INPUT  = 65535;



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SOUND_ADD(_tag, _type, _clock) \
	MCFG_DEVICE_ADD(_tag, _type, _clock)
#define MCFG_SOUND_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

#define MCFG_SOUND_CLOCK(_clock) \
	MCFG_DEVICE_CLOCK(_clock)

#define MCFG_SOUND_REPLACE(_tag, _type, _clock) \
	MCFG_DEVICE_REPLACE(_tag, _type, _clock)

#define MCFG_SOUND_CONFIG(_config) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SOUND_ROUTE(_output, _target, _gain) \
	device_sound_interface::static_add_route(*device, _output, _target, _gain);
#define MCFG_SOUND_ROUTE_EX(_output, _target, _gain, _input) \
	device_sound_interface::static_add_route(*device, _output, _target, _gain, _input);
#define MCFG_SOUND_ROUTES_RESET() \
	device_sound_interface::static_reset_routes(*device);

#define MCFG_MIXER_ROUTE(_output, _target, _gain, _mixoutput) \
	device_sound_interface::static_add_route(*device, _output, _target, _gain, AUTO_ALLOC_INPUT, _mixoutput);


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
		sound_route(int output, int input, float gain, const char *target, UINT32 mixoutput);

		sound_route *next() const { return m_next; }

		sound_route *       m_next;             // pointer to next route
		UINT32              m_output;           // output index, or ALL_OUTPUTS
		UINT32              m_input;            // target input index
		UINT32              m_mixoutput;        // target mixer output
		float               m_gain;             // gain
		std::string         m_target;           // target tag
	};

	// construction/destruction
	device_sound_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_sound_interface();

	// configuration access
	const simple_list<sound_route> &routes() const { return m_route_list; }

	// static inline configuration helpers
	static sound_route &static_add_route(device_t &device, UINT32 output, const char *target, double gain, UINT32 input = AUTO_ALLOC_INPUT, UINT32 mixoutput = 0);
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
	void set_input_gain(int inputnum, float gain);
	void set_output_gain(int outputnum, float gain);
	int inputnum_from_device(device_t &device, int outputnum = 0) const;

protected:
	// optional operation overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;

	// internal state
	simple_list<sound_route> m_route_list;      // list of sound routes
	int             m_outputs;                  // number of outputs from this instance
	int             m_auto_allocated_inputs;    // number of auto-allocated inputs targeting us
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
	virtual void interface_pre_start() override;
	virtual void interface_post_load() override;

	// sound interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	UINT8               m_outputs;              // number of outputs
	std::vector<UINT8>       m_outputmap;            // map of inputs to outputs
	sound_stream *      m_mixer_stream;         // mixing stream
};

// iterator
typedef device_interface_iterator<device_mixer_interface> mixer_interface_iterator;


#endif  /* __DISOUND_H__ */

// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    disound.c

    Device sound interfaces.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVICE CONFIG SOUND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sound_interface - constructor
//-------------------------------------------------

device_sound_interface::device_sound_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "sound"),
		m_outputs(0),
		m_auto_allocated_inputs(0)
{
}


//-------------------------------------------------
//  ~device_sound_interface - destructor
//-------------------------------------------------

device_sound_interface::~device_sound_interface()
{
}


//-------------------------------------------------
//  static_add_route - configuration helper to add
//  a new route to the device
//-------------------------------------------------

device_sound_interface::sound_route &device_sound_interface::static_add_route(device_t &device, UINT32 output, const char *target, double gain, UINT32 input, UINT32 mixoutput)
{
	// find our sound interface
	device_sound_interface *sound;
	if (!device.interface(sound))
		throw emu_fatalerror("MCFG_SOUND_ROUTE called on device '%s' with no sound interface", device.tag().c_str());

	// append a new route to the list
	return sound->m_route_list.append(*global_alloc(sound_route(output, input, gain, target, mixoutput)));
}


//-------------------------------------------------
//  static_reset_routes - configuration helper to
//  reset all existing routes to the device
//-------------------------------------------------

void device_sound_interface::static_reset_routes(device_t &device)
{
	// find our sound interface
	device_sound_interface *sound;
	if (!device.interface(sound))
		throw emu_fatalerror("MCFG_SOUND_ROUTES_RESET called on device '%s' with no sound interface", device.tag().c_str());

	// reset the routine list
	sound->m_route_list.reset();
}


//-------------------------------------------------
//  stream_alloc - allocate a stream implicitly
//  associated with this device
//-------------------------------------------------

sound_stream *device_sound_interface::stream_alloc(int inputs, int outputs, int sample_rate)
{
	return device().machine().sound().stream_alloc(*this, inputs, outputs, sample_rate);
}


//-------------------------------------------------
//  inputs - return the total number of inputs
//  for the given device
//-------------------------------------------------

int device_sound_interface::inputs() const
{
	// scan the list counting streams we own and summing their inputs
	int inputs = 0;
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
		if (&stream->device() == &m_device)
			inputs += stream->input_count();
	return inputs;
}


//-------------------------------------------------
//  outputs - return the total number of outputs
//  for the given device
//-------------------------------------------------

int device_sound_interface::outputs() const
{
	// scan the list counting streams we own and summing their outputs
	int outputs = 0;
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
		if (&stream->device() == &m_device)
			outputs += stream->output_count();
	return outputs;
}


//-------------------------------------------------
//  input_to_stream_input - convert a device's
//  input index to a stream and the input index
//  on that stream
//-------------------------------------------------

sound_stream *device_sound_interface::input_to_stream_input(int inputnum, int &stream_inputnum)
{
	assert(inputnum >= 0);

	// scan the list looking for streams owned by this device
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
		if (&stream->device() == &m_device)
		{
			if (inputnum < stream->input_count())
			{
				stream_inputnum = inputnum;
				return stream;
			}
			inputnum -= stream->input_count();
		}

	// not found
	return nullptr;
}


//-------------------------------------------------
//  output_to_stream_output - convert a device's
//  output index to a stream and the output index
//  on that stream
//-------------------------------------------------

sound_stream *device_sound_interface::output_to_stream_output(int outputnum, int &stream_outputnum)
{
	assert(outputnum >= 0);

	// scan the list looking for streams owned by this device
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
		if (&stream->device() == &device())
		{
			if (outputnum < stream->output_count())
			{
				stream_outputnum = outputnum;
				return stream;
			}
			outputnum -= stream->output_count();
		}

	// not found
	return nullptr;
}


//-------------------------------------------------
//  set_input_gain - set the gain on the given
//  input index of the device
//-------------------------------------------------

void device_sound_interface::set_input_gain(int inputnum, float gain)
{
	int stream_inputnum;
	sound_stream *stream = input_to_stream_input(inputnum, stream_inputnum);
	if (stream != nullptr)
		stream->set_input_gain(stream_inputnum, gain);
}


//-------------------------------------------------
//  set_output_gain - set the gain on the given
//  output index of the device
//-------------------------------------------------

void device_sound_interface::set_output_gain(int outputnum, float gain)
{
	// handle ALL_OUTPUTS as a special case
	if (outputnum == ALL_OUTPUTS)
	{
		for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
			if (&stream->device() == &device())
				for (int num = 0; num < stream->output_count(); num++)
					stream->set_output_gain(num, gain);
	}

	// look up the stream and stream output index
	else
	{
		int stream_outputnum;
		sound_stream *stream = output_to_stream_output(outputnum, stream_outputnum);
		if (stream != nullptr)
			stream->set_output_gain(stream_outputnum, gain);
	}
}


//-------------------------------------------------
//  inputnum_from_device - return the input number
//  that is connected to the given device's output
//-------------------------------------------------

int device_sound_interface::inputnum_from_device(device_t &source_device, int outputnum) const
{
	int overall = 0;
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
		if (&stream->device() == &device())
			for (int inputnum = 0; inputnum < stream->input_count(); inputnum++, overall++)
				if (stream->input_source_device(inputnum) == &source_device && stream->input_source_outputnum(inputnum) == outputnum)
					return overall;
	return -1;
}


//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_sound_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all the routes
	for (const sound_route *route = first_route(); route != nullptr; route = route->next())
	{
		// find a device with the requested tag
		const device_t *target = device().siblingdevice(route->m_target.c_str());
		if (target == nullptr)
			osd_printf_error("Attempting to route sound to non-existant device '%s'\n", route->m_target.c_str());

		// if it's not a speaker or a sound device, error
		const device_sound_interface *sound;
		if (target != nullptr && target->type() != SPEAKER && !target->interface(sound))
			osd_printf_error("Attempting to route sound to a non-sound device '%s' (%s)\n", route->m_target.c_str(), target->name().c_str());
	}
}


//-------------------------------------------------
//  interface_pre_start - make sure all our input
//  devices are started
//-------------------------------------------------

void device_sound_interface::interface_pre_start()
{
	// scan all the sound devices
	sound_interface_iterator iter(m_device.machine().root_device());
	for (device_sound_interface *sound = iter.first(); sound != nullptr; sound = iter.next())
	{
		// scan each route on the device
		for (const sound_route *route = sound->first_route(); route != nullptr; route = route->next())
		{
			// see if we are the target of this route; if we are, make sure the source device is started
			device_t *target_device = sound->device().siblingdevice(route->m_target.c_str());
			if (target_device == &m_device && !sound->device().started())
				throw device_missing_dependencies();
		}
	}

	// now iterate through devices again and assign any auto-allocated inputs
	m_auto_allocated_inputs = 0;
	for (device_sound_interface *sound = iter.first(); sound != nullptr; sound = iter.next())
	{
		// scan each route on the device
		for (const sound_route *route = sound->first_route(); route != nullptr; route = route->next())
		{
			// see if we are the target of this route
			device_t *target_device = sound->device().siblingdevice(route->m_target.c_str());
			if (target_device == &m_device && route->m_input == AUTO_ALLOC_INPUT)
			{
				const_cast<sound_route *>(route)->m_input = m_auto_allocated_inputs;
				m_auto_allocated_inputs += (route->m_output == ALL_OUTPUTS) ? sound->outputs() : 1;
			}
		}
	}
}


//-------------------------------------------------
//  interface_post_start - verify that state was
//  properly set up
//-------------------------------------------------

void device_sound_interface::interface_post_start()
{
	// iterate over all the sound devices
	sound_interface_iterator iter(m_device.machine().root_device());
	for (device_sound_interface *sound = iter.first(); sound != nullptr; sound = iter.next())
	{
		// scan each route on the device
		for (const sound_route *route = sound->first_route(); route != nullptr; route = route->next())
		{
			// if we are the target of this route, hook it up
			device_t *target_device = sound->device().siblingdevice(route->m_target.c_str());
			if (target_device == &m_device)
			{
				// iterate over all outputs, matching any that apply
				int inputnum = route->m_input;
				int numoutputs = sound->outputs();
				for (int outputnum = 0; outputnum < numoutputs; outputnum++)
					if (route->m_output == outputnum || route->m_output == ALL_OUTPUTS)
					{
						// find the output stream to connect from
						int streamoutputnum;
						sound_stream *outputstream = sound->output_to_stream_output(outputnum, streamoutputnum);
						if (outputstream == nullptr)
							fatalerror("Sound device '%s' specifies route for non-existant output #%d\n", route->m_target.c_str(), outputnum);

						// find the input stream to connect to
						int streaminputnum;
						sound_stream *inputstream = input_to_stream_input(inputnum++, streaminputnum);
						if (inputstream == nullptr)
							fatalerror("Sound device '%s' targeted output #%d to non-existant device '%s' input %d\n", route->m_target.c_str(), outputnum, m_device.tag().c_str(), inputnum - 1);

						// set the input
						inputstream->set_input(streaminputnum, outputstream, streamoutputnum, route->m_gain);
					}
			}
		}
	}
}


//-------------------------------------------------
//  interface_pre_reset - called prior to
//  resetting the device
//-------------------------------------------------

void device_sound_interface::interface_pre_reset()
{
	// update all streams on this device prior to reset
	for (sound_stream *stream = m_device.machine().sound().first_stream(); stream != nullptr; stream = stream->next())
		if (&stream->device() == &device())
			stream->update();
}



//**************************************************************************
//  SOUND ROUTE
//**************************************************************************

//-------------------------------------------------
//  sound_route - constructor
//-------------------------------------------------

device_sound_interface::sound_route::sound_route(int output, int input, float gain, const char *target, UINT32 mixoutput)
	: m_next(nullptr),
		m_output(output),
		m_input(input),
		m_mixoutput(mixoutput),
		m_gain(gain),
		m_target(target)
{
}



//**************************************************************************
//  SIMPLE DERIVED MIXER INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_mixer_interface - constructor
//-------------------------------------------------

device_mixer_interface::device_mixer_interface(const machine_config &mconfig, device_t &device, int outputs)
	: device_sound_interface(mconfig, device),
		m_outputs(outputs),
		m_mixer_stream(nullptr)
{
}


//-------------------------------------------------
//  ~device_mixer_interface - destructor
//-------------------------------------------------

device_mixer_interface::~device_mixer_interface()
{
}


//-------------------------------------------------
//  interface_pre_start - perform startup prior
//  to the device startup
//-------------------------------------------------

void device_mixer_interface::interface_pre_start()
{
	// call our parent
	device_sound_interface::interface_pre_start();

	// no inputs? that's weird
	if (m_auto_allocated_inputs == 0)
	{
		device().logerror("Warning: mixer \"%s\" has no inputs\n", device().tag().c_str());
		return;
	}

	// generate the output map
	m_outputmap.resize(m_auto_allocated_inputs);

	// iterate through all routes that point to us and note their mixer output
	sound_interface_iterator iter(m_device.machine().root_device());
	for (device_sound_interface *sound = iter.first(); sound != nullptr; sound = iter.next())
		for (const sound_route *route = sound->first_route(); route != nullptr; route = route->next())
		{
			// see if we are the target of this route
			device_t *target_device = sound->device().siblingdevice(route->m_target.c_str());
			if (target_device == &device() && route->m_input < m_auto_allocated_inputs)
			{
				int count = (route->m_output == ALL_OUTPUTS) ? sound->outputs() : 1;
				for (int output = 0; output < count; output++)
					m_outputmap[route->m_input + output] = route->m_mixoutput;
			}
		}

	// allocate the mixer stream
	m_mixer_stream = stream_alloc(m_auto_allocated_inputs, m_outputs, device().machine().sample_rate());
}


//-------------------------------------------------
//  interface_post_load - after we load a save
//  state be sure to update the mixer stream's
//  output sample rate
//-------------------------------------------------

void device_mixer_interface::interface_post_load()
{
	m_mixer_stream->set_sample_rate(device().machine().sample_rate());

	// call our parent
	device_sound_interface::interface_post_load();
}


//-------------------------------------------------
//  mixer_update - mix all inputs to one output
//-------------------------------------------------

void device_mixer_interface::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// clear output buffers
	for (int output = 0; output < m_outputs; output++)
		memset(outputs[output], 0, samples * sizeof(outputs[0][0]));

	// loop over samples
	const UINT8 *outmap = &m_outputmap[0];
	for (int pos = 0; pos < samples; pos++)
	{
		// for each input, add it to the appropriate output
		for (int inp = 0; inp < m_auto_allocated_inputs; inp++)
			outputs[outmap[inp]][pos] += inputs[inp][pos];
	}
}

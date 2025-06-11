// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    disound.cpp

    Device sound interfaces.

***************************************************************************/

#include "emu.h"
#include "speaker.h"



//**************************************************************************
//  DEVICE CONFIG SOUND INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sound_interface - constructor
//-------------------------------------------------

device_sound_interface::device_sound_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "sound"),
	m_sound_requested_inputs_mask(0),
	m_sound_requested_outputs_mask(0),
	m_sound_requested_inputs(0),
	m_sound_requested_outputs(0),
	m_sound_hook(false)
{
}


//-------------------------------------------------
//  ~device_sound_interface - destructor
//-------------------------------------------------

device_sound_interface::~device_sound_interface()
{
}


//-------------------------------------------------
//  add_route - send sound output to a consumer
//-------------------------------------------------

device_sound_interface &device_sound_interface::add_route(u32 output, const char *target, double gain, u32 channel)
{
	return add_route(output, device().mconfig().current_device(), target, gain, channel);
}

device_sound_interface &device_sound_interface::add_route(u32 output, device_sound_interface &target, double gain, u32 channel)
{
	return add_route(output, target.device(), DEVICE_SELF, gain, channel);
}

device_sound_interface &device_sound_interface::add_route(u32 output, device_t &base, const char *target, double gain, u32 channel)
{
	assert(!device().started());
	m_route_list.emplace_back(sound_route{ output, channel, float(gain), base, target, nullptr });
	return *this;
}


//-------------------------------------------------
//  stream_alloc - allocate a stream implicitly
//  associated with this device
//-------------------------------------------------

sound_stream *device_sound_interface::stream_alloc(int inputs, int outputs, int sample_rate, sound_stream_flags flags)
{
	sound_stream *stream = device().machine().sound().stream_alloc(*this, inputs, outputs, sample_rate, stream_update_delegate(&device_sound_interface::sound_stream_update, this), flags);
	m_sound_streams.push_back(stream);
	return stream;
}



//-------------------------------------------------
//  inputs - return the total number of inputs
//  forthe given device
//-------------------------------------------------

int device_sound_interface::inputs() const
{
	// scan the list counting streams we own and summing their inputs
	int inputs = 0;
	for(sound_stream *stream : m_sound_streams)
		inputs += stream->input_count();
	return inputs;
}


//-------------------------------------------------
//  outputs - return the total number of outputs
//  forthe given device
//-------------------------------------------------

int device_sound_interface::outputs() const
{
	// scan the list counting streams we own and summing their outputs
	int outputs = 0;
	for(auto *stream : m_sound_streams)
		outputs += stream->output_count();
	return outputs;
}


//-------------------------------------------------
//  input_to_stream_input - convert a device's
//  input index to a stream and the input index
//  on that stream
//-------------------------------------------------

std::pair<sound_stream *, int> device_sound_interface::input_to_stream_input(int inputnum) const
{
	assert(inputnum >= 0);
	int orig_inputnum = inputnum;

	// scan the list looking forstreams owned by this device
	for(auto *stream : m_sound_streams) {
		if(inputnum < stream->input_count())
			return std::make_pair(stream, inputnum);
		inputnum -= stream->input_count();
	}

	fatalerror("Requested input %d on sound device %s which only has %d.", orig_inputnum, device().tag(), inputs());
}


//-------------------------------------------------
//  output_to_stream_output - convert a device's
//  output index to a stream and the output index
//  on that stream
//-------------------------------------------------

std::pair<sound_stream *, int> device_sound_interface::output_to_stream_output(int outputnum) const
{
	assert(outputnum >= 0);
	int orig_outputnum = outputnum;

	// scan the list looking forstreams owned by this device
	for(auto *stream : m_sound_streams) {
		if(outputnum < stream->output_count())
			return std::make_pair(stream, outputnum);
		outputnum -= stream->output_count();
	}

	fatalerror("Requested output %d on sound device %s which only has %d.", orig_outputnum, device().tag(), outputs());
}


//-------------------------------------------------
//  input_gain - return the gain on the given
//  input index of the device
//-------------------------------------------------

float device_sound_interface::input_gain(int inputnum) const
{
	auto [stream, input] = input_to_stream_input(inputnum);
	return stream->input_gain(input);
}


//-------------------------------------------------
//  output_gain - return the gain on the given
//  output index of the device
//-------------------------------------------------

float device_sound_interface::output_gain(int outputnum) const
{
	auto [stream, output] = output_to_stream_output(outputnum);
	return stream->output_gain(output);
}


//-------------------------------------------------
//  user_output_gain - return the user gain for the device
//-------------------------------------------------

float device_sound_interface::user_output_gain() const
{
	if(!outputs())
		fatalerror("Requested user output gain on sound device %s which has no outputs.", device().tag());
	return m_sound_streams.front()->user_output_gain();
}


//-------------------------------------------------
//  user_output_gain - return the user gain on the given
//  output index of the device
//-------------------------------------------------

float device_sound_interface::user_output_gain(int outputnum) const
{
	auto [stream, output] = output_to_stream_output(outputnum);
	return stream->user_output_gain(output);
}


//-------------------------------------------------
//  set_input_gain - set the gain on the given
//  input index of the device
//-------------------------------------------------

void device_sound_interface::set_input_gain(int inputnum, float gain)
{
	auto [stream, input] = input_to_stream_input(inputnum);
	stream->set_input_gain(input, gain);
}


//-------------------------------------------------
//  set_output_gain - set the gain on the given
//  output index of the device
//-------------------------------------------------

void device_sound_interface::set_output_gain(int outputnum, float gain)
{
	// handle ALL_OUTPUTS as a special case
	if(outputnum == ALL_OUTPUTS)
	{
		if(!outputs())
			fatalerror("Requested setting output gain on sound device %s which has no outputs.", device().tag());
		for(auto *stream : m_sound_streams)
			for(int num = 0; num < stream->output_count(); num++)
				stream->set_output_gain(num, gain);
	}

	// look up the stream and stream output index
	else
	{
		auto [stream, output] = output_to_stream_output(outputnum);
		stream->set_output_gain(output, gain);
	}
}

//-------------------------------------------------
//  user_set_output_gain - set the user gain on the device
//-------------------------------------------------

void device_sound_interface::set_user_output_gain(float gain)
{
	if(!outputs())
		fatalerror("Requested setting user output gain on sound device %s which has no outputs.", device().tag());
	for(auto *stream : m_sound_streams)
		stream->set_user_output_gain(gain);
}



//-------------------------------------------------
//  set_user_output_gain - set the user gain on the given
//  output index of the device
//-------------------------------------------------

void device_sound_interface::set_user_output_gain(int outputnum, float gain)
{
	auto [stream, output] = output_to_stream_output(outputnum);
	stream->set_user_output_gain(output, gain);
}


//-------------------------------------------------
//  set_route_gain - set the gain on a route
//-------------------------------------------------

void device_sound_interface::set_route_gain(int source_channel, device_sound_interface *target, int target_channel, float gain)
{
	auto [sstream, schan] = output_to_stream_output(source_channel);
	auto [tstream, tchan] = target->input_to_stream_input(target_channel);
	tstream->update();
	if(tstream->set_route_gain(sstream, schan, tchan, gain))
		return;

	fatalerror("Trying to change the gain on a non-existant route between %s channel %d and %s channel %d\n", device().tag(), source_channel, target->device().tag(), target_channel);
}



//-------------------------------------------------
//  interface_validity_check - validation for a
//  device after the configuration has been
//  constructed
//-------------------------------------------------

void device_sound_interface::interface_validity_check(validity_checker &valid) const
{
	// loop over all the routes
	for(sound_route const &route : routes())
	{
		// find a device with the requested tag
		device_t const *const target = route.m_base.get().subdevice(route.m_target);
		if(!target)
			osd_printf_error("Attempting to route sound to non-existent device '%s'\n", route.m_base.get().subtag(route.m_target));

		// if it's not a speaker or a sound device, error
		device_sound_interface const *sound;
		if(target && !target->interface(sound))
			osd_printf_error("Attempting to route sound to a non-sound device '%s' (%s)\n", target->tag(), target->name());
	}
}


//-------------------------------------------------
//  interface_pre_start - make sure all our input
//  devices are started
//-------------------------------------------------

void device_sound_interface::sound_before_devices_init()
{
	for(sound_route &route : routes()) {
		device_t *dev = route.m_base.get().subdevice(route.m_target);
		dev->interface(route.m_interface);
		if(route.m_output != ALL_OUTPUTS && m_sound_requested_outputs <= route.m_output) {
			m_sound_requested_outputs_mask |= u64(1) << route.m_output;
			m_sound_requested_outputs = route.m_output + 1;
		}
		route.m_interface->sound_request_input(route.m_input);
	}
}

void device_sound_interface::sound_after_devices_init()
{
	for(sound_route &route : routes()) {
		auto [si, ii] = route.m_interface->input_to_stream_input(route.m_input);
		if(!si)
			fatalerror("Requesting sound route to device %s input %d which doesn't exist\n", route.m_interface->device().tag(), route.m_input);
		if(route.m_output != ALL_OUTPUTS) {
			auto [so, io] = output_to_stream_output(route.m_output);
			if(!so)
				fatalerror("Requesting sound route from device %s output %d which doesn't exist\n", device().tag(), route.m_output);
			si->add_bw_route(so, io, ii, route.m_gain);
			so->add_fw_route(si, ii, io);

		} else {
			for(sound_stream *so : m_sound_streams)
				for(int io = 0; io != so->output_count(); io ++) {
					si->add_bw_route(so, io, ii, route.m_gain);
					so->add_fw_route(si, ii, io);
				}
		}
	}
}

void device_sound_interface::sound_request_input(u32 input)
{
	m_sound_requested_inputs_mask |= u64(1) << input;
	if(m_sound_requested_inputs <= input)
		m_sound_requested_inputs = input + 1;
}

device_mixer_interface::device_mixer_interface(const machine_config &mconfig, device_t &device) :
	device_sound_interface(mconfig, device)
{
}

device_mixer_interface::~device_mixer_interface()
{
}

void device_mixer_interface::interface_pre_start()
{
	u32 ni = get_sound_requested_inputs();
	u32 no = get_sound_requested_outputs();
	u32 nc = ni > no ? ni : no;
	for(u32 i = 0; i != nc; i++)
		stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
}

void device_mixer_interface::sound_stream_update(sound_stream &stream)
{
	stream.copy(0, 0);
}

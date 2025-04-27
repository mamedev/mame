// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    speaker.cpp

    Speaker output sound device.
    Microphone input sound device.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "speaker.h"



DEFINE_DEVICE_TYPE(SPEAKER, speaker_device, "speaker", "Speaker")
DEFINE_DEVICE_TYPE(MICROPHONE, microphone_device, "microphone", "Microphone")

const sound_io_device::position_name_mapping sound_io_device::position_name_mappings[] = {
	{  0.0,  0.0,  1.0, "Front center" },
	{ -0.2,  0.0,  1.0, "Front left"   },
	{  0.0, -0.5,  1.0, "Front floor" },
	{  0.2,  0.0,  1.0, "Front right" },
	{  0.0,  0.0, -0.5, "Rear center" },
	{ -0.2,  0.0, -0.5, "Rear left" },
	{  0.2,  0.0, -0.5, "Read right" },
	{  0.0,  0.0, -0.1, "Headrest center" },
	{ -0.1,  0.0, -0.1, "Headrest left" },
	{  0.1,  0.0, -0.1, "Headrest right" },
	{  0.0, -0.5,  0.0, "Seat" },
	{  0.0, -0.2,  0.1, "Backrest" },
	{ }
};

std::string sound_io_device::get_position_name(u32 channel) const
{
	for(unsigned int i = 0; position_name_mappings[i].m_name; i++)
		if(m_positions[channel][0] == position_name_mappings[i].m_x && m_positions[channel][1] == position_name_mappings[i].m_y && m_positions[channel][2] == position_name_mappings[i].m_z)
			return position_name_mappings[i].m_name;
	return util::string_format("#%d", channel);
}

sound_io_device &sound_io_device::set_position(u32 channel, double x, double y, double z)
{
	if(channel >= m_positions.size())
		fatalerror("%s: Requested channel %d on %d channel device\n", tag(), channel, m_positions.size());
	m_positions[channel][0] = x;
	m_positions[channel][1] = y;
	m_positions[channel][2] = z;
	return *this;
}

sound_io_device::sound_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 channels) :
	device_t(mconfig, type, tag, owner, 0),
	device_sound_interface(mconfig, *this),
	m_positions(channels ? channels : 1)
{
}


sound_io_device::~sound_io_device()
{
}

speaker_device::~speaker_device()
{
}

microphone_device::~microphone_device()
{
}


void speaker_device::device_start()
{
	m_stream = stream_alloc(m_positions.size(), 0, machine().sample_rate());
}

void microphone_device::device_start()
{
	m_stream = stream_alloc(0, m_positions.size(), machine().sample_rate());
}

void speaker_device::sound_stream_update(sound_stream &stream)
{
	machine().sound().output_push(m_id, stream);
}

void microphone_device::sound_stream_update(sound_stream &stream)
{
	machine().sound().input_get(m_id, stream);
}

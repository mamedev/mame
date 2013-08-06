/***************************************************************************

  PeT mess@utanet.at

***************************************************************************/

#include "emu.h"
#include "sound/s2636.h"

const device_type S2636_SOUND = &device_creator<s2636_sound_device>;

s2636_sound_device::s2636_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, S2636_SOUND, "S2636", tag, owner, clock, "s2636", __FILE__),
		device_sound_interface(mconfig, *this),
		m_channel(NULL),
		m_size(0),
		m_pos(0),
		m_level(0)
{
	for (int i = 0; i < 1; i++)
	m_reg[i] = 0;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void s2636_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s2636_sound_device::device_start()
{
	m_channel = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate(), this);
	save_item(NAME(m_size));
	save_item(NAME(m_pos));
	save_item(NAME(m_level));
	
	for (int i = 0; i < 1; i++)
	save_item(NAME(m_reg[i]), i);
}


void s2636_sound_device::soundport_w (int offset, int data)
{
	m_channel->update();
	m_reg[offset] = data;
	switch (offset)
	{
		case 0:
			m_pos = 0;
			m_level = TRUE;
			// frequency 7874/(data+1)
			m_size = machine().sample_rate() * (data + 1) /7874;
			break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void s2636_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int i;
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++, buffer++)
	{
		*buffer = 0;
		if (m_reg[0] && m_pos <= m_size / 2)
		{
			*buffer = 0x7fff;
		}
		if (m_pos <= m_size)
			m_pos++;
		if (m_pos > m_size)
			m_pos = 0;
	}
}

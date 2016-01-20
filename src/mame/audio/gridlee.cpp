// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Basic Gridlee sound driver

*************************************************************************/

#include "emu.h"
#include "includes/gridlee.h"


// device type definition
const device_type GRIDLEE = &device_creator<gridlee_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gridlee_sound_device - constructor
//-------------------------------------------------

gridlee_sound_device::gridlee_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GRIDLEE, "Gridlee Audio Custom", tag, owner, clock, "gridlee_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_tone_step(0),
		m_tone_fraction(0),
		m_tone_volume(0),
		m_stream(nullptr),
		m_samples(nullptr),
		m_freq_to_step(0.0)
{
	memset(m_sound_data, 0, sizeof(UINT8)*24);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gridlee_sound_device::device_start()
{
	/* allocate the stream */
	m_stream = stream_alloc(0, 1, machine().sample_rate());

	m_samples = machine().device<samples_device>("samples");

	m_freq_to_step = (double)(1 << 24) / (double)machine().sample_rate();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gridlee_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (samples--)
	{
		/* tone channel */
		m_tone_fraction += m_tone_step;
		*buffer++ = (m_tone_fraction & 0x0800000) ? (m_tone_volume << 6) : 0;
	}
}



WRITE8_MEMBER( gridlee_sound_device::gridlee_sound_w )
{
	UINT8 *sound_data = m_sound_data;
	samples_device *samples = m_samples;

	m_stream->update();

	switch (offset)
	{
		case 0x04:
			if (data == 0xef && sound_data[offset] != 0xef)
				samples->start(4, 1);
			else if (data != 0xef && sound_data[offset] == 0xef)
				samples->stop(4);
//          if (!(data & 0x01) && (sound_data[offset] & 0x01))
//              samples->start(5, 1);
//          else if ((data & 0x01) && !(sound_data[offset] & 0x01))
//              samples->stop(5);
			break;

		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			if ((data & 1) && !(sound_data[offset] & 1))
				samples->start(offset - 0x0c, 1 - sound_data[offset - 4]);
			else if (!(data & 1) && (sound_data[offset] & 1))
				samples->stop(offset - 0x0c);
			break;

		case 0x08+0x08:
			if (data)
				m_tone_step = m_freq_to_step * (double)(data * 5);
			else
				m_tone_step = 0;
			break;

		case 0x09+0x08:
			m_tone_volume = data;
			break;

		case 0x0b+0x08:
//          tone_volume = (data | sound_data[0x0c+0x08]) ? 0xff : 0x00;
			break;

		case 0x0c+0x08:
//          tone_volume = (data | sound_data[0x0b+0x08]) ? 0xff : 0x00;
			break;

		case 0x0d+0x08:
//          if (data)
//              tone_step = freq_to_step * (double)(data * 11);
//          else
//              tone_step = 0;
			break;
	}
	sound_data[offset] = data;



#if 0
{
static int first = 1;
FILE *f;
f = fopen("sound.log", first ? "w" : "a");
first = 0;
fprintf(f, "[%02x=%02x] %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\n",
	offset,data,
	sound_data[0],
	sound_data[1],
	sound_data[2],
	sound_data[3],
	sound_data[4],
	sound_data[5],
	sound_data[6],
	sound_data[7],
	sound_data[8],
	sound_data[9],
	sound_data[10],
	sound_data[11],
	sound_data[12],
	sound_data[13],
	sound_data[14],
	sound_data[15],
	sound_data[16],
	sound_data[17],
	sound_data[18],
	sound_data[19],
	sound_data[20],
	sound_data[21],
	sound_data[22],
	sound_data[23]);
fclose(f);
}
#endif
}

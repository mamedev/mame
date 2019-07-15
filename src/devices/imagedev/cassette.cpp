// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    cassette.cpp

    Interface to the cassette image abstraction code

*********************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "cassette.h"
#include "ui/uimain.h"

#define LOG_WARN          (1U<<1)   // Warnings
#define LOG_DETAIL        (1U<<2)   // Details

#define VERBOSE ( LOG_WARN )

#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(CASSETTE, cassette_image_device, "cassette_image", "Cassette")

//-------------------------------------------------
//  cassette_image_device - constructor
//-------------------------------------------------

cassette_image_device::cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CASSETTE, tag, owner, clock),
	device_image_interface(mconfig, *this),
	device_sound_interface(mconfig, *this),
	m_cassette(nullptr),
	m_state(CASSETTE_STOPPED),
	m_position(0),
	m_position_time(0),
	m_value(0),
	m_channel(0),
	m_speed(0),
	m_direction(0),
	m_formats(cassette_default_formats),
	m_create_opts(nullptr),
	m_default_state(CASSETTE_PLAY),
	m_interface(nullptr),
	m_stereo(false)
{
}

//-------------------------------------------------
//  cassette_image_device - destructor
//-------------------------------------------------

cassette_image_device::~cassette_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cassette_image_device::device_config_complete()
{
	m_extension_list[0] = '\0';
	for (int i = 0; m_formats[i]; i++ )
		image_specify_extension( m_extension_list, 256, m_formats[i]->extensions );
}


/*********************************************************************
    cassette IO
*********************************************************************/

bool cassette_image_device::is_motor_on()
{
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED)
		return false;
	if ((m_state & CASSETTE_MASK_MOTOR) != CASSETTE_MOTOR_ENABLED)
		return false;
	else
		return true;
}



void cassette_image_device::update()
{
	double cur_time = machine().time().as_double();

	if (is_motor_on())
	{
		double new_position = m_position + (cur_time - m_position_time)*m_speed*m_direction;

		switch(m_state & CASSETTE_MASK_UISTATE) {
		case CASSETTE_RECORD:
			cassette_put_sample(m_cassette, m_channel, m_position, new_position - m_position, m_value);
			break;

		case CASSETTE_PLAY:
			if ( m_cassette )
			{
				cassette_get_sample(m_cassette, m_channel, new_position, 0.0, &m_value);
				/* See if reached end of tape */
				double length = get_length();
				if (new_position > length)
				{
					m_state = (cassette_state)(( m_state & ~CASSETTE_MASK_UISTATE ) | CASSETTE_STOPPED);
					new_position = length;
				}
				else if (new_position < 0)
				{
					m_state = (cassette_state)(( m_state & ~CASSETTE_MASK_UISTATE ) | CASSETTE_STOPPED);
					new_position = 0;
				}
			}
			break;
		}
		m_position = new_position;
	}
	m_position_time = cur_time;
}

void cassette_image_device::change_state(cassette_state state, cassette_state mask)
{
	cassette_state new_state;

	new_state = m_state;
	new_state = (cassette_state)(new_state & ~mask);
	new_state = (cassette_state)(new_state | (state & mask));

	if (new_state != m_state)
	{
		update();
		m_state = new_state;
	}
}



double cassette_image_device::input()
{
	int32_t sample;
	double double_value;

	update();
	sample = m_value;
	double_value = sample / ((double) 0x7FFFFFFF);

	LOGMASKED(LOG_DETAIL, "cassette_input(): time_index=%g value=%g\n", m_position, double_value);

	return double_value;
}



void cassette_image_device::output(double value)
{
	if (((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD) && (m_value != value))
	{
		update();

		value = std::min(value, 1.0);
		value = std::max(value, -1.0);

		m_value = (int32_t) (value * 0x7FFFFFFF);
	}
}


double cassette_image_device::get_position()
{
	double position = m_position;

	if (is_motor_on())
		position += (machine().time().as_double() - m_position_time)*m_speed*m_direction;
	return position;
}



double cassette_image_device::get_length()
{
	struct CassetteInfo info;

	cassette_get_info(m_cassette, &info);
	return ((double) info.sample_count) / info.sample_frequency;
}

void cassette_image_device::set_channel(int channel)
{
	m_channel = channel;
}

void cassette_image_device::set_speed(double speed)
{
	m_speed = speed;
}

void cassette_image_device::go_forward()
{
	m_direction = 1;
}

void cassette_image_device::go_reverse()
{
	m_direction = -1;
}


void cassette_image_device::seek(double time, int origin)
{
	double length;

	update();

	length = get_length();

	switch(origin) {
	case SEEK_SET:
		break;

	case SEEK_END:
		time += length;
		break;

	case SEEK_CUR:
		time += get_position();
		break;
	}

	/* clip position into legal bounds */
	if (time < 0)
		time = 0;
	else
	if (time > length)
		time = length;

	m_position = time;
}



/*********************************************************************
    cassette device init/load/unload/specify
*********************************************************************/

void cassette_image_device::device_start()
{
	/* set to default state */
	m_cassette = nullptr;
	m_state = m_default_state;
	m_value = 0;

	machine().sound().stream_alloc(*this, 0, (m_stereo? 2:1), machine().sample_rate());
}

image_init_result cassette_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	return internal_load(true);
}

image_init_result cassette_image_device::call_load()
{
	return internal_load(false);
}

image_init_result cassette_image_device::internal_load(bool is_create)
{
	cassette_image::error err;
	device_image_interface *image = nullptr;
	interface(image);

	if (is_create)
	{
		// creating an image
		err = cassette_create((void *)image, &image_ioprocs, &wavfile_format, m_create_opts, CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT, &m_cassette);
		if (err != cassette_image::error::SUCCESS)
			goto error;
	}
	else
	{
		// opening an image
		bool retry;
		do
		{
			// we probably don't want to retry...
			retry = false;

			// try opening the cassette
			int cassette_flags = is_readonly()
				? CASSETTE_FLAG_READONLY
				: (CASSETTE_FLAG_READWRITE | CASSETTE_FLAG_SAVEONEXIT);
			err = cassette_open_choices((void *)image, &image_ioprocs, filetype(), m_formats, cassette_flags, &m_cassette);

			// special case - if we failed due to readwrite not being supported, make the image be read only and retry
			if (err == cassette_image::error::READ_WRITE_UNSUPPORTED)
			{
				make_readonly();
				retry = true;
			}
		}
		while(retry);

		if (err != cassette_image::error::SUCCESS)
			goto error;
	}

	/* set to default state, but only change the UI state */
	change_state(m_default_state, CASSETTE_MASK_UISTATE);

	/* reset the position */
	m_position = 0.0;
	m_position_time = machine().time().as_double();

	/* default channel to 0, speed multiplier to 1 */
	m_channel = 0;
	m_speed = 1;
	m_direction = 1;

	return image_init_result::PASS;

error:
	image_error_t imgerr = IMAGE_ERROR_UNSPECIFIED;
	switch(err)
	{
		case cassette_image::error::INTERNAL:
			imgerr = IMAGE_ERROR_INTERNAL;
			break;
		case cassette_image::error::UNSUPPORTED:
			imgerr = IMAGE_ERROR_UNSUPPORTED;
			break;
		case cassette_image::error::OUT_OF_MEMORY:
			imgerr = IMAGE_ERROR_OUTOFMEMORY;
			break;
		case cassette_image::error::INVALID_IMAGE:
			imgerr = IMAGE_ERROR_INVALIDIMAGE;
			break;
		default:
			imgerr = IMAGE_ERROR_UNSPECIFIED;
			break;
	}
	image->seterror(imgerr, "" );
	return image_init_result::FAIL;
}



void cassette_image_device::call_unload()
{
	/* if we are recording, write the value to the image */
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
		update();

	/* close out the cassette */
	cassette_close(m_cassette);
	m_cassette = nullptr;

	/* set to default state, but only change the UI state */
	change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}


//-------------------------------------------------
//  display a small tape animation, with the
//  current position in the tape image
//-------------------------------------------------

std::string cassette_image_device::call_display()
{
	const int ANIMATION_FPS = 1;

	std::string result;

	// only show the image when a cassette is loaded and the motor is on
	if (exists() && is_motor_on())
	{
		int n;
		double position, length;
		cassette_state uistate;
		static char const *const shapes[] = { u8"\u2500", u8"\u2572", u8"\u2502", u8"\u2571" };

		// figure out where we are in the cassette
		position = get_position();
		length = get_length();
		uistate = (cassette_state)(get_state() & CASSETTE_MASK_UISTATE);

		// choose which frame of the animation we are at
		n = ((int)position / ANIMATION_FPS) % ARRAY_LENGTH(shapes);

		// play or record
		const char *status_icon = (uistate == CASSETTE_PLAY)
			? u8"\u25BA"
			: u8"\u25CF";

		// Since you can have anything in a BDF file, we will use crude ascii characters instead
		result = string_format("%s %s %02d:%02d (%04d) [%02d:%02d (%04d)]",
			shapes[n],                  // animation
			status_icon,                // play or record
			((int)position / 60),
			((int)position % 60),
			(int)position,
			((int)length / 60),
			((int)length % 60),
			(int)length);

		// make sure tape stops at end when playing
		if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
		{
			if (m_cassette)
			{
				if (get_position() > get_length())
				{
					m_state = (cassette_state)((m_state & ~CASSETTE_MASK_UISTATE) | CASSETTE_STOPPED);
				}
			}
		}
	}
	return result;
}

//-------------------------------------------------
//  Cassette sound
//-------------------------------------------------

void cassette_image_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	cassette_state state;
	double time_index;
	double duration;
	stream_sample_t *left_buffer = outputs[0];
	stream_sample_t *right_buffer = nullptr;
	int i;

	if (m_stereo)
		right_buffer = outputs[1];

	state = (cassette_state)(get_state() & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER));

	if (exists() && (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)))
	{
		cassette_image *cassette = get_image();
		time_index = get_position();
		duration = ((double) samples) / machine().sample_rate();

		cassette_get_samples(cassette, 0, time_index, duration, samples, 2, left_buffer, CASSETTE_WAVEFORM_16BIT);
		if (m_stereo)
			cassette_get_samples(cassette, 1, time_index, duration, samples, 2, right_buffer, CASSETTE_WAVEFORM_16BIT);

		for (i = samples - 1; i >= 0; i--)
		{
			left_buffer[i] = ((int16_t *) left_buffer)[i];
			if (m_stereo)
				right_buffer[i] = ((int16_t *) right_buffer)[i];
		}
	}
	else
	{
		memset(left_buffer, 0, sizeof(*left_buffer) * samples);
		if (m_stereo)
			memset(right_buffer, 0, sizeof(*right_buffer) * samples);
	}
}

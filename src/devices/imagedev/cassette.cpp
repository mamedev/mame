// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    cassette.cpp

    Interface to the cassette image abstraction code

*********************************************************************/

#include "emu.h"
#include "cassette.h"
#include "softlist_dev.h"

#include "formats/imageutl.h"

#include "util/ioprocs.h"
#include "util/ioprocsfilter.h"

#include <regex>

#define LOG_WARN          (1U << 1)   // Warnings
#define LOG_DETAIL        (1U << 2)   // Details

#define VERBOSE (LOG_WARN)

#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(CASSETTE, cassette_image_device, "cassette_image", "Cassette")

//-------------------------------------------------
//  cassette_image_device - constructor
//-------------------------------------------------

cassette_image_device::cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CASSETTE, tag, owner, clock),
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

void cassette_image_device::update()
{
	double cur_time = machine().time().as_double();

	if (!is_stopped() && motor_on())
	{
		double new_position = m_position + (cur_time - m_position_time)*m_speed*m_direction;

		switch (int(m_state & CASSETTE_MASK_UISTATE)) // cast to int to suppress unhandled enum value warning
		{
		case CASSETTE_RECORD:
			m_cassette->put_sample(m_channel, m_position, new_position - m_position, m_value);
			break;

		case CASSETTE_PLAY:
			if (m_cassette)
			{
				m_cassette->get_sample(m_channel, new_position, 0.0, &m_value);
				// See if reached end of tape
				double length = get_length();
				if (new_position > length)
				{
					m_state = (m_state & ~CASSETTE_MASK_UISTATE) | CASSETTE_STOPPED;
					new_position = length;
				}
				else if (new_position < 0)
				{
					m_state = (m_state & ~CASSETTE_MASK_UISTATE) | CASSETTE_STOPPED;
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
	new_state &= ~mask;
	new_state |= (state & mask);

	if (new_state != m_state)
	{
		update();
		m_state = new_state;
	}
}



double cassette_image_device::input()
{
	update();
	int32_t sample = m_value;
	double double_value = sample / (double(0x7FFFFFFF));

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

		m_value = int32_t(value * 0x7FFFFFFF);
	}
}


double cassette_image_device::get_position()
{
	double position = m_position;

	if (!is_stopped() && motor_on())
		position += (machine().time().as_double() - m_position_time)*m_speed*m_direction;
	return position;
}



double cassette_image_device::get_length()
{
	cassette_image::Info info = m_cassette->get_info();
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
	update();

	double length = get_length();

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

	stream_alloc(0, m_stereo? 2:1, machine().sample_rate());
}

const software_list_loader &cassette_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}

std::pair<std::error_condition, std::string> cassette_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	return std::make_pair(internal_load(true), std::string());
}

std::pair<std::error_condition, std::string> cassette_image_device::call_load()
{
	return std::make_pair(internal_load(false), std::string());
}

bool cassette_image_device::has_any_extension(std::string_view candidate_extensions) const
{
	const char separator = ',';
	std::istringstream extension_stream(std::string{candidate_extensions});
	for (std::string extension; std::getline(extension_stream, extension, separator);)
		if (is_filetype(extension))
			return true;
	return false;
}

std::error_condition cassette_image_device::internal_load(bool is_create)
{
	cassette_image::error err;
	device_image_interface *image = nullptr;
	interface(image);

	check_for_file();
	if (is_create || (length()==0)) // empty existing images are fine to write over.
	{
		auto io = util::random_read_write_fill(image_core_file(), 0x00);
		if (io)
		{
			err = cassette_image::create(
					std::move(io),
					has_any_extension(cassette_image::flacfile_format.extensions) ? &cassette_image::flacfile_format : &cassette_image::wavfile_format,
					m_create_opts,
					cassette_image::FLAG_READWRITE|cassette_image::FLAG_SAVEONEXIT,
					m_cassette);
		}
		else
		{
			err = cassette_image::error::OUT_OF_MEMORY;
		}
	}
	else
	{
		// opening an image
		bool retry;
		do
		{
			// we probably don't want to retry...
			retry = false;

			auto io = util::random_read_write_fill(image_core_file(), 0x00);
			if (io)
			{
				// try opening the cassette
				int const cassette_flags = is_readonly()
						? cassette_image::FLAG_READONLY
						: (cassette_image::FLAG_READWRITE | cassette_image::FLAG_SAVEONEXIT);
				err = cassette_image::open_choices(
						std::move(io),
						filetype(),
						m_formats,
						cassette_flags,
						m_cassette);
			}
			else
			{
				err = cassette_image::error::OUT_OF_MEMORY;
			}

			// special case - if we failed due to readwrite not being supported, make the image be read only and retry
			if (err == cassette_image::error::READ_WRITE_UNSUPPORTED)
			{
				make_readonly();
				retry = true;
			}
		}
		while(retry);
	}

	if (err == cassette_image::error::SUCCESS)
	{
		/* set to default state, but only change the UI state */
		change_state(m_default_state, CASSETTE_MASK_UISTATE);

		/* reset the position */
		m_position = 0.0;
		m_position_time = machine().time().as_double();

		/* default channel to 0, speed multiplier to 1 */
		m_channel = 0;
		m_speed = 1;
		m_direction = 1;

		return std::error_condition();
	}
	else
	{
		std::error_condition imgerr = image_error::UNSPECIFIED;
		switch(err)
		{
			case cassette_image::error::INTERNAL:
				imgerr = image_error::INTERNAL;
				break;
			case cassette_image::error::UNSUPPORTED:
				imgerr = image_error::UNSUPPORTED;
				break;
			case cassette_image::error::OUT_OF_MEMORY:
				imgerr = std::errc::not_enough_memory;
				break;
			case cassette_image::error::INVALID_IMAGE:
				imgerr = image_error::INVALIDIMAGE;
				break;
			default:
				imgerr = image_error::UNSPECIFIED;
				break;
		}
		return imgerr;
	}
}



void cassette_image_device::call_unload()
{
	/* if we are recording, write the value to the image */
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
		update();

	/* close out the cassette */
	m_cassette.reset();

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
	if (exists() && !is_stopped() && motor_on())
	{
		static char const *const shapes[] = { u8"\u2500", u8"\u2572", u8"\u2502", u8"\u2571" };

		// figure out where we are in the cassette
		double position = get_position();
		double length = get_length();
		cassette_state uistate = get_state() & CASSETTE_MASK_UISTATE;

		// choose which frame of the animation we are at
		int n = (int(position) / ANIMATION_FPS) % std::size(shapes);

		// play or record
		const char *status_icon = (uistate == CASSETTE_PLAY)
			? u8"\u25BA"
			: u8"\u25CF";

		// create information string
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
					m_state = ((m_state & ~CASSETTE_MASK_UISTATE) | CASSETTE_STOPPED);
				}
			}
		}
	}
	return result;
}

//-------------------------------------------------
//  Cassette sound
//-------------------------------------------------

void cassette_image_device::sound_stream_update(sound_stream &stream)
{
	cassette_state state = get_state() & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER);

	if (exists() && (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED)))
	{
		int samples = stream.samples();
		cassette_image *cassette = get_image();
		double time_index = get_position();
		double duration = ((double) samples) / stream.sample_rate();

		if (m_samples.size() < samples)
			m_samples.resize(samples);

		const cassette_image::Info info = cassette->get_info();
		for (int ch = 0; ch < stream.output_count(); ch++)
		{
			if (ch < info.channels)
				cassette->get_samples(ch, time_index, duration, samples, 2, &m_samples[0], cassette_image::WAVEFORM_16BIT);
			else
				cassette->get_samples(0, time_index, duration, samples, 2, &m_samples[0], cassette_image::WAVEFORM_16BIT);
			for (int sampindex = 0; sampindex < samples; sampindex++)
				stream.put_int(ch, sampindex, m_samples[sampindex], 32768);
		}
	}
}

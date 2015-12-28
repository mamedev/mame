// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/*********************************************************************

    cassette.c

    Interface to the cassette image abstraction code

*********************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "cassette.h"
#include "ui/ui.h"
#include "image.h"


#define ANIMATION_FPS       1
#define ANIMATION_FRAMES    4

#define VERBOSE             0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

// device type definition
const device_type CASSETTE = &device_creator<cassette_image_device>;

//-------------------------------------------------
//  cassette_image_device - constructor
//-------------------------------------------------

cassette_image_device::cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CASSETTE, "Cassette", tag, owner, clock, "cassette_image", __FILE__),
	device_image_interface(mconfig, *this),
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
	m_interface(nullptr)
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

	// set brief and instance name
	update_names();
}


/*********************************************************************
    cassette IO
*********************************************************************/

bool cassette_image_device::is_motor_on()
{
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED)
		return FALSE;
	if ((m_state & CASSETTE_MASK_MOTOR) != CASSETTE_MOTOR_ENABLED)
		return FALSE;
	return TRUE;
}



void cassette_image_device::update()
{
	double cur_time = device().machine().time().as_double();

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
	INT32 sample;
	double double_value;

	update();
	sample = m_value;
	double_value = sample / ((double) 0x7FFFFFFF);

	LOG(("cassette_input(): time_index=%g value=%g\n", m_position, double_value));

	return double_value;
}



void cassette_image_device::output(double value)
{
	if (((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD) && (m_value != value))
	{
		update();

		value = MIN(value, 1.0);
		value = MAX(value, -1.0);

		m_value = (INT32) (value * 0x7FFFFFFF);
	}
}


double cassette_image_device::get_position()
{
	double position = m_position;

	if (is_motor_on())
		position += (device().machine().time().as_double() - m_position_time)*m_speed*m_direction;
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
}

bool cassette_image_device::call_create(int format_type, option_resolution *format_options)
{
	return call_load();
}

bool cassette_image_device::call_load()
{
	casserr_t err;
	int cassette_flags;
	const char *extension;
	int is_writable;
	device_image_interface *image = nullptr;
	interface(image);

	if ((has_been_created()) || (length() == 0))
	{
		/* creating an image */
		err = cassette_create((void *)image, &image_ioprocs, &wavfile_format, m_create_opts, CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT, &m_cassette);
		if (err)
			goto error;
	}
	else
	{
		/* opening an image */
		do
		{
			is_writable = !is_readonly();
			cassette_flags = is_writable ? (CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT) : CASSETTE_FLAG_READONLY;
			std::string fname;
			if (software_entry()==nullptr) {
				extension = filetype();
			} else {
				fname = m_mame_file->filename();
				int loc = fname.find_last_of('.');
				if (loc!=-1) {
					extension = fname.substr(loc + 1,fname.length()-loc).c_str();
				} else {
					extension = "";
				}
			}
			err = cassette_open_choices((void *)image, &image_ioprocs, extension, m_formats, cassette_flags, &m_cassette);

			/* this is kind of a hack */
			if (err && is_writable)
				make_readonly();
		}
		while(err && is_writable);

		if (err)
			goto error;
	}

	/* set to default state, but only change the UI state */
	change_state(m_default_state, CASSETTE_MASK_UISTATE);

	/* reset the position */
	m_position = 0.0;
	m_position_time = device().machine().time().as_double();

	/* default channel to 0, speed multiplier to 1 */
	m_channel = 0;
	m_speed = 1;
	m_direction = 1;

	return IMAGE_INIT_PASS;

error:
	image_error_t imgerr = IMAGE_ERROR_UNSPECIFIED;
	switch(err)
	{
		case CASSETTE_ERROR_INTERNAL:
			imgerr = IMAGE_ERROR_INTERNAL;
			break;
		case CASSETTE_ERROR_UNSUPPORTED:
			imgerr = IMAGE_ERROR_UNSUPPORTED;
			break;
		case CASSETTE_ERROR_OUTOFMEMORY:
			imgerr = IMAGE_ERROR_OUTOFMEMORY;
			break;
		case CASSETTE_ERROR_INVALIDIMAGE:
			imgerr = IMAGE_ERROR_INVALIDIMAGE;
			break;
		default:
			imgerr = IMAGE_ERROR_UNSPECIFIED;
			break;
	}
	image->seterror(imgerr, "" );
	return IMAGE_INIT_FAIL;
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



/*
    display a small tape icon, with the current position in the tape image
*/
void cassette_image_device::call_display()
{
	char buf[65];
	float x, y;
	int n;
	double position, length;
	cassette_state uistate;
	cassette_image_device *dev;
	static const UINT8 shapes[8] = { 0x2d, 0x5c, 0x7c, 0x2f, 0x2d, 0x20, 0x20, 0x20 };

	/* abort if we should not be showing the image */
	if (!exists())
		return;
	if (!is_motor_on())
		return;

	/* figure out where we are in the cassette */
	position = get_position();
	length = get_length();
	uistate = (cassette_state)(get_state() & CASSETTE_MASK_UISTATE);

	/* choose a location on the screen */
	x = 0.2f;
	y = 0.5f;

	cassette_device_iterator iter(device().machine().root_device());
	for (dev = iter.first(); dev != nullptr && strcmp( dev->tag(), device().tag() ); dev = iter.next())
		y += 1;

	y *= device().machine().ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
	/* choose which frame of the animation we are at */
	n = ((int) position / ANIMATION_FPS) % ANIMATION_FRAMES;
	/* Since you can have anything in a BDF file, we will use crude ascii characters instead */
	snprintf(buf, ARRAY_LENGTH(buf), "%c%c %c %02d:%02d (%04d) [%02d:%02d (%04d)]",
#if 0
	/* THE ANIMATION HASN'T WORKED SINCE 0.114 - LEFT HERE FOR REFERENCE */
	/* NEVER SEEN THE PLAY / RECORD ICONS */
	/* character pairs 2-3, 4-5, 6-7, 8-9 form little tape cassette images */
		n * 2 + 2,                              /* cassette icon left */
		n * 2 + 3,                              /* cassette icon right */
		(uistate == CASSETTE_PLAY) ? 16 : 14,   /* play or record icon */
#else
		shapes[n],                  /* cassette icon left */
		shapes[n|4],                    /* cassette icon right */
		(uistate == CASSETTE_PLAY) ? 0x50 : 0x52,   /* play (P) or record (R) */
#endif
		((int) position / 60),
		((int) position % 60),
		(int) position,
		((int) length / 60),
		((int) length % 60),
		(int) length);

	// draw the cassette
	device().machine().ui().draw_text_box(&device().machine().render().ui_container(), buf, JUSTIFY_LEFT, x, y, UI_BACKGROUND_COLOR);

	// make sure tape stops at end when playing
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
	{
		if ( m_cassette )
		{
			if (position > length)
			{
				m_state = (cassette_state)(( m_state & ~CASSETTE_MASK_UISTATE ) | CASSETTE_STOPPED);
			}
		}
	}
}

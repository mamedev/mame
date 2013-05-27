/*********************************************************************

    cassette.c

    Interface to the cassette image abstraction code

*********************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "cassette.h"


#define ANIMATION_FPS       1
#define ANIMATION_FRAMES    4

#define VERBOSE             0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* Default cassette_interface for drivers only wav files */
const cassette_interface default_cassette_interface =
{
	cassette_default_formats,
	NULL,
	CASSETTE_PLAY,
	NULL,
	NULL
};


// device type definition
const device_type CASSETTE = &device_creator<cassette_image_device>;

//-------------------------------------------------
//  cassette_image_device - constructor
//-------------------------------------------------

cassette_image_device::cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CASSETTE, "Cassette", tag, owner, clock),
		device_image_interface(mconfig, *this)
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
	// inherit a copy of the static data
	const cassette_interface *intf = reinterpret_cast<const cassette_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cassette_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_formats, 0, sizeof(m_formats));
		memset(&m_create_opts, 0, sizeof(m_create_opts));
		memset(&m_default_state, 0, sizeof(m_default_state));
		memset(&m_interface, 0, sizeof(m_interface));
		memset(&m_device_displayinfo, 0, sizeof(m_device_displayinfo));
	}

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
	m_cassette = NULL;
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
	device_image_interface *image = NULL;
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
			astring fname;
			if (software_entry()==NULL) {
				extension = filetype();
			} else {
				fname = m_mame_file->filename();
				int loc = fname.rchr(0,'.');
				if (loc!=-1) {
					extension = fname.substr(loc + 1,fname.len()-loc).cstr();
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
	m_cassette = NULL;

	/* set to default state, but only change the UI state */
	change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}



/*
    display a small tape icon, with the current position in the tape image
*/
void cassette_image_device::call_display()
{
}

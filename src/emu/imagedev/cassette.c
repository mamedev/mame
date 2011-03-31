/*********************************************************************

    cassette.c

    Interface to the cassette image abstraction code

*********************************************************************/

#include "emu.h"
#include "imageutl.h"
#include "cassette.h"
#include "cassimg.h"
#include "ui.h"


#define CASSETTE_TAG		"cassette"
#define ANIMATION_FPS		1
#define ANIMATION_FRAMES	4

#define VERBOSE				0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


typedef struct _dev_cassette_t	dev_cassette_t;
struct _dev_cassette_t
{
	const cassette_config	*config;
	cassette_image	*cassette;
	cassette_state	state;
	double			position;
	double			position_time;
	INT32			value;
};


/* Default cassette_config for drivers only wav files */
const cassette_config default_cassette_config =
{
	cassette_default_formats,
	NULL,
	CASSETTE_PLAY,
	NULL
};


INLINE dev_cassette_t *get_safe_token(device_t *device)
{
	assert( device != NULL );
	assert( device->type() == CASSETTE );
	return (dev_cassette_t *) downcast<legacy_device_base *>(device)->token();
}


/*********************************************************************
    cassette IO
*********************************************************************/

INLINE int cassette_is_motor_on(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );

	if ((cassette->state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED)
		return FALSE;
	if ((cassette->state & CASSETTE_MASK_MOTOR) != CASSETTE_MOTOR_ENABLED)
		return FALSE;
	return TRUE;
}



static void cassette_update(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );
	double cur_time = device->machine().time().as_double();

	if (cassette_is_motor_on(device))
	{
		double new_position = cassette->position + (cur_time - cassette->position_time);

		switch(cassette->state & CASSETTE_MASK_UISTATE) {
		case CASSETTE_RECORD:
			cassette_put_sample(cassette->cassette, 0, cassette->position, new_position - cassette->position, cassette->value);
			break;

		case CASSETTE_PLAY:
			if ( cassette->cassette )
			{
				cassette_get_sample(cassette->cassette, 0, new_position, 0.0, &cassette->value);
				/* See if reached end of tape */
				double length = cassette_get_length(device);
				if (new_position > length)
				{
					cassette->state = (cassette_state)(( cassette->state & ~CASSETTE_MASK_UISTATE ) | CASSETTE_STOPPED);
					new_position = length;
				}
			}
			break;
		}
		cassette->position = new_position;
	}
	cassette->position_time = cur_time;
}



cassette_state cassette_get_state(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );
	return cassette->state;
}



void cassette_change_state(device_t *device, cassette_state state, cassette_state mask)
{
	dev_cassette_t	*cassette = get_safe_token( device );
	cassette_state new_state;

	new_state = cassette->state;
	new_state = (cassette_state)(new_state & ~mask);
	new_state = (cassette_state)(new_state | (state & mask));

	if (new_state != cassette->state)
	{
		cassette_update(device);
		cassette->state = new_state;
	}
}



void cassette_set_state(device_t *device, cassette_state state)
{
	cassette_change_state(device, state, (cassette_state)(~0));
}



double cassette_input(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );
	INT32 sample;
	double double_value;

	cassette_update(device);
	sample = cassette->value;
	double_value = sample / ((double) 0x7FFFFFFF);

	LOG(("cassette_input(): time_index=%g value=%g\n", cassette->position, double_value));

	return double_value;
}



void cassette_output(device_t *device, double value)
{
	dev_cassette_t	*cassette = get_safe_token( device );

	if (((cassette->state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD) && (cassette->value != value))
	{
		cassette_update(device);

		value = MIN(value, 1.0);
		value = MAX(value, -1.0);

		cassette->value = (INT32) (value * 0x7FFFFFFF);
	}
}



cassette_image *cassette_get_image(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );

	return cassette->cassette;
}



double cassette_get_position(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );
	double position;

	position = cassette->position;

	if (cassette_is_motor_on(device))
		position += device->machine().time().as_double() - cassette->position_time;
	return position;
}



double cassette_get_length(device_t *device)
{
	dev_cassette_t	*cassette = get_safe_token( device );
	struct CassetteInfo info;

	cassette_get_info(cassette->cassette, &info);
	return ((double) info.sample_count) / info.sample_frequency;
}



void cassette_seek(device_t *device, double time, int origin)
{
	dev_cassette_t	*cassette = get_safe_token( device );

	double length;

	cassette_update(device);

	length = cassette_get_length(device);

	switch(origin) {
	case SEEK_SET:
		break;

	case SEEK_END:
		time += length;
		break;

	case SEEK_CUR:
		time += cassette_get_position(device);
		break;
	}

	/* clip position into legal bounds */
	if (time < 0)
		time = 0;
	else
	if (time > length)
		time = length;

	cassette->position = time;
}



/*********************************************************************
    cassette device init/load/unload/specify
*********************************************************************/

static DEVICE_START( cassette )
{
	dev_cassette_t	*cassette = get_safe_token( device );

	/* set to default state */
	cassette->config = (const cassette_config*)device->baseconfig().static_config();
	cassette->cassette = NULL;
	cassette->state = cassette->config->default_state;
}



static DEVICE_IMAGE_LOAD( cassette )
{
	device_t *device = &image.device();
	dev_cassette_t	*cassette = get_safe_token( device );
	casserr_t err;
	int cassette_flags;
	const struct CassetteFormat * const *formats;
	const struct CassetteOptions *create_opts;
	const char *extension;
	int is_writable;

	/* figure out the cassette format */
	formats = cassette->config->formats;

	if (image.has_been_created())
	{
		/* creating an image */
		create_opts = cassette->config->create_opts;
		err = cassette_create((void *) &image, &image_ioprocs, &wavfile_format, create_opts, CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT, &cassette->cassette);
		if (err)
			goto error;
	}
	else
	{
		/* opening an image */
		do
		{
			is_writable = image.is_writable();
			cassette_flags = is_writable ? (CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT) : CASSETTE_FLAG_READONLY;
			extension = image.filetype();
			err = cassette_open_choices((void *) &image, &image_ioprocs, extension, formats, cassette_flags, &cassette->cassette);

			/* this is kind of a hack */
			if (err && is_writable)
				image.make_readonly();
		}
		while(err && is_writable);

		if (err)
			goto error;
	}

	/* set to default state, but only change the UI state */
	cassette_change_state(device, cassette->config->default_state, CASSETTE_MASK_UISTATE);

	/* reset the position */
	cassette->position = 0.0;
	cassette->position_time = device->machine().time().as_double();

	return IMAGE_INIT_PASS;

error:
	return IMAGE_INIT_FAIL;
}



static DEVICE_IMAGE_UNLOAD( cassette )
{
	device_t *device = &image.device();
	dev_cassette_t	*cassette = get_safe_token( device );

	/* if we are recording, write the value to the image */
	if ((cassette->state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
		cassette_update(device);

	/* close out the cassette */
	cassette_close(cassette->cassette);
	cassette->cassette = NULL;

	/* set to default state, but only change the UI state */
	cassette_change_state(device, CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}



/*
    display a small tape icon, with the current position in the tape image
*/
static DEVICE_IMAGE_DISPLAY(cassette)
{
	device_t *device = &image.device();
	char buf[65];
	float x, y;
	int n;
	double position, length;
	cassette_state uistate;
	device_t *dev;
	static const UINT8 shapes[8] = { 0x2d, 0x5c, 0x7c, 0x2f, 0x2d, 0x20, 0x20, 0x20 };

	/* abort if we should not be showing the image */
	if (!image.exists())
		return;
	if (!cassette_is_motor_on(device))
		return;

	/* figure out where we are in the cassette */
	position = cassette_get_position(device);
	length = cassette_get_length(device);
	uistate = (cassette_state)(cassette_get_state(device) & CASSETTE_MASK_UISTATE);

	/* choose a location on the screen */
	x = 0.2f;
	y = 0.5f;

	dev = device->machine().m_devicelist.first(CASSETTE );

	while ( dev && strcmp( dev->tag(), device->tag() ) )
	{
		y += 1;
		dev = dev->typenext();
	}

	y *= ui_get_line_height(device->machine()) + 2.0f * UI_BOX_TB_BORDER;
	/* choose which frame of the animation we are at */
	n = ((int) position / ANIMATION_FPS) % ANIMATION_FRAMES;
	/* Since you can have anything in a BDF file, we will use crude ascii characters instead */
	snprintf(buf, ARRAY_LENGTH(buf), "%c%c %c %02d:%02d (%04d) [%02d:%02d (%04d)]",
#if 0
	/* THE ANIMATION HASN'T WORKED SINCE 0.114 - LEFT HERE FOR REFERENCE */
	/* NEVER SEEN THE PLAY / RECORD ICONS */
	/* character pairs 2-3, 4-5, 6-7, 8-9 form little tape cassette images */
		n * 2 + 2,								/* cassette icon left */
		n * 2 + 3,								/* cassette icon right */
		(uistate == CASSETTE_PLAY) ? 16 : 14,	/* play or record icon */
#else
		shapes[n],					/* cassette icon left */
		shapes[n|4],					/* cassette icon right */
		(uistate == CASSETTE_PLAY) ? 0x50 : 0x52,	/* play (P) or record (R) */
#endif
		((int) position / 60),
		((int) position % 60),
		(int) position,
		((int) length / 60),
		((int) length % 60),
		(int) length);

	/* draw the cassette */
	ui_draw_text_box(&device->machine().render().ui_container(), buf, JUSTIFY_LEFT, x, y, UI_BACKGROUND_COLOR);
}

/*-------------------------------------------------
    DEVICE_IMAGE_SOFTLIST_LOAD(cassette)
-------------------------------------------------*/
static DEVICE_IMAGE_SOFTLIST_LOAD(cassette)
{
	return image.load_software(swlist, swname, start_entry);
}

DEVICE_GET_INFO(cassette)
{
	switch( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(dev_cassette_t); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0; break;
		case DEVINFO_INT_IMAGE_TYPE:				info->i = IO_CASSETTE; break;
		case DEVINFO_INT_IMAGE_READABLE:			info->i = 1; break;
		case DEVINFO_INT_IMAGE_WRITEABLE:			info->i = 1; break;
		case DEVINFO_INT_IMAGE_CREATABLE:			info->i = 1; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(cassette); break;
		case DEVINFO_FCT_IMAGE_LOAD:				info->f = (genf *) DEVICE_IMAGE_LOAD_NAME(cassette); break;
		case DEVINFO_FCT_IMAGE_UNLOAD:				info->f = (genf *) DEVICE_IMAGE_UNLOAD_NAME(cassette); break;
		case DEVINFO_FCT_IMAGE_DISPLAY:				info->f = (genf *) DEVICE_IMAGE_DISPLAY_NAME(cassette); break;
		case DEVINFO_FCT_IMAGE_SOFTLIST_LOAD:		info->f = (genf *) DEVICE_IMAGE_SOFTLIST_LOAD_NAME(cassette);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "Cassette"); break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Cassette"); break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:
			if ( device && device->static_config() )
			{
				const struct CassetteFormat * const *formats = ((cassette_config *)device->static_config())->formats;
				int		i;

				/* set up a temporary string */
				info->s[0] = '\0';

				for ( i = 0; formats[i]; i++ )
					image_specify_extension( info->s, 256, formats[i]->extensions );
			}
			break;
		case DEVINFO_STR_IMAGE_INTERFACE:
			if ( device && device->static_config() && ((cassette_config *)device->static_config())->interface)
			{
				strcpy(info->s, ((cassette_config *)device->static_config())->interface );
			}
			break;
	}
}

DEFINE_LEGACY_IMAGE_DEVICE(CASSETTE, cassette);

/*********************************************************************

    ds1315.c

    Dallas Semiconductor's Phantom Time Chip DS1315.

    by tim lindner, November 2001.

*********************************************************************/

#include "ds1315.h"
#include "coreutil.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef enum
{
	DS_SEEK_MATCHING,
	DS_CALENDAR_IO
} ds1315_mode_t;


typedef struct _ds1315_t ds1315_t;
struct _ds1315_t
{
	int count;
	ds1315_mode_t mode;
	UINT8 raw_data[8*8];
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static const UINT8 ds1315_pattern[] =
{
	1, 0, 1, 0, 0, 0, 1, 1,
	0, 1, 0, 1, 1, 1, 0, 0,
	1, 1, 0, 0, 0, 1, 0, 1,
	0, 0, 1, 1, 1, 0, 1, 0,
	1, 0, 1, 0, 0, 0, 1, 1,
	0, 1, 0, 1, 1, 1, 0, 0,
	1, 1, 0, 0, 0, 1, 0, 1,
	0, 0, 1, 1, 1, 0, 1, 0
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void ds1315_fill_raw_data(device_t *device);
static void ds1315_input_raw_data(device_t *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE ds1315_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DS1315);
	return (ds1315_t *) downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START( ds1315 )
-------------------------------------------------*/

static DEVICE_START(ds1315)
{
	ds1315_t *ds1315 = get_token(device);

	memset(ds1315, 0, sizeof(*ds1315));
	ds1315->count = 0;
	ds1315->mode = DS_SEEK_MATCHING;
}


/*-------------------------------------------------
    DEVICE_START( ds1315 )
-------------------------------------------------*/

READ8_DEVICE_HANDLER( ds1315_r_0 )
{
	ds1315_t *ds1315 = get_token(device);

	if (ds1315_pattern[ds1315->count++] == 0)
	{
		if (ds1315->count == 64)
		{
			/* entire pattern matched */
			ds1315->count = 0;
			ds1315->mode = DS_CALENDAR_IO;
			ds1315_fill_raw_data(device);
		}

		return 0;
	}

	ds1315->count = 0;
	ds1315->mode = DS_SEEK_MATCHING;
	return 0;
}


/*-------------------------------------------------
    ds1315_r_1
-------------------------------------------------*/

READ8_DEVICE_HANDLER ( ds1315_r_1 )
{
	ds1315_t *ds1315 = get_token(device);

	if (ds1315_pattern[ ds1315->count++ ] == 1)
	{
		ds1315->count %= 64;
		return 0;
	}

	ds1315->count = 0;
	ds1315->mode = DS_SEEK_MATCHING;
	return 0;
}


/*-------------------------------------------------
    ds1315_r_data
-------------------------------------------------*/

READ8_DEVICE_HANDLER ( ds1315_r_data )
{
	UINT8 result;
	ds1315_t *ds1315 = get_token(device);

	if (ds1315->mode == DS_CALENDAR_IO)
	{
		result = ds1315->raw_data[ ds1315->count++ ];

		if (ds1315->count == 64)
		{
			ds1315->mode = DS_SEEK_MATCHING;
			ds1315->count = 0;
		}

		return result;
	}

	ds1315->count = 0;
	return 0;
}


/*-------------------------------------------------
    ds1315_w_data
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER ( ds1315_w_data )
{
	ds1315_t *ds1315 = get_token(device);

	if (ds1315->mode == DS_CALENDAR_IO)
	{
		ds1315->raw_data[ds1315->count++] = data & 0x01;

		if (ds1315->count == 64)
		{
			ds1315->mode = DS_SEEK_MATCHING;
			ds1315->count = 0;
			ds1315_input_raw_data(device);
		}
		return;
	}

	ds1315->count = 0;
}


/*-------------------------------------------------
    ds1315_fill_raw_data
-------------------------------------------------*/

static void ds1315_fill_raw_data(device_t *device)
{
	/* This routine will (hopefully) call a standard 'C' library routine to get the current
       date and time and then fill in the raw data struct.
    */

	system_time systime;
	ds1315_t *ds1315 = get_token(device);
	int raw[8], i, j;

	/* get the current date/time from the core */
	device->machine().current_datetime(systime);

	raw[0] = 0;	/* tenths and hundreths of seconds are always zero */
	raw[1] = dec_2_bcd(systime.local_time.second);
	raw[2] = dec_2_bcd(systime.local_time.minute);
	raw[3] = dec_2_bcd(systime.local_time.hour);

	raw[4] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
	raw[5] = dec_2_bcd(systime.local_time.mday);
	raw[6] = dec_2_bcd(systime.local_time.month + 1);
	raw[7] = dec_2_bcd(systime.local_time.year - 1900); /* Epoch is 1900 */

	/* Ok now we have the raw bcd bytes. Now we need to push them into our bit array */

	for (i = 0; i < 64; i++)
	{
		j = i / 8;
		ds1315->raw_data[i] = (raw[j] & 0x0001);
		raw[j] = raw[j] >> 1;
	}
}


/*-------------------------------------------------
    ds1315_input_raw_data
-------------------------------------------------*/

static void ds1315_input_raw_data(device_t *device)
{
	/* This routine is called when new date and time has been written to the
       clock chip. Currently we ignore setting the date and time in the clock
       chip.

       We always return the host's time when asked.
    */
}


/*-------------------------------------------------
    DEVICE_GET_INFO( ds1315 )
-------------------------------------------------*/

DEVICE_GET_INFO( ds1315 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ds1315_t);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(ds1315);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Dallas Semiconductor DS1315");	break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Dallas Semiconductor DS1315");	break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}

DEFINE_LEGACY_DEVICE(DS1315, ds1315);

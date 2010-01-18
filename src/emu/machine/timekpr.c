/*
 * STmicroelectronics TIMEKEEPER SRAM
 *
 * Supports:
 *           M48T02
 *           M48T35
 *           M48T58
 *           MK48T08
 *
 */

#include "emu.h"
#include "machine/timekpr.h"

typedef struct
{
	UINT8 control;
	UINT8 seconds;
	UINT8 minutes;
	UINT8 hours;
	UINT8 day;
	UINT8 date;
	UINT8 month;
	UINT8 year;
	UINT8 century;
	UINT8 *data;
	UINT8 *default_data;
	running_device *device;
	int size;
	int offset_control;
	int offset_seconds;
	int offset_minutes;
	int offset_hours;
	int offset_day;
	int offset_date;
	int offset_month;
	int offset_year;
	int offset_century;
	int offset_flags;
} timekeeper_state;

#define MASK_SECONDS ( 0x7f )
#define MASK_MINUTES ( 0x7f )
#define MASK_HOURS ( 0x3f )
#define MASK_DAY ( 0x07 )
#define MASK_DATE ( 0x3f )
#define MASK_MONTH ( 0x1f )
#define MASK_YEAR ( 0xff )
#define MASK_CENTURY ( 0xff )

#define CONTROL_W ( 0x80 )
#define CONTROL_R ( 0x40 )
#define CONTROL_S ( 0x20 ) /* not emulated */
#define CONTROL_CALIBRATION ( 0x1f ) /* not emulated */

#define SECONDS_ST ( 0x80 )

#define DAY_FT ( 0x40 ) /* not emulated */
#define DAY_CEB ( 0x20 ) /* M48T35/M48T58 */
#define DAY_CB ( 0x10 ) /* M48T35/M48T58 */

#define DATE_BLE ( 0x80 ) /* M48T58: not emulated */
#define DATE_BL ( 0x40 ) /* M48T58: not emulated */

#define FLAGS_BL ( 0x10 ) /* MK48T08: not emulated */

INLINE UINT8 make_bcd(UINT8 data)
{
	return ( ( ( data / 10 ) % 10 ) << 4 ) + ( data % 10 );
}

INLINE UINT8 from_bcd( UINT8 data )
{
	return ( ( ( data >> 4 ) & 15 ) * 10 ) + ( data & 15 );
}

static int inc_bcd( UINT8 *data, int mask, int min, int max )
{
	int bcd;
	int carry;

	bcd = ( *( data ) + 1 ) & mask;
	carry = 0;

	if( ( bcd & 0x0f ) > 9 )
	{
		bcd &= 0xf0;
		bcd += 0x10;
		if( bcd > max )
		{
			bcd = min;
			carry = 1;
		}
	}

	*( data ) = ( *( data ) & ~mask ) | ( bcd & mask );
	return carry;
}

static void counter_to_ram( UINT8 *data, int offset, int counter )
{
	if( offset >= 0 )
	{
		data[ offset ] = counter;
	}
}

static void counters_to_ram( timekeeper_state *c )
{
	counter_to_ram( c->data, c->offset_control, c->control );
	counter_to_ram( c->data, c->offset_seconds, c->seconds );
	counter_to_ram( c->data, c->offset_minutes, c->minutes );
	counter_to_ram( c->data, c->offset_hours, c->hours );
	counter_to_ram( c->data, c->offset_day, c->day );
	counter_to_ram( c->data, c->offset_date, c->date );
	counter_to_ram( c->data, c->offset_month, c->month );
	counter_to_ram( c->data, c->offset_year, c->year );
	counter_to_ram( c->data, c->offset_century, c->century );
}

static int counter_from_ram( UINT8 *data, int offset )
{
	if( offset >= 0 )
	{
		return data[ offset ];
	}
	return 0;
}

static void counters_from_ram( timekeeper_state *c )
{
	c->control = counter_from_ram( c->data, c->offset_control );
	c->seconds = counter_from_ram( c->data, c->offset_seconds );
	c->minutes = counter_from_ram( c->data, c->offset_minutes );
	c->hours = counter_from_ram( c->data, c->offset_hours );
	c->day = counter_from_ram( c->data, c->offset_day );
	c->date = counter_from_ram( c->data, c->offset_date );
	c->month = counter_from_ram( c->data, c->offset_month );
	c->year = counter_from_ram( c->data, c->offset_year );
	c->century = counter_from_ram( c->data, c->offset_century );
}

static TIMER_CALLBACK( timekeeper_tick )
{
	timekeeper_state *c = (timekeeper_state *) ptr;

	int carry;

	if( ( c->seconds & SECONDS_ST ) != 0 ||
		( c->control & CONTROL_W ) != 0 )
	{
		return;
	}

	carry = inc_bcd( &c->seconds, MASK_SECONDS, 0x00, 0x59 );
	if( carry )
	{
		carry = inc_bcd( &c->minutes, MASK_MINUTES, 0x00, 0x59 );
	}
	if( carry )
	{
		carry = inc_bcd( &c->hours, MASK_HOURS, 0x00, 0x23 );
	}

	if( carry )
	{
		UINT8 month;
		UINT8 year;
		UINT8 maxdays;
		static const UINT8 daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

		inc_bcd( &c->day, MASK_DAY, 0x01, 0x07 );

		month = from_bcd( c->month );
		year = from_bcd( c->year );

		if( month == 2 && ( year % 4 ) == 0 )
		{
			maxdays = 0x29;
		}
		else if( month >= 1 && month <= 12 )
		{
			maxdays = daysinmonth[ month - 1 ];
		}
		else
		{
			maxdays = 0x31;
		}

		carry = inc_bcd( &c->date, MASK_DATE, 0x01, maxdays );
	}
	if( carry )
	{
		carry = inc_bcd( &c->month, MASK_MONTH, 0x01, 0x12 );
	}
	if( carry )
	{
		carry = inc_bcd( &c->year, MASK_YEAR, 0x00, 0x99 );
	}
	if( carry )
	{
		carry = inc_bcd( &c->century, MASK_CENTURY, 0x00, 0x99 );

		if( c->device->type == M48T35 ||
			c->device->type == M48T58 )
		{
			if( ( c->day & DAY_CEB ) != 0 )
			{
				c->day ^= DAY_CB;
			}
		}
	}

	if( ( c->control & CONTROL_R ) == 0 )
	{
		counters_to_ram( c );
	}
}

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is the right type
-------------------------------------------------*/

INLINE timekeeper_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert((device->type == M48T02) ||
		   (device->type == M48T35) ||
		   (device->type == M48T58) ||
		   (device->type == MK48T08));

	return (timekeeper_state *)device->token;
}

/* memory handlers */

WRITE8_DEVICE_HANDLER( timekeeper_w )
{
	timekeeper_state *c = get_safe_token(device);

	if( offset == c->offset_control )
	{
		if( ( c->control & CONTROL_W ) != 0 &&
			( data & CONTROL_W ) == 0 )
		{
			counters_from_ram( c );
		}
		c->control = data;
	}
	else if( offset == c->offset_day )
	{
		if( c->device->type == M48T35 ||
			c->device->type == M48T58 )
		{
			c->day = ( c->day & ~DAY_CEB ) | ( data & DAY_CEB );
		}
	}
	else if( offset == c->offset_date && c->device->type == M48T58 )
	{
		data &= ~DATE_BL;
	}
	else if( offset == c->offset_flags && c->device->type == MK48T08 )
	{
		data &= ~FLAGS_BL;
	}

//  logerror( "%s: timekeeper_write( %s, %04x, %02x )\n", cpuexec_describe_context(machine), c->device->tag, offset, data );
	c->data[ offset ] = data;
}

READ8_DEVICE_HANDLER( timekeeper_r )
{
	timekeeper_state *c = get_safe_token(device);
	UINT8 data = c->data[ offset ];
//  logerror( "%s: timekeeper_read( %s, %04x ) %02x\n", cpuexec_describe_context(machine), c->device->tag, offset, data );
	return data;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START(timekeeper)
{
	timekeeper_state *c = get_safe_token(device);
	emu_timer *timer;
	attotime duration;
	mame_system_time systime;

	/* validate some basic stuff */
	assert(device != NULL);
//  assert(device->baseconfig().static_config != NULL);
	assert(device->baseconfig().inline_config == NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	mame_get_base_datetime(device->machine, &systime);

	c->device = device;
	c->control = 0;
	c->seconds = make_bcd( systime.local_time.second );
	c->minutes = make_bcd( systime.local_time.minute );
	c->hours = make_bcd( systime.local_time.hour );
	c->day = make_bcd( systime.local_time.weekday + 1 );
	c->date = make_bcd( systime.local_time.mday );
	c->month = make_bcd( systime.local_time.month + 1 );
	c->year = make_bcd( systime.local_time.year % 100 );
	c->century = make_bcd( systime.local_time.year / 100 );
	c->data = auto_alloc_array( device->machine, UINT8, c->size );

	c->default_data = *device->region;
	assert( device->region->bytes() == c->size );

	state_save_register_device_item( device, 0, c->control );
	state_save_register_device_item( device, 0, c->seconds );
	state_save_register_device_item( device, 0, c->minutes );
	state_save_register_device_item( device, 0, c->hours );
	state_save_register_device_item( device, 0, c->day );
	state_save_register_device_item( device, 0, c->date );
	state_save_register_device_item( device, 0, c->month );
	state_save_register_device_item( device, 0, c->year );
	state_save_register_device_item( device, 0, c->century );
	state_save_register_device_item_pointer( device, 0, c->data, c->size );

	timer = timer_alloc( device->machine, timekeeper_tick, c );
	duration = ATTOTIME_IN_SEC(1);
	timer_adjust_periodic( timer, duration, 0, duration );
}

static DEVICE_START(m48t02)
{
	timekeeper_state *c = get_safe_token(device);

	c->offset_control = 0x7f8;
	c->offset_seconds = 0x7f9;
	c->offset_minutes = 0x7fa;
	c->offset_hours = 0x7fb;
	c->offset_day = 0x7fc;
	c->offset_date = 0x7fd;
	c->offset_month = 0x7fe;
	c->offset_year = 0x7ff;
	c->offset_century = -1;
	c->offset_flags = -1;
	c->size = 0x800;

	DEVICE_START_CALL( timekeeper );
}

static DEVICE_START(m48t35)
{
	timekeeper_state *c = get_safe_token(device);

	c->offset_control = 0x7ff8;
	c->offset_seconds = 0x7ff9;
	c->offset_minutes = 0x7ffa;
	c->offset_hours = 0x7ffb;
	c->offset_day = 0x7ffc;
	c->offset_date = 0x7ffd;
	c->offset_month = 0x7ffe;
	c->offset_year = 0x7fff;
	c->offset_century = -1;
	c->offset_flags = -1;
	c->size = 0x8000;

	DEVICE_START_CALL( timekeeper );
}

static DEVICE_START(m48t58)
{
	timekeeper_state *c = get_safe_token(device);

	c->offset_control = 0x1ff8;
	c->offset_seconds = 0x1ff9;
	c->offset_minutes = 0x1ffa;
	c->offset_hours = 0x1ffb;
	c->offset_day = 0x1ffc;
	c->offset_date = 0x1ffd;
	c->offset_month = 0x1ffe;
	c->offset_year = 0x1fff;
	c->offset_century = -1;
	c->offset_flags = -1;
	c->size = 0x2000;

	DEVICE_START_CALL( timekeeper );
}

static DEVICE_START(mk48t08)
{
	timekeeper_state *c = get_safe_token(device);

	c->offset_control = 0x1ff8;
	c->offset_seconds = 0x1ff9;
	c->offset_minutes = 0x1ffa;
	c->offset_hours = 0x1ffb;
	c->offset_day = 0x1ffc;
	c->offset_date = 0x1ffd;
	c->offset_month = 0x1ffe;
	c->offset_year = 0x1fff;
	c->offset_century = 0x1ff1;
	c->offset_flags = 0x1ff0;
	c->size = 0x2000;

	DEVICE_START_CALL( timekeeper );
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET(timekeeper)
{
}

static DEVICE_NVRAM(timekeeper)
{
	timekeeper_state *c = get_safe_token(device);

	if( read_or_write )
	{
		mame_fwrite( file, c->data, c->size );
	}
	else
	{
		if( file )
		{
			mame_fread( file, c->data, c->size );
		}
		else
		{
			if( c->default_data != NULL )
			{
				memcpy( c->data, c->default_data, c->size );
			}
			else
			{
				memset( c->data, 0xff, c->size );
			}
		}

		counters_to_ram( c );
	}
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

static DEVICE_GET_INFO(timekeeper)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(timekeeper_state); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = 0; break; // sizeof(timekeeper_config)
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(timekeeper); break;
		case DEVINFO_FCT_STOP:					/* nothing */ break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(timekeeper); break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME(timekeeper); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Timekeeper"); break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "EEPROM"); break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0"); break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEVICE_GET_INFO( m48t02 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M48T02");					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(m48t02);	break;

		default:										DEVICE_GET_INFO_CALL(timekeeper);			break;
	}
}

DEVICE_GET_INFO( m48t35 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M48T35");					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(m48t35);	break;

		default:										DEVICE_GET_INFO_CALL(timekeeper);			break;
	}
}

DEVICE_GET_INFO( m48t58 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "M48T58");					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(m48t58);	break;

		default:										DEVICE_GET_INFO_CALL(timekeeper);			break;
	}
}

DEVICE_GET_INFO( mk48t08 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MK48T08");					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mk48t08);	break;

		default:										DEVICE_GET_INFO_CALL(timekeeper);			break;
	}
}

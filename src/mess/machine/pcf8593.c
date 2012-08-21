/*********************************************************************

    Philips PCF8593 CMOS clock/calendar circuit

    (c) 2001-2007 Tim Schuerewegen

*********************************************************************/

#include <time.h>
#include "pcf8593.h"


/***************************************************************************
    PARAMETERS/CONSTANTS/MACROS
***************************************************************************/

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

#define RTC_MODE_NONE  0
#define RTC_MODE_SEND  1
#define RTC_MODE_RECV  2

// get/set date
#define RTC_GET_DATE_YEAR       ((rtc->data[5] >> 6) & 3)
#define RTC_SET_DATE_YEAR(x)    rtc->data[5] = (rtc->data[5] & 0x3F) | (((x) % 4) << 6)
#define RTC_GET_DATE_MONTH      bcd_to_dec( rtc->data[6])
#define RTC_SET_DATE_MONTH(x)   rtc->data[6] = dec_to_bcd( x)
#define RTC_GET_DATE_DAY        (bcd_to_dec( rtc->data[5] & 0x3F))
#define RTC_SET_DATE_DAY(x)     rtc->data[5] = (rtc->data[5] & 0xC0) | dec_to_bcd( x)

// get/set time
#define RTC_GET_TIME_HOUR       bcd_to_dec( rtc->data[4])
#define RTC_SET_TIME_HOUR(x)    rtc->data[4] = dec_to_bcd( x)
#define RTC_GET_TIME_MINUTE     bcd_to_dec( rtc->data[3])
#define RTC_SET_TIME_MINUTE(x)  rtc->data[3] = dec_to_bcd( x)
#define RTC_GET_TIME_SECOND     bcd_to_dec( rtc->data[2])
#define RTC_SET_TIME_SECOND(x)  rtc->data[2] = dec_to_bcd( x)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pcf8593_t pcf8593_t;
struct _pcf8593_t
{
	UINT8 data[16];
	int pin_scl, pin_sda, inp;
	int active;
	int bits;
	UINT8 data_recv_index, data_recv[50];
	UINT8 mode, pos;
	emu_timer *timer;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

static void pcf8593_clear_buffer_rx(device_t *device);
static TIMER_CALLBACK( pcf8593_timer_callback );


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE pcf8593_t *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PCF8593);

	return (pcf8593_t *) downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    DEVICE_START( pcf8593 )
-------------------------------------------------*/

static DEVICE_START( pcf8593 )
{
	pcf8593_t *rtc = get_token(device);

	_logerror( 0, ("pcf8593_init\n"));
	memset( rtc, 0, sizeof(*rtc));
	rtc->timer = device->machine().scheduler().timer_alloc(FUNC(pcf8593_timer_callback), (void *) device);
	rtc->timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
}



/*-------------------------------------------------
    DEVICE_RESET( pcf8593 )
-------------------------------------------------*/

static DEVICE_RESET( pcf8593 )
{
	pcf8593_t *rtc = get_token(device);

	_logerror( 0, ("pcf8593_reset\n"));
	rtc->pin_scl = 1;
	rtc->pin_sda = 1;
	rtc->active  = FALSE;
	rtc->inp     = 0;
	rtc->mode    = RTC_MODE_RECV;
	rtc->bits    = 0;
	pcf8593_clear_buffer_rx(device);
	rtc->pos     = 0;
}



/*-------------------------------------------------
    pcf8593_pin_scl
-------------------------------------------------*/

void pcf8593_pin_scl(device_t *device, int data)
{
	pcf8593_t *rtc = get_token(device);

	// send bit
	if ((rtc->active) && (!rtc->pin_scl) && (data))
	{
		switch (rtc->mode)
		{
			// HOST -> RTC
			case RTC_MODE_RECV :
			{
				// get bit
	    		if (rtc->pin_sda) rtc->data_recv[rtc->data_recv_index] = rtc->data_recv[rtc->data_recv_index] | (0x80 >> rtc->bits);
				rtc->bits++;
				// bit 9 = end
				if (rtc->bits > 8)
				{
					_logerror( 2, ("pcf8593_write_byte(%02X)\n", rtc->data_recv[rtc->data_recv_index]));
					// enter receive mode when 1st byte = 0xA3
					if ((rtc->data_recv[0] == 0xA3) && (rtc->data_recv_index == 0))
					{
						rtc->mode = RTC_MODE_SEND;
					}
					// A2 + xx = "read from pos xx" command
					if ((rtc->data_recv[0] == 0xA2) && (rtc->data_recv_index == 1))
					{
						rtc->pos = rtc->data_recv[1];
					}
					// A2 + xx + .. = write byte
					if ((rtc->data_recv[0] == 0xA2) && (rtc->data_recv_index >= 2))
					{
						UINT8 rtc_pos, rtc_val;
						rtc_pos = rtc->data_recv[1] + (rtc->data_recv_index - 2);
						rtc_val = rtc->data_recv[rtc->data_recv_index];
						//if (rtc_pos == 0) rtc_val = rtc_val & 3; // what is this doing here?
						rtc->data[rtc_pos] = rtc_val;
					}
					// next byte
					rtc->bits = 0;
					rtc->data_recv_index++;
				}
			}
			break;
			// RTC -> HOST
			case RTC_MODE_SEND :
			{
				// set bit
				rtc->inp = (rtc->data[rtc->pos] >> (7 - rtc->bits)) & 1;
				rtc->bits++;
				// bit 9 = end
				if (rtc->bits > 8)
				{
					_logerror( 2, ("pcf8593_read_byte(%02X)\n", rtc->data[rtc->pos]));
					// end ?
					if (rtc->pin_sda)
					{
						_logerror( 2, ("pcf8593 end\n"));
						rtc->mode = RTC_MODE_RECV;
        				pcf8593_clear_buffer_rx(device);
					}
					// next byte
					rtc->bits = 0;
					rtc->pos++;
				}
			}
			break;
		}
	}
	// save scl
	rtc->pin_scl = data;
}



/*-------------------------------------------------
    pcf8593_pin_sda_w
-------------------------------------------------*/

void pcf8593_pin_sda_w(device_t *device, int data)
{
	pcf8593_t *rtc = get_token(device);

	// clock is high
	if (rtc->pin_scl)
	{
		// log init I2C
		if (data) _logerror( 1, ("pcf8593 init i2c\n"));
		// start condition (high to low when clock is high)
		if ((!data) && (rtc->pin_sda))
		{
			_logerror( 1, ("pcf8593 start condition\n"));
			rtc->active          = TRUE;
			rtc->bits            = 0;
			rtc->data_recv_index = 0;
			pcf8593_clear_buffer_rx(device);
			//rtc->pos = 0;
		}
		// stop condition (low to high when clock is high)
		if ((data) && (!rtc->pin_sda))
		{
			_logerror( 1, ("pcf8593 stop condition\n"));
			rtc->active = FALSE;
		}
	}
	// save sda
	rtc->pin_sda = data;
}



/*-------------------------------------------------
    pcf8593_pin_sda_r
-------------------------------------------------*/

int pcf8593_pin_sda_r(device_t *device)
{
	pcf8593_t *rtc = get_token(device);
	return rtc->inp;
}



/*-------------------------------------------------
    pcf8593_clear_buffer_rx
-------------------------------------------------*/

void pcf8593_clear_buffer_rx(device_t *device)
{
	pcf8593_t *rtc = get_token(device);
	memset( &rtc->data_recv[0], 0, sizeof( rtc->data_recv));
	rtc->data_recv_index = 0;
}



/*-------------------------------------------------
    dec_to_bcd
-------------------------------------------------*/

static UINT8 dec_to_bcd( UINT8 data)
{
	return ((data / 10) << 4) | ((data % 10) << 0);
}



/*-------------------------------------------------
    bcd_to_dec
-------------------------------------------------*/

static UINT8 bcd_to_dec( UINT8 data)
{
	if ((data & 0x0F) >= 0x0A) data = data - 0x0A + 0x10;
	if ((data & 0xF0) >= 0xA0) data = data - 0xA0;
	return (data & 0x0F) + (((data & 0xF0) >> 4) * 10);
}



/*-------------------------------------------------
    bcd_to_dec
-------------------------------------------------*/

static void pcf8593_set_time(device_t *device, int hour, int minute, int second)
{
	pcf8593_t *rtc = get_token(device);
	RTC_SET_TIME_HOUR( hour);
	RTC_SET_TIME_MINUTE( minute);
	RTC_SET_TIME_SECOND( second);
	rtc->data[1] = 0; // hundreds of a seconds
}



/*-------------------------------------------------
    pcf8593_set_date
-------------------------------------------------*/

static void pcf8593_set_date(device_t *device, int year, int month, int day)
{
	pcf8593_t *rtc = get_token(device);
	RTC_SET_DATE_YEAR( year);
	RTC_SET_DATE_MONTH( month);
	RTC_SET_DATE_DAY( day);
}



/*-------------------------------------------------
    get_days_in_month
-------------------------------------------------*/

static int get_days_in_month( int year, int month)
{
	static const int table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if ((month == 2) && ((year & 0x03) == 0)) return 29;
	return table[month-1];
}



/*-------------------------------------------------
    TIMER_CALLBACK( pcf8593_timer_callback )
-------------------------------------------------*/

static TIMER_CALLBACK( pcf8593_timer_callback )
{
	device_t *device = (device_t *) ptr;
	pcf8593_t *rtc = get_token(device);
	int value;

	_logerror( 2, ("pcf8593_timer_callback (%d)\n", param));
	// check if counting is enabled
	if (rtc->data[0] & 0x80) return;
	// increment second
	value = RTC_GET_TIME_SECOND;
	if (value < 59)
	{
		RTC_SET_TIME_SECOND( value + 1);
	}
	else
	{
		RTC_SET_TIME_SECOND( 0);
		// increment minute
		value = RTC_GET_TIME_MINUTE;
		if (value < 59)
		{
			RTC_SET_TIME_MINUTE( value + 1);
		}
		else
		{
			RTC_SET_TIME_MINUTE( 0);
			// increment hour
			value = RTC_GET_TIME_HOUR;
			if (value < 23)
			{
				RTC_SET_TIME_HOUR( value + 1);
			}
			else
			{
				RTC_SET_TIME_HOUR( 0);
				// increment day
				value = RTC_GET_DATE_DAY;
				if (value < get_days_in_month( RTC_GET_DATE_YEAR, RTC_GET_DATE_MONTH))
				{
					RTC_SET_DATE_DAY( value + 1);
				}
				else
				{
					RTC_SET_DATE_DAY( 1);
					// increase month
					value = RTC_GET_DATE_MONTH;
					if (value < 12)
					{
						RTC_SET_DATE_MONTH( value + 1);
					}
					else
					{
						RTC_SET_DATE_MONTH( 1);
						// increase year
						RTC_SET_DATE_YEAR( RTC_GET_DATE_YEAR + 1);
					}
				}
			}
		}
	}
}



/*-------------------------------------------------
    pcf8593_load
-------------------------------------------------*/

void pcf8593_load(device_t *device, emu_file *file)
{
	pcf8593_t *rtc = get_token(device);
	system_time systime;

	_logerror( 0, ("pcf8593_load (%p)\n", file));
	file->read(rtc->data, sizeof(rtc->data));
	device->machine().current_datetime(systime);
	pcf8593_set_date(device, systime.local_time.year, systime.local_time.month + 1, systime.local_time.mday);
	pcf8593_set_time(device, systime.local_time.hour, systime.local_time.minute, systime.local_time.second);
}



/*-------------------------------------------------
    pcf8593_save
-------------------------------------------------*/

void pcf8593_save(device_t *device, emu_file *file)
{
	pcf8593_t *rtc = get_token(device);

	_logerror( 0, ("pcf8593_save (%p)\n", file));
	file->write(rtc->data, sizeof(rtc->data));
}

#ifdef UNUSED_FUNCTION
NVRAM_HANDLER( pcf8593 )
{
    _logerror( 0, ("nvram_handler_pcf8593 (%p/%d)\n", file, read_or_write));
    if (read_or_write)
    {
        pcf8593_save( file);
    }
    else
    {
        if (file)
        {
            pcf8593_load( file);
        }
        else
        {
            memset( rtc->data, 0, rtc->size);
        }
    }
}
#endif


/*-------------------------------------------------
    DEVICE_GET_INFO( pcf8593 )
-------------------------------------------------*/

DEVICE_GET_INFO( pcf8593 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(pcf8593_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(pcf8593);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(pcf8593);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "PCF8593 RTC");					break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "PCF8593 RTC");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}

DEFINE_LEGACY_DEVICE(PCF8593, pcf8593);

/************************************************************

    DALLAS DS1302

    RTC + BACKUP RAM



    Emulation by ElSemi


    Missing Features:
      - Burst Mode
      - Clock programming (useless)



    2009-05 Converted to be a device

************************************************************/


#include "emu.h"
#include "ds1302.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ds1302_state ds1302_state;
struct _ds1302_state
{

	UINT32 shift_in;
	UINT8  shift_out;
	UINT8  icount;
	UINT8  last_clk;
	UINT8  last_cmd;
	UINT8  sram[0x20];
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE ds1302_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == DS1302);
	return (ds1302_state *)device->token;
}

INLINE UINT8 convert_to_bcd(int val)
{
	return ((val / 10) << 4) | (val % 10);
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ds1302_dat_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( ds1302_dat_w )
{
	ds1302_state *ds1302 = get_safe_token(device);

	if (data)
		ds1302->shift_in |= (1 << ds1302->icount);
	else
		ds1302->shift_in &= ~(1 << ds1302->icount);
}


/*-------------------------------------------------
    ds1302_clk_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( ds1302_clk_w )
{
	ds1302_state *ds1302 = get_safe_token(device);

	if (data != ds1302->last_clk)
	{
		if (data)	//Rising, shift in command
		{
			ds1302->icount++;
			if(ds1302->icount == 8)	//Command start
			{
				mame_system_time systime;
				mame_get_base_datetime(device->machine, &systime);

				switch(ds1302->shift_in)
				{
					case 0x81:	//Sec
						ds1302->shift_out = convert_to_bcd(systime.local_time.second);
						break;
					case 0x83:	//Min
						ds1302->shift_out = convert_to_bcd(systime.local_time.minute);
						break;
					case 0x85:	//Hour
						ds1302->shift_out = convert_to_bcd(systime.local_time.hour);
						break;
					case 0x87:	//Day
						ds1302->shift_out = convert_to_bcd(systime.local_time.mday);
						break;
					case 0x89:	//Month
						ds1302->shift_out = convert_to_bcd(systime.local_time.month + 1);
						break;
					case 0x8b:	//weekday
						ds1302->shift_out = convert_to_bcd(systime.local_time.weekday);
						break;
					case 0x8d:	//Year
						ds1302->shift_out = convert_to_bcd(systime.local_time.year % 100);
						break;
					default:
						ds1302->shift_out = 0x0;
				}

				if(ds1302->shift_in > 0xc0)
					ds1302->shift_out = ds1302->sram[(ds1302->shift_in >> 1) & 0x1f];
				ds1302->last_cmd = ds1302->shift_in & 0xff;
				ds1302->icount++;
			}

			if(ds1302->icount == 17 && !(ds1302->last_cmd & 1))
			{
				UINT8 val = (ds1302->shift_in >> 9) & 0xff;

				switch(ds1302->last_cmd)
				{
					case 0x80:	//Sec

						break;
					case 0x82:	//Min

						break;
					case 0x84:	//Hour

						break;
					case 0x86:	//Day

						break;
					case 0x88:	//Month

						break;
					case 0x8a:	//weekday

						break;
					case 0x8c:	//Year

						break;
					default:
						ds1302->shift_out = 0x0;
				}
				if(ds1302->last_cmd > 0xc0)
				{
					ds1302->sram[(ds1302->last_cmd >> 1) & 0x1f] = val;
				}



			}
		}
	}
	ds1302->last_clk = data;
}


/*-------------------------------------------------
    ds1302_read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( ds1302_read )
{
	ds1302_state *ds1302 = get_safe_token(device);
	return (ds1302->shift_out & (1 << (ds1302->icount - 9))) ? 1 : 0;
}


/*-------------------------------------------------
    DEVICE_START( ds1302 )
-------------------------------------------------*/

static DEVICE_START( ds1302 )
{
	ds1302_state *ds1302 = get_safe_token(device);

	/* register for state saving */
	state_save_register_device_item(device, 0, ds1302->shift_in);
	state_save_register_device_item(device, 0, ds1302->shift_out);
	state_save_register_device_item(device, 0, ds1302->icount);
	state_save_register_device_item(device, 0, ds1302->last_clk);
	state_save_register_device_item(device, 0, ds1302->last_cmd);
	state_save_register_device_item_array(device, 0, ds1302->sram);
}

/*-------------------------------------------------
    DEVICE_START( ds1302 )
-------------------------------------------------*/

static DEVICE_RESET( ds1302 )
{
	ds1302_state *ds1302 = get_safe_token(device);

	ds1302->shift_in  = 0;
	ds1302->shift_out = 0;
	ds1302->icount    = 0;
	ds1302->last_clk  = 0;
	ds1302->last_cmd  = 0;
}

/*-------------------------------------------------
    DEVICE_GET_INFO( ds1302 )
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##ds1302##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"Dallas DS1302 RTC"
#define DEVTEMPLATE_FAMILY		"Dallas DS1302 RTC"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"

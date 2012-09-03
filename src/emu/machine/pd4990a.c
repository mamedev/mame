/***************************************************************************

    NEC uPD4990A

    Serial I/O Calendar & Clock IC used in the
        NEO GEO and probably a couple of other machines.


    Completed by ElSemi.

    I haven't found any schematics for this device
    so I had to make some assumptions about how it works.

    The three input bits seem to be used for a serial protocol

    bit 0 - data
    bit 1 - clock
    bit 2 - command end (?)

    the commands I've found so far are:

    0x0 - ?? sent after 2
    0x1 - Reset the (probable) shift register used for output
    0x2 - Store the contents of the shift reg to the current date
    0x3 - Load Shift register with current date

    0x7 - Switch test bit every frame
    0x8 - Switch test bit every half-second


    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "machine/pd4990a.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define DATA_BIT	0x01
#define CLOCK_BIT	0x02
#define END_BIT		0x04

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _upd4990a_state upd4990a_state;
struct _upd4990a_state
{
	int seconds;	/* seconds BCD */
	int minutes;	/* minutes BCD */
	int hours;		/* hours   BCD */
	int days;		/* days    BCD */
	int month;		/* month   Hexadecimal form */
	int year;		/* year    BCD */
	int weekday;	/* weekday BCD */

	UINT32 shiftlo;
	UINT32 shifthi;

	int retraces;	/* Assumes 60 retraces a second */
	int testwaits;
	int maxwaits;	/* Switch test every frame*/
	int testbit;	/* Pulses a bit in order to simulate test output */

	int outputbit;
	int bitno;
	INT8 reading;
	INT8 writing;

	int clock_line;
	int command_line;	//??
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE upd4990a_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == UPD4990A));
	return (upd4990a_state *)downcast<upd4990a_device *>(device)->token();
}

INLINE UINT8 convert_to_bcd(int val)
{
	return ((val / 10) << 4) | (val % 10);
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    upd4990a_increment_month
-------------------------------------------------*/

static void upd4990a_increment_month( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	upd4990a->month++;
	if (upd4990a->month == 13)
	{
		upd4990a->month = 1;
		upd4990a->year++;
		if ((upd4990a->year & 0x0f) >= 10)
		{
			upd4990a->year &= 0xf0;
			upd4990a->year += 0x10;
		}
		if (upd4990a->year == 0xA0)
			upd4990a->year = 0;
	}
}

/*-------------------------------------------------
    upd4990a_increment_day
-------------------------------------------------*/

static void upd4990a_increment_day( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);
	int real_year;

	upd4990a->days++;
	if ((upd4990a->days & 0x0f) >= 10)
	{
		upd4990a->days &= 0xf0;
		upd4990a->days += 0x10;
	}

	upd4990a->weekday++;
	if (upd4990a->weekday == 7)
		upd4990a->weekday = 0;

	switch (upd4990a->month)
	{
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			if (upd4990a->days == 0x32)
			{
				upd4990a->days = 1;
				upd4990a_increment_month(device);
			}
			break;
		case 2:
			real_year = (upd4990a->year >> 4) * 10 + (upd4990a->year & 0xf);
			if ((real_year % 4) && (!(real_year % 100) || (real_year % 400)))
			{
				if (upd4990a->days == 0x29)
				{
					upd4990a->days = 1;
					upd4990a_increment_month(device);
				}
			}
			else
			{
				if (upd4990a->days == 0x30)
				{
					upd4990a->days = 1;
					upd4990a_increment_month(device);
				}
			}
			break;
		case 4: case 6: case 9: case 11:
			if (upd4990a->days == 0x31)
			{
				upd4990a->days = 1;
				upd4990a_increment_month(device);
			}
			break;
	}
}

/*-------------------------------------------------
    upd4990a_addretrace
-------------------------------------------------*/

void upd4990a_addretrace( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	++upd4990a->testwaits;
	if(upd4990a->testwaits >= upd4990a->maxwaits)
	{
		upd4990a->testbit ^= 1;
		upd4990a->testwaits = 0;
	}

	upd4990a->retraces++;
	if (upd4990a->retraces < 60)
		return;

	upd4990a->retraces = 0;
	upd4990a->seconds++;
	if ((upd4990a->seconds & 0x0f) < 10)
		return;

	upd4990a->seconds &= 0xf0;
	upd4990a->seconds += 0x10;
	if (upd4990a->seconds < 0x60)
		return;

	upd4990a->seconds = 0;
	upd4990a->minutes++;
	if ((upd4990a->minutes & 0x0f) < 10)
		return;

	upd4990a->minutes &= 0xf0;
	upd4990a->minutes += 0x10;
	if (upd4990a->minutes < 0x60)
		return;

	upd4990a->minutes = 0;
	upd4990a->hours++;
	if ((upd4990a->hours & 0x0f) < 10)
		return;

	upd4990a->hours &= 0xf0;
	upd4990a->hours += 0x10;
	if (upd4990a->hours < 0x24)
		return;

	upd4990a->hours = 0;
	upd4990a_increment_day(device);
}

/*-------------------------------------------------
    upd4990a_testbit_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( upd4990a_testbit_r )
{
	upd4990a_state *upd4990a = get_safe_token(device);
	return upd4990a->testbit;
}

/*-------------------------------------------------
    upd4990a_databit_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( upd4990a_databit_r )
{
	upd4990a_state *upd4990a = get_safe_token(device);
	return upd4990a->outputbit;
}

/*-------------------------------------------------
    upd4990a_readbit
-------------------------------------------------*/

static void upd4990a_readbit( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	switch (upd4990a->bitno)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			upd4990a->outputbit = (upd4990a->seconds >> upd4990a->bitno) & 0x01;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			upd4990a->outputbit = (upd4990a->minutes >> (upd4990a->bitno - 0x08)) & 0x01;
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			upd4990a->outputbit = (upd4990a->hours >> (upd4990a->bitno - 0x10)) & 0x01;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			upd4990a->outputbit = (upd4990a->days >> (upd4990a->bitno - 0x18)) & 0x01;
			break;
		case 0x20: case 0x21: case 0x22: case 0x23:
			upd4990a->outputbit = (upd4990a->weekday >> (upd4990a->bitno - 0x20)) & 0x01;
			break;
		case 0x24: case 0x25: case 0x26: case 0x27:
			upd4990a->outputbit = (upd4990a->month >> (upd4990a->bitno - 0x24)) & 0x01;
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			upd4990a->outputbit = (upd4990a->year >> (upd4990a->bitno - 0x28)) & 0x01;
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
			//unknown
			break;
	}
}

/*-------------------------------------------------
    upd4990a_resetbitstream
-------------------------------------------------*/

static void upd4990a_resetbitstream( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	upd4990a->shiftlo = 0;
	upd4990a->shifthi = 0;
	upd4990a->bitno = 0;
}

/*-------------------------------------------------
    upd4990a_writebit
-------------------------------------------------*/

static void upd4990a_writebit( device_t *device , UINT8 bit )
{
	upd4990a_state *upd4990a = get_safe_token(device);
	if (upd4990a->bitno <= 31)	//low part
		upd4990a->shiftlo |= bit << upd4990a->bitno;
	else	//high part
		upd4990a->shifthi |= bit << (upd4990a->bitno - 32);
}

/*-------------------------------------------------
    upd4990a_nextbit
-------------------------------------------------*/

static void upd4990a_nextbit( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);
	++upd4990a->bitno;

	if (upd4990a->reading)
		upd4990a_readbit(device);

	if (upd4990a->reading && upd4990a->bitno == 0x34)
	{
		upd4990a->reading = 0;
		upd4990a_resetbitstream(device);
	}

}

/*-------------------------------------------------
    upd4990a_getcommand
-------------------------------------------------*/

static UINT8 upd4990a_getcommand( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);
	//Warning: problems if the 4 bits are in different
	//parts, It's very strange that this case could happen.
	if(upd4990a->bitno <= 31)
		return upd4990a->shiftlo >> (upd4990a->bitno - 4);
	else
		return upd4990a->shifthi >> (upd4990a->bitno - 32 - 4);
}

/*-------------------------------------------------
    upd4990a_update_date
-------------------------------------------------*/

static void upd4990a_update_date( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	upd4990a->seconds = (upd4990a->shiftlo >> 0 ) & 0xff;
	upd4990a->minutes = (upd4990a->shiftlo >> 8 ) & 0xff;
	upd4990a->hours   = (upd4990a->shiftlo >> 16) & 0xff;
	upd4990a->days    = (upd4990a->shiftlo >> 24) & 0xff;
	upd4990a->weekday = (upd4990a->shifthi >> 0 ) & 0x0f;
	upd4990a->month   = (upd4990a->shifthi >> 4 ) & 0x0f;
	upd4990a->year    = (upd4990a->shifthi >> 8 ) & 0xff;
}

/*-------------------------------------------------
    upd4990a_process_command
-------------------------------------------------*/

static void upd4990a_process_command( device_t *device )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	switch(upd4990a_getcommand(device))
	{
		case 0x1:	//load output register
			upd4990a->bitno = 0;
			if (upd4990a->reading)
				upd4990a_readbit(device);	//prepare first bit
			upd4990a->shiftlo = 0;
			upd4990a->shifthi = 0;
			break;
		case 0x2:
			upd4990a->writing = 0;	//store register to current date
			upd4990a_update_date(device);
			break;
		case 0x3:	//start reading
			upd4990a->reading = 1;
			break;
		case 0x7:	//switch testbit every frame
			upd4990a->maxwaits = 1;
			break;
		case 0x8:	//switch testbit every half-second
			upd4990a->maxwaits = 30;
			break;
	}
	upd4990a_resetbitstream(device);
}

/*-------------------------------------------------
    upd4990a_serial_control
-------------------------------------------------*/

static void upd4990a_serial_control( device_t *device, UINT8 data )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	//Check for command end
	if(upd4990a->command_line && !(data & END_BIT)) //end of command
	{
		upd4990a_process_command(device);
	}
	upd4990a->command_line = data & END_BIT;

	if(upd4990a->clock_line && !(data & CLOCK_BIT))	//clock lower edge
	{
		upd4990a_writebit(device, data & DATA_BIT);
		upd4990a_nextbit(device);
	}
	upd4990a->clock_line = data & CLOCK_BIT;
}

/*-------------------------------------------------
    upd4990a_control_16_w
-------------------------------------------------*/

WRITE16_DEVICE_HANDLER( upd4990a_control_16_w )
{
	upd4990a_serial_control(device, data & 0x7);
}


/*-------------------------------------------------
    DEVICE_START( upd4990a )
-------------------------------------------------*/

static DEVICE_START( upd4990a )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	system_time curtime, *systime = &curtime;
	device->machine().current_datetime(curtime);

#if 0
	upd4990a->seconds = 0x00;
	upd4990a->minutes = 0x00;
	upd4990a->hours = 0x00;
	upd4990a->days = 0x09;
	upd4990a->month = 9;
	upd4990a->year = 0x73;
	upd4990a->weekday = 1;
#endif
	/* HACK: load time counter from system time */
	upd4990a->seconds = convert_to_bcd(systime->local_time.second);
	upd4990a->minutes = convert_to_bcd(systime->local_time.minute);
	upd4990a->hours = convert_to_bcd(systime->local_time.hour);
	upd4990a->days = convert_to_bcd(systime->local_time.mday);
	upd4990a->month = systime->local_time.month + 1;
	upd4990a->year = ((((systime->local_time.year - 1900) % 100) / 10) << 4) | ((systime->local_time.year - 1900) % 10);
	upd4990a->weekday = systime->local_time.weekday;

	/* register for state saving */
	device->save_item(NAME(upd4990a->seconds));
	device->save_item(NAME(upd4990a->minutes));
	device->save_item(NAME(upd4990a->hours));
	device->save_item(NAME(upd4990a->days));
	device->save_item(NAME(upd4990a->month));
	device->save_item(NAME(upd4990a->year));
	device->save_item(NAME(upd4990a->weekday));

	device->save_item(NAME(upd4990a->shiftlo));
	device->save_item(NAME(upd4990a->shifthi));

	device->save_item(NAME(upd4990a->retraces));
	device->save_item(NAME(upd4990a->testwaits));
	device->save_item(NAME(upd4990a->maxwaits));
	device->save_item(NAME(upd4990a->testbit));

	device->save_item(NAME(upd4990a->outputbit));
	device->save_item(NAME(upd4990a->bitno));
	device->save_item(NAME(upd4990a->reading));
	device->save_item(NAME(upd4990a->writing));

	device->save_item(NAME(upd4990a->clock_line));
	device->save_item(NAME(upd4990a->command_line));
}


/*-------------------------------------------------
    DEVICE_RESET( upd4990a )
-------------------------------------------------*/

static DEVICE_RESET( upd4990a )
{
	upd4990a_state *upd4990a = get_safe_token(device);

	upd4990a->shiftlo = 0;
	upd4990a->shifthi = 0;

	upd4990a->retraces = 0;
	upd4990a->testwaits = 0;
	upd4990a->maxwaits = 1;
	upd4990a->testbit = 0;

	upd4990a->outputbit = 0;
	upd4990a->bitno = 0;
	upd4990a->reading = 0;
	upd4990a->writing = 0;

	upd4990a->clock_line = 0;
	upd4990a->command_line = 0;
}

const device_type UPD4990A = &device_creator<upd4990a_device>;

upd4990a_device::upd4990a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD4990A, "NEC uPD4990A", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(upd4990a_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd4990a_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4990a_device::device_start()
{
	DEVICE_START_NAME( upd4990a )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd4990a_device::device_reset()
{
	DEVICE_RESET_NAME( upd4990a )(this);
}



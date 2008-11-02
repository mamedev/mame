/*
 *  Emulation for the NEC PD4990A.
 *
 *  The PD4990A is a serial I/O Calendar & Clock IC used in the
 *      NEO GEO and probably a couple of other machines.


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



 */

#include "driver.h"
#include "machine/pd4990a.h"


/* Set the data in the chip to Monday 09/09/73 00:00:00     */
/* If you ever read this Leejanne, you know what I mean :-) */
static const struct pd4990a_s pd4990a_initval =
{
	0x00,	/* seconds BCD */
	0x00,	/* minutes BCD */
	0x00,	/* hours   BCD */
	0x09,	/* days    BCD */
	9,		/* month   Hexadecimal form */
	0x73,	/* year    BCD */
	1		/* weekday BCD */
};

struct pd4990a_s pd4990a;

static UINT32 shiftlo,shifthi;

static int retraces = 0;	/* Assumes 60 retraces a second */
static int testwaits= 0;
static int maxwaits= 0;	/*switch test every frame*/
static int testbit = 0;	/* Pulses a bit in order to simulate */
					/* test output */
static int outputbit=0;
static int bitno=0;
static INT8 reading=0;
static INT8 writting=0;

static int clock_line=0;
static int command_line=0;	//??

#define DATA_BIT	0x1
#define CLOCK_BIT	0x2
#define END_BIT		0x4

void pd4990a_addretrace()
{
	++testwaits;
	if(testwaits>=maxwaits)
	{
		testbit ^= 1;
		testwaits=0;
	}
	retraces++;
	if (retraces < 60)
		return;
	retraces = 0;
	pd4990a.seconds++;
	if ( (pd4990a.seconds & 0x0f) < 10 )
		return;
	pd4990a.seconds &= 0xf0;
	pd4990a.seconds += 0x10;
	if (pd4990a.seconds < 0x60)
		return;
	pd4990a.seconds = 0;
	pd4990a.minutes++;
	if ( (pd4990a.minutes & 0x0f) < 10 )
		return;
	pd4990a.minutes &= 0xf0;
	pd4990a.minutes += 0x10;
	if (pd4990a.minutes < 0x60)
		return;
	pd4990a.minutes = 0;
	pd4990a.hours++;
	if ( (pd4990a.hours & 0x0f) < 10 )
		return;
	pd4990a.hours &= 0xf0;
	pd4990a.hours += 0x10;
	if (pd4990a.hours < 0x24)
		return;
	pd4990a.hours = 0;
	pd4990a_increment_day();
}

void pd4990a_increment_day(void)
{
	int real_year;

	pd4990a.days++;
	if ((pd4990a.days & 0x0f) >= 10)
	{
		pd4990a.days &= 0xf0;
		pd4990a.days += 0x10;
	}

	pd4990a.weekday++;
	if (pd4990a.weekday == 7)
		pd4990a.weekday=0;

	switch(pd4990a.month)
	{
	case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		if (pd4990a.days == 0x32)
		{
			pd4990a.days = 1;
			pd4990a_increment_month();
		}
		break;
	case 2:
		real_year = (pd4990a.year>>4)*10 + (pd4990a.year&0xf);
		if ((real_year % 4) && (!(real_year % 100) || (real_year % 400)))
		{
			if (pd4990a.days == 0x29)
			{
				pd4990a.days = 1;
				pd4990a_increment_month();
			}
		}
		else
		{
			if (pd4990a.days == 0x30)
			{
				pd4990a.days = 1;
				pd4990a_increment_month();
			}
		}
		break;
	case 4: case 6: case 9: case 11:
		if (pd4990a.days == 0x31)
		{
			pd4990a.days = 1;
			pd4990a_increment_month();
		}
		break;
	}
}

void pd4990a_increment_month(void)
{
	pd4990a.month++;
	if (pd4990a.month == 13)
	{
		pd4990a.month = 1;
		pd4990a.year++;
		if ( (pd4990a.year & 0x0f) >= 10 )
		{
			pd4990a.year &= 0xf0;
			pd4990a.year += 0x10;
		}
		if (pd4990a.year == 0xA0)
			pd4990a.year = 0;
	}
}

READ8_HANDLER( pd4990a_testbit_r )
{
	return (testbit);
}

READ8_HANDLER( pd4990a_databit_r )
{
	return (outputbit);
}

static void pd4990a_readbit(void)
{
	switch(bitno)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			outputbit=(pd4990a.seconds >> bitno) & 0x01;
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			outputbit=(pd4990a.minutes >> (bitno-0x08)) & 0x01;
			break;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			outputbit=(pd4990a.hours >> (bitno-0x10)) & 0x01;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			outputbit=(pd4990a.days >> (bitno-0x18)) & 0x01;
			break;
		case 0x20: case 0x21: case 0x22: case 0x23:
			outputbit=(pd4990a.weekday >> (bitno-0x20)) & 0x01;
			break;
		case 0x24: case 0x25: case 0x26: case 0x27:
			outputbit=(pd4990a.month >> (bitno-0x24)) & 0x01;
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			outputbit=(pd4990a.year >> (bitno-0x28)) & 0x01;
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
			//unknown
			break;
	}
}




static void pd4990a_resetbitstream(void)
{
	shiftlo=0;
	shifthi=0;
	bitno=0;
}

static void pd4990a_writebit(UINT8 bit)
{
	if(bitno<=31)	//low part
		shiftlo|=bit<<bitno;
	else	//high part
		shifthi|=bit<<(bitno-32);
}

static void pd4990a_nextbit(void)
{
	++bitno;
	if(reading)
		pd4990a_readbit();
	if(reading && bitno==0x34)
	{
		reading=0;
		pd4990a_resetbitstream();
	}

}

static UINT8 pd4990a_getcommand(void)
{
	//Warning: problems if the 4 bits are in different
	//parts, It's very strange that this case could happen.
	if(bitno<=31)
		return shiftlo>>(bitno-4);
	else
		return shifthi>>(bitno-32-4);
}

static void pd4990a_update_date(void)
{
	pd4990a.seconds=(shiftlo>>0 )&0xff;
	pd4990a.minutes=(shiftlo>>8 )&0xff;
	pd4990a.hours  =(shiftlo>>16)&0xff;
	pd4990a.days   =(shiftlo>>24)&0xff;
	pd4990a.weekday=(shifthi>>0 )&0x0f;
	pd4990a.month  =(shifthi>>4 )&0x0f;
	pd4990a.year   =(shifthi>>8 )&0xff;
}

static void pd4990a_process_command(void)
{
	switch(pd4990a_getcommand())
	{
		case 0x1:	//load output register
			bitno=0;
			if(reading)
				pd4990a_readbit();	//prepare first bit
			shiftlo=0;
			shifthi=0;
			break;
		case 0x2:
			writting=0;	//store register to current date
			pd4990a_update_date();
			break;
		case 0x3:	//start reading
			reading=1;
			break;
		case 0x7:	//switch testbit every frame
			maxwaits=1;
			break;
		case 0x8:	//switch testbit every half-second
			maxwaits=30;
			break;
	}
	pd4990a_resetbitstream();
}


static void pd4990a_serial_control(UINT8 data)
{
	//Check for command end
	if(command_line && !(data&END_BIT)) //end of command
	{
		pd4990a_process_command();
	}
	command_line=data&END_BIT;

	if(clock_line && !(data&CLOCK_BIT))	//clock lower edge
	{
		pd4990a_writebit(data&DATA_BIT);
		pd4990a_nextbit();
	}
	clock_line=data&CLOCK_BIT;
}

WRITE16_HANDLER( pd4990a_control_16_w )
{
	pd4990a_serial_control(data&0x7);
}

void pd4990a_init(void)
{
	pd4990a = pd4990a_initval;
	shiftlo = 0;
	shifthi = 0;
	retraces = 0;
	testwaits = 0;
	maxwaits = 1;
	testbit = 0;
	outputbit = 0;
	bitno = 0;
	reading = 0;
	writting = 0;
	clock_line = 0;
	command_line =0;

	state_save_register_item("pd4990a", 0, pd4990a.seconds);
	state_save_register_item("pd4990a", 0, pd4990a.minutes);
	state_save_register_item("pd4990a", 0, pd4990a.hours);
	state_save_register_item("pd4990a", 0, pd4990a.days);
	state_save_register_item("pd4990a", 0, pd4990a.month);
	state_save_register_item("pd4990a", 0, pd4990a.year);
	state_save_register_item("pd4990a", 0, pd4990a.weekday);

	state_save_register_item("pd4990a", 0, shiftlo);
	state_save_register_item("pd4990a", 0, shifthi);

	state_save_register_item("pd4990a", 0, retraces);
	state_save_register_item("pd4990a", 0, testwaits);
	state_save_register_item("pd4990a", 0, maxwaits);
	state_save_register_item("pd4990a", 0, testbit);

	state_save_register_item("pd4990a", 0, outputbit);
	state_save_register_item("pd4990a", 0, bitno);
	state_save_register_item("pd4990a", 0, reading);
	state_save_register_item("pd4990a", 0, writting);

	state_save_register_item("pd4990a", 0, clock_line);
	state_save_register_item("pd4990a", 0, command_line);
}

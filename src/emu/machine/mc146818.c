/*********************************************************************

    mc146818.c

    Implementation of the MC146818 chip

    Real time clock chip with battery buffered ram (or CMOS)
    Used in IBM PC/AT, several PC clones, Amstrad NC200

    Nathan Woods  (npwoods@mess.org)
    Peter Trauner (peter.trauner@jk.uni-linz.ac.at)

    PC CMOS info (based on info from Padgett Peterson):

    Clock Related:
        0x00 Seconds       (BCD 00-59, Hex 00-3B) Note: Bit 7 is read only
        0x01 Second Alarm  (BCD 00-59, Hex 00-3B; "don't care" if C0-FF)
        0x02 Minutes       (BCD 00-59, Hex 00-3B)
        0x03 Minute Alarm  (BCD 00-59, Hex 00-3B; "don't care" if C0-FF))
        0x04 Hours         (BCD 00-23, Hex 00-17 if 24 hr mode)
                        (BCD 01-12, Hex 01-0C if 12 hr am)
                        (BCD 81-92. Hex 81-8C if 12 hr pm)
        0x05 Hour Alarm    (same as hours; "don't care" if C0-FF))
        0x06 Day of Week   (01-07 Sunday=1)
        0x07 Date of Month (BCD 01-31, Hex 01-1F)
        0x08 Month         (BCD 01-12, Hex 01-0C)
        0x09 Year          (BCD 00-99, Hex 00-63)
        0x0B Status Register B (read/write)
            Bit 7 - 1 enables cycle update, 0 disables
            Bit 6 - 1 enables periodic interrupt
            Bit 5 - 1 enables alarm interrupt
            Bit 4 - 1 enables update-ended interrupt
            Bit 3 - 1 enables square wave output
            Bit 2 - Data Mode - 0: BCD, 1: Binary
            Bit 1 - 24/12 hour selection - 1 enables 24 hour mode
            Bit 0 - Daylight Savings Enable - 1 enables
        0x0C Status Register C (Read only)
            Bit 7 - Interrupt request flag - 1 when any or all of bits 6-4 are
                        1 and appropriate enables (Register B) are set to 1. Generates
                        IRQ 8 when triggered.
            Bit 6 - Periodic Interrupt flag
            Bit 5 - Alarm Interrupt flag
            Bit 4 - Update-Ended Interrupt Flag
            Bit 3-0 ???
        0x0D Status Register D (read only)
            Bit 7 - Valid RAM - 1 indicates batery power good, 0 if dead or
                        disconnected.
            Bit 6-0 ???

    Non-clock related:
        0x0E (PS/2) Diagnostic Status Byte
            Bit 7 - When set (1) indicates clock has lost power
            Bit 6 - (1) indicates incorrect checksum
            Bit 5 - (1) indicates that equipment configuration is incorrect
                            power-on check requires that atleast one floppy be installed
            Bit 4 - (1) indicates error in memory size
            Bit 3 - (1) indicates that controller or disk drive failed initialization
            Bit 2 - (1) indicates that time is invalid
            Bit 1 - (1) indicates installed adaptors do not match configuration
            Bit 0 - (1) indicates a time-out while reading adaptor ID
        0x0E (AMSTRAD) 6  BYTEs time and date machine last used
        0x0F Reset Code (IBM PS/2 "Shutdown Status Byte")
            0x00-0x03   perform power-on reset
            0x04        INT 19h reboot
            0x05        flush keyboard and jump via 0040:0067
            0x06-0x07   reserved
            0x08        used by POST during protected-mode RAM test
            0x09        used for INT 15/87h (block move) support
            0x0A        jump via 0040:0067
            0x0B-0xFF   perform power-on reset

*********************************************************************/

#include "emu.h"
#include "coreutil.h"
#include "machine/mc146818.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_MC146818		0



//**************************************************************************
//  MACROS
//**************************************************************************

#define HOURS_24	(m_data[0xb]&2)
#define BCD_MODE	!(m_data[0xb]&4) // book has other description!
#define CENTURY		m_data[50]
#define YEAR		m_data[9]
#define MONTH		m_data[8]
#define DAY			m_data[7]
#define WEEK_DAY	m_data[6]



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MC146818 = mc146818_device_config::static_alloc_device_config;



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  mc146818_device_config - constructor
//-------------------------------------------------

mc146818_device_config::mc146818_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "NVRAM", tag, owner, clock),
	  device_config_nvram_interface(mconfig, *this),
	  m_type(MC146818_STANDARD)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *mc146818_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(mc146818_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *mc146818_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, mc146818_device(machine, *this));
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void mc146818_device_config::static_set_type(device_config *device, mc146818_type type)
{
	downcast<mc146818_device_config *>(device)->m_type = type;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc146818_device - constructor
//-------------------------------------------------

mc146818_device::mc146818_device(running_machine &_machine, const mc146818_device_config &config)
	: device_t(_machine, config),
	  device_nvram_interface(_machine, config, *this),
	  m_config(config),
	  m_index(0),
	  m_eindex(0),
	  m_updated(false),
	  m_last_refresh(attotime_zero)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc146818_device::device_start()
{
	m_last_refresh = timer_get_time(&m_machine);
	emu_timer *timer = device_timer_alloc(*this);
	timer_adjust_periodic(timer, ATTOTIME_IN_HZ(1), 0, ATTOTIME_IN_HZ(1));
	set_base_datetime();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mc146818_device::device_timer(emu_timer &timer, int param, void *ptr)
{
	int year/*, month*/;

	if (BCD_MODE)
	{
		m_data[0]=bcd_adjust(m_data[0]+1);
		if (m_data[0]>=0x60)
		{
			m_data[0]=0;
			m_data[2]=bcd_adjust(m_data[2]+1);
			if (m_data[2]>=0x60)
			{
				m_data[2]=0;
				m_data[4]=bcd_adjust(m_data[4]+1);
				// different handling of hours
				if (m_data[4]>=0x24)
				{
					m_data[4]=0;
					WEEK_DAY=bcd_adjust(WEEK_DAY+1)%7;
					DAY=bcd_adjust(DAY+1);
					//month=bcd_2_dec(MONTH);
					year=bcd_2_dec(YEAR);
					if (m_config.m_type!=mc146818_device_config::MC146818_IGNORE_CENTURY) year+=bcd_2_dec(CENTURY)*100;
					else year+=2000; // save for julian_days_in_month calculation
					DAY=bcd_adjust(DAY+1);
					if (DAY>gregorian_days_in_month(MONTH, year))
					{
						DAY=1;
						MONTH=bcd_adjust(MONTH+1);
						if (MONTH>0x12)
						{
							MONTH=1;
							YEAR=year=bcd_adjust(YEAR+1);
							if (m_config.m_type!=mc146818_device_config::MC146818_IGNORE_CENTURY)
							{
								if (year>=0x100)
								{
									CENTURY=bcd_adjust(CENTURY+1);
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		m_data[0]=m_data[0]+1;
		if (m_data[0]>=60)
		{
			m_data[0]=0;
			m_data[2]=m_data[2]+1;
			if (m_data[2]>=60) {
				m_data[2]=0;
				m_data[4]=m_data[4]+1;
				// different handling of hours //?
				if (m_data[4]>=24) {
					m_data[4]=0;
					WEEK_DAY=(WEEK_DAY+1)%7;
					year=YEAR;
					if (m_config.m_type!=mc146818_device_config::MC146818_IGNORE_CENTURY) year+=CENTURY*100;
					else year+=2000; // save for julian_days_in_month calculation
					if (++DAY>gregorian_days_in_month(MONTH, year)) {
						DAY=1;
						if (++MONTH>12) {
							MONTH=1;
							YEAR++;
							if (m_config.m_type!=mc146818_device_config::MC146818_IGNORE_CENTURY) {
								if (YEAR>=100) { CENTURY++;YEAR=0; }
							} else {
								YEAR%=100;
							}
						}
					}
				}
			}
		}
	}
	m_updated = true;  /* clock has been updated */
	m_last_refresh = timer_get_time(&m_machine);
}



//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void mc146818_device::nvram_default()
{
	set_base_datetime();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void mc146818_device::nvram_read(mame_file &file)
{
	mame_fread(&file, m_data, sizeof(m_data));
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void mc146818_device::nvram_write(mame_file &file)
{
	mame_fwrite(&file, m_data, sizeof(m_data));
}


//-------------------------------------------------
//  dec_2_local - convert from decimal to BCD if
//  necessary
//-------------------------------------------------

inline int mc146818_device::dec_2_local(int a)
{
	return BCD_MODE ? dec_2_bcd(a) : a;
}


//-------------------------------------------------
//  dec_2_local - convert from decimal to BCD if
//  necessary
//-------------------------------------------------

void mc146818_device::set_base_datetime()
{
	system_time systime;

	m_machine.base_datetime(systime);

	if (HOURS_24 || (systime.local_time.hour < 12))
		m_data[4] = dec_2_local(systime.local_time.hour);
	else
		m_data[4] = dec_2_local(systime.local_time.hour - 12) | 0x80;

	if (m_config.m_type != mc146818_device_config::MC146818_IGNORE_CENTURY)
		CENTURY = dec_2_local(systime.local_time.year /100);

	m_data[0]	= dec_2_local(systime.local_time.second);
	m_data[2]	= dec_2_local(systime.local_time.minute);
	DAY					= dec_2_local(systime.local_time.day);
	MONTH				= dec_2_local(systime.local_time.month + 1);
	YEAR				= dec_2_local(systime.local_time.year % 100);

	WEEK_DAY = systime.local_time.weekday;
	if (systime.local_time.is_dst)
		m_data[0xb] |= 1;
	else
		m_data[0xb] &= ~1;
}


//-------------------------------------------------
//  read - I/O handler for reading
//-------------------------------------------------

READ8_MEMBER( mc146818_device::read )
{
	UINT8 data = 0;
	switch (offset) {
	case 0:
		data = m_index;
		break;

	case 1:
		switch (m_index % MC146818_DATA_SIZE) {
		case 0xa:
			data = m_data[m_index  % MC146818_DATA_SIZE];
			if (attotime_compare(attotime_sub(timer_get_time(space.machine), m_last_refresh), ATTOTIME_IN_HZ(32768)) < 0)
				data |= 0x80;
#if 0
			/* for pc1512 bios realtime clock test */
			m_data[m_index % MC146818_DATA_SIZE] ^= 0x80; /* 0x80 update in progress */
#endif
			break;

		case 0xc:
			if (m_updated) /* the clock has been updated */
				data = 0x10;
			else
				data = 0x00;
			break;
		case 0xd:
			/* battery ok */
			data = m_data[m_index % MC146818_DATA_SIZE] | 0x80;
			break;

		default:
			data = m_data[m_index % MC146818_DATA_SIZE];
			break;
		}
		break;
	}

	if (LOG_MC146818)
		logerror("mc146818_port_r(): index=0x%02x data=0x%02x\n", m_index, data);
	return data;
}


//-------------------------------------------------
//  write - I/O handler for writing
//-------------------------------------------------

WRITE8_MEMBER( mc146818_device::write )
{
	if (LOG_MC146818)
		logerror("mc146818_port_w(): index=0x%02x data=0x%02x\n", m_index, data);

	switch (offset) {
	case 0:
		m_index = data;
		break;

	case 1:
		switch(m_index % MC146818_DATA_SIZE)
		{
		case 0x0b:
			if(data & 0x80)
				m_updated = false;
			m_data[m_index % MC146818_DATA_SIZE] = data;
			break;
		default:
			m_data[m_index % MC146818_DATA_SIZE] = data;
		}
		break;
	}
}

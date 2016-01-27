// license:BSD-3-Clause
// copyright-holders:Aaron Giles,smf
/***************************************************************************

    timekpr.h

    Various ST Microelectronics timekeeper SRAM implementations:
        - M48T02
        - M48T35
        - M48T37
        - M48T58
        - MK48T08

***************************************************************************/

#include "emu.h"
#include "machine/timekpr.h"


// device type definition
const device_type M48T02 = &device_creator<m48t02_device>;
const device_type M48T35 = &device_creator<m48t35_device>;
const device_type M48T37 = &device_creator<m48t37_device>;
const device_type M48T58 = &device_creator<m48t58_device>;
const device_type MK48T08 = &device_creator<mk48t08_device>;


/***************************************************************************
    MACROS
***************************************************************************/

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

#define DAY_FT ( 0x40 ) /* M48T37 - not emulated */
#define DAY_CEB ( 0x20 ) /* M48T35/M48T58 */
#define DAY_CB ( 0x10 ) /* M48T35/M48T58 */

#define DATE_BLE ( 0x80 ) /* M48T58: not emulated */
#define DATE_BL ( 0x40 ) /* M48T58: not emulated */

#define FLAGS_BL ( 0x10 ) /* MK48T08/M48T37: not emulated */
#define FLAGS_AF ( 0x40 ) /* M48T37: not emulated */
#define FLAGS_WDF ( 0x80 ) /* M48T37: not emulated */


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

static inline UINT8 make_bcd(UINT8 data)
{
	return ( ( ( data / 10 ) % 10 ) << 4 ) + ( data % 10 );
}

static inline UINT8 from_bcd( UINT8 data )
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

static int counter_from_ram( UINT8 *data, int offset )
{
	if( offset >= 0 )
	{
		return data[ offset ];
	}
	return 0;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  timekeeper_device_config - constructor
//-------------------------------------------------

timekeeper_device::timekeeper_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source)
	, device_nvram_interface(mconfig, *this)
	, m_default_data(*this, DEVICE_SELF)
{
}

m48t02_device::m48t02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: timekeeper_device(mconfig, M48T02, "M48T02 Timekeeper", tag, owner, clock, "m48t02", __FILE__)
{
	m_offset_control = 0x7f8;
	m_offset_seconds = 0x7f9;
	m_offset_minutes = 0x7fa;
	m_offset_hours = 0x7fb;
	m_offset_day = 0x7fc;
	m_offset_date = 0x7fd;
	m_offset_month = 0x7fe;
	m_offset_year = 0x7ff;
	m_offset_century = -1;
	m_offset_flags = -1;
	m_size = 0x800;
}

m48t35_device::m48t35_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: timekeeper_device(mconfig, M48T35, "M48T35 Timekeeper", tag, owner, clock, "m48t35", __FILE__)
{
	m_offset_control = 0x7ff8;
	m_offset_seconds = 0x7ff9;
	m_offset_minutes = 0x7ffa;
	m_offset_hours = 0x7ffb;
	m_offset_day = 0x7ffc;
	m_offset_date = 0x7ffd;
	m_offset_month = 0x7ffe;
	m_offset_year = 0x7fff;
	m_offset_century = -1;
	m_offset_flags = -1;
	m_size = 0x8000;
}

m48t37_device::m48t37_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: timekeeper_device(mconfig, M48T37, "M48T37 Timekeeper", tag, owner, clock, "m48t37", __FILE__)
{
	m_offset_control = 0x7ff8;
	m_offset_seconds = 0x7ff9;
	m_offset_minutes = 0x7ffa;
	m_offset_hours = 0x7ffb;
	m_offset_day = 0x7ffc;
	m_offset_date = 0x7ffd;
	m_offset_month = 0x7ffe;
	m_offset_year = 0x7fff;
	m_offset_century = 0x7ff1;
	m_offset_flags = 0x7ff0;
	m_size = 0x8000;
}

m48t58_device::m48t58_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: timekeeper_device(mconfig, M48T58, "M48T58 Timekeeper", tag, owner, clock, "m48t58", __FILE__)
{
	m_offset_control = 0x1ff8;
	m_offset_seconds = 0x1ff9;
	m_offset_minutes = 0x1ffa;
	m_offset_hours = 0x1ffb;
	m_offset_day = 0x1ffc;
	m_offset_date = 0x1ffd;
	m_offset_month = 0x1ffe;
	m_offset_year = 0x1fff;
	m_offset_century = -1;
	m_offset_flags = -1;
	m_size = 0x2000;
}

mk48t08_device::mk48t08_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: timekeeper_device(mconfig, MK48T08, "MK48T08 Timekeeper", tag, owner, clock, "m48t08", __FILE__)
{
	m_offset_control = 0x1ff8;
	m_offset_seconds = 0x1ff9;
	m_offset_minutes = 0x1ffa;
	m_offset_hours = 0x1ffb;
	m_offset_day = 0x1ffc;
	m_offset_date = 0x1ffd;
	m_offset_month = 0x1ffe;
	m_offset_year = 0x1fff;
	m_offset_century = 0x1ff1;
	m_offset_flags = 0x1ff0;
	m_size = 0x2000;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void timekeeper_device::device_start()
{
	system_time systime;

	/* validate some basic stuff */
	assert(this != nullptr);

	machine().base_datetime(systime);

	m_control = 0;
	m_seconds = make_bcd( systime.local_time.second );
	m_minutes = make_bcd( systime.local_time.minute );
	m_hours = make_bcd( systime.local_time.hour );
	m_day = make_bcd( systime.local_time.weekday + 1 );
	m_date = make_bcd( systime.local_time.mday );
	m_month = make_bcd( systime.local_time.month + 1 );
	m_year = make_bcd( systime.local_time.year % 100 );
	m_century = make_bcd( systime.local_time.year / 100 );
	m_data.resize( m_size );

	if (m_default_data)
	{
		assert(m_default_data.bytes() == m_size);
	}

	save_item( NAME(m_control) );
	save_item( NAME(m_seconds) );
	save_item( NAME(m_minutes) );
	save_item( NAME(m_hours) );
	save_item( NAME(m_day) );
	save_item( NAME(m_date) );
	save_item( NAME(m_month) );
	save_item( NAME(m_year) );
	save_item( NAME(m_century) );
	save_item( NAME(m_data) );

	emu_timer *timer = timer_alloc();
	timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void timekeeper_device::device_reset() { }

void timekeeper_device::counters_to_ram()
{
	counter_to_ram( &m_data[0], m_offset_control, m_control );
	counter_to_ram( &m_data[0], m_offset_seconds, m_seconds );
	counter_to_ram( &m_data[0], m_offset_minutes, m_minutes );
	counter_to_ram( &m_data[0], m_offset_hours, m_hours );
	counter_to_ram( &m_data[0], m_offset_day, m_day );
	counter_to_ram( &m_data[0], m_offset_date, m_date );
	counter_to_ram( &m_data[0], m_offset_month, m_month );
	counter_to_ram( &m_data[0], m_offset_year, m_year );
	counter_to_ram( &m_data[0], m_offset_century, m_century );
}

void timekeeper_device::counters_from_ram()
{
	m_control = counter_from_ram( &m_data[0], m_offset_control );
	m_seconds = counter_from_ram( &m_data[0], m_offset_seconds );
	m_minutes = counter_from_ram( &m_data[0], m_offset_minutes );
	m_hours = counter_from_ram( &m_data[0], m_offset_hours );
	m_day = counter_from_ram( &m_data[0], m_offset_day );
	m_date = counter_from_ram( &m_data[0], m_offset_date );
	m_month = counter_from_ram( &m_data[0], m_offset_month );
	m_year = counter_from_ram( &m_data[0], m_offset_year );
	m_century = counter_from_ram( &m_data[0], m_offset_century );
}

void timekeeper_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if( ( m_seconds & SECONDS_ST ) != 0 ||
		( m_control & CONTROL_W ) != 0 )
	{
		return;
	}

	int carry = inc_bcd( &m_seconds, MASK_SECONDS, 0x00, 0x59 );
	if( carry )
	{
		carry = inc_bcd( &m_minutes, MASK_MINUTES, 0x00, 0x59 );
	}
	if( carry )
	{
		carry = inc_bcd( &m_hours, MASK_HOURS, 0x00, 0x23 );
	}

	if( carry )
	{
		UINT8 maxdays;
		static const UINT8 daysinmonth[] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };

		inc_bcd( &m_day, MASK_DAY, 0x01, 0x07 );

		UINT8 month = from_bcd( m_month );
		UINT8 year = from_bcd( m_year );

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

		carry = inc_bcd( &m_date, MASK_DATE, 0x01, maxdays );
	}
	if( carry )
	{
		carry = inc_bcd( &m_month, MASK_MONTH, 0x01, 0x12 );
	}
	if( carry )
	{
		carry = inc_bcd( &m_year, MASK_YEAR, 0x00, 0x99 );
	}
	if( carry )
	{
		carry = inc_bcd( &m_century, MASK_CENTURY, 0x00, 0x99 );

		if( type() == M48T35 ||
			type() == M48T58 )
		{
			if( ( m_day & DAY_CEB ) != 0 )
			{
				m_day ^= DAY_CB;
			}
		}
	}

	if( ( m_control & CONTROL_R ) == 0 )
	{
		counters_to_ram();
	}
}

WRITE8_MEMBER( timekeeper_device::write )
{
	if( offset == m_offset_control )
	{
		if( ( m_control & CONTROL_W ) != 0 &&
			( data & CONTROL_W ) == 0 )
		{
			counters_from_ram();
		}
		m_control = data;
	}
	else if( offset == m_offset_day )
	{
		if( type() == M48T35 ||
			type() == M48T58 )
		{
			m_day = ( m_day & ~DAY_CEB ) | ( data & DAY_CEB );
		}
	}

	m_data[ offset ] = data;
}

READ8_MEMBER( timekeeper_device::read )
{
	UINT8 result = m_data[ offset ];
	if( offset == m_offset_date && type() == M48T58 )
	{
		result &= ~DATE_BL;
	}
	else if( offset == m_offset_flags && (type() == MK48T08 || type() == M48T37) )
	{
		result = 0;
	}
	return result;
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void timekeeper_device::nvram_default()
{
	if( m_default_data != nullptr )
	{
		memcpy( &m_data[0], m_default_data, m_size );
	}
	else
	{
		memset( &m_data[0], 0xff, m_data.size());
	}

	if ( m_offset_flags >= 0 )
		m_data[ m_offset_flags ] = 0;
	counters_to_ram();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void timekeeper_device::nvram_read(emu_file &file)
{
	file.read( &m_data[0], m_size );

	counters_to_ram();
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void timekeeper_device::nvram_write(emu_file &file)
{
	file.write( &m_data[0], m_size );
}

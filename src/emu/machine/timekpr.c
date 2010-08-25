/***************************************************************************

    timekpr.h

    Various ST Microelectronics timekeeper SRAM implementations:
    	- M48T02
    	- M48T35
    	- M48T58
    	- MK48T08

***************************************************************************/

#include "emu.h"
#include "machine/timekpr.h"


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

#define DAY_FT ( 0x40 ) /* not emulated */
#define DAY_CEB ( 0x20 ) /* M48T35/M48T58 */
#define DAY_CB ( 0x10 ) /* M48T35/M48T58 */

#define DATE_BLE ( 0x80 ) /* M48T58: not emulated */
#define DATE_BL ( 0x40 ) /* M48T58: not emulated */

#define FLAGS_BL ( 0x10 ) /* MK48T08: not emulated */

#define TIMEKPR_DEV_DERIVED_CTOR(devtype) \
	devtype##_device::devtype##_device(running_machine &_machine, const devtype##_device_config &config) \
		: timekeeper_device(_machine, config) \
	{ }

#define TIMEKPR_DEVCFG_DERIVED_CTOR(devtype, name) \
	devtype##_device_config::devtype##_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock) \
    : timekeeper_device_config(mconfig, name, tag, owner, clock) \
	{ }

#define TIMEKPR_DEVCFG_DERIVED_STATIC_ALLOC(devtype) \
	device_config *devtype##_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock) \
	{ return global_alloc(devtype##_device_config(mconfig, tag, owner, clock)); }

#define TIMEKPR_DEVCFG_DERIVED_DEV_ALLOC(devtype) \
	device_t *devtype##_device_config::alloc_device(running_machine &machine) const \
	{ return auto_alloc(&machine, devtype##_device(machine, *this)); }

#define TIMEKPR_DERIVE(devtype, name) \
	TIMEKPR_DEV_DERIVED_CTOR(devtype) \
	TIMEKPR_DEVCFG_DERIVED_CTOR(devtype, name) \
	TIMEKPR_DEVCFG_DERIVED_STATIC_ALLOC(devtype) \
	TIMEKPR_DEVCFG_DERIVED_DEV_ALLOC(devtype)

//**************************************************************************
//  STATIC DATA
//**************************************************************************

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  timekeeper_device_config - constructor
//-------------------------------------------------

timekeeper_device_config::timekeeper_device_config(const machine_config &mconfig, const char *type, const char *tag, const device_config *owner, UINT32 clock)
    : device_config(mconfig, static_alloc_device_config, type, tag, owner, clock),
	  device_config_nvram_interface(mconfig, *this)
{

}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *timekeeper_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
    return global_alloc(timekeeper_device_config(mconfig, "TIMEKEEPER", tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *timekeeper_device_config::alloc_device(running_machine &machine) const
{
    return auto_alloc(&machine, timekeeper_device(machine, *this));
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

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

const device_type M48T02 = m48t02_device_config::static_alloc_device_config;
const device_type M48T35 = m48t35_device_config::static_alloc_device_config;
const device_type M48T58 = m48t58_device_config::static_alloc_device_config;
const device_type MK48T08 = mk48t08_device_config::static_alloc_device_config;

TIMEKPR_DERIVE(m48t02, "M48T02")
TIMEKPR_DERIVE(m48t35, "M48T35")
TIMEKPR_DERIVE(m48t58, "M48T58")
TIMEKPR_DERIVE(mk48t08, "MK48T08")

//-------------------------------------------------
//  timekeeper_device - constructor
//-------------------------------------------------

timekeeper_device::timekeeper_device(running_machine &_machine, const timekeeper_device_config &config)
    : device_t(_machine, config),
	  device_nvram_interface(_machine, config, *this),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void timekeeper_device::device_start()
{
	emu_timer *timer;
	attotime duration;
	system_time systime;

	/* validate some basic stuff */
	assert(this != NULL);

	m_machine.base_datetime(systime);

	m_control = 0;
	m_seconds = make_bcd( systime.local_time.second );
	m_minutes = make_bcd( systime.local_time.minute );
	m_hours = make_bcd( systime.local_time.hour );
	m_day = make_bcd( systime.local_time.weekday + 1 );
	m_date = make_bcd( systime.local_time.mday );
	m_month = make_bcd( systime.local_time.month + 1 );
	m_year = make_bcd( systime.local_time.year % 100 );
	m_century = make_bcd( systime.local_time.year / 100 );
	m_data = auto_alloc_array( &m_machine, UINT8, m_size );

	m_default_data = *region();
	if (m_default_data)
	{
		assert( region()->bytes() == m_size );
	}

	state_save_register_device_item( this, 0, m_control );
	state_save_register_device_item( this, 0, m_seconds );
	state_save_register_device_item( this, 0, m_minutes );
	state_save_register_device_item( this, 0, m_hours );
	state_save_register_device_item( this, 0, m_day );
	state_save_register_device_item( this, 0, m_date );
	state_save_register_device_item( this, 0, m_month );
	state_save_register_device_item( this, 0, m_year );
	state_save_register_device_item( this, 0, m_century );
	state_save_register_device_item_pointer( this, 0, m_data, m_size );

	timer = timer_alloc( &m_machine, timekeeper_tick_callback, (void *)this );
	duration = ATTOTIME_IN_SEC(1);
	timer_adjust_periodic( timer, duration, 0, duration );
}

void m48t02_device::device_start()
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

	timekeeper_device::device_start();
}

void m48t35_device::device_start()
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

	timekeeper_device::device_start();
}

void m48t58_device::device_start()
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

	timekeeper_device::device_start();
}

void mk48t08_device::device_start()
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

	timekeeper_device::device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void timekeeper_device::device_reset()
{

}


void timekeeper_device::counters_to_ram()
{
	counter_to_ram( m_data, m_offset_control, m_control );
	counter_to_ram( m_data, m_offset_seconds, m_seconds );
	counter_to_ram( m_data, m_offset_minutes, m_minutes );
	counter_to_ram( m_data, m_offset_hours, m_hours );
	counter_to_ram( m_data, m_offset_day, m_day );
	counter_to_ram( m_data, m_offset_date, m_date );
	counter_to_ram( m_data, m_offset_month, m_month );
	counter_to_ram( m_data, m_offset_year, m_year );
	counter_to_ram( m_data, m_offset_century, m_century );
}

void timekeeper_device::counters_from_ram()
{
	m_control = counter_from_ram( m_data, m_offset_control );
	m_seconds = counter_from_ram( m_data, m_offset_seconds );
	m_minutes = counter_from_ram( m_data, m_offset_minutes );
	m_hours = counter_from_ram( m_data, m_offset_hours );
	m_day = counter_from_ram( m_data, m_offset_day );
	m_date = counter_from_ram( m_data, m_offset_date );
	m_month = counter_from_ram( m_data, m_offset_month );
	m_year = counter_from_ram( m_data, m_offset_year );
	m_century = counter_from_ram( m_data, m_offset_century );
}

TIMER_CALLBACK( timekeeper_device::timekeeper_tick_callback )
{
    reinterpret_cast<timekeeper_device *>(ptr)->timekeeper_tick();
}

void timekeeper_device::timekeeper_tick()
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

void timekeeper_device::write(UINT16 offset, UINT8 data)
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
	else if( offset == m_offset_date && type() == M48T58 )
	{
		data &= ~DATE_BL;
	}
	else if( offset == m_offset_flags && type() == MK48T08 )
	{
		data &= ~FLAGS_BL;
	}

	m_data[ offset ] = data;
}

UINT8 timekeeper_device::read(UINT16 offset)
{
	return m_data[ offset ];
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void timekeeper_device::nvram_default()
{
	if( m_default_data != NULL )
	{
		memcpy( m_data, m_default_data, m_size );
	}
	else
	{
		memset( m_data, 0xff, m_size );
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void timekeeper_device::nvram_read(mame_file &file)
{
	mame_fread( &file, m_data, m_size );

	counters_to_ram();
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void timekeeper_device::nvram_write(mame_file &file)
{
	mame_fwrite( &file, m_data, m_size );
}



/***************************************************************************
    TRAMPOLINES
***************************************************************************/

WRITE8_DEVICE_HANDLER( timekeeper_w ) { downcast<timekeeper_device *>(device)->write(offset, data); }
READ8_DEVICE_HANDLER( timekeeper_r ) { return downcast<timekeeper_device*>(device)->read(offset); }


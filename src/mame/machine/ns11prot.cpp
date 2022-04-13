// license:BSD-3-Clause
// copyright-holders:smf
/*
 * Namco System 11 Protection
 *
 * The 409 has been decapped and verified to be a ULA, so probably every keycus is.
 */

#include "emu.h"
#include "ns11prot.h"

ns11_keycus_device::ns11_keycus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
{
}

void ns11_keycus_device::device_start()
{
	save_item( NAME( m_p1 ) );
	save_item( NAME( m_p2 ) );
	save_item( NAME( m_p3 ) );
}

void ns11_keycus_device::device_reset()
{
	m_p1 = 0;
	m_p2 = 0;
	m_p3 = 0;
}

/* tekken 2 */

keycus_c406_device::keycus_c406_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C406, tag, owner, clock)
{
}

uint16_t keycus_c406_device::read(offs_t offset)
{
	if( offset == 0 && m_p1 == 0x1234 && m_p2 == 0x5678 && m_p3 == 0x000f )
	{
		return 0x3256;
	}

	logerror( "keycus_c406_device::read unexpected offset=%d m_p1=%04x m_p2=%04x m_p3=%04x\n", offset, m_p1, m_p2, m_p3 );
	return machine().rand();
}

void keycus_c406_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 1:
		m_p1 = data;
		return;

	case 2:
		m_p2 = data;
		return;

	case 3:
		m_p3 = data;
		return;
	}

	logerror( "keycus_c406_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C406, keycus_c406_device, "keycus_c406", "Namco C406 KEYCUS")

/* soul edge */

keycus_c409_device::keycus_c409_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C409, tag, owner, clock)
{
}

uint16_t keycus_c409_device::read(offs_t offset)
{
	if( offset == 7 )
	{
		int a2 = (m_p1 - 0x01) & 0x1f;
		int a3 = (0x20 - m_p1) & 0x1f;
		int r = (((m_p2 & 0x1f) * a2) + ((m_p3 & 0x1f) * a3)) / 0x1f;
		int g = ((((m_p2 >> 5) & 0x1f) * a2) + (((m_p3 >> 5) & 0x1f) * a3)) / 0x1f;
		int b = ((((m_p2 >> 10) & 0x1f) * a2) + (((m_p3 >> 10) & 0x1f) * a3)) / 0x1f;
		return r | (g << 5) | (b << 10);
	}

	logerror( "keycus_c409_device::read unexpected offset=%d m_p1=%04x m_p2=%04x m_p3=%04x\n", offset, m_p1, m_p2, m_p3 );
	return machine().rand();
}

void keycus_c409_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 1:
		m_p1 = data;
		return;

	case 3:
		m_p2 = data;
		return;

	case 7:
		m_p3 = data;
		return;
	}

	logerror( "keycus_c409_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C409, keycus_c409_device, "keycus_c409", "Namco C409 KEYCUS")

/* dunk mania */

keycus_c410_device::keycus_c410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C410, tag, owner, clock)
{
}

uint16_t keycus_c410_device::read(offs_t offset)
{
	if( m_p2 == 0 )
	{
		uint16_t value = m_p1;
		if( value == 0xfffe )
		{
			value = 410;
		}

		switch(offset)
		{
		case 1:
			return ( ( value / 1 ) % 10 );

		case 2:
			return ( ( value / 100 ) % 10 ) |
				( ( ( value / 1000 ) % 10 ) << 8 );

		case 3:
			return ( ( value / 10000 ) % 10 ) |
				( ( ( value / 10 ) % 10 ) << 8 );
		}
	}

	logerror( "keycus_c410_device::read unexpected offset=%d m_p1=%04x m_p2=%04x\n", offset, m_p1, m_p2 );
	return machine().rand();
}

void keycus_c410_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 0:
		m_p1 = data;
		return;

	case 2:
		m_p2 = data;
		return;
	}

	logerror( "keycus_c410_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C410, keycus_c410_device, "keycus_c410", "Namco C410 KEYCUS")

/* prime goal ex */

keycus_c411_device::keycus_c411_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C411, tag, owner, clock)
{
}

uint16_t keycus_c411_device::read(offs_t offset)
{
	if( m_p2 == 0x0000 && ( ( ( m_p1 == 0x0000 || m_p1 == 0x100 ) && m_p3 == 0xff7f ) || ( m_p1 == 0x7256 ) ) )
	{
		uint16_t value = m_p3;
		if( m_p1 != 0x7256 )
		{
			value = 411;
		}

		switch( offset )
		{
		case 0:
			return ( ( ( value / 10 ) % 10 ) << 8 ) | ( ( value / 1 ) % 10 );

		case 2:
			return ( ( ( value / 1000 ) % 10 ) << 8 ) | ( ( value / 100 ) % 10 );

		case 8:
			return ( ( value / 10000 ) % 10 );
		}
	}

	logerror( "keycus_c411_device::read unexpected offset=%d m_p1=%04x m_p2=%04x m_p3=%04x\n", offset, m_p1, m_p2, m_p3 );
	return machine().rand();
}

void keycus_c411_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 2:
		m_p1 = data;
		return;

	case 8:
		m_p2 = data;
		return;

	case 10:
		m_p3 = data;
		return;
	}

	logerror( "keycus_c411_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C411, keycus_c411_device, "keycus_c411", "Namco C411 KEYCUS")

/* xevious 3d/g */

keycus_c430_device::keycus_c430_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C430, tag, owner, clock)
{
}

uint16_t keycus_c430_device::read(offs_t offset)
{
	if( m_p2 == 0x0000 && ( ( m_p1 == 0xbfff && m_p3 == 0x0000 ) || m_p3 == 0xe296 ) )
	{
		uint16_t value = m_p1;

		if( m_p3 != 0xe296 )
		{
			value = 430;
		}

		switch( offset )
		{
		case 1:
			return ( ( value / 10000 ) % 10 );

		case 4:
			return ( ( value / 100 ) % 10 ) |
				( ( ( value / 1000 ) % 10 ) << 8 );

		case 5:
			return ( ( value / 1 ) % 10 ) |
				( ( ( value / 10 ) % 10 ) << 8 );
		}
	}

	logerror( "keycus_c430_device::read unexpected offset=%d m_p1=%04x m_p2=%04x\n", offset, m_p1, m_p2 );
	return machine().rand();
}

void keycus_c430_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 0:
		m_p1 = data;
		return;

	case 1:
		m_p2 = data;
		return;

	case 4:
		m_p3 = data;
		return;
	}

	logerror( "keycus_c430_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C430, keycus_c430_device, "keycus_c430", "Namco C430 KEYCUS")

/* dancing eyes */

keycus_c431_device::keycus_c431_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C431, tag, owner, clock)
{
}

uint16_t keycus_c431_device::read(offs_t offset)
{
	if( m_p2 == 0x0000 && ( ( ( m_p1 == 0x0000 || m_p1 == 0xab50 ) && m_p3 == 0x7fff ) || m_p1 == 0x9e61 ) )
	{
		uint16_t value = m_p3;

		if( m_p1 != 0x9e61 )
		{
			value = 431;
		}

		switch( offset )
		{
		case 0:
			return  ( ( value / 1 ) % 10 ) |
				( ( ( value / 10 ) % 10 ) << 8 );
		case 4:
			return  ( ( value / 100 ) % 10 ) |
				( ( ( value / 1000 ) % 10 ) << 8 );

		case 8:
			return ( value / 10000 ) % 10;
		}
	}

	logerror( "keycus_c431_device::read unexpected offset=%d m_p1=%04x m_p2=%04x m_p3=%04x\n", offset, m_p1, m_p2, m_p3 );
	return machine().rand();
}

void keycus_c431_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 0:
		m_p1 = data;
		return;

	case 4:
		m_p2 = data;
		return;

	case 12:
		m_p3 = data;
		return;
	}

	logerror( "keycus_c431_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C431, keycus_c431_device, "keycus_c431", "Namco C431 KEYCUS")

/* pocket racer */

keycus_c432_device::keycus_c432_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C432, tag, owner, clock)
{
}

uint16_t keycus_c432_device::read(offs_t offset)
{
	if( m_p1 == 0x0000 && ( ( ( m_p3 == 0x0000 || m_p3 == 0x00dc ) && m_p2 == 0xefff ) || m_p3 == 0x2f15 ) )
	{
		uint16_t value = m_p2;

		if( m_p3 != 0x00002f15 )
		{
			value = 432;
		}

		switch( offset )
		{
		case 2:
			return ( ( value / 1 ) % 10 ) |
				( ( ( value / 10 ) % 10 ) << 8 );

		case 4:
			return ( ( value / 100 ) % 10 ) |
				( ( ( value / 1000 ) % 10 ) << 8 );

		case 6:
			return  ( ( value / 10000 ) % 10 ) |
				( ( ( value / 100000 ) % 10 ) << 8 );
		}
	}

	logerror( "keycus_c432_device::read unexpected offset=%d m_p1=%04x m_p2=%04x m_p3=%04x\n", offset, m_p1, m_p2, m_p3 );
	return machine().rand();
}

void keycus_c432_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 0:
		m_p1 = data;
		return;

	case 2:
		m_p2 = data;
		return;

	case 6:
		m_p3 = data;
		return;
	}

	logerror( "keycus_c432_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C432, keycus_c432_device, "keycus_c432", "Namco C432 KEYCUS")

/* star sweep */

keycus_c442_device::keycus_c442_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C442, tag, owner, clock)
{
}

uint16_t keycus_c442_device::read(offs_t offset)
{
	if( ( offset == 0 && m_p1 == 0x0020 && m_p2 == 0x0000 ) ||
		( offset == 0 && m_p1 == 0x0020 && m_p2 == 0x0021 ) )
	{
		return 0x0000;
	}

	if( ( offset == 1 && m_p1 == 0x0020 && m_p2 == 0x0020 ) ||
		( offset == 1 && m_p1 == 0x0020 && m_p2 == 0x3af2 ) )
	{
		return 0x0000;
	}

	if( ( offset == 1 && m_p1 == 0x0020 && m_p2 == 0x0021 ) )
	{
		return 0xc442;
	}

	logerror( "keycus_c442_device::read unexpected offset=%d m_p1=%04x m_p2=%04x\n", offset, m_p1, m_p2 );
	return machine().rand();
}

void keycus_c442_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 0:
		m_p1 = data;
		return;

	case 1:
		m_p2 = data;
		return;
	}

	logerror( "keycus_c442_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C442, keycus_c442_device, "keycus_c442", "Namco C442 KEYCUS")

/* kosodate quiz my angel 3 / point blank 2 */

keycus_c443_device::keycus_c443_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ns11_keycus_device(mconfig, KEYCUS_C443, tag, owner, clock)
{
}

uint16_t keycus_c443_device::read(offs_t offset)
{
	if( offset == 0 && m_p1 == 0x0020 && ( m_p2 == 0x0000 || m_p2 == 0xffff || m_p2 == 0xffe0 ) )
	{
		return 0x0020;
	}

	if( offset == 1 && m_p1 == 0x0020 && m_p2 == 0xffdf )
	{
		return 0x0000;
	}

	if( offset == 1 && m_p1 == 0x0020 && ( m_p2 == 0xffff || m_p2 == 0xffe0 ) )
	{
		return 0xc443;
	}

	logerror( "keycus_c443_device::read unexpected offset=%d m_p1=%04x m_p2=%04x\n", offset, m_p1, m_p2 );
	return machine().rand();
}

void keycus_c443_device::write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
	case 0:
		m_p1 = data;
		return;

	case 1:
		m_p2 = data;
		return;
	}

	logerror( "keycus_c443_device::write unexpected offset=%d data=%04x\n", offset, data );
}

DEFINE_DEVICE_TYPE(KEYCUS_C443, keycus_c443_device, "keycus_c443", "Namco C443 KEYCUS")

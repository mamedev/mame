// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation IRQ emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "psx.h"
#include "irq.h"

#define VERBOSE_LEVEL ( 0 )

#define PSX_IRQ_MASK ( 0x7fd )

INLINE void ATTR_PRINTF(3,4) verboselog( device_t& device, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device.logerror( "%s: %s", device.machine().describe_context(), buf );
	}
}

const device_type PSX_IRQ = &device_creator<psxirq_device>;

psxirq_device::psxirq_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PSX_IRQ, "Sony PSX IRQ", tag, owner, clock, "psxirq", __FILE__), n_irqdata(0), n_irqmask(0),
	m_irq_handler(*this)
{
}

void psxirq_device::device_reset()
{
	n_irqdata = 0;
	n_irqmask = 0;

	psx_irq_update();
}

void psxirq_device::device_post_load()
{
	psx_irq_update();
}

void psxirq_device::device_start()
{
	m_irq_handler.resolve_safe();

	save_item( NAME( n_irqdata ) );
	save_item( NAME( n_irqmask ) );
}

void psxirq_device::set( UINT32 bitmask )
{
	verboselog( *this, 2, "psx_irq_set %08x\n", bitmask );
	n_irqdata |= bitmask;
	psx_irq_update();
}

void psxirq_device::psx_irq_update( void )
{
	if( ( n_irqdata & n_irqmask ) != 0 )
	{
		verboselog( *this, 2, "psx irq assert\n" );
		m_irq_handler( ASSERT_LINE );
	}
	else
	{
		verboselog( *this, 2, "psx irq clear\n" );
		m_irq_handler( CLEAR_LINE );
	}
}

WRITE32_MEMBER( psxirq_device::write )
{
	switch( offset )
	{
	case 0x00:
		verboselog( *this, 2, "psx irq data ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, n_irqdata, ( n_irqdata & ~mem_mask ) | ( n_irqdata & n_irqmask & data ) );
		n_irqdata = ( n_irqdata & ~mem_mask ) | ( n_irqdata & n_irqmask & data );
		psx_irq_update();
		break;
	case 0x01:
		verboselog( *this, 2, "psx irq mask ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, n_irqmask, ( n_irqmask & ~mem_mask ) | data );
		n_irqmask = ( n_irqmask & ~mem_mask ) | data;
		if( ( n_irqmask &~ PSX_IRQ_MASK ) != 0 )
		{
			verboselog( *this, 0, "psx_irq_w( %08x, %08x, %08x ) unknown irq\n", offset, data, mem_mask );
		}
		psx_irq_update();
		break;
	default:
		verboselog( *this, 0, "psx_irq_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}

READ32_MEMBER( psxirq_device::read )
{
	switch( offset )
	{
	case 0x00:
		verboselog( *this, 1, "psx_irq_r irq data %08x\n", n_irqdata );
		return n_irqdata;
	case 0x01:
		verboselog( *this, 1, "psx_irq_r irq mask %08x\n", n_irqmask );
		return n_irqmask;
	default:
		verboselog( *this, 0, "psx_irq_r unknown register %d\n", offset );
		break;
	}
	return 0;
}

WRITE_LINE_MEMBER( psxirq_device::intin0 )
{
	if( state )
	{
		set( 1 << 0 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin1 )
{
	if( state )
	{
		set( 1 << 1 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin2 )
{
	if( state )
	{
		set( 1 << 2 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin3 )
{
	if( state )
	{
		set( 1 << 3 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin4 )
{
	if( state )
	{
		set( 1 << 4 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin5 )
{
	if( state )
	{
		set( 1 << 5 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin6 )
{
	if( state )
	{
		set( 1 << 6 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin7 )
{
	if( state )
	{
		set( 1 << 7 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin8 )
{
	if( state )
	{
		set( 1 << 8 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin9 )
{
	if( state )
	{
		set( 1 << 9 );
	}
}

WRITE_LINE_MEMBER( psxirq_device::intin10 )
{
	if( state )
	{
		set( 1 << 10 );
	}
}

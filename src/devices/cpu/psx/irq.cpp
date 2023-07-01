// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation IRQ emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "emu.h"
#include "psx.h"
#include "irq.h"

#define VERBOSE ( 0 )
#include "logmacro.h"

#define PSX_IRQ_MASK ( 0x7fd )

DEFINE_DEVICE_TYPE(PSX_IRQ, psxirq_device, "psxirq", "Sony PSX IRQ")

psxirq_device::psxirq_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSX_IRQ, tag, owner, clock), n_irqdata(0), n_irqmask(0),
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
	save_item(NAME(n_irqdata));
	save_item(NAME(n_irqmask));
}

void psxirq_device::set( uint32_t bitmask )
{
	LOG( "%s: psx_irq_set %08x\n", machine().describe_context(), bitmask );
	n_irqdata |= bitmask;
	psx_irq_update();
}

void psxirq_device::psx_irq_update( void )
{
	if( ( n_irqdata & n_irqmask ) != 0 )
	{
		LOG( "%s: psx irq assert\n", machine().describe_context() );
		m_irq_handler( ASSERT_LINE );
	}
	else
	{
		LOG( "%s: psx irq clear\n", machine().describe_context() );
		m_irq_handler( CLEAR_LINE );
	}
}

void psxirq_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch( offset )
	{
	case 0x00:
		LOG( "%s: psx irq data ( %08x, %08x ) %08x -> %08x\n", machine().describe_context(), data, mem_mask, n_irqdata, ( n_irqdata & ~mem_mask ) | ( n_irqdata & n_irqmask & data ) );
		n_irqdata = ( n_irqdata & ~mem_mask ) | ( n_irqdata & n_irqmask & data );
		psx_irq_update();
		break;
	case 0x01:
		LOG( "%s: psx irq mask ( %08x, %08x ) %08x -> %08x\n", machine().describe_context(), data, mem_mask, n_irqmask, ( n_irqmask & ~mem_mask ) | data );
		n_irqmask = ( n_irqmask & ~mem_mask ) | data;
		if( ( n_irqmask &~ PSX_IRQ_MASK ) != 0 )
		{
			logerror( "%s: psx_irq_w( %08x, %08x, %08x ) unknown irq\n", machine().describe_context(), offset, data, mem_mask );
		}
		psx_irq_update();
		break;
	default:
		logerror( "%s: psx_irq_w( %08x, %08x, %08x ) unknown register\n", machine().describe_context(), offset, data, mem_mask );
		break;
	}
}

uint32_t psxirq_device::read(offs_t offset)
{
	switch( offset )
	{
	case 0x00:
		LOG( "%s: psx_irq_r irq data %08x\n", machine().describe_context(), n_irqdata );
		return n_irqdata;
	case 0x01:
		LOG( "%s: psx_irq_r irq mask %08x\n", machine().describe_context(), n_irqmask );
		return n_irqmask;
	default:
		logerror( "%s: psx_irq_r unknown register %d\n", machine().describe_context(), offset );
		break;
	}
	return 0;
}

void psxirq_device::intin0(int state)
{
	if( state )
	{
		set( 1 << 0 );
	}
}

void psxirq_device::intin1(int state)
{
	if( state )
	{
		set( 1 << 1 );
	}
}

void psxirq_device::intin2(int state)
{
	if( state )
	{
		set( 1 << 2 );
	}
}

void psxirq_device::intin3(int state)
{
	if( state )
	{
		set( 1 << 3 );
	}
}

void psxirq_device::intin4(int state)
{
	if( state )
	{
		set( 1 << 4 );
	}
}

void psxirq_device::intin5(int state)
{
	if( state )
	{
		set( 1 << 5 );
	}
}

void psxirq_device::intin6(int state)
{
	if( state )
	{
		set( 1 << 6 );
	}
}

void psxirq_device::intin7(int state)
{
	if( state )
	{
		set( 1 << 7 );
	}
}

void psxirq_device::intin8(int state)
{
	if( state )
	{
		set( 1 << 8 );
	}
}

void psxirq_device::intin9(int state)
{
	if( state )
	{
		set( 1 << 9 );
	}
}

void psxirq_device::intin10(int state)
{
	if( state )
	{
		set( 1 << 10 );
	}
}

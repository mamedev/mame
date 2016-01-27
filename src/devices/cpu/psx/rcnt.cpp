// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation Root Counter emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "rcnt.h"

#define VERBOSE_LEVEL ( 0 )

static inline void ATTR_PRINTF(3,4) verboselog( device_t& device, int n_level, const char *s_fmt, ... )
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

const device_type PSX_RCNT = &device_creator<psxrcnt_device>;

psxrcnt_device::psxrcnt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PSX_RCNT, "Sony PSX RCNT", tag, owner, clock, "psxrcnt", __FILE__),
	m_irq0_handler(*this),
	m_irq1_handler(*this),
	m_irq2_handler(*this)
{
}

void psxrcnt_device::device_reset()
{
}

void psxrcnt_device::device_post_load()
{
	int n;
	for( n = 0; n < 3; n++ )
	{
		root_timer_adjust( n );
	}
}

void psxrcnt_device::device_start()
{
	int n;

	m_irq0_handler.resolve_safe();
	m_irq1_handler.resolve_safe();
	m_irq2_handler.resolve_safe();

	for( n = 0; n < 3; n++ )
	{
		root_counter[ n ].timer = timer_alloc(n);
		save_item(NAME(root_counter[ n ].n_count), n);
		save_item(NAME(root_counter[ n ].n_mode), n);
		save_item(NAME(root_counter[ n ].n_target), n);
		save_item(NAME(root_counter[ n ].n_start), n);
		root_counter[ n ].n_count = 0;
		root_counter[ n ].n_mode = 0;
		root_counter[ n ].n_target = 0;
		root_counter[ n ].n_start = 0;
	}
}

WRITE32_MEMBER( psxrcnt_device::write )
{
	int n_counter = offset / 4;
	psx_root *root = &root_counter[ n_counter ];

	verboselog( *this, 1, "psx_counter_w ( %08x, %08x, %08x )\n", offset, data, mem_mask );

	switch( offset % 4 )
	{
	case 0:
		root->n_count = data;
		root->n_start = gettotalcycles();
		break;
	case 1:
		root->n_count = root_current( n_counter );
		root->n_start = gettotalcycles();

		if( ( data & PSX_RC_RESET ) != 0 )
		{
			data &= ~( PSX_RC_RESET | PSX_RC_STOP );
			root->n_count = 0;
		}

		root->n_mode = data;

#if 0
		if( ( data & 0xfca6 ) != 0 ||
			( ( data & 0x0100 ) != 0 && n_counter != 0 && n_counter != 1 ) ||
			( ( data & 0x0200 ) != 0 && n_counter != 2 ) )
		{
			osd_printf_debug( "mode %d 0x%04x\n", n_counter, data & 0xfca6 );
		}
#endif
		break;
	case 2:
		root->n_target = data;
		break;
	default:
		verboselog( *this, 0, "psx_counter_w( %08x, %08x, %08x ) unknown register\n", offset, mem_mask, data );
		return;
	}

	root_timer_adjust( n_counter );
}

READ32_MEMBER( psxrcnt_device::read )
{
	int n_counter = offset / 4;
	psx_root *root = &root_counter[ n_counter ];
	UINT32 data;

	switch( offset % 4 )
	{
	case 0:
		data = root_current( n_counter );
		break;
	case 1:
		data = root->n_mode;
		break;
	case 2:
		data = root->n_target;
		break;
	default:
		verboselog( *this, 0, "psx_counter_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		return 0;
	}
	verboselog( *this, 1, "psx_counter_r ( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

UINT64 psxrcnt_device::gettotalcycles( void )
{
	/* TODO: should return the start of the current tick. */
	return ((cpu_device *)owner())->total_cycles() * 2;
}

int psxrcnt_device::root_divider( int n_counter )
{
	psx_root *root = &root_counter[ n_counter ];

	if( n_counter == 0 && ( root->n_mode & PSX_RC_CLC ) != 0 )
	{
		/* TODO: pixel clock, probably based on resolution */
		return 5;
	}
	else if( n_counter == 1 && ( root->n_mode & PSX_RC_CLC ) != 0 )
	{
		return 2150;
	}
	else if( n_counter == 2 && ( root->n_mode & PSX_RC_DIV ) != 0 )
	{
		return 8;
	}
	return 1;
}

UINT16 psxrcnt_device::root_current( int n_counter )
{
	psx_root *root = &root_counter[ n_counter ];

	if( ( root->n_mode & PSX_RC_STOP ) != 0 )
	{
		return root->n_count;
	}
	else
	{
		UINT64 n_current;
		n_current = gettotalcycles() - root->n_start;
		n_current /= root_divider( n_counter );
		n_current += root->n_count;
		if( n_current > 0xffff )
		{
			/* TODO: use timer for wrap on 0x10000. */
			root->n_count = n_current;
			root->n_start = gettotalcycles();
		}
		return n_current;
	}
}

int psxrcnt_device::root_target( int n_counter )
{
	psx_root *root = &root_counter[ n_counter ];

	if( ( root->n_mode & PSX_RC_COUNTTARGET ) != 0 ||
		( root->n_mode & PSX_RC_IRQTARGET ) != 0 )
	{
		return root->n_target;
	}
	return 0x10000;
}

void psxrcnt_device::root_timer_adjust( int n_counter )
{
	psx_root *root = &root_counter[ n_counter ];

	if( ( root->n_mode & PSX_RC_STOP ) != 0 )
	{
		root->timer->adjust( attotime::never, n_counter);
	}
	else
	{
		int n_duration;

		n_duration = root_target( n_counter ) - root_current( n_counter );
		if( n_duration < 1 )
		{
			n_duration += 0x10000;
		}

		n_duration *= root_divider( n_counter );

		// TODO: figure out if this should be calculated from the cpu clock for 50mhz boards?
		root->timer->adjust( attotime::from_hz(33868800) * n_duration, n_counter);
	}
}

void psxrcnt_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int n_counter = id;
	psx_root *root = &root_counter[ n_counter ];

	verboselog( *this, 2, "root_finished( %d ) %04x\n", n_counter, root_current( n_counter ) );
	//if( ( root->n_mode & PSX_RC_COUNTTARGET ) != 0 )
	{
		/* TODO: wrap should be handled differently as PSX_RC_COUNTTARGET & PSX_RC_IRQTARGET don't have to be the same. */
		root->n_count = 0;
		root->n_start = gettotalcycles();
	}
	if( ( root->n_mode & PSX_RC_REPEAT ) != 0 )
	{
		root_timer_adjust( n_counter );
	}
	if( ( root->n_mode & PSX_RC_IRQOVERFLOW ) != 0 ||
		( root->n_mode & PSX_RC_IRQTARGET ) != 0 )
	{
		switch( n_counter )
		{
		case 0:
			m_irq0_handler(1);
			break;
		case 1:
			m_irq1_handler(1);
			break;
		case 2:
			m_irq2_handler(1);
			break;
		}
	}
}

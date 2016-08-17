// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PlayStation DMA emulator
 *
 * Copyright 2003-2011 smf
 *
 */

#include "emu.h"
#include "dma.h"

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

const device_type PSX_DMA = &device_creator<psxdma_device>;

psxdma_device::psxdma_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PSX_DMA, "Sony PSX DMA", tag, owner, clock, "psxdma", __FILE__), m_ram( ), m_ramsize(0), m_dpcp(0), m_dicr(0),
	m_irq_handler(*this)
{
}

void psxdma_device::device_reset()
{
	int n;

	m_dpcp = 0;
	m_dicr = 0;

	for( n = 0; n < 7; n++ )
	{
		dma_stop_timer( n );
	}
}

void psxdma_device::device_post_load()
{
	int n;

	for( n = 0; n < 7; n++ )
	{
		dma_timer_adjust( n );
	}
}

void psxdma_device::device_start()
{
	m_irq_handler.resolve_safe();

	for( int index = 0; index < 7; index++ )
	{
		psx_dma_channel *dma = &m_channel[ index ];

		dma->timer = timer_alloc(index);

		save_item( NAME( dma->n_base ), index );
		save_item( NAME( dma->n_blockcontrol ), index );
		save_item( NAME( dma->n_channelcontrol ), index );
		save_item( NAME( dma->n_ticks ), index );
		save_item( NAME( dma->b_running ), index );
	}

	save_item( NAME(m_dpcp) );
	save_item( NAME(m_dicr) );
}

void psxdma_device::dma_start_timer( int index, UINT32 n_ticks )
{
	psx_dma_channel *dma = &m_channel[ index ];

	dma->timer->adjust( attotime::from_hz(33868800) * n_ticks, index);
	dma->n_ticks = n_ticks;
	dma->b_running = 1;
}

void psxdma_device::dma_stop_timer( int index )
{
	psx_dma_channel *dma = &m_channel[ index ];

	dma->timer->adjust( attotime::never);
	dma->b_running = 0;
}

void psxdma_device::dma_timer_adjust( int index )
{
	psx_dma_channel *dma = &m_channel[ index ];

	if( dma->b_running )
	{
		dma_start_timer( index, dma->n_ticks );
	}
	else
	{
		dma_stop_timer( index );
	}
}

void psxdma_device::dma_interrupt_update()
{
	int n_int;
	int n_mask;

	n_int = ( m_dicr >> 24 ) & 0x7f;
	n_mask = ( m_dicr >> 16 ) & 0xff;

	if( ( n_mask & 0x80 ) != 0 && ( n_int & n_mask ) != 0 )
	{
		verboselog( *this, 2, "dma_interrupt_update( %02x, %02x ) interrupt triggered\n", n_int, n_mask );
		m_dicr |= 0x80000000;
		m_irq_handler(1);
	}
	else if( n_int != 0 )
	{
		verboselog( *this, 2, "dma_interrupt_update( %02x, %02x ) interrupt not enabled\n", n_int, n_mask );
	}
	m_dicr &= 0x00ffffff | ( m_dicr << 8 );
}

void psxdma_device::dma_finished( int index )
{
	psx_dma_channel *dma = &m_channel[ index ];

	if( dma->n_channelcontrol == 0x01000401 && index == 2 )
	{
		UINT32 n_size;
		UINT32 n_total;
		UINT32 n_address = ( dma->n_base & 0xffffff );
		UINT32 n_adrmask = m_ramsize - 1;
		UINT32 n_nextaddress;

		if( n_address != 0xffffff )
		{
			n_total = 0;
			for( ;; )
			{
				if( n_address == 0xffffff )
				{
					dma->n_base = n_address;
					//HACK: fixes pse bios 2.x & other texture uploading issues, breaks kdeadeye test mode, gtrfrk7m & gtrkfrk8m loading
					//dma_start_timer( index, 19000 );
					dma_start_timer( index, 500 );
					return;
				}
				if( n_total > 65535 )
				{
					dma->n_base = n_address;
					//FIXME:
					// 16000 below is based on try and error.
					// Mametesters.org: sfex20103red
					//dma_start_timer( index, 16 );
					dma_start_timer( index, 16000 );
					return;
				}
				n_address &= n_adrmask;
				n_nextaddress = m_ram[ n_address / 4 ];
				n_size = n_nextaddress >> 24;
				dma->fn_write( m_ram, n_address + 4, n_size );
				//FIXME:
				// The following conditions will cause an endless loop.
				// If stopping the transfer is correct I cannot judge
				// The patch is meant as a hint for somebody who knows
				// the hardware.
				// Mametesters.org: psyforce0105u5red, raystorm0111u1red
				if ((n_nextaddress & 0xffffff) != 0xffffff)
				{
					if (n_address == m_ram[ (n_nextaddress & n_adrmask) / 4] ||
						n_address == (n_nextaddress & n_adrmask) )
					{
						break;
					}
				}
				n_address = ( n_nextaddress & 0xffffff );

				n_total += ( n_size + 1 );
			}
		}
	}

	dma->n_channelcontrol &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );

	m_dicr |= 1 << ( 24 + index );
	dma_interrupt_update();
	dma_stop_timer( index );
}

void psxdma_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	dma_finished(id);
}

void psxdma_device::install_read_handler( int index, psx_dma_read_delegate p_fn_dma_read )
{
	m_channel[ index ].fn_read = p_fn_dma_read;
}

void psxdma_device::install_write_handler( int index, psx_dma_read_delegate p_fn_dma_write )
{
	m_channel[ index ].fn_write = p_fn_dma_write;
}

WRITE32_MEMBER( psxdma_device::write )
{
	int index = offset / 4;
	psx_dma_channel *dma = &m_channel[ index ];

	if( index < 7 )
	{
		switch( offset % 4 )
		{
		case 0:
			verboselog( *this, 2, "dmabase( %d ) = %08x\n", index, data );
			dma->n_base = data;
			break;
		case 1:
			verboselog( *this, 2, "dmablockcontrol( %d ) = %08x\n", index, data );
			dma->n_blockcontrol = data;
			break;
		case 2:
			verboselog( *this, 2, "dmachannelcontrol( %d ) = %08x\n", index, data );
			dma->n_channelcontrol = data;
			if( ( dma->n_channelcontrol & ( 1L << 0x18 ) ) != 0 && ( m_dpcp & ( 1 << ( 3 + ( index * 4 ) ) ) ) != 0 )
			{
				INT32 n_size;
				UINT32 n_address;
				UINT32 n_nextaddress;
				UINT32 n_adrmask;

				n_adrmask = m_ramsize - 1;

				n_address = ( dma->n_base & n_adrmask );
				n_size = dma->n_blockcontrol;
				if( ( dma->n_channelcontrol & 0x200 ) != 0 )
				{
					UINT32 n_ba;
					n_ba = dma->n_blockcontrol >> 16;
					if( n_ba == 0 )
					{
						n_ba = 0x10000;
					}
					n_size = ( n_size & 0xffff ) * n_ba;
				}

				if( dma->n_channelcontrol == 0x01000000 &&
					!dma->fn_read.isnull() )
				{
					verboselog( *this, 1, "dma %d read block %08x %08x\n", index, n_address, n_size );
					dma->fn_read( m_ram, n_address, n_size );
					dma_finished( index );
				}
				else if ((dma->n_channelcontrol & 0xffbffeff) == 0x11000000 && // CD DMA
					!dma->fn_read.isnull() )
				{
					verboselog( *this, 1, "dma %d read block %08x %08x\n", index, n_address, n_size );

					// pSX's CD DMA size calc formula
					int oursize = (dma->n_blockcontrol>>16);
					oursize = (oursize > 1) ? oursize : 1;
					oursize *= (dma->n_blockcontrol&0xffff);

					dma->fn_read( m_ram, n_address, oursize );
					dma_finished( index );
				}
				else if( dma->n_channelcontrol == 0x01000200 &&
					!dma->fn_read.isnull() )
				{
					verboselog( *this, 1, "dma %d read block %08x %08x\n", index, n_address, n_size );
					dma->fn_read( m_ram, n_address, n_size );
					if( index == 1 )
					{
						dma_start_timer( index, 26000 );
					}
					else
					{
						dma_finished( index );
					}
				}
				else if( dma->n_channelcontrol == 0x01000201 &&
					!dma->fn_write.isnull() )
				{
					verboselog( *this, 1, "dma %d write block %08x %08x\n", index, n_address, n_size );
					dma->fn_write( m_ram, n_address, n_size );
					dma_finished( index );
				}
				else if( dma->n_channelcontrol == 0x11050100 &&
					!dma->fn_write.isnull() )
				{
					/* todo: check this is a write not a read... */
					verboselog( *this, 1, "dma %d write block %08x %08x\n", index, n_address, n_size );
					dma->fn_write( m_ram, n_address, n_size );
					dma_finished( index );
				}
				else if( dma->n_channelcontrol == 0x11150100 &&
					!dma->fn_write.isnull() )
				{
					/* todo: check this is a write not a read... */
					verboselog( *this, 1, "dma %d write block %08x %08x\n", index, n_address, n_size );
					dma->fn_write( m_ram, n_address, n_size );
					dma_finished( index );
				}
				else if( dma->n_channelcontrol == 0x01000401 &&
					index == 2 &&
					!dma->fn_write.isnull() )
				{
					verboselog( *this, 1, "dma %d write linked list %08x\n",
						index, dma->n_base );

					dma_finished( index );
				}
				else if( dma->n_channelcontrol == 0x11000002 &&
					index == 6 )
				{
					verboselog( *this, 1, "dma 6 reverse clear %08x %08x\n",
						dma->n_base, dma->n_blockcontrol );
					if( n_size > 0 )
					{
						n_size--;
						while( n_size > 0 )
						{
							n_nextaddress = ( n_address - 4 ) & 0xffffff;
							m_ram[ n_address / 4 ] = n_nextaddress;
							n_address = n_nextaddress;
							n_size--;
						}
						m_ram[ n_address / 4 ] = 0xffffff;
					}
					dma_start_timer( index, 2150 );
				}
				else
				{
					verboselog( *this, 1, "dma %d unknown mode %08x\n", index, dma->n_channelcontrol );
				}
			}
			else if( dma->n_channelcontrol != 0 )
			{
				verboselog( *this, 1, "psx_dma_w( %04x, %08x, %08x ) channel not enabled\n", offset, dma->n_channelcontrol, mem_mask );
			}
			break;
		default:
			verboselog( *this, 1, "psx_dma_w( %04x, %08x, %08x ) Unknown dma channel register\n", offset, data, mem_mask );
			break;
		}
	}
	else
	{
		switch( offset % 4 )
		{
		case 0x0:
			verboselog( *this, 1, "psx_dma_w( %04x, %08x, %08x ) dpcp\n", offset, data, mem_mask );
			m_dpcp = ( m_dpcp & ~mem_mask ) | data;
			break;
		case 0x1:

			m_dicr = ( m_dicr & ( 0x80000000 | ~mem_mask ) ) |
				( m_dicr & ~data & 0x7f000000 & mem_mask ) |
				( data & 0x00ffffff & mem_mask );

			if( ( m_dicr & 0x80000000 ) != 0 && ( m_dicr & 0x7f000000 ) == 0 )
			{
				verboselog( *this, 2, "dma interrupt cleared\n" );
				m_dicr &= ~0x80000000;
			}

			verboselog( *this, 1, "psx_dma_w( %04x, %08x, %08x ) dicr -> %08x\n", offset, data, mem_mask, m_dicr );
			break;
		default:
			verboselog( *this, 0, "psx_dma_w( %04x, %08x, %08x ) Unknown dma control register\n", offset, data, mem_mask );
			break;
		}
	}
}

READ32_MEMBER( psxdma_device::read )
{
	int index = offset / 4;
	psx_dma_channel *dma = &m_channel[ index ];

	if( index < 7 )
	{
		switch( offset % 4 )
		{
		case 0:
			verboselog( *this, 1, "psx_dma_r dmabase[ %d ] ( %08x )\n", index, dma->n_base );
			return dma->n_base;
		case 1:
			verboselog( *this, 1, "psx_dma_r dmablockcontrol[ %d ] ( %08x )\n", index, dma->n_blockcontrol );
			return dma->n_blockcontrol;
		case 2:
			verboselog( *this, 1, "psx_dma_r dmachannelcontrol[ %d ] ( %08x )\n", index, dma->n_channelcontrol );
			return dma->n_channelcontrol;
		default:
			verboselog( *this, 0, "psx_dma_r( %08x, %08x ) Unknown dma channel register\n", offset, mem_mask );
			break;
		}
	}
	else
	{
		switch( offset % 4 )
		{
		case 0x0:
			verboselog( *this, 1, "psx_dma_r dpcp ( %08x )\n", m_dpcp );
			return m_dpcp;
		case 0x1:
			verboselog( *this, 1, "psx_dma_r dicr ( %08x )\n", m_dicr );
			return m_dicr;
		default:
			verboselog( *this, 0, "psx_dma_r( %08x, %08x ) Unknown dma control register\n", offset, mem_mask );
			break;
		}
	}
	return 0;
}

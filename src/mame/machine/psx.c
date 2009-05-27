/***************************************************************************

  machine/psx.c

  Thanks to Olivier Galibert for IDCT information

***************************************************************************/

#include "driver.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

UINT32 *g_p_n_psxram;
size_t g_n_psxramsize;

#ifdef UNUSED_FUNCTION
INLINE void psxwriteword( UINT32 n_address, UINT16 n_data )
{
	*( (UINT16 *)( (UINT8 *)g_p_n_psxram + WORD_XOR_LE( n_address ) ) ) = n_data;
}
#endif

INLINE UINT16 psxreadword( UINT32 n_address )
{
	return *( (UINT16 *)( (UINT8 *)g_p_n_psxram + WORD_XOR_LE( n_address ) ) );
}

static UINT32 psx_com_delay = 0;

WRITE32_HANDLER( psx_com_delay_w )
{
	COMBINE_DATA( &psx_com_delay );
	verboselog( space->machine, 1, "psx_com_delay_w( %08x %08x )\n", data, mem_mask );
}

READ32_HANDLER( psx_com_delay_r )
{
	verboselog( space->machine, 1, "psx_com_delay_r( %08x )\n", mem_mask );
	return psx_com_delay;
}

/* IRQ */

static UINT32 m_n_irqdata;
static UINT32 m_n_irqmask;

static void psx_irq_update( running_machine *machine )
{
	if( ( m_n_irqdata & m_n_irqmask ) != 0 )
	{
		verboselog( machine, 2, "psx irq assert\n" );
		cputag_set_input_line(machine, "maincpu", PSXCPU_IRQ0, ASSERT_LINE );
	}
	else
	{
		verboselog( machine, 2, "psx irq clear\n" );
		cputag_set_input_line(machine, "maincpu", PSXCPU_IRQ0, CLEAR_LINE );
	}
}

WRITE32_HANDLER( psx_irq_w )
{
	running_machine *machine = space->machine;

	switch( offset )
	{
	case 0x00:
		verboselog( machine, 2, "psx irq data ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, m_n_irqdata, ( m_n_irqdata & ~mem_mask ) | ( m_n_irqdata & m_n_irqmask & data ) );
		m_n_irqdata = ( m_n_irqdata & ~mem_mask ) | ( m_n_irqdata & m_n_irqmask & data );
		psx_irq_update(space->machine);
		break;
	case 0x01:
		verboselog( machine, 2, "psx irq mask ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, m_n_irqmask, ( m_n_irqmask & ~mem_mask ) | data );
		m_n_irqmask = ( m_n_irqmask & ~mem_mask ) | data;
		if( ( m_n_irqmask & ~( 0x1 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80 | 0x100 | 0x200 | 0x400 ) ) != 0 )
		{
			verboselog( machine, 0, "psx_irq_w( %08x, %08x, %08x ) unknown irq\n", offset, data, mem_mask );
		}
		psx_irq_update(space->machine);
		break;
	default:
		verboselog( machine, 0, "psx_irq_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}

READ32_HANDLER( psx_irq_r )
{
	running_machine *machine = space->machine;

	switch( offset )
	{
	case 0x00:
		verboselog( machine, 1, "psx_irq_r irq data %08x\n", m_n_irqdata );
		return m_n_irqdata;
	case 0x01:
		verboselog( machine, 1, "psx_irq_r irq mask %08x\n", m_n_irqmask );
		return m_n_irqmask;
	default:
		verboselog( machine, 0, "psx_irq_r unknown register %d\n", offset );
		break;
	}
	return 0;
}

void psx_irq_set( running_machine *machine, UINT32 data )
{
	verboselog( machine, 2, "psx_irq_set %08x\n", data );
	m_n_irqdata |= data;
	psx_irq_update(machine);
}

/* DMA */

static UINT32 m_p_n_dmabase[ 7 ];
static UINT32 m_p_n_dmablockcontrol[ 7 ];
static UINT32 m_p_n_dmachannelcontrol[ 7 ];
static emu_timer *m_p_timer_dma[ 7 ];
static psx_dma_read_handler m_p_fn_dma_read[ 7 ];
static psx_dma_write_handler m_p_fn_dma_write[ 7 ];
static UINT32 m_p_n_dma_ticks[ 7 ];
static UINT32 m_p_b_dma_running[ 7 ];
static UINT32 m_n_dpcp;
static UINT32 m_n_dicr;

static void dma_start_timer( int n_channel, UINT32 n_ticks )
{
	timer_adjust_oneshot( m_p_timer_dma[ n_channel ], attotime_mul(ATTOTIME_IN_HZ(33868800), n_ticks), n_channel);
	m_p_n_dma_ticks[ n_channel ] = n_ticks;
	m_p_b_dma_running[ n_channel ] = 1;
}

static void dma_stop_timer( int n_channel )
{
	timer_adjust_oneshot( m_p_timer_dma[ n_channel ], attotime_never, 0);
	m_p_b_dma_running[ n_channel ] = 0;
}

static void dma_timer_adjust( int n_channel )
{
	if( m_p_b_dma_running[ n_channel ] )
	{
		dma_start_timer( n_channel, m_p_n_dma_ticks[ n_channel ] );
	}
	else
	{
		dma_stop_timer( n_channel );
	}
}

static void dma_interrupt_update( running_machine *machine )
{
	int n_int;
	int n_mask;

	n_int = ( m_n_dicr >> 24 ) & 0x7f;
	n_mask = ( m_n_dicr >> 16 ) & 0xff;

	if( ( n_mask & 0x80 ) != 0 && ( n_int & n_mask ) != 0 )
	{
		verboselog( machine, 2, "dma_interrupt_update( %02x, %02x ) interrupt triggered\n", n_int, n_mask );
		m_n_dicr |= 0x80000000;
		psx_irq_set( machine, 0x0008 );
	}
	else if( ( m_n_dicr & 0x80000000 ) != 0 )
	{
		verboselog( machine, 2, "dma_interrupt_update( %02x, %02x ) interrupt cleared\n", n_int, n_mask );
		m_n_dicr &= ~0x80000000;
	}
	else if( n_int != 0 )
	{
		verboselog( machine, 2, "dma_interrupt_update( %02x, %02x ) interrupt not enabled\n", n_int, n_mask );
	}

	m_n_dicr &= 0x00ffffff | ( m_n_dicr << 8 );
}

static void dma_finished(running_machine *machine, int n_channel)
{
	if( m_p_n_dmachannelcontrol[ n_channel ] == 0x01000401 && n_channel == 2 )
	{
		UINT32 n_size;
		UINT32 n_total;
		UINT32 n_address = ( m_p_n_dmabase[ n_channel ] & 0xffffff );
		UINT32 n_adrmask = g_n_psxramsize - 1;
		UINT32 n_nextaddress;

		if( n_address != 0xffffff )
		{
			n_total = 0;
			for( ;; )
			{
				if( n_address == 0xffffff )
				{
					m_p_n_dmabase[ n_channel ] = n_address;
					dma_start_timer( n_channel, 19000 );
					return;
				}
				if( n_total > 65535 )
				{
					m_p_n_dmabase[ n_channel ] = n_address;
					//FIXME:
					// 16000 below is based on try and error.
					// Mametesters.org: sfex20103red
					//dma_start_timer( n_channel, 16 );
					dma_start_timer( n_channel, 16000 );
					return;
				}
				n_address &= n_adrmask;
				n_nextaddress = g_p_n_psxram[ n_address / 4 ];
				n_size = n_nextaddress >> 24;
				m_p_fn_dma_write[ n_channel ]( machine, n_address + 4, n_size );
				//FIXME:
				// The following conditions will cause an endless loop.
				// If stopping the transfer is correct I cannot judge
				// The patch is meant as a hint for somebody who knows
				// the hardware.
				// Mametesters.org: psyforce0105u5red, raystorm0111u1red
				if ((n_nextaddress & 0xffffff) != 0xffffff)
					if (n_address == g_p_n_psxram[ (n_nextaddress & 0xffffff) / 4])
						break;
				if (n_address == (n_nextaddress & 0xffffff) )
					break;
				n_address = ( n_nextaddress & 0xffffff );

				n_total += ( n_size + 1 );
			}
		}
	}

	m_p_n_dmachannelcontrol[ n_channel ] &= ~( ( 1L << 0x18 ) | ( 1L << 0x1c ) );

	m_n_dicr |= 1 << ( 24 + n_channel );
	dma_interrupt_update(machine);
	dma_stop_timer( n_channel );
}

static TIMER_CALLBACK( dma_finished_callback )
{
	dma_finished(machine, param);
}

void psx_dma_install_read_handler( int n_channel, psx_dma_read_handler p_fn_dma_read )
{
	m_p_fn_dma_read[ n_channel ] = p_fn_dma_read;
}

void psx_dma_install_write_handler( int n_channel, psx_dma_read_handler p_fn_dma_write )
{
	m_p_fn_dma_write[ n_channel ] = p_fn_dma_write;
}

WRITE32_HANDLER( psx_dma_w )
{
	running_machine *machine = space->machine;
	static int n_channel;
	n_channel = offset / 4;
	if( n_channel < 7 )
	{
		switch( offset % 4 )
		{
		case 0:
			verboselog( machine, 2, "dmabase( %d ) = %08x\n", n_channel, data );
			m_p_n_dmabase[ n_channel ] = data;
			break;
		case 1:
			verboselog( machine, 2, "dmablockcontrol( %d ) = %08x\n", n_channel, data );
			m_p_n_dmablockcontrol[ n_channel ] = data;
			break;
		case 2:
			verboselog( machine, 2, "dmachannelcontrol( %d ) = %08x\n", n_channel, data );
			m_p_n_dmachannelcontrol[ n_channel ] = data;
			if( ( m_p_n_dmachannelcontrol[ n_channel ] & ( 1L << 0x18 ) ) != 0 && ( m_n_dpcp & ( 1 << ( 3 + ( n_channel * 4 ) ) ) ) != 0 )
			{
				INT32 n_size;
				UINT32 n_address;
				UINT32 n_nextaddress;
				UINT32 n_adrmask;

				n_adrmask = g_n_psxramsize - 1;

				n_address = ( m_p_n_dmabase[ n_channel ] & n_adrmask );
				n_size = m_p_n_dmablockcontrol[ n_channel ];
				if( ( m_p_n_dmachannelcontrol[ n_channel ] & 0x200 ) != 0 )
				{
					UINT32 n_ba;
					n_ba = m_p_n_dmablockcontrol[ n_channel ] >> 16;
					if( n_ba == 0 )
					{
						n_ba = 0x10000;
					}
					n_size = ( n_size & 0xffff ) * n_ba;
				}

				if( m_p_n_dmachannelcontrol[ n_channel ] == 0x01000000 &&
					m_p_fn_dma_read[ n_channel ] != NULL )
				{
					verboselog( machine, 1, "dma %d read block %08x %08x\n", n_channel, n_address, n_size );
					m_p_fn_dma_read[ n_channel ]( space->machine, n_address, n_size );
					dma_finished( machine, n_channel );
				}
				else if( m_p_n_dmachannelcontrol[ n_channel ] == 0x01000200 &&
					m_p_fn_dma_read[ n_channel ] != NULL )
				{
					verboselog( machine, 1, "dma %d read block %08x %08x\n", n_channel, n_address, n_size );
					m_p_fn_dma_read[ n_channel ]( space->machine, n_address, n_size );
					if( n_channel == 1 )
					{
						dma_start_timer( n_channel, 26000 );
					}
					else
					{
						dma_finished( machine, n_channel );
					}
				}
				else if( m_p_n_dmachannelcontrol[ n_channel ] == 0x01000201 &&
					m_p_fn_dma_write[ n_channel ] != NULL )
				{
					verboselog( machine, 1, "dma %d write block %08x %08x\n", n_channel, n_address, n_size );
					m_p_fn_dma_write[ n_channel ]( space->machine, n_address, n_size );
					dma_finished( machine, n_channel );
				}
				else if( m_p_n_dmachannelcontrol[ n_channel ] == 0x11050100 &&
					m_p_fn_dma_write[ n_channel ] != NULL )
				{
					/* todo: check this is a write not a read... */
					verboselog( machine, 1, "dma %d write block %08x %08x\n", n_channel, n_address, n_size );
					m_p_fn_dma_write[ n_channel ]( space->machine, n_address, n_size );
					dma_finished( machine, n_channel );
				}
				else if( m_p_n_dmachannelcontrol[ n_channel ] == 0x11150100 &&
					m_p_fn_dma_write[ n_channel ] != NULL )
				{
					/* todo: check this is a write not a read... */
					verboselog( machine, 1, "dma %d write block %08x %08x\n", n_channel, n_address, n_size );
					m_p_fn_dma_write[ n_channel ]( space->machine, n_address, n_size );
					dma_finished( machine, n_channel );
				}
				else if( m_p_n_dmachannelcontrol[ n_channel ] == 0x01000401 &&
					n_channel == 2 &&
					m_p_fn_dma_write[ n_channel ] != NULL )
				{
					verboselog( machine, 1, "dma %d write linked list %08x\n",
						n_channel, m_p_n_dmabase[ n_channel ] );

					dma_finished( machine, n_channel );
				}
				else if( m_p_n_dmachannelcontrol[ n_channel ] == 0x11000002 &&
					n_channel == 6 )
				{
					verboselog( machine, 1, "dma 6 reverse clear %08x %08x\n",
						m_p_n_dmabase[ n_channel ], m_p_n_dmablockcontrol[ n_channel ] );
					if( n_size > 0 )
					{
						n_size--;
						while( n_size > 0 )
						{
							n_nextaddress = ( n_address - 4 ) & 0xffffff;
							g_p_n_psxram[ n_address / 4 ] = n_nextaddress;
							n_address = n_nextaddress;
							n_size--;
						}
						g_p_n_psxram[ n_address / 4 ] = 0xffffff;
					}
					dma_start_timer( n_channel, 2150 );
				}
				else
				{
					verboselog( machine, 0, "dma %d unknown mode %08x\n", n_channel, m_p_n_dmachannelcontrol[ n_channel ] );
				}
			}
			else if( m_p_n_dmachannelcontrol[ n_channel ] != 0 )
			{
				verboselog( machine, 1, "psx_dma_w( %04x, %08x, %08x ) channel not enabled\n", offset, m_p_n_dmachannelcontrol[ n_channel ], mem_mask );
			}
			break;
		default:
			verboselog( machine, 1, "psx_dma_w( %04x, %08x, %08x ) Unknown dma channel register\n", offset, data, mem_mask );
			break;
		}
	}
	else
	{
		switch( offset % 4 )
		{
		case 0x0:
			verboselog( machine, 1, "psx_dma_w( %04x, %08x, %08x ) dpcp\n", offset, data, mem_mask );
			m_n_dpcp = ( m_n_dpcp & ~mem_mask ) | data;
			break;
		case 0x1:
			m_n_dicr = ( m_n_dicr & ~mem_mask ) |
				( mem_mask & 0x80000000 & m_n_dicr ) |
				( ~data & mem_mask & 0x7f000000 & m_n_dicr ) |
				( data & mem_mask & 0x00ffffff );
/* todo: find out whether to do this instead of dma_interrupt_update()

            if( ( m_n_dicr & 0x7f000000 ) != 0 )
            {
                m_n_dicr &= ~0x80000000;
            }
*/
			verboselog( machine, 1, "psx_dma_w( %04x, %08x, %08x ) dicr -> %08x\n", offset, data, mem_mask, m_n_dicr );
			dma_interrupt_update(space->machine);
			break;
		default:
			verboselog( machine, 0, "psx_dma_w( %04x, %08x, %08x ) Unknown dma control register\n", offset, data, mem_mask );
			break;
		}
	}
}

READ32_HANDLER( psx_dma_r )
{
	running_machine *machine = space->machine;
	static int n_channel;
	n_channel = offset / 4;
	if( n_channel < 7 )
	{
		switch( offset % 4 )
		{
		case 0:
			verboselog( machine, 1, "psx_dma_r dmabase[ %d ] ( %08x )\n", n_channel, m_p_n_dmabase[ n_channel ] );
			return m_p_n_dmabase[ n_channel ];
		case 1:
			verboselog( machine, 1, "psx_dma_r dmablockcontrol[ %d ] ( %08x )\n", n_channel, m_p_n_dmablockcontrol[ n_channel ] );
			return m_p_n_dmablockcontrol[ n_channel ];
		case 2:
			verboselog( machine, 1, "psx_dma_r dmachannelcontrol[ %d ] ( %08x )\n", n_channel, m_p_n_dmachannelcontrol[ n_channel ] );
			return m_p_n_dmachannelcontrol[ n_channel ];
		default:
			verboselog( machine, 0, "psx_dma_r( %08x, %08x ) Unknown dma channel register\n", offset, mem_mask );
			break;
		}
	}
	else
	{
		switch( offset % 4 )
		{
		case 0x0:
			verboselog( machine, 1, "psx_dma_r dpcp ( %08x )\n", m_n_dpcp );
			return m_n_dpcp;
		case 0x1:
			verboselog( machine, 1, "psx_dma_r dicr ( %08x )\n", m_n_dicr );
			return m_n_dicr;
		default:
			verboselog( machine, 0, "psx_dma_r( %08x, %08x ) Unknown dma control register\n", offset, mem_mask );
			break;
		}
	}
	return 0;
}

/* Root Counters */

static emu_timer *m_p_timer_root[ 3 ];
static UINT16 m_p_n_root_count[ 3 ];
static UINT16 m_p_n_root_mode[ 3 ];
static UINT16 m_p_n_root_target[ 3 ];
static UINT64 m_p_n_root_start[ 3 ];

#define RC_STOP ( 0x01 )
#define RC_RESET ( 0x04 ) /* guess */
#define RC_COUNTTARGET ( 0x08 )
#define RC_IRQTARGET ( 0x10 )
#define RC_IRQOVERFLOW ( 0x20 )
#define RC_REPEAT ( 0x40 )
#define RC_CLC ( 0x100 )
#define RC_DIV ( 0x200 )

static UINT64 psxcpu_gettotalcycles( running_machine *machine )
{
	/* TODO: should return the start of the current tick. */
	return cpu_get_total_cycles(cputag_get_cpu(machine, "maincpu")) * 2;
}

static int root_divider( int n_counter )
{
	if( n_counter == 0 && ( m_p_n_root_mode[ n_counter ] & RC_CLC ) != 0 )
	{
		/* TODO: pixel clock, probably based on resolution */
		return 5;
	}
	else if( n_counter == 1 && ( m_p_n_root_mode[ n_counter ] & RC_CLC ) != 0 )
	{
		return 2150;
	}
	else if( n_counter == 2 && ( m_p_n_root_mode[ n_counter ] & RC_DIV ) != 0 )
	{
		return 8;
	}
	return 1;
}

static UINT16 root_current( running_machine *machine, int n_counter )
{
	if( ( m_p_n_root_mode[ n_counter ] & RC_STOP ) != 0 )
	{
		return m_p_n_root_count[ n_counter ];
	}
	else
	{
		UINT64 n_current;
		n_current = psxcpu_gettotalcycles(machine) - m_p_n_root_start[ n_counter ];
		n_current /= root_divider( n_counter );
		n_current += m_p_n_root_count[ n_counter ];
		if( n_current > 0xffff )
		{
			/* TODO: use timer for wrap on 0x10000. */
			m_p_n_root_count[ n_counter ] = n_current;
			m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(machine);
		}
		return n_current;
	}
}

static int root_target( int n_counter )
{
	if( ( m_p_n_root_mode[ n_counter ] & RC_COUNTTARGET ) != 0 ||
		( m_p_n_root_mode[ n_counter ] & RC_IRQTARGET ) != 0 )
	{
		return m_p_n_root_target[ n_counter ];
	}
	return 0x10000;
}

static void root_timer_adjust( running_machine *machine, int n_counter )
{
	if( ( m_p_n_root_mode[ n_counter ] & RC_STOP ) != 0 )
	{
		timer_adjust_oneshot( m_p_timer_root[ n_counter ], attotime_never, n_counter);
	}
	else
	{
		int n_duration;

		n_duration = root_target( n_counter ) - root_current( machine, n_counter );
		if( n_duration < 1 )
		{
			n_duration += 0x10000;
		}

		n_duration *= root_divider( n_counter );

		timer_adjust_oneshot( m_p_timer_root[ n_counter ], attotime_mul(ATTOTIME_IN_HZ(33868800), n_duration), n_counter);
	}
}

static TIMER_CALLBACK( root_finished )
{
	int n_counter = param;

	verboselog( machine, 2, "root_finished( %d ) %04x\n", n_counter, root_current( machine, n_counter ) );
//  if( ( m_p_n_root_mode[ n_counter ] & RC_COUNTTARGET ) != 0 )
	{
		/* TODO: wrap should be handled differently as RC_COUNTTARGET & RC_IRQTARGET don't have to be the same. */
		m_p_n_root_count[ n_counter ] = 0;
		m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(machine);
	}
	if( ( m_p_n_root_mode[ n_counter ] & RC_REPEAT ) != 0 )
	{
		root_timer_adjust( machine, n_counter );
	}
	if( ( m_p_n_root_mode[ n_counter ] & RC_IRQOVERFLOW ) != 0 ||
		( m_p_n_root_mode[ n_counter ] & RC_IRQTARGET ) != 0 )
	{
		psx_irq_set( machine, 0x10 << n_counter );
	}
}

WRITE32_HANDLER( psx_counter_w )
{
	int n_counter;

	verboselog( space->machine, 1, "psx_counter_w ( %08x, %08x, %08x )\n", offset, data, mem_mask );

	n_counter = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		m_p_n_root_count[ n_counter ] = data;
		m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(space->machine);
		break;
	case 1:
		m_p_n_root_count[ n_counter ] = root_current( space->machine, n_counter );
		m_p_n_root_start[ n_counter ] = psxcpu_gettotalcycles(space->machine);
		m_p_n_root_mode[ n_counter ] = data;

		if( ( m_p_n_root_mode[ n_counter ] & RC_RESET ) != 0 )
		{
			m_p_n_root_count[ n_counter ] = 0;
		}
//      if( ( data & 0xfca6 ) != 0 ||
//          ( ( data & 0x0100 ) != 0 && n_counter != 0 && n_counter != 1 ) ||
//          ( ( data & 0x0200 ) != 0 && n_counter != 2 ) )
//      {
//          mame_printf_debug( "mode %d 0x%04x\n", n_counter, data & 0xfca6 );
//      }
		break;
	case 2:
		m_p_n_root_target[ n_counter ] = data;
		break;
	default:
		verboselog( space->machine, 0, "psx_counter_w( %08x, %08x, %08x ) unknown register\n", offset, mem_mask, data );
		return;
	}

	root_timer_adjust( space->machine, n_counter );
}

READ32_HANDLER( psx_counter_r )
{
	int n_counter;
	UINT32 data;

	n_counter = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		data = root_current( space->machine, n_counter );
		break;
	case 1:
		data = m_p_n_root_mode[ n_counter ];
		break;
	case 2:
		data = m_p_n_root_target[ n_counter ];
		break;
	default:
		verboselog( space->machine, 0, "psx_counter_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		return 0;
	}
	verboselog( space->machine, 1, "psx_counter_r ( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

/* SIO */

#define SIO_BUF_SIZE ( 8 )

static UINT32 m_p_n_sio_status[ 2 ];
static UINT32 m_p_n_sio_mode[ 2 ];
static UINT32 m_p_n_sio_control[ 2 ];
static UINT32 m_p_n_sio_baud[ 2 ];
static UINT32 m_p_n_sio_tx[ 2 ];
static UINT32 m_p_n_sio_rx[ 2 ];
static UINT32 m_p_n_sio_tx_prev[ 2 ];
static UINT32 m_p_n_sio_rx_prev[ 2 ];
static UINT32 m_p_n_sio_tx_data[ 2 ];
static UINT32 m_p_n_sio_rx_data[ 2 ];
static UINT32 m_p_n_sio_tx_shift[ 2 ];
static UINT32 m_p_n_sio_rx_shift[ 2 ];
static UINT32 m_p_n_sio_tx_bits[ 2 ];
static UINT32 m_p_n_sio_rx_bits[ 2 ];

static emu_timer *m_p_timer_sio[ 2 ];
static psx_sio_handler m_p_f_sio_handler[ 2 ];

#define SIO_STATUS_TX_RDY ( 1 << 0 )
#define SIO_STATUS_RX_RDY ( 1 << 1 )
#define SIO_STATUS_TX_EMPTY ( 1 << 2 )
#define SIO_STATUS_OVERRUN ( 1 << 4 )
#define SIO_STATUS_DSR ( 1 << 7 )
#define SIO_STATUS_IRQ ( 1 << 9 )

#define SIO_CONTROL_TX_ENA ( 1 << 0 )
#define SIO_CONTROL_IACK ( 1 << 4 )
#define SIO_CONTROL_RESET ( 1 << 6 )
#define SIO_CONTROL_TX_IENA ( 1 << 10 )
#define SIO_CONTROL_RX_IENA ( 1 << 11 )
#define SIO_CONTROL_DSR_IENA ( 1 << 12 )
#define SIO_CONTROL_DTR ( 1 << 13 )

static void sio_interrupt( running_machine *machine, int n_port )
{
	verboselog( machine, 1, "sio_interrupt( %d )\n", n_port );
	m_p_n_sio_status[ n_port ] |= SIO_STATUS_IRQ;
	if( n_port == 0 )
	{
		psx_irq_set( machine, 0x80 );
	}
	else
	{
		psx_irq_set( machine, 0x100 );
	}
}

static void sio_timer_adjust( running_machine *machine, int n_port )
{
	attotime n_time;
	if( ( m_p_n_sio_status[ n_port ] & SIO_STATUS_TX_EMPTY ) == 0 || m_p_n_sio_tx_bits[ n_port ] != 0 )
	{
		int n_prescaler;

		switch( m_p_n_sio_mode[ n_port ] & 3 )
		{
		case 1:
			n_prescaler = 1;
			break;
		case 2:
			n_prescaler = 16;
			break;
		case 3:
			n_prescaler = 64;
			break;
		default:
			n_prescaler = 0;
			break;
		}

		if( m_p_n_sio_baud[ n_port ] != 0 && n_prescaler != 0 )
		{
			n_time = attotime_mul(ATTOTIME_IN_HZ(33868800), n_prescaler * m_p_n_sio_baud[n_port]);
			verboselog( machine, 2, "sio_timer_adjust( %d ) = %s ( %d x %d )\n", n_port, attotime_string(n_time, 9), n_prescaler, m_p_n_sio_baud[ n_port ] );
		}
		else
		{
			n_time = attotime_never;
			verboselog( machine, 0, "sio_timer_adjust( %d ) invalid baud rate ( %d x %d )\n", n_port, n_prescaler, m_p_n_sio_baud[ n_port ] );
		}
	}
	else
	{
		n_time = attotime_never;
		verboselog( machine, 2, "sio_timer_adjust( %d ) finished\n", n_port );
	}
	timer_adjust_oneshot( m_p_timer_sio[ n_port ], n_time, n_port);
}

static TIMER_CALLBACK( sio_clock )
{
	int n_port = param;
	verboselog( machine, 2, "sio tick\n" );

	if( m_p_n_sio_tx_bits[ n_port ] == 0 &&
		( m_p_n_sio_control[ n_port ] & SIO_CONTROL_TX_ENA ) != 0 &&
		( m_p_n_sio_status[ n_port ] & SIO_STATUS_TX_EMPTY ) == 0 )
	{
		m_p_n_sio_tx_bits[ n_port ] = 8;
		m_p_n_sio_tx_shift[ n_port ] = m_p_n_sio_tx_data[ n_port ];
		if( n_port == 0 )
		{
			m_p_n_sio_rx_bits[ n_port ] = 8;
			m_p_n_sio_rx_shift[ n_port ] = 0;
		}
		m_p_n_sio_status[ n_port ] |= SIO_STATUS_TX_EMPTY;
		m_p_n_sio_status[ n_port ] |= SIO_STATUS_TX_RDY;
	}

	if( m_p_n_sio_tx_bits[ n_port ] != 0 )
	{
		m_p_n_sio_tx[ n_port ] = ( m_p_n_sio_tx[ n_port ] & ~PSX_SIO_OUT_DATA ) | ( ( m_p_n_sio_tx_shift[ n_port ] & 1 ) * PSX_SIO_OUT_DATA );
		m_p_n_sio_tx_shift[ n_port ] >>= 1;
		m_p_n_sio_tx_bits[ n_port ]--;

		if( m_p_f_sio_handler[ n_port ] != NULL )
		{
			if( n_port == 0 )
			{
				m_p_n_sio_tx[ n_port ] &= ~PSX_SIO_OUT_CLOCK;
				m_p_f_sio_handler[ n_port ]( machine, m_p_n_sio_tx[ n_port ] );
				m_p_n_sio_tx[ n_port ] |= PSX_SIO_OUT_CLOCK;
			}
			m_p_f_sio_handler[ n_port ]( machine, m_p_n_sio_tx[ n_port ] );
		}

		if( m_p_n_sio_tx_bits[ n_port ] == 0 &&
			( m_p_n_sio_control[ n_port ] & SIO_CONTROL_TX_IENA ) != 0 )
		{
			sio_interrupt( machine, n_port );
		}
	}

	if( m_p_n_sio_rx_bits[ n_port ] != 0 )
	{
		m_p_n_sio_rx_shift[ n_port ] = ( m_p_n_sio_rx_shift[ n_port ] >> 1 ) | ( ( ( m_p_n_sio_rx[ n_port ] & PSX_SIO_IN_DATA ) / PSX_SIO_IN_DATA ) << 7 );
		m_p_n_sio_rx_bits[ n_port ]--;

		if( m_p_n_sio_rx_bits[ n_port ] == 0 )
		{
			if( ( m_p_n_sio_status[ n_port ] & SIO_STATUS_RX_RDY ) != 0 )
			{
				m_p_n_sio_status[ n_port ] |= SIO_STATUS_OVERRUN;
			}
			else
			{
				m_p_n_sio_rx_data[ n_port ] = m_p_n_sio_rx_shift[ n_port ];
				m_p_n_sio_status[ n_port ] |= SIO_STATUS_RX_RDY;
			}
			if( ( m_p_n_sio_control[ n_port ] & SIO_CONTROL_RX_IENA ) != 0 )
			{
				sio_interrupt( machine, n_port );
			}
		}
	}

	sio_timer_adjust( machine, n_port );
}

void psx_sio_input( running_machine *machine, int n_port, int n_mask, int n_data )
{
	verboselog( machine, 1, "psx_sio_input( %d, %02x, %02x )\n", n_port, n_mask, n_data );
	m_p_n_sio_rx[ n_port ] = ( m_p_n_sio_rx[ n_port ] & ~n_mask ) | ( n_data & n_mask );

	if( ( m_p_n_sio_rx[ n_port ] & PSX_SIO_IN_DSR ) != 0 )
	{
		m_p_n_sio_status[ n_port ] |= SIO_STATUS_DSR;
		if( ( m_p_n_sio_rx_prev[ n_port ] & PSX_SIO_IN_DSR ) == 0 &&
			( m_p_n_sio_control[ n_port ] & SIO_CONTROL_DSR_IENA ) != 0 )
		{
			sio_interrupt( machine, n_port );
		}
	}
	else
	{
		m_p_n_sio_status[ n_port ] &= ~SIO_STATUS_DSR;
	}
	m_p_n_sio_rx_prev[ n_port ] = m_p_n_sio_rx[ n_port ];
}

WRITE32_HANDLER( psx_sio_w )
{
	running_machine *machine = space->machine;
	int n_port;

	n_port = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		verboselog( machine, 1, "psx_sio_w %d data %02x (%08x)\n", n_port, data, mem_mask );
		m_p_n_sio_tx_data[ n_port ] = data;
		m_p_n_sio_status[ n_port ] &= ~( SIO_STATUS_TX_RDY );
		m_p_n_sio_status[ n_port ] &= ~( SIO_STATUS_TX_EMPTY );
		sio_timer_adjust( machine, n_port );
		break;
	case 1:
		verboselog( machine, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	case 2:
		if( ACCESSING_BITS_0_15 )
		{
			m_p_n_sio_mode[ n_port ] = data & 0xffff;
			verboselog( machine, 1, "psx_sio_w %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine, 1, "psx_sio_w %d control %04x\n", n_port, data >> 16 );
			m_p_n_sio_control[ n_port ] = data >> 16;

			if( ( m_p_n_sio_control[ n_port ] & SIO_CONTROL_RESET ) != 0 )
			{
				verboselog( machine, 1, "psx_sio_w reset\n" );
				m_p_n_sio_status[ n_port ] |= SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
				m_p_n_sio_status[ n_port ] &= ~( SIO_STATUS_RX_RDY | SIO_STATUS_OVERRUN | SIO_STATUS_IRQ );
			}
			if( ( m_p_n_sio_control[ n_port ] & SIO_CONTROL_IACK ) != 0 )
			{
				verboselog( machine, 1, "psx_sio_w iack\n" );
				m_p_n_sio_status[ n_port ] &= ~( SIO_STATUS_IRQ );
				m_p_n_sio_control[ n_port ] &= ~( SIO_CONTROL_IACK );
			}
			if( ( m_p_n_sio_control[ n_port ] & SIO_CONTROL_DTR ) != 0 )
			{
				m_p_n_sio_tx[ n_port ] |= PSX_SIO_OUT_DTR;
			}
			else
			{
				m_p_n_sio_tx[ n_port ] &= ~PSX_SIO_OUT_DTR;
			}

			if( ( ( m_p_n_sio_tx[ n_port ] ^ m_p_n_sio_tx_prev[ n_port ] ) & PSX_SIO_OUT_DTR ) != 0 )
			{
				if( m_p_f_sio_handler[ n_port ] != NULL )
				{
					m_p_f_sio_handler[ n_port ]( space->machine, m_p_n_sio_tx[ n_port ] );
				}
			}
			m_p_n_sio_tx_prev[ n_port ] = m_p_n_sio_tx[ n_port ];

		}
		break;
	case 3:
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		}
		if( ACCESSING_BITS_16_31 )
		{
			m_p_n_sio_baud[ n_port ] = data >> 16;
			verboselog( machine, 1, "psx_sio_w %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		verboselog( machine, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	}
}

READ32_HANDLER( psx_sio_r )
{
	running_machine *machine = space->machine;
	UINT32 data;
	int n_port;

	n_port = offset / 4;

	switch( offset % 4 )
	{
	case 0:
		data = m_p_n_sio_rx_data[ n_port ];
		m_p_n_sio_status[ n_port ] &= ~( SIO_STATUS_RX_RDY );
		m_p_n_sio_rx_data[ n_port ] = 0xff;
		verboselog( machine, 1, "psx_sio_r %d data %02x (%08x)\n", n_port, data, mem_mask );
		break;
	case 1:
		data = m_p_n_sio_status[ n_port ];
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine, 1, "psx_sio_r %d status %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		break;
	case 2:
		data = ( m_p_n_sio_control[ n_port ] << 16 ) | m_p_n_sio_mode[ n_port ];
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine, 1, "psx_sio_r %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine, 1, "psx_sio_r %d control %04x\n", n_port, data >> 16 );
		}
		break;
	case 3:
		data = m_p_n_sio_baud[ n_port ] << 16;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( machine, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( machine, 1, "psx_sio_r %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		data = 0;
		verboselog( machine, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}
	return data;
}

void psx_sio_install_handler( int n_port, psx_sio_handler p_f_sio_handler )
{
	m_p_f_sio_handler[ n_port ] = p_f_sio_handler;
}

/* MDEC */

static int mdec_decoded = 0;
static int mdec_offset = 0;
static UINT16 mdec_output[ 24 * 16 ];

#define	DCTSIZE ( 8 )
#define	DCTSIZE2 ( DCTSIZE * DCTSIZE )

static INT32 m_p_n_mdec_quantize_y[ DCTSIZE2 ];
static INT32 m_p_n_mdec_quantize_uv[ DCTSIZE2 ];
static INT32 m_p_n_mdec_cos[ DCTSIZE2 ];
static INT32 m_p_n_mdec_cos_precalc[ DCTSIZE2 * DCTSIZE2 ];

static UINT32 m_n_mdec0_command;
static UINT32 m_n_mdec0_address;
static UINT32 m_n_mdec0_size;
static UINT32 m_n_mdec1_command;
static UINT32 m_n_mdec1_status;

static UINT16 m_p_n_mdec_clamp8[ 256 * 3 ];
static UINT16 m_p_n_mdec_r5[ 256 * 3 ];
static UINT16 m_p_n_mdec_g5[ 256 * 3 ];
static UINT16 m_p_n_mdec_b5[ 256 * 3 ];

static const UINT32 m_p_n_mdec_zigzag[ DCTSIZE2 ] =
{
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static INT32 m_p_n_mdec_unpacked[ DCTSIZE2 * 6 * 2 ];

#define MDEC_COS_PRECALC_BITS ( 21 )

static void mdec_cos_precalc( void )
{
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_u;
	UINT32 n_v;
	INT32 *p_n_precalc;

	p_n_precalc = m_p_n_mdec_cos_precalc;

	for( n_y = 0; n_y < 8; n_y++ )
	{
		for( n_x = 0; n_x < 8; n_x++ )
		{
			for( n_v = 0; n_v < 8; n_v++ )
			{
				for( n_u = 0; n_u < 8; n_u++ )
				{
					*( p_n_precalc++ ) =
						( ( m_p_n_mdec_cos[ ( n_u * 8 ) + n_x ] *
						m_p_n_mdec_cos[ ( n_v * 8 ) + n_y ] ) >> ( 30 - MDEC_COS_PRECALC_BITS ) );
				}
			}
		}
	}
}

static void mdec_idct( INT32 *p_n_src, INT32 *p_n_dst )
{
	UINT32 n_yx;
	UINT32 n_vu;
	INT32 p_n_z[ 8 ];
	INT32 *p_n_data;
	INT32 *p_n_precalc;

	p_n_precalc = m_p_n_mdec_cos_precalc;

	for( n_yx = 0; n_yx < DCTSIZE2; n_yx++ )
	{
		p_n_data = p_n_src;

		memset( p_n_z, 0, sizeof( p_n_z ) );

		for( n_vu = 0; n_vu < DCTSIZE2 / 8; n_vu++ )
		{
			p_n_z[ 0 ] += p_n_data[ 0 ] * p_n_precalc[ 0 ];
			p_n_z[ 1 ] += p_n_data[ 1 ] * p_n_precalc[ 1 ];
			p_n_z[ 2 ] += p_n_data[ 2 ] * p_n_precalc[ 2 ];
			p_n_z[ 3 ] += p_n_data[ 3 ] * p_n_precalc[ 3 ];
			p_n_z[ 4 ] += p_n_data[ 4 ] * p_n_precalc[ 4 ];
			p_n_z[ 5 ] += p_n_data[ 5 ] * p_n_precalc[ 5 ];
			p_n_z[ 6 ] += p_n_data[ 6 ] * p_n_precalc[ 6 ];
			p_n_z[ 7 ] += p_n_data[ 7 ] * p_n_precalc[ 7 ];
			p_n_data += 8;
			p_n_precalc += 8;
		}

		*( p_n_dst++ ) = ( p_n_z[ 0 ] + p_n_z[ 1 ] + p_n_z[ 2 ] + p_n_z[ 3 ] +
			p_n_z[ 4 ] + p_n_z[ 5 ] + p_n_z[ 6 ] + p_n_z[ 7 ] ) >> ( MDEC_COS_PRECALC_BITS + 2 );
	}
}

INLINE UINT16 mdec_unpack_run( UINT16 n_packed )
{
	return n_packed >> 10;
}

INLINE INT32 mdec_unpack_val( UINT16 n_packed )
{
	return ( ( (INT32)n_packed ) << 22 ) >> 22;
}

static UINT32 mdec_unpack( UINT32 n_address )
{
	UINT8 n_z;
	INT32 n_qscale;
	UINT16 n_packed;
	UINT32 n_block;
	INT32 *p_n_block;
	INT32 p_n_unpacked[ 64 ];
	INT32 *p_n_q;

	p_n_q = m_p_n_mdec_quantize_uv;
	p_n_block = m_p_n_mdec_unpacked;

	for( n_block = 0; n_block < 6; n_block++ )
	{
		memset( p_n_unpacked, 0, sizeof( p_n_unpacked ) );

		if( n_block == 2 )
		{
			p_n_q = m_p_n_mdec_quantize_y;
		}
		n_packed = psxreadword( n_address );
		n_address += 2;
		if( n_packed == 0xfe00 )
		{
			break;
		}

		n_qscale = mdec_unpack_run( n_packed );
		p_n_unpacked[ 0 ] = mdec_unpack_val( n_packed ) * p_n_q[ 0 ];

		n_z = 0;
		for( ;; )
		{
			n_packed = psxreadword( n_address );
			n_address += 2;

			if( n_packed == 0xfe00 )
			{
				break;
			}
			n_z += mdec_unpack_run( n_packed ) + 1;
			if( n_z > 63 )
			{
				break;
			}
			p_n_unpacked[ m_p_n_mdec_zigzag[ n_z ] ] = ( mdec_unpack_val( n_packed ) * p_n_q[ n_z ] * n_qscale ) / 8;
		}
		mdec_idct( p_n_unpacked, p_n_block );
		p_n_block += DCTSIZE2;
	}
	return n_address;
}

INLINE INT32 mdec_cr_to_r( INT32 n_cr )
{
	return ( 1435 * n_cr ) >> 10;
}

INLINE INT32 mdec_cr_to_g( INT32 n_cr )
{
	return ( -731 * n_cr ) >> 10;
}

INLINE INT32 mdec_cb_to_g( INT32 n_cb )
{
	return ( -351 * n_cb ) >> 10;
}

INLINE INT32 mdec_cb_to_b( INT32 n_cb )
{
	return ( 1814 * n_cb ) >> 10;
}

INLINE UINT16 mdec_clamp_r5( INT32 n_r )
{
	return m_p_n_mdec_r5[ n_r + 128 + 256 ];
}

INLINE UINT16 mdec_clamp_g5( INT32 n_g )
{
	return m_p_n_mdec_g5[ n_g + 128 + 256 ];
}

INLINE UINT16 mdec_clamp_b5( INT32 n_b )
{
	return m_p_n_mdec_b5[ n_b + 128 + 256 ];
}

INLINE void mdec_makergb15( UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT16 n_stp )
{
	mdec_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_n_y[ 0 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 0 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 0 ] + n_b );

	mdec_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = n_stp |
		mdec_clamp_r5( p_n_y[ 1 ] + n_r ) |
		mdec_clamp_g5( p_n_y[ 1 ] + n_g ) |
		mdec_clamp_b5( p_n_y[ 1 ] + n_b );
}

static void mdec_yuv2_to_rgb15( void )
{
	INT32 n_r;
	INT32 n_g;
	INT32 n_b;
	INT32 n_cb;
	INT32 n_cr;
	INT32 *p_n_cb;
	INT32 *p_n_cr;
	INT32 *p_n_y;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_z;
	UINT16 n_stp;
	int n_address = 0;

	if( ( m_n_mdec0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x8000;
	}
	else
	{
		n_stp = 0x0000;
	}

	p_n_cr = &m_p_n_mdec_unpacked[ 0 ];
	p_n_cb = &m_p_n_mdec_unpacked[ DCTSIZE2 ];
	p_n_y = &m_p_n_mdec_unpacked[ DCTSIZE2 * 2 ];

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb15( ( n_address + 32 ), n_r, n_g, n_b, p_n_y + 8, n_stp );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb15( ( n_address + 16 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb15( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 4;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 48;
		}
		p_n_y += DCTSIZE2;
	}
	mdec_decoded = ( 16 * 16 ) / 2;
}

INLINE UINT16 mdec_clamp8( INT32 n_r )
{
	return m_p_n_mdec_clamp8[ n_r + 128 + 256 ];
}

INLINE void mdec_makergb24( UINT32 n_address, INT32 n_r, INT32 n_g, INT32 n_b, INT32 *p_n_y, UINT32 n_stp )
{
	mdec_output[ WORD_XOR_LE( n_address + 0 ) / 2 ] = ( mdec_clamp8( p_n_y[ 0 ] + n_g ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_r );
	mdec_output[ WORD_XOR_LE( n_address + 2 ) / 2 ] = ( mdec_clamp8( p_n_y[ 1 ] + n_r ) << 8 ) | mdec_clamp8( p_n_y[ 0 ] + n_b );
	mdec_output[ WORD_XOR_LE( n_address + 4 ) / 2 ] = ( mdec_clamp8( p_n_y[ 1 ] + n_b ) << 8 ) | mdec_clamp8( p_n_y[ 1 ] + n_g );
}

static void mdec_yuv2_to_rgb24( void )
{
	INT32 n_r;
	INT32 n_g;
	INT32 n_b;
	INT32 n_cb;
	INT32 n_cr;
	INT32 *p_n_cb;
	INT32 *p_n_cr;
	INT32 *p_n_y;
	UINT32 n_x;
	UINT32 n_y;
	UINT32 n_z;
	UINT32 n_stp;
	int n_address = 0;

	if( ( m_n_mdec0_command & ( 1L << 25 ) ) != 0 )
	{
		n_stp = 0x80008000;
	}
	else
	{
		n_stp = 0x00000000;
	}

	p_n_cr = &m_p_n_mdec_unpacked[ 0 ];
	p_n_cb = &m_p_n_mdec_unpacked[ DCTSIZE2 ];
	p_n_y = &m_p_n_mdec_unpacked[ DCTSIZE2 * 2 ];

	for( n_z = 0; n_z < 2; n_z++ )
	{
		for( n_y = 0; n_y < 4; n_y++ )
		{
			for( n_x = 0; n_x < 4; n_x++ )
			{
				n_cr = *( p_n_cr );
				n_cb = *( p_n_cb );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( ( n_address +  0 ), n_r, n_g, n_b, p_n_y, n_stp );
				mdec_makergb24( ( n_address + 48 ), n_r, n_g, n_b, p_n_y + 8, n_stp );

				n_cr = *( p_n_cr + 4 );
				n_cb = *( p_n_cb + 4 );
				n_r = mdec_cr_to_r( n_cr );
				n_g = mdec_cr_to_g( n_cr ) + mdec_cb_to_g( n_cb );
				n_b = mdec_cb_to_b( n_cb );

				mdec_makergb24( ( n_address + 24 ), n_r, n_g, n_b, p_n_y + DCTSIZE2, n_stp );
				mdec_makergb24( ( n_address + 72 ), n_r, n_g, n_b, p_n_y + DCTSIZE2 + 8, n_stp );

				p_n_cr++;
				p_n_cb++;
				p_n_y += 2;
				n_address += 6;
			}
			p_n_cr += 4;
			p_n_cb += 4;
			p_n_y += 8;
			n_address += 72;
		}
		p_n_y += DCTSIZE2;
	}
	mdec_decoded = ( 24 * 16 ) / 2;
}

static void mdec0_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	int n_index;

	verboselog( machine, 2, "mdec0_write( %08x, %08x )\n", n_address, n_size );
	switch( m_n_mdec0_command >> 28 )
	{
	case 0x3:
		verboselog( machine, 1, "mdec decode %08x %08x %08x\n", m_n_mdec0_command, n_address, n_size );
		m_n_mdec0_address = n_address;
		m_n_mdec0_size = n_size * 4;
		m_n_mdec1_status |= ( 1L << 29 );
		break;
	case 0x4:
		verboselog( machine, 1, "mdec quantize table %08x %08x %08x\n", m_n_mdec0_command, n_address, n_size );
		n_index = 0;
		while( n_size > 0 )
		{
			if( n_index < DCTSIZE2 )
			{
				m_p_n_mdec_quantize_y[ n_index + 0 ] = ( g_p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
				m_p_n_mdec_quantize_y[ n_index + 1 ] = ( g_p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
				m_p_n_mdec_quantize_y[ n_index + 2 ] = ( g_p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
				m_p_n_mdec_quantize_y[ n_index + 3 ] = ( g_p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			}
			else if( n_index < DCTSIZE2 * 2 )
			{
				m_p_n_mdec_quantize_uv[ n_index + 0 - DCTSIZE2 ] = ( g_p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
				m_p_n_mdec_quantize_uv[ n_index + 1 - DCTSIZE2 ] = ( g_p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
				m_p_n_mdec_quantize_uv[ n_index + 2 - DCTSIZE2 ] = ( g_p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
				m_p_n_mdec_quantize_uv[ n_index + 3 - DCTSIZE2 ] = ( g_p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			}
			n_index += 4;
			n_address += 4;
			n_size--;
		}
		break;
	case 0x6:
		verboselog( machine, 1, "mdec cosine table %08x %08x %08x\n", m_n_mdec0_command, n_address, n_size );
		n_index = 0;
		while( n_size > 0 )
		{
			m_p_n_mdec_cos[ n_index + 0 ] = (INT16)( ( g_p_n_psxram[ n_address / 4 ] >> 0 ) & 0xffff );
			m_p_n_mdec_cos[ n_index + 1 ] = (INT16)( ( g_p_n_psxram[ n_address / 4 ] >> 16 ) & 0xffff );
			n_index += 2;
			n_address += 4;
			n_size--;
		}
		mdec_cos_precalc();
		break;
	default:
		verboselog( machine, 0, "mdec unknown command %08x %08x %08x\n", m_n_mdec0_command, n_address, n_size );
		break;
	}
}

static void mdec1_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	UINT32 n_this;
	UINT32 n_nextaddress;

	verboselog( machine, 2, "mdec1_read( %08x, %08x )\n", n_address, n_size );
	if( ( m_n_mdec0_command & ( 1L << 29 ) ) != 0 && m_n_mdec0_size != 0 )
	{
		while( n_size > 0 )
		{
			if( mdec_decoded == 0 )
			{
				if( (int)m_n_mdec0_size <= 0 )
				{
					mame_printf_debug( "ran out of data %08x\n", n_size );
					m_n_mdec0_size = 0;
					break;
				}

				n_nextaddress = mdec_unpack( m_n_mdec0_address );
				m_n_mdec0_size -= n_nextaddress - m_n_mdec0_address;
				m_n_mdec0_address = n_nextaddress;

				if( ( m_n_mdec0_command & ( 1L << 27 ) ) != 0 )
				{
					mdec_yuv2_to_rgb15();
				}
				else
				{
					mdec_yuv2_to_rgb24();
				}
				mdec_offset = 0;
			}

			n_this = mdec_decoded;
			if( n_this > n_size )
			{
				n_this = n_size;
			}
			mdec_decoded -= n_this;

			memcpy( (UINT8 *)g_p_n_psxram + n_address, (UINT8 *)mdec_output + mdec_offset, n_this * 4 );
			mdec_offset += n_this * 4;
			n_address += n_this * 4;
			n_size -= n_this;
		}

		if( (int)m_n_mdec0_size < 0 )
		{
			mame_printf_debug( "ran out of data %d\n", m_n_mdec0_size );
		}
	}
	else
	{
		mame_printf_debug( "mdec1_read no conversion :%08x:%08x:\n", m_n_mdec0_command, m_n_mdec0_size );
	}
	m_n_mdec1_status &= ~( 1L << 29 );
}

WRITE32_HANDLER( psx_mdec_w )
{
	switch( offset )
	{
	case 0:
		verboselog( space->machine, 2, "mdec 0 command %08x\n", data );
		m_n_mdec0_command = data;
		break;
	case 1:
		verboselog( space->machine, 2, "mdec 1 command %08x\n", data );
		m_n_mdec1_command = data;
		break;
	}
}

READ32_HANDLER( psx_mdec_r )
{
	switch( offset )
	{
	case 0:
		verboselog( space->machine, 2, "mdec 0 status %08x\n", 0 );
		return 0;
	case 1:
		verboselog( space->machine, 2, "mdec 1 status %08x\n", m_n_mdec1_status );
		return m_n_mdec1_status;
	}
	return 0;
}

static void gpu_read( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	psx_gpu_read( machine, &g_p_n_psxram[ n_address / 4 ], n_size );
}

static void gpu_write( running_machine *machine, UINT32 n_address, INT32 n_size )
{
	psx_gpu_write( machine, &g_p_n_psxram[ n_address / 4 ], n_size );
}

void psx_machine_init( running_machine *machine )
{
	int n;

	/* irq */
	m_n_irqdata = 0;
	m_n_irqmask = 0;

	/* dma */
	m_n_dpcp = 0;
	m_n_dicr = 0;

	m_n_mdec0_command = 0;
	m_n_mdec0_address = 0;
	m_n_mdec0_size = 0;
	m_n_mdec1_command = 0;
	m_n_mdec1_status = 0;

	for( n = 0; n < 7; n++ )
	{
		dma_stop_timer( n );
	}

	for( n = 0; n < 2; n++ )
	{
		m_p_n_sio_status[ n ] = SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
		m_p_n_sio_mode[ n ] = 0;
		m_p_n_sio_control[ n ] = 0;
		m_p_n_sio_baud[ n ] = 0;
		m_p_n_sio_tx[ n ] = 0;
		m_p_n_sio_rx[ n ] = 0;
		m_p_n_sio_tx_prev[ n ] = 0;
		m_p_n_sio_rx_prev[ n ] = 0;
		m_p_n_sio_rx_data[ n ] = 0;
		m_p_n_sio_tx_data[ n ] = 0;
		m_p_n_sio_rx_shift[ n ] = 0;
		m_p_n_sio_tx_shift[ n ] = 0;
		m_p_n_sio_rx_bits[ n ] = 0;
		m_p_n_sio_tx_bits[ n ] = 0;
	}

	psx_gpu_reset(machine);
}

static STATE_POSTLOAD( psx_postload )
{
	int n;

	psx_irq_update(machine);

	for( n = 0; n < 7; n++ )
	{
		dma_timer_adjust( n );
	}

	for( n = 0; n < 3; n++ )
	{
		root_timer_adjust( machine, n );
	}

	for( n = 0; n < 2; n++ )
	{
		sio_timer_adjust( machine, n );
	}

	mdec_cos_precalc();
}

void psx_driver_init( running_machine *machine )
{
	int n;

	for( n = 0; n < 7; n++ )
	{
		m_p_timer_dma[ n ] = timer_alloc( machine, dma_finished_callback, machine );
		m_p_fn_dma_read[ n ] = NULL;
		m_p_fn_dma_write[ n ] = NULL;
	}

	for( n = 0; n < 3; n++ )
	{
		m_p_timer_root[ n ] = timer_alloc( machine, root_finished , NULL);
	}

	for( n = 0; n < 2; n++ )
	{
		m_p_timer_sio[ n ] = timer_alloc( machine, sio_clock , NULL);
	}

	for( n = 0; n < 256; n++ )
	{
		m_p_n_mdec_clamp8[ n ] = 0;
		m_p_n_mdec_clamp8[ n + 256 ] = n;
		m_p_n_mdec_clamp8[ n + 512 ] = 255;

		m_p_n_mdec_r5[ n ] = 0;
		m_p_n_mdec_r5[ n + 256 ] = ( n >> 3 );
		m_p_n_mdec_r5[ n + 512 ] = ( 255 >> 3 );

		m_p_n_mdec_g5[ n ] = 0;
		m_p_n_mdec_g5[ n + 256 ] = ( n >> 3 ) << 5;
		m_p_n_mdec_g5[ n + 512 ] = ( 255 >> 3 ) << 5;

		m_p_n_mdec_b5[ n ] = 0;
		m_p_n_mdec_b5[ n + 256 ] = ( n >> 3 ) << 10;
		m_p_n_mdec_b5[ n + 512 ] = ( 255 >> 3 ) << 10;
	}

	for( n = 0; n < 2; n++ )
	{
		m_p_f_sio_handler[ n ] = NULL;
	}

	psx_dma_install_read_handler( 1, mdec1_read );
	psx_dma_install_read_handler( 2, gpu_read );

	psx_dma_install_write_handler( 0, mdec0_write );
	psx_dma_install_write_handler( 2, gpu_write );

	state_save_register_global(machine,  m_n_irqdata );
	state_save_register_global(machine,  m_n_irqmask );
	state_save_register_global_array(machine,  m_p_n_dmabase );
	state_save_register_global_array(machine,  m_p_n_dmablockcontrol );
	state_save_register_global_array(machine,  m_p_n_dmachannelcontrol );
	state_save_register_global_array(machine,  m_p_n_dma_ticks );
	state_save_register_global_array(machine,  m_p_b_dma_running );
	state_save_register_global(machine,  m_n_dpcp );
	state_save_register_global(machine,  m_n_dicr );
	state_save_register_global_array(machine,  m_p_n_root_count );
	state_save_register_global_array(machine,  m_p_n_root_mode );
	state_save_register_global_array(machine,  m_p_n_root_target );
	state_save_register_global_array(machine,  m_p_n_root_start );

	state_save_register_global_array(machine,  m_p_n_sio_status );
	state_save_register_global_array(machine,  m_p_n_sio_mode );
	state_save_register_global_array(machine,  m_p_n_sio_control );
	state_save_register_global_array(machine,  m_p_n_sio_baud );
	state_save_register_global_array(machine,  m_p_n_sio_tx );
	state_save_register_global_array(machine,  m_p_n_sio_rx );
	state_save_register_global_array(machine,  m_p_n_sio_tx_prev );
	state_save_register_global_array(machine,  m_p_n_sio_rx_prev );
	state_save_register_global_array(machine,  m_p_n_sio_rx_data );
	state_save_register_global_array(machine,  m_p_n_sio_tx_data );
	state_save_register_global_array(machine,  m_p_n_sio_rx_shift );
	state_save_register_global_array(machine,  m_p_n_sio_tx_shift );
	state_save_register_global_array(machine,  m_p_n_sio_rx_bits );
	state_save_register_global_array(machine,  m_p_n_sio_tx_bits );

	state_save_register_global(machine,  m_n_mdec0_command );
	state_save_register_global(machine,  m_n_mdec0_address );
	state_save_register_global(machine,  m_n_mdec0_size );
	state_save_register_global(machine,  m_n_mdec1_command );
	state_save_register_global(machine,  m_n_mdec1_status );
	state_save_register_global_array(machine,  m_p_n_mdec_quantize_y );
	state_save_register_global_array(machine,  m_p_n_mdec_quantize_uv );
	state_save_register_global_array(machine,  m_p_n_mdec_cos );

	state_save_register_postload( machine, psx_postload, NULL );
}

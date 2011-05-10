/***************************************************************************

  machine/psx.c

  Thanks to Olivier Galibert for IDCT information

***************************************************************************/

#include "emu.h"
#include "cpu/psx/psx.h"
#include "includes/psx.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( psx_machine *p_psx, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", p_psx->machine().describe_context(), buf );
	}
}

WRITE32_HANDLER( psx_com_delay_w )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;

	COMBINE_DATA( &p_psx->n_com_delay );
	verboselog( p_psx, 1, "psx_com_delay_w( %08x %08x )\n", data, mem_mask );
}

READ32_HANDLER( psx_com_delay_r )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;

	verboselog( p_psx, 1, "psx_com_delay_r( %08x )\n", mem_mask );
	return p_psx->n_com_delay;
}

/* IRQ */

static void psx_irq_update( psx_machine *p_psx )
{
	if( ( p_psx->n_irqdata & p_psx->n_irqmask ) != 0 )
	{
		verboselog( p_psx, 2, "psx irq assert\n" );
		cputag_set_input_line( p_psx->machine(), "maincpu", PSXCPU_IRQ0, ASSERT_LINE );
	}
	else
	{
		verboselog( p_psx, 2, "psx irq clear\n" );
		cputag_set_input_line( p_psx->machine(), "maincpu", PSXCPU_IRQ0, CLEAR_LINE );
	}
}

WRITE32_HANDLER( psx_irq_w )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;

	switch( offset )
	{
	case 0x00:
		verboselog( p_psx, 2, "psx irq data ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, p_psx->n_irqdata, ( p_psx->n_irqdata & ~mem_mask ) | ( p_psx->n_irqdata & p_psx->n_irqmask & data ) );
		p_psx->n_irqdata = ( p_psx->n_irqdata & ~mem_mask ) | ( p_psx->n_irqdata & p_psx->n_irqmask & data );
		psx_irq_update(p_psx);
		break;
	case 0x01:
		verboselog( p_psx, 2, "psx irq mask ( %08x, %08x ) %08x -> %08x\n", data, mem_mask, p_psx->n_irqmask, ( p_psx->n_irqmask & ~mem_mask ) | data );
		p_psx->n_irqmask = ( p_psx->n_irqmask & ~mem_mask ) | data;
		if( ( p_psx->n_irqmask &~ PSX_IRQ_MASK ) != 0 )
		{
			verboselog( p_psx, 0, "psx_irq_w( %08x, %08x, %08x ) unknown irq\n", offset, data, mem_mask );
		}
		psx_irq_update(p_psx);
		break;
	default:
		verboselog( p_psx, 0, "psx_irq_w( %08x, %08x, %08x ) unknown register\n", offset, data, mem_mask );
		break;
	}
}

READ32_HANDLER( psx_irq_r )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;

	switch( offset )
	{
	case 0x00:
		verboselog( p_psx, 1, "psx_irq_r irq data %08x\n", p_psx->n_irqdata );
		return p_psx->n_irqdata;
	case 0x01:
		verboselog( p_psx, 1, "psx_irq_r irq mask %08x\n", p_psx->n_irqmask );
		return p_psx->n_irqmask;
	default:
		verboselog( p_psx, 0, "psx_irq_r unknown register %d\n", offset );
		break;
	}
	return 0;
}

void psx_irq_set( running_machine &machine, UINT32 data )
{
	psx_machine *p_psx = machine.driver_data<psx_state>()->m_p_psx;

	verboselog( p_psx, 2, "psx_irq_set %08x\n", data );
	p_psx->n_irqdata |= data;
	psx_irq_update(p_psx);
}

/* DMA */

void psx_dma_install_read_handler( running_machine &machine, int n_channel, psx_dma_read_delegate p_fn_dma_read )
{
	psxcpu_device::install_dma_read_handler( *machine.device("maincpu"), "maincpu", n_channel, p_fn_dma_read );
}

void psx_dma_install_write_handler( running_machine &machine, int n_channel, psx_dma_read_delegate p_fn_dma_write )
{
	psxcpu_device::install_dma_write_handler( *machine.device("maincpu"), "maincpu", n_channel, p_fn_dma_write );
}

/* Root Counters */

static UINT64 psxcpu_gettotalcycles( psx_machine *p_psx )
{
	/* TODO: should return the start of the current tick. */
	return p_psx->machine().firstcpu->total_cycles() * 2;
}

static int root_divider( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

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

static UINT16 root_current( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( ( root->n_mode & PSX_RC_STOP ) != 0 )
	{
		return root->n_count;
	}
	else
	{
		UINT64 n_current;
		n_current = psxcpu_gettotalcycles(p_psx) - root->n_start;
		n_current /= root_divider( p_psx, n_counter );
		n_current += root->n_count;
		if( n_current > 0xffff )
		{
			/* TODO: use timer for wrap on 0x10000. */
			root->n_count = n_current;
			root->n_start = psxcpu_gettotalcycles(p_psx);
		}
		return n_current;
	}
}

static int root_target( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( ( root->n_mode & PSX_RC_COUNTTARGET ) != 0 ||
		( root->n_mode & PSX_RC_IRQTARGET ) != 0 )
	{
		return root->n_target;
	}
	return 0x10000;
}

static void root_timer_adjust( psx_machine *p_psx, int n_counter )
{
	psx_root *root = &p_psx->root[ n_counter ];

	if( ( root->n_mode & PSX_RC_STOP ) != 0 )
	{
		root->timer->adjust( attotime::never, n_counter);
	}
	else
	{
		int n_duration;

		n_duration = root_target( p_psx, n_counter ) - root_current( p_psx, n_counter );
		if( n_duration < 1 )
		{
			n_duration += 0x10000;
		}

		n_duration *= root_divider( p_psx, n_counter );

		root->timer->adjust( attotime::from_hz(33868800) * n_duration, n_counter);
	}
}

static TIMER_CALLBACK( root_finished )
{
	psx_machine *p_psx = machine.driver_data<psx_state>()->m_p_psx;
	int n_counter = param;
	psx_root *root = &p_psx->root[ n_counter ];

	verboselog( p_psx, 2, "root_finished( %d ) %04x\n", n_counter, root_current( p_psx, n_counter ) );
	//if( ( root->n_mode & PSX_RC_COUNTTARGET ) != 0 )
	{
		/* TODO: wrap should be handled differently as PSX_RC_COUNTTARGET & PSX_RC_IRQTARGET don't have to be the same. */
		root->n_count = 0;
		root->n_start = psxcpu_gettotalcycles(p_psx);
	}
	if( ( root->n_mode & PSX_RC_REPEAT ) != 0 )
	{
		root_timer_adjust( p_psx, n_counter );
	}
	if( ( root->n_mode & PSX_RC_IRQOVERFLOW ) != 0 ||
		( root->n_mode & PSX_RC_IRQTARGET ) != 0 )
	{
		psx_irq_set( machine, PSX_IRQ_ROOTCOUNTER0 << n_counter );
	}
}

WRITE32_HANDLER( psx_counter_w )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;
	int n_counter = offset / 4;
	psx_root *root = &p_psx->root[ n_counter ];

	verboselog( p_psx, 1, "psx_counter_w ( %08x, %08x, %08x )\n", offset, data, mem_mask );

	switch( offset % 4 )
	{
	case 0:
		root->n_count = data;
		root->n_start = psxcpu_gettotalcycles(p_psx);
		break;
	case 1:
		root->n_count = root_current( p_psx, n_counter );
		root->n_start = psxcpu_gettotalcycles(p_psx);

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
			mame_printf_debug( "mode %d 0x%04x\n", n_counter, data & 0xfca6 );
		}
#endif
		break;
	case 2:
		root->n_target = data;
		break;
	default:
		verboselog( p_psx, 0, "psx_counter_w( %08x, %08x, %08x ) unknown register\n", offset, mem_mask, data );
		return;
	}

	root_timer_adjust( p_psx, n_counter );
}

READ32_HANDLER( psx_counter_r )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;
	int n_counter = offset / 4;
	psx_root *root = &p_psx->root[ n_counter ];
	UINT32 data;

	switch( offset % 4 )
	{
	case 0:
		data = root_current( p_psx, n_counter );
		break;
	case 1:
		data = root->n_mode;
		break;
	case 2:
		data = root->n_target;
		break;
	default:
		verboselog( p_psx, 0, "psx_counter_r( %08x, %08x ) unknown register\n", offset, mem_mask );
		return 0;
	}
	verboselog( p_psx, 1, "psx_counter_r ( %08x, %08x ) %08x\n", offset, mem_mask, data );
	return data;
}

/* SIO */

static void sio_interrupt( psx_machine *p_psx, int n_port )
{
	psx_sio *sio = &p_psx->sio[ n_port ];

	verboselog( p_psx, 1, "sio_interrupt( %d )\n", n_port );
	sio->n_status |= SIO_STATUS_IRQ;
	if( n_port == 0 )
	{
		psx_irq_set( p_psx->machine(), PSX_IRQ_SIO0 );
	}
	else
	{
		psx_irq_set( p_psx->machine(), PSX_IRQ_SIO1 );
	}
}

static void sio_timer_adjust( psx_machine *p_psx, int n_port )
{
	psx_sio *sio = &p_psx->sio[ n_port ];
	attotime n_time;

	if( ( sio->n_status & SIO_STATUS_TX_EMPTY ) == 0 || sio->n_tx_bits != 0 )
	{
		int n_prescaler;

		switch( sio->n_mode & 3 )
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

		if( sio->n_baud != 0 && n_prescaler != 0 )
		{
			n_time = attotime::from_hz(33868800) * (n_prescaler * sio->n_baud);
			verboselog( p_psx, 2, "sio_timer_adjust( %d ) = %s ( %d x %d )\n", n_port, n_time.as_string(), n_prescaler, sio->n_baud );
		}
		else
		{
			n_time = attotime::never;
			verboselog( p_psx, 0, "sio_timer_adjust( %d ) invalid baud rate ( %d x %d )\n", n_port, n_prescaler, sio->n_baud );
		}
	}
	else
	{
		n_time = attotime::never;
		verboselog( p_psx, 2, "sio_timer_adjust( %d ) finished\n", n_port );
	}
	sio->timer->adjust( n_time, n_port);
}

static TIMER_CALLBACK( sio_clock )
{
	psx_machine *p_psx = machine.driver_data<psx_state>()->m_p_psx;
	int n_port = param;
	psx_sio *sio = &p_psx->sio[ n_port ];
	verboselog( p_psx, 2, "sio tick\n" );

	if( sio->n_tx_bits == 0 &&
		( sio->n_control & SIO_CONTROL_TX_ENA ) != 0 &&
		( sio->n_status & SIO_STATUS_TX_EMPTY ) == 0 )
	{
		sio->n_tx_bits = 8;
		sio->n_tx_shift = sio->n_tx_data;
		if( n_port == 0 )
		{
			sio->n_rx_bits = 8;
			sio->n_rx_shift = 0;
		}
		sio->n_status |= SIO_STATUS_TX_EMPTY;
		sio->n_status |= SIO_STATUS_TX_RDY;
	}

	if( sio->n_tx_bits != 0 )
	{
		sio->n_tx = ( sio->n_tx & ~PSX_SIO_OUT_DATA ) | ( ( sio->n_tx_shift & 1 ) * PSX_SIO_OUT_DATA );
		sio->n_tx_shift >>= 1;
		sio->n_tx_bits--;

		if( sio->fn_handler != NULL )
		{
			if( n_port == 0 )
			{
				sio->n_tx &= ~PSX_SIO_OUT_CLOCK;
				(*sio->fn_handler)( machine, sio->n_tx );
				sio->n_tx |= PSX_SIO_OUT_CLOCK;
			}
			(*sio->fn_handler)( machine, sio->n_tx );
		}

		if( sio->n_tx_bits == 0 &&
			( sio->n_control & SIO_CONTROL_TX_IENA ) != 0 )
		{
			sio_interrupt( p_psx, n_port );
		}
	}

	if( sio->n_rx_bits != 0 )
	{
		sio->n_rx_shift = ( sio->n_rx_shift >> 1 ) | ( ( ( sio->n_rx & PSX_SIO_IN_DATA ) / PSX_SIO_IN_DATA ) << 7 );
		sio->n_rx_bits--;

		if( sio->n_rx_bits == 0 )
		{
			if( ( sio->n_status & SIO_STATUS_RX_RDY ) != 0 )
			{
				sio->n_status |= SIO_STATUS_OVERRUN;
			}
			else
			{
				sio->n_rx_data = sio->n_rx_shift;
				sio->n_status |= SIO_STATUS_RX_RDY;
			}
			if( ( sio->n_control & SIO_CONTROL_RX_IENA ) != 0 )
			{
				sio_interrupt( p_psx, n_port );
			}
		}
	}

	sio_timer_adjust( p_psx, n_port );
}

void psx_sio_input( running_machine &machine, int n_port, int n_mask, int n_data )
{
	psx_machine *p_psx = machine.driver_data<psx_state>()->m_p_psx;
	psx_sio *sio = &p_psx->sio[ n_port ];
	verboselog( p_psx, 1, "psx_sio_input( %d, %02x, %02x )\n", n_port, n_mask, n_data );
	sio->n_rx = ( sio->n_rx & ~n_mask ) | ( n_data & n_mask );

	if( ( sio->n_rx & PSX_SIO_IN_DSR ) != 0 )
	{
		sio->n_status |= SIO_STATUS_DSR;
		if( ( sio->n_rx_prev & PSX_SIO_IN_DSR ) == 0 &&
			( sio->n_control & SIO_CONTROL_DSR_IENA ) != 0 )
		{
			sio_interrupt( p_psx, n_port );
		}
	}
	else
	{
		sio->n_status &= ~SIO_STATUS_DSR;
	}
	sio->n_rx_prev = sio->n_rx;
}

WRITE32_HANDLER( psx_sio_w )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;
	int n_port = offset / 4;
	psx_sio *sio = &p_psx->sio[ n_port ];

	switch( offset % 4 )
	{
	case 0:
		verboselog( p_psx, 1, "psx_sio_w %d data %02x (%08x)\n", n_port, data, mem_mask );
		sio->n_tx_data = data;
		sio->n_status &= ~( SIO_STATUS_TX_RDY );
		sio->n_status &= ~( SIO_STATUS_TX_EMPTY );
		sio_timer_adjust( p_psx, n_port );
		break;
	case 1:
		verboselog( p_psx, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	case 2:
		if( ACCESSING_BITS_0_15 )
		{
			sio->n_mode = data & 0xffff;
			verboselog( p_psx, 1, "psx_sio_w %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 1, "psx_sio_w %d control %04x\n", n_port, data >> 16 );
			sio->n_control = data >> 16;

			if( ( sio->n_control & SIO_CONTROL_RESET ) != 0 )
			{
				verboselog( p_psx, 1, "psx_sio_w reset\n" );
				sio->n_status |= SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
				sio->n_status &= ~( SIO_STATUS_RX_RDY | SIO_STATUS_OVERRUN | SIO_STATUS_IRQ );
			}
			if( ( sio->n_control & SIO_CONTROL_IACK ) != 0 )
			{
				verboselog( p_psx, 1, "psx_sio_w iack\n" );
				sio->n_status &= ~( SIO_STATUS_IRQ );
				sio->n_control &= ~( SIO_CONTROL_IACK );
			}
			if( ( sio->n_control & SIO_CONTROL_DTR ) != 0 )
			{
				sio->n_tx |= PSX_SIO_OUT_DTR;
			}
			else
			{
				sio->n_tx &= ~PSX_SIO_OUT_DTR;
			}

			if( ( ( sio->n_tx ^ sio->n_tx_prev ) & PSX_SIO_OUT_DTR ) != 0 )
			{
				if( sio->fn_handler != NULL )
				{
					(*sio->fn_handler)( space->machine(), sio->n_tx );
				}
			}
			sio->n_tx_prev = sio->n_tx;

		}
		break;
	case 3:
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		}
		if( ACCESSING_BITS_16_31 )
		{
			sio->n_baud = data >> 16;
			verboselog( p_psx, 1, "psx_sio_w %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		verboselog( p_psx, 0, "psx_sio_w( %08x, %08x, %08x )\n", offset, data, mem_mask );
		break;
	}
}

READ32_HANDLER( psx_sio_r )
{
	psx_machine *p_psx = space->machine().driver_data<psx_state>()->m_p_psx;
	int n_port = offset / 4;
	psx_sio *sio = &p_psx->sio[ n_port ];
	UINT32 data;

	switch( offset % 4 )
	{
	case 0:
		data = sio->n_rx_data;
		sio->n_status &= ~( SIO_STATUS_RX_RDY );
		sio->n_rx_data = 0xff;
		verboselog( p_psx, 1, "psx_sio_r %d data %02x (%08x)\n", n_port, data, mem_mask );
		break;
	case 1:
		data = sio->n_status;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d status %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		break;
	case 2:
		data = ( sio->n_control << 16 ) | sio->n_mode;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d mode %04x\n", n_port, data & 0xffff );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d control %04x\n", n_port, data >> 16 );
		}
		break;
	case 3:
		data = sio->n_baud << 16;
		if( ACCESSING_BITS_0_15 )
		{
			verboselog( p_psx, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		}
		if( ACCESSING_BITS_16_31 )
		{
			verboselog( p_psx, 1, "psx_sio_r %d baud %04x\n", n_port, data >> 16 );
		}
		break;
	default:
		data = 0;
		verboselog( p_psx, 0, "psx_sio_r( %08x, %08x ) %08x\n", offset, mem_mask, data );
		break;
	}
	return data;
}

void psx_sio_install_handler( running_machine &machine, int n_port, psx_sio_handler p_f_sio_handler )
{
	psx_machine *p_psx = machine.driver_data<psx_state>()->m_p_psx;

	p_psx->sio[ n_port ].fn_handler = p_f_sio_handler;
}

static void gpu_read( psx_state *state, UINT32 n_address, INT32 n_size )
{
	psx_machine *p_psx = state->m_p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;

	psx_gpu_read( state->machine(), &p_n_psxram[ n_address / 4 ], n_size );
}

static void gpu_write( psx_state *state, UINT32 n_address, INT32 n_size )
{
	psx_machine *p_psx = state->m_p_psx;
	UINT32 *p_n_psxram = p_psx->p_n_psxram;

	psx_gpu_write( state->machine(), &p_n_psxram[ n_address / 4 ], n_size );
}

void psx_machine_init( running_machine &machine )
{
	psx_machine *p_psx = machine.driver_data<psx_state>()->m_p_psx;
	int n;

	/* irq */
	p_psx->n_irqdata = 0;
	p_psx->n_irqmask = 0;

	for( n = 0; n < 2; n++ )
	{
		p_psx->sio[ n ].n_status = SIO_STATUS_TX_EMPTY | SIO_STATUS_TX_RDY;
		p_psx->sio[ n ].n_mode = 0;
		p_psx->sio[ n ].n_control = 0;
		p_psx->sio[ n ].n_baud = 0;
		p_psx->sio[ n ].n_tx = 0;
		p_psx->sio[ n ].n_rx = 0;
		p_psx->sio[ n ].n_tx_prev = 0;
		p_psx->sio[ n ].n_rx_prev = 0;
		p_psx->sio[ n ].n_rx_data = 0;
		p_psx->sio[ n ].n_tx_data = 0;
		p_psx->sio[ n ].n_rx_shift = 0;
		p_psx->sio[ n ].n_tx_shift = 0;
		p_psx->sio[ n ].n_rx_bits = 0;
		p_psx->sio[ n ].n_tx_bits = 0;
	}

	psx_gpu_reset(machine);
}

static void psx_postload(psx_machine *p_psx)
{
	int n;

	psx_irq_update(p_psx);

	for( n = 0; n < 3; n++ )
	{
		root_timer_adjust( p_psx, n );
	}

	for( n = 0; n < 2; n++ )
	{
		sio_timer_adjust( p_psx, n );
	}
}

void psx_driver_init( running_machine &machine )
{
	psx_state *state = machine.driver_data<psx_state>();
	psx_machine *p_psx = auto_alloc_clear(machine, psx_machine);
	int n;

	state->m_p_psx = p_psx;
	state->m_p_n_psxram = (UINT32 *)memory_get_shared(machine, "share1", state->m_n_psxramsize);

	p_psx->m_machine = &machine;
	p_psx->p_n_psxram = state->m_p_n_psxram;
	p_psx->n_psxramsize = state->m_n_psxramsize;

	for( n = 0; n < 3; n++ )
	{
		p_psx->root[ n ].timer = machine.scheduler().timer_alloc( FUNC(root_finished ));
	}

	for( n = 0; n < 2; n++ )
	{
		p_psx->sio[ n ].timer = machine.scheduler().timer_alloc( FUNC(sio_clock ));
		p_psx->sio[ n ].fn_handler = NULL;
	}

	psx_dma_install_read_handler( machine, 2, psx_dma_read_delegate( FUNC( gpu_read ), state ) );
	psx_dma_install_write_handler( machine, 2, psx_dma_write_delegate( FUNC( gpu_write ), state ) );

	state_save_register_global( machine, p_psx->n_irqdata );
	state_save_register_global( machine, p_psx->n_irqmask );
	for ( n = 0; n < 3; n++ )
	{
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_count );
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_mode );
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_target );
		state_save_register_item( machine, "psxroot", NULL, n, p_psx->root[n].n_start );
	}
	for( n = 0; n < 2; n++ )
	{
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_status );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_mode );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_control );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_baud );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_prev );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_prev );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_data );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_data );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_shift );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_shift );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_rx_bits );
		state_save_register_item( machine, "psxsio", NULL, n, p_psx->sio[n].n_tx_bits );
	}

	machine.save().register_postload( save_prepost_delegate(FUNC(psx_postload), p_psx ));
}

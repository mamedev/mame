/***************************************************************************

  machine/psx.c

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

void psx_sio_install_handler( running_machine &machine, int n_port, psx_sio_handler p_f_sio_handler )
{
	psxcpu_device::install_sio_handler( *machine.device("maincpu"), "maincpu", n_port, p_f_sio_handler );
}

void psx_sio_input( running_machine &machine, int n_port, int n_mask, int n_data )
{
	psxcpu_device::sio_input( *machine.device("maincpu"), "maincpu", n_port, n_mask, n_data );
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

	/* irq */
	p_psx->n_irqdata = 0;
	p_psx->n_irqmask = 0;

	psx_gpu_reset(machine);
}

static void psx_postload(psx_machine *p_psx)
{
	psx_irq_update(p_psx);
}

void psx_driver_init( running_machine &machine )
{
	psx_state *state = machine.driver_data<psx_state>();
	psx_machine *p_psx = auto_alloc_clear(machine, psx_machine);

	state->m_p_psx = p_psx;
	state->m_p_n_psxram = (UINT32 *)memory_get_shared(machine, "share1", state->m_n_psxramsize);

	p_psx->m_machine = &machine;
	p_psx->p_n_psxram = state->m_p_n_psxram;
	p_psx->n_psxramsize = state->m_n_psxramsize;

	psx_dma_install_read_handler( machine, 2, psx_dma_read_delegate( FUNC( gpu_read ), state ) );
	psx_dma_install_write_handler( machine, 2, psx_dma_write_delegate( FUNC( gpu_write ), state ) );

	state_save_register_global( machine, p_psx->n_irqdata );
	state_save_register_global( machine, p_psx->n_irqmask );

	machine.save().register_postload( save_prepost_delegate(FUNC(psx_postload), p_psx ));
}

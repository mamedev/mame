/***************************************************************************

  machine/psx.c

***************************************************************************/

#include "emu.h"
#include "cpu/psx/psx.h"
#include "video/psx.h"
#include "includes/psx.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( psx_state *p_psx, int n_level, const char *s_fmt, ... )
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

void psx_driver_init( running_machine &machine )
{
	psx_state *p_psx = machine.driver_data<psx_state>();

	memory_share *share = machine.root_device().memshare("share1");
	p_psx->m_p_n_psxram = (UINT32 *)share->ptr();
	p_psx->m_n_psxramsize = share->bytes();
}

WRITE32_HANDLER( psx_com_delay_w )
{
	psx_state *p_psx = space.machine().driver_data<psx_state>();

	COMBINE_DATA( &p_psx->n_com_delay );
	verboselog( p_psx, 1, "psx_com_delay_w( %08x %08x )\n", data, mem_mask );
}

READ32_HANDLER( psx_com_delay_r )
{
	psx_state *p_psx = space.machine().driver_data<psx_state>();

	verboselog( p_psx, 1, "psx_com_delay_r( %08x )\n", mem_mask );
	return p_psx->n_com_delay;
}

/* SIO */

void psx_sio_install_handler( running_machine &machine, int n_port, psx_sio_handler p_f_sio_handler )
{
	psxcpu_device::install_sio_handler( *machine.device("maincpu^"), "maincpu", n_port, p_f_sio_handler );
}

void psx_sio_input( running_machine &machine, int n_port, int n_mask, int n_data )
{
	psxcpu_device::sio_input( *machine.device("maincpu^"), "maincpu", n_port, n_mask, n_data );
}

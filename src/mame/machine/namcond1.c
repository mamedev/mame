/***************************************************************************

  Namco ND-1

  machine.c

  Functions to emulate general aspects of the machine
  (RAM, ROM, interrupts, I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/ygv608.h"
#include "includes/namcond1.h"

/* Perform basic machine initialisation */


MACHINE_START( namcond1 )
{
	namcond1_state *state = machine.driver_data<namcond1_state>();
	state_save_register_global(machine, state->m_h8_irq5_enabled);
}

MACHINE_RESET( namcond1 )
{
	namcond1_state *state = machine.driver_data<namcond1_state>();
#ifdef MAME_DEBUG
    /*UINT8   *ROM = machine.region(REGION_CPU1)->base();*/
    /*UINT32 debug_trigger_addr;*/
    /*int             i;*/

#if 0
    // debug trigger patch
    // insert a "move.b $B0000000,D2" into the code
    debug_trigger_addr = 0x152d4; // after ygv_init
    ROM[debug_trigger_addr++] = 0x39;
    ROM[debug_trigger_addr++] = 0x14;
    ROM[debug_trigger_addr++] = 0xB0;
    ROM[debug_trigger_addr++] = 0x00;
    ROM[debug_trigger_addr++] = 0x00;
    ROM[debug_trigger_addr++] = 0x00;
#endif
#endif

    // initialise MCU states
    state->m_h8_irq5_enabled = 0;

    // halt the MCU
    cputag_set_input_line(machine, "mcu", INPUT_LINE_RESET, ASSERT_LINE);
}

// instance of the shared ram pointer

READ16_MEMBER(namcond1_state::namcond1_shared_ram_r)
{
	return m_shared_ram[offset];
}

// $c3ff00-$c3ffff
READ16_MEMBER(namcond1_state::namcond1_cuskey_r)
{
    switch( offset )
    {
        // this address returns a jump vector inside ISR2
        // - if zero then the ISR returns without jumping
        case (0x2e>>1):
            return( 0x0000 );
        case (0x30>>1):
            return( 0x0000 );

        default :
            logerror( "offset $%X accessed from $%X\n",
                      offset<<1, cpu_get_pc(&space.device()) );
            return( 0 );
    }
}

WRITE16_MEMBER(namcond1_state::namcond1_shared_ram_w)
{

    switch( offset )
    {
        default :
            COMBINE_DATA( m_shared_ram + offset );
            break;
    }
}

WRITE16_MEMBER(namcond1_state::namcond1_cuskey_w)
{
    switch( offset )
    {
        case (0x0a>>1):
            // this is a kludge until we emulate the h8
	    if ((m_h8_irq5_enabled == 0) && (data != 0x0000))
	    {
	    	cputag_set_input_line(machine(), "mcu", INPUT_LINE_RESET, CLEAR_LINE);
	    }
            m_h8_irq5_enabled = ( data != 0x0000 );
            break;

		case (0x0c>>1):
			ygv608_set_gfxbank((data & 0x0002) >> 1); // i think
			// should mark tilemaps dirty but i think they already are
			break;

        default :
            break;
    }
}

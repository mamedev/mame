/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "includes/stfight.h"

// this prototype will move to the driver



/*

Encryption PAL 16R4 on CPU board

          +---U---+
     CP --|       |-- VCC
 ROM D1 --|       |-- ROM D0          M1 = 0                M1 = 1
 ROM D3 --|       |-- (NC)
 ROM D4 --|       |-- D6         D6 = D1 ^^ D3          D6 = / ( D1 ^^ D0 )
 ROM D6 --|       |-- D4         D4 = / ( D6 ^^ A7 )    D4 = D3 ^^ A0
     A0 --|       |-- D3         D3 = / ( D0 ^^ A1 )    D3 = D4 ^^ A4
     A1 --|       |-- D0         D0 = D1 ^^ D4          D0 = / ( D6 ^^ A0 )
     A4 --|       |-- (NC)
     A7 --|       |-- /M1
    GND --|       |-- /OE
          +-------+

*/


DRIVER_INIT( empcity )
{
	stfight_state *state = machine.driver_data<stfight_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *rom = machine.region("maincpu")->base();
	int A;

	state->m_decrypt = auto_alloc_array(machine, UINT8, 0x8000);
	space->set_decrypted_region(0x0000, 0x7fff, state->m_decrypt);

	for (A = 0;A < 0x8000;A++)
	{
		UINT8 src = rom[A];

		// decode opcode
		state->m_decrypt[A] =
				( src & 0xA6 ) |
				( ( ( ( src << 2 ) ^ src ) << 3 ) & 0x40 ) |
				( ~( ( src ^ ( A >> 1 ) ) >> 2 ) & 0x10 ) |
				( ~( ( ( src << 1 ) ^ A ) << 2 ) & 0x08 ) |
				( ( ( src ^ ( src >> 3 ) ) >> 1 ) & 0x01 );

		// decode operand
		rom[A] =
				( src & 0xA6 ) |
				( ~( ( src ^ ( src << 1 ) ) << 5 ) & 0x40 ) |
				( ( ( src ^ ( A << 3 ) ) << 1 ) & 0x10 ) |
				( ( ( src ^ A ) >> 1 ) & 0x08 ) |
				( ~( ( src >> 6 ) ^ A ) & 0x01 );
	}
}

DRIVER_INIT( stfight )
{
	stfight_state *state = machine.driver_data<stfight_state>();
	DRIVER_INIT_CALL(empcity);

	/* patch out a tight loop during startup - is the code waiting */
	/* for NMI to wake it up? */
	state->m_decrypt[0xb1] = 0x00;
	state->m_decrypt[0xb2] = 0x00;
	state->m_decrypt[0xb3] = 0x00;
	state->m_decrypt[0xb4] = 0x00;
	state->m_decrypt[0xb5] = 0x00;
}

MACHINE_RESET( stfight )
{
	stfight_state *state = machine.driver_data<stfight_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	state->m_adpcm_data_offs = state->m_adpcm_data_end = 0;
	state->m_toggle = 0;
	state->m_fm_data = 0;
	state->m_coin_mech_latch[0] = 0x02;
	state->m_coin_mech_latch[1] = 0x01;

	state->m_coin_mech_query_active = 0;
	state->m_coin_mech_query = 0;

    // initialise rom bank
    state->stfight_bank_w(*space, 0, 0 );
}

// It's entirely possible that this bank is never switched out
// - in fact I don't even know how/where it's switched in!
WRITE8_MEMBER(stfight_state::stfight_bank_w)
{
	UINT8   *ROM2 = machine().region("maincpu")->base() + 0x10000;

	memory_set_bankptr(machine(),  "bank1", &ROM2[data<<14] );
}

/*
 *      CPU 1 timed interrupt - 60Hz???
 */

static TIMER_CALLBACK( stfight_interrupt_1 )
{
    // Do a RST08
    cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, 0xcf);
}

INTERRUPT_GEN( stfight_vb_interrupt )
{
    // Do a RST10
    device_set_input_line_and_vector(device, 0, HOLD_LINE, 0xd7);
    device->machine().scheduler().timer_set(attotime::from_hz(120), FUNC(stfight_interrupt_1));
}

/*
 *      Hardware handlers
 */

// Perhaps define dipswitches as active low?
READ8_MEMBER(stfight_state::stfight_dsw_r)
{
    return( ~input_port_read(machine(), offset ? "DSW1" : "DSW0") );
}

READ8_MEMBER(stfight_state::stfight_coin_r)
{
    int coin_mech_data;
    int i;

    // Was the coin mech queried by software?
    if( m_coin_mech_query_active )
    {
        m_coin_mech_query_active = 0;
        return( (~m_coin_mech_query) & 0x03 );
    }

    /*
     *      Is this really necessary?
     *      - we can control impulse length so that the port is
     *        never strobed twice within the impulse period
     *        since it's read by the 30Hz interrupt ISR
     */

    coin_mech_data = input_port_read(machine(), "COIN");

    for( i=0; i<2; i++ )
    {
        /* Only valid on signal edge */
        if( ( coin_mech_data & (1<<i) ) != m_coin_mech_latch[i] )
            m_coin_mech_latch[i] = coin_mech_data & (1<<i);
        else
            coin_mech_data |= coin_mech_data & (1<<i);
    }

    return( coin_mech_data );
}

WRITE8_MEMBER(stfight_state::stfight_coin_w)
{
    // interrogate coin mech
    m_coin_mech_query_active = 1;
    m_coin_mech_query = data;
}

/*
 *      Machine hardware for MSM5205 ADPCM sound control
 */

static const int sampleLimits[] =
{
    0x0000,     // machine gun fire?
    0x1000,     // player getting shot
    0x2C00,     // player shooting
    0x3C00,     // girl screaming
    0x5400,     // girl getting shot
    0x7200      // (end of samples)
};

void stfight_adpcm_int(device_t *device)
{
	stfight_state *state = device->machine().driver_data<stfight_state>();
	UINT8 *SAMPLES = device->machine().region("adpcm")->base();
	int adpcm_data = SAMPLES[state->m_adpcm_data_offs & 0x7fff];

    // finished playing sample?
    if( state->m_adpcm_data_offs == state->m_adpcm_data_end )
    {
        msm5205_reset_w( device, 1 );
        return;
    }

	if( state->m_toggle == 0 )
		msm5205_data_w( device, ( adpcm_data >> 4 ) & 0x0f );
	else
	{
		msm5205_data_w( device, adpcm_data & 0x0f );
		state->m_adpcm_data_offs++;
	}

	state->m_toggle ^= 1;
}

WRITE8_DEVICE_HANDLER( stfight_adpcm_control_w )
{
	stfight_state *state = device->machine().driver_data<stfight_state>();
    if( data < 0x08 )
    {
        state->m_adpcm_data_offs = sampleLimits[data];
        state->m_adpcm_data_end = sampleLimits[data+1];
    }

    msm5205_reset_w( device, data & 0x08 ? 1 : 0 );
}

WRITE8_MEMBER(stfight_state::stfight_e800_w)
{
}

/*
 *      Machine hardware for YM2303 fm sound control
 */

WRITE8_MEMBER(stfight_state::stfight_fm_w)
{
    // the sound cpu ignores any fm data without bit 7 set
    m_fm_data = 0x80 | data;
}

READ8_MEMBER(stfight_state::stfight_fm_r)
{
    int data = m_fm_data;

    // clear the latch?!?
    m_fm_data &= 0x7f;

    return( data );
}

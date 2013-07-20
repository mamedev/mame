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


DRIVER_INIT_MEMBER(stfight_state,empcity)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *rom = memregion("maincpu")->base();
	int A;

	m_decrypt = auto_alloc_array(machine(), UINT8, 0x8000);
	space.set_decrypted_region(0x0000, 0x7fff, m_decrypt);

	for (A = 0;A < 0x8000;A++)
	{
		UINT8 src = rom[A];

		// decode opcode
		m_decrypt[A] =
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

DRIVER_INIT_MEMBER(stfight_state,stfight)
{
	DRIVER_INIT_CALL(empcity);

	/* patch out a tight loop during startup - is the code waiting */
	/* for NMI to wake it up? */
	m_decrypt[0xb1] = 0x00;
	m_decrypt[0xb2] = 0x00;
	m_decrypt[0xb3] = 0x00;
	m_decrypt[0xb4] = 0x00;
	m_decrypt[0xb5] = 0x00;
}

DRIVER_INIT_MEMBER(stfight_state,cshooter)
{
	UINT8 *rom = memregion("maincpu")->base();

	/* patch out initial coin mech check for now */
	rom[0x5068] = 0xc9; /* ret z -> ret*/
}

void stfight_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_adpcm_data_offs = m_adpcm_data_end = 0;
	m_toggle = 0;
	m_fm_data = 0;
	m_coin_mech_latch[0] = 0x02;
	m_coin_mech_latch[1] = 0x01;

	m_coin_mech_query_active = 0;
	m_coin_mech_query = 0;

	// initialise rom bank
	stfight_bank_w(space, 0, 0 );
}

// It's entirely possible that this bank is never switched out
// - in fact I don't even know how/where it's switched in!
WRITE8_MEMBER(stfight_state::stfight_bank_w)
{
	UINT8   *ROM2 = memregion("maincpu")->base() + 0x10000;
	UINT16 bank_num;

	bank_num = 0;

	if(data & 0x80)
		bank_num |= 0x8000;

	if(data & 0x04)
		bank_num |= 0x4000;

	membank("bank1")->set_base(&ROM2[bank_num] );
}

/*
 *      CPU 1 timed interrupt - 60Hz???
 */

void stfight_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_STFIGHT_INTERRUPT_1:
		// Do a RST08
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7);
		break;
	default:
		assert_always(FALSE, "Unknown id in stfight_state::device_timer");
	}
}

INTERRUPT_GEN_MEMBER(stfight_state::stfight_vb_interrupt)
{
	// Do a RST10
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0xcf);
	timer_set(attotime::from_hz(120), TIMER_STFIGHT_INTERRUPT_1);
}

/*
 *      Hardware handlers
 */

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

	coin_mech_data = ioport("COIN")->read();

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

READ8_MEMBER(stfight_state::cshooter_mcu_unk1_r)
{
	return 0xff; // hack so it boots
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

WRITE_LINE_MEMBER(stfight_state::stfight_adpcm_int)
{
	UINT8 *SAMPLES = memregion("adpcm")->base();
	int adpcm_data = SAMPLES[m_adpcm_data_offs & 0x7fff];

	// finished playing sample?
	if( m_adpcm_data_offs == m_adpcm_data_end )
	{
		m_msm->reset_w(1);
		return;
	}

	if( m_toggle == 0 )
		m_msm->data_w((adpcm_data >> 4) & 0x0f);
	else
	{
		m_msm->data_w(adpcm_data & 0x0f);
		m_adpcm_data_offs++;
	}

	m_toggle ^= 1;
}

WRITE8_MEMBER(stfight_state::stfight_adpcm_control_w)
{
	if( data < 0x08 )
	{
		m_adpcm_data_offs = sampleLimits[data];
		m_adpcm_data_end = sampleLimits[data+1];
	}

	m_msm->reset_w(BIT(data, 3));
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

/*
 * Cross Shooter MCU communications
 *
 * TODO: everything, especially MCU to main comms (tied with coinage ports?)
 */

WRITE8_MEMBER(stfight_state::cshooter_68705_ddr_a_w)
{
	m_ddrA = data;
}

WRITE8_MEMBER(stfight_state::cshooter_68705_ddr_b_w)
{
	m_ddrB = data;
}

WRITE8_MEMBER(stfight_state::cshooter_68705_ddr_c_w)
{
	m_ddrC = data;
}

WRITE8_MEMBER(stfight_state::cshooter_mcu_w)
{
	m_mcu->set_input_line(0, HOLD_LINE);
	m_from_main = data;
	m_main_sent = 1;

}

READ8_MEMBER(stfight_state::cshooter_68705_port_a_r)
{
	return (m_portA_out & m_ddrA) | (m_portA_in & ~m_ddrA);
}

WRITE8_MEMBER(stfight_state::cshooter_68705_port_a_w)
{
	m_portA_out = data;
}

READ8_MEMBER(stfight_state::cshooter_68705_port_b_r)
{
	return (m_portB_out & m_ddrB) | (m_portB_in & ~m_ddrB);
}

WRITE8_MEMBER(stfight_state::cshooter_68705_port_b_w)
{
	if ((m_ddrB & 0x01) && (data & 0x01) && (~m_portB_out & 0x01))
	{
		printf("bit 0\n");
	}
	if ((m_ddrB & 0x02) && (data & 0x02) && (~m_portB_out & 0x02))
	{
		printf("bit 1\n");
	}
	if ((m_ddrB & 0x04) && (data & 0x04) && (~m_portB_out & 0x04))
	{
		printf("bit 2\n");
	}
	if ((m_ddrB & 0x08) && (data & 0x08) && (~m_portB_out & 0x08))
	{
		printf("bit 3\n");
	}
	if ((m_ddrB & 0x10) && (data & 0x10) && (~m_portB_out & 0x10))
	{
		printf("bit 4\n");
	}
	if ((m_ddrB & 0x20) && (data & 0x20) && (~m_portB_out & 0x20))
	{
		m_portA_in = m_from_main;

		//if (m_main_sent)
		//	m_mcu->set_input_line(0, CLEAR_LINE);

		m_main_sent = 0;
	}
	if ((m_ddrB & 0x40) && (data & 0x40) && (~m_portB_out & 0x40))
	{
		printf("bit 6\n");
	}
	if ((m_ddrB & 0x80) && (data & 0x80) && (~m_portB_out & 0x80))
	{
		printf("bit 7\n");
	}

	m_portB_out = data;
}

READ8_MEMBER(stfight_state::cshooter_68705_port_c_r)
{
	return (m_portC_out & m_ddrC) | (m_portC_in & ~m_ddrC);
}

WRITE8_MEMBER(stfight_state::cshooter_68705_port_c_w)
{
	m_portC_out = data;
}

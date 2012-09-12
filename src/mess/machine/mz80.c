/***************************************************************************

        MZ80 driver by Miodrag Milanovic

        22/11/2008 Preliminary driver.

****************************************************************************/

#include "includes/mz80.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(mz80_state,mz80k)
{
}

MACHINE_RESET( mz80k )
{
	mz80_state *state = machine.driver_data<mz80_state>();
	state->m_mz80k_tempo_strobe = 0;
	state->m_mz80k_vertical = 0;
	state->m_mz80k_cursor_cnt = 0;
	state->m_mz80k_keyboard_line = 0;
}


READ8_MEMBER( mz80_state::mz80k_8255_portb_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"LINE%d", m_mz80k_keyboard_line);
	if (m_mz80k_keyboard_line > 9)
		return 0xff;
	else
		return ioport(kbdrow)->read();
}

READ8_MEMBER( mz80_state::mz80k_8255_portc_r )
{
	UINT8 val = 0;
	val |= m_mz80k_vertical ? 0x80 : 0x00;
	val |= ((m_mz80k_cursor_cnt & 0x3f) > 31) ? 0x40 : 0x00;
	val |= (m_cass->get_state() & CASSETTE_MASK_UISTATE)== CASSETTE_PLAY ? 0x10 : 0x00;

	if (m_cass->input() > 0.00)
		val |= 0x20;

	return val;
}

WRITE8_MEMBER( mz80_state::mz80k_8255_porta_w )
{
	m_mz80k_keyboard_line = data & 0x0f;
}

WRITE8_MEMBER( mz80_state::mz80k_8255_portc_w )
{
//  logerror("mz80k_8255_portc_w %02x\n",data);
}


WRITE_LINE_MEMBER( mz80_state::pit_out0_changed )
{
	if(!m_prev_state && state)
		m_speaker_level++;

	m_prev_state = state;
	speaker_level_w(m_speaker, BIT(m_speaker_level, 1));
}

WRITE_LINE_MEMBER( mz80_state::pit_out2_changed )
{
	machine().device("maincpu")->execute().set_input_line(0, HOLD_LINE);
}

I8255_INTERFACE( mz80k_8255_int )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mz80_state, mz80k_8255_porta_w),
	DEVCB_DRIVER_MEMBER(mz80_state, mz80k_8255_portb_r),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(mz80_state, mz80k_8255_portc_r),
	DEVCB_DRIVER_MEMBER(mz80_state, mz80k_8255_portc_w),
};

const struct pit8253_config mz80k_pit8253_config =
{
	{
		/* clockin        gate        callback    */
		{ XTAL_8MHz/  4,  DEVCB_NULL, DEVCB_DRIVER_LINE_MEMBER(mz80_state, pit_out0_changed) },
		{ XTAL_8MHz/256,  DEVCB_NULL, DEVCB_DEVICE_LINE("pit8253", pit8253_clk2_w)   },
		{	      0,  DEVCB_NULL, DEVCB_DRIVER_LINE_MEMBER(mz80_state, pit_out2_changed) },
	}
};

READ8_MEMBER( mz80_state::mz80k_strobe_r )
{
	return 0x7e | (UINT8)m_mz80k_tempo_strobe;
}

WRITE8_MEMBER( mz80_state::mz80k_strobe_w )
{
	pit8253_gate0_w(m_pit, BIT(data, 0));
}

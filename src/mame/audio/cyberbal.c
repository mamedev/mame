/***************************************************************************

    Cyberball 68000 sound simulator

****************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "machine/atarigen.h"
#include "includes/cyberbal.h"


static void update_sound_68k_interrupts(running_machine &machine);



void cyberbal_sound_reset(running_machine &machine)
{
	cyberbal_state *state = machine.driver_data<cyberbal_state>();

	/* reset the sound system */
	state->m_bank_base = &state->memregion("audiocpu")->base()[0x10000];
	state->membank("soundbank")->set_base(&state->m_bank_base[0x0000]);
	state->m_fast_68k_int = state->m_io_68k_int = 0;
	state->m_sound_data_from_68k = state->m_sound_data_from_6502 = 0;
	state->m_sound_data_from_68k_ready = state->m_sound_data_from_6502_ready = 0;
}



/*************************************
 *
 *  6502 Sound Interface
 *
 *************************************/

READ8_MEMBER(cyberbal_state::cyberbal_special_port3_r)
{
	int temp = ioport("JSAII")->read();
	if (!(ioport("IN0")->read() & 0x8000)) temp ^= 0x80;
	if (m_cpu_to_sound_ready) temp ^= 0x40;
	if (m_sound_to_cpu_ready) temp ^= 0x20;
	return temp;
}


READ8_MEMBER(cyberbal_state::cyberbal_sound_6502_stat_r)
{
	int temp = 0xff;
	if (m_sound_data_from_6502_ready) temp ^= 0x80;
	if (m_sound_data_from_68k_ready) temp ^= 0x40;
	return temp;
}


WRITE8_MEMBER(cyberbal_state::cyberbal_sound_bank_select_w)
{
	membank("soundbank")->set_base(&m_bank_base[0x1000 * ((data >> 6) & 3)]);
	coin_counter_w(machine(), 1, (data >> 5) & 1);
	coin_counter_w(machine(), 0, (data >> 4) & 1);
	machine().device("dac")->execute().set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x01)) machine().device("ymsnd")->reset();
}


READ8_MEMBER(cyberbal_state::cyberbal_sound_68k_6502_r)
{
	m_sound_data_from_68k_ready = 0;
	return m_sound_data_from_68k;
}


WRITE8_MEMBER(cyberbal_state::cyberbal_sound_68k_6502_w)
{
	m_sound_data_from_6502 = data;
	m_sound_data_from_6502_ready = 1;

	if (!m_io_68k_int)
	{
		m_io_68k_int = 1;
		update_sound_68k_interrupts(machine());
	}
}



/*************************************
 *
 *  68000 Sound Interface
 *
 *************************************/

static void update_sound_68k_interrupts(running_machine &machine)
{
	cyberbal_state *state = machine.driver_data<cyberbal_state>();
	machine.device("dac")->execute().set_input_line(6, state->m_fast_68k_int ? ASSERT_LINE : CLEAR_LINE);
	machine.device("dac")->execute().set_input_line(2, state->m_io_68k_int   ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(cyberbal_state::cyberbal_sound_68k_irq_gen)
{
	if (!m_fast_68k_int)
	{
		m_fast_68k_int = 1;
		update_sound_68k_interrupts(machine());
	}
}


WRITE16_MEMBER(cyberbal_state::cyberbal_io_68k_irq_ack_w)
{
	if (m_io_68k_int)
	{
		m_io_68k_int = 0;
		update_sound_68k_interrupts(machine());
	}
}


READ16_MEMBER(cyberbal_state::cyberbal_sound_68k_r)
{
	int temp = (m_sound_data_from_6502 << 8) | 0xff;

	m_sound_data_from_6502_ready = 0;

	if (m_sound_data_from_6502_ready) temp ^= 0x08;
	if (m_sound_data_from_68k_ready) temp ^= 0x04;
	return temp;
}


WRITE16_MEMBER(cyberbal_state::cyberbal_sound_68k_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_sound_data_from_68k = (data >> 8) & 0xff;
		m_sound_data_from_68k_ready = 1;
	}
}


WRITE16_MEMBER(cyberbal_state::cyberbal_sound_68k_dac_w)
{
	dac_device *dac = machine().device<dac_device>((offset & 8) ? "dac2" : "dac1");
	dac->write_unsigned16((((data >> 3) & 0x800) | ((data >> 2) & 0x7ff)) << 4);

	if (m_fast_68k_int)
	{
		m_fast_68k_int = 0;
		update_sound_68k_interrupts(machine());
	}
}

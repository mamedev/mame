// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cyberball 68000 sound simulator

****************************************************************************/

#include "emu.h"
#include "sound/2151intf.h"
#include "machine/atarigen.h"
#include "includes/cyberbal.h"


void cyberbal_state::cyberbal_sound_reset()
{
	/* reset the sound system */
	m_bank_base = &memregion("audiocpu")->base()[0x10000];
	membank("soundbank")->set_base(&m_bank_base[0x0000]);
	m_fast_68k_int = m_io_68k_int = 0;
	m_sound_data_from_68k = m_sound_data_from_6502 = 0;
	m_sound_data_from_68k_ready = m_sound_data_from_6502_ready = 0;
}



/*************************************
 *
 *  6502 Sound Interface
 *
 *************************************/

READ8_MEMBER(cyberbal_state::special_port3_r)
{
	int temp = ioport("jsa:JSAII")->read();
	if (!(ioport("IN0")->read() & 0x8000)) temp ^= 0x80;
	if (m_soundcomm->main_to_sound_ready()) temp ^= 0x40;
	if (m_soundcomm->sound_to_main_ready()) temp ^= 0x20;
	return temp;
}


READ8_MEMBER(cyberbal_state::sound_6502_stat_r)
{
	int temp = 0xff;
	if (m_sound_data_from_6502_ready) temp ^= 0x80;
	if (m_sound_data_from_68k_ready) temp ^= 0x40;
	return temp;
}


WRITE8_MEMBER(cyberbal_state::sound_bank_select_w)
{
	membank("soundbank")->set_base(&m_bank_base[0x1000 * ((data >> 6) & 3)]);
	coin_counter_w(machine(), 1, (data >> 5) & 1);
	coin_counter_w(machine(), 0, (data >> 4) & 1);
	m_daccpu->set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x01)) machine().device("ymsnd")->reset();
}


READ8_MEMBER(cyberbal_state::sound_68k_6502_r)
{
	m_sound_data_from_68k_ready = 0;
	return m_sound_data_from_68k;
}


WRITE8_MEMBER(cyberbal_state::sound_68k_6502_w)
{
	m_sound_data_from_6502 = data;
	m_sound_data_from_6502_ready = 1;

	if (!m_io_68k_int)
	{
		m_io_68k_int = 1;
		update_sound_68k_interrupts();
	}
}



/*************************************
 *
 *  68000 Sound Interface
 *
 *************************************/

void cyberbal_state::update_sound_68k_interrupts()
{
	m_daccpu->set_input_line(6, m_fast_68k_int ? ASSERT_LINE : CLEAR_LINE);
	m_daccpu->set_input_line(2, m_io_68k_int   ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(cyberbal_state::sound_68k_irq_gen)
{
	if (!m_fast_68k_int)
	{
		m_fast_68k_int = 1;
		update_sound_68k_interrupts();
	}
}


WRITE16_MEMBER(cyberbal_state::io_68k_irq_ack_w)
{
	if (m_io_68k_int)
	{
		m_io_68k_int = 0;
		update_sound_68k_interrupts();
	}
}


READ16_MEMBER(cyberbal_state::sound_68k_r)
{
	int temp = (m_sound_data_from_6502 << 8) | 0xff;

	m_sound_data_from_6502_ready = 0;

	if (m_sound_data_from_6502_ready) temp ^= 0x08;
	if (m_sound_data_from_68k_ready) temp ^= 0x04;
	return temp;
}


WRITE16_MEMBER(cyberbal_state::sound_68k_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_sound_data_from_68k = (data >> 8) & 0xff;
		m_sound_data_from_68k_ready = 1;
	}
}


WRITE16_MEMBER(cyberbal_state::sound_68k_dac_w)
{
	dac_device *dac = (offset & 8) ? m_dac2 : m_dac1;
	dac->write_unsigned16((((data >> 3) & 0x800) | ((data >> 2) & 0x7ff)) << 4);

	if (m_fast_68k_int)
	{
		m_fast_68k_int = 0;
		update_sound_68k_interrupts();
	}
}

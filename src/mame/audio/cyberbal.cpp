// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cyberball 68000 sound simulator

****************************************************************************/

#include "emu.h"
#include "sound/ym2151.h"
#include "machine/atarigen.h"
#include "includes/cyberbal.h"


void cyberbal_state::cyberbal_sound_reset()
{
	/* reset the sound system */
	membank("soundbank")->set_entry(0);
	m_fast_68k_int = m_io_68k_int = 0;
	m_sound_data_from_68k = m_sound_data_from_6502 = 0;
	m_sound_data_from_68k_ready = m_sound_data_from_6502_ready = 0;
}



/*************************************
 *
 *  6502 Sound Interface
 *
 *************************************/

uint8_t cyberbal_state::special_port3_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	int temp = ioport("jsa:JSAII")->read();
	if (!(ioport("IN0")->read() & 0x8000)) temp ^= 0x80;
	if (m_soundcomm->main_to_sound_ready()) temp ^= 0x40;
	if (m_soundcomm->sound_to_main_ready()) temp ^= 0x20;
	return temp;
}


uint8_t cyberbal_state::sound_6502_stat_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	int temp = 0xff;
	if (m_sound_data_from_6502_ready) temp ^= 0x80;
	if (m_sound_data_from_68k_ready) temp ^= 0x40;
	return temp;
}


void cyberbal_state::sound_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	membank("soundbank")->set_entry((data >> 6) & 3);
	machine().bookkeeping().coin_counter_w(1, (data >> 5) & 1);
	machine().bookkeeping().coin_counter_w(0, (data >> 4) & 1);
	m_daccpu->set_input_line(INPUT_LINE_RESET, (data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
	if (!(data & 0x01)) machine().device("ymsnd")->reset();
}


uint8_t cyberbal_state::sound_68k_6502_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_sound_data_from_68k_ready = 0;
	return m_sound_data_from_68k;
}


void cyberbal_state::sound_68k_6502_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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


void cyberbal_state::sound_68k_irq_gen(device_t &device)
{
	if (!m_fast_68k_int)
	{
		m_fast_68k_int = 1;
		update_sound_68k_interrupts();
	}
}


void cyberbal_state::io_68k_irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_io_68k_int)
	{
		m_io_68k_int = 0;
		update_sound_68k_interrupts();
	}
}


uint16_t cyberbal_state::sound_68k_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	int temp = (m_sound_data_from_6502 << 8) | 0xff;

	m_sound_data_from_6502_ready = 0;

	if (m_sound_data_from_6502_ready) temp ^= 0x08;
	if (m_sound_data_from_68k_ready) temp ^= 0x04;
	return temp;
}


void cyberbal_state::sound_68k_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_sound_data_from_68k = (data >> 8) & 0xff;
		m_sound_data_from_68k_ready = 1;
	}
}


void cyberbal_state::sound_68k_dac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//int clip = BIT(data, 15);
	//int off0b = BIT(data, 13) | BIT(data, 14);
	//int off4b = BIT(data, 13) & BIT(data, 14);
	uint16 sample = ((data >> 3) & 0x800) | ((data >> 2) & 0x7ff);

	if (offset & 8)
		m_ldac->write(sample);
	else
		m_rdac->write(sample);

	if (m_fast_68k_int)
	{
		m_fast_68k_int = 0;
		update_sound_68k_interrupts();
	}
}

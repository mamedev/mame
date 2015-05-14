// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

****************************************************************************/


#include "includes/llc.h"


// LLC1 BASIC keyboard
READ8_MEMBER(llc_state::llc1_port2_b_r)
{
	UINT8 retVal = 0;

	if (m_term_status)
	{
		retVal = m_term_status;
		m_term_status = 0;
	}
	else
		retVal = m_term_data;

	return retVal;
}

READ8_MEMBER(llc_state::llc1_port2_a_r)
{
	return 0;
}

// LLC1 Monitor keyboard
READ8_MEMBER(llc_state::llc1_port1_a_r)
{
	UINT8 data = 0;
	if (!BIT(m_porta, 4))
		data = ioport("X4")->read();
	if (!BIT(m_porta, 5))
		data = ioport("X5")->read();
	if (!BIT(m_porta, 6))
		data = ioport("X6")->read();
	if (data & 0xf0)
		data = (data >> 4) | 0x80;

	data |= (m_porta & 0x70);

	// do not repeat key
	if (data & 15)
	{
		if (data == m_llc1_key)
			data &= 0x70;
		else
			m_llc1_key = data;
	}
	else
	if ((data & 0x70) == (m_llc1_key & 0x70))
		m_llc1_key = 0;

	return data;
}

WRITE8_MEMBER(llc_state::llc1_port1_a_w)
{
	m_porta = data;
}

WRITE8_MEMBER(llc_state::llc1_port1_b_w)
{
	static UINT8 count = 0, digit = 0;

	if (data == 0)
	{
		digit = 0;
		count = 0;
	}
	else
		count++;

	if (count == 1)
		output_set_digit_value(digit, data & 0x7f);
	else
	if (count == 3)
	{
		count = 0;
		digit++;
	}
}

DRIVER_INIT_MEMBER(llc_state,llc1)
{
}

MACHINE_RESET_MEMBER(llc_state,llc1)
{
	m_term_status = 0;
	m_llc1_key = 0;
}

MACHINE_START_MEMBER(llc_state,llc1)
{
}

/* Driver initialization */
DRIVER_INIT_MEMBER(llc_state,llc2)
{
	m_p_videoram.set_target( m_ram->pointer() + 0xc000,m_p_videoram.bytes());
}

MACHINE_RESET_MEMBER(llc_state,llc2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.unmap_write(0x0000, 0x3fff);
	membank("bank1")->set_base(memregion("maincpu")->base());

	space.unmap_write(0x4000, 0x5fff);
	membank("bank2")->set_base(memregion("maincpu")->base() + 0x4000);

	space.unmap_write(0x6000, 0xbfff);
	membank("bank3")->set_base(memregion("maincpu")->base() + 0x6000);

	space.install_write_bank(0xc000, 0xffff, "bank4");
	membank("bank4")->set_base(m_ram->pointer() + 0xc000);

}

WRITE8_MEMBER(llc_state::llc2_rom_disable_w)
{
	address_space &mem_space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	mem_space.install_write_bank(0x0000, 0xbfff, "bank1");
	membank("bank1")->set_base(ram);

	mem_space.install_write_bank(0x4000, 0x5fff, "bank2");
	membank("bank2")->set_base(ram + 0x4000);

	mem_space.install_write_bank(0x6000, 0xbfff, "bank3");
	membank("bank3")->set_base(ram + 0x6000);

	mem_space.install_write_bank(0xc000, 0xffff, "bank4");
	membank("bank4")->set_base(ram + 0xc000);

}

WRITE8_MEMBER(llc_state::llc2_basic_enable_w)
{
	address_space &mem_space = m_maincpu->space(AS_PROGRAM);
	if (data & 0x02)
	{
		mem_space.unmap_write(0x4000, 0x5fff);
		membank("bank2")->set_base(memregion("maincpu")->base() + 0x10000);
	}
	else
	{
		mem_space.install_write_bank(0x4000, 0x5fff, "bank2");
		membank("bank2")->set_base(m_ram->pointer() + 0x4000);
	}

}

READ8_MEMBER(llc_state::llc2_port1_b_r)
{
	return 0;
}

WRITE8_MEMBER(llc_state::llc2_port1_b_w)
{
	m_speaker->level_w(BIT(data, 6));
	m_rv = BIT(data, 5);
}

READ8_MEMBER(llc_state::llc2_port2_a_r)
{
	return 0; // bit 2 low or hangs on ^Z^X^C sequence
}

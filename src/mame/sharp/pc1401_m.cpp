// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "pocketc.h"
#include "pc1401.h"
#include "machine/ram.h"

/* C-CE while reset, program will not be destroyed! */

/* error codes
1 syntax error
2 calculation error
3 illegal function argument
4 too large a line number
5 next without for
  return without gosub
6 memory overflow
7 print using error
8 i/o device error
9 other errors*/

void pc1401_state::out_b_w(uint8_t data)
{
	m_outb = data;
}

void pc1401_state::out_c_w(uint8_t data)
{
	m_portc = data;
}

uint8_t pc1401_state::in_a_r()
{
	int data = m_outa;

	for (int bit = 0; bit < 5; bit++)
		if (BIT(m_outb, bit))
			data |= m_keys[bit]->read();

	if (m_outb & 0x20)
	{
		data |= m_keys[5]->read();

		/* At Power Up we fake a 'C-CE' pressure */
		if (m_power)
			data |= 0x01;
	}

	for (int bit = 0, key = 6; bit < 7; bit++, key++)
		if (BIT(m_outa, bit))
			data |= m_keys[key]->read();

	return data;
}

uint8_t pc1401_state::in_b_r()
{
	int data = m_outb;

	if (BIT(m_extra->read(), 2))
		data |= 0x01;

	return data;
}

READ_LINE_MEMBER(pc1401_state::reset_r)
{
	return (m_extra->read() & 0x02);
}

void pc1401_state::machine_start()
{
	pocketc_state::machine_start();

	m_ram_nvram->set_base(memregion("maincpu")->base() + 0x2000, 0x2800);

	uint8_t *gfx = memregion("gfx1")->base();
	for (int i = 0; i < 128; i++)
		gfx[i] = i;
}

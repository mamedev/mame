// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "pocketc.h"
#include "pc1251.h"

/* C-CE while reset, program will not be destroyed! */

void pc1251_state::out_b_w(uint8_t data)
{
	m_outb = data;
}

void pc1251_state::out_c_w(uint8_t data)
{
}

uint8_t pc1251_state::in_a_r()
{
	int data = m_outa;

	if (BIT(m_outb, 0))
	{
		data |= m_keys[0]->read();

		/* At Power Up we fake a 'CL' pressure */
		if (m_power)
			data |= 0x02;       // problem with the deg lcd
	}

	if (BIT(m_outb, 1))
		data |= m_keys[1]->read();

	if (BIT(m_outb, 2))
		data |= m_keys[2]->read();

	for (int bit = 0, key = 3; bit < 7; bit++, key++)
		if (BIT(m_outa, bit))
			data |= m_keys[key]->read();

	return data;
}

uint8_t pc1251_state::in_b_r()
{
	int data = m_outb;

	if (BIT(m_outb, 3))
		data |= m_mode->read() & 0x07;

	return data;
}

READ_LINE_MEMBER(pc1251_state::reset_r)
{
	return BIT(m_extra->read(), 1);
}

void pc1251_state::machine_start()
{
	pocketc_state::machine_start();

	m_ram_nvram->set_base(memregion("maincpu")->base() + 0x8000, 0x4800);

	uint8_t *gfx = memregion("gfx1")->base();
	for (int i = 0; i < 128; i++)
		gfx[i] = i;
}

void pc1260_state::machine_start()
{
	pocketc_state::machine_start();

	m_ram_nvram->set_base(memregion("maincpu")->base() + 0x4000, 0x2800);

	uint8_t *gfx = memregion("gfx1")->base();
	for (int i = 0; i < 128; i++)
		gfx[i] = i;
}

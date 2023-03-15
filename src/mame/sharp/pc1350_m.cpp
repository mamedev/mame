// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "pocketc.h"
#include "pc1350.h"
#include "machine/ram.h"

void pc1350_state::out_b_w(uint8_t data)
{
	m_outb = data;
}

void pc1350_state::out_c_w(uint8_t data)
{
}

uint8_t pc1350_state::in_a_r()
{
	int data = m_outa;
	int t = keyboard_line_r();

	for (int bit = 0; bit < 6; bit++)
		if (BIT(t, bit))
			data |= m_keys[bit]->read();

	for (int bit = 0, key = 6; bit < 2; bit++, key++)
		if (BIT(m_outa, bit))
			data |= m_keys[key]->read();

	if (BIT(m_outa, 2))
	{
		data |= m_keys[8]->read();

		/* At Power Up we fake a 'CLS' pressure */
		if (m_power)
			data |= 0x08;
	}

	for (int bit = 3, key = 9; bit < 5; bit++, key++)
		if (BIT(m_outa, bit))
			data |= m_keys[key]->read();

	if (m_outa & 0xc0)
		data |= m_keys[11]->read();

	// missing lshift

	return data;
}

uint8_t pc1350_state::in_b_r()
{
	return m_outb;
}

void pc1350_state::machine_start()
{
	pocketc_state::machine_start();

	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_ram(0x6000, 0x6fff, &m_ram->pointer()[0x0000]);

	if (m_ram->size() >= 0x3000)
	{
		space.install_ram(0x4000, 0x5fff, &m_ram->pointer()[0x1000]);
	}
	else
	{
		space.nop_readwrite(0x4000, 0x5fff);
	}

	if (m_ram->size() >= 0x5000)
	{
		space.install_ram(0x2000, 0x3fff, &m_ram->pointer()[0x3000]);
	}
	else
	{
		space.nop_readwrite(0x2000, 0x3fff);
	}

	m_ram_nvram->set_base(memregion("maincpu")->base() + 0x2000, 0x5000);
}

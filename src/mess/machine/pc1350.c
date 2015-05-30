// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "includes/pocketc.h"
#include "includes/pc1350.h"
#include "machine/ram.h"



WRITE8_MEMBER(pc1350_state::pc1350_outa)
{
	m_outa=data;
}

WRITE8_MEMBER(pc1350_state::pc1350_outb)
{
	m_outb=data;
}

WRITE8_MEMBER(pc1350_state::pc1350_outc)
{
}

READ8_MEMBER(pc1350_state::pc1350_ina)
{
	int data = m_outa;
	int t = pc1350_keyboard_line_r(space,0);

	if (t & 0x01)
		data |= ioport("KEY0")->read();

	if (t & 0x02)
		data |= ioport("KEY1")->read();

	if (t & 0x04)
		data |= ioport("KEY2")->read();

	if (t & 0x08)
		data |= ioport("KEY3")->read();

	if (t & 0x10)
		data |= ioport("KEY4")->read();

	if (t & 0x20)
		data |= ioport("KEY5")->read();

	if (m_outa & 0x01)
		data |= ioport("KEY6")->read();

	if (m_outa & 0x02)
		data |= ioport("KEY7")->read();

	if (m_outa & 0x04)
	{
		data |= ioport("KEY8")->read();

		/* At Power Up we fake a 'CLS' pressure */
		if (m_power)
			data |= 0x08;
	}

	if (m_outa & 0x08)
		data |= ioport("KEY9")->read();

	if (m_outa & 0x10)
		data |= ioport("KEY10")->read();

	if (m_outa & 0xc0)
		data |= ioport("KEY11")->read();

	// missing lshift

	return data;
}

READ8_MEMBER(pc1350_state::pc1350_inb)
{
	return m_outb;
}

READ_LINE_MEMBER(pc1350_state::pc1350_brk)
{
	return (ioport("EXTRA")->read() & 0x01);
}

void pc1350_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POWER_UP:
		m_power=0;
		break;
	default:
		assert_always(FALSE, "Unknown id in pc1350_state::device_timer");
	}
}

void pc1350_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_power = 1;
	timer_set(attotime::from_seconds(1), TIMER_POWER_UP);

	space.install_readwrite_bank(0x6000, 0x6fff, "bank1");
	membank("bank1")->set_base(&m_ram->pointer()[0x0000]);

	if (m_ram->size() >= 0x3000)
	{
		space.install_readwrite_bank(0x4000, 0x5fff, "bank2");
		membank("bank2")->set_base(&m_ram->pointer()[0x1000]);
	}
	else
	{
		space.nop_readwrite(0x4000, 0x5fff);
	}

	if (m_ram->size() >= 0x5000)
	{
		space.install_readwrite_bank(0x2000, 0x3fff, "bank3");
		membank("bank3")->set_base(&m_ram->pointer()[0x3000]);
	}
	else
	{
		space.nop_readwrite(0x2000, 0x3fff);
	}

	UINT8 *ram = memregion("maincpu")->base() + 0x2000;
	UINT8 *cpu = m_maincpu->internal_ram();

	machine().device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine().device<nvram_device>("ram_nvram")->set_base(ram, 0x5000);
}

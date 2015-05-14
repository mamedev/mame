// license:GPL-2.0+
// copyright-holders:Peter Trauner
#include "emu.h"
#include "cpu/sc61860/sc61860.h"

#include "includes/pocketc.h"
#include "includes/pc1251.h"

/* C-CE while reset, program will not be destroyed! */



WRITE8_MEMBER(pc1251_state::pc1251_outa)
{
	m_outa = data;
}

WRITE8_MEMBER(pc1251_state::pc1251_outb)
{
	m_outb = data;
}

WRITE8_MEMBER(pc1251_state::pc1251_outc)
{
}

READ8_MEMBER(pc1251_state::pc1251_ina)
{
	int data = m_outa;

	if (m_outb & 0x01)
	{
		data |= ioport("KEY0")->read();

		/* At Power Up we fake a 'CL' pressure */
		if (m_power)
			data |= 0x02;       // problem with the deg lcd
	}

	if (m_outb & 0x02)
		data |= ioport("KEY1")->read();

	if (m_outb & 0x04)
		data |= ioport("KEY2")->read();

	if (m_outa & 0x01)
		data |= ioport("KEY3")->read();

	if (m_outa & 0x02)
		data |= ioport("KEY4")->read();

	if (m_outa & 0x04)
		data |= ioport("KEY5")->read();

	if (m_outa & 0x08)
		data |= ioport("KEY6")->read();

	if (m_outa & 0x10)
		data |= ioport("KEY7")->read();

	if (m_outa & 0x20)
		data |= ioport("KEY8")->read();

	if (m_outa & 0x40)
		data |= ioport("KEY9")->read();

	return data;
}

READ8_MEMBER(pc1251_state::pc1251_inb)
{
	int data = m_outb;

	if (m_outb & 0x08)
		data |= (ioport("MODE")->read() & 0x07);

	return data;
}

READ_LINE_MEMBER(pc1251_state::pc1251_brk)
{
	return (ioport("EXTRA")->read() & 0x01);
}

READ_LINE_MEMBER(pc1251_state::pc1251_reset)
{
	return (ioport("EXTRA")->read() & 0x02);
}

void pc1251_state::machine_start()
{
	UINT8 *ram = memregion("maincpu")->base() + 0x8000;
	UINT8 *cpu = m_maincpu->internal_ram();

	machine().device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine().device<nvram_device>("ram_nvram")->set_base(ram, 0x4800);
}

MACHINE_START_MEMBER(pc1251_state,pc1260 )
{
	UINT8 *ram = memregion("maincpu")->base() + 0x4000;
	UINT8 *cpu = m_maincpu->internal_ram();

	machine().device<nvram_device>("cpu_nvram")->set_base(cpu, 96);
	machine().device<nvram_device>("ram_nvram")->set_base(ram, 0x2800);
}

void pc1251_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POWER_UP:
		m_power = 0;
		break;
	default:
		assert_always(FALSE, "Unknown id in pc1251_state::device_timer");
	}
}

DRIVER_INIT_MEMBER(pc1251_state,pc1251)
{
	int i;
	UINT8 *gfx = memregion("gfx1")->base();
	for (i=0; i<128; i++) gfx[i]=i;

	m_power = 1;
	timer_set(attotime::from_seconds(1), TIMER_POWER_UP);
}

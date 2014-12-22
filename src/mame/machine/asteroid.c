/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "machine/atari_vg.h"
#include "video/avgdvg.h"
#include "includes/asteroid.h"


INTERRUPT_GEN_MEMBER(asteroid_state::asteroid_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if (!(ioport("IN0")->read() & 0x80))
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(asteroid_state::asterock_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if ((ioport("IN0")->read() & 0x80))
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(asteroid_state::llander_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if (ioport("IN0")->read() & 0x02)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER(asteroid_state::asteroid_IN0_r)
{
	int res = ioport("IN0")->read();
	int bitmask = (1 << offset);

	if (res & bitmask)
		res = 0x80;
	else
		res = ~0x80;

	return res;
}


READ8_MEMBER(asteroid_state::asterock_IN0_r)
{
	int res = ioport("IN0")->read();
	int bitmask = (1 << offset);

	if (res & bitmask)
		res = ~0x80;
	else
		res = 0x80;

	return res;
}

/*
 * These 7 memory locations are used to read the player's controls.
 * Typically, only the high bit is used. This is handled by one input port.
 */

READ8_MEMBER(asteroid_state::asteroid_IN1_r)
{
	int res = ioport("IN1")->read();
	int bitmask = (1 << (offset & 0x7));

	if (res & bitmask)
		res = 0x80;
	else
		res = ~0x80;

	return res;
}


READ8_MEMBER(asteroid_state::asteroid_DSW1_r)
{
	int res;
	int res1;

	res1 = ioport("DSW1")->read();

	res = 0xfc | ((res1 >> (2 * (3 - (offset & 0x3)))) & 0x3);
	return res;
}


WRITE8_MEMBER(asteroid_state::asteroid_bank_switch_w)
{
	if (data & 4)
	{
		membank("bank1")->set_base(m_ram2);
		membank("bank2")->set_base(m_ram1);
	}
	else
	{
		membank("bank1")->set_base(m_ram1);
		membank("bank2")->set_base(m_ram2);
	}

	set_led_status (machine(), 0, ~data & 0x02);
	set_led_status (machine(), 1, ~data & 0x01);
}


WRITE8_MEMBER(asteroid_state::astdelux_bank_switch_w)
{
	m_astdelux_bankswitch = data;
	astdelux_bankswitch_restore();
}

void asteroid_state::astdelux_bankswitch_restore()
{
	if (m_astdelux_bankswitch & 0x80)
	{
		membank("bank1")->set_base(m_ram2);
		membank("bank2")->set_base(m_ram1);
	}
	else
	{
		membank("bank1")->set_base(m_ram1);
		membank("bank2")->set_base(m_ram2);
	}
}

WRITE8_MEMBER(asteroid_state::astdelux_led_w)
{
	set_led_status(machine(), offset, (data & 0x80) ? 0 : 1);
}

void asteroid_state::machine_reset()
{
	asteroid_bank_switch_w(m_maincpu->space(AS_PROGRAM), 0, 0);
	m_dvg->reset_w(m_maincpu->space(AS_PROGRAM), 0, 0);
}

MACHINE_RESET_MEMBER(asteroid_state, llander)
{
	m_dvg->reset_w(m_maincpu->space(AS_PROGRAM), 0, 0);
}

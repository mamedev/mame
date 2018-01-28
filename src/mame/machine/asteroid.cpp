// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
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
	// 765432--  not used
	// ------1-  ls253 dsw selector 2y
	// -------0  ls253 dsw selector 1y

	uint8_t val = m_dsw1->read();

	m_dsw_sel->i3a_w(BIT(val, 0));
	m_dsw_sel->i3b_w(BIT(val, 1));
	m_dsw_sel->i2a_w(BIT(val, 2));
	m_dsw_sel->i2b_w(BIT(val, 3));
	m_dsw_sel->i1a_w(BIT(val, 4));
	m_dsw_sel->i1b_w(BIT(val, 5));
	m_dsw_sel->i0a_w(BIT(val, 6));
	m_dsw_sel->i0b_w(BIT(val, 7));

	m_dsw_sel->s_w(space, 0, offset & 0x03);

	return 0xfc | (m_dsw_sel->zb_r() << 1) | m_dsw_sel->za_r();
}


WRITE8_MEMBER(asteroid_state::asteroid_bank_switch_w)
{
	// 76------  not used
	// --5-----  coin counter right coin
	// ---4----  coin counter center coin
	// ----3---  coin counter left coin
	// -----2--  ramsel
	// ------1-  start2 led
	// -------0  start1 led

	start1_led_w(BIT(data, 0));
	start2_led_w(BIT(data, 1));

	int bank = BIT(data, 2);
	m_ram1->set_entry(bank);
	m_ram2->set_entry(bank);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 4));
	machine().bookkeeping().coin_counter_w(2, BIT(data, 5));
}


WRITE_LINE_MEMBER(asteroid_state::start1_led_w)
{
	output().set_led_value(0, state ? 0 : 1);
}

WRITE_LINE_MEMBER(asteroid_state::start2_led_w)
{
	output().set_led_value(1, state ? 0 : 1);
}

void asteroid_state::machine_start()
{
	/* configure RAM banks if present (not on llander) */
	if (m_ram1.target() != nullptr)
	{
		uint8_t *ram1 = reinterpret_cast<uint8_t *>(memshare("ram1")->ptr());
		uint8_t *ram2 = reinterpret_cast<uint8_t *>(memshare("ram2")->ptr());

		/* swapped */
		m_ram1->configure_entry(1, ram2);
		m_ram2->configure_entry(1, ram1);
		/* normal */
		m_ram1->configure_entry(0, ram1);
		m_ram2->configure_entry(0, ram2);
	}
}

void asteroid_state::machine_reset()
{
	m_dvg->reset_w(m_maincpu->space(AS_PROGRAM), 0, 0);

	/* reset RAM banks if present */
	if (m_ram1.target() != nullptr)
	{
		m_ram1->set_entry(0);
		m_ram2->set_entry(0);
	}
}

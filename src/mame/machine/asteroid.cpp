// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "video/avgdvg.h"
#include "includes/asteroid.h"


INTERRUPT_GEN_MEMBER(asteroid_state::asteroid_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if (!(ioport("IN0")->read() & 0x80))
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(asteroid_state::asterock_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if ((ioport("IN0")->read() & 0x80))
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INTERRUPT_GEN_MEMBER(asteroid_state::llander_interrupt)
{
	/* Turn off interrupts if self-test is enabled */
	if (ioport("IN0")->read() & 0x02)
		device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE_LINE_MEMBER(asteroid_state::cocktail_inv_w)
{
	// Inverter circuit is only hooked up for Cocktail Asteroids
	int flip = state && m_cocktail->read();
	m_dvg->set_flip_x(flip);
	m_dvg->set_flip_y(flip);
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
	m_dvg->reset_w();
	if (m_earom.found())
		earom_control_w(machine().dummy_space(), 0, 0);

	/* reset RAM banks if present */
	if (m_ram1.target() != nullptr)
	{
		m_ram1->set_entry(0);
		m_ram2->set_entry(0);
	}
}

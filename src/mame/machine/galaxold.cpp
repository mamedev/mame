// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "includes/galaxold.h"


IRQ_CALLBACK_MEMBER(galaxold_state::hunchbkg_irq_callback)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0x03;
}

/* FIXME: remove trampoline */
WRITE_LINE_MEMBER(galaxold_state::galaxold_7474_9m_2_q_callback)
{
	/* Q bar clocks the other flip-flop,
	   Q is VBLANK (not visible to the CPU) */
	m_7474_9m_1->clock_w(state);
}

WRITE_LINE_MEMBER(galaxold_state::galaxold_7474_9m_1_callback)
{
	/* Q goes to the NMI line */
	m_maincpu->set_input_line(m_irq_line, state ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_MEMBER(galaxold_state::galaxold_nmi_enable_w)
{
	m_7474_9m_1->preset_w(data ? 1 : 0);
}


TIMER_DEVICE_CALLBACK_MEMBER(galaxold_state::galaxold_interrupt_timer)
{
	/* 128V, 64V and 32V go to D */
	m_7474_9m_2->d_w(((param & 0xe0) != 0xe0) ? 1 : 0);

	/* 16V clocks the flip-flop */
	m_7474_9m_2->clock_w(((param & 0x10) == 0x10) ? 1 : 0);

	param = (param + 0x10) & 0xff;

	timer.adjust(m_screen->time_until_pos(param), param);
}


void galaxold_state::machine_reset_common(int line)
{
	m_irq_line = line;

	/* initalize main CPU interrupt generator flip-flops */
	m_7474_9m_2->preset_w(1);
	m_7474_9m_2->clear_w (1);

	m_7474_9m_1->clear_w (1);
	m_7474_9m_1->d_w     (0);
	m_7474_9m_1->preset_w(0);

	/* start a timer to generate interrupts */
	timer_device *int_timer = machine().device<timer_device>("int_timer");
	int_timer->adjust(m_screen->time_until_pos(0));
}

MACHINE_RESET_MEMBER(galaxold_state,galaxold)
{
	machine_reset_common(INPUT_LINE_NMI);
}

MACHINE_RESET_MEMBER(galaxold_state,devilfsg)
{
	machine_reset_common(0);
}

MACHINE_RESET_MEMBER(galaxold_state,hunchbkg)
{
	machine_reset_common(0);
}

WRITE8_MEMBER(galaxold_state::galaxold_coin_lockout_w)
{
	machine().bookkeeping().coin_lockout_global_w(~data & 1);
}


WRITE8_MEMBER(galaxold_state::galaxold_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset, data & 0x01);
}

WRITE8_MEMBER(galaxold_state::galaxold_coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
}

WRITE8_MEMBER(galaxold_state::galaxold_coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(2, data & 0x01);
}


WRITE8_MEMBER(galaxold_state::galaxold_leds_w)
{
	output().set_led_value(offset,data & 1);
}

READ8_MEMBER(galaxold_state::scramblb_protection_1_r)
{
	switch (space.device().safe_pc())
	{
	case 0x01da: return 0x80;
	case 0x01e4: return 0x00;
	default:
		logerror("%04x: read protection 1\n",space.device().safe_pc());
		return 0;
	}
}

READ8_MEMBER(galaxold_state::scramblb_protection_2_r)
{
	switch (space.device().safe_pc())
	{
	case 0x01ca: return 0x90;
	default:
		logerror("%04x: read protection 2\n",space.device().safe_pc());
		return 0;
	}
}


WRITE8_MEMBER(galaxold_state::_4in1_bank_w)
{
	m__4in1_bank = data & 0x03;
	galaxold_gfxbank_w(space, 0, m__4in1_bank);
	membank("bank1")->set_entry(m__4in1_bank);
}

CUSTOM_INPUT_MEMBER(galaxold_state::_4in1_fake_port_r)
{
	static const char *const portnames[] = { "FAKE1", "FAKE2", "FAKE3", "FAKE4" };
	int bit_mask = (FPTR)param;

	return (ioport(portnames[m__4in1_bank])->read() & bit_mask) ? 0x01 : 0x00;
}

DRIVER_INIT_MEMBER(galaxold_state,4in1)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	offs_t i, len = memregion("maincpu")->bytes();
	UINT8 *RAM = memregion("maincpu")->base();

	/* Decrypt Program Roms */
	for (i = 0; i < len; i++)
		RAM[i] = RAM[i] ^ (i & 0xff);

	/* games are banked at 0x0000 - 0x3fff */
	membank("bank1")->configure_entries(0, 4, &RAM[0x10000], 0x4000);

	_4in1_bank_w(space, 0, 0); /* set the initial CPU bank */

	save_item(NAME(m__4in1_bank));
}

INTERRUPT_GEN_MEMBER(galaxold_state::hunchbks_vh_interrupt)
{
	generic_pulse_irq_line_and_vector(device.execute(),0,0x03,1);
}

DRIVER_INIT_MEMBER(galaxold_state,ladybugg)
{
	/* Doesn't actually use the bank, but it mustn't have a coin lock! */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x6002, 0x6002, write8_delegate(FUNC(galaxold_state::galaxold_gfxbank_w),this));
}

DRIVER_INIT_MEMBER(galaxold_state,bullsdrtg)
{
	int i;

	// patch char supposed to be space
	UINT8 *gfxrom = memregion("gfx1")->base();
	for (i = 0; i < 8; i++)
	{
		gfxrom[i] = 0;
	}

	// patch gfx for charset (seems to be 1bpp with bitplane data in correct rom)
	for (i = 0*8; i < 27*8; i++)
	{
		gfxrom[0x1000 + i] = 0;
	}

	// patch gfx for digits (seems to be 1bpp with bitplane data in correct rom)
	for (i = 48*8; i < (48+10)*8; i++ )
	{
		gfxrom[0x1000 + i] = 0;
	}
}

// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  galaxold.cpp

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "galaxold.h"


uint8_t galaxold_state::hunchbkg_intack()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0x03;
}

/* FIXME: remove trampoline */
void galaxold_state::galaxold_7474_9m_2_q_callback(int state)
{
	/* Q bar clocks the other flip-flop,
	   Q is VBLANK (not visible to the CPU) */
	m_7474_9m_1->clock_w(state);
}

void galaxold_state::galaxold_7474_9m_1_callback(int state)
{
	/* Q goes to the NMI line */
	m_maincpu->set_input_line(m_irq_line, state ? CLEAR_LINE : ASSERT_LINE);
}

void galaxold_state::galaxold_nmi_enable_w(uint8_t data)
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

	/* initialize main CPU interrupt generator flip-flops */
	m_7474_9m_2->preset_w(1);
	m_7474_9m_2->clear_w (1);

	m_7474_9m_1->clear_w (1);
	m_7474_9m_1->d_w     (0);
	m_7474_9m_1->preset_w(0);

	/* start a timer to generate interrupts */
	subdevice<timer_device>("int_timer")->adjust(m_screen->time_until_pos(0));
}

MACHINE_RESET_MEMBER(galaxold_state,galaxold)
{
	machine_reset_common(INPUT_LINE_NMI);
}

MACHINE_RESET_MEMBER(galaxold_state,hunchbkg)
{
	machine_reset_common(0);
}

void galaxold_state::galaxold_coin_lockout_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_global_w(~data & 1);
}


void galaxold_state::galaxold_coin_counter_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_counter_w(offset, data & 0x01);
}

void galaxold_state::galaxold_coin_counter_1_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(1, data & 0x01);
}

void galaxold_state::galaxold_coin_counter_2_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(2, data & 0x01);
}


void galaxold_state::galaxold_leds_w(offs_t offset, uint8_t data)
{
	m_leds[offset] = BIT(data, 0);
}

uint8_t galaxold_state::scramblb_protection_1_r()
{
	switch (m_maincpu->pc())
	{
	case 0x01da: return 0x80;
	case 0x01e4: return 0x00;
	default:
		logerror("%04x: read protection 1\n",m_maincpu->pc());
		return 0;
	}
}

uint8_t galaxold_state::scramblb_protection_2_r()
{
	switch (m_maincpu->pc())
	{
	case 0x01ca: return 0x90;
	default:
		logerror("%04x: read protection 2\n",m_maincpu->pc());
		return 0;
	}
}


void galaxold_state::_4in1_bank_w(uint8_t data)
{
	m_4in1_bank = data & 0x03;
	galaxold_gfxbank_w(0, m_4in1_bank);
	membank("bank1")->set_entry(m_4in1_bank);
}

void galaxold_state::init_4in1()
{
	const offs_t len = memregion("maincpu")->bytes();
	uint8_t *RAM = memregion("maincpu")->base();

	/* Decrypt Program Roms */
	for (offs_t i = 0; i < len; i++)
		RAM[i] = RAM[i] ^ (i & 0xff);

	/* games are banked at 0x0000 - 0x3fff */
	membank("bank1")->configure_entries(0, 4, &RAM[0x10000], 0x4000);

	std::fill(std::begin(m_gfxbank), std::end(m_gfxbank), 0);

	_4in1_bank_w(0); /* set the initial CPU bank */

	save_item(NAME(m_4in1_bank));
}

void galaxold_state::init_bullsdrtg()
{
	// patch char supposed to be space
	uint8_t *gfxrom = memregion("gfx1")->base();
	for (int i = 0; i < 8; i++)
	{
		gfxrom[i] = 0;
	}

	// patch gfx for charset (seems to be 1bpp with bitplane data in correct rom)
	for (int i = 0*8; i < 27*8; i++)
	{
		gfxrom[0x1000 + i] = 0;
	}

	// patch gfx for digits (seems to be 1bpp with bitplane data in correct rom)
	for (int i = 48*8; i < (48+10)*8; i++ )
	{
		gfxrom[0x1000 + i] = 0;
	}
}

void galaxold_state::init_superbikg()
{
	save_item(NAME(m_superbikg_latch));
}

void galaxold_state::init_dkingjrv()
{
	uint8_t *rom = memregion("maincpu")->base();

	membank("bank1")->configure_entry(0, &rom[0x8000]);
	membank("bank1")->configure_entry(1, &rom[0x4000]);

	membank("bank2")->configure_entry(0, &rom[0xa000]);
	membank("bank2")->configure_entry(1, &rom[0x6000]);
}

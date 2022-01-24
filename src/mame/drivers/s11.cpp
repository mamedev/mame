// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/****************************************************************************************

    Pinball
    Williams System 11

    Status of games:


ToDo:
- tic-tac-strike belongs in s11a.cpp
- gold mine belongs in s11b.cpp
- top dawg belongs in s11b.cpp
- shuffle inn belongs in s11b.cpp

    Known keys necessary to get games to start (so the proper number of balls are detected):
    - Grand Lizard: Press S, D, F, and press "1" after inserting 1 or more credits.
    - Road Kings: press 'Up' (the direction key) and 'Delete', and press "1" after inserting 1 or more credits.
    - High Speed: press D (ball trough center), F (ball trough lower right) and Enter (Ball Shooter) after inserting 1 or more credits.

*****************************************************************************************/

#include "emu.h"
#include "includes/s11.h"

#include "cpu/m6809/m6809.h"
#include "speaker.h"

#include "s11.lh"

/*
15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 RW MEMPROT
0  0  0  0  0  x  x  x  0  *  *  *  *  *  *  *  *  * battery backed RAM RW normal
0  0  0  0  0  x  x  0  x  *  *  *  *  *  *  *  *  * battery backed RAM RW normal
0  0  0  0  0  x  0  x  x  *  *  *  *  *  *  *  *  * battery backed RAM RW normal
0  0  0  0  0  0  x  x  x  *  *  *  *  *  *  *  *  * battery backed RAM RW normal
0  0  0  0  0  1  1  1  1  *  *  *  *  *  *  *  *  0 battery backed RAM RW in protected area, but protection switch is off
0  0  0  0  0  1  1  1  1  *  *  *  *  *  *  *  R  1 battery backed RAM Read in protected area, but protection switch is on, read is allowed
0  0  0  0  0  1  1  1  1  *  *  *  *  *  *  *  W  1 battery backed RAM Write in protected area, but protection switch is on, write is BLOCKED
*/
void s11_state::s11_main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	// TODO:
	//map(0x0000, 0x077f).ram().share("nvram"); // unprotected ram area
	//map(0x0780, 0x07ff).rw(FUNC(prot_ram_r), FUNC(prot_ram_w)); // protected ram area
	map(0x2100, 0x2103).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).w(FUNC(s11_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x2c00, 0x2c03).rw(m_pia2c, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // alphanumeric display
	map(0x3000, 0x3003).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x3400, 0x3403).rw(m_pia34, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // widget
	map(0x4000, 0xffff).rom();
}

void s11_state::s11_audio_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram();
	map(0x1000, 0x1fff).w(FUNC(s11_state::bank_w));
	map(0x2000, 0x2003).mirror(0x0ffc).rw(m_pias, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xbfff).bankr("bank0");
	map(0xc000, 0xffff).bankr("bank1");
}

static INPUT_PORTS_START( s11 )
	PORT_START("SW.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT ) // always plumb-bob tilt
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RSHIFT) // a relay from the power section
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LSHIFT) // usually slam tilt
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RCONTROL) // usually high score reset

	PORT_START("SW.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("SW.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("SW.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("SW.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("SW.6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s11_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s11_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

void s11_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_IRQ:
		// handle the cd4020 14-bit timer irq counter; this timer fires on the rising and falling edges of Q5, every 32 clocks
		m_timer_count += 0x20;
		m_timer_count &= 0x3fff;
		// handle the reset case (happens on the high level of Eclock, so supersedes the firing of the timer int)
		if (BIT(m_timer_count, 5) && (m_timer_irq_active || m_pia_irq_active))
		{
			m_timer_count = 0;
			m_timer_irq_active = false;
		}
		else // handle the timer int firing case
		{
#ifndef S11_W15
			// W14 jumper present (Q7), W15 absent (Q10)
			m_timer_irq_active = (BIT(m_timer_count, 7) && BIT(m_timer_count, 8) && BIT(m_timer_count, 9));
#else
			// W14 jumper absent (Q7), W15 present (Q10)
			m_timer_irq_active = (BIT(m_timer_count, 10) && BIT(m_timer_count, 8) && BIT(m_timer_count, 9));
#endif
		}

		m_mainirq->in_w<0>(m_timer_irq_active);
		if(m_pias)
			m_pias->cb1_w(m_timer_irq_active);
		m_irq_timer->adjust(attotime::from_ticks(32,E_CLOCK),0);
		break;
	}
}

void s11_state::machine_reset()
{
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
}

INPUT_CHANGED_MEMBER( s11_state::main_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( s11_state::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		if(m_audiocpu)
			m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE_LINE_MEMBER( s11_state::pia_irq )
{
	m_pia_irq_active = state;
	m_mainirq->in_w<1>(state);
}

WRITE_LINE_MEMBER( s11_state::main_irq )
{
	// handle the fact that the Advance and Up/Down switches are gated by the combined timer/pia irq signal
	if(state == CLEAR_LINE)
	{
		m_pia28->ca1_w(1);
		m_pia28->cb1_w(1);
	}
	else
	{
		m_pia28->ca1_w(BIT(ioport("DIAGS")->read(), 2));  // Advance
		m_pia28->cb1_w(BIT(ioport("DIAGS")->read(), 3));  // Up/Down
	}
	m_maincpu->set_input_line(M6802_IRQ_LINE, state);
}

void s11_state::sol3_w(uint8_t data)
{
}

void s11_state::sound_w(uint8_t data)
{
	m_sound_data = data;
}

WRITE_LINE_MEMBER( s11_state::pia21_ca2_w )
{
// sound ns
	if(m_pias)
		m_pias->ca1_w(state);
}

void s11_state::lamp0_w(uint8_t data)
{
}

void s11_state::dig0_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7447
	data &= 0x7f;
	m_strobe = data & 15;
	m_diag = (data & 0x70) >> 4;
	m_digits[60] = patterns[data>>4]; // diag digit
	m_segment1 = 0;
	m_segment2 = 0;
}

void s11_state::dig1_w(uint8_t data)
{
	m_segment2 |= data;
	m_segment2 |= 0x20000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}

uint8_t s11_state::pia28_w7_r()
{
	uint8_t ret = 0x80;

	ret |= m_strobe;
	ret |= m_diag << 4;

	if(BIT(ioport("DIAGS")->read(), 4))  // W7 Jumper
		ret &= ~0x80;

	return ret;
}

void s11_state::pia2c_pa_w(uint8_t data)
{
	m_segment1 |= (data<<8);
	m_segment1 |= 0x10000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe] = bitswap<16>(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment1 |= 0x40000;
	}
}

void s11_state::pia2c_pb_w(uint8_t data)
{
	m_segment1 |= data;
	m_segment1 |= 0x20000;
	if ((m_segment1 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe] = bitswap<16>(m_segment1, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment1 |= 0x40000;
	}
}

uint8_t s11_state::switch_r()
{
	uint8_t retval = 0xff;
	// scan all 8 input columns, since multiple can be selected at once
	for (int i = 0; i < 7; i++)
	{
		if (m_switch_col & (1<<i))
			retval &= m_swarray[i]->read();
	}
	//retval &= ioport("OPTOS")->read(); // optos should be read here as well, and are always active even if no column is selected
	return ~retval;
}

void s11_state::switch_w(uint8_t data)
{
	// this drives the pulldown 2N3904 NPN transistors Q42-Q49, each of which drives one column of the switch matrix low
	// it is possible for multiple columns to be enabled at once, this is handled in switch_r above.
	m_switch_col = data;
}

void s11_state::pia34_pa_w(uint8_t data)
{
	m_segment2 |= (data<<8);
	m_segment2 |= 0x10000;
	if ((m_segment2 & 0x70000) == 0x30000)
	{
		m_digits[m_strobe+16] = bitswap<16>(m_segment2, 7, 15, 12, 10, 8, 14, 13, 9, 11, 6, 5, 4, 3, 2, 1, 0);
		m_segment2 |= 0x40000;
	}
}

void s11_state::pia34_pb_w(uint8_t data)
{
	if(m_bg)
		m_bg->data_w(data);
	else if (m_ps88)
		m_ps88->data_w(data);
}

WRITE_LINE_MEMBER( s11_state::pia34_cb2_w )
{
	if(m_bg)
		m_bg->ctrl_w(state); // MCB2 through CPU interface
	else if (m_ps88)
		m_ps88->strobe_w(state);
}

void s11_state::bank_w(uint8_t data)
{
	membank("bank0")->set_entry(BIT(data, 1));
	membank("bank1")->set_entry(BIT(data, 0));
}

WRITE_LINE_MEMBER( s11_state::pias_ca2_w )
{
// speech clock
	if(m_hc55516)
		m_hc55516->clock_w(state);
}

WRITE_LINE_MEMBER( s11_state::pias_cb2_w )
{
// speech data
	if(m_hc55516)
		m_hc55516->digit_w(state);
}

uint8_t s11_state::sound_r()
{
	return m_sound_data;
}

void s11_state::init_s11()
{
	uint8_t *ROM = memregion("audiocpu")->base();
	membank("bank0")->configure_entries(0, 2, &ROM[0x10000], 0x4000);
	membank("bank1")->configure_entries(0, 2, &ROM[0x18000], 0x4000);
	membank("bank0")->set_entry(0);
	membank("bank1")->set_entry(0);
	m_timer_count = 0;
	m_irq_timer = timer_alloc(TIMER_IRQ);
	m_irq_timer->adjust(attotime::from_ticks(32,E_CLOCK),0);
	m_timer_irq_active = false;
	m_pia_irq_active = false;
}

void s11_state::s11(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s11_state::s11_main_map);
	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set(FUNC(s11_state::main_irq));
	INPUT_MERGER_ANY_HIGH(config, m_piairq).output_handler().set(FUNC(s11_state::pia_irq));

	/* Video */
	config.set_default_layout(layout_s11);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->readpa_handler().set(FUNC(s11_state::sound_r));
	m_pia21->set_port_a_input_overrides_output_mask(0xff);
	m_pia21->writepa_handler().set(FUNC(s11_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s11_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s11_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s11_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<1>));
	m_pia21->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<2>));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s11_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s11_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s11_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<3>));
	m_pia24->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<4>));

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s11_state::pia28_w7_r));
	m_pia28->set_port_a_input_overrides_output_mask(0xff);
	m_pia28->writepa_handler().set(FUNC(s11_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s11_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s11_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s11_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<5>));
	m_pia28->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<6>));

	PIA6821(config, m_pia2c, 0);
	m_pia2c->writepa_handler().set(FUNC(s11_state::pia2c_pa_w));
	m_pia2c->writepb_handler().set(FUNC(s11_state::pia2c_pb_w));
	m_pia2c->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<7>));
	m_pia2c->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<8>));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s11_state::switch_r));
	m_pia30->set_port_a_input_overrides_output_mask(0xff);
	m_pia30->writepb_handler().set(FUNC(s11_state::switch_w));
	m_pia30->cb2_handler().set(FUNC(s11_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<9>));
	m_pia30->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<10>));

	PIA6821(config, m_pia34, 0);
	m_pia34->writepa_handler().set(FUNC(s11_state::pia34_pa_w));
	m_pia34->writepb_handler().set(FUNC(s11_state::pia34_pb_w));
	m_pia34->cb2_handler().set(FUNC(s11_state::pia34_cb2_w));
	m_pia34->irqa_handler().set(m_piairq, FUNC(input_merger_device::in_w<11>));
	m_pia34->irqb_handler().set(m_piairq, FUNC(input_merger_device::in_w<12>));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Add the soundcard */
	M6808(config, m_audiocpu, XTAL(4'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &s11_state::s11_audio_map);
	INPUT_MERGER_ANY_HIGH(config, m_audioirq).output_handler().set_inputline(m_audiocpu, M6808_IRQ_LINE);

	MC1408(config, m_dac, 0);

	// common CVSD filter for system 11 and 11a, this is also the same filter circuit as Sinistar/System 6 uses, and is ALMOST the same filter from the s11 bg sound boards, see /mame/audio/s11c_bg.cpp
	// The CVSD filter has a large gain, about 4.6x
	// The filter is boosting the ~5vpp audio signal from the CVSD chip to a ~23vpp (really ~17vpp) theoretical audio signal that the s11
	// mainboard outputs on its volume control-repurposed-as-audio-out connector.
	// In reality, the S11 mainboard outputs audio at a virtual ground level between +5v and -12v (so, 17VPP balanced around -7VDC), but since
	// the CVSD chip's internal DAC can only output between a bit over +0x180/-0x180 out of 0x200, the most voltage it can ever output is
	// between (assuming 0x1ff is 5VDC and 0x300 is 0VDC) a max of 4.375VDC and a min of 0.625VDC, i.e. 3.75VPP centered on 2.5VDC.
	// In reality, the range is likely less than that.
	// This means multiplying a 3.75VPP signal by 4.6 is 17.25VPP, which is almost exactly the expected 17V (12v+5v) VPP the output should have.
	FILTER_BIQUAD(config, m_cvsd_filter2).opamp_mfb_lowpass_setup(RES_K(27), RES_K(15), RES_K(27), CAP_P(4700), CAP_P(1200));
	FILTER_BIQUAD(config, m_cvsd_filter).opamp_mfb_lowpass_setup(RES_K(43), RES_K(36), RES_K(180), CAP_P(1800), CAP_P(180));
	m_cvsd_filter->add_route(ALL_OUTPUTS, m_cvsd_filter2, 1.0);
	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, m_cvsd_filter, 1.0);

	PIA6821(config, m_pias, 0);
	m_pias->readpa_handler().set(FUNC(s11_state::sound_r));
	m_pias->set_port_a_input_overrides_output_mask(0xff);
	m_pias->writepa_handler().set(FUNC(s11_state::sound_w));
	m_pias->writepb_handler().set(m_dac, FUNC(dac_byte_interface::data_w));
	m_pias->ca2_handler().set(m_hc55516, FUNC(hc55516_device::clock_w));
	m_pias->cb2_handler().set(m_hc55516, FUNC(hc55516_device::digit_w));
	m_pias->irqa_handler().set(m_audioirq, FUNC(input_merger_device::in_w<0>));
	m_pias->irqb_handler().set(m_audioirq, FUNC(input_merger_device::in_w<1>));
}

void s11_state::s11_bgs(machine_config &config)
{
	s11(config);
	/* Add the background sound card */
	S11_BGS(config, m_bg);
	m_dac->add_route(ALL_OUTPUTS, m_bg, 0.5/2.0);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_bg, 0.5/2.0);
	m_pia34->ca2_handler().set(m_bg, FUNC(s11_bgs_device::resetq_w));
	m_bg->pb_cb().set(m_pia34, FUNC(pia6821_device::portb_w));
	m_bg->cb2_cb().set(m_pia34, FUNC(pia6821_device::cb1_w));
	SPEAKER(config, "speaker").front_center();
	m_bg->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void s11_state::s11_bgm(machine_config &config)
{
	s11(config);
	/* Add the background music card */
	S11_BGM(config, m_bg);
	m_dac->add_route(ALL_OUTPUTS, m_bg, 0.5319/2.0);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, m_bg, 0.5319/2.0);
	m_pia34->ca2_handler().set(m_bg, FUNC(s11_bgm_device::resetq_w));
	m_bg->pb_cb().set(m_pia34, FUNC(pia6821_device::portb_w));
	m_bg->cb2_cb().set(m_pia34, FUNC(pia6821_device::cb1_w));
	SPEAKER(config, "speaker").front_center();
	m_bg->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

void s11_state::s11_only(machine_config &config)
{
	s11(config);
	SPEAKER(config, "speaker").front_center();
	m_dac->add_route(ALL_OUTPUTS, "speaker", 0.25);
	m_cvsd_filter2->add_route(ALL_OUTPUTS, "speaker", 0.25);
}

/*----------------------------
/ Grand Lizard 04/86 (#523)
/-----------------------------*/
ROM_START(grand_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u26.l3", 0x4000, 0x2000, CRC(5fe50db6) SHA1(7e2adfefce5c33ad605606574dbdfb2642aa0e85))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("lzrd_u27.l4", 0x8000, 0x8000, CRC(6462ca55) SHA1(0ebfa998d3cefc213ada9ed815d44977120e5d6d))
	ROM_FILL(0x6035, 1, 0x00) // default to English

	// according to the manual these should be 32K roms just like the other games here
	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u21.l1", 0x1c000, 0x4000, CRC(98859d37) SHA1(08429b9e6a3b3007815373dc280b985e3441aa9f))
	ROM_RELOAD( 0x18000, 0x4000)
	ROM_LOAD("lzrd_u22.l1", 0x14000, 0x4000, CRC(4e782eba) SHA1(b44ab499128300175bdb57f07ffe2992c82e47e4))
	ROM_RELOAD( 0x10000, 0x4000)

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u4.l1", 0x0000, 0x8000, CRC(4baafc11) SHA1(3507f5f37e02688fa56cf5bb303eaccdcedede06))
ROM_END

ROM_START(grand_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u26.l3", 0x4000, 0x2000, CRC(5fe50db6) SHA1(7e2adfefce5c33ad605606574dbdfb2642aa0e85))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("lzrd_u27.l3", 0x8000, 0x8000, CRC(9061dfdc) SHA1(06e0add721afa0a89ad4961cddbc5409f95362df))
	ROM_FILL(0x6035, 1, 0x00) // default to English

	// according to the manual these should be 32K roms just like the other games here
	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u21.l1", 0x1c000, 0x4000, CRC(98859d37) SHA1(08429b9e6a3b3007815373dc280b985e3441aa9f))
	ROM_RELOAD( 0x18000, 0x4000)
	ROM_LOAD("lzrd_u22.l1", 0x14000, 0x4000, CRC(4e782eba) SHA1(b44ab499128300175bdb57f07ffe2992c82e47e4))
	ROM_RELOAD( 0x10000, 0x4000)

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("lzrd_u4.l1", 0x0000, 0x8000, CRC(4baafc11) SHA1(3507f5f37e02688fa56cf5bb303eaccdcedede06))
ROM_END

/*-------------------------
/ High Speed 01/86 (#541)
/--------------------------*/
ROM_START(hs_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u26.l4", 0x4000, 0x2000, CRC(38b73830) SHA1(df89670f3df2b657dcf1f8ee08e506e54e016028))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("hs_u27.l4", 0x8000, 0x8000, CRC(24c6f7f0) SHA1(bb0058650ec0908f88b6a202df79e971b46f8594))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u21.l2", 0x18000, 0x8000, CRC(c0580037) SHA1(675ca65a6a20f8607232c532b4d127641f77d837))
	ROM_LOAD("hs_u22.l2", 0x10000, 0x8000, CRC(c03be631) SHA1(53823e0f55377a45aa181882c310dd307cf368f5))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u4.l1", 0x0000, 0x8000, CRC(0f96e094) SHA1(58650705a02a71ced85f5c2a243722a35282cbf7))
ROM_END

ROM_START(hs_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u26-l3.rom", 0x4000, 0x2000, CRC(fd587959) SHA1(20fe6d7bd617b1fa886362ce520393a25be9a632))
	ROM_RELOAD( 0x6000, 0x2000)
	ROM_LOAD("hs_u27.l4", 0x8000, 0x8000, CRC(24c6f7f0) SHA1(bb0058650ec0908f88b6a202df79e971b46f8594))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u21.l2", 0x18000, 0x8000, CRC(c0580037) SHA1(675ca65a6a20f8607232c532b4d127641f77d837))
	ROM_LOAD("hs_u22.l2", 0x10000, 0x8000, CRC(c03be631) SHA1(53823e0f55377a45aa181882c310dd307cf368f5))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("hs_u4.l1", 0x0000, 0x8000, CRC(0f96e094) SHA1(58650705a02a71ced85f5c2a243722a35282cbf7))
ROM_END

/*-------------------------
/ Road Kings 07/86 (#542)
/--------------------------*/

ROM_START(rdkng_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u26.l1", 0x4000, 0x4000, CRC(19abe96b) SHA1(d6c3b6dab328f23cc4506e4f56cd0beeb06fb3cb))
	ROM_LOAD("road_u27.l1", 0x8000, 0x8000, CRC(3dcad794) SHA1(0cf06f8e16d738f0bc0111e2e12351a26e2f02c6))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x0000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

ROM_START(rdkng_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u26.l1", 0x4000, 0x4000, CRC(19abe96b) SHA1(d6c3b6dab328f23cc4506e4f56cd0beeb06fb3cb))
	ROM_LOAD("road_u27.l2", 0x8000, 0x8000, CRC(aff45e2b) SHA1(c52aca20639f519a940951ef04c2bd179a596b30))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x0000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

ROM_START(rdkng_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u26.l3", 0x4000, 0x4000, CRC(9bade45d) SHA1(c1791724761cdd1d863e12b02655c5fed8936162))
	ROM_LOAD("road_u27.l3", 0x8000, 0x8000, CRC(97b599dc) SHA1(18524d22a75b0569bb480d847cef8047ee51f91e))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x0000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

ROM_START(rdkng_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u26.l4", 0x4000, 0x4000, CRC(4ea27d67) SHA1(cf46e8c5e417999150403d6d40adf8c36b1c0347))
	ROM_LOAD("road_u27.l4", 0x8000, 0x8000, CRC(5b88e755) SHA1(6438505bb335f670e0892126764819a48eec9b88))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u21.l1", 0x18000, 0x8000, CRC(f34efbf4) SHA1(cb5ffe9818994f4681e3492a5cd46f410d2e5353))
	ROM_LOAD("road_u22.l1", 0x10000, 0x8000, CRC(a9803804) SHA1(a400d4621c3f7a6e47546b2f33dc4920183a5a74))

	ROM_REGION(0x80000, "bg:cpu", ROMREGION_ERASEFF)
	ROM_LOAD("road_u4.l1", 0x0000, 0x8000, CRC(4395b48f) SHA1(2325ce6ba7f6f92f884c302e6f053c31229dc774))
ROM_END

/************************ From here, not pinball machines **************************************/

/*------------------------------
/ Alley Cats (Shuffle) (#918)
/-------------------------------*/
ROM_START(alcat_l7)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u26_rev7.rom", 0xd000, 0x1000, CRC(4d274dd3) SHA1(80d72bd0f85ce2cac04f6d9f59dc1fcccc86d402))
	ROM_LOAD("u27_rev7.rom", 0xe000, 0x2000, CRC(9c7faf8a) SHA1(dc1a561948b9a303f7924d7bebcd972db766827b))

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("acs_u21.bin", 0x18000, 0x8000, CRC(c54cd329) SHA1(4b86b10e60a30c4de5d97129074f5657447be676))
	ROM_LOAD("acs_u22.bin", 0x10000, 0x8000, CRC(56c1011a) SHA1(c817a3410c643617f3643897b8f529ae78546b0d))
ROM_END

/*--------------------
/ Tic-Tac-Strike (#919)
/--------------------*/
ROM_START(tts_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u27_l2.128", 0x8000, 0x4000, CRC(edbcab92) SHA1(0f6b2dc01874984f9a17ee873f2fa0b6c9bba5be))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u21.256", 0x18000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x10000, 0x8000, NO_DUMP)
ROM_END

ROM_START(tts_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u27.128", 0x8000, 0x4000, CRC(f540c53c) SHA1(1c7a318278ad1afdcbe6aaf81f9b774882b069d6))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tts_u21.256", 0x18000, 0x8000, NO_DUMP)
	ROM_LOAD("tts_u22.256", 0x10000, 0x8000, NO_DUMP)
ROM_END

/*-------------------------------
/ Gold Mine (Shuffle) (#920) s11b
/--------------------------------*/
ROM_START(gmine_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u27.128", 0x8000, 0x4000, CRC(99c6e049) SHA1(356faec0598a54892050a28857e9eb5cdbf35833))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("u21.256", 0x18000, 0x8000, CRC(3b801570) SHA1(50b50ff826dcb031a30940fa3099bd3a8d773831))
	ROM_LOAD("u22.256", 0x10000, 0x8000, CRC(08352101) SHA1(a7437847a71cf037a80686292f9616b1e08922df))
ROM_END

/*-------------------------
/ Top Dawg (Shuffle) (#921)
/--------------------------*/
ROM_START(tdawg_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("tdu27r1.128", 0x8000, 0x4000, CRC(0b4bb586) SHA1(a927ebf7167609cc84b38c22aa35d0c4d259dd8b))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("tdsu21r1.256", 0x18000, 0x8000, CRC(6a323227) SHA1(7c7263754e5672c654a2ee9582f0b278e637a909))
	ROM_LOAD("tdsu22r1.256", 0x10000, 0x8000, CRC(58407eb4) SHA1(6bd9b304c88d9470eae5afb6621187f4a8313573))
ROM_END

/*----------------------------
/ Shuffle Inn (Shuffle) (#922)
/-----------------------------*/
ROM_START(shfin_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("u27rom-1.rv1", 0x8000, 0x4000, CRC(40cfb74a) SHA1(8cee4212ea8bb6b360060391df3208e1e129d7e5))
	ROM_RELOAD( 0xc000, 0x4000)

	ROM_REGION(0x20000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("u21snd-2.rv1", 0x18000, 0x8000, CRC(80ddce05) SHA1(9498260e5ccd2fe0eb03ff321dd34eb945b0213a))
	ROM_LOAD("u22snd-2.rv1", 0x10000, 0x8000, CRC(6894abaf) SHA1(2d661765fbfce33a73a20778c41233c0bd9933e9))
ROM_END

// Pinball
GAME( 1986, grand_l4, 0,        s11_bgs,  s11, s11_state, init_s11, ROT0, "Williams", "Grand Lizard (L-4)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, grand_l3, grand_l4, s11_bgs,  s11, s11_state, init_s11, ROT0, "Williams", "Grand Lizard (L-3)",             MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, hs_l4,    0,        s11_bgs,  s11, s11_state, init_s11, ROT0, "Williams", "High Speed (L-4)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, hs_l3,    hs_l4,    s11_bgs,  s11, s11_state, init_s11, ROT0, "Williams", "High Speed (L-3)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, rdkng_l4, 0,        s11_bgm,  s11, s11_state, init_s11, ROT0, "Williams", "Road Kings (L-4)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, rdkng_l1, rdkng_l4, s11_bgm,  s11, s11_state, init_s11, ROT0, "Williams", "Road Kings (L-1)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, rdkng_l2, rdkng_l4, s11_bgm,  s11, s11_state, init_s11, ROT0, "Williams", "Road Kings (L-2)",               MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, rdkng_l3, rdkng_l4, s11_bgm,  s11, s11_state, init_s11, ROT0, "Williams", "Road Kings (L-3)",               MACHINE_IS_SKELETON_MECHANICAL )

// Shuffle
GAME( 1985, alcat_l7, 0,        s11_only, s11, s11_state, init_s11, ROT0, "Williams", "Alley Cats (Shuffle) (L-7)",     MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, tts_l2,   0,        s11_only, s11, s11_state, init_s11, ROT0, "Williams", "Tic-Tac-Strike (Shuffle) (L-2)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1986, tts_l1,   tts_l2,   s11_only, s11, s11_state, init_s11, ROT0, "Williams", "Tic-Tac-Strike (Shuffle) (L-1)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, gmine_l2, 0,        s11_only, s11, s11_state, init_s11, ROT0, "Williams", "Gold Mine (Shuffle) (L-2)",      MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, tdawg_l1, 0,        s11_only, s11, s11_state, init_s11, ROT0, "Williams", "Top Dawg (Shuffle) (L-1)",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1987, shfin_l1, 0,        s11_only, s11, s11_state, init_s11, ROT0, "Williams", "Shuffle Inn (Shuffle) (L-1)",    MACHINE_IS_SKELETON_MECHANICAL )

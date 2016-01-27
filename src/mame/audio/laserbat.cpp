// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
    Laser Battle / Lazarian (c) 1981 Zaccaria
    Cat and Mouse           (c) 1982 Zaccaria

    audio emulation by Vas Crabb
*/

#include "includes/laserbat.h"


READ8_MEMBER(laserbat_state_base::rhsc_r)
{
	return m_rhsc;
}

WRITE8_MEMBER(laserbat_state_base::whsc_w)
{
	m_whsc = data;
}

WRITE8_MEMBER(laserbat_state_base::csound1_w)
{
	m_csound1 = data;
}

WRITE8_MEMBER(laserbat_state_base::csound2_w)
{
	m_csound2 = data;
}


/*
    The Laser Battle/Lazarian sound board has a SN76477 CSG, two TMS3615
    tone synthesisers, and a TDA1010 power amplifier.  It receives
    commands from the game board over a 16-bit unidirectional data bus.
    The CPU cannot write all sixteen lines atomically, it write to lines
    1-8 as one group and 9-16 as another group.

    The game board makes the the audio output from the first S2636 PVI
    (5E) available on a pin at the sound board interface connector, but
    it isn't routed anywhere, so you won't hear it.

    The TMS3615 at 05 is clocked at 250kHz (4MHz crystal oscillator
    divided by 16), and its divide-by-two output is used to clock the
    TMS3615 at 04.  This gives a base 16' note of C3.  The combined 8'
    or 16' outputs are selectable by jumper, allowing board to be
    switched between two octaves.  There's no indication of which octave
    would have been selected in the Lazarian manual.

    There's a filter network between the TMS3615 outputs and the power
    amplifier with several parameters controllable from the game board.

    The audio output of the SN76477 isn't actually used.  Rather the
    signal from before the output amplifier is taken and used to gate
    distortion elements in the analog filter network.

    +-----+----------------------------------------------------------+
    | Bit | Function                                                 |
    +-----+----------------------------------------------------------+
    |   1 | Multiplexed data bit 1                                   |
    |   2 | Multiplexed data bit 2                                   |
    |   3 | Multiplexed data bit 3                                   |
    |   4 | Multiplexed data bit 4                                   |
    |   5 | Multiplexed data bit 5                                   |
    |   6 | Multiplexed data bit 6                                   |
    |   7 | Multiplexed data bit 7                                   |
    |   8 | Multiplexed data bit 8                                   |
    |   9 | SN76477/distortion control (positive edge trigger)       |
    |  10 | High octave key 13                                       |
    |  11 | Key/SLF resistor select A                                |
    |  12 | Key/SLF resistor select B                                |
    |  13 | Key/SLF resistor control                                 |
    |  14 | Gates 22k/10nF low-pass filter and one SN-driven effect  |
    |  15 | SN76477 VCO select (inverted)                            |
    |  16 | TMS3615 reset                                            |
    +-----+----------------------------------------------------------+

    +-------+----------+-----------------------------------+
    | 11-13 | SLF res. | Key select                        |
    +-------+----------+-----------------------------------+
    |   0   |   27k    |                                   |
    |   1   |   22k    |                                   |
    |   2   |   22k    |                                   |
    |   3   |   12k    |                                   |
    |   4   |   inf.   |                                   |
    |   5   |   inf.   | Low octave 1-8                    |
    |   6   |   inf.   | Low octave 9-13, high octave 2-4  |
    |   7   |   inf.   | High octave 5-12                  |
    +-------+----------+-----------------------------------+

    When bit 13 is high, no SLF control resistor is connected to the
    CSG.  When bit 13 is low, bits 11 and 12 select between 27k, 22k and
    12k.

    When bit 13 is high, the clock input of a latch driving eight of
    TMS3615 key inputs will be driven low, depending on bits 11 and 12.
    These latches are positive edge triggered, and you have to beware of
    glitches, so the safe way to set key inputs is:
    * Ensure bit 13 is low
    * Write desired key data pattern to CSOUND1
    * Write CSOUND2 setting bits 11 and 12 to select keys to set
    * Write CSOUND2 setting bit 13 high
    * Write CSOUND2 setting bit 13 low
    * Write CSOUND2 setting bits 11 and 12 to select SLF resistor

    Note that the SLF resistor will necessarily thrash around during the
    process of setting key data.  It's a side-effect of the way control
    lines are overloaded.

    The SN76477 control bits are defined as follows:

    +-----+---------------------------------------------------------+
    | Bit | Function                                                |
    +-----+---------------------------------------------------------+
    |  1  | Noise filter/VCO resistor select A                      |
    |  2  | Noise filter/VCO resistor select B                      |
    |  3  | Noise filter/VCO resistor select C                      |
    |  4  | AB SOUND (connected to System Inhibit input)            |
    |  5  | VCO/NOISE (connected to Mixer Select B input)           |
    |  6  | DEGR (controls a distortion element in filter network)  |
    |  7  | FILT (gates 10nF capacitor across distortion elements)  |
    |  8  | A (controls a distortion element in filter network)     |
    +-----+---------------------------------------------------------+

*/

WRITE8_MEMBER(laserbat_state::csound2_w)
{
	// there are a bunch of edge-triggered things, so grab changes
	unsigned const diff = data ^ m_csound2;

	// SN76477 and distortion control
	if (data & diff & 0x01)
	{
		switch (m_csound1 & 0x07)
		{
		case 0x00:
			m_csg->noise_filter_res_w(RES_K(270));  // R30
			m_csg->vco_res_w(RES_K(47));            // R47
			break;
		case 0x01:
			m_csg->noise_filter_res_w(RES_K(220));  // R23
			m_csg->vco_res_w(RES_K(27));            // R40
			break;
		case 0x02:
			m_csg->noise_filter_res_w(RES_K(150));  // R24
			m_csg->vco_res_w(RES_K(22));            // R41
			break;
		case 0x03:
			m_csg->noise_filter_res_w(RES_K(120));  // R25
			m_csg->vco_res_w(RES_K(15));            // R42
			break;
		case 0x04:
			m_csg->noise_filter_res_w(RES_K(82));   // R29
			m_csg->vco_res_w(RES_K(12));            // R46
			break;
		case 0x05:
			m_csg->noise_filter_res_w(RES_K(68));   // R28
			m_csg->vco_res_w(RES_K(8.2));           // R45
			break;
		case 0x06:
			m_csg->noise_filter_res_w(RES_K(47));   // R27
			m_csg->vco_res_w(RES_K(6.8));           // R44
			break;
		case 0x07:
			m_csg->noise_filter_res_w(RES_K(33));   // R26
			m_csg->vco_res_w(RES_K(4.7));           // R43
			break;
		}
		m_csg->enable_w((m_csound1 & 0x08) ? 1 : 0);
		m_csg->mixer_b_w((m_csound1 & 0x10) ? 1 : 0);
		// TODO: DEGR/FILT/A
	}

	// edge-triggered latches plus a dedicated bit for note control
	m_keys = (m_keys & 0x00ffffff) | ((data & 0x02) ? 0x01000000 : 0x00000000);
	if ((m_csound2 & 0x10) && (diff & 0x1c))
	{
		switch (m_csound2 & 0x0c)
		{
		case 0x00:
			break;
		case 0x04:
			m_keys = (m_keys & 0x01ffff00) | (unsigned(m_csound1) << 0);
			break;
		case 0x08:
			m_keys = (m_keys & 0x01ff00ff) | (unsigned(m_csound1) << 8);
			break;
		case 0x0c:
			m_keys = (m_keys & 0x0100ffff) | (unsigned(m_csound1) << 16);
			break;
		}
	}
	m_synth_low->enable_w((m_keys >> 0) & 0x01fff);
	m_synth_high->enable_w((m_keys >> 12) & 0x01ffe);

	// bits 11-13 set the SLF control register directly
	switch (data & 0x1c)
	{
	case 0x00:  // R54
		m_csg->slf_res_w(RES_K(27));
		break;
	case 0x04:  // R53
	case 0x08:  // R52
		m_csg->slf_res_w(RES_K(22));
		break;
	case 0x0c:  // R51
		m_csg->slf_res_w(RES_K(12));
		break;
	default:    // NC
		m_csg->slf_res_w(RES_INF);
		break;
	}

	// TODO: BIT14 filter control

	// inverted VCO select
	m_csg->vco_w((data & 0x40) ? 0 : 1);

	// TODO: BIT15 TMS reset

	// keep for detecting changes next time
	m_csound2 = data;
}


/*
    The Cat and Mouse sound board has a 6802 processor with three ROMs,
    a 6821 PIA, two AY-3-8910 PSGs, and some other logic and analog
    circuitry.  Unfortunately we lack a schematic, so all knowledge of
    this board is based on tracing the sound program.

    The 6821 PIA is mapped at addresses $005C..$005F.  The known PIA
    signal assignments are as follows:

    +------+-----------------------+
    | PA0  | PSG1/PSG2 DA0         |
    | PA1  | PSG1/PSG2 DA1         |
    | PA2  | PSG1/PSG2 DA2         |
    | PA3  | PSG1/PSG2 DA3         |
    | PA4  | PSG1/PSG2 DA4         |
    | PA5  | PSG1/PSG2 DA5         |
    | PA6  | PSG1/PSG2 DA6         |
    | PA7  | PSG1/PSG2 DA7         |
    | PB0  | PSG1 BC1              |
    | PB1  | PSG1 BDIR             |
    | PB2  | PSG2 BC1              |
    | PB3  | PSG2 BDIR             |
    | CA1  | Host interface bit 6  |
    | CB1  | periodic IRQ source   |
    | IRQA | 6802 NMI              |
    | IRQB | 6802 IRQ              |
    +------+-----------------------+

    The program makes use of I/O port A on the first PSG as outputs.  At
    a guess, it could have the same function as it does on other
    Zaccaria sound boards.

    The first PSG receives commands from the game board in the low five
    bits of port B.  Commands are processed on receiving an NMI.  The
    sound program always masks out the high three bits of the value so
    they could be connected to anything on the board.

    The I/O ports on the second PSG don't appear to be used at all.

    The game board sends commands to the sound board over a 16-bit
    unidirectional data bus.  The CPU cannot write all sixteen lines
    atomically, it write to lines 1-8 as one group and 9-16 as another
    group.  However only seven lines are actually connected to the sound
    board:

    +-----+----------+-------------+
    | Bit | Name     | Connection  |
    +-----+----------+-------------+
    |   1 | SOUND 0  | PSG1 IOB0   |
    |   2 | SOUND 1  | PSG1 IOB1   |
    |   3 | SOUND 2  | PSG1 IOB2   |
    |   4 | SOUND 3  | PSG1 IOB3   |
    |   5 | SOUND 4  | PSG1 IOB4   |
    |   6 | SOUND 5  | PIA CA1     |
    |   7 |          |             |
    |   8 |          |             |
    |   9 |          |             |
    |  10 |          |             |
    |  11 |          |             |
    |  12 |          |             |
    |  13 |          |             |
    |  14 |          |             |
    |  15 |          |             |
    |  16 | RESET    | Unknown     |
    +-----+----------+-------------+

    There could well be other connections on the sound board - these are
    just what can be deduced by tracing the sound program.

    The game board makes the the audio output from the first S2636 PVI
    (5E) available on a pin at the sound board interface connector, but
    it isn't routed anywhere.
*/

WRITE8_MEMBER(catnmous_state::csound1_w)
{
	m_pia->ca1_w((data & 0x20) ? 1 : 0);
	m_csound1 = data;
}

WRITE8_MEMBER(catnmous_state::csound2_w)
{
	// the top bit is called RESET on the wiring diagram - assume it resets the sound CPU
	m_audiocpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
	m_csound2 = data;
}

READ8_MEMBER(catnmous_state::pia_porta_r)
{
	UINT8 const control = m_pia->b_output();
	UINT8 data = 0xff;

	if (0x01 == (control & 0x03))
		data &= m_psg1->data_r(space, 0);

	if (0x04 == (control & 0x0c))
		data &= m_psg2->data_r(space, 0);

	return data;
}

WRITE8_MEMBER(catnmous_state::pia_porta_w)
{
	UINT8 const control = m_pia->b_output();

	if (control & 0x02)
		m_psg1->data_address_w(space, (control >> 0) & 0x01, data);

	if (control & 0x08)
		m_psg2->data_address_w(space, (control >> 2) & 0x01, data);
}

WRITE8_MEMBER(catnmous_state::pia_portb_w)
{
	if (data & 0x02)
		m_psg1->data_address_w(space, (data >> 0) & 0x01, m_pia->a_output());

	if (data & 0x08)
		m_psg2->data_address_w(space, (data >> 2) & 0x01, m_pia->a_output());
}

WRITE_LINE_MEMBER(catnmous_state::pia_irqa)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(catnmous_state::pia_irqb)
{
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(catnmous_state::psg1_porta_w)
{
	// similar to zaccaria.c since we have no clue how this board really works
	// this code could be completely wrong/inappropriate for this game for all we know
	static double const table[8] = {
			RES_K(8.2),
			RES_R(820),
			RES_K(3.3),
			RES_R(150),
			RES_K(5.6),
			RES_R(390),
			RES_K(1.5),
			RES_R(47) };
	RES_VOLTAGE_DIVIDER(RES_K(4.7), table[data & 0x07]);
	m_psg2->set_volume(1, 150 * RES_VOLTAGE_DIVIDER(RES_K(4.7), table[data & 0x07]));
}

READ8_MEMBER(catnmous_state::psg1_portb_r)
{
	// the sound program masks out the three most significant bits
	// assume they're not connected and read high from the internal pull-ups
	return m_csound1 | 0xe0;
}

INTERRUPT_GEN_MEMBER(catnmous_state::cb1_toggle)
{
	m_cb1 = !m_cb1;
	m_pia->cb1_w(m_cb1 ? 1 : 0);
}

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*********************************************************************************************

PINBALL
Playmatic MPU 3,4,5

Status:
- Lamps, Solenoids to add
- AY chips output port adds various components across the analog outputs
- Lots of loud siren-like noises when there should be silence
- Mechanical sounds to add
- Most games work
-- Spain82: not working (no manual available; uses same sound board as Cerberus)
-- Nautilus: sound is broken (runs into the weeds)
-- Skill Flight, Phantom Ship: not working
-- Eight Ball Champ, Cobra: not working (different hardware)
-- Miss Disco: not working (no manual available)
-- Meg Aaton: Ball number doesn't show

Note: The input lines INT, EF1-4 are inverted (not true voltage).

First time:
- The default settings are fine, so start with a clean slate
- Wait for it to say No Ball, then hold down X.
- Keep holding X, insert credits, and press start. When it says ball 1, let go immediately.
- If you hold down X longer than you should, it says Coil Error, and the game is ended. You
  should let go the instant the ball number increments.
- Any games marked working actually do work, but like most pinballs they are a pain to play
  with the keyboard.

Setting up:
The manual is not that clear, there's a lot we don't know, this *seems* to work...
- Start machine, wait for it to say No Ball, and it plays a tune
- Press 8 (to simulate opening the front door)
- Press 1, it says BooP (Bookkeeping)
- Press 8 and the credits number increments to 8, then 01 and on to 17 (2 pushes per number)
- When it gets to 01, the display says Adjust
- To adjust a setting, press 1 to choose a digit, then tap 9 until it is correct.
- The settings in the manual do not fully line up with what we see in the display.


***********************************************************************************************/


#include "emu.h"
#include "machine/genpin.h"
#include "audio/efo_zsu.h"

#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"
#include "machine/7474.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "play_3.lh"



class play_3_state : public genpin_class
{
public:
	play_3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
		, m_aysnd1(*this, "aysnd1")
		, m_aysnd2(*this, "aysnd2")
		, m_zsu(*this, "zsu")
		, m_keyboard(*this, "X.%u", 0)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void sklflite(machine_config &config);
	void play_3(machine_config &config);
	void megaaton(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(megaaton_port01_w);
	DECLARE_WRITE8_MEMBER(port02_w);
	DECLARE_WRITE8_MEMBER(port03_w);
	DECLARE_WRITE8_MEMBER(sklflite_port03_w);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_READ8_MEMBER(port05_r);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_WRITE8_MEMBER(port07_w);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef1_r);
	DECLARE_READ_LINE_MEMBER(ef4_r);
	DECLARE_WRITE_LINE_MEMBER(q4013a_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(clock2_w);
	DECLARE_WRITE8_MEMBER(port01_a_w);
	DECLARE_READ8_MEMBER(port02_a_r);
	DECLARE_READ_LINE_MEMBER(clear_a_r);

	void megaaton_io(address_map &map);
	void play_3_audio_io(address_map &map);
	void play_3_audio_map(address_map &map);
	void play_3_io(address_map &map);
	void play_3_map(address_map &map);
	void sklflite_io(address_map &map);

	u16 m_clockcnt;
	u16 m_resetcnt;
	u16 m_resetcnt_a;
	u8 m_soundlatch;
	u8 m_port03_old;
	u8 m_a_irqset;
	u16 m_a_irqcnt;
	u8 m_kbdrow;
	u8 m_segment[5];
	bool m_disp_sw;
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cosmac_device> m_maincpu;
	optional_device<cosmac_device> m_audiocpu;
	required_device<ttl7474_device> m_4013a;
	required_device<ttl7474_device> m_4013b;
	optional_device<ay8910_device> m_aysnd1;
	optional_device<ay8910_device> m_aysnd2;
	optional_device<efo_zsu_device> m_zsu;
	required_ioport_array<10> m_keyboard;
	output_finder<66> m_digits;
};


void play_3_state::play_3_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x80ff).ram().share("nvram"); // pair of 5101, battery-backed
}

void play_3_state::play_3_io(address_map &map)
{
	map(0x01, 0x01).w(FUNC(play_3_state::port01_w)); // digits, scan-lines
	map(0x02, 0x02).w(FUNC(play_3_state::port02_w)); // sound code
	map(0x03, 0x03).w(FUNC(play_3_state::port03_w)); //
	map(0x04, 0x04).r(FUNC(play_3_state::port04_r)); // switches
	map(0x05, 0x05).r(FUNC(play_3_state::port05_r)); // more switches
	map(0x06, 0x06).w(FUNC(play_3_state::port06_w)); // segments
	map(0x07, 0x07).w(FUNC(play_3_state::port07_w)); // flipflop clear
}

void play_3_state::megaaton_io(address_map &map)
{
	play_3_io(map);
	map(0x01, 0x01).w(FUNC(play_3_state::megaaton_port01_w)); // digits, scan-lines
}

void play_3_state::sklflite_io(address_map &map)
{
	play_3_io(map);
	map(0x03, 0x03).w(FUNC(play_3_state::sklflite_port03_w)); //
}

void play_3_state::play_3_audio_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4001).mirror(0x1ffe).rw(m_aysnd1, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x6000, 0x6001).mirror(0x1ffe).rw(m_aysnd2, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x8000, 0x80ff).ram();
}

void play_3_state::play_3_audio_io(address_map &map)
{
	map(0x01, 0x01).w(FUNC(play_3_state::port01_a_w)); // irq counter
	map(0x02, 0x02).r(FUNC(play_3_state::port02_a_r)); // sound code
}


static INPUT_PORTS_START( play_3 )
	PORT_START("X.0") // 11-18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("X.1") // 21-28
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X.2") // 31-38
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X.3") // 41-48
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X.4") // 51-58
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("X.5") // 61-68
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X.6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_TOGGLE // zone select (door switch)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X) // outhole
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9) PORT_TOGGLE // test button

	PORT_START("X.7")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0xE0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X.8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X.9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE )
INPUT_PORTS_END

static INPUT_PORTS_START( megaaton )
	PORT_INCLUDE( play_3 )
	PORT_MODIFY("X.0") // 11-18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X) // outhole

	PORT_MODIFY("X.6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_TOGGLE // zone select (door switch)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9) PORT_TOGGLE // test button
INPUT_PORTS_END

void play_3_state::machine_reset()
{
	m_clockcnt = 0;
	m_resetcnt = 0;
	m_resetcnt_a = 0;
	m_4013b->d_w(1);
	m_a_irqset = 54;  // default value of the CDP1863
	m_a_irqcnt = (m_a_irqset << 3) | 7;
	m_soundlatch = 0;
	m_kbdrow = 0;
	m_disp_sw = 0;
	for (uint8_t i = 0; i < 5; i++)
		m_segment[i] = 0;
	m_port03_old = 0;
}

WRITE8_MEMBER( play_3_state::port01_w )
{
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (uint8_t j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (uint8_t i = 0; i < 5; i++)
				{
					m_digits[j*10 + i] = m_segment[i] & 0x7f;
					// decimal dot on tens controls if last 0 shows or not
					if ((j == 5) && BIT(m_segment[i], 7))
						m_digits[60 + i] = 0x3f;
				}
	}
}

// megaaton status digits are rearranged slightly
WRITE8_MEMBER( play_3_state::megaaton_port01_w )
{
	uint8_t i,j,digit;
	m_kbdrow = data;
	if (m_kbdrow && m_disp_sw)
	{
		m_disp_sw = 0;
		for (j = 0; j < 6; j++)
			if (BIT(m_kbdrow, j))
				for (i = 0; i < 5; i++)
				{
					digit = j*10+i;
					if (digit == 44)
						digit = 64;
					m_digits[digit] = m_segment[i] & 0x7f;
					// decimal dot on tens controls if last 0 shows or not
					if ((j == 5) && (i < 4) && BIT(m_segment[i], 7))
						m_digits[60 + i] = 0x3f;
				}
	}
}

WRITE8_MEMBER( play_3_state::port02_w )
{
	m_soundlatch = data;
}

WRITE8_MEMBER( play_3_state::port03_w )
{
	if (BIT(data, 6))
		m_audiocpu->ef1_w(1); // inverted
	if (BIT(data, 5))
	{
		if (m_soundlatch == 11)
			m_samples->start(0, 5); // outhole
		//if (m_soundlatch == 13)
			//m_samples->start(0, 6); // no knocker?
	}

}

WRITE8_MEMBER( play_3_state::sklflite_port03_w )
{
	if (BIT(data, 6) && !BIT(m_port03_old, 6))
		m_zsu->sound_command_w(space, 0, m_soundlatch);
	if (BIT(data, 5))
	{
		if (m_soundlatch == 11)
			m_samples->start(0, 5); // outhole
		//if (m_soundlatch == 13)
			//m_samples->start(0, 6); // no knocker?
	}

	m_port03_old = data;
}

READ8_MEMBER( play_3_state::port04_r )
{
	if (m_kbdrow & 0x3f)
		for (uint8_t i = 0; i < 6; i++)
			if (BIT(m_kbdrow, i))
				return m_keyboard[i]->read();

	return 0;
}

READ8_MEMBER( play_3_state::port05_r )
{
	uint8_t data = 0, key8 = m_keyboard[8]->read() & 0x0f;
	if (BIT(m_kbdrow, 0))
		data |= m_keyboard[6]->read();
	if (BIT(m_kbdrow, 1))
		data |= m_keyboard[7]->read();
	return (data & 0xf0) | key8;
}

WRITE8_MEMBER( play_3_state::port06_w )
{
	m_segment[4] = m_segment[3];
	m_segment[3] = m_segment[2];
	m_segment[2] = m_segment[1];
	m_segment[1] = m_segment[0];
	m_segment[0] = data;
	m_disp_sw = 1;
}

WRITE8_MEMBER( play_3_state::port07_w )
{
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
}

WRITE8_MEMBER( play_3_state::port01_a_w )
{
	m_a_irqset = data;
	m_a_irqcnt = (m_a_irqset << 3) | 7;
}

READ8_MEMBER( play_3_state::port02_a_r )
{
	m_audiocpu->ef1_w(0); // inverted
	return m_soundlatch;
}

READ_LINE_MEMBER( play_3_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	return (m_resetcnt == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_3_state::clear_a_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt_a < 0xffff)
		m_resetcnt_a++;
	return (m_resetcnt_a == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_3_state::ef1_r )
{
	return (!BIT(m_clockcnt, 10)); // inverted
}

READ_LINE_MEMBER( play_3_state::ef4_r )
{
	return BIT(m_keyboard[9]->read(), 0); // inverted test button - doesn't seem to do anything
}

WRITE_LINE_MEMBER( play_3_state::clock_w )
{
	m_4013a->clock_w(state);

	if (!state)
	{
		m_clockcnt++;
		// simulate 4020 chip
		if ((m_clockcnt & 0x3ff) == 0)
			m_4013b->preset_w(BIT(m_clockcnt, 10)); // Q10 output

		if (m_audiocpu.found())
		{
			// sound irq
			m_a_irqcnt--;
			if (m_a_irqcnt == 1)
				m_audiocpu->int_w(1); // inverted
			else
			if (m_a_irqcnt == 0)
			{
				m_a_irqcnt = (m_a_irqset << 3) | 7;
				m_audiocpu->int_w(0); // inverted
			}
		}
	}
}

WRITE_LINE_MEMBER( play_3_state::clock2_w )
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(state); // inverted
}

WRITE_LINE_MEMBER( play_3_state::q4013a_w )
{
	m_clockcnt = 0;
}

void play_3_state::play_3(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &play_3_state::play_3_map);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::play_3_io);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(play_3_state::clear_r));
	m_maincpu->ef1_cb().set(FUNC(play_3_state::ef1_r));
	m_maincpu->ef4_cb().set(FUNC(play_3_state::ef4_r));
	m_maincpu->q_cb().set("4013a", FUNC(ttl7474_device::clear_w));
	m_maincpu->tpb_cb().set(FUNC(play_3_state::clock_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_play_3);

	// Devices
	clock_device &xpoint(CLOCK(config, "xpoint", 60)); // crossing-point detector
	xpoint.signal_handler().set(FUNC(play_3_state::clock2_w));

	// This is actually a 4013 chip (has 2 RS flipflops)
	TTL7474(config, m_4013a, 0);
	m_4013a->output_cb().set(FUNC(play_3_state::q4013a_w));
	m_4013a->comp_output_cb().set(m_4013a, FUNC(ttl7474_device::d_w));

	TTL7474(config, m_4013b, 0);
	m_4013b->output_cb().set(m_maincpu, FUNC(cosmac_device::ef2_w)).invert(); // inverted
	m_4013b->comp_output_cb().set(m_maincpu, FUNC(cosmac_device::int_w)).invert(); // inverted

	/* Sound */
	genpin_audio(config);

	CDP1802(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &play_3_state::play_3_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &play_3_state::play_3_audio_io);
	m_audiocpu->wait_cb().set_constant(1);
	m_audiocpu->clear_cb().set(FUNC(play_3_state::clear_a_r));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AY8910(config, m_aysnd1, 3.579545_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "lspeaker", 0.75);
	AY8910(config, m_aysnd2, 3.579545_MHz_XTAL / 2).add_route(ALL_OUTPUTS, "rspeaker", 0.75);
}

void play_3_state::megaaton(machine_config &config)
{
	play_3(config);

	m_maincpu->set_clock(2.95_MHz_XTAL);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::megaaton_io);
}

void play_3_state::sklflite(machine_config &config)
{
	play_3(config);
	m_maincpu->set_addrmap(AS_IO, &play_3_state::sklflite_io);

	config.device_remove("audiocpu");
	config.device_remove("aysnd1");
	config.device_remove("aysnd2");
	config.device_remove("lspeaker");
	config.device_remove("rspeaker");

	EFO_ZSU1(config, m_zsu, 0);
}


/*-------------------------------------------------------------------
/ Spain 82 (10/82)
/-------------------------------------------------------------------*/
ROM_START(spain82)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("spaic12.bin", 0x0000, 0x1000, CRC(cd37ecdc) SHA1(ff2d406b6ac150daef868121e5857a956aabf005))
	ROM_LOAD("spaic11.bin", 0x1000, 0x0800, CRC(c86c0801) SHA1(1b52539538dae883f9c8fe5bc6454f9224780d11))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("spasnd.bin", 0x0000, 0x2000, CRC(62412e2e) SHA1(9e48dc3295e78e1024f726906be6e8c3fe3e61b1))
ROM_END

/*-------------------------------------------------------------------
/ Meg Aaton
/-------------------------------------------------------------------*/
ROM_START(megaaton)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpumegat.bin", 0x0000, 0x2000, CRC(7e7a4ede) SHA1(3194b367cbbf6e0cb2629cd5d82ddee6fe36985a))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
ROM_END

ROM_START(megaatona)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("mega_u12.bin", 0x0000, 0x1000, CRC(65761b02) SHA1(dd9586eaf70698ef7a80ce1be293322f64829aea))
	ROM_LOAD("mega_u11.bin", 0x1000, 0x1000, CRC(513f3683) SHA1(0f080a33426df1ffdb14e9b2e6382304e201e335))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
ROM_END

/*-------------------------------------------------------------------
/ ??/84 Nautilus
/-------------------------------------------------------------------*/
ROM_START(nautilus)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("nautilus.rom", 0x0000, 0x2000, CRC(197e5492) SHA1(0f83fc2e742fd0cca0bd162add4bef68c6620067))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("nautilus.snd", 0x0000, 0x2000, CRC(413d110f) SHA1(8360f652296c46339a70861efb34c41e92b25d0e)) // Bad?
ROM_END

/*-------------------------------------------------------------------
/ ??/84 The Raid
/-------------------------------------------------------------------*/
ROM_START(theraid)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("theraid.rom", 0x0000, 0x2000, CRC(97aa1489) SHA1(6b691b287138cc78cfc1010f380ff8c66342c39b))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("theraid.snd", 0x0000, 0x2000, CRC(e33f8363) SHA1(e7f251c334b15e12b1eb7e079c2e9a5f64338052))
ROM_END

/*-------------------------------------------------------------------
/ 11/84 UFO-X
/-------------------------------------------------------------------*/
ROM_START(ufo_x)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("ufoxcpu.rom", 0x0000, 0x2000, CRC(cf0f7c52) SHA1(ce52da05b310ac84bdd57609e21b0401ee3a2564))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("ufoxu3.rom", 0x0000, 0x2000, CRC(6ebd8ee1) SHA1(83522b76a755556fd38d7b292273b4c68bfc0ddf))
	ROM_LOAD("ufoxu4.rom", 0x2000, 0x0800, CRC(aa54ede6) SHA1(7dd7e2852d42aa0f971936dbb84c7708727ce0e7))
ROM_END

/*-------------------------------------------------------------------
/ KZ-26 (1984)
/-------------------------------------------------------------------*/
ROM_START(kz26)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("kz26.cpu", 0x0000, 0x2000, CRC(8030a699) SHA1(4f86b325801d8ce16011f7b6ba2f3633e2f2af35))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound1.su3", 0x0000, 0x2000, CRC(8ad1a804) SHA1(6177619f09af4302ffddd8c0c1b374dab7f47e91))
	ROM_LOAD("sound2.su4", 0x2000, 0x0800, CRC(355dc9ad) SHA1(eac8bc27157afd908f9bc5b5a7c40be5b9427269))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Rock 2500
/-------------------------------------------------------------------*/
ROM_START(rock2500)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("r2500cpu.rom", 0x0000, 0x2000, CRC(9c07e373) SHA1(5bd4e69d11e69fdb911a6e65b3d0a7192075abc8))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("r2500snd.rom", 0x0000, 0x2000, CRC(24fbaeae) SHA1(20ff35ed689291f321e483287a977c02e84d4524))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Star Fire
/-------------------------------------------------------------------*/
ROM_START(starfirp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("starfcpu.rom", 0x0000, 0x2000, CRC(450ddf20) SHA1(c63c4e3833ffc1f69fcec39bafecae9c80befb2a))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
ROM_END

ROM_START(starfirpa)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("starcpua.rom", 0x0000, 0x2000, CRC(29bac350) SHA1(ab3e3ea4881be954f7fa7278800ffd791c4581da))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
ROM_END

/*-------------------------------------------------------------------
/ Trailer (1985)
/-------------------------------------------------------------------*/
ROM_START(trailer)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("trcpu.rom", 0x0000, 0x2000, CRC(cc81f84d) SHA1(7a3282a47de271fde84cfddbaceb118add0df116))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("trsndu3.rom", 0x0000, 0x2000, CRC(05975c29) SHA1(e54d3a5613c3e39fc0338a53dbadc2e91c09ffe3))
	ROM_LOAD("trsndu4.rom", 0x2000, 0x0800, CRC(bda2a735) SHA1(134b5abb813ed8bf2eeac0861b4c88c7176582d8))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Stop Ship
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ ??/86 Flash Dragon
/-------------------------------------------------------------------*/
ROM_START(fldragon)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("fldrcpu1.rom", 0x0000, 0x2000, CRC(e513ded0) SHA1(64ed3dcff53311fb93bd50d105a4c1186043fdd7))
	ROM_LOAD("fldraudiocpu.rom", 0x2000, 0x2000, CRC(6ff2b276) SHA1(040b614f0b0587521ef5550b5587b94a7f3f178b))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("fdsndu3.rom", 0x0000, 0x2000, CRC(aa9c52a8) SHA1(97d5d63b14d10c70a5eb80c08ccf5a1f3df7596d))
	ROM_LOAD("fdsndu4.rom", 0x2000, 0x0800, CRC(0a7dc1d2) SHA1(32c7be5e9fbe4fa9ca661af7b7b5ea13ef250ce6))
ROM_END

ROM_START(fldragona)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("fldr_1a.cpu", 0x0000, 0x2000, CRC(21fda8e8) SHA1(feea608c2605cea1cdf9f7ed884297a95993f754))
	ROM_LOAD("fldr_2a.cpu", 0x2000, 0x2000, CRC(3592a0b7) SHA1(4c4ed7930dcbbf81ce2e5296c0b36bb615bd2270))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("fdsndu3.rom", 0x0000, 0x2000, CRC(aa9c52a8) SHA1(97d5d63b14d10c70a5eb80c08ccf5a1f3df7596d))
	ROM_LOAD("fdsndu4.rom", 0x2000, 0x0800, CRC(0a7dc1d2) SHA1(32c7be5e9fbe4fa9ca661af7b7b5ea13ef250ce6))
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Skill Flight
/-------------------------------------------------------------------*/
ROM_START(sklflite)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("skflcpu1.rom", 0x0000, 0x2000, CRC(8f833b55) SHA1(1729203582c22b51d1cc401aa8f270aa5cdadabe))
	ROM_LOAD("skflaudiocpu.rom", 0x2000, 0x2000, CRC(ffc497aa) SHA1(3e88539ae1688322b9268f502d8ca41cffb28df3))

	ROM_REGION(0x28000, "zsu:soundcpu", 0) // Z80A soundcard
	ROM_LOAD("skflsnd.rom", 0x0000, 0x8000, CRC(926a1da9) SHA1(16c762fbfe6a55597f26ff55d380192bb8647ee0))
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Phantom Ship
/-------------------------------------------------------------------*/
ROM_START(phntmshp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("video1.bin", 0x0000, 0x2000, CRC(2b61a8d2) SHA1(1b5cabbab252b2ffb6ed12fb7e4181de7695ed9a))
	ROM_LOAD("video2.bin", 0x2000, 0x2000, CRC(50126db1) SHA1(58d89e44131554cb087c4cad62869f90366704ad))

	ROM_REGION(0x28000, "zsu:soundcpu", 0) // Z80A soundcard
	ROM_LOAD("sonido1.bin", 0x00000, 0x8000, CRC(3294611d) SHA1(5f790b41bcb6d87418c80e61ac8ae69c57864b1d))
	ROM_LOAD("sonido2.bin", 0x08000, 0x8000, CRC(c2efc826) SHA1(44ee144b902627745853011968e0d654b35b3b08))
	ROM_LOAD("sonido3.bin", 0x10000, 0x8000, CRC(13d50f39) SHA1(70624de2dd8412c83866183a83f16cc5b8bdccb8))
	ROM_LOAD("sonido4.bin", 0x18000, 0x8000, CRC(b53f73ed) SHA1(bb928cfee418e8d9698d7bee78a32426f793c6e9))
ROM_END

/*-------------------------------------------------------------------
/ ??/86 Rider's Surf (JocMatic)
/-------------------------------------------------------------------*/
ROM_START(ridersrf)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpu.bin", 0x0000, 0x2000, CRC(4941938e) SHA1(01e44054e65166d68602d6a38217eda7ea669761))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.bin", 0x0000, 0x2000, CRC(2db2ecb2) SHA1(365fcac208607acc3e134affeababd6c89dbc74d))
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Iron Balls (Stargame)
/-------------------------------------------------------------------*/
ROM_START(ironball)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("video.bin", 0x0000, 0x2000, CRC(1867ebff) SHA1(485e46c742d914febcbdd58cb5a886f1d773282a))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sound.bin", 0x0000, 0x2000, CRC(83165483) SHA1(5076e5e836105d69c4ba606d8b995ecb16f88504))
ROM_END

/*-------------------------------------------------------------------
/ ??/83 Miss Disco (Bingo machine)
/-------------------------------------------------------------------*/
ROM_START(msdisco)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x1000, CRC(06fb7da9) SHA1(36c6fda166b2a07a5ed9ad5d2b6fdfe8fd707b0f))

	ROM_REGION(0x4000, "audiocpu", ROMREGION_ERASEFF)
ROM_END

// Eight Ball Champ (Maibesa) on EFO "Z-Pinball" hardware - very different from the Bally original
// actual year uncertain; schematic in manual says hardware was designed in 1986, so probably not 1985 as claimed
ROM_START(eballchps)
	ROM_REGION(0x8000, "maincpu", 0) // Z80-based
	ROM_LOAD("u18-jeb 5a0 - cpu.bin", 0x0000, 0x8000, CRC(87615a7d) SHA1(b27ca2d863040a2641f88f9bd3143467a83f181b))

	ROM_REGION(0x28000, "zsu:soundcpu", 0) // Z80-based
	ROM_LOAD("u3-ebe a02 - sonido.bin", 0x00000, 0x8000, CRC(34be32ee) SHA1(ce0271540164639f28d617753760ecc479b6b0d0))
	ROM_LOAD("u4-ebe b01 - sonido.bin", 0x08000, 0x8000, CRC(d696c4e8) SHA1(501e18c258e6d42819d25d72e1907984a6cfeecb))
	ROM_LOAD("u5-ebe c01 - sonido.bin", 0x10000, 0x8000, CRC(fe78d7ef) SHA1(ed91c51dd230854a007f88446011f786759687ca))
	ROM_LOAD("u6-ebe d02 - sonido.bin", 0x18000, 0x8000, CRC(a507081b) SHA1(72d025852a12f455981c61a405f97eaaac9c6fac))
ROM_END

// Cobra (Playbar)
ROM_START(cobrapb)
	ROM_REGION(0x8000, "maincpu", 0) // Z80-based
	ROM_LOAD("u18 - jcb 4 a0 - cpu.bin", 0x0000, 0x8000, CRC(c663910e) SHA1(c38692343f114388259c4e7b7943e5be934189ca))

	ROM_REGION(0x28000, "zsu:soundcpu", 0) // Z80-based
	ROM_LOAD("u3 - scb 1 a0 - sonido.bin", 0x00000, 0x8000, CRC(d3675770) SHA1(882ce748308f2d78cccd28fc8cd64fe69bd223e3))
	ROM_LOAD("u4 - scb 1 b0 - sonido.bin", 0x08000, 0x8000, CRC(e8e1bdbb) SHA1(215bdfab751cb0ea47aa529df0ac30976de4f772))
	ROM_LOAD("u5 - scb 1 c0 - sonido.bin", 0x10000, 0x8000, CRC(c36340ab) SHA1(cd662457959de3a929ba02779e2046ed18b797e2))
ROM_END

// Come Back (Nondum)
ROM_START(comeback)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("jeb_5a0.u18", 0x0000, 0x8000, CRC(87615a7d) SHA1(b27ca2d863040a2641f88f9bd3143467a83f181b))

	ROM_REGION(0x28000, "zsu:soundcpu", 0)
	ROM_LOAD("cbs_3a0.u3", 0x00000, 0x8000, CRC(d0f55dc9) SHA1(91186e2cbe248323380418911240a9a5887063fb))
	ROM_LOAD("cbs_3b0.u4", 0x08000, 0x8000, CRC(1da16d36) SHA1(9f7a27ae23064c1183a346ff042e6cba148257c7))
	ROM_LOAD("cbs_1c0.u5", 0x10000, 0x8000, CRC(794ae588) SHA1(adaa5e69232523369a6a2da865ac05102cc04ec8))
ROM_END


GAME(1982,  spain82,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Spain '82",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1983,  megaaton,  0,        megaaton, megaaton, play_3_state, empty_init, ROT0, "Playmatic", "Meg-Aaton",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1983,  megaatona, megaaton, megaaton, megaaton, play_3_state, empty_init, ROT0, "Playmatic", "Meg-Aaton (alternate set)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1984,  nautilus,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Nautilus",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1984,  theraid,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "The Raid",                     MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1984,  ufo_x,     0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "UFO-X",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1984,  kz26,      0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "KZ-26",                        MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1985,  rock2500,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Rock 2500",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1985,  starfirp,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Star Fire",                    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1985,  starfirpa, starfirp, play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Star Fire (alternate set)",    MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1985,  trailer,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Trailer",                      MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1986,  fldragon,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Flash Dragon",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1986,  fldragona, fldragon, play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Flash Dragon (alternate set)", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1987,  phntmshp,  0,        sklflite, play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Phantom Ship",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1987,  sklflite,  0,        sklflite, play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Skill Flight (Playmatic)",     MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// not by Playmatic, but same hardware
GAME(1986,  ridersrf,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "JocMatic",  "Rider's Surf",                 MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
GAME(1987,  ironball,  0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Stargame",  "Iron Balls",                   MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
// "Z-Pinball" hardware, Z80 main and sound CPUs - to be split (?)
GAME(1986,  eballchps, eballchp, sklflite, play_3,   play_3_state, empty_init, ROT0, "Bally (Maibesa license)", "Eight Ball Champ (Spain, Z-Pinball hardware)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987,  cobrapb,   0,        sklflite, play_3,   play_3_state, empty_init, ROT0, "Playbar",   "Cobra (Playbar)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(198?,  comeback, 0,         sklflite, play_3,   play_3_state, empty_init, ROT0, "Nondum / CIFA", "Come Back",                MACHINE_IS_SKELETON_MECHANICAL)
// bingo hardware, to be split (?)
GAME(1983,  msdisco,   0,        play_3,   play_3,   play_3_state, empty_init, ROT0, "Playmatic", "Miss Disco (Bingo)",           MACHINE_IS_SKELETON_MECHANICAL)

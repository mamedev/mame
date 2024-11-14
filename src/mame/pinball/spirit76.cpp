// license:BSD-3-Clause
// copyright-holders:Robbbert
// PINBALL
/*********************************************************************************************************************
Skeleton driver for Mirco's Spirit of 76, one of the first if not the first commercial solid-state pinball game.
Hardware listing and ROM definitions from PinMAME.

No schematic of the CPU or sound boards has been located, so this is largely guesswork.

   Hardware:
CPU:   1 x M6800
IO:    1x PIA 6821
SOUND: Knocker, 3 Chimes (10, 100, 1000) 10 has highest tone.
LED DISPLAYS: 2x 6-digit displays (player scores), 2x 2-digit displays (in Credits area - Credits and Match).
OTHER INDICATORS:  5 red leds for the ball in play
  Tilt light
  Game over light
  5x lights for Drum Bonus, 13 lights for the Cannon
Settings are done with 2x 8 dipswitches
Switch at G6 (sw1-4 = free game scores, sw5-8 = config), at G8 (coin chute settings)

2021-08-29 Game mostly working [Robbbert]

Status:
- Playable

TODO:
- Outputs (lamps)
- After starting a 2-player game, the credit button will let you waste away your remaining credits.
- The 240k/280k preset high score free games are actually 200k.
- The settings for the right-hand coin slot are ignored; both slots use the left-hand slot's settings.
- Find tilt input
- There's supposed to be a boom box and sound card in the cabinet to make various noisy effects, but
  no examples have been found.


*********************************************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "spirit76.lh"

namespace {

class spirit76_state : public genpin_class
{
public:
	spirit76_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_leds(*this, "led%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void spirit76(machine_config &config);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	void porta_w(u8 data);
	void portb_w(u8 data);
	u8 porta_r();
	u8 sw_r();
	void maincpu_map(address_map &map) ATTR_COLD;

	u8 m_t_c = 0U;
	u8 m_strobe = 0U;
	u8 m_segment = 0U;
	u8 m_last_solenoid[2]{ };
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_ioport_array<16> m_io_keyboard;
	output_finder<16> m_digits;
	output_finder<8> m_leds;
	output_finder<16> m_io_outputs;
};

void spirit76_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xfff);
	map(0x0000, 0x00ff).ram(); // 2x 2112
	map(0x0200, 0x0203).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // 6820
	map(0x0400, 0x0400).r(FUNC(spirit76_state::sw_r));
	map(0x0600, 0x0fff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( spirit76 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Upper right advance bonus")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Middle right advance bonus")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Lower right collect bonus")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Right 50pt wing target")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Right sling")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Lower right advance bonus")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("6 rollover")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Upper right target")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Lower right target")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Right 100pt wing target")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Right bumper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("2nd 7 rollover")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Upper eagle targets")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Lower right switch")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Right 500 wing target")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("Left 1000pt target")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Left 500pt wing target")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Centre bumper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("1st 7 rollover")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Mystery 1000pt")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Left 100pt wing target")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Left bumper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Lower left switch")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("1 rollover")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Left sling")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Lower left target")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Left 50pt wing target")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Upper left target")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("Lower left advance bonus")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Upper left advance bonus")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Middle left advance bonus")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Lower left collect bonus")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("X12")
	PORT_DIPNAME( 0x08, 0x00, "Preset score award allowed")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x00, "Match")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "Add-a-ball")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x01, 0x01, "Balls")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x01, "3")

	PORT_START("X13")
	PORT_DIPNAME( 0x0f, 0x0f, "Preset score")
	PORT_DIPSETTING(    0x0f, "40k") // more free games at 70k, 100k - undocumented
	PORT_DIPSETTING(    0x0e, "50k")
	PORT_DIPSETTING(    0x0d, "60k")
	PORT_DIPSETTING(    0x0c, "70k")
	PORT_DIPSETTING(    0x0b, "80k")
	PORT_DIPSETTING(    0x0a, "90k")
	PORT_DIPSETTING(    0x09, "100k")
	PORT_DIPSETTING(    0x08, "120k")
	PORT_DIPSETTING(    0x07, "140k")
	PORT_DIPSETTING(    0x06, "160k")
	PORT_DIPSETTING(    0x05, "180k")
	PORT_DIPSETTING(    0x04, "200k")
	PORT_DIPSETTING(    0x03, "240k") // actually 200k
	PORT_DIPSETTING(    0x02, "280k") // actually 200k
	PORT_DIPSETTING(    0x01, "320k")
	PORT_DIPSETTING(    0x00, "400k")

	PORT_START("X14")
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage Left Slot")
	PORT_DIPSETTING(    0x0f, "1C_1C")
	PORT_DIPSETTING(    0x0e, "1C_2C")
	PORT_DIPSETTING(    0x0d, "1C_3C")
	PORT_DIPSETTING(    0x0c, "1C_4C")

	PORT_START("X15")
	PORT_DIPNAME( 0x0f, 0x0f, "Coinage Right Slot")   // has no effect - it does what left slot does
	PORT_DIPSETTING(    0x0f, "1C_1C")
	PORT_DIPSETTING(    0x0e, "1C_2C")
	PORT_DIPSETTING(    0x0d, "1C_3C")
	PORT_DIPSETTING(    0x0c, "1C_4C")
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER( spirit76_state::irq )
{
	m_t_c++;
	if (m_t_c > 0x13)
		m_t_c = 0x10;
	if (m_t_c > 0x10)
	{
		if (m_t_c < 0x12)
			m_maincpu->set_input_line(M6800_IRQ_LINE, ASSERT_LINE);
		else
			m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
	}
}


void spirit76_state::porta_w(u8 data)
{
	m_segment = data & 15;
}

void spirit76_state::portb_w(u8 data)
{
	static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 }; // unknown decoder that blanks out 10-15
	// DISPLAYS
	if (BIT(data, 7))
	{
		m_strobe = data & 15;
		if (m_strobe == 7) // indicate player-up
		{
			if (BIT(m_segment, 0))
			{
				m_leds[5] = BIT(m_segment, 2);
				m_leds[6] = !BIT(m_segment, 2);
				m_segment = 15;  // blank the match
			}
			else
			{
				m_leds[5] = 0;
				m_leds[6] = 0;
				m_segment = 0;
			}
		}
		else
		if (m_strobe == 5) // indicate ball in play
		{
			u8 ball = 15;
			switch (m_segment)
			{
				case 1: ball = 0; break;
				case 7: ball = 1; break;
				case 3: ball = 2; break;
				case 2: ball = 3; break;
				case 8: ball = 4; break;
				default:
					break;
			}
			for (u8 i = 0; i < 5; i++)
				m_leds[i] = (ball == i);
			m_segment = 0;
		}

		m_digits[m_strobe] = patterns[m_segment & 15];
	}

	// SOLENOIDS
	// Some solenoids get continuously pulsed, which is absorbed by the real thing, but
	// causes issues for us. So we need to use only the first occurrence of a particular sound.
	if (BIT(data, 5))
	{
		data &= 15;
		if (data == 0)
		{
			if (m_last_solenoid[0])
				m_io_outputs[m_last_solenoid[0]] = 0;   // turn off last solenoid
			m_last_solenoid[1] = m_last_solenoid[0];   // store it away
			m_last_solenoid[0] = 0;
		}
		else
		{
			m_last_solenoid[0] = data;
			if (m_last_solenoid[1] != data)
			{
				m_io_outputs[data] = 1;
				switch (data)
				{
					case 1: m_samples->start(3, 3); break; // 10 chime
					case 2: m_samples->start(2, 2); break; // 100 chime
					case 3: m_samples->start(1, 1); break; // 1000 chime
					case 4: case 14: m_samples->start(0, 6); break; // knocker
					//case 5: m_samples->start(x, 6); break;  // Right flipper
					//case 7: m_samples->start(x, 6); break;  // Left flipper
					case 8: case 9: case 10: m_samples->start(4, 0); break; // 3 bumpers
					case 11: case 12: m_samples->start(4, 7); break; // 2 slings
					case 13: m_samples->start(5, 5); break; // outhole
					default: break;
				}
			}
		}
	}
}

u8 spirit76_state::porta_r()
{
	return 0xff;
}

u8 spirit76_state::sw_r()
{
	return ~m_io_keyboard[m_strobe]->read();
}

void spirit76_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();
	m_io_outputs.resolve();
	save_item(NAME(m_t_c));
	save_item(NAME(m_strobe));
	save_item(NAME(m_segment));
	save_item(NAME(m_last_solenoid));
}

void spirit76_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_t_c = 0;
	m_strobe = 0;
	m_segment = 0;
	m_last_solenoid[0] = 0;
	m_last_solenoid[1] = 0;
}

void spirit76_state::spirit76(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &spirit76_state::maincpu_map);

	TIMER(config, "irq").configure_periodic(FUNC(spirit76_state::irq), attotime::from_hz(1200));

	/* video hardware */
	config.set_default_layout(layout_spirit76);

	//6821pia
	pia6821_device &pia(PIA6821(config, "pia"));
	pia.writepa_handler().set(FUNC(spirit76_state::porta_w));
	pia.writepb_handler().set(FUNC(spirit76_state::portb_w));
	pia.readpa_handler().set(FUNC(spirit76_state::porta_r));

	/* sound hardware */
	genpin_audio(config);
}


ROM_START(spirit76)
	ROM_REGION(0xa00, "maincpu", 0)
	ROM_LOAD_NIB_LOW("1g.bin",  0x0000, 0x0200, CRC(57d7213c) SHA1(0897876f5c662b2518a680bcbfe282bb3a19a161))
	ROM_LOAD_NIB_HIGH("5g.bin", 0x0000, 0x0200, CRC(90e22786) SHA1(da9e0eae1e8576c6c8ac734a9557784d9e59c141))
	ROM_LOAD_NIB_LOW("2c.bin",  0x0200, 0x0200, CRC(4b996a52) SHA1(c73378e61598f84e20c1022b811780e300b01cd1))
	ROM_LOAD_NIB_HIGH("3c.bin", 0x0200, 0x0200, CRC(448626fa) SHA1(658b9589ba60ef62ff692192f743038d622776ba))
	ROM_LOAD_NIB_LOW("2e.bin",  0x0400, 0x0200, CRC(faaa907e) SHA1(ee9227944911a7c068216dd7b1b8dec284f90e3b))
	ROM_LOAD_NIB_HIGH("3e.bin", 0x0400, 0x0200, CRC(3463168e) SHA1(d98643179eac5ecbf1a559df59da620ea544bdee))
	ROM_LOAD_NIB_LOW("2f.bin",  0x0600, 0x0200, CRC(4d1a71ec) SHA1(6d3aa8fc4f7cec27d7fae2ecc73425388f8d9d52))
	ROM_LOAD_NIB_HIGH("3f.bin", 0x0600, 0x0200, CRC(bf23f0fd) SHA1(62e2ef7df0c057f25685a99e57cf95aae2e75cdb))
	ROM_LOAD_NIB_LOW("2g.bin",  0x0800, 0x0200, CRC(6236f053) SHA1(6183c8fa7dbd32ec40c4668cab8010b5e8c49949))
	ROM_LOAD_NIB_HIGH("3g.bin", 0x0800, 0x0200, CRC(ae7192cd) SHA1(9ba76e81b8603163c22f47f1a99da310b4325e84))
ROM_END

} // Anonymous namespace

GAME( 1975, spirit76, 0, spirit76, spirit76, spirit76_state, empty_init, ROT0, "Mirco", "Spirit of 76", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

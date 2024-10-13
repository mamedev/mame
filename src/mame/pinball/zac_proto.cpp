// license:BSD-3-Clause
// copyright-holders:Robbbert
/*********************************************************************

PINBALL
Zaccaria Prototype

These use the INS8060 (SC/MP) processor, and are Zaccaria's first
  digital machines. The term "prototype" is misleading - they were
  normal production machines with a different computer board.

After starting a game, press X, wait for the sound of the ball
 being ejected, then press Z. Now you can get a score.

The ball number is how many balls are left. 1 = last ball.

Games:
- Strike
- Ski Jump
- Space City

Status:
- All games are working.

ToDo:
- Doesn't appear to have a knocker. To be confirmed.
- Is there a test mode? If so, how to get to it?

**********************************************************************/

#include "emu.h"
#include "machine/clock.h"
#include "genpin.h"
#include "sound/spkrdev.h"
#include "cpu/scmp/scmp.h"
#include "speaker.h"
#include "zac_proto.lh"

namespace {

class zac_proto_state : public genpin_class
{
public:
	zac_proto_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void zac_proto(machine_config &config);

private:
	void out0_w(offs_t offset, uint8_t data);
	void out1_w(offs_t offset, uint8_t data);
	void digit_w(offs_t offset, uint8_t data);
	void sound_w(offs_t offset, uint8_t data);
	void audio_clock(int state);
	int slam_r();
	void zac_proto_map(address_map &map) ATTR_COLD;
	u8 m_u36 = 0x80U;  // preset divider for u44/u45
	u8 m_u37 = 0U;  // selector for u48
	u8 m_u44u45 = 0U;  // counters for u44/u45
	u8 m_u46u47 = 0U;  // counters for u46/u47

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<scmp_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	output_finder<11> m_digits;
	output_finder<12+84> m_io_outputs;
};


void zac_proto_state::zac_proto_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0d00, 0x0dff).ram().share("nvram");
	map(0x0e00, 0x0e00).portr("X0");
	map(0x0e01, 0x0e01).portr("X1");
	map(0x0e02, 0x0e02).portr("X2");
	map(0x0e03, 0x0e03).portr("X3");
	map(0x0e04, 0x0e04).portr("X4");
	map(0x0e05, 0x0e05).portr("X5");
	map(0x0e06, 0x0e06).portr("X6");
	map(0x0e07, 0x0e07).portr("X7");
	map(0x0e00, 0x0e01).w(FUNC(zac_proto_state::out0_w));
	map(0x0e02, 0x0e06).w(FUNC(zac_proto_state::digit_w));
	map(0x0e07, 0x0e08).w(FUNC(zac_proto_state::sound_w));
	map(0x0e09, 0x0e16).w(FUNC(zac_proto_state::out1_w));
	map(0x1400, 0x17ff).rom();
}

static INPUT_PORTS_START( zac_proto )
	// playfield inputs
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Ball Ready")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("INP06")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("INP07")
	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("INP08")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("INP09")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("INP10")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("INP11")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("INP12")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("INP13")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INP14")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("INP15")
	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("INP16")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("INP17")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("INP18")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("INP19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("INP20")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("INP21")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("INP22")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("INP23")
	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("INP24")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("INP25")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("INP26")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("INP27")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("INP28")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("INP29")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP30")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP31")
	// None of the games respond to this row, even though the Strike manual says INP32 is used.
	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP32")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP33")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP34")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP35")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP36")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP37")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP38")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP39")

	// dipswitches
	PORT_START("X5")
	PORT_DIPNAME( 0x0f, 0x02, "Coinage Slot 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x09, "2 Coins/9 Credits" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0b, "2 Coins/11 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0d, "2 Coins/13 Credits" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0f, "2 Coins/15 Credits" )
	PORT_DIPNAME( 0xf0, 0x20, "Coinage Slot 2" )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x90, "2 Coins/9 Credits" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xb0, "2 Coins/11 Credits" )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0xd0, "2 Coins/13 Credits" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0xf0, "2 Coins/15 Credits" )

	PORT_START("X6")
	PORT_DIPNAME( 0x03, 0x01, "High Score" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Beat High Score" )
	PORT_DIPSETTING(    0x04, "Super Bonus" )
	PORT_DIPSETTING(    0x00, "Game" )
	PORT_DIPNAME( 0x08, 0x08, "Match" )
	PORT_DIPSETTING(    0x08, "Enabled" )
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPNAME( 0x30, 0x20, "Beat High/Random Score" )
	PORT_DIPSETTING(    0x00, "500000 points" )
	PORT_DIPSETTING(    0x10, "Extra Ball" )
	PORT_DIPSETTING(    0x20, "Extra Game" )
	PORT_DIPSETTING(    0x30, "Super Bonus" )
	PORT_DIPNAME( 0xc0, 0x80, "Reward for Special" )
	PORT_DIPSETTING(    0x00, "500000 points" )
	PORT_DIPSETTING(    0x40, "Extra Ball" )
	PORT_DIPSETTING(    0x80, "Extra Game" )
	PORT_DIPSETTING(    0xc0, "Super Bonus" )

	PORT_START("X7")
	PORT_DIPNAME( 0x01, 0x00, "Random" )
	PORT_DIPSETTING(    0x00, "Enabled" )
	PORT_DIPSETTING(    0x01, "Disabled" )
	PORT_DIPNAME( 0x06, 0x02, "Balls" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "7" )
	PORT_DIPNAME( 0x18, 0x10, "Strikes to get special" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPNAME( 0x20, 0x20, "Unlimited Specials" )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, "Bonus Ball Award" )
	PORT_DIPSETTING(    0x40, "Extra Ball" )
	PORT_DIPSETTING(    0x00, "200000 points" )
	PORT_DIPNAME( 0x80, 0x80, "SW24" )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_0)
INPUT_PORTS_END

int zac_proto_state::slam_r()
{
	return BIT(ioport("X8")->read(), 0);
}

// solenoids (not knocker)
void zac_proto_state::out0_w(offs_t offset, uint8_t data)
{
	data &= 0x3f;
	uint16_t t = data << (offset*6);

	switch (t)
	{
		case 0x01:
			m_samples->start(0, 5); // outhole
			break;
		case 0x02:
		case 0x04:
			m_samples->start(4, 7); // slings
			break;
		case 0x08:
		case 0x10:
		case 0x20:
			m_samples->start(5, 0); // bumpers
			break;
	}
	for (u8 i = 0; i < 6; i++)
		m_io_outputs[offset*6+i] = BIT(data, i);
}

// lamps
void zac_proto_state::out1_w(offs_t offset, uint8_t data)
{
	data &= 0x3f;
	for (u8 i = 0; i < 6; i++)
		m_io_outputs[12+offset*6+i] = BIT(data, i);
}

// need to implement blanking of leading zeroes
void zac_proto_state::digit_w(offs_t offset, uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 }; // 9368 (outputs 0-9,A-F)
	static const uint8_t decimals[10] = { 0, 0, 0x80, 0, 0, 0x80, 0, 0, 0, 0 };
	offset<<=1;
	m_digits[offset] = patterns[data&15] | decimals[offset];
	offset++;
	m_digits[offset] = patterns[data>>4] | decimals[offset];
}

void zac_proto_state::sound_w(offs_t offset, uint8_t data)
{
	data &= 0x3f;
	if (offset)
	{
		data ^= 0x0e;
		// sch labels shows bit 1 as enable, but pin numbers show bit 4 being enable.
		// we will go by the pin numbers.
		m_u37 = BIT(data, 1, 4);
		m_u36 = (m_u36 & 0x3f) | (BIT(data, 0) << 6) | 0x80;
	}
	else
	{
		m_u36 = (m_u36 & 0x40) | data | 0x80;
	}
}

void zac_proto_state::audio_clock(int state)
{
	if (state)
	{
		m_u44u45--;
		if (m_u44u45 == 0)
		{
			m_u44u45 = m_u36;
			m_u46u47--;
			// if sound muted set speaker to 0, otherwise use the selected frequency.
			m_speaker->level_w(BIT(m_u37, 3) ? (BIT(m_u46u47, m_u37 & 7) ? 1 : -1) : 0);
		}
	}
}

void zac_proto_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_u36));
	save_item(NAME(m_u37));
	save_item(NAME(m_u44u45));
	save_item(NAME(m_u46u47));
}

void zac_proto_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	m_digits[10] = 0x3f; // units shows zero all the time
}

void zac_proto_state::zac_proto(machine_config &config)
{
	/* basic machine hardware */
	INS8060(config, m_maincpu, XTAL(4'000'000) / 2); // Using SC/MP II chip which has an internal /2 circuit.
	m_maincpu->set_addrmap(AS_PROGRAM, &zac_proto_state::zac_proto_map);
	m_maincpu->sense_b().set(FUNC(zac_proto_state::slam_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_zac_proto);

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	clock_device &snd_clock(CLOCK(config, "sys_clock", XTAL(4'000'000) / 2));
	snd_clock.signal_handler().set(FUNC(zac_proto_state::audio_clock));
}

// The PCB has a space for another rom, U27, @0x1800, but it was never used.

/*--------------------------------
/ Strike (09/78)
/-------------------------------*/
ROM_START(strike)
	ROM_REGION(0x1800, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("strike1.u23", 0x0000, 0x0400, CRC(650abc54) SHA1(6a4f83016a38338ba6a04271532f0880264e61a7))
	ROM_LOAD("strike2.u24", 0x0400, 0x0400, CRC(13c5a168) SHA1(2da3a5bc0c28a2aacd8c1396dac95cf35f8797cd))
	ROM_LOAD("strike3.u25", 0x0800, 0x0400, CRC(ebbbf315) SHA1(c87e961c8e5e99b0672cd632c5e104ea52088b5d))
	ROM_LOAD("strike4.u26", 0x1400, 0x0400, CRC(ca0eddd0) SHA1(52f9faf791c56b68b1806e685d0479ea67aba019))
ROM_END

/*--------------------------------
/ Ski Jump (10/78)
/-------------------------------*/
ROM_START(skijump)
	ROM_REGION(0x1800, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("skijump1.u23", 0x0000, 0x0400, CRC(c0c0e18c) SHA1(d28ec2541f6c2e86e5b5514c7f9e558df68be72a))
	ROM_LOAD("skijump2.u24", 0x0400, 0x0400, CRC(b08aafb5) SHA1(ff6df4efa20a4461d525209a487d04896eeef29e))
	ROM_LOAD("skijump3.u25", 0x0800, 0x0400, CRC(9a8731c0) SHA1(9f7aaa8c6df04b925c8beff8b426c59bc3696f50))
	ROM_LOAD("skijump4.u26", 0x1400, 0x0400, CRC(fa064b51) SHA1(d4d02ca661e4084805f00247f31c0701320ab62d))
ROM_END

/*--------------------------------
/ Space City (09/79)
/-------------------------------*/
ROM_START(spacecty)
	ROM_REGION(0x1800, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("zsc1.u23", 0x0000, 0x0400, CRC(4405368f) SHA1(037ad7e7158424bb714b28e4effa2c96c8736ce4))
	ROM_LOAD("zsc2.u24", 0x0400, 0x0400, CRC(a6c41475) SHA1(7d7d851efb2db7d9a1988265cdff676260d753c3))
	ROM_LOAD("zsc3.u25", 0x0800, 0x0400, CRC(e6a2dcee) SHA1(d2dfff896ae90208c28179f9bbe43f93d7f2131c))
	ROM_LOAD("zsc4.u26", 0x1400, 0x0400, CRC(69e0bb95) SHA1(d9a1d0159bf49445b0ece0f9d7806ed80657c2b2))
ROM_END

} // anonymous namespace

GAME(1978,  strike,    0,  zac_proto,  zac_proto, zac_proto_state, empty_init, ROT0, "Zaccaria", "Strike",     MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1978,  skijump,   0,  zac_proto,  zac_proto, zac_proto_state, empty_init, ROT0, "Zaccaria", "Ski Jump",   MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1979,  spacecty,  0,  zac_proto,  zac_proto, zac_proto_state, empty_init, ROT0, "Zaccaria", "Space City", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

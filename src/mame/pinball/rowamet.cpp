// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************

PINBALL
Rowamet : Heavy Metal

PinMAME used as reference (couldn't find a manual)
Seems to be almost the same as Taito. Some code copied from taito.cpp

Known games from this company: Solar Ride, Vulcan IV, Sherokee, Jet Surf, Diana,
 Heavy Metal, Conan. Most of them are TTL (no CPU).

Status:
- Working but many sounds are missing.
- When starting a game, hold down X and press 1.

ToDO:
- Outputs
- Bad sound rom
- In PinMAME, the display cycles between various attract modes. This doesn't happen
  in MAME.
- Dips copied from Taito, but they don't respond.

*************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "speaker.h"

#include "rowamet.lh"

namespace {

class rowamet_state : public genpin_class
{
public:
	rowamet_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void rowamet(machine_config &config);

private:
	u8 sound_r();
	void mute_w(u8 data);
	u8 io_r(offs_t offset);
	void io_w(offs_t offset, u8 data);
	void main_mem_map(address_map &map) ATTR_COLD;
	void audio_mem_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;

	u8 m_sndcmd = 0xffU;
	u8 m_io[32]{};
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	output_finder<32> m_digits;
	output_finder<96> m_io_outputs;   // 32 solenoids + 64 lamps
};


void rowamet_state::main_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2800, 0x2800).mirror(0xf8).portr("X0");
	map(0x2801, 0x2801).mirror(0xf8).portr("X1");
	map(0x2802, 0x2802).mirror(0xf8).portr("X2");
	map(0x2803, 0x2803).mirror(0xf8).portr("X3");
	map(0x2804, 0x2804).mirror(0xf8).portr("X4");
	map(0x2805, 0x2805).mirror(0xf8).portr("X5");
	map(0x2806, 0x2806).mirror(0xf8).portr("X6");
	map(0x2807, 0x2807).mirror(0xf8).portr("X7");
	map(0x4000, 0x40ff).ram();
	map(0x4080, 0x409f).rw(FUNC(rowamet_state::io_r), FUNC(rowamet_state::io_w));
}

void rowamet_state::audio_mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x17ff).ram();
}

void rowamet_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(rowamet_state::sound_r), FUNC(rowamet_state::mute_w));
	map(0x01, 0x01).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x81, 0x81).nopw();
}

static INPUT_PORTS_START( rowamet )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_DIPNAME( 0x10, 0x10, "Service")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Porta") //?? does nothing?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Caixa") //?? does nothing?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP17")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP20")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP23")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP27")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP37")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP53")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP54")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP55")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP56")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP57")

	PORT_START("X6")
	PORT_DIPNAME( 0x01, 0x01, "Code D0")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "Code D1")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "Code D2")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "Code D3")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "Code D4")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "Code D5")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "Code D6")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPNAME( 0x80, 0x80, "Code D7")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x80, DEF_STR(Off))

	PORT_START("X7")
	PORT_DIPNAME( 0x01, 0x01, "Diagnose")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "Statistics")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "Adjust")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "Conform")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "Enter")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "Sem")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


u8 rowamet_state::sound_r()
{
	return m_sndcmd;
}

void rowamet_state::mute_w(u8 data)
{
	machine().sound().system_mute(data != 0);
}

u8 rowamet_state::io_r(offs_t offset)
{
	return m_io[offset];
}

void rowamet_state::io_w(offs_t offset, u8 data)
{
	m_io[offset] = data;

	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // don't know, 7446 assumed
	if (offset < 0x10)
	{
		m_digits[offset*2] = patterns[data>>4];
		m_digits[offset*2+1] = patterns[data&15];
		return;
	}

	offset &= 15;

	// offsets 0 and 1 use high nybble for solenoids
	if (offset < 2)
		for (u8 i = 0; i < 4; i++)
			m_io_outputs[offset*4+i] = BIT(data, i+4);

	// everything uses low nybble for general outputs
	for (u8 i = 0; i < 4; i++)
		m_io_outputs[8+offset*4+i] = BIT(data, i);

	// offsets 2 and 3 use high nybble for sound
	u8 cmd = BIT(m_io[0x12], 4, 4) | (m_io[0x13] & 0xf0);
	if (offset == 2) // does offset 3 trigger sound too?
	{
		if (cmd != m_sndcmd)
		{
			m_sndcmd = cmd;
			m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}
	if ((offset == 1) && (data == 0x10))
		m_samples->start(0, 5); // outhole
}

void rowamet_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_sndcmd));
}

void rowamet_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_sndcmd = 0xff;
	std::fill(std::begin(m_io), std::end(m_io), 0);
}


void rowamet_state::rowamet(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 1888888);
	m_maincpu->set_addrmap(AS_PROGRAM, &rowamet_state::main_mem_map);

	Z80(config, m_audiocpu, 1888888);
	m_audiocpu->set_addrmap(AS_PROGRAM, &rowamet_state::audio_mem_map);
	m_audiocpu->set_addrmap(AS_IO, &rowamet_state::audio_io_map);

	/* Video */
	config.set_default_layout(layout_rowamet);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.75); // unknown DAC
}

/*-------------------------------------------------------------------
/ Conan (1983)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Heavy Metal (????)
/-------------------------------------------------------------------*/
ROM_START(heavymtl)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("hvymtl_c.bin", 0x0000, 0x1000, CRC(8f36d3da) SHA1(beec79c5d794ede96d95105bad7466b67762606d))
	ROM_LOAD("hvymtl_b.bin", 0x1000, 0x1000, CRC(357f1252) SHA1(ddc55ded0dc1c8632c31d809bfadfb45ae248cfd))

	ROM_REGION(0x1000, "audiocpu", 0)
	ROM_LOAD("hvymtl_s.bin", 0x0000, 0x1000, BAD_DUMP CRC(c525e6cb) SHA1(144e06fbbdd1f3e45ccca8bace6b04f876b1312c))
	ROM_FILL(0x0000, 1, 0xaf) // bad byte
	ROM_FILL(0x0551, 1, 0xdd) // another bad byte
ROM_END

/*-------------------------------------------------------------------
/ Vulcan IV (1982)
/-------------------------------------------------------------------*/

} // Anonymous namespace

GAME(198?, heavymtl, 0, rowamet, rowamet, rowamet_state, empty_init, ROT0, "Rowamet", "Heavy Metal", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

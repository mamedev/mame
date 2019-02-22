// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************

  PINBALL
  Rowamet : Heavy Metal

  PinMAME used as reference (couldn't find a manual)
  Seems to be almost the same as Taito.

  Known games from this company: Solar Ride, Vulcan IV, Sherokee, Jet Surf, Diana,
                                 Heavy Metal, Conan.

  You need to have a ball in the outhole (hold down X) when starting a game.

ToDO:
- Outputs
- Bad sound rom
- In PinMAME, the display cycles between various attract modes. This doesn't happen
  in MAME.
- Display flickers ingame

*************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "rowamet.lh"


class rowamet_state : public driver_device
{
public:
	rowamet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cpu2(*this, "cpu2")
		, m_p_ram(*this, "ram")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void rowamet(machine_config &config);

private:
	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(mute_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
	void rowamet_map(address_map &map);
	void rowamet_sub_io(address_map &map);
	void rowamet_sub_map(address_map &map);

	uint8_t m_out_offs;
	uint8_t m_sndcmd;
	uint8_t m_io[16];
	virtual void machine_reset() override;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_shared_ptr<uint8_t> m_p_ram;
	output_finder<32> m_digits;
};


void rowamet_state::rowamet_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("roms", 0);
	map(0x2800, 0x2800).portr("X0");
	map(0x2801, 0x2801).portr("X1");
	map(0x2802, 0x2802).portr("X2");
	map(0x2803, 0x2803).portr("X3");
	map(0x2804, 0x2804).portr("X4");
	map(0x2805, 0x2805).portr("X5");
	map(0x2806, 0x2806).portr("X6");
	map(0x2807, 0x2807).portr("X7");
	map(0x2808, 0x2808).portr("X8");
	map(0x4000, 0x407f).ram();
	map(0x4080, 0x408f).ram().share("ram");
	map(0x4090, 0x409f).rw(FUNC(rowamet_state::io_r), FUNC(rowamet_state::io_w));
	map(0x40a0, 0x40ff).ram();
}

void rowamet_state::rowamet_sub_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("roms", 0x2000);
	map(0x1000, 0x17ff).ram();
}

void rowamet_state::rowamet_sub_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(rowamet_state::sound_r), FUNC(rowamet_state::mute_w));
	map(0x01, 0x01).w("dac", FUNC(dac_byte_interface::data_w));
}

static INPUT_PORTS_START( rowamet )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT ) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)

	// from here might be dipswitches
	PORT_START("X6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


READ8_MEMBER( rowamet_state::sound_r )
{
	return m_sndcmd;
}

WRITE8_MEMBER( rowamet_state::mute_w )
{
	machine().sound().system_enable(data ? 0 : 1);
}

READ8_MEMBER( rowamet_state::io_r )
{
	return m_io[offset];
}

WRITE8_MEMBER( rowamet_state::io_w )
{
	m_io[offset] = data;

	if (offset == 2)
	{
		uint8_t cmd = (m_io[2]>>4) | (m_io[3] & 0xf0);
		if (cmd != m_sndcmd)
		{
			m_sndcmd = cmd;
			m_cpu2->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		}
	}
}

void rowamet_state::machine_reset()
{
	uint8_t i;
	m_out_offs = 0;
	m_sndcmd = 0;
	for (i = 0; i < 16; i++)
		m_io[i] = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER( rowamet_state::timer_a )
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7446
	m_out_offs &= 15;

	uint8_t digit = m_out_offs << 1;
	m_digits[digit] = patterns[m_p_ram[m_out_offs]>>4];
	m_digits[++digit] = patterns[m_p_ram[m_out_offs++]&15];
}

void rowamet_state::rowamet(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 1888888);
	m_maincpu->set_addrmap(AS_PROGRAM, &rowamet_state::rowamet_map);

	Z80(config, m_cpu2, 1888888);
	m_cpu2->set_addrmap(AS_PROGRAM, &rowamet_state::rowamet_sub_map);
	m_cpu2->set_addrmap(AS_IO, &rowamet_state::rowamet_sub_io);

	TIMER(config, "timer_a").configure_periodic(FUNC(rowamet_state::timer_a), attotime::from_hz(200));

	/* Video */
	config.set_default_layout(layout_rowamet);

	/* Sound */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}

/*-------------------------------------------------------------------
/ Conan (1983)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Heavy Metal (????)
/-------------------------------------------------------------------*/
ROM_START(heavymtl)
	ROM_REGION(0x3000, "roms", 0)
	ROM_LOAD("hvymtl_c.bin", 0x0000, 0x1000, CRC(8f36d3da) SHA1(beec79c5d794ede96d95105bad7466b67762606d))
	ROM_LOAD("hvymtl_b.bin", 0x1000, 0x1000, CRC(357f1252) SHA1(ddc55ded0dc1c8632c31d809bfadfb45ae248cfd))
	ROM_LOAD("hvymtl_s.bin", 0x2000, 0x1000, BAD_DUMP CRC(c525e6cb) SHA1(144e06fbbdd1f3e45ccca8bace6b04f876b1312c))
	ROM_FILL(0x2000, 1, 0xaf) // bad byte
	ROM_FILL(0x2551, 1, 0xdd) // another bad byte
ROM_END

/*-------------------------------------------------------------------
/ Vulcan IV (1982)
/-------------------------------------------------------------------*/


GAME(198?, heavymtl, 0, rowamet, rowamet, rowamet_state, empty_init, ROT0, "Rowamet", "Heavy Metal", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

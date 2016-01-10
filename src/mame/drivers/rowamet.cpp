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

#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "rowamet.lh"

class rowamet_state : public driver_device
{
public:
	rowamet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cpu2(*this, "cpu2")
		, m_p_ram(*this, "ram")
	{ }

	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(mute_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
private:
	UINT8 m_out_offs;
	UINT8 m_sndcmd;
	UINT8 m_io[16];
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_shared_ptr<UINT8> m_p_ram;
};


static ADDRESS_MAP_START( rowamet_map, AS_PROGRAM, 8, rowamet_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("X0")
	AM_RANGE(0x2801, 0x2801) AM_READ_PORT("X1")
	AM_RANGE(0x2802, 0x2802) AM_READ_PORT("X2")
	AM_RANGE(0x2803, 0x2803) AM_READ_PORT("X3")
	AM_RANGE(0x2804, 0x2804) AM_READ_PORT("X4")
	AM_RANGE(0x2805, 0x2805) AM_READ_PORT("X5")
	AM_RANGE(0x2806, 0x2806) AM_READ_PORT("X6")
	AM_RANGE(0x2807, 0x2807) AM_READ_PORT("X7")
	AM_RANGE(0x2808, 0x2808) AM_READ_PORT("X8")
	AM_RANGE(0x4000, 0x407f) AM_RAM
	AM_RANGE(0x4080, 0x408f) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x4090, 0x409f) AM_READWRITE(io_r,io_w)
	AM_RANGE(0x40a0, 0x40ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rowamet_sub_map, AS_PROGRAM, 8, rowamet_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0x2000)
	AM_RANGE(0x1000, 0x17ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rowamet_sub_io, AS_IO, 8, rowamet_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(sound_r,mute_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("dac", dac_device, write_unsigned8)
ADDRESS_MAP_END

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
	machine().sound().system_enable(~data);
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
		UINT8 cmd = (m_io[2]>>4) | (m_io[3] & 0xf0);
		if (cmd != m_sndcmd)
		{
			m_sndcmd = cmd;
			m_cpu2->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		}
	}
}

void rowamet_state::machine_reset()
{
	UINT8 i;
	m_out_offs = 0;
	m_sndcmd = 0;
	for (i = 0; i < 16; i++)
		m_io[i] = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER( rowamet_state::timer_a )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7446
	m_out_offs &= 15;

	UINT8 digit = m_out_offs << 1;
	output().set_digit_value(digit, patterns[m_p_ram[m_out_offs]>>4]);
	output().set_digit_value(++digit, patterns[m_p_ram[m_out_offs++]&15]);
}

static MACHINE_CONFIG_START( rowamet, rowamet_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 1888888)
	MCFG_CPU_PROGRAM_MAP(rowamet_map)
	MCFG_CPU_ADD("cpu2", Z80, 1888888)
	MCFG_CPU_PROGRAM_MAP(rowamet_sub_map)
	MCFG_CPU_IO_MAP(rowamet_sub_io)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_a", rowamet_state, timer_a, attotime::from_hz(200))

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_rowamet)

	/* Sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

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


GAME(198?, heavymtl, 0, rowamet, rowamet, driver_device, 0,  ROT0,  "Rowamet", "Heavy Metal", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND )

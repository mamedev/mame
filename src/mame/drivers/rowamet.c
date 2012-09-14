/************************************************************************************

    Pinball
    Rowamet : Heavy Metal

    PinMAME used as reference (couldn't find a manual)


*************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
// #include "rowamet.lh"

class rowamet_state : public driver_device
{
public:
	rowamet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cpu2(*this, "cpu2"),
	m_p_ram(*this, "ram")
	{ }

	DECLARE_READ8_MEMBER(sound_r);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_WRITE8_MEMBER(mute_w);
	UINT8 m_out_offs;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_shared_ptr<UINT8> m_p_ram;

protected:

	// devices


	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(rowamet);
};


static ADDRESS_MAP_START( rowamet_map, AS_PROGRAM, 8, rowamet_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2800, 0x2808) AM_READ(switch_r)
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( rowamet_sub_map, AS_PROGRAM, 8, rowamet_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x17ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rowamet_sub_io, AS_IO, 8, rowamet_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(sound_r,mute_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("dac", dac_device, write_unsigned8)
ADDRESS_MAP_END

static INPUT_PORTS_START( rowamet )
INPUT_PORTS_END


READ8_MEMBER( rowamet_state::sound_r )
{
	return (m_p_ram[0x92] >> 4) | (m_p_ram[0x93] & 0xf0);
}

READ8_MEMBER( rowamet_state::switch_r )
{
	return 0;
}

WRITE8_MEMBER( rowamet_state::mute_w )
{
	machine().sound().system_enable(~data);
	printf("%X ",data);
}

//m_cpu2->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
void rowamet_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(rowamet_state,rowamet)
{
}

static TIMER_DEVICE_CALLBACK( rowamet_timer )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0, 0, 0, 0, 0, 0 };
	rowamet_state *state = timer.machine().driver_data<rowamet_state>();
	state->m_out_offs++;
	if (state->m_out_offs > 0x9f)
		state->m_out_offs = 0x80;

	if (state->m_out_offs < 0x90)
	{
		UINT8 digit = state->m_out_offs << 1;
		output_set_digit_value(digit, patterns[state->m_p_ram[state->m_out_offs]>>4]);
		output_set_digit_value(++digit, patterns[state->m_p_ram[state->m_out_offs]&15]);
	}
	//else
	//if (state->m_out_offs == 0x90)
	//	timer.machine().device("maincpu")->execute().set_input_line(0, HOLD_LINE);
	//else
	//if (state->m_out_offs == 0x9f)
	//	timer.machine().device("maincpu")->execute().set_input_line(0, CLEAR_LINE);
}

static MACHINE_CONFIG_START( rowamet, rowamet_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 1888888)
	MCFG_CPU_PROGRAM_MAP(rowamet_map)
	MCFG_CPU_ADD("cpu2", Z80, 1888888)
	MCFG_CPU_PROGRAM_MAP(rowamet_sub_map)
	MCFG_CPU_IO_MAP(rowamet_sub_io)
	MCFG_TIMER_ADD_PERIODIC("rowamet_timer", rowamet_timer, attotime::from_hz(200))

	/* Video */
	//MCFG_DEFAULT_LAYOUT(layout_rowamet)

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
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("hvymtl_c.bin", 0x0000, 0x1000, CRC(8f36d3da) SHA1(beec79c5d794ede96d95105bad7466b67762606d))
	ROM_LOAD("hvymtl_b.bin", 0x1000, 0x1000, CRC(357f1252) SHA1(ddc55ded0dc1c8632c31d809bfadfb45ae248cfd))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("hvymtl_s.bin", 0x0000, 0x1000, CRC(c525e6cb) SHA1(144e06fbbdd1f3e45ccca8bace6b04f876b1312c))
	ROM_FILL(0, 1, 0) // remove erronous FF
ROM_END

/*-------------------------------------------------------------------
/ Vulcan IV (1982)
/-------------------------------------------------------------------*/


GAME(198?,  heavymtl,  0,  rowamet,  rowamet, rowamet_state,  rowamet,  ROT0,  "Rowamet",    "Heavy Metal",      GAME_IS_SKELETON_MECHANICAL)

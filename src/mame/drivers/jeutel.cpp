// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

  PINBALL
  Jeutel

  There are at least 7 machines from this manufacturer. Unable to find anything
  technical at all... so used PinMAME as the reference. Really need a proper
  schematic though.

ToDo:
- Everything!
- Each machine has 4 players, 7-digit, 12-segment florescent display

********************************************************************************/

#include "machine/genpin.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/tms5110.h"
#include "jeutel.lh"

class jeutel_state : public genpin_class
{
public:
	jeutel_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cpu2(*this, "cpu2")
		, m_tms(*this, "tms")
	{ }

	DECLARE_DRIVER_INIT(jeutel);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(ppi0a_w);
	DECLARE_WRITE8_MEMBER(ppi0b_w);
	DECLARE_WRITE8_MEMBER(sndcmd_w);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_a);
private:
	bool m_timer_a;
	UINT8 m_sndcmd;
	UINT8 m_digit;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_device<tms5110_device> m_tms;
};


static ADDRESS_MAP_START( jeutel_map, AS_PROGRAM, 8, jeutel_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("shared")
	AM_RANGE(0xc400, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jeutel_cpu2, AS_PROGRAM, 8, jeutel_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0x2000)
	AM_RANGE(0x2000, 0x2003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x4000, 0x4000) AM_WRITENOP // writes 12 here many times
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_SHARE("shared")
ADDRESS_MAP_END

static ADDRESS_MAP_START( jeutel_cpu3, AS_PROGRAM, 8, jeutel_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("roms", 0x3000)
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_WRITE(sndcmd_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( jeutel_cpu3_io, AS_IO, 8, jeutel_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("aysnd", ay8910_device, address_w)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x04, 0x04) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( jeutel )
INPUT_PORTS_END

WRITE8_MEMBER( jeutel_state::sndcmd_w )
{
	m_sndcmd = data;
}

READ8_MEMBER( jeutel_state::portb_r )
{
	return m_sndcmd;
}

WRITE8_MEMBER( jeutel_state::porta_w )
{
	if ((data & 0xf0) == 0xf0)
	{
		m_tms->ctl_w(space, offset, TMS5110_CMD_RESET);
		m_tms->pdc_w(1);
		m_tms->pdc_w(0);
	}
	else
	if ((data & 0xf0) == 0xd0)
	{
		m_tms->ctl_w(space, offset, TMS5110_CMD_SPEAK);
		m_tms->pdc_w(1);
		m_tms->pdc_w(0);
	}
}

WRITE8_MEMBER( jeutel_state::ppi0a_w )
{
	UINT16 segment;
	bool blank = !BIT(data, 7);

	if BIT(data, 6)
	{
		output().set_digit_value(40+m_digit, 0x3f); //patterns[data&15];
		return;
	}
	switch (data & 0x0f)
	{
		case 0x0a: // letter T
			segment = 0x301;
			break;
		case 0x0b: // letter E
			segment = 0x79;
			break;
		case 0x0c: // letter L
			segment = 0x38;
			break;
		case 0x0d: // letter U
			segment = 0x3e;
			break;
		case 0x0e: // letter J
			segment = 0x1e;
			break;
		default:
			segment = 0x3f; //patterns[data & 0x0f];
	}
	if BIT(data, 4)
	{
		output().set_digit_value(m_digit, (blank) ? 0 : segment);
	}
	else
	if BIT(data, 5)
	{
		output().set_digit_value(20+m_digit, (blank) ? 0 : segment);
	}
}

WRITE8_MEMBER( jeutel_state::ppi0b_w )
{
	m_digit = data & 0x0f;
	if (m_digit > 7)
		m_digit+=2;
}


void jeutel_state::machine_reset()
{
	m_timer_a = 0;
	m_sndcmd = 0;
	m_digit = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER( jeutel_state::timer_a )
{
	m_timer_a ^= 1;
	m_cpu2->set_input_line(0, (m_timer_a) ? ASSERT_LINE : CLEAR_LINE);
	if (m_cpu2->state_int(Z80_HALT))
		m_cpu2->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

DRIVER_INIT_MEMBER( jeutel_state, jeutel )
{
}

static MACHINE_CONFIG_START( jeutel, jeutel_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3300000)
	MCFG_CPU_PROGRAM_MAP(jeutel_map)
	MCFG_CPU_ADD("cpu2", Z80, 3300000)
	MCFG_CPU_PROGRAM_MAP(jeutel_cpu2)
	MCFG_CPU_ADD("cpu3", Z80, 3300000)
	MCFG_CPU_PROGRAM_MAP(jeutel_cpu3)
	MCFG_CPU_IO_MAP(jeutel_cpu3_io)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_jeutel)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 639450)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(jeutel_state,porta_w))
	MCFG_AY8910_PORT_B_READ_CB(READ8(jeutel_state,portb_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("tms", TMS5110A, 640000)
	//MCFG_TMS5110_M0_CB(DEVWRITELINE("tmsprom", tmsprom_device, m0_w))
	//MCFG_TMS5110_DATA_CB(DEVREADLINE("tmsprom", tmsprom_device, data_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* Devices */
	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	//MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(jeutel_state, ppi0a_w))
	//MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(jeutel_state, ppi0b_w))
	//MCFG_I8255_IN_PORTC_CB(IOPORT("EXTRA"))
	//MCFG_I8255_OUT_PORTC_CB(WRITE8(jeutel_state, ppi0c_w))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	//MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	//MCFG_I8255_OUT_PORTA_CB(WRITE8(jeutel_state, ppi1a_w))
	//MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	//MCFG_I8255_OUT_PORTB_CB(WRITE8(jeutel_state, ppi1b_w))
	//MCFG_I8255_IN_PORTC_CB(IOPORT("EXTRA"))
	//MCFG_I8255_OUT_PORTC_CB(WRITE8(jeutel_state, ppi1c_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)
	//MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	//MCFG_I8255_OUT_PORTA_CB(WRITE8(jeutel_state, ppi2a_w))
	//MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	//MCFG_I8255_OUT_PORTB_CB(WRITE8(jeutel_state, ppi2b_w))
	//MCFG_I8255_IN_PORTC_CB(IOPORT("EXTRA"))
	//MCFG_I8255_OUT_PORTC_CB(WRITE8(jeutel_state, ppi2c_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_a", jeutel_state, timer_a, attotime::from_hz(120))
MACHINE_CONFIG_END

/*--------------------------------
/ Le King
/-------------------------------*/
ROM_START(leking)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("game-m.bin",  0x0000, 0x2000, CRC(4b66517a) SHA1(1939ea78932d469a16441507bb90b032c5f77b1e))
	ROM_LOAD("game-v.bin",  0x2000, 0x1000, CRC(cbbc8b55) SHA1(4fe150fa3b565e5618896c0af9d51713b381ed88))
	ROM_LOAD("sound-v.bin", 0x3000, 0x1000, CRC(36130e7b) SHA1(d9b66d43b55272579b3972005355b8a18ce6b4a9))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("sound-p.bin", 0x0000, 0x2000, BAD_DUMP CRC(97eedd6c) SHA1(3bb8e5d32417c49ef97cbe407f2c5eeb214bf72d))
ROM_END

/*--------------------------------
/ Olympic Games
/-------------------------------*/
ROM_START(olympic)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("game-jo1.bin", 0x0000, 0x2000, CRC(c9f040cf) SHA1(c689f3a82d904d3f9fc8688d4c06082c51645b2f))
	ROM_LOAD("game-v.bin",   0x2000, 0x1000, CRC(cd284a20) SHA1(94568e1247994c802266f9fbe4a6f6ed2b55a978))
	ROM_LOAD("sound-j0.bin", 0x3000, 0x1000, CRC(5c70ce72) SHA1(b0b6cc7b6ec3ed9944d738b61a0d144b77b07000))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("sound-p.bin",  0x0000, 0x2000, CRC(97eedd6c) SHA1(3bb8e5d32417c49ef97cbe407f2c5eeb214bf72d))
ROM_END


GAME(1983,  leking,   0,  jeutel,  jeutel, jeutel_state,  jeutel,  ROT0, "Jeutel", "Le King", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  olympic,  0,  jeutel,  jeutel, jeutel_state,  jeutel,  ROT0, "Jeutel", "Olympic Games", MACHINE_IS_SKELETON_MECHANICAL)

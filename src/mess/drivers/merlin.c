#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

/* Layout */
#include "merlin.lh"


class merlin_state : public driver_device
{
public:
	merlin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_speaker(*this, "speaker")
	{ }

	required_device<device_t> m_speaker;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

protected:
	UINT16	m_o;
	UINT16	m_r;
};



#define LOG 1


static INPUT_PORTS_START( merlin )
	PORT_START("O0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)     PORT_NAME("R0")  // R0
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)     PORT_NAME("R1")  // R1
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_NAME("R3")  // R3
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_NAME("R2")  // R2

	PORT_START("O1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_NAME("R4")  // R4
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)     PORT_NAME("R5")  // R5
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)     PORT_NAME("R7")  // R7
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)     PORT_NAME("R6")  // R6

	PORT_START("O2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)     PORT_NAME("R8")  // R8
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)     PORT_NAME("R9")  // R9
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_NAME("SG")  // SG - same game
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("R10")  // R10

	PORT_START("O3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_NAME("CT")  // Comp Turn
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_NAME("HM")  // Hit me
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_NAME("NG")  // NG - new game

INPUT_PORTS_END


/*
The keypad is a 4*4 matrix, connected like so:

       +----+  +----+  +----+  +----+
K1 o---| R0 |--| R1 |--| R2 |--| R3 |
       +----+  +----+  +----+  +----+
          |       |       |       |
       +----+  +----+  +----+  +----+
K2 o---| R4 |--| R5 |--| R6 |--| R7 |
       +----+  +----+  +----+  +----+
          |       |       |       |
       +----+  +----+  +----+  +----+
K4 o---| R8 |--| R9 |--|R10 |--| SG |
       +----+  +----+  +----+  +----+
          |       |       |       |
          |    +----+  +----+  +----+
K8 o------+----| CT |--| NG |--| HM |
          |    +----+  +----+  +----+
          |       |       |       |
          o       o       o       o
         O0      O1      O3      O2

SG = same game, CT = comp turn, NG = new game, HM = hit me
*/

READ8_MEMBER(merlin_state::read_k)
{
	UINT8 data = 0;

	if (LOG)
		logerror( "read_k\n" );

	if ( m_o & 0x01 )
	{
		data |= ioport("O0")->read();
	}

	if ( m_o & 0x02 )
	{
		data |= ioport("O1")->read();
	}

	if ( m_o & 0x04 )
	{
		data |= ioport("O2")->read();
	}

	if ( m_o & 0x08 )
	{
		data |= ioport("O3")->read();
	}

	return data;
}


/*
The speaker is connected to O4 through O6.  The 3 outputs are paralleled for
increased current driving capability.  They are passed thru a 220 ohm resistor
and then to the speaker, which has the other side grounded.  The software then
toggles these lines to make sounds and noises. (There is no audio generator
other than toggling it with a software delay between to make tones).
*/

WRITE16_MEMBER(merlin_state::write_o)
{
	if (LOG)
		logerror( "write_o: write %02x\n", data );

	m_o = data;

	speaker_level_w( m_speaker, m_o & 0x70 );
}


/*

  LEDs:

      R0
 R1   R2   R3
 R4   R5   R6
 R7   R8   R9
      R10

When that particular R output is high, that LED is on.
*/

WRITE16_MEMBER(merlin_state::write_r)
{
	if (LOG)
		logerror( "write_r: write %04x\n", data );

	m_r = data;

	output_set_value( "led_0",  BIT( m_r,  0 ) );
	output_set_value( "led_1",  BIT( m_r,  1 ) );
	output_set_value( "led_2",  BIT( m_r,  2 ) );
	output_set_value( "led_3",  BIT( m_r,  3 ) );
	output_set_value( "led_4",  BIT( m_r,  4 ) );
	output_set_value( "led_5",  BIT( m_r,  5 ) );
	output_set_value( "led_6",  BIT( m_r,  6 ) );
	output_set_value( "led_7",  BIT( m_r,  7 ) );
	output_set_value( "led_8",  BIT( m_r,  8 ) );
	output_set_value( "led_9",  BIT( m_r,  9 ) );
	output_set_value( "led_10", BIT( m_r, 10 ) );
}


static const tms0980_config merlin_tms0980_config =
{
	{
		/* O output PLA configuration currently unknown */
		0x01, 0x10, 0x30, 0x70, 0x02, 0x12, 0x32, 0x72,
		0x04, 0x14, 0x34, 0x74, 0x08, 0x18, 0x38, 0x78,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	},
	DEVCB_DRIVER_MEMBER(merlin_state, read_k),
	DEVCB_DRIVER_MEMBER16(merlin_state, write_o),
	DEVCB_DRIVER_MEMBER16(merlin_state, write_r)
};


static MACHINE_CONFIG_START( merlin, merlin_state )
	MCFG_CPU_ADD( "maincpu", TMS1100, 500000 )	/* Clock is wrong */
	MCFG_CPU_CONFIG( merlin_tms0980_config )

	MCFG_DEFAULT_LAYOUT(layout_merlin)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


ROM_START( merlin )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "mp3404", 0x0000, 0x800, CRC(9362d9f9) SHA1(266d2a4a98cc33944a4fc7ed073ba9321bba8e05) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY            FULLNAME      FLAGS */
CONS( 1978, merlin,     0,      0,      merlin,     merlin, driver_device,  0,      "Parker Brothers", "Merlin", GAME_IS_SKELETON )



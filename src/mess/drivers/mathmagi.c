// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  APF Mathemagician
  * TMS1100 MP1030 - MCU
  * 2 x DS8870N - Hex LED Digit Driver
  * 2 x DS8861N - MOS-to-LED 5-Segment Driver
  
  This is a tabletop educational calculator. It came with plastic overlays
  for playing different kind of games. Refer to the manual on how to use it.
  In short, to start from scratch, press [SEL]. By default the device is in
  calculator teaching mode. If [SEL] is followed with 1-6 and then [NXT],
  one of the games is started.
  
  1) Number Machine
  2) Countin' On
  3) Walk The Plank
  4) Gooey Gumdrop
  5) Football
  6) Lunar Lander


  TODO:
  - some of the led symbols are probably wrong, output PLA is unknown
  - microinstructions PLA is not verified
  
***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "mathmagi.lh"

// master clock is a single stage RC oscillator: R=68K, C=82pf,
// according to the TMS 1000 series data manual this is around 200kHz
#define MASTER_CLOCK (200000)


class mathmagi_state : public driver_device
{
public:
	mathmagi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN")
	{ }

	required_device<tms1xxx_cpu_device> m_maincpu;
	required_ioport_array<6> m_button_matrix;

	UINT16 m_o;
	UINT16 m_r;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(mathmagi_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 6; i++)
	{
		const int ki[6] = { 3, 5, 6, 7, 9, 10 };
		if (m_r >> ki[i] & 1)
			k |= m_button_matrix[i]->read();
	}

	return k;
}

WRITE16_MEMBER(mathmagi_state::write_o)
{
	// O1-O7: led segments A-G
	// O0: N/C
	m_o = data;
}

WRITE16_MEMBER(mathmagi_state::write_r)
{
	// R3,R5-R7,R9,R10: input mux
	// and outputs:
	for (int i = 0; i < 11; i++)
	{
		if (data >> i & 1)
		{
			// R8: custom math symbols digit
			// R9: custom equals digit
			// R10: lamps
			if (i >= 8)
				for (int j = 0; j < 8; j++)
					output_set_lamp_value(i*10 + j, m_o >> j & 1);
			
			// R0-R7: 7seg leds
			else
				output_set_digit_value(i, m_o >> 1 & 0x7f);
		}
	}

	m_r = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

/* physical button layout and labels is like this:

    ON     ONE       [SEL] [NXT] [?]   [/]
     |      |        [7]   [8]   [9]   [x]
    OFF    TWO       [4]   [5]   [6]   [-]
         PLAYERS     [1]   [2]   [3]   [+]
                     [0]   [_]   [r]   [=]
*/

static INPUT_PORTS_START( mathmagi )
	PORT_START("IN.0") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.1") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("_") // blank
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("r")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.2") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME(UTF8_MULTIPLY)

	PORT_START("IN.3") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_NAME("SEL")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_NAME("NXT")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_NAME("?") // check
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("=")

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME(UTF8_DIVIDE)

	PORT_START("IN.5") // R10
	PORT_CONFNAME( 0x01, 0x00, "Players")
	PORT_CONFSETTING(    0x00, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void mathmagi_state::machine_start()
{
	m_o = 0;
	m_r = 0;

	save_item(NAME(m_o));
	save_item(NAME(m_r));
}

// LED segments A-G
enum
{
	lA = 0x02,
	lB = 0x04,
	lC = 0x08,
	lD = 0x10,
	lE = 0x20,
	lF = 0x40,
	lG = 0x80
};

static const UINT16 mathmagi_output_pla[0x20] =
{
	lA+lB+lC+lD+lE+lF,      // 0
	lB+lC,                  // 1
	lA+lB+lG+lE+lD,         // 2
	lA+lB+lG+lC+lD,         // 3
	lF+lB+lG+lC,            // 4
	lA+lF+lG+lC+lD,         // 5
	lA+lF+lG+lC+lD+lE,      // 6
	lA+lB+lC,               // 7
	lA+lB+lC+lD+lE+lF+lG,   // 8
	lA+lB+lG+lF+lC+lD,      // 9
	lA+lB+lG+lE,            // question mark
	lE+lG,                  // r
	lD,                     // underscore?
	lA+lF+lG+lE+lD,         // E
	lG,                     // -
	0,                      // empty
	0,                      // empty
	lG,                     // lamp 4 or MATH -
	lD,                     // lamp 3
	lF+lE+lD+lC+lG,         // b
	lB,                     // lamp 2
	lB+lG,                  // MATH +
	lB+lC,                  // MATH mul
	lF+lG+lB+lC+lD,         // y
	lA,                     // lamp 1
	lA+lG,                  // MATH div
	lA+lD,                  // EQUALS
	0,                      // ?
	0,                      // ?
	lE+lD+lC+lG,            // o
	0,                      // ?
	lA+lF+lE+lD+lC          // G
};


static MACHINE_CONFIG_START( mathmagi, mathmagi_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(mathmagi_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(mathmagi_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(mathmagi_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(mathmagi_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_mathmagi)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mathmagi )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "mp1030", 0x0000, 0x800, CRC(a81d7ccb) SHA1(4756ce42f1ea28ce5fe6498312f8306f10370969) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_mathmagi_opla.pla", 0, 365, NO_DUMP )
ROM_END


COMP( 1980, mathmagi, 0, 0, mathmagi, mathmagi, driver_device, 0, "APF Electronics Inc.", "Mathemagician", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )

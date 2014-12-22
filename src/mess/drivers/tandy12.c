// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Tandy Radio Shack Computerized Arcade (1981, 1982, 1995)
  * TMS1100 CD7282SL
  
  This handheld contains 12 minigames. It looks and plays like "Fabulous Fred"
  by the Japanese company Mego Corp. in 1980, which in turn is a mix of Merlin
  and Simon. Unlike Merlin and Simon, spin-offs like these were not successful.
  There were releases with and without the prefix "Tandy-12", I don't know
  which name was more common. Also not worth noting is that it needed five
  batteries; 4 C-cells and a 9-volt.
  
  Some of the games require accessories included with the toy (eg. the Baseball
  game is played with a board representing the playing field). To start a game,
  hold the [SELECT] button, then press [START] when the game button lights up.
  As always, refer to the official manual for more information.
  
  See below at the input defs for a list of the games.

  
  TODO:
  - output PLA is not verified
  - microinstructions PLA is not verified

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "tandy12.lh" // clickable

// master clock is a single stage RC oscillator: R=39K, C=47pf,
// according to the TMS 1000 series data manual this is around 400kHz
#define MASTER_CLOCK (400000)


class tandy12_state : public driver_device
{
public:
	tandy12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<tms1xxx_cpu_device> m_maincpu;
	required_ioport_array<5> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_r;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(tandy12_state::read_k)
{
	UINT8 k = 0;
	
	// read selected button rows
	for (int i = 0; i < 5; i++)
		if (m_r >> (i+5) & 1)
			k |= m_button_matrix[i]->read();

	return k;
}

WRITE16_MEMBER(tandy12_state::write_o)
{
	// O0-O7: button lamps 1-8
	for (int i = 0; i < 8; i++)
		output_set_lamp_value(i+1, data >> i & 1);
}

WRITE16_MEMBER(tandy12_state::write_r)
{
	// R0-R3: button lamps 9-12
	for (int i = 0; i < 4; i++)
		output_set_lamp_value(i+1 + 8, data >> i & 1);

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R5-R9: input mux
	m_r = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

/* physical button layout and labels is like this:

        REPEAT-2              SPACE-2
          [O]     OFF--ON       [O]

    [purple]1     [blue]5       [l-green]9
    ORGAN         TAG-IT        TREASURE HUNT
    
    [l-orange]2   [turquoise]6  [red]10
    SONG WRITER   ROULETTE      COMPETE
    
    [pink]3       [yellow]7     [violet]11
    REPEAT        BASEBALL      FIRE AWAY

    [green]4      [orange]8     [brown]12
    TORPEDO       REPEAT PLUS   HIDE 'N SEEK

          [O]        [O]        [O]
         START      SELECT    PLAY-2/HIT-7
*/

static INPUT_PORTS_START( tandy12 )
	PORT_START("IN.0") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("12")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("11")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("10")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")

	PORT_START("IN.1") // R6
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_NAME("Space-2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_NAME("Play-2/Hit-7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_NAME("Select")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Start")

	PORT_START("IN.2") // R7
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_NAME("Repeat-2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R8
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")

	PORT_START("IN.4") // R9
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tandy12_state::machine_start()
{
	m_r = 0;
	save_item(NAME(m_r));
}


static const UINT16 tandy12_output_pla[0x20] =
{
	// these are certain
	0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
	0x80, 0x00, 0x00, 0x00, 0x00,
	
	// rest is unused?
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static MACHINE_CONFIG_START( tandy12, tandy12_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(tandy12_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(tandy12_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tandy12_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tandy12_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_tandy12)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tandy12 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "cd7282sl", 0x0000, 0x800, CRC(a10013dd) SHA1(42ebd3de3449f371b99937f9df39c240d15ac686) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tandy12_opla.pla", 0, 365, NO_DUMP )
ROM_END


CONS( 1981, tandy12, 0, 0, tandy12, tandy12, driver_device, 0, "Tandy Radio Shack", "Tandy-12: Computerized Arcade", GAME_SUPPORTS_SAVE )

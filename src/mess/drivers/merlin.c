// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/***************************************************************************

  Parker Bros Merlin handheld computer game
  * TMS1100NLL MP3404A-N2 (has internal ROM)
  
  To start a game, press NEW GAME, followed by a number:
  1: Tic-Tac-Toe
  2: Music Machine
  3: Echo
  4: Blackjack 13
  5: Magic Square
  6: Mindbender
  
  Refer to the official manual for more information on the games.
  
  
  Other handhelds assumed to be on similar hardware:
  - Dr. Smith - by Tomy, released in Japan (basically a white version of Merlin,
    let's assume for now that the ROM contents is identical)
  - Master Merlin
  
  Another sequel, called Split Second, looks like different hardware.


  TODO:
  - accurate speaker levels (tone pitch sounds good though)
  - is the rom dump good?

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "merlin.lh" // clickable

// master clock is a single stage RC oscillator: R=33K, C=100pf,
// according to the TMS 1000 series data manual this is around 350kHz
#define MASTER_CLOCK (350000)


class merlin_state : public driver_device
{
public:
	merlin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();

protected:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_o;
};


/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(merlin_state::read_k)
{
	UINT8 k = 0;
	
	// read selected button rows
	for (int i = 0; i < 4; i++)
		if (m_o >> i & 1)
			k |= m_button_matrix[i]->read();

	return k;
}

WRITE16_MEMBER(merlin_state::write_o)
{
	/* The speaker is connected to O4 through O6.  The 3 outputs are paralleled for
	increased current driving capability.  They are passed thru a 220 ohm resistor
	and then to the speaker, which has the other side grounded.  The software then
	toggles these lines to make sounds and noises. (There is no audio generator
	other than toggling it with a software delay between to make tones). */
	static const int count[8] = { 0, 1, 1, 2, 1, 2, 2, 3 };
	m_speaker->level_w(count[data >> 4 & 7]);

	// O0-O3: input mux
	// O7: N/C
	m_o = data;
}

WRITE16_MEMBER(merlin_state::write_r)
{
	/* LEDs:

	     R0
	R1   R2   R3
	R4   R5   R6
	R7   R8   R9
	     R10
	*/
	for (int i = 0; i < 11; i++)
		output_set_lamp_value(i, data >> i & 1);
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( merlin )
	PORT_START("IN.0") // O0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("Button 0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Button 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Button 3")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Button 2")

	PORT_START("IN.1") // O1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Button 5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Button 7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Button 6")

	PORT_START("IN.2") // O2
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Button 8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Button 9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_CODE(KEYCODE_S) PORT_NAME("Same Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Button 10")

	PORT_START("IN.3") // O3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON13) PORT_CODE(KEYCODE_C) PORT_NAME("Comp Turn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON15) PORT_CODE(KEYCODE_H) PORT_NAME("Hit Me")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON14) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("New Game")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void merlin_state::machine_start()
{
	m_o = 0;
	save_item(NAME(m_o));
}


static const UINT16 merlin_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x01, 0x10, 0x30, 0x70, 0x02, 0x12, 0x32, 0x72,
	0x04, 0x14, 0x34, 0x74, 0x08, 0x18, 0x38, 0x78,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const INT16 speaker_levels[] = { 0, 32767, 0, 32767 }; // unknown too, due to output_pla being unknown


static MACHINE_CONFIG_START( merlin, merlin_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(merlin_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(merlin_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(merlin_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(merlin_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_merlin)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(4, speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( merlin )
	ROM_REGION( 0x800, "maincpu", 0 )
	// This rom needs verification, that's why it is marked as a bad dump
	// We had to change one byte in the original dump at offset 0x096 from
	// 0x5E to 0x1E to make 'Music Machine' working.
	// The hashes below are from the manually changed dump
	ROM_LOAD( "mp3404", 0x0000, 0x800, BAD_DUMP CRC(7515a75d) SHA1(76ca3605d3fde1df62f79b9bb1f534c2a2ae0229) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_merlin_opla.pla", 0, 365, NO_DUMP )
ROM_END


CONS( 1978, merlin, 0, 0, merlin, merlin, driver_device, 0, "Parker Brothers", "Merlin", GAME_SUPPORTS_SAVE )

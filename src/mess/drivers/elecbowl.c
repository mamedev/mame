// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  Marx Series 300 Electronic Bowling Game
  * TMS1100NLL MP3403 DBS 7836 SINGAPORE

  10 lamps for bowling pins + 3 more bulbs, and 7segs for frame number and
  scores. Board size is 10-12" by 6-8".

  some clues:
  - it's from 1978
  - Merlin is MP3404, Amaze-A-Tron is MP3405, this one is MP3403
  - it plays some short jingles (you need to be lucky with button mashing)

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

// internal artwork
#include "elecbowl.lh"


class elecbowl_state : public driver_device
{
public:
	elecbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_inp_matrix;
	required_device<speaker_sound_device> m_speaker;
	
	UINT16 m_r;
	UINT16 m_o;
	UINT16 m_inp_mux;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);

	virtual void machine_start();
};



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(elecbowl_state::read_k)
{
	UINT8 k = 0;

	// read selected input rows
	for (int i = 0; i < 4; i++)
		if (m_inp_mux >> i & 1)
			k |= m_inp_matrix[i]->read();

	return k;
}

WRITE16_MEMBER(elecbowl_state::write_r)
{
	// R4-R7: input mux
	m_inp_mux = data >> 4 & 0xf;

	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// R10: maybe a switch or other button row?
	// others: ?
}

WRITE16_MEMBER(elecbowl_state::write_o)
{
	// ?
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( elecbowl )
	PORT_START("IN.0") // R4
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)

	PORT_START("IN.2") // R6
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) // reset/newgame?

	PORT_START("IN.3") // R7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void elecbowl_state::machine_start()
{
	// zerofill
	m_o = 0;
	m_r = 0;
	m_inp_mux = 0;

	// register for savestates
	save_item(NAME(m_o));
	save_item(NAME(m_r));
	save_item(NAME(m_inp_mux));
}


static const UINT16 elecbowl_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static MACHINE_CONFIG_START( elecbowl, elecbowl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 300000) // approximation - unknown freq
	MCFG_TMS1XXX_OUTPUT_PLA(elecbowl_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(elecbowl_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(elecbowl_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(elecbowl_state, write_o))

	MCFG_DEFAULT_LAYOUT(layout_elecbowl)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( elecbowl )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "mp3403", 0x0000, 0x0800, CRC(9eabaa7d) SHA1(b1f54587ed7f2bbf3a5d49075c807296384c2b06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_elecbowl_opla.pla", 0, 365, NO_DUMP )
ROM_END


CONS( 1978, elecbowl, 0, 0, elecbowl, elecbowl, driver_device, 0, "Marx", "Electronic Bowling", GAME_SUPPORTS_SAVE | GAME_MECHANICAL | GAME_NOT_WORKING )

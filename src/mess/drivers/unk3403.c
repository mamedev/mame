// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  TMS1100NLL MP3403 DBS 7836 SINGAPORE some game board with 7-segs.

  What old electronic game is this?

  some clues:
  - it's from 1978
  - Merlin is MP3404, Amaze-A-Tron is MP3405, this one is MP3403
  - it plays some short jingles (you need to be lucky with button mashing),
    jingles feel like maybe a horse racing game

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

// master clock is unknown, the value below is an approximation
#define MASTER_CLOCK (350000)


class unk3403_state : public driver_device
{
public:
	unk3403_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_r;
	UINT16 m_o;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	void leds_update();

	virtual void machine_start();
};



/***************************************************************************

  I/O

***************************************************************************/

void unk3403_state::leds_update()
{
	// show debug clues
	static UINT8 leds[0x10] = { 0 };
	char msg[0x100] = { 0 };
	char dig[0x100] = { 0 };
	sprintf(msg, "R,  *R,  O[R]");

	for (int i = 0; i < 11; i++)
	{
		if (m_r >> i & 1)
		{
			leds[i]=m_o;
		}
		sprintf(dig, "\n  %X   %c   %02X",i, (m_r >> i & 1) ? 'X' : '_', leds[i]);
		strcat(msg, dig);
	}

	popmessage("%s", msg);
}

READ8_MEMBER(unk3403_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 4; i++)
	{
		if (m_r >> (i + 4) & 1)
			k |= m_button_matrix[i]->read();
	}

	return k;
}

WRITE16_MEMBER(unk3403_state::write_r)
{
	// R4-R7: input mux
	// R10: maybe a switch or other button row?
	// R9: speaker out
	m_speaker->level_w(data >> 9 & 1);

	// others: ?
	m_r = data;
	leds_update();
}

WRITE16_MEMBER(unk3403_state::write_o)
{
	// ?
	m_o = data;
	leds_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( unk3403 )
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

void unk3403_state::machine_start()
{
	m_r = 0;
	m_o = 0;

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static const UINT16 unk3403_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};


static MACHINE_CONFIG_START( unk3403, unk3403_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(unk3403_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(unk3403_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(unk3403_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(unk3403_state, write_r))

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( unk3403 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "tms1100nll_mp3403", 0x0000, 0x0800, CRC(9eabaa7d) SHA1(b1f54587ed7f2bbf3a5d49075c807296384c2b06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_xxx_opla.pla", 0, 365, NO_DUMP )
ROM_END


CONS( 1978, unk3403, 0, 0, unk3403, unk3403, driver_device, 0, "<unknown>", "unknown TMS1100 electronic game", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

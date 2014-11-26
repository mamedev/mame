// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Milton Bradley Simon

  Revision A hardware:
  * TMS1000 (has internal ROM), DS75494 lamp driver

  Newer revisions have a smaller 16-pin MB4850 chip instead of the TMS1000.
  This one has been decapped too, but we couldn't find an internal ROM.
  It is possibly a cost-reduced custom ASIC specifically for Simon.

  Other games assumed to be on similar hardware:
  - Pocket Simon, but there's a chance it only exists with MB4850 chip
  - Super Simon (TMS1100)

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "simon.lh"

// master clock is a single stage RC oscillator: R=33K, C=100pf,
// according to the TMS 1000 series data manual this is around 350kHz
#define SIMON_RC_CLOCK (350000)


class simon_state : public driver_device
{
public:
	simon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_button_matrix;
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

READ8_MEMBER(simon_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 4; i++)
	{
		static int r[4] = { 0, 1, 2, 9 };
		if (m_r & (1 << r[i]))
			k |= m_button_matrix[i]->read();
	}

	return k;
}

WRITE16_MEMBER(simon_state::write_r)
{
	// R4-R8 go through an 75494 IC first:
	// R4 -> 75494 IN6 -> green lamp
	// R5 -> 75494 IN3 -> red lamp
	// R6 -> 75494 IN5 -> yellow lamp
	// R7 -> 75494 IN2 -> blue lamp
	for (int i = 0; i < 4; i++)
		output_set_lamp_value(i, data >> (4 + i) & 1);

	// R8 -> 75494 IN0 -> speaker
	m_speaker->level_w(data >> 8 & 1);

	// R0,R1,R2,R9: input mux
	// R3: GND
	// other bits: N/C
	m_r = data;
}

WRITE16_MEMBER(simon_state::write_o)
{
	// N/C
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( simon )
	PORT_START("IN.0")
	PORT_CONFNAME( 0x07, 0x02, "Game Select")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Green Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Red Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Blue Button")

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Longest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3")
	PORT_CONFNAME( 0x0f, 0x01, "Skill Level")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void simon_state::machine_start()
{
	m_r = 0;
	save_item(NAME(m_r));
}


static const UINT16 simon_output_pla[0x20] =
{
	/* The output PLA just maps 1 2 4 8 and SL to O0-O4 */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};


static MACHINE_CONFIG_START( simon, simon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, SIMON_RC_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(simon_output_pla)
	MCFG_TMS1XXX_READ_K(READ8(simon_state, read_k))
	MCFG_TMS1XXX_WRITE_O(WRITE16(simon_state, write_o))
	MCFG_TMS1XXX_WRITE_R(WRITE16(simon_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_simon)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( simon )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1000.u1", 0x0000, 0x0400, CRC(9961719d) SHA1(35dddb018a8a2b31f377ab49c1f0cb76951b81c0) )
ROM_END


CONS( 1978, simon, 0, 0, simon, simon, driver_device, 0, "Milton Bradley", "Simon (Rev. A)", GAME_SUPPORTS_SAVE )

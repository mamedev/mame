// license:BSD-3-Clause
// copyright-holders:hap, Kevin Horton
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.cpp) **

  Marx Series 300 Electronic Bowling Game
  Main board:
  * TMS1100NLL MP3403 DBS 7836 SINGAPORE
  * 4*SN75492 quad segment driver, 2*SN74259 8-line demultiplexer,
    2*CD4043 quad r/s input latch
  * 5 7seg LEDs, 15 lamps(10 lamps projected to bowling pins reflection),
    1bit-sound with crude volume control
  * edge connector to sensors(switches trigger when ball rolls over)
    and other inputs

  lamp translation table: SN74259.u5(mux 1) goes to MAME output lamp5x,
  SN74259.u6(mux 2) goes to MAME output lamp6x. u1-u3 are SN75492 ICs,
  where other: u1 A2 is N/C, u3 A1 is from O2 and goes to digits seg C.

    u5 Q0 -> u1 A4 -> L2 (pin #2)       u6 Q0 -> u3 A4 -> L1 (pin #1)
    u5 Q1 -> u1 A5 -> L4 (pin #4)       u6 Q1 -> u3 A5 -> L5 (pin #5)
    u5 Q2 -> u1 A6 -> L7 (pin #7)       u6 Q2 -> u2 A3 -> L11 (player 1)
    u5 Q3 -> u1 A1 -> L8 (pin #8)       u6 Q3 -> u2 A2 -> L12 (player 2)
    u5 Q4 -> u3 A2 -> L3 (pin #3)       u6 Q4 -> u2 A1 -> L15 (?)
    u5 Q5 -> u2 A6 -> L6 (pin #6)       u6 Q5 -> u3 A6 -> L14 (?)
    u5 Q6 -> u2 A5 -> L10 (pin #10)     u6 Q6 -> u1 A3 -> L13 (spare)
    u5 Q7 -> u2 A4 -> L9 (pin #9)       u6 Q7 -> u3 A3 -> digit 4 B+C

***************************************************************************/

#include "includes/hh_tms1k.h"
#include "elecbowl.lh"


class elecbowl_state : public hh_tms1k_state
{
public:
	elecbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);
};


/***************************************************************************

  Display

***************************************************************************/

void elecbowl_state::prepare_display()
{
	// standard 7segs
	for (int y = 0; y < 4; y++)
	{
		m_display_segmask[y] = 0x7f;
		m_display_state[y] = (m_r >> (y + 4) & 1) ? m_o : 0;
	}

	// lamp muxes
	UINT8 mask = 1 << (m_o & 7);
	UINT8 d = (m_r & 2) ? mask : 0;
	if (~m_r & 1)
		m_display_state[5] = (m_display_state[5] & ~mask) | d;
	if (~m_r & 4)
		m_display_state[6] = (m_display_state[6] & ~mask) | d;

	// digit 4 is from mux2 Q7
	m_display_segmask[4] = 6;
	m_display_state[4] = (m_display_state[6] & 0x80) ? 6 : 0;

	set_display_size(8, 7);
	display_update();
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE16_MEMBER(elecbowl_state::write_r)
{
	// R5-R7,R10: input mux
	m_inp_mux = (data >> 5 & 7) | (data >> 7 & 8);

	// R9: speaker out
	// R3,R8: speaker volume..
	m_speaker->level_w(data >> 9 & 1);

	// R4-R7: select digit
	// R0,R2: lamp mux1,2 _enable
	// R1: lamp muxes state
	m_r = data;
	prepare_display();
}

WRITE16_MEMBER(elecbowl_state::write_o)
{
	//if (data & 0x80) printf("%X ",data&0x7f);

	// O0-O2: lamp muxes select
	// O0-O6: digit segments A-G
	// O7: N/C
	m_o = data & 0x7f;
	prepare_display();
}

READ8_MEMBER(elecbowl_state::read_k)
{
	// K: multiplexed inputs
	return read_inputs(4);
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( elecbowl )
	PORT_START("IN.0") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)

	PORT_START("IN.1") // R6
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) // reset/test?
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) // reset/test?

	PORT_START("IN.2") // R7
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.3") // R10
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) // 2 players sw?
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

// output PLA is not dumped
static const UINT16 elecbowl_output_pla[0x20] =
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

	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0,1,2,3,4,5,6,7,        // lamp muxes select
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f
};

static MACHINE_CONFIG_START( elecbowl, elecbowl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // RC osc. R=33K, C=100pf -> ~350kHz
	MCFG_TMS1XXX_OUTPUT_PLA(elecbowl_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(elecbowl_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(elecbowl_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(elecbowl_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_elecbowl)

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
	ROM_LOAD( "mp3403.u9", 0x0000, 0x0800, CRC(9eabaa7d) SHA1(b1f54587ed7f2bbf3a5d49075c807296384c2b06) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_elecbowl_output.pla", 0, 365, NO_DUMP )
ROM_END


CONS( 1978, elecbowl, 0, 0, elecbowl, elecbowl, driver_device, 0, "Marx", "Electronic Bowling (Marx)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_NOT_WORKING )

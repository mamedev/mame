// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  This driver is a collection of simple dedicated handheld and tabletop
  toys based around the TMS1000 MCU series. Anything more complex or clearly
  part of a series is (or will be) in its own driver.


***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "ebball.lh"
#include "tc4.lh"


class hh_tms1k_state : public driver_device
{
public:
	hh_tms1k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_maxy(0),
		m_display_maxx(0),
		m_display_wait(50)
	{ }

	required_device<cpu_device> m_maincpu;
	optional_ioport_array<7> m_inp_matrix; // max 7
	optional_device<speaker_sound_device> m_speaker;
	
	UINT16 m_r;
	UINT16 m_o;
	UINT16 m_inp_mux;

	int m_display_maxy;
	int m_display_maxx;
	int m_display_wait;
	
	UINT32 m_display_state[0x20];
	UINT32 m_display_cache[0x20];
	UINT8 m_display_decay[0x20][0x20];
	UINT16 m_7seg_mask[0x20];
	
	UINT8 read_inputs(int columns);

	// game-specific handlers
	void tc4_display();
	DECLARE_READ8_MEMBER(tc4_read_k);
	DECLARE_WRITE16_MEMBER(tc4_write_o);
	DECLARE_WRITE16_MEMBER(tc4_write_r);

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	bool index_is_7segled(int index);
	void display_update();


	virtual void machine_start();
};


void hh_tms1k_state::machine_start()
{
	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_cache, 0, sizeof(m_display_cache));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_7seg_mask, 0, sizeof(m_7seg_mask));

	m_o = 0;
	m_r = 0;
	m_inp_mux = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_cache));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_7seg_mask));

	save_item(NAME(m_o));
	save_item(NAME(m_r));
	save_item(NAME(m_inp_mux));
}


/***************************************************************************

  Helper Functions

***************************************************************************/


// The device strobes the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.


void hh_tms1k_state::display_update()
{
	UINT32 active_state[0x20];

	for (int y = 0; y < m_display_maxy; y++)
	{
		active_state[y] = 0;

		for (int x = 0; x < m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			int ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state[y] |= (ds << x);
		}
	}

	// on difference, send to output
	for (int y = 0; y < m_display_maxy; y++)
		if (m_display_cache[y] != active_state[y])
		{
			if (m_7seg_mask[y] != 0)
				output_set_digit_value(y, active_state[y] & m_7seg_mask[y]);

			const int mul = (m_display_maxx <= 10) ? 10 : 100;
			for (int x = 0; x < m_display_maxx; x++)
				output_set_lamp_value(y * mul + x, active_state[y] >> x & 1);
		}

	memcpy(m_display_cache, active_state, sizeof(m_display_cache));
}

TIMER_DEVICE_CALLBACK_MEMBER(hh_tms1k_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x < m_display_maxx; x++)
			if (!(m_display_state[y] >> x & 1) && m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;
	
	display_update();
}


UINT8 hh_tms1k_state::read_inputs(int columns)
{
	UINT8 k = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			k |= m_inp_matrix[i]->read();

	return k;
}



/***************************************************************************

  Minidrivers (I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Coleco Total Control 4
  * TMS1400NLL MP7334-N2 (die labeled MP7334)

  This is a head to head electronic tabletop LED-display sports console.
  One cartridge(Football) was included with the console, the other three were
  sold separately. Gameplay has emphasis on strategy, read the official manual
  on how to play. Remember that you can rotate the view in MESS: rotate left
  for Home(P1) orientation, rotate right for Visitor(P2) orientation.

  Cartridge socket:
  1 N/C
  2 9V+
  3 power switch
  4 K8
  5 K4
  6 K2
  7 K1
  8 R9

  The cartridge connects pin 8 with one of the K-pins.

  Available cartridges:
  - Football    (K8, confirmed)
  - Hockey      (K4?)
  - Soccer      (K2?)
  - Basketball  (K1?)


  TODO:
  - pin configuration of other carts is guessed
  - softlist for the cartridges?
  - offsensive players leds are supposed to look brighter
  - MCU clock is unknown

***************************************************************************/

void hh_tms1k_state::tc4_display()
{
	m_display_maxy = 10;
	m_display_maxx = 9;
	
	// R5,7,8,9 are 7segs
	for (int y = 0; y < m_display_maxy; y++)
		if (y >= 5 && y <= 9 && y != 6)
			m_7seg_mask[y] = 0x7f;
	
	// update current state (note: R6 as extra column!)
	for (int y = 0; y < m_display_maxy; y++)
		m_display_state[y] = (m_r >> y & 1) ? (m_o | (m_r << 2 & 0x100)) : 0;
	
	display_update();
}

READ8_MEMBER(hh_tms1k_state::tc4_read_k)
{
	UINT8 k = read_inputs(6);

	// read from cartridge
	if (m_inp_mux & 0x200)
		k |= m_inp_matrix[6]->read();

	return k;
}

WRITE16_MEMBER(hh_tms1k_state::tc4_write_o)
{
	// O0-O7: leds/7segment
	m_o = data;
	tc4_display();
}

WRITE16_MEMBER(hh_tms1k_state::tc4_write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// R0-R5: input mux
	// R9: to cartridge slot
	m_inp_mux = data & 0x23f;
	
	// R6: led column 8
	// +other: select leds
	m_r = data;
	tc4_display();
}


static INPUT_PORTS_START( tc4 )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Pass/Shoot Button 3") // right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pass/Shoot Button 1") // left
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pass/Shoot Button 2") // middle
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 D/K Button")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN )

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN )

	PORT_START("IN.3") // R3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)

	PORT_START("IN.4") // R4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)

	PORT_START("IN.5") // R5
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Pass/Shoot Button 3") // right
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Pass/Shoot Button 1") // left
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Pass/Shoot Button 2") // middle
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 D/K Button")

	PORT_START("IN.6") // R9
	PORT_CONFNAME( 0x0f, 0x08, "Cartridge")
	PORT_CONFSETTING(    0x01, "Basketball" )
	PORT_CONFSETTING(    0x02, "Soccer" )
	PORT_CONFSETTING(    0x04, "Hockey" )
	PORT_CONFSETTING(    0x08, "Football" )
INPUT_PORTS_END

static MACHINE_CONFIG_START( tc4, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 450000) // approximation - RC osc. R=27.3K, C=100pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, tc4_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, tc4_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, tc4_write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))

	MCFG_DEFAULT_LAYOUT(layout_tc4)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Entex Baseball
  * TMS1000NLP MP0914 (die labeled MP0914A)

***************************************************************************/

// inputs
static INPUT_PORTS_START( ebball )
INPUT_PORTS_END

// machine config
static MACHINE_CONFIG_START( ebball, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 350000) // RC osc. R=43K, C=47pf -> ~350kHz

	MCFG_DEFAULT_LAYOUT(layout_ebball)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tc4 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "tms1400nll_mp7334", 0x0000, 0x1000, CRC(923f3821) SHA1(a9ae342d7ff8dae1dedcd1e4984bcfae68586581) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_tc4_opla.pla", 0, 557, CRC(3b908725) SHA1(f83bf5faa5b3cb51f87adc1639b00d6f9a71ad19) )
ROM_END


ROM_START( ebball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0914", 0x0000, 0x0400, CRC(3c6fb05b) SHA1(b2fe4b3ca72d6b4c9bfa84d67f64afdc215e7178) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ebball_mpla.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball_opla.pla", 0, 365, CRC(062bf5bb) SHA1(8d73ee35444299595961225528b153e3a5fe66bf) )
ROM_END


CONS( 1979, ebball, 0, 0, ebball, ebball, driver_device, 0, "Entex", "Baseball (Entex)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

CONS( 1981, tc4, 0, 0, tc4, tc4, driver_device, 0, "Coleco", "Total Control 4", GAME_SUPPORTS_SAVE )

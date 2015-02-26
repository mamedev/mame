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

#include "amaztron.lh"
#include "ebball.lh"
#include "comp4.lh"
#include "simon.lh"
#include "tc4.lh"


class hh_tms1k_state : public driver_device
{
public:
	hh_tms1k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker"),
		m_display_maxy(1),
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

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	bool index_is_7segled(int index);
	void display_update();
	
	UINT8 read_inputs(int columns);

	// game-specific handlers
	void amaztron_display();
	DECLARE_READ8_MEMBER(amaztron_read_k);
	DECLARE_WRITE16_MEMBER(amaztron_write_o);
	DECLARE_WRITE16_MEMBER(amaztron_write_r);

	void tc4_display();
	DECLARE_READ8_MEMBER(tc4_read_k);
	DECLARE_WRITE16_MEMBER(tc4_write_o);
	DECLARE_WRITE16_MEMBER(tc4_write_r);

	DECLARE_READ8_MEMBER(comp4_read_k);
	DECLARE_WRITE16_MEMBER(comp4_write_o);
	DECLARE_WRITE16_MEMBER(comp4_write_r);

	DECLARE_READ8_MEMBER(simon_read_k);
	DECLARE_WRITE16_MEMBER(simon_write_o);
	DECLARE_WRITE16_MEMBER(simon_write_r);

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

  Coleco Amaze-A-Tron, by Ralph Baer
  * TMS1100 MCU, labeled MP3405(die label too)

  This is an electronic board game with a selection of 8 maze games,
  most of them for 2 players. A 5x5 playing grid and four markers are
  required to play. Refer to the official manual for more information.

***************************************************************************/


void hh_tms1k_state::amaztron_display()
{
	m_display_maxy = 3;
	m_display_maxx = 8;

	// R8,R9: select digit
	for (int y = 0; y < 2; y++)
	{
		m_7seg_mask[y] = 0x7f;
		m_display_state[y] = (m_r >> (y + 8) & 1) ? m_o : 0;
	}
	
	// R6,R7: lamps -> lamp20, lamp21
	m_display_state[2] = m_r >> 6 & 3;
	
	display_update();
}

READ8_MEMBER(hh_tms1k_state::amaztron_read_k)
{
	UINT8 k = read_inputs(6);

	// the 5th column is tied to K4+K8
	if (k & 0x10) k |= 0xc;
	return k & 0xf;
}

WRITE16_MEMBER(hh_tms1k_state::amaztron_write_r)
{
	// R0-R5: input mux
	m_inp_mux = data & 0x3f;
	
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);

	// other bits:
	m_r = data;
	amaztron_display();
}

WRITE16_MEMBER(hh_tms1k_state::amaztron_write_o)
{
	// O0-O6: digit segments
	// O7: N/C
	m_o = data & 0x7f;
	amaztron_display();
}


static INPUT_PORTS_START( amaztron )
	PORT_START("IN.0") // R0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Button 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Button 11")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Button 16")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Button 21")

	PORT_START("IN.1") // R1
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Button 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Button 7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Button 12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Button 17")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Button 22")

	PORT_START("IN.2") // R2
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Button 3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Button 8")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Button 13")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Button 18")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Button 23")

	PORT_START("IN.3") // R3
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Button 4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Button 9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Button 14")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Button 19")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Button 24")

	PORT_START("IN.4") // R4
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Button 5")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Button 10")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Button 15")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Button 20")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Button 25")

	PORT_START("IN.5") // R5
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Game Select")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("Game Start")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


static MACHINE_CONFIG_START( amaztron, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, 350000) // RC osc. R=33K?, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, amaztron_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, amaztron_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, amaztron_write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))

	MCFG_DEFAULT_LAYOUT(layout_amaztron)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END




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

  Milton Bradley Comp IV
  * TMC0904NL CP0904A (die labeled 4A0970D-04A)

  This is small tabletop Mastermind game; a code-breaking game where the player
  needs to find out the correct sequence of colours (numbers in our case).
  It is known as Logic 5 in Europe, and as Pythaligoras in Japan.

  Press the R key to start, followed by a set of unique numbers and E.
  Refer to the official manual for more information.


  TODO:
  - MCU clock is unknown

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::comp4_read_k)
{
	return read_inputs(3);
}

WRITE16_MEMBER(hh_tms1k_state::comp4_write_r)
{
	// leds:
	// R4    R9
	// R10!  R8
	// R2    R7
	// R1    R6
	// R0    R5
	m_display_maxx = 11;
	m_display_state[0] = data;
	display_update();
}

WRITE16_MEMBER(hh_tms1k_state::comp4_write_o)
{
	// O0: leds common (always writes 1)
	// O1-O3: input mux
	// other bits: N/C
	m_inp_mux = data >> 1 & 7;
}


static INPUT_PORTS_START( comp4 )
	PORT_START("IN.0") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("R")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("4")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("7")

	PORT_START("IN.1") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("5")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("8")

	PORT_START("IN.2") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("E")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("3")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("6")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9")
INPUT_PORTS_END



static MACHINE_CONFIG_START( comp4, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, 250000) // approximation - unknown freq
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, comp4_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, comp4_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, comp4_write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))

	MCFG_DEFAULT_LAYOUT(layout_comp4)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END


/***************************************************************************

  Milton Bradley Simon, created by Ralph Baer

  Revision A hardware:
  * TMS1000 (has internal ROM), DS75494 lamp driver

  Newer revisions have a smaller 16-pin MB4850 chip instead of the TMS1000.
  This one has been decapped too, but we couldn't find an internal ROM.
  It is possibly a cost-reduced custom ASIC specifically for Simon.

  Other games assumed to be on similar hardware:
  - Pocket Simon, but there's a chance it only exists with MB4850 chip
  - Super Simon (TMS1100)

***************************************************************************/





/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(hh_tms1k_state::simon_read_k)
{
	return read_inputs(4);
}

WRITE16_MEMBER(hh_tms1k_state::simon_write_r)
{
	// R4-R8 go through an 75494 IC first:
	// R4 -> 75494 IN6 -> green lamp
	// R5 -> 75494 IN3 -> red lamp
	// R6 -> 75494 IN5 -> yellow lamp
	// R7 -> 75494 IN2 -> blue lamp
	m_display_maxx = 4;
	m_display_state[0] = data >> 4 & 0xf;
	display_update();

	// R8 -> 75494 IN0 -> speaker
	m_speaker->level_w(data >> 8 & 1);

	// R0,R1,R2,R9: input mux
	// R3: GND
	// other bits: N/C
	m_inp_mux = (data & 7) | (data >> 6 & 8);
}

WRITE16_MEMBER(hh_tms1k_state::simon_write_o)
{
	// N/C
}


static INPUT_PORTS_START( simon )
	PORT_START("IN.0") // R0
	PORT_CONFNAME( 0x07, 0x02, "Game Select")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x01, "2" )
	PORT_CONFSETTING(    0x04, "3" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Green Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Red Button")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Blue Button")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START ) PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Last")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Longest")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN.3") // R9
	PORT_CONFNAME( 0x0f, 0x01, "Skill Level")
	PORT_CONFSETTING(    0x02, "1" )
	PORT_CONFSETTING(    0x04, "2" )
	PORT_CONFSETTING(    0x08, "3" )
	PORT_CONFSETTING(    0x01, "4" )
INPUT_PORTS_END


static MACHINE_CONFIG_START( simon, hh_tms1k_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1000, 350000) // RC osc. R=33K, C=100pf -> ~350kHz
	MCFG_TMS1XXX_READ_K_CB(READ8(hh_tms1k_state, simon_read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(hh_tms1k_state, simon_write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(hh_tms1k_state, simon_write_r))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))

	MCFG_DEFAULT_LAYOUT(layout_simon)

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


ROM_START( amaztron )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "tms1100nll_mp3405", 0x0000, 0x0800, CRC(9cbc0009) SHA1(17772681271b59280687492f37fa0859998f041d) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_amaztron_mpla.pla", 0, 867, CRC(03574895) SHA1(04407cabfb3adee2ee5e4218612cb06c12c540f4) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_amaztron_opla.pla", 0, 365, CRC(f3875384) SHA1(3c256a3db4f0aa9d93cf78124db39f4cbdc57e4a) )
ROM_END




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


ROM_START( comp4 )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tmc0904nl_cp0904a", 0x0000, 0x0400, CRC(6233ee1b) SHA1(738e109b38c97804b4ec52bed80b00a8634ad453) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_comp4_mpla.pla", 0, 860, CRC(ee9d7d9e) SHA1(25484e18f6a07f7cdb21a07220e2f2a82fadfe7b) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_comp4_opla.pla", 0, 352, CRC(a0f887d1) SHA1(3c666663d484d5bed81e1014f8715aab8a3d489f) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_comp4_spla.pla", 0, 157, CRC(e5bddd90) SHA1(4b1c6512c70e5bcd23c2dbf0c88cd8aa2c632a10) )
ROM_END


ROM_START( simon )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "tms1000.u1", 0x0000, 0x0400, CRC(9961719d) SHA1(35dddb018a8a2b31f377ab49c1f0cb76951b81c0) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_simon_mpla.pla", 0, 867, CRC(52f7c1f1) SHA1(dbc2634dcb98eac173ad0209df487cad413d08a5) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_simon_opla.pla", 0, 365, CRC(2943c71b) SHA1(bd5bb55c57e7ba27e49c645937ec1d4e67506601) )
ROM_END




CONS( 1979, amaztron, 0, 0, amaztron, amaztron, driver_device, 0, "Coleco", "Amaze-A-Tron", GAME_SUPPORTS_SAVE )
CONS( 1981, tc4, 0, 0, tc4, tc4, driver_device, 0, "Coleco", "Total Control 4", GAME_SUPPORTS_SAVE )

CONS( 1979, ebball, 0, 0, ebball, ebball, driver_device, 0, "Entex", "Baseball (Entex)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

CONS( 1977, comp4, 0, 0, comp4, comp4, driver_device, 0, "Milton Bradley", "Comp IV", GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )
CONS( 1978, simon, 0, 0, simon, simon, driver_device, 0, "Milton Bradley", "Simon (Rev. A)", GAME_SUPPORTS_SAVE )

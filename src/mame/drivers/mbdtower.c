// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  ** subclass of hh_tms1k_state (includes/hh_tms1k.h, drivers/hh_tms1k.c) **

  Milton Bradley Dark Tower
  * TMS1400NLL MP7332-N1.U1(Rev. B) or MP7332-N2LL(Rev. C), die labeled MP7332
    (assume same ROM contents between revisions)
  * SN75494N MOS-to-LED digit driver
  * motorized rotating reel + lightsensor, 1bit-sound

  This is a board game, it obviously requires game pieces and the board.
  The emulated part is the centerpiece, a black tower with a rotating card
  panel and LED digits for displaying health, amount of gold, etc. As far
  as MAME is concerned, the game works fine.

  To start up the game, first press [MOVE], the machine now does a self-test.
  Then select level and number of players and the game will start. Read the
  official manual on how to play the game.

***************************************************************************/

#include "includes/hh_tms1k.h"
#include "mbdtower.lh"


class mbdtower_state : public hh_tms1k_state
{
public:
	mbdtower_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void prepare_display();
	bool sensor_led_on() { return m_display_decay[0][0] != 0; }

	int m_motor_pos;
	int m_motor_pos_prev;
	int m_motor_decay;
	bool m_motor_on;
	bool m_sensor_blind;

	TIMER_DEVICE_CALLBACK_MEMBER(motor_sim_tick);

	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

protected:
	virtual void machine_start();
};


/***************************************************************************

  Display, Motor

***************************************************************************/

void mbdtower_state::prepare_display()
{
	// declare display matrix size and the 2 7segs
	set_display_size(7, 3);
	m_display_segmask[1] = m_display_segmask[2] = 0x7f;

	// update current state
	if (~m_r & 0x10)
	{
		UINT8 o = BITSWAP8(m_o,7,0,4,3,2,1,6,5) & 0x7f;
		m_display_state[2] = (m_o & 0x80) ? o : 0;
		m_display_state[1] = (m_o & 0x80) ? 0 : o;
		m_display_state[0] = (m_r >> 8 & 1) | (m_r >> 4 & 0xe);

		display_update();
	}
	else
	{
		// display items turned off
		display_matrix(7, 3, 0, 0);
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(mbdtower_state::motor_sim_tick)
{
	// it rotates counter-clockwise (when viewed from above)
	if (m_motor_on)
	{
		m_motor_pos = (m_motor_pos - 1) & 0x7f;

		// give it some time to spin out when it's turned off
		if (m_r & 0x200)
			m_motor_decay += (m_motor_decay < 4);
		else if (m_motor_decay > 0)
			m_motor_decay--;
		else
			m_motor_on = false;
	}

	// 8 evenly spaced holes in the rotation disc for the sensor to 'see' through.
	// The first hole is much bigger, enabling the game to determine the position.
	if ((m_motor_pos & 0xf) < 4 || m_motor_pos < 0xc)
		m_sensor_blind = false;
	else
		m_sensor_blind = true;

	// on change, output info
	if (m_motor_pos != m_motor_pos_prev)
		output_set_value("motor_pos", 100 * (m_motor_pos / (float)0x80));

	/* 3 display cards per hole, like this:

	    (0)                <---- display increments this way <----                   (7)

	    CURSED   VICTORY    WIZARD         DRAGON    GOLD KEY     SCOUT    WARRIOR   (void)
	    LOST     WARRIORS   BAZAAR CLOSED  SWORD     SILVER KEY   HEALER   FOOD      (void)
	    PLAGUE   BRIGANDS   KEY MISSING    PEGASUS   BRASS KEY    GOLD     BEAST     (void)
	*/
	int card_pos = m_motor_pos >> 4 & 7;
	if (card_pos != (m_motor_pos_prev >> 4 & 7))
		output_set_value("card_pos", card_pos);

	m_motor_pos_prev = m_motor_pos;
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE16_MEMBER(mbdtower_state::write_r)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;

	// R9: motor on
	if ((m_r ^ data) & 0x200)
		output_set_value("motor_on", data >> 9 & 1);
	if (data & 0x200)
		m_motor_on = true;

	// R3: N/C
	// R4: 75494 /EN (speaker, lamps, digit select go through that IC)
	// R5-R7: tower lamps
	// R8: rotation sensor led
	m_r = data;
	prepare_display();

	// R10: speaker out
	m_speaker->level_w(~data >> 4 & data >> 10 & 1);
}

WRITE16_MEMBER(mbdtower_state::write_o)
{
	// O0-O6: led segments A-G
	// O7: digit select
	m_o = data;
	prepare_display();
}

READ8_MEMBER(mbdtower_state::read_k)
{
	// K: multiplexed inputs
	// K8: rotation sensor
	return read_inputs(3) | ((!m_sensor_blind && sensor_led_on()) ? 8 : 0);
}



/***************************************************************************

  Inputs

***************************************************************************/

/* physical button layout and labels is like this:

    (green)     (l.blue)    (red)
    [YES/       [REPEAT]    [NO/
     BUY]                    END]

    (yellow)    (blue)      (white)
    [HAGGLE]    [BAZAAR]    [CLEAR]

    (blue)      (blue)      (blue)
    [TOMB/      [MOVE]      [SANCTUARY/
     RUIN]                   CITADEL]

    (orange)    (blue)      (d.yellow)
    [DARK       [FRONTIER]  [INVENTORY]
     TOWER]
*/

static INPUT_PORTS_START( mbdtower )
	PORT_START("IN.0") // R0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Inventory")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_NAME("No/End")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Clear")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("Sanctuary/Citadel")

	PORT_START("IN.1") // R1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Frontier")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_NAME("Repeat")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Bazaar")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Move")

	PORT_START("IN.2") // R2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Dark Tower")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_NAME("Yes/Buy")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Haggle")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Tomb/Ruin")
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void mbdtower_state::machine_start()
{
	hh_tms1k_state::machine_start();

	// zerofill
	m_motor_pos = 0;
	m_motor_pos_prev = -1;
	m_motor_decay = 0;
	m_motor_on = false;
	m_sensor_blind = false;

	// register for savestates
	save_item(NAME(m_motor_pos));
	/* save_item(NAME(m_motor_pos_prev)); */ // don't save!
	save_item(NAME(m_motor_decay));
	save_item(NAME(m_motor_on));
	save_item(NAME(m_sensor_blind));
}


static MACHINE_CONFIG_START( mbdtower, mbdtower_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 425000) // approximation - RC osc. R=43K, C=56pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(mbdtower_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(mbdtower_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(mbdtower_state, write_o))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("tower_motor", mbdtower_state, motor_sim_tick, attotime::from_msec(3500/0x80)) // ~3.5sec for a full rotation
	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", hh_tms1k_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_mbdtower)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mbdtower )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mp7332", 0x0000, 0x1000, CRC(ebeab91a) SHA1(7edbff437da371390fa8f28b3d183f833eaa9be9) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_mbdtower_output.pla", 0, 557, CRC(64c84697) SHA1(72ce6d24cedf9c606f1742cd5620f75907246e87) )
ROM_END


CONS( 1981, mbdtower, 0, 0, mbdtower, mbdtower, driver_device, 0, "Milton Bradley", "Dark Tower (Milton Bradley)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL )

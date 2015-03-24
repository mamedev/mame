// license:BSD-3-Clause
// copyright-holders:hap, Sean Riddle
/***************************************************************************

  Milton Bradley Dark Tower
  * TMS1400NLL MP7332-N1.U1(Rev. B) or MP7332-N2LL(Rev. C), die labeled MP7332
  * SN75494N MOS-to-LED digit driver

  x

***************************************************************************/

#include "includes/hh_tms1k.h"
#include "mbdtower.lh"


class mbdtower_state : public hh_tms1k_state
{
public:
	mbdtower_state(const machine_config &mconfig, device_type type, const char *tag)
		: hh_tms1k_state(mconfig, type, tag)
	{ }

	void mbdtower_display();
	DECLARE_WRITE16_MEMBER(write_r);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_READ8_MEMBER(read_k);

	bool m_motor_on;
	bool m_sensor;

protected:
	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

void mbdtower_state::mbdtower_display()
{
}

WRITE16_MEMBER(mbdtower_state::write_r)
{
	// R0-R2: input mux
	m_inp_mux = data & 7;
	
	// R3: N/C
	// R4: 75494 enable (speaker, lamps, digit select go through that IC)
	// R5-R7: tower lamps
	// R8: rotation sensor led
	// R9: motor on
	
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);
}

WRITE16_MEMBER(mbdtower_state::write_o)
{
	// O0-O6: led segments A-G
	// O7: digit select
	m_o = data;
}


READ8_MEMBER(mbdtower_state::read_k)
{
	// rotation sensor is on K8
	
	return read_inputs(3);
}


/* tower motor simulation:

*/



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

	// zerofill/register for savestates
	m_motor_on = false;
	m_sensor = false;
	
	save_item(NAME(m_motor_on));
	save_item(NAME(m_sensor));
}



static MACHINE_CONFIG_START( mbdtower, mbdtower_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1400, 400000) // approximation - RC osc. R=43K, C=56pf, but unknown RC curve
	MCFG_TMS1XXX_READ_K_CB(READ8(mbdtower_state, read_k))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(mbdtower_state, write_r))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(mbdtower_state, write_o))

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
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 557, "maincpu:opla", 0 )
	ROM_LOAD( "tms1400_mbdtower_opla.pla", 0, 557, CRC(64c84697) SHA1(72ce6d24cedf9c606f1742cd5620f75907246e87) )
ROM_END


CONS( 1981, mbdtower, 0, 0, mbdtower, mbdtower, driver_device, 0, "Milton Bradley", "Dark Tower (Milton Bradley)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )


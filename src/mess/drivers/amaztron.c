// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Coleco Amaze-A-Tron, by Ralph Baer
  * TMS1100 MCU, labeled MP3405(die label too)

  This is an electronic board game with a selection of 8 maze games,
  most of them for 2 players. A 5x5 playing grid and four markers are
  required to play. Refer to the official manual for more information.

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "amaztron.lh"

// master clock is a single stage RC oscillator: R=33K?, C=100pf,
// according to the TMS 1000 series data manual this is around 350kHz
#define MASTER_CLOCK (350000)


class amaztron_state : public driver_device
{
public:
	amaztron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<6> m_button_matrix;
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

void amaztron_state::leds_update()
{
	for (int i = 0; i < 2; i++)
		if (m_r >> (i + 8) & 1)
			output_set_digit_value(i, m_o);
}

READ8_MEMBER(amaztron_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 6; i++)
	{
		if (m_r >> i & 1)
			k |= m_button_matrix[i]->read();
	}

	// the 5th row is tied to K4+K8
	if (k & 0x10) k |= 0xc;
	return k & 0xf;
}

WRITE16_MEMBER(amaztron_state::write_r)
{
	// R0-R5: input mux
	// R6,R7: lamps
	output_set_lamp_value(0, data >> 6 & 1);
	output_set_lamp_value(1, data >> 7 & 1);

	// R8,R9: select digit
	m_r = data;
	leds_update();

	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);
}

WRITE16_MEMBER(amaztron_state::write_o)
{
	// O0-O6: digit segments
	// O7: N/C
	m_o = data & 0x7f;
	leds_update();
}



/***************************************************************************

  Inputs

***************************************************************************/

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



/***************************************************************************

  Machine Config

***************************************************************************/

void amaztron_state::machine_start()
{
	m_r = 0;
	m_o = 0;

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( amaztron, amaztron_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(amaztron_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(amaztron_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(amaztron_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_amaztron)

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


CONS( 1979, amaztron, 0, 0, amaztron, amaztron, driver_device, 0, "Coleco", "Amaze-A-Tron", GAME_SUPPORTS_SAVE )

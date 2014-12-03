// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Code Name: Sector
  * MP0905BNL ZA0379 (die labeled 0970F-05B)


***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

#include "cnsector.lh"


// master clock is cpu internal, the value below is an approximation
#define MASTER_CLOCK (250000)


class cnsector_state : public driver_device
{
public:
	cnsector_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<5> m_button_matrix;

	UINT16 m_o;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};



/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(cnsector_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 5; i++)
		if (m_o & (1 << i))
			k |= m_button_matrix[i]->read();
	
	return k;
}

WRITE16_MEMBER(cnsector_state::write_r)
{
	// R0-R5: select digit
	for (int i = 0; i < 6; i++)
		output_set_digit_value(i, (data >> i & 1) ? m_o : 0);

	// R6-R9: direction leds
	for (int i = 6; i < 10; i++)
		output_set_lamp_value(i - 6, data >> i & 1);
}

WRITE16_MEMBER(cnsector_state::write_o)
{
	// O0-O4: input mux
	// O0-O7: digit segments
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cnsector )
	PORT_START("IN.0") // O0
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) // ?  next
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) // ?  left
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) // nc?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) // ?  range

	PORT_START("IN.1") // O1
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) // ?  aim
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) // ?  right
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) // nc?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) // ?nc

	PORT_START("IN.2") // O2
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) // ?  fire
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) // ?  evasive
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) // nc?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) // ?  recall

	PORT_START("IN.3") // O3
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) // ?  finder
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) // ?  slow
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) // nc?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) // ?nc

	PORT_START("IN.4") // O4
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) // ?  teach
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) // ?  fast
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) // nc?
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) // ?  move
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void cnsector_state::machine_start()
{
	m_o = 0;
	
	save_item(NAME(m_o));
}


static MACHINE_CONFIG_START( cnsector, cnsector_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, MASTER_CLOCK)
	MCFG_TMS1XXX_READ_K_CB(READ8(cnsector_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(cnsector_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(cnsector_state, write_r))
	
	MCFG_DEFAULT_LAYOUT(layout_cnsector)

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cnsector )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0905bnl_za0379", 0x0000, 0x0400, CRC(564fe1a0) SHA1(825840a73175eee12e9712c871799f00e3be2c53) )

	ROM_REGION( 782, "maincpu:ipla", 0 )
	ROM_LOAD( "tms0970_default_ipla.pla", 0, 782, CRC(e038fc44) SHA1(dfc280f6d0a5828d1bb14fcd59ac29caf2c2d981) )
	ROM_REGION( 860, "maincpu:mpla", 0 )
	ROM_LOAD( "tms0970_cnsector_mpla.pla", 0, 860, CRC(059f5bb4) SHA1(2653766f9fd74d41d44013bb6f54c0973a6080c9) )
	ROM_REGION( 352, "maincpu:opla", 0 )
	ROM_LOAD( "tms0970_cnsector_opla.pla", 0, 352, CRC(7c0bdcd6) SHA1(dade774097e8095dca5deac7b2367d0c701aca51) )
	ROM_REGION( 157, "maincpu:spla", 0 )
	ROM_LOAD( "tms0970_cnsector_spla.pla", 0, 157, CRC(56c37a4f) SHA1(18ecc20d2666e89673739056483aed5a261ae927) )
ROM_END


CONS( 1977, cnsector, 0, 0, cnsector, cnsector, driver_device, 0, "Parker Brothers", "Code Name: Sector", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )

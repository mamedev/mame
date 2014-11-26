// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Texas Instruments TMS1xxx/0970/0980 handheld calculators

  Texas Instruments WIZ-A-TRON
  * TMC0907NL DP0907BS (die labeled 0970F-07B)

  Other handhelds assumed to be on similar hardware:
  - Math Magic
  - Little Professor


  TODO:
  - the rom goes in an infinite loop very soon, cpu missing emulation?

***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"

// master clock is cpu internal, the value below is an approximation
#define MASTER_CLOCK (250000)


class ticalc1x_state : public driver_device
{
public:
	ticalc1x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_button_matrix(*this, "IN")
	{ }

	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_button_matrix;

	UINT16 m_r;
	UINT16 m_o;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(ticalc1x_state::read_k)
{
	UINT8 k = 0;

	// read selected button rows
	for (int i = 0; i < 4; i++)
		if (m_o & (1 << (i + 1)))
			k |= m_button_matrix[i]->read();

	return k;
}

WRITE16_MEMBER(ticalc1x_state::write_r)
{
	// R..: select digit
	m_r = data;
}

WRITE16_MEMBER(ticalc1x_state::write_o)
{
	// O0-O6: digit segments A-G
	// O1-O4: input mux
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( wizatron )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )

	PORT_START("IN.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void ticalc1x_state::machine_start()
{
	m_r = 0;
	m_o = 0;

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static const UINT16 wizatron_output_pla[0x20] =
{
	// 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, +, -, X, /, r
	0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70,
	0x7f, 0x7b, 0x26, 0x02, 0x35, 0x4a, 0x05, 0x00,
	0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00,
	0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00, 0xff00
};


static MACHINE_CONFIG_START( wizatron, ticalc1x_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(wizatron_output_pla)
	MCFG_TMS1XXX_READ_K(READ8(ticalc1x_state, read_k))
	MCFG_TMS1XXX_WRITE_O(WRITE16(ticalc1x_state, write_o))
	MCFG_TMS1XXX_WRITE_R(WRITE16(ticalc1x_state, write_r))

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wizatron )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "dp0907bs", 0x0000, 0x0400, CRC(5a6af094) SHA1(b1f27e1f13f4db3b052dd50fb08dbf9c4d8db26e) )
ROM_END


CONS( 1977, wizatron, 0, 0, wizatron, wizatron, driver_device, 0, "Texas Instruments", "Wiz-A-Tron", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )

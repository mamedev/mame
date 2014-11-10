// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Texas Instruments WIZ-A-TRON
  * TMC0907NL ZA0379 (die labeled 0970F-07B)

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


class wizatron_state : public driver_device
{
public:
	wizatron_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

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

READ8_MEMBER(wizatron_state::read_k)
{
	UINT8 k = 0;
	
	return k;
}

WRITE16_MEMBER(wizatron_state::write_r)
{
	m_r = data;
}

WRITE16_MEMBER(wizatron_state::write_o)
{
	m_o = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( wizatron )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void wizatron_state::machine_start()
{
	m_r = 0;
	m_o = 0;

	save_item(NAME(m_r));
	save_item(NAME(m_o));
}


static const UINT16 wizatron_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};


static MACHINE_CONFIG_START( wizatron, wizatron_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS0970, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(wizatron_output_pla)
	MCFG_TMS1XXX_READ_K(READ8(wizatron_state, read_k))
	MCFG_TMS1XXX_WRITE_O(WRITE16(wizatron_state, write_o))
	MCFG_TMS1XXX_WRITE_R(WRITE16(wizatron_state, write_r))

	/* no video! */

	/* no sound! */
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wizatron )
	ROM_REGION( 0x0400, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "za0379", 0x0000, 0x0400, CRC(5a6af094) SHA1(b1f27e1f13f4db3b052dd50fb08dbf9c4d8db26e) )
ROM_END


CONS( 1977, wizatron, 0, 0, wizatron, wizatron, driver_device, 0, "Texas Instruments", "Wiz-A-Tron", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE | GAME_NO_SOUND_HW )

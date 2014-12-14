// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Tandy Radio Shack Tandy-12 - Computerized Arcade
  * TMS1100 CD7282SL
  
  This tabletop game looks and plays like "Fabulous Fred" by the Japanese
  company Mego Corp. in 1980, which in turn is a mix of Merlin and Simon.
  Unlike Merlin and Simon, these spin-offs were not successful.
  
***************************************************************************/

#include "emu.h"
#include "cpu/tms0980/tms0980.h"
#include "sound/speaker.h"

#include "tandy12.lh" // clickable

// master clock is a single stage RC oscillator: R=39K, C=47pf,
// according to the TMS 1000 series data manual this is around 400kHz
#define MASTER_CLOCK (400000)


class tandy12_state : public driver_device
{
public:
	tandy12_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<tms1xxx_cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_o;
	UINT16 m_r;

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);

	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

READ8_MEMBER(tandy12_state::read_k)
{
	return 0;
}

WRITE16_MEMBER(tandy12_state::write_o)
{
	m_o = data;
}

WRITE16_MEMBER(tandy12_state::write_r)
{
	// R10: speaker out
	m_speaker->level_w(data >> 10 & 1);
	
	m_r = data;
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( tandy12 )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void tandy12_state::machine_start()
{
	m_o = 0;
	m_r = 0;

	save_item(NAME(m_o));
	save_item(NAME(m_r));
}


static const UINT16 tandy12_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};


static MACHINE_CONFIG_START( tandy12, tandy12_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS1100, MASTER_CLOCK)
	MCFG_TMS1XXX_OUTPUT_PLA(tandy12_output_pla)
	MCFG_TMS1XXX_READ_K_CB(READ8(tandy12_state, read_k))
	MCFG_TMS1XXX_WRITE_O_CB(WRITE16(tandy12_state, write_o))
	MCFG_TMS1XXX_WRITE_R_CB(WRITE16(tandy12_state, write_r))

	MCFG_DEFAULT_LAYOUT(layout_tandy12)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tandy12 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "cd7282sl", 0x0000, 0x800, CRC(a10013dd) SHA1(42ebd3de3449f371b99937f9df39c240d15ac686) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_default_mpla.pla", 0, 867, BAD_DUMP CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) ) // not verified
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_tandy12_opla.pla", 0, 365, NO_DUMP )
ROM_END


CONS( 1981, tandy12, 0, 0, tandy12, tandy12, driver_device, 0, "Tandy Radio Shack", "Tandy-12 - Computerized Arcade", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

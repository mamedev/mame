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


class hh_tms1k_state : public driver_device
{
public:
	hh_tms1k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
//		m_button_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
//	required_ioport_array<3> m_button_matrix;
	required_device<speaker_sound_device> m_speaker;

	virtual void machine_start();
};


static INPUT_PORTS_START( ebball )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void hh_tms1k_state::machine_start()
{
}


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

ROM_START( ebball )
	ROM_REGION( 0x0400, "maincpu", 0 )
	ROM_LOAD( "mp0914", 0x0000, 0x0400, CRC(3c6fb05b) SHA1(b2fe4b3ca72d6b4c9bfa84d67f64afdc215e7178) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1000_ebball_mpla.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1000_ebball_opla.pla", 0, 365, CRC(062bf5bb) SHA1(8d73ee35444299595961225528b153e3a5fe66bf) )
ROM_END


CONS( 1979, ebball, 0, 0, ebball, ebball, driver_device, 0, "Entex", "Baseball (Entex)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

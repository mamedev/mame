// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Collection of PIC16xx/16Cxx-driven dedicated handhelds or other simple devices


***************************************************************************/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/speaker.h"

#include "maniac.lh"


class hh_pic16_state : public driver_device
{
public:
	hh_pic16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
//		m_inp_matrix(*this, "IN"),
		m_speaker(*this, "speaker")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
//	optional_ioport_array<3> m_inp_matrix; // max 3
	optional_device<speaker_sound_device> m_speaker;

	virtual void machine_start();
};


void hh_pic16_state::machine_start()
{
}






/***************************************************************************

  Minidrivers (I/O, Inputs, Machine Config)

***************************************************************************/

/***************************************************************************

  Ideal Maniac, by Ralph Baer
  * PIC1655-036


***************************************************************************/


static INPUT_PORTS_START( maniac )
INPUT_PORTS_END


static MACHINE_CONFIG_START( maniac, hh_pic16_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", PIC16C55, 500000)

	MCFG_DEFAULT_LAYOUT(layout_maniac)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



ROM_START( maniac )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "1655-036", 0x0000, 0x0400, CRC(a96f7011) SHA1(e97ae44d3c1e74c7e1024bb0bdab03eecdc9f827) )
ROM_END


CONS( 1979, maniac, 0, 0, maniac, maniac, driver_device, 0, "Ideal", "Maniac", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

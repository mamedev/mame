// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Parker Brothers Wildfire
  * AMI S2150, labeled C10641


***************************************************************************/

#include "emu.h"
#include "cpu/amis2000/amis2000.h"
#include "sound/speaker.h"

#include "wildfire.lh"

// master clock is a single stage RC oscillator: R=?K, C=?pf,
// S2150 default frequency is 850kHz
#define MASTER_CLOCK (850000)


class wildfire_state : public driver_device
{
public:
	wildfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	DECLARE_WRITE8_MEMBER(write_d);
	DECLARE_WRITE16_MEMBER(write_a);

	virtual void machine_start();
};


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(wildfire_state::write_d)
{
}

WRITE16_MEMBER(wildfire_state::write_a)
{
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( wildfire )
	PORT_START("IN1") // I
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Shooter Button")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Right Flipper")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void wildfire_state::machine_start()
{
}


static MACHINE_CONFIG_START( wildfire, wildfire_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", AMI_S2150, MASTER_CLOCK)
	MCFG_AMI_S2000_READ_I_CB(IOPORT("IN1"))
	MCFG_AMI_S2000_WRITE_D_CB(WRITE8(wildfire_state, write_d))
	MCFG_AMI_S2000_WRITE_A_CB(WRITE16(wildfire_state, write_a))

	MCFG_DEFAULT_LAYOUT(layout_wildfire)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wildfire )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "us4341385", 0x0000, 0x0400, CRC(84ac0f1f) SHA1(1e00ddd402acfc2cc267c34eed4b89d863e2144f) ) // from patent US4334679, data should be correct (it included checksums)
	ROM_CONTINUE(          0x0600, 0x0200 )
ROM_END


CONS( 1979, wildfire, 0, 0, wildfire, wildfire, driver_device, 0, "Parker Brothers", "Wildfire (prototype)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

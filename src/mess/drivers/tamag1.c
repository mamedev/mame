// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Bandai Tamagotchi generation 1 hardware
  
  ** SKELETON Driver - feel free to add notes here, but driver itself is WIP

***************************************************************************/

#include "emu.h"
#include "cpu/e0c6200/e0c6s46.h"
#include "sound/speaker.h"


class tamag1_state : public driver_device
{
public:
	tamag1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<e0c6s46_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	DECLARE_PALETTE_INIT(tama);
};



PALETTE_INIT_MEMBER(tamag1_state, tama)
{
	palette.set_pen_color(0, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00));
}




static INPUT_PORTS_START( tama )
INPUT_PORTS_END



static MACHINE_CONFIG_START( tama, tamag1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", E0C6S46, XTAL_32_768kHz)
//	MCFG_E0C6S46_PIXEL_UPDATE_CB(tama_pixel_update)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(XTAL_32_768kHz/1024)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40, 16)
	MCFG_SCREEN_VISIBLE_AREA(0, 40-1, 0, 16-1)
//	MCFG_DEFAULT_LAYOUT(layout_tama)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", e0c6s46_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(tamag1_state, tama)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tama )
	ROM_REGION( 0x3000, "maincpu", 0 )
//	ROM_LOAD( "test.b", 0x0000, 0x3000, CRC(4372220e) SHA1(6e13d015113e16198c0059b9d0c38d7027ae7324) )
	ROM_LOAD( "tama.b", 0x0000, 0x3000, CRC(5c864cb1) SHA1(4b4979cf92dc9d2fb6d7295a38f209f3da144f72) )
ROM_END


CONS( 1997, tama, 0, 0, tama, tama, driver_device, 0, "Bandai", "Tamagotchi (USA)", GAME_NOT_WORKING )

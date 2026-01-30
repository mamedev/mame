// license:BSD-3-Clause
// copyright-holders:


/*********************************************************************************************

    Skeleton driver for toy computers on VTech 5303 hardware.

    PCB with a 25VQ16A serial flash and a 8 MHz xtal on one side and a big glob on the other.

*********************************************************************************************/


#include "emu.h"

#include "cpu/m6502/w65c02.h"

#include "screen.h"
#include "speaker.h"


namespace {


class vtech5303_state : public driver_device
{
public:
	vtech5303_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void vtech5303(machine_config &config) ATTR_COLD;

protected:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update_vtech5303(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t vtech5303_state::screen_update_vtech5303(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* The Paw Patrol the keyboard has:
    -6 "character" buttons (Marshall, Rocky, Chase, Skye, Ryder, Rubble).
    -6 "activity" buttons (bone, letters, maths, pairs, maze, adventure).
    -26 leter keys in QWERTY layout (QWERTYUIOP ASDFGHJKL ZXCVBNM).
    -Mute button to the left of Z.
    -3 navigation buttons (right arrow, left arrow, OK).
    -Clock button.
    The Spanish version repurposes the mute button as a letter key and organises
    the letters in alphabetical order (ABCDEFGHIJ KLMNÑOPQR STUVWXYZ)
*/
INPUT_PORTS_START( vtech5303 )
INPUT_PORTS_END

void vtech5303_state::vtech5303(machine_config &config)
{
	W65C02(config, m_maincpu, 8_MHz_XTAL); // Unknown core and frequency, probably 6802

	SCREEN(config, m_screen, SCREEN_TYPE_LCD); // Monochrome 64x32 LCD screen
	m_screen->set_refresh_hz(60); // Guess
	m_screen->set_size(64, 32);
	m_screen->set_visarea(0, 64-1, 0, 32-1);
	m_screen->set_screen_update(FUNC(vtech5303_state::screen_update_vtech5303));

	SPEAKER(config, "mono").front_center();
}

// Spanish machine
ROM_START( pawmoviesp )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "internal.bin", 0x000000, 0x010000, NO_DUMP ) // Unknown CPU type, unknown internal ROM size

	ROM_REGION( 0x200300, "program", 0 )
	ROM_LOAD( "vtech_paw_patrol_5303_25vq16a.u5", 0x000000, 0x200300, CRC(fee8abd7) SHA1(4ea120246fb4a7efc699e0295864beba4e3317cc) )
ROM_END

} // anonymous namespace


CONS( 2022, pawmoviesp, 0, 0, vtech5303, vtech5303, vtech5303_state, empty_init, "VTech", "Paw Patrol: The Movie Learning Tablet (Spanish)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

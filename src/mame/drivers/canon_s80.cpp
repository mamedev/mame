// license:GPL-2.0+
// copyright-holders:FelipeSanches
/*
 * canon_s80.c
 *
 *    CANON S-80 electronic typewriter
 *
 * skeleton driver by:
 *    Felipe Correa da Silva Sanches <juca@members.fsf.org>
 *
 * known issues:
 *  - memory-map is uncertain
 *  - maincpu clock is guessed
 *  - still lacks description of the keyboard inputs
 *  - as well as a "paper" device to plot the output of the dot matrix print head
 */

#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

class canons80_state : public driver_device
{
public:
	canons80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void canons80(machine_config &config);
	void init_canons80();

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	void canons80_map(address_map &map);
};


HD44780_PIXEL_UPDATE(canons80_state::pixel_update)
{
	if (pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void canons80_state::canons80_map(address_map &map)
{
	map(0x0000, 0x001f).m("maincpu", FUNC(hd6301x0_cpu_device::m6801_io));
	map(0x0040, 0x00ff).ram();
	map(0x0100, 0x07ff).ram();
	map(0x2000, 0x2001).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x4000, 0x7fff).rom().region("external", 0x4000);
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void canons80_state::canons80(machine_config &config)
{
	/* basic machine hardware */
	hd6301x0_cpu_device &maincpu(HD6301X0(config, "maincpu", 5000000)); /* hd63a01xop 5 MHz guessed: TODO: check on PCB */
	maincpu.set_addrmap(AS_PROGRAM, &canons80_state::canons80_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea(0, 16*6-1, 0, 16-1);
	screen.set_palette("palette");

	hd44780_device &hd44780(HD44780(config, "lcdc"));
	hd44780.set_lcd_size(2, 16);
	hd44780.set_pixel_update_cb(FUNC(canons80_state::pixel_update));

	PALETTE(config, "palette").set_entries(2);
}

void canons80_state::init_canons80()
{
}

ROM_START( canons80 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "hd63a1x0p.bin", 0x0000, 0x1000, NO_DUMP )
	ROM_FILL( 0xffe, 1, 0x40 )
	ROM_FILL( 0xfff, 1, 0x00 )

	ROM_REGION( 0x8000, "external", 0 )
	ROM_LOAD( "canon_8735kx_nh4-0029_064.ic6", 0x0000, 0x8000, CRC(b6cd2ff7) SHA1(e47a136300c826e480fac1be7fc090523078a2a6) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT  CLASS           INIT           COMPANY  FULLNAME                            FLAGS */
COMP( 1988, canons80, 0,      0,      canons80, 0,     canons80_state, init_canons80, "Canon", "Canon S-80 electronic typewriter", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

// license:BSD-3-Clause
// copyright-holders:
/*
    Skeleton driver for gambling (?) games running on hardware manufactured by 'Minivideo'.

    Probably manufactured in Italy since PCBs' solder sides are marked LS, which stands for 'lato
    saldature' (solder side in Italian).

    All dumps are missing the HD6473258P10 internal ROM. This chip is also used in Tecmo's V Goal Soccer as a MCU.

    Devices:
    1x  HD6473258P10 at u14 - 16-bit Single-Chip Microcomputer with undumped internal ROM (H8/325)
    1x oscillator 20.000MHz at xt1
    1x oscillator 8.000MHz at xt2

    ROMs
    3x TMS27C040

    RAMs
    1x M48Z08-100PC1 at u37

    Others
    1x 28x2 non-JAMMA edge connector + JAMMA adapter
    1x trimmer (volume)(R34)
    1x 2 legs jumpers (JP3)
    2x 3 legs jumpers (JP1,JP2)
*/

#include "emu.h"
#include "emupal.h"
#include "cpu/h8/h83002.h"
#include "screen.h"


namespace {

class minivideo_state : public driver_device
{
public:
	minivideo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void minivideo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void minivideo_state::video_start()
{
}

uint32_t minivideo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static INPUT_PORTS_START( minivideo )
INPUT_PORTS_END

static const gfx_layout tiles_layout = // wrong
{
	8, 8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "gfx", 0, tiles_layout, 0, 1 )  // wrong
GFXDECODE_END

void minivideo_state::machine_start()
{
}

void minivideo_state::machine_reset()
{
}

void minivideo_state::minivideo(machine_config &config)
{
	H83002(config, m_maincpu, 20_MHz_XTAL);  // TODO: correct CPU type, should be HD6473258P10 (H8/325); unknown divider

	// all wrong
	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(minivideo_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	// sound hw?
}

/***************************************************************************

  Game drivers

***************************************************************************/

/*
PCB is marked: "MINIVIDEO 1.3" on component side
PCB is marked: "MINIVIDEO 1.3" and "LS" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
PCB is labelled: "LF1.3" on component side
*/
ROM_START( fiches )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hd6473258p10-lf1.3.u14", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx", 0 ) // all 27C040
	ROM_LOAD( "lf0.u13", 0x000000, 0x80000, CRC(35b68444) SHA1(e8270cf7da224a98407c8951a40a31a100593876) )
	ROM_LOAD( "lf1.u15", 0x080000, 0x80000, CRC(2fcca8af) SHA1(c2c10d20001897d9ae2f7e4822ec25770ad2ceba) )
	ROM_LOAD( "lf2.u16", 0x100000, 0x80000, CRC(1ceefe34) SHA1(d6af04c8f6369ee634eb178bc9f117ef7419ac94) )
ROM_END

/*
PCB is marked: "MINIVIDEO 1.2" on component side
PCB is marked: "MINIVIDEO 1.2" and "LS" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")
*/
ROM_START( fiches12 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD( "hd6473258p10-lf1.2.u14", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx", 0 ) // all 27C4001
	ROM_LOAD( "lf1.u13", 0x000000, 0x80000, CRC(245d7351) SHA1(b32d6ff366d14b995330eae8aab383420dbde6bd) )
	ROM_LOAD( "lf2.u15", 0x080000, 0x80000, CRC(4194751d) SHA1(a774000a39a87207212da3ef6acc96443d152315) )
	ROM_LOAD( "lf3.u16", 0x100000, 0x80000, CRC(539e3ae3) SHA1(e23d7e2c914682688dae60f3485d13e822af6736) )
ROM_END

} // anonymous namespace


GAME( 1995?, fiches,        0, minivideo, minivideo, minivideo_state, empty_init, ROT0, "Minivideo", "Les Fiches (ver 1.3)", MACHINE_IS_SKELETON )
GAME( 1995?, fiches12, fiches, minivideo, minivideo, minivideo_state, empty_init, ROT0, "Minivideo", "Les Fiches (ver 1.2)", MACHINE_IS_SKELETON )

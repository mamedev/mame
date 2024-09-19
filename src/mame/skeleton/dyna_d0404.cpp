// license:BSD-3-Clause
// copyright-holders:

/*
Dyna D0404 PCB

This PCB is newer hardware than the one is misc/cb2001.cpp, possibly the next one.
It seems Dyna got rid of some legacy features like physical DIPs, color PROMs and AY sound.
While the CPU is sanded off, ROM seems to contain valid ARM code.
Currently unknown what produces the sound.

The main components are:
208-pin custom (sanded off). MCU/CPU?
176-pin custom (sanded off). GFX?
4x AMIC LP61 1024S-12 SRAM or equivalent (for CPU?)
2x IDT71024 SRAM or equivalent (for GFX?)
2x A625308AM-70S SRAM or equivalent (for GFX?)
24.000 MHz XTAL
2x LH28F800SGE-L70 or equivalent flash ROMs
2x GAL16V8D
Oki M62X42B RTC
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/msm6242.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class dyna_d0404_state : public driver_device
{
public:
	dyna_d0404_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void dyna_d0404(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t dyna_d0404_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void dyna_d0404_state::video_start()
{
}


void dyna_d0404_state::program_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom();
}


static INPUT_PORTS_START( cm2005 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static GFXDECODE_START( gfx_dyna_d0404 )
	// TODO
GFXDECODE_END


void dyna_d0404_state::dyna_d0404(machine_config &config)
{
	ARM7(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dyna_d0404_state::program_map);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(dyna_d0404_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_dyna_d0404);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();
	// TODO: identify
}


ROM_START( cm2005 ) // DYNA CM2005 VER. 1.10U at 0xc0000 in ROM
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "a29800uv.11b", 0x000000, 0x100000, CRC(b86c6953) SHA1(73c78a2529abad6aa61fda8285c172bdb6c380cd) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "a29800uv.12b", 0x000000, 0x100000, CRC(c1ae3e9a) SHA1(6cf883b586e074f7517aef4db1c7b006d0ed8df6) )

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )
ROM_END

ROM_START( cm2005a ) // DYNA CM2005 VER. 1.02U at 0xc0000 in ROM
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "lh28f800.11b", 0x000000, 0x100000, CRC(f567782c) SHA1(6138d3170dc4505e1ab2444f26e76e56d5393444) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "lh28f800.12b", 0x000000, 0x100000, CRC(c1ae3e9a) SHA1(6cf883b586e074f7517aef4db1c7b006d0ed8df6) )

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )
ROM_END

ROM_START( cm2005b ) // DYNA CM2005 VER. 0.14H at 0xc0000 in ROM
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "lh28f800.11b", 0x000000, 0x100000, CRC(1af9c956) SHA1(77daffbb8279098218e8401260363af9de4b1d7f) ) // SLDH

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "lh28f800.12b", 0x000000, 0x100000, CRC(74163225) SHA1(14853e347e4e439166e218cd2713e372c871a984) ) // SLDH

	ROM_REGION( 0x80000, "plds", 0 ) // read as 27C020, need reduction
	ROM_LOAD( "gal16v8.10a", 0x00000, 0x40000, CRC(a9f2655a) SHA1(07d9b767b5929bbeac917697ac618c293206bf72) )
	ROM_LOAD( "gal16v8.10b", 0x40000, 0x40000, CRC(d8320940) SHA1(a65973fefaff03696cdbb60a02bfb5e352254951) )
ROM_END

} // anonymous namespace


GAME( 2005, cm2005,  0,      dyna_d0404, cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 1.10U)", MACHINE_IS_SKELETON )
GAME( 2005, cm2005a, cm2005, dyna_d0404, cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 1.02U)", MACHINE_IS_SKELETON )
GAME( 2005, cm2005b, cm2005, dyna_d0404, cm2005, dyna_d0404_state, empty_init, ROT0, "Dyna",  "Cherry Master 2005 (Ver. 0.14H)", MACHINE_IS_SKELETON )

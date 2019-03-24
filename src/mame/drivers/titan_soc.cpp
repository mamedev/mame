// license:BSD-3-Clause
// copyright-holders:David Haywood
/************************************************************************

    Titan 1.0C (System on a Chip - ARM based processor)

    used by

    Atari / Colecovision  / Intellivision Flashback (not dumped)
    TecToy Mega Drive 4


    Notes:

    It is possible to connect a debug terminal
    Has a USB port for user to plug in a flash drive etc.
    4MB RAM

    Emulators run on the ARM, games don't use some modes 100% correctly compared to original
    hardware, only correct for the included emulator.  Some games are not emulation based.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "emupal.h"
#include "screen.h"

class titan_soc_state : public driver_device
{
public:
	titan_soc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_maincpu(*this, "maincpu")
	{ }

	void titan_soc(machine_config &config);

	void init_titan_soc();

private:
	required_shared_ptr<uint32_t> m_mainram;
	required_device<cpu_device> m_maincpu;

	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_titan_soc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void map(address_map &map);
};



void titan_soc_state::map(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram().share("mainram");
}

static INPUT_PORTS_START( titan_soc )

INPUT_PORTS_END


void titan_soc_state::video_start()
{
}

uint32_t titan_soc_state::screen_update_titan_soc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void titan_soc_state::machine_reset()
{
}

void titan_soc_state::titan_soc(machine_config &config)
{
	/* basic machine hardware */
	ARM920T(config, m_maincpu, 200000000); // type + clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &titan_soc_state::map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(320, 256);
	screen.set_visarea(0, 320-1, 0, 256-1);
	screen.set_screen_update(FUNC(titan_soc_state::screen_update_titan_soc));

	PALETTE(config, "palette").set_entries(256);
}



ROM_START( megadri4 )
	ROM_REGION( 0x80000, "serial", 0 ) // this was only dumped from one of the PCBS, not sure which one, might not be correct for both
	ROM_LOAD( "25l4005a - rom de boot megadrive4.bin",     0x000000, 0x80000, CRC(1b5c3c31) SHA1(97f301ae441ca23d3c52a901319e375654920867) )

	ROM_REGION( 0x08400000, "flash", 0 )
	ROM_LOAD( "ic9 megadrive4 titan.bin",     0x000000, 0x08400000, CRC(ed92b81a) SHA1(a3d51a2febf670820d6df009660b96ff6407f475) )
ROM_END

ROM_START( megadri4a )
	ROM_REGION( 0x80000, "serial", 0 ) // this was only dumped from one of the PCBS, not sure which one, might not be correct for both
	ROM_LOAD( "25l4005a - rom de boot megadrive4.bin",     0x000000, 0x80000, CRC(1b5c3c31) SHA1(97f301ae441ca23d3c52a901319e375654920867) )

	ROM_REGION( 0x08400000, "flash", 0 )
	ROM_LOAD( "ic9 megadrive4 titan segunda placa.bin",     0x000000, 0x08400000, CRC(4b423898) SHA1(293127d2f6169717a7fbfcf18f13e4b1735236f7) )
ROM_END



void titan_soc_state::init_titan_soc()
{
	// can either run directly from serial ROM, or copies it to RAM on startup
	memcpy(m_mainram, memregion("serial")->base(), 0x80000);
}

CONS( 2009, megadri4,  0,        0, titan_soc, titan_soc, titan_soc_state, init_titan_soc, "Tectoy (licensed from Sega)", "Mega Drive 4 / Guitar Idol (set 1)",      MACHINE_IS_SKELETON )
CONS( 2009, megadri4a, megadri4, 0, titan_soc, titan_soc, titan_soc_state, init_titan_soc, "Tectoy (licensed from Sega)", "Mega Drive 4 / Guitar Idol (set 2)",      MACHINE_IS_SKELETON )

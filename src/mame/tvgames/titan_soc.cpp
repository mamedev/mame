// license:BSD-3-Clause
// copyright-holders:David Haywood
/************************************************************************

    Titan 1.0C (System on a Chip - ARM based processor)

    used by

    Atari Flashback (not dumped)
    Colecovision Flashback
    Intellivision Flashback (not dumped)
    TecToy Mega Drive 4


    Notes:

    It is possible to connect a debug terminal
    Has a USB port for user to plug in a flash drive etc.
    4MB RAM

    Emulators run on the ARM, games don't use some modes 100% correctly compared to original
    hardware, only correct for the included emulator.  Some games are not emulation based.


    Colecovision Flashback, AtGames, 2014
    Hardware info by Guru
    ---------------------

    This is a plug & play mini console made in China that looks a bit like the real Colecovision
    console which has 60 built-in Colecovision games. There is another version with 61 games included.
    In the dumped ROM, plain text shows what appears to be an OS called 'Titan' and the Colecovision
    ROMs appear to be the same files dumped for emulators (.cv) then gzipped so the ROM files are
    compressed *.cv.gz
    The ROM is not full and has some empty space at 0x1a56e0 to the end (approx. 370kB).
    There is a fixed A/V cable attached with 2 RCA plugs for composite video and mono audio.
    Power input is 5VDC 500mA via a 5.5mm barrel jack. Regulators on the PCB make 3.3V and 1.8V.
    On top of the console are 2 buttons for reset (when in-game this jumps back to the menu)
    and power (toggle on/off).
    The main CPU/SOC is an epoxy-blob and is likely one of the very common ARM-based SOCs used in other
    cheap Chinese mini plug & play or x-in-1 retro-gaming consoles.

    PCB Layout
    ----------

    IN460_MAIN_V2.0_20140510
    |--------------------------------|
    |VA  1.8V                  ROM.U4|
    |3.3V              M12L16161A    |
    |G5R       BLOB                  |
    |   27MHz                        |
    |PAD1                        PAD2|
    |--------------------------------|
    Notes:
          M12L16161A - Elite Semiconductor Memory Technology Inc. M12L16161A 512kB x16bit x 2banks 3.3V Synchronous DRAM
                BLOB - Epoxy blob 1.8V System-On-a-Chip. This is likely one of the common Arm-based Chinese SOCs used in
                       many other mini-consoles. Clock input is 27.000MHz
              PAD1/2 - DB9 connectors for Colecovision look-alike joysticks
              ROM.U4 - SST SST39VF1602 1Mb x16bit CMOS Multi-Purpose 3.3V TSOP48 Flash ROM
                  VA - 3 Wires for ground, composite video and audio outputs
                 G5R - 3 Wires (GND, 5V, Reset) connecting to a small PCB containing DC Jack, power LED, ON/OFF toggle
                       switch and a reset momentary push button
*/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "emupal.h"
#include "screen.h"


namespace {

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

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint32_t> m_mainram;
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void map(address_map &map) ATTR_COLD;
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

uint32_t titan_soc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
	screen.set_screen_update(FUNC(titan_soc_state::screen_update));

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

ROM_START( colecofl )
	ROM_REGION( 0x200000, "flash", 0 )
	ROM_LOAD( "colecovision_flashback_sst39vf1602c.u4", 0x000000, 0x200000, CRC(f78849a2) SHA1(d687b19dabcb404ae4787c798ae4cea69fa7acc7) )
ROM_END


void titan_soc_state::init_titan_soc()
{
	// can either run directly from serial ROM, or copies it to RAM on startup
	memcpy(m_mainram, memregion("serial")->base(), 0x80000);
}

} // anonymous namespace


CONS( 2009, megadri4,  0,        0, titan_soc, titan_soc, titan_soc_state, init_titan_soc, "Tectoy (licensed from Sega)", "Mega Drive 4 / Guitar Idol (set 1)",      MACHINE_IS_SKELETON )
CONS( 2009, megadri4a, megadri4, 0, titan_soc, titan_soc, titan_soc_state, init_titan_soc, "Tectoy (licensed from Sega)", "Mega Drive 4 / Guitar Idol (set 2)",      MACHINE_IS_SKELETON )
CONS( 2004, colecofl,  0,        0, titan_soc, titan_soc, titan_soc_state, empty_init,     "AtGames",                     "Colecovision Flashback",                  MACHINE_IS_SKELETON )

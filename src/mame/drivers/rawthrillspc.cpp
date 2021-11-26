// license:BSD-3-Clause
// copyright-holders:
/*
    Skeleton driver for Raw Thrills PC-based games.
    Common base configuration:
    - Dell Optiplex 740 (Athlon 64 X2, 2GB DDR3 RAM).
      * Other supported setups are:
        · Dell Optiplex 580.
        · Dell Optiplex 380.
        · Dell Optiplex 390.
        · Dell Optiplex 580.
        · Microtel w/ASRock N68C-GS FX AM3+ motherboard.
    - Video GeForce GT730.
      * Other supported setups are:
        · Nvidia 8400GS (256MB+).
        · Nvidia 7300GS.
    -Custom I/O boards (outside the PC, depending on each game).
    -Security dongle (HASP, USB or parallel port).
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class rawthrillspc_state : public driver_device
{
public:
	rawthrillspc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void rawthrillspc(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void rawthrillspc_map(address_map &map);
};

void rawthrillspc_state::video_start()
{
}

uint32_t rawthrillspc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void rawthrillspc_state::rawthrillspc_map(address_map &map)
{
}

static INPUT_PORTS_START( rawthrillspc )
INPUT_PORTS_END


void rawthrillspc_state::machine_start()
{
}

void rawthrillspc_state::machine_reset()
{
}

void rawthrillspc_state::rawthrillspc(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 120000000); // Actually an Athlon 64 X2
	m_maincpu->set_addrmap(AS_PROGRAM, &rawthrillspc_state::rawthrillspc_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(800, 600); // Guess
	screen.set_visarea(0, 800-1, 0, 600-1);
	screen.set_screen_update(FUNC(rawthrillspc_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

#define OPTIPLEX740_BIOS \
	ROM_REGION( 0x20000, "bios", 0 ) \
	ROM_SYSTEM_BIOS( 0, "122", "v1.2.2" ) \
	ROMX_LOAD( "1.2.2_4m.bin", 0x00000, 0x20000, CRC(43d5b4c8) SHA1(6307050961da5d647ca2fa787fd67c5ac9c690c9), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "104", "v1.0.3" ) \
	ROMX_LOAD( "1.0.4_4m.bin", 0x00000, 0x20000, CRC(73f0420b) SHA1(4821d21d2c75084062cb1047eb08b1b3ab2424e1), ROM_BIOS(1) )

ROM_START( guitarheroac )
	OPTIPLEX740_BIOS

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "slax105", 0, NO_DUMP )

	// Recovery DVD
	DISK_REGION( "recovery105" )
	DISK_IMAGE_READONLY( "slax_restore_dvd_ver.1.0.5", 0, SHA1(c063c0032bf88e4ec7a3b973323e9c84a231079a) )
ROM_END

/*
 Two I/O boards on "The Fast and The Furious":
   1. With Xilinx XC95144XL (labeled "FAST & FURIOUS U4 REV 1.0 (c)2004 RightHand Tech, Inc"),
       ST72F63BK4M1 (labeled "U6 FAST&FURIOUS Release 3 3311h (c)2004 RightHand Tech, Inc") and a bank of 8 dipswitches.
   2- With Xilinx XC9536XL (labeled "r1.0 (c)2004 RightHand Tech, Inc")
 Parallel port HASP4 1.5 dongle (MCU Marvin2)
*/
ROM_START( fnf )
	OPTIPLEX740_BIOS

	DISK_REGION( "ide:0:hdd:image" )
	/* Clean image created from the recovery CDs on the original machine.
	   After installing the software from the discs, the PC reboots several times for configurating
	   the hardware devices and peripherals, and then asks for controllers calibration.
	   The image is just up to this point, before performing any calibration. On the first boot from
	   this image, you'll be asked for the calibration, and after it, the game is ready for playing. */
	DISK_IMAGE( "faf306", 0, SHA1(2aefe396a79e3328f58ae5e4ccda0041af1b4a1a) )

	// Two recovery CDs, you need both for a full restore

	DISK_REGION( "recovery306d1" )
	DISK_IMAGE_READONLY( "faf3.06d1", 0, SHA1(681ab1258349e5ceb690606e6697e5b957016446) )

	DISK_REGION( "recovery306d2" )
	DISK_IMAGE_READONLY( "faf3.06d2", 0, SHA1(183664482f6665adffc74d69e28338da740443c5) )
ROM_END

} // Anonymous namespace

GAME(2014, fnf,          0, rawthrillspc, rawthrillspc, rawthrillspc_state, empty_init, ROT0, "Raw Thrills",                                "The Fast And The Furious (v3.06)", MACHINE_IS_SKELETON)
GAME(2008, guitarheroac, 0, rawthrillspc, rawthrillspc, rawthrillspc_state, empty_init, ROT0, "Raw Thrills (Activision / Konami licensed)", "Guitar Hero Arcade (v1.0.5)",      MACHINE_IS_SKELETON)

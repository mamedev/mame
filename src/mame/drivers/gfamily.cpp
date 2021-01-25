// license:BSD-3-Clause
// copyright-holders:
/*
Games Family P4-4P (Pentium 4 - 4 Players)

PC with Chinese Windows 2000 Pro and several emulators, including:
 -MAME v0.78 (Dec 30 2003)
 -MAME v0.96u3 (May 25 2005)
 -MAME v0.106 (May 16 2006)
 -Nebula 2.1.5
 -FB Alpha v0.2.94.98
 -ZiNc 1.1


PC motherboard plus an additional PCB for JAMMA, inputs and basic config.

PC board:
CPU: Intel Celeron 1.7GHz / 128kb / 400MHz SL6SC
RAM: 256MB-DDR400
PCB: 04/082006-SiS-651-6A6IXRMAC-00, Realtec ALC655 audio, Realtec RTL8201BL Ethernet (25.000 MHz xtal), Winbond W83194BG-648 (14.31818 MHz xtal)
BIOS: 686 AMIBIOS (c) 2006 AY36 8897

I/O board:
AT89C2051 + Microchip CF745 + 2 x Microchip PIC12F508 + Altera Max CPLD

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class gfamily_state : public driver_device
{
public:
	gfamily_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void gfamily(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void gfamily_map(address_map &map);
};

void gfamily_state::video_start()
{
}

uint32_t gfamily_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gfamily_state::gfamily_map(address_map &map)
{
}

static INPUT_PORTS_START( gfamily )
INPUT_PORTS_END


void gfamily_state::machine_start()
{
}

void gfamily_state::machine_reset()
{
}

void gfamily_state::gfamily(machine_config &config)
{
	// Basic machine hardware
	PENTIUM4(config, m_maincpu, 1'700'000'000); // Actually an Intel Celeron SL6SC 1.7GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &gfamily_state::gfamily_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(gfamily_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( gmfamily )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("686_amibios_ay36_8897.bin", 0x00000, 0x80000, CRC(e04c5750) SHA1(240ca6b270bdebf129e4ce43e79275aa067b6ada) )

	// PICs and MCUs from the I/O board, all of them protected
	ROM_REGION(0x10000, "unsorted", 0)
	ROM_LOAD("at89c2051.u3", 0x0000, 0x4000, NO_DUMP ) // 2 Kbytes internal ROM
	ROM_LOAD("cf745.u2",     0x4000, 0x2000, NO_DUMP ) // 1 Kbytes internal ROM
	ROM_LOAD("pic12f508.u0", 0x6000, 0x2000, NO_DUMP ) // 1 Kbytes internal ROM
	ROM_LOAD("pic12f508.u6", 0x8000, 0x2000, NO_DUMP ) // 1 Kbytes internal ROM

	DISK_REGION( "ide:0:hdd:image" ) // From a Norton Ghost recovery image
	DISK_IMAGE( "gamesfamily_1.1", 0, SHA1(627584c5fceaa2649a4fbc9670fbf8aa036f82be) )
ROM_END

} // Anonymous namespace


GAME( 200?, gmfamily, 0, gfamily, gfamily, gfamily_state, empty_init, ROT0, "bootleg", "Games Family", MACHINE_IS_SKELETON )

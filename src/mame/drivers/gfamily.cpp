// license:BSD-3-Clause
// copyright-holders:
/*
Games Family P4-4P (Pentium 4 - 4 Players)

PC with Chinese Windows 2000 Pro and several emulators, including (programs may change on other revisions):
 -MAME v0.78 (Dec 30 2003)
 -MAME v0.96u3 (May 25 2005)
 -MAME v0.106 (May 16 2006)
 -Nebula 2.1.5
 -FB Alpha v0.2.94.98
 -ZiNc 1.1

PC motherboard plus an additional PCB for JAMMA, inputs and basic config (and protection) with:
 Atmel AT89C2051 (near a 4 dipswitches bank and a 6.000 MHz xtal)
 Microchip CF745 (near another 4 dipswitches bank and a 4.000 MHz xtal)
 2 x Microchip PIC12F508
 Altera Max EPM7128SQC100-10 CPLD

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
	PENTIUM4(config, m_maincpu, 1'700'000'000); // Actually an Intel Celeron SL6SC 1.7GHz (with the config found with the default BIOS)
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

	/* Different PC motherboards with different configurations.
	   By now, we're throwing all known BIOSes here. */
	ROM_REGION(0x80000, "bios", 0)

	/* CPU: Intel Celeron 1.7GHz / 128kb / 400MHz SL6SC
	   RAM: 256MB-DDR400
	   PCB: Realtec ALC655 audio, Realtec RTL8201BL Ethernet (25.000 MHz xtal), Winbond W83194BG-648 (14.31818 MHz xtal) */
	ROM_SYSTEM_BIOS(0, "ay36_8897", "04/082006-SiS-651-6A6IXRMAC-00")
	ROMX_LOAD("686_amibios_ay36_8897.bin", 0x00000, 0x80000, CRC(e04c5750) SHA1(240ca6b270bdebf129e4ce43e79275aa067b6ada), ROM_BIOS(0))

	/* CPU: Pentium 4 2.40GHz/512/533 SL6DV
	   RAM: 512MB
	   PCB: ECS (Elitegroup) 651-M v2.4. Seems like a later low-cost version of the consumer v2.0 motherboard. */
	ROM_SYSTEM_BIOS(1, "sy_sis651", "05/13/2005-SiS-651-6A6IXE19C-00")
	ROMX_LOAD("award_i6a6ixe19.bin",       0x00000, 0x40000, CRC(95fa392c) SHA1(40f557339649c47e6c3d941670604e0559edf8db), ROM_BIOS(1)) // Winbond W49F002UP12N

	// PICs and MCUs from the I/O board, all of them protected
	ROM_REGION(0x10000, "unsorted", 0)
	ROM_LOAD("at89c2051.u3", 0x0000, 0x4000, NO_DUMP) // 2 Kbytes internal ROM
	ROM_LOAD("cf745.u2",     0x4000, 0x2000, NO_DUMP) // 1 Kbytes internal ROM
	ROM_LOAD("pic12f508.u0", 0x6000, 0x2000, NO_DUMP) // 1 Kbytes internal ROM
	ROM_LOAD("pic12f508.u6", 0x8000, 0x2000, NO_DUMP) // 1 Kbytes internal ROM

	DISK_REGION( "ide:0:hdd:image" ) // From a Norton Ghost recovery image
	DISK_IMAGE( "gamesfamily_1.1", 0, SHA1(cd6b8d19f430c510a95a838d7b8c578f89ccf834) )
ROM_END

} // Anonymous namespace


GAME( 200?, gmfamily, 0, gfamily, gfamily, gfamily_state, empty_init, ROT0, "bootleg", "Games Family", MACHINE_IS_SKELETON )

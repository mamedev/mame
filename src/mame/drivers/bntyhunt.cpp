// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* Bounty Hunter

 PC hardware.. no dumps of the bios roms are currently available

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "emupal.h"
#include "screen.h"


class bntyhunt_state : public driver_device
{
public:
	bntyhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bntyhunt(machine_config &config);
	void bntyhunt_map(address_map &map);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void video_start() override;
};


void bntyhunt_state::video_start()
{
}

uint32_t bntyhunt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void bntyhunt_state::bntyhunt_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom();
}

static INPUT_PORTS_START( bntyhunt )
INPUT_PORTS_END


void bntyhunt_state::bntyhunt(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM(config, m_maincpu, 200000000); /* Probably a Pentium or higher .. ?? Mhz*/
	m_maincpu->set_addrmap(AS_PROGRAM, &bntyhunt_state::bntyhunt_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(bntyhunt_state::screen_update));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100);
}


ROM_START(bntyhunt)
	ROM_REGION32_LE(0x20000, "maincpu", 0)  /* motherboard bios */
	ROM_LOAD("bntyhunt.pcbios", 0x000000, 0x10000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "bntyhunt", 0, SHA1(e50937d14d5c6adfb5e0012db5a7df090eebc2e1) )
ROM_END


GAME( 200?, bntyhunt, 0, bntyhunt, bntyhunt, bntyhunt_state, empty_init, ROT0, "GCTech Co., LTD", "Bounty Hunter (GCTech Co., LTD)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

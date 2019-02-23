// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista
/*
    Uniana PC hardware (shared by other Korean companies)

    Intel Celeron CPU, 1.70GHz
    MSI MS-6566E VER: 2 motherboard
    Samsung 256MB DDR PC2100 CL2.5 RAM
    Inside TNC 128MB(?) VGA Graphics Video Card E-G012-01-1247
    Dynamic 4281 Rev. A sound card by Jin Information Technology with Crystal CS4281 sound chip
    DM Storage DM2560V00 IDE flash storage device
    IO board

    Game Box's Dream Hunting has the same hardware minus the sound card.
    It also sports a gun IO board. Two version exist (CGA and VGA). Currently only VGA is dumped.

    Other games believed to run on this hardware (* denotes a copy is in MAME friendly hands):
    Frenzy Express(Uniana, 2001) *
    S.A.P.T. (Uniana, 2004)

    Possibly on other hardware (Crystal System ?)
    Dojeon! OX Survival (Uniana, 2002)
    Dojeon! OX Survival Plus (Uniana, 2002)
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

class unianapc_state : public driver_device
{
public:
	unianapc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void unianapc(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void unianapc_map(address_map &map);
};

void unianapc_state::video_start()
{
}

uint32_t unianapc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void unianapc_state::unianapc_map(address_map &map)
{
}

static INPUT_PORTS_START( unianapc )
INPUT_PORTS_END


void unianapc_state::machine_start()
{
}

void unianapc_state::machine_reset()
{
}

void unianapc_state::unianapc(machine_config &config)
{
	/* basic machine hardware */
	PENTIUM3(config, m_maincpu, 100000000); // actually a Celeron at 1.70 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &unianapc_state::unianapc_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(unianapc_state::screen_update));
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( dhunting )
	ROM_REGION(0x20000, "bios", 0) \
	ROM_LOAD("mbbios", 0x10000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "vbios", 0 )   // video card BIOS
	ROM_LOAD( "videobios", 0x000000, 0x00d000, NO_DUMP )

	ROM_REGION( 0x2000, "gunio", 0 )   // IO card (VGA version)
	ROM_LOAD( "u2.at89c52", 0x0000, 0x2000, CRC(afb0e1c7) SHA1(2f621be62f935eafa9ff3c14de2096119132a973) )

	DISK_REGION( "ide:0:hdd:image" ) // DM Storage DM2560V00 IDE flash storage
	DISK_IMAGE( "dream hunting", 0, SHA1(3515c0617c52c7e8b7e5dba8de22e363cce00e10) )
ROM_END

ROM_START( hogwild )
	ROM_REGION(0x20000, "bios", 0) \
	ROM_LOAD("mbbios", 0x10000, 0x10000, NO_DUMP )

	ROM_REGION( 0x10000, "vbios", 0 )   // video card BIOS
	ROM_LOAD( "videobios", 0x000000, 0x00d000, NO_DUMP )

	DISK_REGION( "ide:0:hdd:image" ) // DM Storage DM2560V00 IDE flash storage
	DISK_IMAGE( "hog wild", 0, SHA1(f05b7f64830d995db2e2a2f7f95ae0100de5dab1) )
ROM_END

GAME( 2002, dhunting,  0,   unianapc, unianapc, unianapc_state, empty_init, ROT0, "Game Box Entertainment", "Dream Hunting (US)",  MACHINE_IS_SKELETON ) // Ver 1007?
GAME( 2003, hogwild,   0,   unianapc, unianapc, unianapc_state, empty_init, ROT0, "Uniana",                 "Hog Wild (US)",       MACHINE_IS_SKELETON ) // Ver.00.26.b?

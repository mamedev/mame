// license:BSD-3-Clause
// copyright-holders:
/*
Neo Mania:
 The Portuguese (Vila Nova de Gaia) company "Hyper M.A.R." created this machine on 2002 with 40 games,
 and updated it on 2003 increasing the number of games up to 48. There was a latest newer version
 where they added "Strikers 1945" and "Prehistoric Isle 2", reaching 50 games.
 There are Spanish and Portuguese localizations.
 The hardware is a PC with Windows 98 (exact hardware not specified) and a Neo·Geo emulator, with a
 small PCB for converting VGA + Parallel port (inputs) + sound (with volume knob) to JAMMA (named
 "NEO MANIA ADAPTER BOARD").
The "NEO MANIA ADAPTER BOARD" contains:
   3 x Blocks of jumpers to enable or disable features:
    JMP1 (two positions) - With or without Coin Dist.
    JMP2 (two positions)  - With or without Audio Amplifier
    JMP3 (three positions) - Unknown function
   2 x Coin acceptors ports
   1 x Bank of 8 dipswitches (unknown function)
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "screen.h"

namespace {

class neomania_state : public driver_device
{
public:
	neomania_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void neomania(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void neomania_map(address_map &map);
};

void neomania_state::video_start()
{
}

uint32_t neomania_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void neomania_state::neomania_map(address_map &map)
{
}

static INPUT_PORTS_START( neomania )
INPUT_PORTS_END


void neomania_state::machine_start()
{
}

void neomania_state::machine_reset()
{
}

void neomania_state::neomania(machine_config &config)
{
	// Basic machine hardware
	PENTIUM3(config, m_maincpu, 600'000'000); // Exact hardware not specified
	m_maincpu->set_addrmap(AS_PROGRAM, &neomania_state::neomania_map);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(neomania_state::screen_update));
}

/***************************************************************************
  Game drivers
***************************************************************************/

ROM_START( neomania )

	// Different PC motherboards with different configurations.
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD("pcbios.bin", 0x00000, 0x80000, NO_DUMP) // MB BIOS

	// Portuguese version with 48 games, from 2003
	DISK_REGION( "ide:0:hdd:image" ) // From a Norton Ghost recovery image
	DISK_IMAGE( "neomania", 0, SHA1(4a865d1ed67901b98b37f94cfdd591fad38b404a) )
ROM_END

} // Anonymous namespace

GAME( 2003, neomania, 0, neomania, neomania, neomania_state, empty_init, ROT0, "bootleg (Hyper M.A.R.)", "Neo Mania (Portugal)", MACHINE_IS_SKELETON )

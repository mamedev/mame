// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*

There are no boot vectors in the rom, we have to force the CPU to execute the code that is at 0xf8000 in the ROM
so it's possible there is a missing internal ROM, or the 6502 core is customized.

As there is no interrupt vector either this may require a derived 6502 type to run.

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "speaker.h"

class radicasi_state : public driver_device
{
public:
	radicasi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
};

void radicasi_state::video_start()
{
}

uint32_t radicasi_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

static ADDRESS_MAP_START( radicasi_map, AS_PROGRAM, 8, radicasi_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_ROM AM_REGION("maincpu", 0xf8000) // very likely wrong, but we need to force the CPU somewhere to boot
ADDRESS_MAP_END

static INPUT_PORTS_START( radicasi )
INPUT_PORTS_END

void radicasi_state::machine_start()
{
}

void radicasi_state::machine_reset()
{
	// force CPU somewhere due to lack of boot vectors
	m_maincpu->set_state_int(M6502_PC, 0x4000);
}

static MACHINE_CONFIG_START( radicasi )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,8000000) // unknown frequency
	MCFG_CPU_PROGRAM_MAP(radicasi_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(radicasi_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

ROM_START( radicasi )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvadersrom.bin", 0x000000, 0x100000, CRC(5ffb2c8f) SHA1(9bde42ec5c65d9584a802de7d7c8b842ebf8cbd8) )
ROM_END

CONS( 200?, radicasi,  0,   0,  radicasi,  radicasi, radicasi_state, 0, "Radica (licensed from Taito)", "Space Invaders (Radica, Arcade Legends TV Game)", MACHINE_IS_SKELETON )

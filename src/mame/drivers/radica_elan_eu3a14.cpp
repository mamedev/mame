// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
	These use a 6502 derived CPU under a glob
	The CPU die is marked 'ELAN EU3A14'

	There is a second glob surrounded by TSOP48 pads
	this contains the ROM

	Known to be on this hardware

	Golden Tee Golf Home Edition (developed by FarSight Studios)

	Maybe on this hardware

	PlayTV Real Swing Golf (also developed by FarSight, looks similar but with different controls)

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"

class radica_eu3a14_state : public driver_device
{
public:
	radica_eu3a14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	
	void radica_eu3a14(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
};


void radica_eu3a14_state::video_start()
{
}

uint32_t radica_eu3a14_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	return 0;
}

static ADDRESS_MAP_START( radica_eu3a14_map, AS_PROGRAM, 8, radica_eu3a14_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM

	AM_RANGE(0x3000, 0x3fff) AM_ROM AM_REGION("maincpu", 0x7000)

	AM_RANGE(0x4800, 0x4bff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0x0000)
ADDRESS_MAP_END


static INPUT_PORTS_START( radica_eu3a14 )
INPUT_PORTS_END

void radica_eu3a14_state::machine_start()
{
}

void radica_eu3a14_state::machine_reset()
{
	// rather be safe
	m_maincpu->set_state_int(M6502_S, 0x1ff);
}


MACHINE_CONFIG_START(radica_eu3a14_state::radica_eu3a14)
	/* basic machine hardware */	
	MCFG_CPU_ADD("maincpu",M6502,XTAL(21'477'272)/2) // marked as 21'477'270
	MCFG_CPU_PROGRAM_MAP(radica_eu3a14_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(radica_eu3a14_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
MACHINE_CONFIG_END

ROM_START( rad_gtg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "goldentee.bin", 0x000000, 0x400000, CRC(41538e08) SHA1(1aef9a2c678e39243eab8d910bb7f9f47bae0aee) )
ROM_END

CONS( 2006, rad_gtg,  0,   0,  radica_eu3a14,  radica_eu3a14, radica_eu3a14_state, 0, "Radica (licensed from Incredible Technologies)", "Golden Tee Golf: Home Edition", MACHINE_IS_SKELETON )

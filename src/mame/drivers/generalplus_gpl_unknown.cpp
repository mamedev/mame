// license:BSD-3-Clause
// copyright-holders:David Haywood

// pcp8718 contains unsp code, but no obvious startup code / vectors, so it's probably booting from another device / bootstrapped
// these contain the same game selection as the games in unk6502_st2xxx.cpp but on updated hardware

// These use SPI ROMs and unSP2.0 instructions, so will be GeneralPlus branded parts, not SunPlus
// possibly the framebuffer based video ones rather than the ones with tile layers

#include "emu.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/unsp/unsp.h"


class pcp8718_state : public driver_device
{
public:
	pcp8718_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
	{ }

	void pcp8718(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<unsp_20_device> m_maincpu;

	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	void map(address_map &map);
};

uint32_t pcp8718_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}




void pcp8718_state::machine_start()
{
}

void pcp8718_state::machine_reset()
{
}

static INPUT_PORTS_START( pcp8718 )
INPUT_PORTS_END

void pcp8718_state::map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
}


void pcp8718_state::pcp8718(machine_config &config)
{

	UNSP_20(config, m_maincpu, 20000000); // unknown CPU, unsp20 based
	m_maincpu->set_addrmap(AS_PROGRAM, &pcp8718_state::map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(pcp8718_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);
}

ROM_START( pcp8718 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "8718_en25f32.bin", 0x000000, 0x400000, CRC(cc138db4) SHA1(379af3d94ae840f52c06416d6cf32e25923af5ae) ) // dump needs verifying, dumper mentioned it was possibly bad (unit failed in process)
ROM_END

ROM_START( pcp8728 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "pcp 8728 788 in 1.bin", 0x000000, 0x400000, CRC(60115f21) SHA1(e15c39f11e442a76fae3823b6d510178f6166926) )
ROM_END

ROM_START( unkunsp )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "fm25q16a.bin", 0x000000, 0x200000, CRC(aeb472ac) SHA1(500c24b725f6d3308ef8cbdf4259f5be556c7c92) )
ROM_END




CONS( 200?, pcp8718,      0,       0,      pcp8718,   pcp8718, pcp8718_state, empty_init, "PCP", "PCP 8718 - 788 in 1", MACHINE_IS_SKELETON ) // "HD 360 degrees rocker palm eyecare console"
CONS( 200?, pcp8728,      0,       0,      pcp8718,   pcp8718, pcp8718_state, empty_init, "PCP", "PCP 8728 - 788 in 1", MACHINE_IS_SKELETON )
CONS( 200?, unkunsp,      0,       0,      pcp8718,   pcp8718, pcp8718_state, empty_init, "<unknown>", "unknown unSP based handheld", MACHINE_IS_SKELETON )

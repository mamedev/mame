// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    SH6578 NES clone hardware
    enhanced NES, different to VT / OneBus systems
*/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "video/ppu2c0x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class nes_sh6578_state : public driver_device
{
public:
	nes_sh6578_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu")
	{ }

	void nes_sh6578(machine_config &config);

	void init_nes_sh6578();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	WRITE8_MEMBER(sprite_dma_w);

private:
	required_device<n2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;

	void nes_sh6578_map(address_map &map);
};


WRITE8_MEMBER(nes_sh6578_state::sprite_dma_w)
{
	int source = (data & 7);
	m_ppu->spriteram_dma(space, source);
}

void nes_sh6578_state::nes_sh6578_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(FUNC(nes_sh6578_state::sprite_dma_w));
	map(0x5000, 0x57ff).ram();
	map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START( nes_sh6578 )
INPUT_PORTS_END

void nes_sh6578_state::video_start()
{
}

void nes_sh6578_state::machine_reset()
{
}


void nes_sh6578_state::machine_start()
{
//  m_nt_ram = std::make_unique<uint8_t[]>(0x1000);
//  m_nt_page[0] = m_nt_ram.get();
//  m_nt_page[1] = m_nt_ram.get() + 0x400;
//  m_nt_page[2] = m_nt_ram.get() + 0x800;
//  m_nt_page[3] = m_nt_ram.get() + 0xc00;

//  m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(*this, FUNC(nes_sh6578_state::nes_sh6578_nt_r)), write8_delegate(*this, FUNC(nes_sh6578_state::nes_sh6578_nt_w)));
//  m_ppu->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, "bank1");
//  membank("bank1")->set_base(memregion("gfx1")->base());
}

void nes_sh6578_state::nes_sh6578(machine_config &config)
{
	/* basic machine hardware */
	N2A03(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_sh6578_state::nes_sh6578_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 262);
	screen.set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	screen.set_screen_update("ppu", FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_cpu_tag("maincpu");
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);
}



void nes_sh6578_state::init_nes_sh6578()
{
}


ROM_START( bandgpad )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "gamepad.bin", 0x00000, 0x100000, CRC(e2fbb532) SHA1(e9170a7739a8355acbf263fe2b1d291951dc07f0) )
ROM_END

ROM_START( ts_handy11 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tvplaypowercontroller.bin", 0x00000, 0x100000, CRC(9c7fe9ff) SHA1(c872e91ca835b66c9dd3b380e8374b51f12bcae0) ) // 29LV008B
ROM_END

ROM_START( cpatrolm )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "citypatrolman.bin", 0x00000, 0x100000, CRC(4b139c67) SHA1(a5b03f472a94ee879f58bbff201b671fbf4f1ea1) )
ROM_END

ROM_START( ablwikid )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "mx29f1610atc.u2", 0x00000, 0x200000, CRC(f16abf79) SHA1(aeccbb40d7fdd451ba8e5cca20464da2cf116461) )
ROM_END


CONS( 1997, bandgpad,  0,  0,  nes_sh6578,    nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Bandai", "Multi Game Player Gamepad", MACHINE_NOT_WORKING )

// possibly newer than 2001
CONS( 2001, ts_handy11,  0,  0,  nes_sh6578,    nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Techno Source", "Handy Boy 11-in-1 (TV Play Power)", MACHINE_NOT_WORKING )

CONS( 200?, cpatrolm,  0,  0,  nes_sh6578,    nes_sh6578, nes_sh6578_state, init_nes_sh6578, "<unknown>", "City Patrolman", MACHINE_NOT_WORKING )

// this may or may not belong here, it appears to have 4bpp gfx, but unless the banks are scrambled, doesn't fit a standard VT scheme
CONS( 200?, ablwikid,   0,        0,  nes_sh6578, nes_sh6578, nes_sh6578_state, init_nes_sh6578, "Advance Bright Ltd.",         "Wikid Joystick", MACHINE_NOT_WORKING ) // or Wik!d Joystick

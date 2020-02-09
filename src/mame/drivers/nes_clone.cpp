// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
    NES clones that don't fit anywhere else / Plug & Play (non-VT)
*/

#include "emu.h"
#include "cpu/m6502/n2a03.h"
#include "video/ppu2c0x.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class nes_clone_state : public driver_device
{
public:
	nes_clone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppu(*this, "ppu")
	{ }

	void nes_clone(machine_config &config);

	void init_nes_clone();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	WRITE8_MEMBER(sprite_dma_w);

private:
	required_device<n2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;

	void nes_clone_map(address_map &map);
};


WRITE8_MEMBER(nes_clone_state::sprite_dma_w)
{
	int source = (data & 7);
	m_ppu->spriteram_dma(space, source);
}

void nes_clone_state::nes_clone_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(FUNC(nes_clone_state::sprite_dma_w));
	map(0x8000, 0xffff).rom();
}

static INPUT_PORTS_START( nes_clone )
INPUT_PORTS_END

void nes_clone_state::video_start()
{
}

void nes_clone_state::machine_reset()
{
}


void nes_clone_state::machine_start()
{
//  m_nt_ram = std::make_unique<uint8_t[]>(0x1000);
//  m_nt_page[0] = m_nt_ram.get();
//  m_nt_page[1] = m_nt_ram.get() + 0x400;
//  m_nt_page[2] = m_nt_ram.get() + 0x800;
//  m_nt_page[3] = m_nt_ram.get() + 0xc00;

//  m_ppu->space(AS_PROGRAM).install_readwrite_handler(0x2000, 0x3eff, read8_delegate(*this, FUNC(nes_clone_state::nes_clone_nt_r)), write8_delegate(*this, FUNC(nes_clone_state::nes_clone_nt_w)));
//  m_ppu->space(AS_PROGRAM).install_read_bank(0x0000, 0x1fff, "bank1");
//  membank("bank1")->set_base(memregion("gfx1")->base());
}

void nes_clone_state::nes_clone(machine_config &config)
{
	/* basic machine hardware */
	N2A03(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_clone_state::nes_clone_map);

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

ROM_START( pjoypj001 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "powerjoy_pj001_lh28f008sc_89a6.bin", 0x00000, 0x100000, CRC(e655e0aa) SHA1(c96d3422e26451c366fee2151fedccb95014cbc7) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "powerjoy_pj001_te28f400ceb_00894471.bin", 0x00000, 0x80000, CRC(edca9b66) SHA1(f2f6d9043f524748282065b2fa0ca323ddd7d008) )
ROM_END

ROM_START( afbm7800 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "atariflashbackmini7800.bin", 0x00000, 0x100000, CRC(da4d9483) SHA1(c04465ff5bd5ca7abf088fe771b8e71c157afb89) )
ROM_END


void nes_clone_state::init_nes_clone()
{
}

CONS( 200?, pjoypj001, 0, 0, nes_clone, nes_clone, nes_clone_state, init_nes_clone, "Trump Grand", "PowerJoy (PJ001, NES based plug & play)", 0 )

// "Flashback Mini 7800 uses normal NES-style hardware, together with a mapper chipset similar to the Waixing kk33xx cartridges (NES 2.0 Mapper 534)"
CONS( 2004, afbm7800,  0,  0,  nes_clone,    nes_clone, nes_clone_state, init_nes_clone, "Atari", "Atari Flashback Mini 7800", MACHINE_NOT_WORKING )

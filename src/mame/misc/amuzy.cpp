// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    amuzy.cpp - Amuzy cartridge arcade/medal system
    Skeleton by R. Belmont

    H8/3007 CPU
    Yamaha YGV625 sprite processor and CRTC
    OKI M9810 sound

    For mmhammer:
    NMI vector is valid but doesn't return (error handler?)
    Timer/counter B0 IRQ vector is valid and runs
    SCI0 and SCI1 (UART) IRQs are valid
    All other vectors are RTE.

************************************************************************/

#include "emu.h"

#include "cpu/h8/h83006.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "sound/okim9810.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class amuzy_state : public driver_device
{
public:
	amuzy_state(const machine_config &mconfig, device_type type, const char *tag);

	void amuzy(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<okim9810_device> m_oki;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void amuzy_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

amuzy_state::amuzy_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_oki(*this, "oki")
{
}

u32 amuzy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(amuzy_state::scanline)
{
}


void amuzy_state::amuzy_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x200000, 0x20ffff).ram();
	map(0x600000, 0x603fff).ram();  // possibly YGV625 display list
}

void amuzy_state::amuzy(machine_config &config)
{
	H83007(config, m_maincpu, 20_MHz_XTAL); // 20 MHz rated part, 20 MHz oscillator module is present
	m_maincpu->set_addrmap(AS_PROGRAM, &amuzy_state::amuzy_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// screen parameters are completely made up
	m_screen->set_refresh_hz(59.62);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(80, 400-1, 16, 240-1);
	m_screen->set_screen_update(FUNC(amuzy_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 256);

	TIMER(config, "scantimer").configure_scanline(FUNC(amuzy_state::scanline), m_screen, 0, 1);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM9810(config, m_oki, XTAL(4'096'000));
	m_oki->add_route(0, "lspeaker", 1.00);
	m_oki->add_route(1, "rspeaker", 1.00);
}

static INPUT_PORTS_START( amuzy )
INPUT_PORTS_END

ROM_START( mmhammer )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(f6aa7880) SHA1(c3dfdc5250875c365c7146b6fe6288d1605d17e5) )

	// YGV625 has a 32 bit data bus to the ROMs, so these are likely word-interleaved
	ROM_REGION(0x400000, "gfx", 0)
	ROM_LOAD32_WORD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(beb65917) SHA1(835a0ceef2fdfee2730d88e04a4a131575048979) )
	ROM_LOAD32_WORD( "s29al016d70tfi01.u3", 0x000002, 0x200000, CRC(cedb6c55) SHA1(c2981b2547468723da6f5416a81b937b293576fc) )
ROM_END

}   // anonymous namespace

COMP( 1994, mmhammer,  0, 0, amuzy, amuzy, amuzy_state, empty_init, "Amuzy", "Mogu Mogu Hammer",  MACHINE_NOT_WORKING )

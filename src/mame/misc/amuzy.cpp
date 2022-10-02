// license:BSD-3-Clause
// copyright-holders:R. Belmont
/******************************************************************************

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

===============================================================================

    YGV625 preliminary pinout

    D15-0 CPU data bus
    A13-1 CPU address bus
    A0[WRH_N] CPU address bus/write pulse input
    CS_N chip select
    RD_N Read pulse input
    WRL_N Light (?) pulse input
    WAIT_N CPU pass wait (tristate)
    READY_N CPU bus ready (tristate)
    INT_N irq (open drain)
    C16_N CPU bus width selection
    LEND_N endian control
    RESET_N reset input

    MD31-0 CG memory data bus
    MA24-1 CG memory address bus
    CE3-0_N CG memory chip enable
    OE3-0_N CG memory output enable
    WEH_N, WEL_N CG memory write enable

******************************************************************************/

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

	u16 status_r(offs_t offset);
	void status_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 m_status[2]{};

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

amuzy_state::amuzy_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_screen(*this, "screen"),
	m_palette(*this, "palette"),
	m_oki(*this, "oki")
{
	std::fill(std::begin(m_status), std::end(m_status), 0);

}

u32 amuzy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(amuzy_state::scanline)
{
}

// video or FIFO status bits
u16 amuzy_state::status_r(offs_t offset)
{
	// Note: if bit 0 doesn't act like a heartbeat then a watchdog reset will eventually occur.
	if (offset == 0)
		return (m_status[0] & 0xfffe) | m_screen->vblank();

	// PC=0xb2ee, FIFO empty?
	m_status[1] ^= 0x20;
	return m_status[1];
}

void amuzy_state::status_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_status[offset]);
}


void amuzy_state::amuzy_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x200000, 0x20ffff).ram();
	//  0x220000- 0x22000d  YGV625 CG memory readback?
	//  0x600000- 0x601fff) YGV625 display list
	map(0x600000, 0x603fff).ram();
	//  0x603c00- 0x603c7f (at least) YGV625 registers
	map(0x603c4c, 0x603c4f).rw(FUNC(amuzy_state::status_r), FUNC(amuzy_state::status_w));
}

void amuzy_state::amuzy(machine_config &config)
{
	H83007(config, m_maincpu, 20_MHz_XTAL); // 20 MHz rated part, 20 MHz oscillator module is present
	m_maincpu->set_addrmap(AS_PROGRAM, &amuzy_state::amuzy_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// screen parameters are completely made up
	m_screen->set_refresh_hz(59.62);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
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

	ROM_REGION(0x400000, "gfx", 0)
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(beb65917) SHA1(835a0ceef2fdfee2730d88e04a4a131575048979) )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x200000, 0x200000, CRC(cedb6c55) SHA1(c2981b2547468723da6f5416a81b937b293576fc) )
ROM_END

ROM_START( docchift )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(b69d97f6) SHA1(8ecb6300d435200cf694f6f0d6a847d60354dbae) )

	ROM_REGION(0x400000, "gfx", 0)
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(044f004b) SHA1(c9f8797fcd5f67831311e4fea2621d7337c74fa2) )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x200000, 0x200000, CRC(fb668dbd) SHA1(49514b0c886578f065e47d9c7a5453e09622ba55) )
ROM_END

ROM_START( amhbattl )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(b24f7bf4) SHA1(254e814c26a1430d6fecc68e07e7ee2cdab77f21) )

	ROM_REGION(0x400000, "gfx", 0)
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(cb1fd823) SHA1(f425a37ca425315f294366298146c3f6547a28c0) )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x200000, 0x200000, CRC(f5bfb1e8) SHA1(e36be311782e4bcbd00a8bc93473f23e5c39c67a) )
ROM_END

}   // anonymous namespace

GAME( 2006, amhbattl,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Acchi Muite Hoi Battle",  MACHINE_NOT_WORKING )
GAME( 2007, docchift,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Docchi Fighter",  MACHINE_NOT_WORKING )
GAME( 2008, mmhammer,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Mogu Mogu Hammer",  MACHINE_NOT_WORKING )

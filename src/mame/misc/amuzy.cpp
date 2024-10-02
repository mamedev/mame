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

===============================================================================

    cart pinout

    +------------------------------------------+
    |                                          |
    |  +------+       +------+      +------+   |
    |  |      |       |      |      |      |   |
    |  |CHIP 1|       |CHIP 2|      |CHIP 3|   |
    |  |      |       |      |      |      |   |
    |  +------+       +------+      +------+   |
    |                                          |
    ++     +-+                        +-+     ++
     |_____| |________________________| |_____|
    A66                                      A01

    +------------------------------------------+
    |                                          |
    |                                          |
    |                                          |
    |                                          |
    |                                          |
    |                                          |
    |                                          |
    ++     +-+                        +-+     ++
     |_____| |________________________| |_____|
    B01                                      B66



      A  |  |  B  |
    ---------------
     GND |01| GND         ---------+
     A08 |02| D00                  |
     A09 |03| D01                  |
     A10 |04| D02                  |
     A11 |05| D03                  |
     A12 |06| D04                  |
     A13 |07| D05                  |
     A14 |08| D06                  |
     A15 |09| D07                  | CHIP U2
     VCC |10| VCC                  |
     A16 |11| A00                  |
     A17 |12| A01                  |
     A18 |13| A02                  |
     A19 |14| A03                  |
     A20 |15| A04                  |
     A21 |16| A05                  |
      CE |17| A06                  |
      OE |18| A07                  |
     GND |19| GND                  |
    WE_U2|20| A19         ---------+
    RE_U2|21| A18                  |
     D15 |22| A17                  |
     D14 |23| A16                  |
     D13 |24| A15                  |
     D12 |25| A14                  |
     D11 |26| A13                  |
     D10 |27| A12                  |
     VCC |28| VCC                  |
     D09 |29| A11                  |
     D08 |30| A10                  |  CHIP U1
     D07 |31| A09                  |
     D06 |32| A08                  |
     D05 |33| A07                  |
     D04 |34| A06                  |
     D03 |35| A05                  |
     D02 |36| A04                  |
     GND |37| GND                  |
     D01 |38| A03                  |
     D00 |39| A02                  |
      CE |40| A01                  |
      OE |41| A00                  |
    WE_U1|42| A00         ---------+
    RE_U3|43| A01                  |
     D00 |44| A02                  |
     D01 |45| A03                  |
     VCC |46| VCC                  |
     D02 |47| A04                  |
     D03 |48| A05                  |
     D04 |49| A06                  |
     D05 |50| A07                  |
     D06 |51| A08                  |
     D07 |52| A09                  |
     GND |53| GND                  |
     D08 |54| A10                  | CHIP U3
     D09 |55| A11                  |
     D10 |56| A12                  |
     D11 |57| A13                  |
     D12 |58| A14                  |
     D13 |59| A15                  |
     VCC |60| VCC                  |
     D14 |61| A16                  |
     D15 |62| A17                  |
      CE |63| A18                  |
      OE |64| A19                  |
      WE |65| A20                  |
     GND |66| GND         ---------+
    --------------
      A  |  |  B
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
	required_device<h83007_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<okim9810_device> m_oki;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void amuzy_map(address_map &map) ATTR_COLD;

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
	m_maincpu->read_port7().set_ioport("IN0");
	m_maincpu->read_portb().set_ioport("IN1");

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
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	// boobood PC=1756 xor.b #h'f8, r4l (-> active high for bits 0-2, hopper related?)
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

ROM_START( mmhammer )
	ROM_REGION(0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(f6aa7880) SHA1(c3dfdc5250875c365c7146b6fe6288d1605d17e5) )

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(beb65917) SHA1(835a0ceef2fdfee2730d88e04a4a131575048979) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x000000, 0x200000, CRC(cedb6c55) SHA1(c2981b2547468723da6f5416a81b937b293576fc) )
ROM_END

ROM_START( docchift )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(b69d97f6) SHA1(8ecb6300d435200cf694f6f0d6a847d60354dbae) )

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(044f004b) SHA1(c9f8797fcd5f67831311e4fea2621d7337c74fa2) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x000000, 0x200000, CRC(fb668dbd) SHA1(49514b0c886578f065e47d9c7a5453e09622ba55) )
ROM_END

ROM_START( amhbattl )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(b24f7bf4) SHA1(254e814c26a1430d6fecc68e07e7ee2cdab77f21) )

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(cb1fd823) SHA1(f425a37ca425315f294366298146c3f6547a28c0) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x000000, 0x200000, CRC(f5bfb1e8) SHA1(e36be311782e4bcbd00a8bc93473f23e5c39c67a) )
ROM_END

ROM_START( shpchamp ) // HA9022-0
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004.u1", 0x000000, 0x080000, CRC(194dc931) SHA1(eae05e6627d09daa4b71154f665237348bf0947c) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "s29al016.u2", 0x000000, 0x200000, CRC(677cf07b) SHA1(f30892aa18da14c2077e8847a295112616b06386) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "s29al016.u3", 0x000000, 0x200000, CRC(d218c777) SHA1(7cb09925419864ec784f31802b10e9649eba3e58) )
ROM_END

ROM_START( zenponta ) // HA9020-0
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "29f400.u1", 0x000000, 0x080000, CRC(1debce88) SHA1(035cb45da6c44fa54756282401003c60b44174eb) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "29f016.u2",  0x000000, 0x200000, CRC(0ae3354b) SHA1(05c22650ab8c60ad09d30ca83c0d3c628f4be622) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "29lv160.u3", 0x000000, 0x200000, CRC(76ed6b4a) SHA1(5b3523353771d91c0382b8c2440cae795a8207ed) ) // 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

ROM_START( wwdash )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "s29al004d70tfi01.u1", 0x000000, 0x080000, CRC(b8984518) SHA1(23f8f2988a44bf0f0a14c39363f97f8e1b12cf62) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "s29al016d70tfi01.u2", 0x000000, 0x200000, CRC(c02c4fed) SHA1(e3fd56621fc7ca4558f3044a1b13b6b58dd4c368) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "s29al016d70tfi01.u3", 0x000000, 0x200000, CRC(cd66dc10) SHA1(76051b248882f7b5c0adfd0ae62283a16d440523) )
ROM_END

ROM_START( boobood ) // ブーブードンパッチ, HA9019-0
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "29f400.u1", 0x000000, 0x080000, CRC(79411d05) SHA1(31727db741aa14224e60d7baa817081ba272c8cf) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "29f016a.u2", 0x000000, 0x200000, CRC(edf8d7e2) SHA1(aee4d70dfd0fcb0d766365722b8712bb3a63efb4) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "29lv160.u3", 0x000000, 0x200000, CRC(684523f0) SHA1(ca96918abde4ec63f33d82cf30b121f2ac05f68d) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

ROM_START( fishbatl ) // フィッシャーマンバトル, HA9008-0
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "29f400.u1", 0x000000, 0x080000, CRC(9d6a8322) SHA1(4819370bb1f092f1c018353f153e623e0297a263) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "29f016a.u2", 0x000000, 0x200000, CRC(95f664f8) SHA1(050f074f7646336b6ce7f5e9c34d50d68d0a9a00) )

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "29lv160.u3", 0x000000, 0x200000, CRC(dc8f6b48) SHA1(75f92d38b1e716e3c1c0cdb25f444671789dd23a) )
ROM_END

ROM_START( wanpakup ) // わんぱくパイレーツ, HA9020-0
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "29f400.u1", 0x000000, 0x080000, CRC(8280d58f) SHA1(2be99ae6ddae795495a09fcbab55c880a0adb890) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION(0x1000000, "oki", ROMREGION_ERASEFF )
	ROM_LOAD( "29f016a.u2", 0x000000, 0x200000, CRC(ae4b65ad) SHA1(5afb0dbaa37ba90c03ed18929062f6e2c136dca0) ) // 1xxxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION(0x200000, "gfx", 0 )
	ROM_LOAD( "29lv160.u3", 0x000000, 0x200000, CRC(91fe39d5) SHA1(e900824a9edc47edb9444812daa2e416f8365e0c) ) // 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

}   // anonymous namespace

GAME( 2005, boobood,   0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Boo Boo Donpatchi (Japan, ver 1.01)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, fishbatl,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Fisherman Battle (Japan, ver 1.03)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2005, zenponta,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Zenigata Ponta (Japan, ver 1.02)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2006, amhbattl,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Acchi Muite Hoi Battle (Japan, ver 1.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2006, wanpakup,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Wanpaku Pirates (Japan, ver 1.00)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2007, docchift,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Docchi Fighter (Japan, ver 1.02)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2007, wwdash,    0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Wan Wan Dash (Japan, ver 1.01)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2008, mmhammer,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Mogu Mogu Hammer (Japan, ver 1.01)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2008, shpchamp,  0, amuzy, amuzy, amuzy_state, empty_init, ROT0, "Amuzy Corporation", "Shippe Champion (Japan, ver 1.02)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

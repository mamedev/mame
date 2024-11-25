// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Jackpot by Electronic Projects

TODO:
- Missing master unit comms (for game override?)
  cfr. loads of unused GFXs, irq and NMI in work RAM space
- Glitches in attract, service and D-Up screens (requires an NMI mask?)
- Palette, RAMDAC?
- Sound;
- Complete I/O, identify chip type(s);
- EEPROM defaults (needs to be initialized in service mode for proper operation);

Notes:
The title is wrongly spelt as 'Jeckpot' in the main CPU ROM

===================================================================================================

PCB: GAME CPU MOD.02

1x TMPZ84C0010PEC or compatible 8-bit Microprocessor - main
1x MC6845P CRT Controller (CRTC)
1x Z84C0010PEC or compatible 8-bit Microprocessor - sound
1x AY-3-8910 Programmable Sound Generator - sound
1x YM3526 FM Operator Type-L (OPL) - sound (not populated on one PCB, possibly removed?)
1x LM324N Quad Operational Amplifier - sound
2x LM358N Dual Operational Amplifier - sound
1x Y3014B D/A Converter (DAC) - sound
1x TLC7524CN 8-bit Multiplying Digital-to-Analog Converter (DAC)
1x TDA2003 Audio Amplifier - sound
1x oscillator 13.000MHz
1x oscillator 3.579545
1x yellow resonator 500
1x 28x2 JAMMA edge connector
1x pushbutton (TEST)
1x trimmer (VOLUME)
1x green LED
1x red LED

NOTE: the PCB from which Version 16 was dumped has the following components removed
(cost cutting measure?): YM3256, 1 LM358N, Y3014B, 16 TTL74xx

and some resistors and caps. Strangely, the sound ROMs are unchanged.

In the same period Electronic Projects also released games on different platforms
(see jackpool.cpp and spool99.cpp)

**************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class jackpot_state : public driver_device
{
public:
	jackpot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io(*this, "IN%u", 0U)
		, m_crtc(*this, "crtc")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vram(*this, "vram")
		, m_attr(*this, "attr")
	{
	}

	void jackpot(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<2> m_io;
	required_device<mc6845_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_attr;

	tilemap_t *m_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(get_tile_info);
	void vram_w(offs_t offset, uint8_t data);
	void attr_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
};


void jackpot_state:: video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackpot_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 48, 32);
}

TILE_GET_INFO_MEMBER(jackpot_state::get_tile_info)
{
	const u8 attr = m_attr[tile_index];
	const u16 code = m_vram[tile_index] | ((attr & 0x1f) << 8);
	// TODO: pinpoint how it banks to the other ROM
	// (definitely can mix in the scanline renderer)
	//const u8 color = (attr & 0xe0) >> 6;
	const u8 gfx_region = 0;
	tileinfo.set(gfx_region, code, 0, 0);
}

void jackpot_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void jackpot_state::attr_w(offs_t offset, uint8_t data)
{
	m_attr[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}


uint32_t jackpot_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect);
	return 0;
}


void jackpot_state::prg_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom(); // after this, ROM is 0xff filled but in the range 0x1e000 - 0x1efff. Where does it get mapped?
	// 0xd514, 0xd516 triplets for a 1bpp RAMDAC (???)
	map(0xd518, 0xd51f).lr8(
		NAME([this] (offs_t offset) { return BIT(m_io[0]->read(), offset); })
	);
	map(0xd520, 0xd527).lr8(
		NAME([this] (offs_t offset) { return BIT(m_io[1]->read(), offset); })
	);
	map(0xd600, 0xd600).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xd601, 0xd601).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0xd700, 0xd707).rw("uart", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));

	map(0xd800, 0xdfff).ram().share("nvram"); // MK48Z02B?
	// 384x256 needs 0x600 bytes, assume the rest unmapped
	map(0xe000, 0xe5ff).ram().share(m_vram).w(FUNC(jackpot_state::vram_w));
	map(0xe600, 0xefff).noprw();
	map(0xf000, 0xf5ff).ram().share(m_attr).w(FUNC(jackpot_state::attr_w));
	map(0xf600, 0xf7ff).noprw();
}

void jackpot_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	// bank?
	map(0xe000, 0xe7ff).ram();
}

void jackpot_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ymsnd", FUNC(ym3526_device::write));
	map(0x04, 0x04).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x06, 0x06).w("aysnd", FUNC(ay8910_device::address_w));
}


static INPUT_PORTS_START( jackpot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) // TODO: unconfirmed
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Double Up")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 ) // literally labeled as such in service mode, unused?

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Cancel")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// no dips on PCB
INPUT_PORTS_END


static const gfx_layout jackpot_layout = // taken from jackpool.cpp, needs work
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_jackpot )
	GFXDECODE_ENTRY( "gfx1", 0, jackpot_layout,   0x00, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, jackpot_layout,   0x00, 1 )
GFXDECODE_END


void jackpot_state::jackpot(machine_config &config) // clocks not verified
{
	Z80(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &jackpot_state::prg_map);
	// TODO: IM1, irq0 and NMI valid (vectors provided in work RAM space)
	// irq0 used for serial connection with master unit
	// NMI must be at least masked and tied to 6845 vsync
	m_maincpu->set_vblank_int("screen", FUNC(jackpot_state::nmi_line_pulse));

	z80_device &audiocpu(Z80(config, "audiocpu", 3.579545_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &jackpot_state::sound_prg_map);
	audiocpu.set_addrmap(AS_IO, &jackpot_state::sound_io_map);

	EEPROM_93C66_8BIT(config, "eeprom");
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// TODO: Serial comms with a master unit for game override?
	ins8250_device &uart(INS8250(config, "uart", 1843200)); // actual type and clock unknown
	uart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// reset by the CRTC with 384x256, 55.93 Hz (tentative)
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(jackpot_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_jackpot);
	PALETTE(config, "palette").set_entries(0x100);

	MC6845(config, m_crtc, 13_MHz_XTAL / 16); // divisor guessed
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
//  m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	SPEAKER(config, "mono").front_center();

	ym3526_device &ymsnd(YM3526(config, "ymsnd", 3.579545_MHz_XTAL));
	ymsnd.irq_handler().set_inputline("audiocpu", INPUT_LINE_IRQ0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.30);

	AY8910(config, "aysnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( jackpot )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "electronic_projects_ver_0.16.16l.bin", 0x00000, 0x20000, CRC(9affed89) SHA1(397b5698468a61acd98c7ab50449aaf9fae8fab8) )

	ROM_REGION(0x110000, "audiocpu", 0)
	ROM_LOAD( "electronic_projects_ver_snd02.bin", 0x000000, 0x10000, CRC(91ff806e) SHA1(4035ed34bf00e655865a096863381e84bde02bbc) )
	ROM_LOAD( "electronic_projects_ver_wav1.bin",  0x010000, 0x80000, CRC(83c8f52c) SHA1(216009aeceee7ed8883cde708895d06934d33092) )
	ROM_LOAD( "electronic_projects_ver_wav2.bin",  0x090000, 0x80000, CRC(993a8a01) SHA1(91f9e61dd1844dba079886f97582f74d6191c645) )

	ROM_REGION(0x40000, "gfx1", 0)
	ROM_LOAD( "electronic_projects_ver_cm0116.bin", 0x00000, 0x10000, CRC(48828c6c) SHA1(45769616ca3d88b3fe36fd4c7cfad47478e26825) )
	ROM_LOAD( "electronic_projects_ver_cm0216.bin", 0x10000, 0x10000, CRC(29e37e9c) SHA1(cc6db27f0d6d45cdfd93c89cac0b8c975e96ea97) )
	ROM_LOAD( "electronic_projects_ver_cm0316.bin", 0x20000, 0x10000, CRC(ac70bfaf) SHA1(1b76110a90685c1a285f1ce2312890d630129515) )
	ROM_LOAD( "electronic_projects_ver_cm0416.bin", 0x30000, 0x10000, CRC(7a2490bd) SHA1(1ca35afb6231247f898310c088997e7645657bc8) )

	ROM_REGION(0x80000, "gfx2", 0)
	ROM_LOAD( "electronic_projects_ver_cf0116.bin", 0x00000, 0x20000, CRC(a1c35934) SHA1(bad97a660b2f039c7274496cdf9c4d5dd38eeea4) )
	ROM_LOAD( "electronic_projects_ver_cf0216.bin", 0x20000, 0x20000, CRC(00d66627) SHA1(67185474181b10b7a2fc1f7a43f0c8e2a9d8eee3) )
	ROM_LOAD( "electronic_projects_ver_cf0316.bin", 0x40000, 0x20000, CRC(7d8c8781) SHA1(5085301332c4f642f8c6157eda1aaad1866da246) )
	ROM_LOAD( "electronic_projects_ver_cf0416.bin", 0x60000, 0x20000, CRC(c266c470) SHA1(5042c6a2d554e1e2437d2d79c5aa7c9c3642e173) )

	ROM_REGION(0x800, "nvram", 0)
	ROM_LOAD( "mk48z02b.bin", 0x000, 0x800, CRC(b182f4f3) SHA1(b6f04c343fefca2c506a99fb3293697e6e8522f6) )

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD( "93c66.bin", 0x000, 0x200, CRC(6c81bdb6) SHA1(1514bc690f66950862c2f7732a9d5b951ae83666) )

	ROM_REGION(0x700, "plds", 0) // all read protected
	ROM_LOAD( "palce22v8h-25pc-4", 0x000, 0x2dd, NO_DUMP )
	ROM_LOAD( "palce20v8h-25pc-4", 0x300, 0x157, NO_DUMP )
	ROM_LOAD( "gal16v8s-20hb1",    0x500, 0x117, NO_DUMP )
ROM_END

ROM_START( jackpota )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "new16.bin", 0x00000, 0x20000, CRC(d20ef98a) SHA1(a7d4384c7b965a7a2bbe2ef11ffdd40328e74345) )

	ROM_REGION(0x110000, "audiocpu", 0)
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.10.bin", 0x000000, 0x10000, CRC(91ff806e) SHA1(4035ed34bf00e655865a096863381e84bde02bbc) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.12.bin", 0x010000, 0x80000, CRC(83c8f52c) SHA1(216009aeceee7ed8883cde708895d06934d33092) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.11.bin", 0x090000, 0x80000, CRC(993a8a01) SHA1(91f9e61dd1844dba079886f97582f74d6191c645) )

	// both gfx regions have duplicated GFXs with different bitplane info
	// is it accessible by the game via a bank?
	ROM_REGION(0x80000, "gfx1", 0)
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.2.bin", 0x000000, 0x20000, CRC(c1fcc900) SHA1(7c2e8f3e59fa2af99a358127705e8b49d577d777) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.3.bin", 0x020000, 0x20000, CRC(6f9eac8e) SHA1(a775d23fb70b69e573a94f2cd33dfc7664f158c2) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.4.bin", 0x040000, 0x20000, CRC(6301c5ca) SHA1(6ca1d4c34ad5a25608ffd44e3e0ab40d546a3d7c) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.5.bin", 0x060000, 0x20000, CRC(b60fb8ef) SHA1(5597bdff871d5a80efe1f054d0a7bf9820d4f75f) )

	ROM_REGION(0x100000, "gfx2", 0)
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.6.bin", 0x000000, 0x40000, CRC(1c6cde6c) SHA1(89d0274083e4b94b9c6715084b30b5ee5db1d96e) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.7.bin", 0x040000, 0x40000, CRC(07f1f69e) SHA1(ff966f1b1da8ea81a06f909dae3fa57d88bbd496) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.8.bin", 0x080000, 0x40000, CRC(e7090d10) SHA1(1a72b13d68b7e6c9182ec4786f6bb578e9fc8f0e) )
	ROM_LOAD( "jackpot_electronic_projects_eprom_n.9.bin", 0x0c0000, 0x40000, CRC(ee76110c) SHA1(5634f8de0cc8613b3f016abc7614a2abb0730060) )

	ROM_REGION(0x800, "nvram", 0)
	ROM_LOAD( "mk48z02b.bin", 0x000, 0x800, CRC(005d9ae1) SHA1(cd738ed44c7f016948a42612d37d00b0a56658c3) )

	ROM_REGION(0x200, "eeprom", 0)
	ROM_LOAD( "93c66.bin", 0x000, 0x200, CRC(4a3811bf) SHA1(dfebaeacd2dcf11d1ed08c1317f1b9669c0bad54) )

	ROM_REGION(0x600, "plds", 0) // all read protected
	ROM_LOAD( "gal20v8a-25lp", 0x000, 0x157, NO_DUMP )
	ROM_LOAD( "yp20v8e-25pc",  0x200, 0x157, NO_DUMP )
	ROM_LOAD( "hy18cv8s-25",   0x400, 0x117, NO_DUMP )
ROM_END

} // Anonymous namespace


// Both mentions "QL ELETTRONICA" as HW manufacturer during attract
GAME( 1998, jackpot,        0, jackpot, jackpot, jackpot_state, empty_init, ROT0, "Electronic Projects", "Jackpot Bowling (Ver 16.16L)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // 08.09.98
GAME( 1998, jackpota, jackpot, jackpot, jackpot, jackpot_state, empty_init, ROT0, "Electronic Projects", "Jackpot Bowling (Ver 1.16L)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // 01.09.98 title mispelled as "Jackpot Bowlinng" on initial full screen title, otherwise correct during attract
